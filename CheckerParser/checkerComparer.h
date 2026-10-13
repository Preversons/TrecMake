#pragma once
#ifndef CHECKERCOMPARER_H
#define CHECKERCOMPARER_H

#include "checkerBasic.h"
#include "checkerParser.h"
#include "macros.h"
#include "overloads.h"
#include <vector>
#include <string>
#include <variant>
#include <functional>
#include <map>
using namespace std;

trecmake_begin

namespace checker {

	// 比对器错误信息
	namespace ComparisonErrorInfomations {
		
		// 比对器错误信息基类
		class ComparisonError {
		private:

			// 错误信息
			string message;

			// 缓存错误信息
			mutable string cachedMessage;

		public:

			ComparisonError() = default;
			ComparisonError(const string& message) : message(message) {}
			virtual ~ComparisonError() = default;

			// 错误位置
			struct ErrorPosition {

				// 行号
				int lineNumber;

				// 列号
				int rowNumber;

				ErrorPosition() : lineNumber(0), rowNumber(0) {}

			} position;

			// 获取错误信息
			const char* what() const {
				cachedMessage = 
					"\033[91mComparisonError:\n"
					"\tAt line " + to_string(position.lineNumber) + ", row " + to_string(position.rowNumber) + "\n"
					"\t\033[0m" + message;
				return cachedMessage.c_str();
			}
			
		};
	}

	// 比对器，用于比对输入是否符合配置信息
	class Comparator {
	private:

		// 配置信息
		Parser::parseTree config;

		// token列表类型别名
		using TokenList = vector<string>;

	public:

		Comparator() = default;
		Comparator(const Parser::parseTree& config) : config(config) {}

		// 比对结果类
		class ComparisonResult {
		public:
			
			// 比对结果
			bool isMatch;

			// 错误列表，以指针存储
			using ErrorList = vector<ComparisonErrorInfomations::ComparisonError*>;

			// 错误信息
			ErrorList errors;

			ComparisonResult(bool isMatch, const ErrorList& errors) : isMatch(isMatch), errors(errors) {}

		};

		// 设置配置信息
		// @param config 配置信息
		Comparator& setConfig(const Parser::parseTree& config) {
			this->config = config;

			// 起始行配置，可能为NULL
			const Parser::parseLineNode* starterConfig = NULL;

			for (const auto& node : config) {

				if (node.getToken() == "$__starter__") {
					// 如果当前行的起始token是 $__starter__ ，则说明当前行是特殊的，
					// 用于定义一个任意开启的行配置，比如（以C++示例）：
					// $__starter__ -> "for", "if", "while", ... 这些都是任意开启的行的配置
					// 
					// 而之所以需要这个 $__starter__， 是为了在函数的语法配置中更好地
					// 进行编写配置文件而设计的，如：
					// 
					// -- ((configure code --
					// 
					// ...
					// ")"(function_arg_list_define_end) -> "{"(function_body_begin), ";";
					// 
					// # 此时如果使用传统的配置方式，将会是：
					// # "{"(function_body_begin) -> "for", "if", "while", ...;
					// # 这样将可能会导致配置文件的编写变得复杂，尤其是当函数体内有多种语法结构时
					// # 
					// # 于是我们引入 $__starter__ 来简化配置文件的编写
					// # 如此使用：
					// 
					// "{"(function_body_begin) -> $__starter__;
					// 
					// # 在检查器中，$__starter__ 不会尝试匹配token，而是像一个占位符一样
					// # 表示任意开启的行配置
					// 
					// -- cofigure code )) --
					//

					starterConfig = &node;
				}
			}

			if (starterConfig == NULL) {
				throw ComparisonErrorInfomations::ComparisonError("No starter config found (need \"$__starter__\").");
			}

			return *this;
		}


		
		// 匹配并返回比对结果
		// @param input 输入的一个token行
		// @param matchFunctions 匹配函数的哈希表，key为token，value为匹配函数
		ComparisonResult match(
			const TokenList& input,
			const map<string, function<bool(const string&)>>& matchFunctions)
			const {

			// 获取指定行和列在所有行中的索引
			// @param lineIndex 行号
			// @param rowIndex 列号
			function<int(int, int)> getIndexInAllLine = [&](int lineIndex, int rowIndex) -> int {
				int index = 0;
				for (int i = 0; i < lineIndex; i++) {
					index += input[i].size();
				}
				return index + rowIndex;
				};

			// 起始行配置，可能为NULL
			const Parser::parseLineNode* starterConfig = NULL;

			for (const auto& node : config) {

				if (node.getToken() == "$__starter__") {
					// 如果当前行的起始token是 $__starter__ ，则说明当前行是特殊的，
					// 用于定义一个任意开启的行配置，比如（以C++示例）：
					// $start -> "for", "if", "while", ... 这些都是任意开启的行的配置

					starterConfig = &node;
				}
			}

			if (!starterConfig) {
				return ComparisonResult(false, {
					new ComparisonErrorInfomations::ComparisonError("No starter config found (need \"$__starter__\").")
					});
			}

			
			// 当前所处的配置节点
			Parser::parseLineNode* currentConfigNode = const_cast<Parser::parseLineNode*>(starterConfig);

			for (int tokenIndex = 0; tokenIndex < input.size(); tokenIndex++) {

				// 当前token
				const string& thisToken = input[tokenIndex];

				// 检查当前token是否在当前配置节点的legalNextNodes中
				bool isLegal = false;
				for (const auto& nextNode : currentConfigNode->getLegalNextNodes()) {
					if (utils::isLegalStringLiteral(nextNode->getToken())) {
						if (nextNode->getToken() == "\"" + thisToken + "\"") {
							isLegal = true;
							currentConfigNode = nextNode;
							break;
						}
					}

					// 如果当前token是一个标识符，则在匹配函数的哈希表中寻找合适的函数并匹配
					else {
						for (const auto& [key, matchFunction] : matchFunctions) {
							if (key == nextNode->getToken()) {
								if (matchFunction(thisToken)) {
									isLegal = true;
									currentConfigNode = nextNode;
									break;
								}
							}
						}
					}

				}
				if (!isLegal) {
					// 如果当前token不合法，则报错
					return ComparisonResult(false, {
						new ComparisonErrorInfomations::ComparisonError("Illegal token \"" + thisToken + "\" at the " + to_string(tokenIndex + 1) + "th token.")
						});
				}
			}




			
		} // end match

	};

} // end checker

trecmake_end


#endif // !CHECKERCOMPARER_H
