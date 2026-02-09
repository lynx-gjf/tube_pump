#include "ttl_comm.h"
#include "ttl_send.h"
#include "cmd_parser.h"

using namespace std;

static unsigned long throughputWindowStart = 0;
static size_t throughputBytes = 0;

// 使用固定大小队列替代 vector（避免 STL）
// 每条消息最大长度（调整为合适值）
constexpr size_t MAX_QUEUE_MESSAGES = 16;
constexpr size_t MAX_MESSAGE_LEN = 128;
static uint8_t messageQueue[MAX_QUEUE_MESSAGES][MAX_MESSAGE_LEN];
static size_t messageQueueLen[MAX_QUEUE_MESSAGES];
static size_t messageQueueHead = 0;
static size_t messageQueueCount = 0;

/*
 Name:		pumps.ino
 Created:	2026/1/12 17:12:28
 Author:	gwq
*/

#define LED_BUILTIN 13

// TTL 发送引脚 (TX)
#define TTL_TX1 2
#define TTL_TX2 3
#define TTL_TX3 4

// TTL 接收引脚 (RX)
#define TTL_RX1 8
#define TTL_RX2 9
#define TTL_RX3 10

// the setup function runs once when you press reset or power the board
void setup() {
	Serial.begin(9600); // 调试串口
	// 初始化 Serial1 为默认通道 1（可以马上重配置也行）
	configureTTL(TTL_RX1, TTL_TX1, TTL_BAUD);

	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, LOW);
}

// the loop function runs over and over again until power down or reset
void loop() {
	// LED 切换
	digitalWrite(LED_BUILTIN, LOW);
	delay(100);



}
