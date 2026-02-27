#include "ttl_comm.h"
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

/*
 Name:      pumps.ino
 Created:   2026/1/12 17:12:28
 Author:    gwq
*/

// 串口通信状态指示灯：发送/接收分别闪烁
#define LED_BUILTIN1 12
#define LED_BUILTIN2 13

// 三个 TTL 通道的引脚映射（索引一一对应）
// 第 n 个通道使用 txPins[n] / rxPins[n]
const uint8_t rxPins[3] = { 2, 3, 10 };
const uint8_t txPins[3] = { 6, 7, 11 };

// 支持的通道数（命令里对应 #1 / #2 / #3）
constexpr int ttl_channel = 3;

// 每次读取 TTL 响应的超时（毫秒）
constexpr unsigned long ttlReadTimeoutMs = 200;

// 串口输入任务 -> TTL 工作任务 的请求消息
struct TtlRequest {
    uint8_t channel;           // 0..ttl_channel-1
    char payload[96];          // 待发字符串（不含末尾 CRLF）
};

// TTL 工作任务 -> 串口输出任务 的响应消息
struct TtlResponse {
    uint8_t channel;           // 来源通道
    char payload[160];         // 返回内容
    bool hasData;              // 是否收到有效数据
};

// 请求队列 / 响应队列
static QueueHandle_t g_reqQ = nullptr;
static QueueHandle_t g_rspQ = nullptr;

// 解析控制台命令，格式示例：#1:STATUS?
// 含义：向通道1发送 "STATUS?"
static bool parseCommand(const String& line, TtlRequest& req)
{
    String s = line;
    s.trim();

    // 最短格式至少 4 个字符：#1:x
    if (s.length() < 4) return false;

    // 第 2 个字符为通道号
    const char ch = s.charAt(1);
    if (ch < '1' || ch > char('0' + ttl_channel)) return false;

    const int channel_num = ch - '1';

    // 第 4 个字符起为正文（跳过 "#n:"）
    String body = s.substring(3);
    body.trim();
    if (body.length() == 0) return false;

    req.channel = static_cast<uint8_t>(channel_num);
    body.toCharArray(req.payload, sizeof(req.payload));
    return true;
}

// 任务1：负责 USB 串口（Serial）输入输出
static void taskSerialConsole(void* /*pv*/)
{
    // 缩短 readStringUntil 等待时间，避免任务阻塞过久
    Serial.setTimeout(20);

    for (;;) {
        // 1) 从 PC 串口读取命令并投递到请求队列
        if (Serial.available() > 0) {
            String line = Serial.readStringUntil('\n');
            TtlRequest req{};
            if (!parseCommand(line, req)) {
                Serial.println("Err: invalid command");
            } else {
                if (xQueueSend(g_reqQ, &req, pdMS_TO_TICKS(10)) != pdPASS) {
                    Serial.println("Err: request queue full");
                }
            }
        }

        // 2) 从响应队列取回 TTL 响应并打印
        TtlResponse rsp{};
        if (xQueueReceive(g_rspQ, &rsp, pdMS_TO_TICKS(5)) == pdPASS) {
            if (rsp.hasData) {
                Serial.printf("[CH%d] %s\n", rsp.channel + 1, rsp.payload);
                digitalWrite(LED_BUILTIN2, HIGH);
                vTaskDelay(pdMS_TO_TICKS(80));
                digitalWrite(LED_BUILTIN2, LOW);
            } else {
                Serial.printf("[CH%d] <no response>\n", rsp.channel + 1);
            }
        }

        // 主动让出 CPU，提升调度公平性
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

// 任务2：负责 TTL（Serial1）收发
static void taskSerial1Worker(void* /*pv*/)
{
    for (;;) {
        TtlRequest req{};
        if (xQueueReceive(g_reqQ, &req, portMAX_DELAY) == pdPASS) {
            // 根据通道号查表得到对应引脚
            const uint8_t tx = txPins[req.channel];
            const uint8_t rx = rxPins[req.channel];

            // 按设备协议补 CRLF 结尾
            String txData(req.payload);
            txData += "\r\n";

            // 发送到目标通道
            sendToChannel(tx, rx, txData);

            // 发送指示灯
            digitalWrite(LED_BUILTIN1, HIGH);
            vTaskDelay(pdMS_TO_TICKS(50));
            digitalWrite(LED_BUILTIN1, LOW);

            // 读取响应
            String got = readFromChannel(tx, rx, ttlReadTimeoutMs);

            // 封装并回传给串口任务
            TtlResponse rsp{};
            rsp.channel = req.channel;
            rsp.hasData = got.length() > 0;
            got.toCharArray(rsp.payload, sizeof(rsp.payload));

            (void)xQueueSend(g_rspQ, &rsp, pdMS_TO_TICKS(10));
        }
    }
}

void setup()
{
    // 调试串口（USB CDC/UART0）
    Serial.begin(115200);

    // 初始化指示灯
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(LED_BUILTIN1, OUTPUT);
    pinMode(LED_BUILTIN2, OUTPUT);
    digitalWrite(LED_BUILTIN1, LOW);
    digitalWrite(LED_BUILTIN2, LOW);

    // 创建任务间通信队列
    g_reqQ = xQueueCreate(8, sizeof(TtlRequest));
    g_rspQ = xQueueCreate(8, sizeof(TtlResponse));

    if (!g_reqQ || !g_rspQ) {
        Serial.println("Err: queue create failed");
        while (true) { delay(1000); }
    }

#if defined(CONFIG_FREERTOS_UNICORE)
    // ESP32-C3：单核，仅做任务分拆，不绑核
    xTaskCreate(taskSerialConsole, "SerialConsole", 4096, nullptr, 2, nullptr);
    xTaskCreate(taskSerial1Worker, "Serial1Worker", 4096, nullptr, 3, nullptr);
#else
    // 双核芯片：控制台任务绑核0，TTL任务绑核1
    xTaskCreatePinnedToCore(taskSerialConsole, "SerialConsole", 4096, nullptr, 2, nullptr, 0);
    xTaskCreatePinnedToCore(taskSerial1Worker, "Serial1Worker", 4096, nullptr, 3, nullptr, 1);
#endif
}

void loop()
{
    // 主循环留空，核心逻辑由 FreeRTOS 任务驱动
    vTaskDelay(pdMS_TO_TICKS(1000));
}
