#ifndef CHECKERPARSER_H
#define CHECKERPARSER_H

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <functional>
#include <variant>
#include "macros.h"
#include "checkerBasic.h"
#include "checkerMatcher.h"
#include "checkerExceptions.h"
#include "overloads.h"
using namespace std;
using namespace trecmake::checker::matchers;


trecmake_begin


namespace checker {



	// 解析器
	class Parser {
	private:

		// 矩阵，仅是定义别名，便于类内使用
		template<typename T> using matrix = vector<vector<T>>;

		// token矩阵，一行为一个句，分为多个词
		matrix<string> tokens;

		// 一个解析行节点
		class parseLineNode {
		private:

			// 实际存储的token
			string token;
			
			// 状态，形如a(b)的都将b称为状态
			string state;

			// 合法的下一个token，这是由配置文件给出的；可以是多种可能，这将是这一套语法检测系统的精髓；每一项皆为指针
			vector<parseLineNode*> legalNextNodes;

			// 该节点的父节点
			parseLineNode* last;

			// [内部] 向legalNextNodes中添加一个合法的下一个token节点
			void insertToLegalNextNodes(parseLineNode node) {
				legalNextNodes.push_back(NULL);
				legalNextNodes.back() = new parseLineNode(node);
				legalNextNodes.back()->last = this;
			}


			friend Parser;

		public:

			parseLineNode() {}
			parseLineNode(const string& token, const string& state, parseLineNode* last, const vector<parseLineNode*>& legalNextNodes = {})
				: token(token), state(state), last(last), legalNextNodes(legalNextNodes) {}

			string getToken()const { return token; }
			string getState()const { return state; }
			vector<parseLineNode*> getLegalNextNodes()const { return legalNextNodes; }
			
			// 以下关于解析行的说明
			
			// 形如
			//     t(s) -> a, b(s1), c[ -> x ], d(s2)[ -> y(s3), z[ -> m ] ], ...
			// 的，我们将其转换为这个节点，如：
			// 
			// token = t
			// state = s
			// legalNextNodes = {
			//     { token = a, state = NONE, legalNextNodes = {} },
			//     { token = b, state = s1,   legalNextNodes = {} },
			//     { token = c, state = NONE, legalNextNodes = { { token = x, state = NONE, legalNextNodes = {} } } },
			//     {
			//         token = d, state = s2,   legalNextNodes = {
			//             { token = y, state = s3,   legalNextNodes = {} },
			//             { token = z, state = NONE, legalNextNodes = {
			//                 token = m, state = NONE, legalNextNodes = {}
			//             }}
			//         }
			//     }
			//     ...
			// }
			//

		};
		
	public: // debug public

		// 解析行列表
		using parseLineList = vector<parseLineNode>;
		

	public:
		Parser() : tokens({}) {}
		Parser(matrix<string> tokens) : tokens(tokens) {}

		// 更新token矩阵
		Parser& setTokens(matrix<string> tokens) {
			this->tokens = tokens;
		}

		// 解析结果
		class ParseResult {
		public:

			// 错误信息，以指针形式存储
			vector<ParserExceptions::ParserProcessingException*> errors;

			// 结果，即解析行列表，用vector存储
			parseLineList results;


			ParseResult(parseLineList results) : results(results) {}
			ParseResult(vector<ParserExceptions::ParserProcessingException*> errors) : errors(errors) {}
			ParseResult(parseLineList results, vector<ParserExceptions::ParserProcessingException*> errors)
				: results(results), errors(errors) {
			}

			// 是否报错
			bool isErrorReport() { return errors.size(); }

			// 访问解析结果的对应下标
			parseLineNode& operator[](int lineIndex) { return results[lineIndex]; }
		};

