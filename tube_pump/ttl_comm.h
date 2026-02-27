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

void configureTTL(unsigned long baud = TTL_BAUD); // 配置并切换到指定的 TTL 通道（rxPin, txPin），设置波特率
void sendToChannel(uint8_t txPin, uint8_t rxPin, const String& data); // 发送数据到指定通道（rxPin, txPin），目前仅按波特率配置 Serial2；保留函数签名以兼容调用方
String readFromChannel(uint8_t txPin, uint8_t rxPin, unsigned long timeoutMs = RX_WAIT_MS); // 从指定通道（rxPin, txPin）读取数据，等待 timeoutMs 毫秒；目前仅按波特率配置 Serial2；保留函数签名以兼容调用方
bool setSerial2Pins(uint8_t txPin, uint8_t rxPin); // 该函数现在仅尝试设置 Serial2 引脚，返回是否成功（即平台是否支持）

#endif // TTL_COMM_H