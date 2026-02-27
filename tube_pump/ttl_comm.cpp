#include "ttl_comm.h"
#include <Arduino.h>

#if defined(ARDUINO_ARCH_ESP32)
#include <HardwareSerial.h>
#else
#error "ttl_comm.cpp 仅支持 ESP32。"
#endif

// 使用统一的 Stream* 作为 TTL 抽象，当前绑定到 Serial1
static Stream* currentStream = nullptr;

// 缓存当前串口配置，避免重复 begin()
static uint8_t s_rxPin = 0xFF;       // 当前 RX 引脚
static uint8_t s_txPin = 0xFF;       // 当前 TX 引脚
static unsigned long s_baud = 0;     // 当前波特率

// 确保 Serial1 已按指定参数初始化
// 注意入参顺序是 (txPin, rxPin)，但 Serial1.begin 需要 (rxPin, txPin)
static void ensureSerial1(uint8_t txPin, uint8_t rxPin, unsigned long baud)
{
    // 参数未变则直接复用当前配置
    if (currentStream == &Serial1 && s_rxPin == rxPin && s_txPin == txPin && s_baud == baud) {
        return;
    }

    // ESP32: begin(baud, config, rxPin, txPin)
    Serial1.begin(baud, SERIAL_8N1, rxPin, txPin);

    // 更新状态缓存
    currentStream = &Serial1;
    s_rxPin = rxPin;
    s_txPin = txPin;
    s_baud = baud;
}

// 向指定通道发送字符串
void sendToChannel(uint8_t txPin, uint8_t rxPin, const String& data)
{
    ensureSerial1(txPin, rxPin, TTL_BAUD);
    if (!currentStream) return;

    currentStream->print(data);  // 写入数据
    currentStream->flush();      // 等待发送缓冲区刷出
}

// 在超时时间内读取指定通道返回数据
String readFromChannel(uint8_t txPin, uint8_t rxPin, unsigned long timeoutMs)
{
    String result;
    result.reserve(64); // 预留空间，减少 String 扩容次数

    ensureSerial1(txPin, rxPin, TTL_BAUD);
    if (!currentStream) return result;

    const unsigned long start = millis();
    while ((unsigned long)(millis() - start) < timeoutMs) {
        // 读取当前可用的全部字节
        while (currentStream->available() > 0) {
            const int c = currentStream->read();
            if (c >= 0) result += static_cast<char>(c);
        }

        // 让出 CPU，避免忙等
        yield();
        delay(1);
    }

    return result;
}