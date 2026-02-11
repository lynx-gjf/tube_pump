#include "ttl_comm.h"
#include "ttl_send.h"
#include "cmd_parser.h"

/*
 Name:		pumps.ino
 Created:	2026/1/12 17:12:28
 Author:	gwq
*/

#define LED_BUILTIN 13

const uint8_t rxPins[3] = { 2, 3, 4 };
const uint8_t txPins[3] = { 5, 6, 7 };

// 每通道单次读取的最大等待（ms），可根据可靠性/延迟调整
constexpr unsigned long perChannelTimeoutMs = 50;

// 本地用于读取的缓冲区长度（与 ttl_send 中 read_string_channel 的要求一致）
constexpr size_t LOCAL_READ_BUF = 128;

// 当前使用的 TTL 通道数（固定）
constexpr int ttl_channel = 3;

void setup() {
	Serial.begin(9600); // 调试串口
	for (int cf = 0; cf < ttl_channel; ++cf) {
		configureTTL(rxPins[cf], txPins[cf], TTL_BAUD); // 初始化 TTL 通道，设置波特率
	}
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, LOW);
}

void loop() {
	delay(1000);
	// 从 Serial 读取输入到 String
	String buf_serial = read_string_serial(perChannelTimeoutMs);

	// 基本输入长度校验（需要至少包含索引 1 和后续数据）
	/*if (buf_serial.length() < 3) {
		Serial.println("Err: input too short");
		delay(10);
		return; // 或者 continue; 视上下文决定；在 Arduino loop 中使用 return 会结束本次 loop 执行
	} */

	// 通道号在第二位（索引1），使用 char 解析并转换为 0 基索引
	char ch = buf_serial.charAt(1);
	/*if (ch < '1' || ch > char('0' + ttl_channel)) {
		Serial.println("Err: invalid channel");
		delay(100);
		return;
	} */
	int channel_num = (ch - '1'); // 将 '1','2','3' 转为 0,1,2

	String pre_send = buf_serial.substring(3);
	pre_send.trim(); // 去除前后空白
	pre_send = pre_send + "\r\n";
	if (pre_send.length() > 5) {
		send_string_channel(txPins[channel_num], rxPins[channel_num], pre_send);
		digitalWrite(LED_BUILTIN, HIGH);
		delay(100); // 短暂延时，确保发送完成
		digitalWrite(LED_BUILTIN, LOW);
		String got_ttl = read_string_channel(rxPins[channel_num], txPins[channel_num], 200);
		// 输出 TTL 响应到 Serial 监视器
		if (got_ttl.length() > 0) {
			Serial.println(got_ttl);
			digitalWrite(LED_BUILTIN, HIGH);
			delay(100);
			digitalWrite(LED_BUILTIN, LOW);
		}
	}
	
	// 延时，避免占用过高 CPU（根据实时需求可调整或移除）
	delay(1000);
}
