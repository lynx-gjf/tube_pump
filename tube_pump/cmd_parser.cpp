#include <Arduino.h>
#include <vector>

using std::vector;

// 将形如 "Pa, Qb, FF" 的字符串解析为 int 数组。
// 规则：
// - 以逗号分隔，允许空格。
// - 如果 token 是纯十六进制字符（例如 "FF" 或 "0xFF"） -> 按 16 进制整体解析并加入一个整数（例如 255）。
// - 如果 token 长度为 2，且形式为 字母 + 十六进制位（例如 "Pa"、"Qb"） -> 仅将第二个字符的十六进制数值加入结果（"Pa" -> 10）。
// - 如果 token 是纯十进制数字 -> 按 10 进制解析加入一个整数。
// - 其它情况 -> 将 token 每个字符的 ASCII 值依次加入结果。
static inline bool isHexChar(char c) {
	return (c >= '0' && c <= '9') ||
	       (c >= 'A' && c <= 'F') ||
	       (c >= 'a' && c <= 'f');
}

static inline int hexCharValue(char c) {
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
	if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
	return 0;
}

static inline bool isAllHex(const String& s) {
	int n = s.length();
	if (n == 0) return false;
	for (int i = 0; i < n; ++i) if (!isHexChar(s.charAt(i))) return false;
	return true;
}

static inline bool isDecimalNumber(const String& s) {
	if (s.length() == 0) return false;
	int i = 0;
	if (s.charAt(0) == '+' || s.charAt(0) == '-') {
		if (s.length() == 1) return false;
		i = 1;
	}
	for (; i < s.length(); ++i) {
		char c = s.charAt(i);
		if (c < '0' || c > '9') return false;
	}
	return true;
}

vector<int> parseCommandString(const String& raw) {
	vector<int> out;
	String s = raw;
	s.trim();
	if (s.length() == 0) return out;

	// 方便判断，去掉空格（但保留其它字符）
	s.replace(" ", "");

	int pos = 0;
	while (pos < s.length()) {
		int comma = s.indexOf(',', pos);
		String tok = (comma == -1) ? s.substring(pos) : s.substring(pos, comma);
		if (comma == -1) pos = s.length();
		else pos = comma + 1;
		tok.trim();
		if (tok.length() == 0) continue;

		// 处理以 0x/0X 前缀的 hex
		if (tok.length() > 2 && (tok.startsWith("0x") || tok.startsWith("0X"))) {
			long val = strtol(tok.c_str(), nullptr, 16);
			out.push_back((int)val);
			continue;
		}

		// 判断是否全为十六进制字符（当作整体 hex 数）
		if (isAllHex(tok)) {
			long val = strtol(tok.c_str(), nullptr, 16);
			out.push_back((int)val);
			continue;
		}

		// 形如 Letter + HexDigit (长度 2)，仅将第二个字符的 hex 值加入结果
		if (tok.length() == 2) {
			char a = tok.charAt(0);
			char b = tok.charAt(1);
			bool aIsAlpha = ((a >= 'A' && a <= 'Z') || (a >= 'a' && a <= 'z'));
			if (aIsAlpha && isHexChar(b)) {
				out.push_back(hexCharValue(b));
				continue;
			}
		}

		// 纯十进制数字 -> 解析为一个整数
		if (isDecimalNumber(tok)) {
			long val = strtol(tok.c_str(), nullptr, 10);
			out.push_back((int)val);
			continue;
		}

		// 其它情况：逐字符加入 ASCII 值
		for (int i = 0; i < tok.length(); ++i) out.push_back((int)tok.charAt(i));
	}

	return out;
}

/* 示例：
#include "cmd_parser.h"
void example() {
	String in = "Pa, Qb, FF";
	auto v = parseCommandString(in); // v => [10,11,255]
	for (int x : v) {
		Serial.println(x);
	}
}
*/