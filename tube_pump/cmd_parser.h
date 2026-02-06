#pragma once
#include <Arduino.h>
#include <stddef.h>
#include <stdint.h>

// 将形如 "P0,G1,255" 或 "FF" 等的命令字符串解析为整数序列。
// outItems: 调用者提供的缓冲区（整数数组）
// maxItems: 缓冲区最大元素数
// 返回值: 写入到 outItems 的元素数量
size_t parseCommandString(const String& raw, int* outItems, size_t maxItems);