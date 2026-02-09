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

// 当前使用的 TTL 通道索引（0, 1, 2），初始为 0
int ttl_channel = 3;

void setup() {
	Serial.begin(9600); // 调试串口
	configureTTL(TTL_RX1, TTL_TX1, TTL_BAUD); // 初始化默认通道
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, LOW);
}



void loop() {
	// 指示灯亮，表明主循环在运行
	digitalWrite(LED_BUILTIN, HIGH);

	// 三个通道按顺序轮询
	const uint8_t rxPins[3] = { TTL_RX1, TTL_RX2, TTL_RX3 };
	const uint8_t txPins[3] = { TTL_TX1, TTL_TX2, TTL_TX3 };

	char buf[ttl_channel][LOCAL_READ_BUF];
	for (int ch = 0; ch < 3; ++ch) {
		// read_string_channel: 将调用 configureTTL 并在超时内读取到 buf（含终止符）
		size_t got = read_string_channel(rxPins[ch], txPins[ch], buf[ch], sizeof(buf[ch]), perChannelTimeoutMs);
		// 让出一点时间给其他系统任务
		yield();
		Serial.println(buf[ch]);
	}

	// 短延时，避免占用过高 CPU（根据实时需求可调整或移除）
	delay(5);
}
