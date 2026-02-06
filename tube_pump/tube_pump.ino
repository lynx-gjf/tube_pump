#include "ttl_comm.h"
#include "ttl_send.h"
#include "cmd_parser.h"

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

// OUTPUT 引脚
#define OUTPUT1 7
#define OUTPUT2 5
#define OUTPUT3 4


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
	analogWrite(OUTPUT1, 20);
}

// the loop function runs over and over again until power down or reset
void loop() {
	analogWrite(OUTPUT1, 128);
	analogWrite(OUTPUT1, 128);
	analogWrite(OUTPUT1, 128);
	delay(2000);
	analogWrite(OUTPUT1, 1);
	analogWrite(OUTPUT1, 1);
	analogWrite(OUTPUT1, 1);
	// LED 切换
	digitalWrite(LED_BUILTIN1, LOW);
	digitalWrite(LED_BUILTIN2, LOW);

	// 发送到调试串口（示例）
	// 修正：将 vector<uint8_t> 转为指针和长度，并补齐参数
	// send_bytes_channel(TTL_TX1, TTL_RX1, testVec1.data(), testVec1.size());
	// sendToChannel(TTL_TX1, TTL_RX1, "P0,G1,1<CR><LF>");
	
	// 从调试串口读取并获取字节数组（已经打印过一次）
	String dbg = "";
	String zero = "";
	while (dbg.compareTo(zero) == 0)
	{
		char dbgBuf[512]; // 预留足够空间
		int dbgLen = read_string_serial(dbgBuf, sizeof(dbgBuf) - 1, 500); // 传入缓冲区和最大长度
		dbg = String(dbgBuf); // 转换为 String 类型
	} 
	digitalWrite(LED_BUILTIN1, HIGH);
	digitalWrite(LED_BUILTIN2, LOW);
	Serial.print(dbg);
	delay(100);
	auto v = parseCommandString(dbg);

	for (int x : v) {
		Serial.println(x);
	}

	if (v[1] == 1)
	{
		analogWrite(OUTPUT1, v[2]);
	}
	else if (v[1] == 2)
	{
		analogWrite(OUTPUT2, v[2]);
	}
	else if (v[1] == 3)
	{
		analogWrite(OUTPUT3, v[2]);
	}
	dbg = "";
	// 从三个 TTL 通道读取（统一接口），处理返回值
	 //vector<uint8_t> b1 = pollAndPrintHexFromChannel(TTL_RX1, TTL_TX1, 100);
	/* if (!b1.empty()) {
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
	*/

	// 可选：从队列中消费消息（示例）
	/* if (!messageQueue.empty()) {
		// 处理并移除队头
		auto msg = messageQueue.front();
		messageQueue.erase(messageQueue.begin());
		// 做进一步解析/分发（示例只是打印长度）
		Serial.print("Consuming queued msg len=");
		Serial.println(msg.size());
	} */

	delay(100); // 避免过快循环
	// 发送到调试串口（示例）
	//send_bytes_channel(TTL_TX1, TTL_RX1, testVec2.data(), testVec2.size()); // 发送到指定通道
	//sendToChannel(TTL_TX1, TTL_RX1, "P0,G1,0<CR><LF>"); // 发送到指定通道
}
