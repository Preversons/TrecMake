#ifndef CHECKERBASIC_H
#define CHECKERBASIC_H

#include <fstream>
#include <string>
#include <vector>
#include <variant>
#include <iostream>
#include <string.h>
#include "macros.h"
using namespace std;

trecmake_begin

namespace checker {

	// 提供多个语法解析相关工具
	namespace utils {

		// 判断是否是合法的标识符字符
		bool isLegalIdentifierChar(char c) {
			return
				(c >= 'a' && c <= 'z') ||
				(c >= 'A' && c <= 'Z') ||
				(c >= '0' && c <= '9') ||
				(c == '_');
		}

		// 判断是否是合法的标识符首字符
		bool isLegalIdentifierStartChar(char c) {
			return
				(c >= 'a' && c <= 'z') ||
				(c >= 'A' && c <= 'Z') ||
				(c == '_');
		}

		// 判断是否是合法的标识符
		bool isLegalIdentifier(const string& s) {
			if (s.empty()) return false;
			if (!isLegalIdentifierStartChar(s[0])) return false;
			for (char c : s) {
				if (!isLegalIdentifierChar(c)) return false;
			}
			return true;
		}

		// 判断是否是合法的字符串常量
		bool isLegalStringLiteral(const string& s) {
			if (s.size() < 2) return false;
			if (s.front() != '"' || s.back() != '"') return false;
			for (int i = 0;i < s.size();i++) {
				char ch = s[i];

				if (ch == '\\') { // 如果将是一个转义符
					// 所有的合法被转义字符
					string legalEscapeChars = "\'\"\\bfnrtv";

					// 下一个（被转义的）字符是否合法
					bool isIllegal = true;

					// 匹配
					for (const char& legalEscapeChar : legalEscapeChars) {
						if (s[i + 1] == legalEscapeChar)isIllegal = false;
					}

					if (isIllegal)return false;

				}

			} // end for

			return true;

		}

		// 判断是否是合法的字符常量
		bool isLegalCharLiteral(const string& s) {
			if (s.size() > 4) return false; // 最多只能有4个字符：'\'', 'x', '\''
			if (s.front() != '\'' || s.back() != '\'') return false;
			if (s[1] == '\\') {
				// 处理转义字符
				string legalEscapeChars = "\'\"\\bfnrtv";
				bool isIllegal = true;
				for (const char& legalEscapeChar : legalEscapeChars) {
					if (s[2] == legalEscapeChar) isIllegal = false;
				}
				if (isIllegal) return false;
			}
			else {
				// 处理普通字符
				if (s[1] == '\'') return false;
			}
			return true;
		}

		// 判断是否是合法的数字常量
		bool isLegalNumberLiteral(const string& s) {
			if (s.empty()) return false;

			// 这整个数字常量的状态，即非十进制或浮点数
			enum { hex, dec, oct, bin, _float }status = dec;

			switch (s.back()) {
			case 'h': status = hex; break;
			case 'o': status = oct; break;
			case 'b': status = bin; break;
			case 'f': status = _float; break;

			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9': break;

			default: return false; // 不合法的结尾字符
			}

			for (int i = 0;i < (status == dec ? s.size() : s.size() - 1);i++) {
				if (s[i] == '.')continue;
				if (status == hex) {
					if (!((s[i] >= '0' && s[i] <= '9') || (s[i] >= 'a' && s[i] <= 'f') || (s[i] >= 'A' && s[i] <= 'F'))) return false;
				}
				else if (status == dec) {
					if (!(s[i] >= '0' && s[i] <= '9')) return false;
				}
				else if (status == oct) {
					if (!(s[i] >= '0' && s[i] <= '7')) return false;
				}
				else if (status == bin) {
					if (!(s[i] == '0' || s[i] == '1')) return false;
				}
				else if (status == _float) {
					if (!(s[i] >= '0' && s[i] <= '9')) return false;
				}

			}

			return true;
		}
	}


}

trecmake_end

#include "checkerScanner.h"


#endif // !CHECKERBASIC_H
