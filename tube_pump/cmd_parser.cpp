#include "cmd_parser.h"
#include <ctype.h>
#include <stdlib.h>

static inline void trim_token(String &t) {
	// Arduino String 的 trim() 去掉首尾空白（包含 CR/LF）
	t.trim();
}

// 解析单个 token，按规则返回是否成功并填充 outVal
// 规则（常见且兼容性强的策略）:
// - 如果 token 中包含数字（0-9）或带符号，从第一个数字/符号处按 base 0 (支持 0x..) 用 strtol 解析十进制或前缀十六进制
// - 否则如果 token 全部为十六进制字符 (0-9A-Fa-f)，按 base 16 解析
// - 其他情况忽略该 token
static bool parse_token_to_int(const String &token, int &outVal) {
	const char* s = token.c_str();
	size_t n = token.length();
	// 找到第一个数字或符号
	int idx = -1;
	for (size_t i = 0; i < n; ++i) {
		char c = s[i];
		if (c == '+' || c == '-' || (c >= '0' && c <= '9')) {
			idx = (int)i;
			break;
		}
	}
	if (idx >= 0) {
		// 从第一个数字/符号开始解析，允许 "0x" 前缀
		char *endptr = nullptr;
		long v = strtol(s + idx, &endptr, 0); // base 0 支持 0x 前缀
		outVal = (int)v;
		return true;
	}
	// 没有数字，尝试全部为 hex 字符（例如 "FF"）
	bool allHex = (n > 0);
	for (size_t i = 0; i < n && allHex; ++i) {
		if (!isxdigit((unsigned char)s[i])) allHex = false;
	}
	if (allHex) {
		long v = strtol(s, nullptr, 16);
		outVal = (int)v;
		return true;
	}
	return false;
}

size_t parseCommandString(const String& raw, int* outItems, size_t maxItems) {
	if (outItems == nullptr || maxItems == 0) return 0;
	// 复制一份并去除首尾空白
	String s = raw;
	trim_token(s);
	size_t count = 0;
	// 按逗号、空格或制表符分割
	int start = 0;
	int len = s.length();
	for (int i = 0; i <= len; ++i) {
		char c = (i < len) ? s.charAt(i) : ','; // 末尾强制 flush
		if (c == ',' || c == ' ' || c == '\t' || c == '\r' || c == '\n') {
			if (i > start) {
				String token = s.substring(start, i);
				trim_token(token);
				if (token.length() > 0 && count < maxItems) {
					int val;
					if (parse_token_to_int(token, val)) {
						outItems[count++] = val;
					}
				}
			}
			start = i + 1;
		}
	}
	return count;
}