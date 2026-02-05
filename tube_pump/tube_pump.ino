#include "ttl_comm.h"
#include "ttl_send.h"
#include "hex_reader.h"

using namespace std;

static unsigned long throughputWindowStart = 0;
static size_t throughputBytes = 0;
// 简单消息队列：存放最近接收到的消息
static vector<vector<uint8_t>> messageQueue;
// 限制队列大小，避免内存无限增长
constexpr size_t MAX_QUEUE_MESSAGES = 16;

/*
 Name:		pumps.ino
 Created:	2026/1/12 17:12:28
 Author:	gwq
*/

#define LED_BUILTIN1 12
#define LED_BUILTIN2 13

// TTL 发送引脚 (TX)
#define TTL_TX1 1
#define TTL_TX2 18
#define TTL_TX3 19

// TTL 接收引脚 (RX)
#define TTL_RX1 2
#define TTL_RX2 10
#define TTL_RX3 6

// TTL 接收引脚 (RX)
#define OUTPUT1 7
#define OUTPUT2 5
#define OUTPUT3 4

// 简单处理函数：打印摘要、做个示例校验（异或）并把消息入队
void handleReceivedMessage(const std::vector<uint8_t>& msg) {
	if (msg.empty()) return;

	// 打印摘要
	Serial.print("handleReceivedMessage: len=");
	Serial.print(msg.size());
	Serial.print(" first=");
	if (msg.size()) {
		if (msg[0] < 16) Serial.print('0');
		Serial.print(msg[0], HEX);
	}
	Serial.println();

	// 示例：计算简单 XOR 校验（不改变 msg）
	uint8_t xorSum = 0;
	for (size_t i = 0; i < msg.size(); ++i) xorSum ^= msg[i];
	Serial.print(" XOR checksum=");
	if (xorSum < 16) Serial.print('0');
	Serial.println(xorSum, HEX);

	// 入队（若队列满，丢弃最早一项）
	if (messageQueue.size() >= MAX_QUEUE_MESSAGES) messageQueue.erase(messageQueue.begin());
	messageQueue.push_back(msg);
}

// 更新吞吐量统计并周期性打印（每秒）
void updateThroughput(size_t newBytes) {
	unsigned long now = millis();
	if (throughputWindowStart == 0) throughputWindowStart = now;
	throughputBytes += newBytes;
	if (now - throughputWindowStart >= 1000) {
		float kbps = (throughputBytes / 1024.0f) / ((now - throughputWindowStart) / 1000.0f);
		Serial.print("Throughput: ");
		Serial.print(throughputBytes);
		Serial.print(" bytes / ");
		Serial.print((now - throughputWindowStart));
		Serial.print(" ms (");
		Serial.print(kbps, 2);
		Serial.println(" KB/s)");
		throughputBytes = 0;
		throughputWindowStart = now;
	}
}

// the setup function runs once when you press reset or power the board
void setup() {
	Serial.begin(9600); // 调试串口
	// 初始化 Serial1 为默认通道 1（可以马上重配置也行）
	configureTTL(TTL_RX1, TTL_TX1, TTL_BAUD);

	pinMode(LED_BUILTIN1, OUTPUT);
	pinMode(LED_BUILTIN2, OUTPUT);
	digitalWrite(LED_BUILTIN1, LOW);
	digitalWrite(LED_BUILTIN2, LOW);
	pinMode(OUTPUT1, OUTPUT);
	pinMode(OUTPUT2, OUTPUT);
	pinMode(OUTPUT3, OUTPUT);
	analogWrite(7, 20);
}

// the loop function runs over and over again until power down or reset
void loop() {
	// LED 切换
	digitalWrite(LED_BUILTIN1, LOW);
	digitalWrite(LED_BUILTIN2, HIGH);

	uint8_t test[] = { 1,2,3,4,5,6,7,8 };
	vector<uint8_t> testVec1 = { 0x55,0x06,0x00,0x05,0x00,0x01,0x55,0xdf };
	vector<uint8_t> testVec2 = { 0x55,0x06,0x00,0x07,0x00,0x01,0xf4,0x1f };
	// 发送到调试串口（示例）

	// 修正：将 vector<uint8_t> 转为指针和长度，并补齐参数
	// send_bytes_channel(TTL_TX1, TTL_RX1, testVec1.data(), testVec1.size());
	// sendToChannel(TTL_TX1, TTL_RX1, "P0,G1,1<CR><LF>");
	// 从调试串口读取并获取字节数组（已经打印过一次）
	vector<uint8_t> dbg = pollAndPrintHex(2000);
	if (!dbg.empty()) {
		updateThroughput(dbg.size());
		handleReceivedMessage(dbg);
	}

	// 从三个 TTL 通道读取（统一接口），处理返回值
	vector<uint8_t> b1 = pollAndPrintHexFromChannel(TTL_RX1, TTL_TX1, 100);
	if (!b1.empty()) {
		updateThroughput(b1.size());
		handleReceivedMessage(b1);
	}

	vector<uint8_t> b2 = pollAndPrintHexFromChannel(TTL_RX2, TTL_TX2, 100);
	if (!b2.empty()) {
		updateThroughput(b2.size());
		handleReceivedMessage(b2);
	}

	vector<uint8_t> b3 = pollAndPrintHexFromChannel(TTL_RX3, TTL_TX3, 100);
	if (!b3.empty()) {
		updateThroughput(b3.size());
		handleReceivedMessage(b3);
	}

	// 可选：从队列中消费消息（示例）
	if (!messageQueue.empty()) {
		// 处理并移除队头
		auto msg = messageQueue.front();
		messageQueue.erase(messageQueue.begin());
		// 做进一步解析/分发（示例只是打印长度）
		Serial.print("Consuming queued msg len=");
		Serial.println(msg.size());
	}

	delay(10000);// 控制主循环频率

	digitalWrite(LED_BUILTIN1, HIGH);
	digitalWrite(LED_BUILTIN2, LOW);

	//send_bytes_channel(TTL_TX1, TTL_RX1, testVec2.data(), testVec2.size()); // 发送到指定通道
	//sendToChannel(TTL_TX1, TTL_RX1, "P0,G1,0<CR><LF>"); // 发送到指定通道
	delay(10000);
}
