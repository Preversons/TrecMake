#ifndef CHECKERMATCHER_H
#define CHECKERMATCHER_H

#include <iostream>
#include <string>
#include <vector>
#include <variant>
#include "macros.h"
#include "checkerBasic.h"
using namespace std;

trecmake_begin

namespace checker {

	// token匹配器，提供对token的匹配功能
	namespace matchers {

		// token类型
		enum class TokenType {
			Identifier,            // 标识符
			StringLiteral,         // 字符串常量
			CharLiteral,           // 字符常量
			NumberLiteral,         // 数字常量
			Operator,              // 运算符
			Keyword,               // 关键字
			Unknown                // 未知类型
		};


		// 匹配器
		class Matcher {
		private:
			// 待匹配的token
			string toMatchToken;

			// 该token合法的类型
			vector<variant<TokenType, string> > legalTypes;

			// [内部] 检测token类型对于给定的类型是否合法
			bool __match(const TokenType& type)const {
				switch (type) {
				case TokenType::Identifier:
					return utils::isLegalIdentifier(toMatchToken);

				case TokenType::StringLiteral:
					return utils::isLegalStringLiteral(toMatchToken);

				case TokenType::CharLiteral:
					return utils::isLegalCharLiteral(toMatchToken);

				case TokenType::NumberLiteral:
					return utils::isLegalNumberLiteral(toMatchToken);

				case TokenType::Operator:
					if (toMatchToken == "->" || toMatchToken == "=")return true;
					else return false;

				case TokenType::Keyword:
					return false;

				case TokenType::Unknown:
					return false;
				}
			}

		public:
			Matcher(const string& toMatchToken, const vector<variant<TokenType, string> >& legalTypes = {}) :
				toMatchToken(toMatchToken), legalTypes(legalTypes) {}

			Matcher(const Matcher& willCopy) :
				toMatchToken(willCopy.toMatchToken), legalTypes(willCopy.legalTypes) {}

			// 检测token是否合法，并返回匹配结果
			variant<TokenType, string> match() const {
				for (const variant<TokenType, string>& type : legalTypes) {
					if (auto p = get_if<TokenType>(&type)) {
						if (__match(*p)) {
							return *p;
						}
					}
					else if (auto p = get_if<string>(&type)) {
						if (toMatchToken == *p) {
							return *p;
						}
					}
				}
				return TokenType::Unknown;
			}

			// 获取待匹配的token
			const string& getToken() const {
				return toMatchToken;
			}

			// 获取合法的类型列表
			const vector<variant<TokenType, string> >& getLegalTypes() const {
				return legalTypes;
			}

			// 是否合法
			operator bool()const {
				auto matchResult = match();

				if (auto result = get_if<TokenType>(&matchResult)) {
					return (*result) != TokenType::Unknown;
				}
				else if (auto result = get_if<string>(&matchResult)) {
					return !result->empty();
				}
				return false;
			}

		};


	}

	namespace utils {

		// 将TokenType转换为字符串
		string tokenTypeToString(matchers::TokenType type) {
			switch (type) {
			case matchers::TokenType::Identifier:
				return "Identifier";

			case matchers::TokenType::StringLiteral:
				return "StringLiteral";

			case matchers::TokenType::CharLiteral:
				return "CharLiteral";

			case matchers::TokenType::NumberLiteral:
				return "NumberLiteral";

			case matchers::TokenType::Operator:
				return "Operator";

			case matchers::TokenType::Keyword:
				return "Keyword";

			case matchers::TokenType::Unknown:
				return "Unknown";

			default:
				return "Unknown";
			}
		}
	}
}


trecmake_end



#endif // !CHECKERMATCHER_H