		// 将token矩阵读取到指定的解析行列表中
		ParseResult init() {

			// 解析结果
			parseLineList results;

			// 错误信息
			vector<ParserExceptions::ParserProcessingException*> errors;

			for (int lineIndex = 0;lineIndex < tokens.size();lineIndex++) {

				const auto& tokenLine = tokens[lineIndex];

				// 检查当前tokenLine是否合法
				{
					//// 应该检查的token下标
					//int checkIndex = 0;
					//
					//// 检查第一个token是否合法
					//if (!(utils::isLegalIdentifier(tokenLine[checkIndex]) || utils::isLegalStringLiteral(tokenLine[checkIndex]))) {
					//	errors.push_back(NULL);
					//	errors.back() = new ParserExceptions::ParserStructureIllegalException("InitialToken");
					//}
					//checkIndex++;
					//
					//// 如果第二个token可能是状态，则检查状态是否合法
					//if (tokenLine[checkIndex][0] == '(') {
					//	if (tokenLine[checkIndex].back() != ')') {
					//		errors.push_back(NULL);
					//		errors.back() = new ParserExceptions::ParserTokenIllegalException(tokenLine[checkIndex], "State or \"->\"");
					//	}
					//	else {
					//		if (!utils::isLegalIdentifier(tokenLine[checkIndex].substr(1, tokenLine[checkIndex].size() - 2))) {
					//			errors.push_back(NULL);
					//			errors.back() = new ParserExceptions::ParserTokenIllegalException(tokenLine[checkIndex], "State or \"->\"");
					//		}
					//	}
					//
					//
					//}
					//if (tokenLine[checkIndex] != "->") {
					//	errors.push_back(NULL);
					//	errors.back() = new ParserExceptions::ParserTokenIllegalException(tokenLine[checkIndex], "State or \"->\"");
					//}
					//checkIndex++;
					;

					// 应该检查的token下标
					int checkIndex = 0;

					// 一种token可能性
					using aPossibility = variant<matchers::TokenType, string>;


					// 检查当前token是否合法，并返回下一个token的可能性
					function< vector<aPossibility>(vector<aPossibility>) > checkTokenIsLegal =
						[tokenLine, &errors, &checkIndex](vector<aPossibility> possibilitys) -> vector<aPossibility> {
						/*
						 * 对于possibilitys的说明：
						 * possibilitys是一个整数集合，表示tokenLine中以checkIndex为下标的token的合法类型集合
						 * 对于每一个项，都使用TokenType枚举类型表示
						 */

						if (tokenLine[checkIndex] == ";") { // 如果当前token是";"，则视为句末直接返回
							return {};
						}

						matchers::Matcher matcher(tokenLine[checkIndex], possibilitys);

						// 如果当前token不合法，则报错
						if (!matcher) {
							string possibilitysStr = "\"";
							for (const auto& possibility : possibilitys) {
								if (auto p = get_if<matchers::TokenType>(&possibility)) {
									possibilitysStr += (utils::tokenTypeToString(*p) + "\" or \"");
								}
								else if (auto p = get_if<string>(&possibility)) {
									possibilitysStr += ((*p) + "\" or \"");
								}
							}
							if (possibilitysStr.size() > 1) {
								// 去掉最后的"\" or \""
								possibilitysStr.pop_back();
								possibilitysStr.pop_back();
								possibilitysStr.pop_back();
								possibilitysStr.pop_back();
								possibilitysStr.pop_back();
								possibilitysStr.pop_back();
							}
							else {
								possibilitysStr = "";
							}

							cout << "\033[91mMade an error:  " << setw(15) << (tokenLine[checkIndex]) << "  should:  " << setw(15) <<
								(possibilitysStr + "  ") << ", before it:  " << tokenLine[checkIndex - (checkIndex > 0 ? 1 : 0)] << "\033[0m" << endl;
							
							errors.push_back(NULL);
							errors.back() =
								new ParserExceptions::ParserTokenIllegalException(tokenLine[checkIndex], possibilitysStr);
							
						}


						// 下一个token的可能性
						vector<aPossibility> nextPossibilitys;

						auto matchResult = matcher.match();
						
						// 如果是以TokenType存储的
						if (auto result = get_if<matchers::TokenType>(&matchResult)) {
							if (*result == matchers::TokenType::Identifier) {
								nextPossibilitys = { "(", ",", "[", ";", ")", "->" };
							}
							if (*result == matchers::TokenType::StringLiteral) {
								nextPossibilitys = { "(", ",", "[", "]", ";", "->"};
							}
						}
						// 如果是以string存储的
						else if (auto result = get_if<string>(&matchResult)) {
							string s = *result;
							if (s == "->") {
								nextPossibilitys = { matchers::TokenType::Identifier, matchers::TokenType::StringLiteral };
							}
							if (s == "[") {
								nextPossibilitys = { "->" };
							}
							if (s == "]") {
								nextPossibilitys = { ",", "]", ";" };
							}
							if (s == ",") {
								nextPossibilitys = {
									matchers::TokenType::Identifier,
									matchers::TokenType::StringLiteral,
									
								};
							}
							if (s == "(") {
								nextPossibilitys = { matchers::TokenType::Identifier };
							}
							if (s == ")") {
								nextPossibilitys = { "[", ",", ";", "->"};
							}
						}
						
						// 返回下一个token的可能性
						return nextPossibilitys;


						}; // end checkTokenIsLegal



					// 检查后续的token是否合法
					{
						vector<aPossibility> possibilitys =
							{ matchers::TokenType::Identifier, matchers::TokenType::StringLiteral, ";" };
						for (;checkIndex < tokenLine.size();checkIndex++) {
							possibilitys = checkTokenIsLegal(possibilitys);
						}
					}

				} // end check

				// 当前解析行节点
				parseLineNode root;

				// 当前写入的节点
				parseLineNode* currentNode = &root;
				
				// 将每一个配置文件的token插入到解析行列表中
				{
					// 正在读取的token下标
					int readingTokenIndex = 0;

					// 先把行头处理掉
					currentNode->token = tokenLine[readingTokenIndex];
					if (tokenLine[readingTokenIndex + 1] == "->") {
						currentNode->state = "";
						readingTokenIndex += 2;
					}
					else if (tokenLine[readingTokenIndex + 1] == "(") {
						currentNode->state = tokenLine[readingTokenIndex + 2];

						// 将状态标识符后面的两个token，即固定的 ")" 和 "->" 报错处理一下
						if (tokenLine[readingTokenIndex + 3] != ")") {
							errors.push_back(NULL);
							errors.back() = new ParserExceptions::ParserStructureIllegalException(")");
						}
						if (tokenLine[readingTokenIndex + 4] != "->") {
							errors.push_back(NULL);
							errors.back() = new ParserExceptions::ParserStructureIllegalException("->");
						}
						
						if (tokenLine[readingTokenIndex + 3] != ")")readingTokenIndex--; // 为了后续的处理能够顺利进行，暂且将这个需要的token看成存在的
						if (tokenLine[readingTokenIndex + 4] != "->")readingTokenIndex--; // 同理如上

						// 现在的readingTokenIndex应该为0，即
						// <token>   "("   <state>   ")"   "->"    ...
						// ^^^0^^^    1       2       3     4
						// ，要想越过 "->"，应该将readingTokenIndex增加5
						// 

						readingTokenIndex += 5;
					}
					else { // 既不是 "->"，也不是 "("，不合法，报错
						errors.push_back(NULL);
						errors.back() = new ParserExceptions::ParserTokenIllegalException(
							tokenLine[readingTokenIndex + 1],
							"\"->\" or \"(\""
						);
					}


					enum class processingStates {
						Normal, // 普通状态
						Special, // 特殊状态，表示处于"[" 和 "]" 之间的状态
					} processingState = processingStates::Normal;


					// 越过"->"后处理legalNextNodes
					for (;readingTokenIndex < tokenLine.size();readingTokenIndex++) {
						if (readingTokenIndex >= tokenLine.size()) {
							break;
						}

						cout << "lineIndex = " << lineIndex << endl;

						if (processingState == processingStates::Normal) {
							auto token = tokenLine[readingTokenIndex];
							auto state = [&tokenLine, &readingTokenIndex, &errors]() -> string {

								// 移到token的下一个对象
								readingTokenIndex++;

								// 如果当前token是"("，则说明后面有状态标识符
								if(readingTokenIndex < tokenLine.size()){
									if (tokenLine[readingTokenIndex] == "(") {
										readingTokenIndex++;

										// 返回状态
										return [&tokenLine, &readingTokenIndex, &errors]() -> string {

											// 状态
											string state = tokenLine[readingTokenIndex];

											readingTokenIndex++;

											// 读取，同时检查 ")" 是否存在
											if (tokenLine[readingTokenIndex] == ")") {
												readingTokenIndex++;
											}
											else {
												errors.push_back(NULL);
												errors.back() = new ParserExceptions::ParserStructureIllegalException(")");
											}


											// 返回状态，同时注意状态标识符是否合法
											if (utils::isLegalIdentifier(state)) {
												return state;
											}
											else {
												errors.push_back(NULL);
												errors.back() = new ParserExceptions::ParserTokenIllegalException(state, "State");
												return string("<Illegal>");
											}

											}(); // end return state
									}
									// 如果不是")"，则返回空字符串
									else {
										return string("");
									}
								} // end if

								// 如果下标即将越界，说明后面没有剩余的项了，返回空字符串
								else {
									return string("");
								}

								}();
							auto last = currentNode;

							currentNode->insertToLegalNextNodes(parseLineNode(token, state, last));
						}
						else if (processingState == processingStates::Special) {
							// 当前token应该是"["后的"->"，如果不是，则报错，并跳过这一组"[" "]"
							if (tokenLine[readingTokenIndex] != "->") {
								errors.push_back(NULL);
								errors.back() = new ParserExceptions::ParserStructureIllegalException("->");
								
								// 进入的"[...]"的处理层级
								int level = 1;

								// 跳过这一组"[" "]"
								for (;level > 0;readingTokenIndex++) {
									if (tokenLine[readingTokenIndex] == "[") {
										level++;
									}
									else if (tokenLine[readingTokenIndex] == "]") {
										level--;
									}
								}

								// 即将短路掉else
							}

							// 合法
							else {
								// 跳过这个"->"
								readingTokenIndex++;

								while (readingTokenIndex < tokenLine.size()) {
									auto token = tokenLine[readingTokenIndex];
									auto state = [&tokenLine, &readingTokenIndex, &errors]() -> string {

										// 移到token的下一个对象
										readingTokenIndex++;

										// 如果当前token是"("，则说明后面有状态标识符
										if (readingTokenIndex < tokenLine.size()) {
											if (tokenLine[readingTokenIndex] == "(") {
												readingTokenIndex++;

												// 返回状态
												return [&tokenLine, &readingTokenIndex, &errors]() -> string {

													// 状态
													string state = tokenLine[readingTokenIndex];

													readingTokenIndex++;

													// 读取，同时检查 ")" 是否存在
													if (tokenLine[readingTokenIndex] == ")") {
														readingTokenIndex++;
													}
													else {
														errors.push_back(NULL);
														errors.back() = new ParserExceptions::ParserStructureIllegalException(")");
													}


													// 返回状态，同时注意状态标识符是否合法
													if (utils::isLegalIdentifier(state)) {
														return state;
													}
													else {
														errors.push_back(NULL);
														errors.back() = new ParserExceptions::ParserTokenIllegalException(state, "State");
														return string("<Illegal>");
													}

													}(); // end return state
											}
											// 如果不是")"，则返回空字符串
											else {
												return string("");
											}
										} // end if

										// 如果下标即将越界，说明后面没有剩余的项了，返回空字符串
										else {
											return string("");
										}

										}();
									auto last = currentNode;

									currentNode->insertToLegalNextNodes(parseLineNode(token, state, last));

									// readingTokenIndex++;
									// 这里之所以不需要readingTokenIndex++,
									// 是因为上面的
									//    auto state = [&tokenLine, &readingTokenIndex, &errors]() -> string
									// 已经存在一个readingTokenIndex++，
									// 用于检测是否是 "("，
									// 如果此处再加，可能产生数组越界
									//

									cout << "readingTokenIndex = " << readingTokenIndex << endl;

									if (tokenLine[readingTokenIndex] == "]") {
										processingState = processingStates::Normal;
										break;
									}
									else if (tokenLine[readingTokenIndex] == ",") {
										readingTokenIndex++;
									}
									else {
										errors.push_back(NULL);
										errors.back() = new ParserExceptions::ParserTokenIllegalException("] or ,");
									}
								}
							}
						}
						

						if (readingTokenIndex >= tokenLine.size()) {
							break;
						}

						if (tokenLine[readingTokenIndex] == ",") {
							continue;
						}
						else if (tokenLine[readingTokenIndex] == "[") {
							currentNode = currentNode->legalNextNodes.back();
							processingState = processingStates::Special;
						}
						else if (tokenLine[readingTokenIndex] == "]") {
							if (currentNode->last == NULL) {
								errors.push_back(NULL);
								errors.back() = new ParserExceptions::ParserTokenIllegalException(tokenLine[readingTokenIndex]);
							}
							else {
								currentNode = currentNode->last;
								processingState = processingStates::Normal;
							}
						}
						else {
							errors.push_back(NULL);
							errors.back() = new ParserExceptions::ParserStructureIllegalException("\",\" or \"[\" or \"]\"");
							cout << "readingTokenIndex = " << readingTokenIndex << endl;
							cout << "tokenLine[readingTokenIndex] = " << tokenLine[readingTokenIndex] << endl;
						}
					}
				}

				

				results.push_back(root);
				
			}
			
			return ParseResult(results, errors);
		}

	};

}


trecmake_end





#endif // !CHECKERPARSER_H
