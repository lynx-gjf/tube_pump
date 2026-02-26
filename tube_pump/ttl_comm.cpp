#include "ttl_comm.h"
#include <Arduino.h> // 包含 Arduino.h 以使用 String、millis() 等功能

#if defined(ARDUINO_ARCH_AVR)
#include <SoftwareSerial.h> // 使用 SoftwareSerial（针对 Arduino Uno R3）
#define USE_SOFTWARESERIAL 1
#else
#define USE_SOFTWARESERIAL 0
#endif

// 始终使用 Stream 指针作为通用接口
static Stream* currentStream = nullptr;

constexpr size_t MAX_CHANNELS = 6;

struct ChannelEntry {
    Stream* inst;
    uint8_t rx;
    uint8_t tx;
    unsigned long baud;
    bool owns; // true if we allocated the instance (needs deletion)
};

static ChannelEntry channels[MAX_CHANNELS] = { { nullptr, 0xFF, 0xFF, 0, false } };

void configureTTL(uint8_t rxPin, uint8_t txPin, unsigned long baud) {
    // 尝试查找匹配的现有通道
    for (size_t i = 0; i < MAX_CHANNELS; ++i) {
        if (channels[i].inst && channels[i].rx == rxPin && channels[i].tx == txPin && channels[i].baud == baud) {
            currentStream = channels[i].inst;
#if USE_SOFTWARESERIAL
            // SoftwareSerial 需要显式 listen()
            SoftwareSerial* ss = static_cast<SoftwareSerial*>(channels[i].inst);
            ss->listen();
#endif
            return;
        }
    }

    // 创建新通道
    for (size_t i = 0; i < MAX_CHANNELS; ++i) {
        if (channels[i].inst == nullptr) {
            channels[i].rx = rxPin;
            channels[i].tx = txPin;
            channels[i].baud = baud;
#if USE_SOFTWARESERIAL
            SoftwareSerial* ss = new SoftwareSerial(rxPin, txPin); // 构造（rx, tx）
            ss->begin(baud);
            ss->listen();
            channels[i].inst = ss;
            channels[i].owns = true;
#else
            // 在非 AVR 平台（例如 ESP32）回退到内置的 Serial
            Serial.begin(baud);
            channels[i].inst = &Serial;
            channels[i].owns = false;
#endif
            currentStream = channels[i].inst;
            return;
        }
    }

    // 通道表已满，复用槽 0。仅当我们实际分配了实例时才 end()/delete。
    if (channels[0].inst) {
        if (channels[0].owns) {
#if USE_SOFTWARESERIAL
            SoftwareSerial* ss = static_cast<SoftwareSerial*>(channels[0].inst);
            ss->end();
            delete ss;
#endif
            channels[0].inst = nullptr;
            channels[0].owns = false;
        }
    }

    channels[0].rx = rxPin;
    channels[0].tx = txPin;
    channels[0].baud = baud;
#if USE_SOFTWARESERIAL
    {
        SoftwareSerial* ss = new SoftwareSerial(rxPin, txPin);
        ss->begin(baud);
        ss->listen();
        channels[0].inst = ss;
        channels[0].owns = true;
    }
#else
    Serial.begin(baud);
    channels[0].inst = &Serial;
    channels[0].owns = false;
#endif
    currentStream = channels[0].inst;
}

void sendToChannel(uint8_t txPin, uint8_t rxPin, const String& data) {
    configureTTL(rxPin, txPin, TTL_BAUD);
    if (!currentStream) return;
    currentStream->print(data);
    // 在 SoftwareSerial 上调用 flush(), 在其他实现上调用通用 flush()
    for (size_t i = 0; i < (sizeof(channels)/sizeof(channels[0])); ++i) {
        if (channels[i].inst && channels[i].inst == currentStream) {
#if USE_SOFTWARESERIAL
            static_cast<SoftwareSerial*>(channels[i].inst)->flush();
#else
            channels[i].inst->flush();
#endif
            break;
        }
    }
}

String readFromChannel(uint8_t rxPin, uint8_t txPin, unsigned long timeoutMs) {
    String result;
    configureTTL(rxPin, txPin, TTL_BAUD);
    if (!currentStream) return result;

    unsigned long start = millis();
    while (millis() - start < timeoutMs) {
        while (currentStream->available()) {
            int c = currentStream->read();
            if (c < 0) continue;
            result += (char)c;
        }
        yield();
        delay(1);
    }
    return result;
}

void sendBytesToChannel(uint8_t txPin, uint8_t rxPin, const uint8_t* data, size_t length) {
    if (data == nullptr || length == 0) return;
    configureTTL(rxPin, txPin, TTL_BAUD);
    if (!currentStream) return;
    // 使用 Print::write 的签名写入字节数组
    currentStream->write(data, length);
    for (size_t i = 0; i < (sizeof(channels)/sizeof(channels[0])); ++i) {
        if (channels[i].inst && channels[i].inst == currentStream) {
#if USE_SOFTWARESERIAL
            static_cast<SoftwareSerial*>(channels[i].inst)->flush();
#else
            channels[i].inst->flush();
#endif
            break;
        }
    }
}

size_t readBytesFromChannel(uint8_t rxPin, uint8_t txPin, uint8_t* outBuffer, size_t maxLen, unsigned long timeoutMs) {
    if (outBuffer == nullptr || maxLen == 0) return 0;
    configureTTL(rxPin, txPin, TTL_BAUD);
    if (!currentStream) return 0;

    size_t written = 0;
    unsigned long start = millis();
    while (millis() - start < timeoutMs) {
        while (currentStream->available()) {
            int c = currentStream->read();
            if (c < 0) continue;
            if (written < maxLen) {
                outBuffer[written++] = (uint8_t)c;
            }
        }
        yield();
        delay(1);
    }
    return written;
}