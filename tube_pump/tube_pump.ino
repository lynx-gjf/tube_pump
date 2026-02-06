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

#define LED_BUILTIN1 12
#define LED_BUILTIN2 13

// TTL 发送引脚 (TX)
#define TTL_TX1 1
#define TTL_TX2 18
#define TTL_TX3 19

// TTL 接收引脚 (RX)
#define TTL_RX1 2
#define TTL_RX2 10
#define TTL_RX3 6

// OUTPUT 引脚
#define OUTPUT1 6
#define OUTPUT2 5
#define OUTPUT3 4

// 模拟输入引脚
int analogInPin = A1;
int sensorValue = 0; // 存储模拟输入的值
float voltage = 0;

// the setup function runs once when you press reset or power the board
void setup() {
	Serial.begin(9600); // 调试串口
	// 初始化 Serial1 为默认通道 1（可以马上重配置也行）
	configureTTL(TTL_RX1, TTL_TX1, TTL_BAUD);

	pinMode(LED_BUILTIN1, OUTPUT);
	pinMode(LED_BUILTIN2, OUTPUT);
	digitalWrite(LED_BUILTIN1, LOW);
	digitalWrite(LED_BUILTIN2, LOW);
	pinMode(OUTPUT1, OUTPUT);
	pinMode(OUTPUT2, OUTPUT);
	pinMode(OUTPUT3, OUTPUT);
	analogWrite(OUTPUT1, 20);
}

// the loop function runs over and over again until power down or reset
void loop() {
	analogWrite(OUTPUT1, 128);
	analogWrite(OUTPUT1, 128);
	analogWrite(OUTPUT1, 128);
	delay(2000);
	analogWrite(OUTPUT1, 1);
	analogWrite(OUTPUT1, 1);
	analogWrite(OUTPUT1, 1);
	// LED 切换
	digitalWrite(LED_BUILTIN2, LOW);

	// 从调试串口读取并获取字节数组（已经打印过一次）
	String dbg = "";
	String zero = "";
	while (dbg.compareTo(zero) == 0)
	{
		char dbgBuf[512]; // 预留足够空间
		int dbgLen = read_string_serial(dbgBuf, sizeof(dbgBuf) - 1, 500); // 传入缓冲区和最大长度
		dbg = String(dbgBuf); // 转换为 String 类型
		sensorValue = analogRead(analogInPin); // 读取模拟输入的值
		voltage = sensorValue * (5.0 / 1023.0); // 将模拟输入的值转换为电压值
		Serial.print("voltage: ");
		Serial.println(voltage);
		delay(1000);
	} 
	digitalWrite(LED_BUILTIN2, HIGH);
	Serial.print(dbg + "\n");
	delay(100);


	// 使用固定大小 C 数组接收解析结果
	int parsed[8];
	size_t parsedCount = parseCommandString(dbg, parsed, sizeof(parsed) / sizeof(parsed[0]));

	for (size_t i = 0; i < parsedCount; ++i) {
		Serial.println(parsed[i]);
	}

	// 安全检查后根据协议执行
	if (parsedCount > 2)
	{
		if (parsed[1] == 1)
		{
			analogWrite(OUTPUT1, parsed[2]);
		}
		else if (parsed[1] == 2)
		{
			analogWrite(OUTPUT2, parsed[2]);
		}
		else if (parsed[1] == 3)
		{
			analogWrite(OUTPUT3, parsed[2]);
		}
	}
	digitalWrite(LED_BUILTIN2, LOW);
	analogWrite(OUTPUT1, 20);
	delay(10000);
	digitalWrite(LED_BUILTIN2, HIGH);
	analogWrite(OUTPUT1, 50);
	delay(10000);
	digitalWrite(LED_BUILTIN2, LOW);
	analogWrite(OUTPUT1, 80);
	delay(10000);
	digitalWrite(LED_BUILTIN2, HIGH);
	// 其余 TTL 读取逻辑和队列操作（如需我可以把 vector 风格的代码也改为循环队列实现）
}
