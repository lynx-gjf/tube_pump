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

void sendToChannel(uint8_t txPin, uint8_t rxPin, const String& data); // 发送数据到指定通道（rxPin, txPin），目前仅按波特率配置 Serial2；保留函数签名以兼容调用方
String readFromChannel(uint8_t txPin, uint8_t rxPin, unsigned long timeoutMs = RX_WAIT_MS); // 从指定通道（rxPin, txPin）读取数据，等待 timeoutMs 毫秒；目前仅按波特率配置 Serial2；保留函数签名以兼容调用方


#endif // TTL_COMM_H