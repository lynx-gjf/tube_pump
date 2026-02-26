#include "ttl_comm.h"
#include <Arduino.h> // 使用 String、millis() 等

// 始终使用 Serial2 作为 TTL 通道
static Stream* currentStream = nullptr;

constexpr size_t MAX_CHANNELS = 6;

struct ChannelEntry {
    Stream* inst;
    uint8_t rx;
    uint8_t tx;
    unsigned long baud;
    bool owns;
};

static ChannelEntry channels[MAX_CHANNELS] = { { nullptr, 0xFF, 0xFF, 0, false } };

// 存储用户请求的 Serial2 引脚（仅在支持的平台会生效）
static uint8_t g_serial2_rx = 0;
static uint8_t g_serial2_tx = 1;
static bool g_serial2_pins_requested = false;

// 尝试设置 Serial2 的 RX/TX 引脚。
// 返回 true 表示已调用平台特定的重映射/重启（或核心支持该重映射）。
// 返回 false 表示该平台/核心不支持在运行时重映射 Serial2 引脚；调用者需知晓。
bool setSerial2Pins(uint8_t txPin, uint8_t rxPin)  // 该函数现在仅尝试设置 Serial2 引脚，返回是否成功（即平台是否支持）
{
    g_serial2_rx = rxPin;
    g_serial2_tx = txPin;
    g_serial2_pins_requested = true;

#if defined(ARDUINO_ARCH_ESP32)
    // ESP32 的 HardwareSerial 支持在 begin 时指定 rx/tx
	Serial2.begin(TTL_BAUD, SERIAL_8N1, g_serial2_rx, g_serial2_tx); // 立即配置 Serial2 使用指定引脚
    currentStream = &Serial2;
    // 更新 channels 表以反映硬件使用的串口
    channels[0].inst = &Serial2;
    channels[0].baud = TTL_BAUD;
    channels[0].rx = g_serial2_rx;
    channels[0].tx = g_serial2_tx;
    return true;
#else
    // 其他架构：多数核心不支持在运行时重新映射硬件串口引脚
    // 我们记录请求的引脚以便在可能支持的核心上使用，但返回 false 表示未真正应用
    return false;
#endif
}

// 将 configureTTL 改为仅接受波特率（unsigned long baud）。
// 目的：把配置语义限定为“确保 Serial2 已使用该波特率被配置并置为 currentStream”。
// 注意：rx/tx 参数已从接口中移除；物理引脚由硬件固定或通过 setSerial2Pins 预先指定。
void configureTTL(unsigned long baud)  // 该函数现在仅按波特率配置 Serial2
{
    static unsigned long s_currentBaud = 0;

    // 如果已经是 Serial2 且波特率未变，则无需再次配置
    if (currentStream == &Serial2 && s_currentBaud == baud) {
        return;
    }

    // 在支持的平台上，若用户请求了特定引脚，则在 begin 时指定它们
#if defined(ARDUINO_ARCH_ESP32)
    if (g_serial2_pins_requested) {
		Serial2.begin(baud, SERIAL_8N1, g_serial2_rx, g_serial2_tx); // 在 ESP32 上，HardwareSerial 支持在 begin 时指定 rx/tx 引脚
        // 更新通道信息以反映映射
        channels[0].rx = g_serial2_rx;
        channels[0].tx = g_serial2_tx;
    } else {
        Serial2.begin(baud);
        channels[0].rx = 0xFF;
        channels[0].tx = 0xFF;
    }
#else
    // 其他平台：调用 begin(baud) 使用默认 Serial2 引脚
    Serial2.begin(baud);
    channels[0].rx = 0xFF;
    channels[0].tx = 0xFF;
#endif

    s_currentBaud = baud;

    // 更新 channels 表的第一个槽（用于兼容现有逻辑）
    channels[0].baud = baud;
    channels[0].inst = &Serial2;
    channels[0].owns = false;

    currentStream = &Serial2;
}

void sendToChannel(uint8_t txPin, uint8_t rxPin, const String& data)  // 该函数现在仅按波特率配置 Serial2；保留函数签名以兼容调用方
{
    // 现在仅按波特率配置 Serial2；保留函数签名以兼容调用方
    configureTTL(TTL_BAUD);
	if (!currentStream) return; // 没有可用的串口实例
	currentStream->print(data); // 使用 print 而非 write 以支持 String 对象；底层会调用 write(const char*, size_t)
    // 硬件串口统一使用 flush()
    for (size_t i = 0; i < MAX_CHANNELS; ++i) {
        if (channels[i].inst && channels[i].inst == currentStream) {
			channels[i].inst->flush(); // 确保数据发送完成
            break;
        }
    }
}

String readFromChannel(uint8_t txPin, uint8_t rxPin, unsigned long timeoutMs) {
    String result;
    configureTTL(TTL_BAUD); 
    if (!currentStream) return result;

	unsigned long start = millis(); // 记录开始时间
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


