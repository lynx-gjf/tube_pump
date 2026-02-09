#include "ttl_comm.h"
#include "ttl_send.h"
#include "cmd_parser.h"

/*
 Name:		pumps.ino
 Created:	2026/1/12 17:12:28
 Author:	gwq
*/

#define LED_BUILTIN 13

// TTL 发送引脚 (TX)
#define TTL_TX1 2
#define TTL_TX2 3
#define TTL_TX3 4

// TTL 接收引脚 (RX)
#define TTL_RX1 8
#define TTL_RX2 9
#define TTL_RX3 10

// 每通道单次读取的最大等待（ms），可根据可靠性/延迟调整
constexpr unsigned long perChannelTimeoutMs = 30;

// 本地用于读取的缓冲区长度（与 ttl_send 中 read_string_channel 的要求一致）
constexpr size_t LOCAL_READ_BUF = 128;

// 当前使用的 TTL 通道数（固定）
constexpr int ttl_channel = 3;

void setup() {
	Serial.begin(9600); // 调试串口
	configureTTL(TTL_RX1, TTL_TX1, TTL_BAUD); // 初始化默认通道
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, LOW);
}

void loop() {
	// 指示灯亮，表明主循环在运行
	digitalWrite(LED_BUILTIN, HIGH);
	const uint8_t rxPins[3] = { TTL_RX1, TTL_RX2, TTL_RX3 };
	const uint8_t txPins[3] = { TTL_TX1, TTL_TX2, TTL_TX3 };

	// 从 Serial 读取输入到 String
	String buf_serial = read_string_serial(perChannelTimeoutMs); 

	//字符串处理后发送到 TTL 通道（保持原调用顺序；注意 tx/rx 参数顺序）
	String channel = String(buf_serial[1]);
	int channnel = channel.toInt();
	String pre_send = buf_serial.substring(3);

	// 将 Serial 输入转发到对应 TTL 通道（保持原调用顺序；注意 tx/rx 参数顺序）
	send_string_channel(rxPins[channnel], txPins[channnel], pre_send); // 修正第二个参数为 TX1，且不赋值给 String

	// 三个通道按顺序轮询


	String buf_ttl[ttl_channel];
	for (int st = 0; st < ttl_channel; ++st) {
		// read_string_channel: 将调用 configureTTL 并在超时内返回 String（可能为空）
		String got_ttl = read_string_channel(rxPins[st], txPins[st], perChannelTimeoutMs);
		buf_ttl[st] = got_ttl;
		// 让出一点时间给其他系统任务
		yield();
		Serial.println(buf_ttl[st]);
	}

	// 短延时，避免占用过高 CPU（根据实时需求可调整或移除）
	delay(5);
}
