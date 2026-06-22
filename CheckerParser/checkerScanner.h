#ifndef SCANNER_H
#define SCANNER_H

#include <fstream>
#include <variant>
#include <iostream>
#include <string>
#include <vector>
#include "macros.h"
#include "checkerBasic.h"
#include "checkerExceptions.h"
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
		bool isLegalIdentifier(const char* s) {
			return isLegalIdentifier(string(s));
		}

	}

	// 扫描器
	class Scanner {
	private:

		// 输入的文件
		variant<string, ifstream> _MyFile;

		// 将参数content字符串以';'格式化为一个vector
		vector<string> formatting(string _content) {
			string content = _content; // 复制输入字符串，避免修改原字符串
			bool isInStringLiteral = false; // 是否在字符串字面量中

			// 删除字符串中的空字符
			for (int i = 0;i < content.size();i++) {
				if (content[i] == '#') { // 如果进入注释
					while (content[i] != '\n' && i < content.size()) {
						content.erase(i, 1); // 删除注释内容
					}
					i--;
					continue;
				}
				if (content[i] == '"') {
					isInStringLiteral = !isInStringLiteral; // 切换字符串字面量状态
				}
				if (isInStringLiteral)continue; // 如果在字符串字面量中，跳过空字符检查
				if (isspace(content[i])) { // 如果是空字符（如' '、'\t'等）
					content.erase(i, 1); // 删除这个字符
					i--; // 防止跳过
					continue; // 下一个循环
				}

			}

			// 删除开头和结尾的空字符
			if (!content.empty())while (isspace(content[0]) && !content.empty())content.erase(0, 1); // 删除开头的空字符
			if (!content.empty())while (isspace(content[content.size() - 1]) && !content.empty())content.erase(content.size() - 1, 1); // 删除结尾的空字符


			cout << "===================================" << endl;
			cout << content << endl;
			cout << "===================================" << endl;


			// 用字符';'分割
			// 同时要考虑到字符串字面量中可能包含';'，所以需要一个状态变量来判断当前是否在字符串字面量中
			vector<string> result = { "" }; // 存储分割结果
			isInStringLiteral = false;

			for (int i = 0;i < content.size();i++) {
				if (content[i] == '"') { // 如果遇到双引号，切换字符串字面量状态
					result.back().push_back('"'); // 先加入这个双引号
					isInStringLiteral = !isInStringLiteral; // 切换状态
					continue; // 下一个循环
				}
				if (content[i] == '\\') { // 如果遇到转义字符，跳过下一个字符（无论是什么字符都跳过）
					result.back().push_back(content[i]); // 先加入当前字符（转义字符）
					result.back().push_back(content[i + 1]); // 再加入下一个字符（被转义的字符）
					i++; // 跳过下一个字符
					continue; // 下一个循环
				}
				if (isInStringLiteral) { // 如果在字符串字面量中，先加入当前字符再跳过
					result.back().push_back(content[i]);
					continue;
				}

				if (content[i] == ';') {
					result.push_back(""); // 添加一个新的分割结果
					continue;
				}

				// 不是任何的特殊情况
				// 则将当前字符添加到当前分割结果的末尾
				result.back().push_back(content[i]);
			}

			for (int i = result.size() - 1;i >= 0;i--) {
				bool isEmpty = true;

				for (auto c : result[i]) {
					cout << "Checking character: \'" << c << "\', ASCII code: " << int(c) << endl;
					if (!isspace(c)) {
						isEmpty = false;
						break;
					}
				}
				if (result.size() == 0)isEmpty = true;

				cout << "isEmpty for Line: " << i << " is " << (isEmpty ? "true" : "false") << endl;

				if (isEmpty) {
					result.erase(result.begin() + i); // 删除空字符串
					cout << "Erased the index: " << i << endl;
				}
			}

			return result;

		} // end formatting

	public:

		Scanner(const string& filePath) : _MyFile(filePath) {}         // 字符串路径构造函数
		Scanner(const char* filePath) : _MyFile(string(filePath)) {}   // C风格字符串路径构造函数
		Scanner(ifstream&& file) : _MyFile(move(file)) {}


		// 分词结果，包含错误信息与分词结果
		class TokenizeResult {
		public:

			// 错误信息，以指针形式存储
			vector<ScannerProcessingException*> errors;

			// 结果
			vector<string> results;


			TokenizeResult(vector<string> results) : results(results) {}
			TokenizeResult(vector<ScannerProcessingException*> errors) : errors(errors) {}
			TokenizeResult(vector<string> results, vector<ScannerProcessingException*> errors)
				: results(results), errors(errors) {}

			// 是否报错
			bool isErrorReport() { return errors.size(); }

			// 访问分词结果的对应下标
			string operator[](int index) { return results[index]; }

		};


		// 将一个字符串转换为多个token，以vector形式存储
		// @param line 需要处理的行，其中不能有换行符
		// @param _LineNumber 当前的行号
		TokenizeResult tokenize(const string& line, int _LineNumber) {

			// 存储分割结果
			vector<string> tokens;

			// 存储分割过程中产生的异常
			vector<ScannerProcessingException*> errors;

			// 当前正在构建的token
			string currentToken;

			for (int i = 0;i < line.size();i++) {
				if (isspace(line[i])) { // 如果遇到空白字符，结束当前token并开始新的token
					if (!currentToken.empty()) {
						tokens.push_back(currentToken); // 将当前token添加到结果中
						currentToken.clear(); // 清空当前token以开始新的token
					}
					continue; // 下一个循环
				}

				// 不是空白字符，继续构建当前token
				switch (line[i]) {
				case '-':
					if (line[i + 1] == '>') { // 如果遇到'->'，将其作为一个整体的token
						currentToken = "->";
					}
					break; // end case '-'
					
				case '[':
					currentToken = "["; // 如果遇到'['，将其作为一个token
					break; // end case '['

				case ']':
					currentToken = "]"; // 如果遇到']'，将其作为一个token
					break; // end case ']'

				case '"':
					currentToken.push_back('"'); // 如果遇到'"'，将其加入当前token
					i++; // 跳过这个双引号

					// 不断读取下一个字符，直到遇到下一个未被转义的双引号为止
					for (;line[i] != '"';i++) {
						// 如果遇到转义字符，先加入转义字符和被转义的字符，再跳过被转义的字符
						if (line[i] == '\\') {
							currentToken.push_back(line[i]); // 先加入当前字符（转义字符）
							currentToken.push_back(line[i + 1]); // 再加入下一个字符（被转义的字符）
							i++; // 跳过下一个字符
						}

						// 否则正常加入当前字符
						currentToken.push_back(line[i]);
					} // end for

					break; // end case '"'

				default:
					if (currentToken.empty()) { // 当前是一个新的token，并且是标识符的开始
						if (utils::isLegalIdentifierStartChar(line[i])) { // 如果当前字符是合法的标识符首字符，开始构建一个新的token
							currentToken.push_back(line[i]);
						}
						else { // 否则报错
							errors.push_back(nullptr); // 加入一个新的错误指针
							errors.back() = new ScannerIdentifierException(
								_LineNumber, // line
								i,           // row
								string({ line[i] })
							);
						}
					}
					else { // 当前token不是空的，即是一个标识符
						if (utils::isLegalIdentifierChar(line[i])) { // 判断是不是合法的字符，如果是
							currentToken.push_back(line[i]); // 加入当前构建的token
						}
						else { // 不合法的字符，报错
							errors.push_back(nullptr); // 加入一个新的错误指针
							errors.back() = new ScannerIdentifierException(
								_LineNumber, // line
								i,           // row
								string(currentToken + string({ line[i],'\0' }))
							);
						}
					}

					break; // end default
				} // end switch
			} // end for

			// 最后检查是否有未添加的token
			if (!currentToken.empty()) {
				tokens.push_back(currentToken);
			}
			
			return TokenizeResult(tokens, errors);

		}

	}; // end class Scanner


} // end namespace checker

trecmake_end


#endif // !SCANNER_H
