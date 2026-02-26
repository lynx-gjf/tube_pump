#include "ttl_send.h"
#include "ttl_comm.h" // 使用 configureTTL / sendBytesToChannel / readBytesFromChannel / sendToChannel / readFromChannel
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <Arduino.h> // 包含 Arduino.h 以使用 String、millis() 等功能

// 内部帮助：将数据发送到指定 TTL 通道（直接使用 C 数组）
// 直接调用 ttl_comm 的字节级接口，避免中间 String 分配，提高效率
static void send_bytes_to_channel_native(uint8_t txPin, uint8_t rxPin, const uint8_t* data, size_t length) {
	if (data == nullptr || length == 0) return;
	// sendBytesToChannel 的参数顺序为 (txPin, rxPin, data, length)
	sendBytesToChannel(txPin, rxPin, data, length);
}

extern "C" {

// 旧的字节接口（保留以兼容）
// 直接使用传入缓冲区，不再构造 std::vector
void send_bytes_channel(uint8_t txPin, uint8_t rxPin, const uint8_t* data, size_t length) {
	send_bytes_to_channel_native(txPin, rxPin, data, length);
}

void send_bytes_serial(const uint8_t* data, size_t length) {
	if (data == nullptr || length == 0) return;
	Serial.write(data, length);
	Serial.flush();
}

// 新增：发送字符串到指定通道（使用 Arduino String）
// 该接口通过 sendToChannel（文本接口）实现
void send_string_channel(uint8_t txPin, uint8_t rxPin, const String& str) {
	if (str.length() == 0) return;
	sendToChannel(txPin, rxPin, str);
}

// 新增：通过默认 Serial 发送字符串
void send_string_serial(const String& str) {
	if (str.length() == 0) return;
	Serial.print(str);
	Serial.flush();
}

// 新版：从指定通道读取到 Arduino String
String read_string_channel(uint8_t rxPin, uint8_t txPin, unsigned long timeoutMs) {
	// 直接使用 ttl_comm 的文本接口，返回 String
	return readFromChannel(rxPin, txPin, timeoutMs);
}

// 新版：从默认 Serial 读取字符串到 Arduino String
String read_string_serial(unsigned long timeoutMs) {
	String out;
	unsigned long start = millis();
	while (millis() - start < timeoutMs) {
		while (Serial.available()) {
			int c = Serial.read();
			if (c < 0) continue;
			out += (char)c;
		}
		yield();
	}
	return out;
}

} // extern "C"