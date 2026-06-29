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

	

	// 扫描器
	class Scanner {
	private:

		// [内部] 分词结果，包含错误信息与分词结果
		class _TokenizeResult {
		public:

			// 错误信息，以指针形式存储
			vector<ScannerExceptions::ScannerProcessingException*> errors;

			// 结果
			vector<string> results;


			_TokenizeResult(vector<string> results) : results(results) {}
			_TokenizeResult(vector<ScannerExceptions::ScannerProcessingException*> errors) : errors(errors) {}
			_TokenizeResult(vector<string> results, vector<ScannerExceptions::ScannerProcessingException*> errors)
				: results(results), errors(errors) {
			}

			// 是否报错
			bool isErrorReport() { return errors.size(); }

			// 访问分词结果的对应下标
			string& operator[](int index) { return results[index]; }

		};


		// 文件路径
		string _MyFilePath;

		// 输入的文件流对象
		ifstream _MyFile;


		// [内部] 将参数content字符串以';'格式化为一个vector
		vector<string> __formatting(string _content) {
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



			// 用字符';'分割
			// 同时要考虑到字符串字面量中可能包含';'，所以需要一个状态变量来判断当前是否在字符串字面量中
			;

			// 存储分割结果
			vector<string> result = { "" };
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
					if (!isspace(c)) {
						isEmpty = false;
						break;
					}
				}
				if (result.size() == 0)isEmpty = true;

				if (isEmpty) {
					result.erase(result.begin() + i); // 删除空字符串
				}
			}

			return result;

		}



		// [内部] 将一个字符串转换为多个token，以vector形式存储
		// @param line 需要处理的行，其中不能有换行符
		// @param _LineNumber 当前的行
		_TokenizeResult __tokenize(const string& line, int _LineNumber) {

			// 存储分割结果
			vector<string> tokens;

			// 存储分割过程中产生的异常
			vector<ScannerExceptions::ScannerProcessingException*> errors;

			// 当前正在构建的token
			string currentToken;



			// 遍历整个字符串
			for (int i = 0;i < line.size();i++) {
				
				if (isspace(line[i])) { // 如果遇到空白字符，先将下标移到这段空白的末尾，再结束当前token并开始新的token

					// 跳过这段空白
					while (isspace(line[i])) {
						i++;
					}

					cout << "Skipped The Index of: " << i << endl;

					// 这个时候i在一个非空白字符上
					// 为了防止跳过，应该再往前一个字符
					i--;

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
						tokens.push_back(currentToken);
						tokens.push_back(""); // 创建一个新的token
						currentToken = "";
						i++;
					}
					else {
						currentToken.push_back(line[i]);
					}
					break; // end case '-'

				case '[':// 如果遇到'['，将其作为一个token
					tokens.push_back(currentToken);
					tokens.push_back("[");
					currentToken = "";
					break; // end case '['

				case ']': // 如果遇到']'，将其作为一个token
					tokens.push_back(currentToken);
					tokens.push_back("]");
					currentToken = "";
					break; // end case ']'

				case '"':// 如果遇到'"'，则一次性将整个字符串字面值常量作为一个token并加入token列表
					tokens.push_back(currentToken); // 保存当前已完成的token
					currentToken = ""; // 清空

					currentToken.push_back('"'); // 将其加入当前token
					i++; // 跳过这个双引号

					// 不断读取下一个字符，直到遇到下一个未被转义的双引号为止
					for (;;i++) {

						// 如果到达了另一个双引号，退出
						if (line[i] == '"')break;

						// 如果遇到转义字符，先加入转义字符和被转义的字符，再跳过被转义的字符
						if (line[i] == '\\') {
							currentToken.push_back(line[i]); // 先加入当前字符（转义字符）
							currentToken.push_back(line[i + 1]); // 再加入下一个字符（被转义的字符）
							i++; // 跳过下一个字符
							continue;
						}

						// 否则正常加入当前字符
						currentToken.push_back(line[i]);

					} // end for
					currentToken.push_back(line[i]); // 再退出循环的时候这个字符是'"'，我们还要把他加回来

					tokens.push_back(currentToken); // 保存当前已完成的token
					currentToken = ""; // 清空


					break; // end case '"'

				case '(':
					// 如果当前token还有余留，加入列表并清空
					if (!currentToken.empty()) {
						tokens.push_back(currentToken);
						currentToken = "";
					}

					// 读取到另一个与它相匹配的 ')'
					{
						// 进入的（小）括号相对层级
						int parentheseLeval = 0;

						// 找到与它对应的那个右括号
						for (;i < line.size();i++) {

							// 如果已经到了末尾
							if (line[i] == '\0') {
								// 报错并直接退出循环，防止出现下标越界
								errors.push_back(NULL);
								errors.back() = new ScannerExceptions::ScannerParenthesesException(_LineNumber, i + 1, '(');
								currentToken = "";
								break;
							}

							currentToken.push_back(line[i]);

							// 如果又遇到了一个左小括号
							if (line[i] == '(') {
								// 增加括号层级
								parentheseLeval++;
							}

							// 如果遇到了一个右小括号
							if (line[i] == ')') {
								parentheseLeval--;
								if (parentheseLeval == 0) { // 这个case遇到的小括号结束了
									break; // 直接退出循环
								}
							}


						} // end for

					} // end READ

					tokens.push_back(currentToken);

					currentToken = "";


					break; // end case '('

				case ')':
					errors.push_back(NULL);
					errors.back() = new ScannerExceptions::ScannerParenthesesException(_LineNumber, i + 1, ')');
					break; // end case ')'

				case ',':
					tokens.push_back(currentToken);
					currentToken = ",";
					tokens.push_back(currentToken);
					currentToken = "";
					break; // end case ','

				default:
					if (currentToken.empty()) { // 当前是一个新的token，并且是标识符的开始
						if (utils::isLegalIdentifierStartChar(line[i])) { // 如果当前字符是合法的标识符首字符，开始构建一个新的token
							currentToken.push_back(line[i]);
						}
						else { // 否则报错
							errors.push_back(nullptr); // 加入一个新的错误指针
							errors.back() = new ScannerExceptions::ScannerIdentifierException(
								_LineNumber, // line
								i,           // row
								string({ line[i] })
							);
						}
					}
					else { // 当前token不是空的，即是一个标识符，而当前字符即是标识符的一部分
						if (utils::isLegalIdentifierChar(line[i])) { // 判断是不是合法的字符，如果是
							currentToken.push_back(line[i]); // 加入当前构建的token
						}
						else { // 不合法的字符，报错
							errors.push_back(nullptr); // 加入一个新的错误指针
							errors.back() = new ScannerExceptions::ScannerIdentifierException(
								_LineNumber, // line
								i,           // row
								string(currentToken + string({ line[i] }))
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

			// 清除所有的无用的空token
			for (int i = 0;i < tokens.size();i++) {

				// 是否是空token
				bool isEmptyToken = true;

				for (const char& ch : tokens[i]) {
					if (!isspace(ch)) {
						isEmptyToken = false;
						break;
					}
				}

				// 如果是空token
				if (isEmptyToken) {
					// 存储当前（空）token的位置
					auto iter = tokens.begin() + i;
					tokens.erase(iter);
					i--;
				}
			}

			// 当前token的开头的字符在整个字符串中的位置
			int currentTokenStartIndex = 0;

			// 检查是否出现不合法的token
			for (int i = 0;i < tokens.size();currentTokenStartIndex += tokens[i].size(), i++) {

				// 可能是字符串字面值常量
				if (tokens[i][0] == '"') {
					if (tokens[i].back() != '"') { // 最后一个字符不是'"'
						errors.push_back(NULL);
						errors.back() = new ScannerExceptions::ScannerStringLiteralException(tokens[i]);
					}

					for (int j = 0;j < tokens[i].size();j++) {
						char ch = tokens[i][j];

						if (ch == '\\') { // 如果将是一个转义符
							// 所有的合法被转义字符
							string legalEscapeChars = "\'\"\\bfnrtv";

							// 下一个（被转义的）字符是否合法
							bool isIllegal = true;

							// 匹配
							for (const char& legalEscapeChar : legalEscapeChars) {
								if (tokens[i][j + 1] == legalEscapeChar)isIllegal = false;
							}
							
							// 如果不合法则报错
							if (isIllegal) {
								errors.push_back(NULL);
								errors.back() = new ScannerExceptions::ScannerStringLiteralException(tokens[i]);
								continue;
							}

						}

					} // end for

				} // end if IT'S STRING LITERAL

				// 可能是特殊符号
				else if (tokens[i]=="->") {}
				else if (tokens[i]=="[") {}
				else if (tokens[i] == "]") {}
				else if (tokens[i] == ",") {}
				
				// 可能是由括号包裹的
				else if (tokens[i][0] == '(') {

					// 括号进入的相对层级
					int parentheseLeval = 0;

					// 括号层级是否合法
					bool parentheseIsLegal = false;

					// 判断括号层级是否合法
					for (int j = 0;j < tokens[i].size();j++) {
						if (tokens[i][j] == '(')parentheseLeval++;
						if (tokens[i][j] == ')') {
							parentheseLeval--;
							if (parentheseLeval == 0) {
								parentheseIsLegal = true;
								break;
							}
						}
					}


					// 如果括号层级不合法，一般是函数自身问题
					if (!parentheseIsLegal) {
						cout << "\033[91mparentheseIllegal: " << tokens[i] << "\033[0m" << endl;
						errors.push_back(NULL);
						errors.back() = new ScannerExceptions::ScannerTokenIllegalException(tokens[i]);
					}

					// 并非以')'结尾，应该也是函数自身问题
					if (tokens[i].back() != ')') {
						cout << "\033[91menderIllegal.\033[0m" << endl;
						errors.push_back(NULL);
						errors.back() = new ScannerExceptions::ScannerTokenIllegalException(tokens[i]);
					}



					// 如果括号中间不是合法的标识符
					if (!utils::isLegalIdentifier(tokens[i].substr(1, tokens[i].size() - 2))) {
						errors.push_back(NULL);
						errors.back() = new ScannerExceptions::ScannerIdentifierException(_LineNumber, currentTokenStartIndex, tokens[i].substr(1, tokens[i].size() - 2));
					}

				}

				// 否则就是标识符
				else {
					// 如果不是合法标识符
					if (!utils::isLegalIdentifier(tokens[i])) {
						errors.push_back(NULL);
						errors.back() = new ScannerExceptions::ScannerIdentifierException(
							_LineNumber,
							currentTokenStartIndex + 1,
							tokens[i]
						);
					}
				}

			} // end CHECK ILLEGAL TOKENS

			return _TokenizeResult(tokens, errors);

		} // end function __tokenize

	public:

		Scanner(const string& filePath) : _MyFilePath(filePath),         _MyFile(filePath.c_str()) {} // 字符串路径构造函数
		Scanner(const char* filePath)   : _MyFilePath(filePath),         _MyFile(filePath) {} // C风格字符串路径构造函数
		Scanner(ifstream&& file)        : _MyFilePath("<Unknown File>"), _MyFile(move(file)) {} // 使用文件流构建，无法知晓文件名，设为未知项

		class TokenizeResult {
		public:

			// 错误信息，以指针形式存储
			vector<ScannerExceptions::ScannerProcessingException*> errors;

			// 结果，以二维数组存储，一个行为一句，一个行内含有多个token
			vector<vector<string>> results;


			TokenizeResult(vector<vector<string>> results) : results(results) {}
			TokenizeResult(vector<ScannerExceptions::ScannerProcessingException*> errors) : errors(errors) {}
			TokenizeResult(vector<vector<string>> results, vector<ScannerExceptions::ScannerProcessingException*> errors)
				: results(results), errors(errors) {
			}

			// 是否报错
			bool isErrorReport() { return errors.size(); }

			// 访问分词结果的对应下标
			vector<string>& operator[](int index) { return results[index]; }
		};

		// 分词函数，外部接口，实际调用内部__tokenize函数
		TokenizeResult tokenize() {

			// 所有的分词结果
			vector<vector<string>> tokens;
			
			// 所有的异常信息
			vector<ScannerExceptions::ScannerProcessingException*> errors;

			// 每一个行，即一条配置语句
			vector<string> lines;

			// 将文件的信息读取到lines中
			{
				// 连续的文件信息
				string fileContents = "";

				// 刷新文件流
				if (_MyFile.is_open()) {
					_MyFile.close();
				}
				_MyFile.open(_MyFilePath.c_str());

				// 检查是否能够读取文件内容
				if (!_MyFile.is_open()) {
					errors.push_back(NULL);
					errors.back() = new ScannerExceptions::ScannerFileCannotOpenException(_MyFilePath);
				}


				// 读取到fileContents中
				string s = "";
				while (!_MyFile.eof()) {
					getline(_MyFile, s);
					fileContents += s;
				}

				lines = __formatting(fileContents);

			} // end READ TO lines

			

			for (int i = 0;i < lines.size();i++) {
				_TokenizeResult tokenizeResults = __tokenize(lines[i], i + 1);

				// 将其拼接在错误列表和结果token列表中
				errors.insert(errors.end(), tokenizeResults.errors.begin(), tokenizeResults.errors.end());
				tokens.push_back(tokenizeResults.results);


			}


			return TokenizeResult(tokens, errors);


		} // end function tokenize
		


		






	}; // end class Scanner


} // end namespace checker

trecmake_end


#endif // !SCANNER_H
