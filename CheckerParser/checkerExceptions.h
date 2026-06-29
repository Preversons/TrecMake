#ifndef CHECKEREXCEPTIONS_H
#define CHECKEREXCEPTIONS_H

#include <exception>
#include <string>
#include <sstream>
#include "macros.h"
using namespace std;

trecmake_begin


namespace checker {

	// 一些扫描器可能产生的异常及其辅助工具
	namespace ScannerExceptions {

		// 扫描器相关的异常类
		class ScannerProcessingException : public exception {
		protected:

			// 错误信息
			string message;

			// 缓存后的信息
			mutable string cachedMessage;

		public:
			ScannerProcessingException(const char* message) : message(message) {}
			ScannerProcessingException(const string& message) : message(message) {}
			ScannerProcessingException(const ScannerProcessingException& willCopy)
				: message(willCopy.what()) {
			}
			ScannerProcessingException() : message("\0") {}

			virtual ~ScannerProcessingException() = default;


			const char* what() const noexcept override {
				if (message.empty()) {
					return "";
				}

				// 处理消息（添加 \0）
				if (message.back() != '\0') {
					cachedMessage = message + '\0';
					return cachedMessage.c_str();
				}
				else {
					return message.c_str();
				}
			}

		};





		// 存储一个东西的坐标，即它的行号、列号，至于其类型将在子类中指明
		class Position {
		public:

			// 行号
			int line;

			// 列号
			int row;

			Position(const int& line, const int& row) :line(line), row(row) {}
			Position() :line(0), row(0) {}

		};

		// 存储一个非法标识符的 行号、列号、值
		class IdentifierInfo : public Position {
		public:

			// 标识符的值
			string value;
			
			IdentifierInfo(int line, int row, const string& value) : Position(line, row), value(value) {}
			IdentifierInfo() : Position(), value("") {}

			// 赋值运算符（拷贝函数）
			IdentifierInfo& operator=(const IdentifierInfo& iden) {
				this->line = iden.line;
				this->row = iden.row;
				this->value = iden.value;
			}
		};

		// 存储一个非法括号的 行号、列号、值
		class ParentheseInfo : public Position {
		public:

			// 非法括号的值
			char value;


			ParentheseInfo(int line, int row, char value) : Position(line, row), value(value) {}
			ParentheseInfo() : Position(), value(0) {}

			// 赋值运算符（拷贝函数）
			ParentheseInfo& operator=(const ParentheseInfo& par) {
				this->line = par.line;
				this->row = par.row;
				this->value = par.value;
			}

			// 获取与这个括号成对的符号
			char getAnother() {
				// 左括号的情况
				if (value == '(')return ')';
				else if (value == '[')return ']';
				else if (value == '{')return '}';

				// 右括号
				else if (value == ')')return '(';
				else if (value == ']')return '[';
				else if (value == '}')return '{';
				else return ' ';
			}

		};




		// 标识符错误
		class ScannerIdentifierException : public ScannerProcessingException {
		protected:

			// 这个非法标识符的信息
			IdentifierInfo ErrorIdentifier;

			// 通过标识符信息生成报错信息
			string createMessage(int line, int row, const string& value)const {
				ostringstream result;
				result << "\033[91mScannerIdentifierException:\033[0m\n";
				result << "\tAt line " << line << ", row " << row << ":\n";
				result << "\tThe identifier: \"" << value << "\" is illegal.";
				return result.str();
			}
			string createMessage(const IdentifierInfo& Iden)const {
				ostringstream result;
				result << "\033[91mScannerIdentifierException:\033[0m\n";
				result << "\tAt line " << Iden.line << ", row " << Iden.row << ":\n";
				result << "\tThe identifier: \"" << Iden.value << "\" is illegal.";
				return result.str();
			}

		public:

			ScannerIdentifierException(int line, int row, const string& value)
				: ScannerProcessingException(createMessage(IdentifierInfo(line, row, value)).c_str()) {
				ErrorIdentifier.line = line;
				ErrorIdentifier.row = row;
				ErrorIdentifier.value = value;
			}
			ScannerIdentifierException(const IdentifierInfo& iden)
				: ScannerProcessingException(createMessage(iden).c_str()) {
				ErrorIdentifier = iden;
			}

			// 使用父类的读取函数
			using ScannerProcessingException::what;

		};

		// 文件无法打开错误
		class ScannerFileCannotOpenException : public ScannerProcessingException {
		public:
			// @param filePath 提供的文件路径，作为异常信息的一部分
			ScannerFileCannotOpenException(const string& filePath) {
				string message = "\033[91mScannerFileCannotOpenException:\033[0m\n";
				message += "\tThe File: ";
				message += filePath;
				message += " cannot open.";

				this->message = message.c_str();
			}
			ScannerFileCannotOpenException(const char* filePath) {
				string message = "\033[91mScannerFileCannotOpenException:\033[0m\n";
				message += "\tThe File: ";
				message += string(filePath);
				message += " cannot open.";

				this->message = message.c_str();
			}
			~ScannerFileCannotOpenException() = default;

			using ScannerProcessingException::what;
		};

		// 字面值常量错误
		class ScannerStringLiteralException : public ScannerProcessingException {
		public:
			// @param errorSLToken 错误的字符串字面值常量的token
			ScannerStringLiteralException(const string& errorSLToken) {
				string message = "\033[91mScannerStringLiteralException:\033[0m\n";
				message += "\tThe String Literal: ";
				message += errorSLToken;
				message += " is illegal.";

				this->message = message.c_str();
			}
			~ScannerStringLiteralException() = default;
		};

		// 括号不匹配错误
		class ScannerParenthesesException : public ScannerProcessingException {
		private:
			// 错误（多余）的括号
			ParentheseInfo ErrorParenthese;

		public:
			
			// @param value 多余的括号
			ScannerParenthesesException(int line, int row, char value) : ErrorParenthese(line, row, value) {
				ostringstream _message;
				_message << "\033[91mScannerParentheseException:\033[0m\n";
				_message << "\tAt line " << line << ", row " << row << ":\n";
				_message << "\tThe parenthese need a '" << ErrorParenthese.getAnother() << "'.";
				message = _message.str().c_str();
			}
			ScannerParenthesesException() : ErrorParenthese(0, 0, ' ') {}

			

		};

		// token不合法错误
		class ScannerTokenIllegalException : public ScannerProcessingException {
		private:
			string illegalToken = "<Unknown>";

		public:
			ScannerTokenIllegalException(const string& illegalToken) :illegalToken(illegalToken) {
				ostringstream _message;
				_message << "\033[90m[Internal] \033[91mScannerTokenIllegalException:\033[0m\n";
				_message << "\tThe token: \"" << illegalToken << "\" is illegal.";
				this->message = _message.str().c_str();
			}
		};
	
	}
}

trecmake_end


#endif // !CHECKEREXCEPTIONS_H
