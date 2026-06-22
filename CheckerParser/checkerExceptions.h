#ifndef CHECKEREXCEPTIONS_H
#define CHECKEREXCEPTIONS_H

#include <exception>
#include <string>
#include <sstream>
#include "macros.h"
using namespace std;

trecmake_begin


namespace checker {

	// 扫描器相关的异常类
	class ScannerProcessingException : public exception {
	protected:
		const char* message; // 错误信息

	public:
		ScannerProcessingException(const char* message) : message(message) {}
		ScannerProcessingException(const string& message) : message(message.c_str()) {}
		ScannerProcessingException(const ScannerProcessingException& willCopy)
			: message(willCopy.what()) {}
		
		virtual ~ScannerProcessingException() = default;
		

		const char* what() const noexcept override {
			return message;
		}

	};

	// 存储一个非法标识符的 行号、列号、值
	class IdentifierInfo {
	public:
		// 起始位置的行号
		int line;

		// 起始位置的列号
		int row;

		// 标识符的值
		string value;

		IdentifierInfo(int line, int row, const string& value) : line(line), row(row), value(value) {}
		IdentifierInfo() : line(0), row(0), value("") {}

		// 赋值运算符（拷贝函数）
		IdentifierInfo& operator=(const IdentifierInfo& iden) {
			this->line = iden.line;
			this->row = iden.row;
			this->value = iden.value;
		}
	};

	// 标识符错误
	class ScannerIdentifierException : public ScannerProcessingException {
	protected:

		// 这个非法标识符的信息
		IdentifierInfo Position;

		// 通过标识符信息生成报错信息
		string createMessage(int line, int row, const string& value)const {
			ostringstream result;
			result << "At line " << line << ", row " << row << ":\n";
			result << "\tThe identifier: " << value << "is illegal.";
			return result.str();
		}
		string createMessage(const IdentifierInfo& Iden)const {
			ostringstream result;
			result << "At line " << Iden.line << ", row " << Iden.row << ":\n";
			result << "\tThe identifier: " << Iden.value << "is illegal.";
			return result.str();
		}

	public:

		// 构造函数
		ScannerIdentifierException(int line, int row, const string& value)
			: ScannerProcessingException(createMessage(IdentifierInfo(line, row, value)).c_str()) {
			Position.line = line;
			Position.row = row;
			Position.value = value;
		}
		ScannerIdentifierException(const IdentifierInfo& iden)
			: ScannerProcessingException(createMessage(iden).c_str()) {
			Position = iden;
		}
		
		// 使用父类的读取函数
		using ScannerProcessingException::what;

	};

}

trecmake_end


#endif // !CHECKEREXCEPTIONS_H
