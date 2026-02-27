#pragma once
#include <stdint.h>
#include <stddef.h>
#include <Arduino.h> // 包含 Arduino.h 以使用 String、millis() 等功能

// 旧的字节接口（保留以兼容）——使用 C linkage，便于被 C 或 extern "C" 的定义链接
#ifdef __cplusplus
extern "C" {
#endif

	void send_bytes_channel(uint8_t txPin, uint8_t rxPin, const uint8_t* data, size_t length);
	void send_bytes_serial(const uint8_t* data, size_t length);
	// 新：字符串接口（使用 Arduino String）
	// 发送到指定 TTL 通道（txPin/rxPin 决定通道），str 为 String
	void send_string_channel(uint8_t txPin, uint8_t rxPin, const String& str);
	// 通过默认 Serial 发送字符串
	void send_string_serial(const String& str);

	// 从指定 TTL 通道读取字符串，返回读取到的 String（可能为空）
	// timeoutMs 为等待时间（ms）
	String read_string_channel(uint8_t rxPin, uint8_t txPin, unsigned long timeoutMs);
	// 从默认 Serial 读取字符串，返回读取到的 String（可能为空）
	String read_string_serial(unsigned long timeoutMs);
}