#pragma once
#include <Arduino.h>
#include <vector>

// 将形如 "Pa, Qb, FF" 的字符串解析为 int 数组。
// 返回值：std::vector<int>，按项目约定解析规则填充整数。
// 函数实现位于 cmd_parser.cpp
std::vector<int> parseCommandString(const String& raw);