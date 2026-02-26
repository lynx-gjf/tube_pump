#pragma once
#ifndef TTL_COMM_H
#define TTL_COMM_H

#include <stdint.h>
#include <Arduino.h> // 包含 Arduino.h 以使用 String、millis() 等功能

/// 针对 Arduino R4 Uno：优先使用硬件串口 Serial1
/// 若需要回退到 SoftwareSerial（多路软串口），将下面宏设为 0
#ifndef USE_SERIAL1
#define USE_SERIAL1 1
#endif

constexpr unsigned long TTL_BAUD = 9600;
constexpr unsigned long RX_WAIT_MS = 100; // 默认读等待 ms

void configureTTL(uint8_t rxPin, uint8_t txPin, unsigned long baud = TTL_BAUD); // 配置并切换到指定的 TTL 通道（rxPin, txPin），设置波特率
void sendToChannel(uint8_t txPin, uint8_t rxPin, const String& data);
String readFromChannel(uint8_t rxPin, uint8_t txPin, unsigned long timeoutMs = RX_WAIT_MS);

// 新增：字节级发送/接收（针对 SoftwareSerial 的二进制读写，避免外部直接访问 swSerial）
void sendBytesToChannel(uint8_t txPin, uint8_t rxPin, const uint8_t* data, size_t length);
size_t readBytesFromChannel(uint8_t rxPin, uint8_t txPin, uint8_t* outBuffer, size_t maxLen, unsigned long timeoutMs = RX_WAIT_MS);

#endif // TTL_COMM_H