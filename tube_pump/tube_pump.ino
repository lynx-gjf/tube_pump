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
const uint8_t txPins[3] = { 8, 9, 10 };

// 每通道单次读取的最大等待（ms），可根据可靠性/延迟调整
constexpr unsigned long perChannelTimeoutMs = 30;

// 本地用于读取的缓冲区长度（与 ttl_send 中 read_string_channel 的要求一致）
constexpr size_t LOCAL_READ_BUF = 128;

// 当前使用的 TTL 通道数（固定）
constexpr int ttl_channel = 3;

void setup() {
	Serial.begin(9600); // 调试串口
	for (int cf = 0; cf < ttl_channel; ++cf) {
		configureTTL(rxPins[cf], txPins[cf], TTL_BAUD);
	}
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, LOW);
}

void loop() {
	// 指示灯亮，表明主循环在运行
	digitalWrite(LED_BUILTIN, HIGH);

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

	Serial.println(String("ch: ") + ch);
	Serial.println(String("idx: ") + channel_num);
	Serial.println(String("payload: ") + pre_send);

	// 将 Serial 输入转发到对应 TTL 通道（注意 tx/rx 参数顺序）
	send_string_channel(rxPins[channel_num], txPins[channel_num], pre_send);
	char* aa = "P1, Q1, 1\r\n";
	send_string_channel(rxPins[2], txPins[2], aa);
	delay(10000);
	char* ab = "P1, Q1, 0\r\n";
	send_string_channel(rxPins[2], txPins[2], ab);
	delay(10000);

	// 三个通道按顺序轮询
	String buf_ttl[ttl_channel];
	for (int st = 0; st < ttl_channel; ++st) {
		// read_string_channel: 将调用 configureTTL 并在超时内返回 String（可能为空）
		String got_ttl = read_string_channel(rxPins[st], txPins[st], perChannelTimeoutMs);
		buf_ttl[st] = got_ttl;
		// 让出一点时间给其他系统任务
		yield();
		Serial.print(st);
		Serial.println(buf_ttl[st]);
		delay(5);
	}

	// 延时，避免占用过高 CPU（根据实时需求可调整或移除）
	delay(1000);
}
