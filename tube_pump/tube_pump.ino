#include "ttl_comm.h"

/*
 Name:		pumps.ino
 Created:	2026/1/12 17:12:28
 Author:	gwq
*/

#define LED_BUILTIN 13

const uint8_t rxPins[3] = { 2, 3, 4 };
const uint8_t txPins[3] = { 8, 9, 10 };

// 每通道单次读取的最大等待（ms），可根据可靠性/延迟调整
constexpr unsigned long perChannelTimeoutMs = 30;

// 本地用于读取的缓冲区长度（与 ttl_send 中 read_string_channel 的要求一致）
constexpr size_t LOCAL_READ_BUF = 128;

// 当前使用的 TTL 通道数（固定）
constexpr int ttl_channel = 3;

void setup() {
	Serial.begin(TTL_BAUD); // 调试串口
	// 为 UNO R3 预先初始化第 0 通道（可选）
	for (int i = 0; i < ttl_channel; ++i) {
		configureTTL(rxPins[i], txPins[i], TTL_BAUD);
	}
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, LOW);
}

void loop() {
	delay(1000);
	// 从 Serial 读取输入到 String
	String buf_serial = Serial.readString();

	// 通道号在第二位（索引1），使用 char 解析并转换为 0 基索引
	if (buf_serial.length() < 2) return; // 简单保护
	char ch = buf_serial.charAt(1);
	int channel_num = (ch - '1'); // 将 '1','2','3' 转为 0,1,2
	if (channel_num < 0 || channel_num >= ttl_channel) {
		// invalid channel, 忽略
		return;
	}

	String pre_send = buf_serial.substring(3);
	pre_send.trim(); // 去除前后空白
	pre_send = pre_send + "\r\n";
	if (pre_send.length() > 5) {
		sendToChannel(txPins[channel_num], rxPins[channel_num], pre_send);
		digitalWrite(LED_BUILTIN, HIGH);
		delay(100); // 短暂延时，确保发送完成
		digitalWrite(LED_BUILTIN, LOW);
		String got_ttl = readFromChannel(txPins[channel_num], rxPins[channel_num], 200);
		// 输出 TTL 响应到 Serial 监视器
		if (got_ttl.length() > 0) {
			Serial.println(got_ttl);
			digitalWrite(LED_BUILTIN, HIGH);
			delay(100);
			digitalWrite(LED_BUILTIN, LOW);
		}
	}

	// 延时，避免占用过高 CPU（根据实时需求可调整或移除）
	delay(100);
}
