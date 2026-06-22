#ifndef PARSER_H
#define PARSER_H

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
	namespace matcher { // 匹配器

		class Matcher { // 匹配器基类
		public:
			virtual bool isLegal(const string&); // 判断匹配器是否合法
			virtual bool isLegal(const char*&);

		};

		class StringMatcher : public Matcher { // 字符串匹配器
		private:
			string value; // 如果给的字符串等于这个字符串则匹配成功

		public:
			bool isLegal(const string& input) override { // 判断字符串匹配器是否合法
				return input == value;
			}
			bool isLegal(const char*& input) override {
				return string(input) == value;
			}

			void setValue(const string& val) { // 设置字符串匹配器的值
				value = val;
			};

		};

		class SpecialMatcher : public Matcher { // 特殊匹配器
		private:
			bool(*function)(const string&); // 如果给的字符串满足这个函数则匹配成功

		public:
			SpecialMatcher() : function(nullptr) {} // 默认构造函数)
			SpecialMatcher(bool(*func)(const string&)) : function(func) {} // 构造函数

			bool isLegal(const string& input) override { // 判断特殊匹配器是否合法
				return function ? function(input) : false;
			}
			bool isLegal(const char*& input) override {
				return function ? function(string(input)) : false;
			}

		};

		namespace utils { // 工具函数

			// 判断字符串是否为数字
			bool isNumber(const string& input) {
				for (char c : input) {
					if (!isdigit(c)) return false;
				}
				return true;
			}

			// 判断字符串是否为标识符
			bool isIdentifier(const string& input) {
				if (input.empty() || !isalpha(input[0])) return false; // 标识符必须以字母开头
				for (char c : input) {
					if (!isalnum(c) && c != '_') return false; // 标识符只能包含字母、数字和下划线
				}
				return true;
			}

			// 判断字符串是否为字符串字面量
			bool isStringLiteral(const string& input) {
				return input.size() >= 2 && input.front() == '"' && input.back() == '"'; // 字符串字面量必须以双引号开头和结尾
			}

		}
	}

	namespace syntaxTree { // 语法树的节点定义以及函数

		struct node { // 语法树节点
			string value; // 节点的值
			vector<node*> children; // 子节点
			node(const string& val) : value(val) {} // 构造函数

			void addChild(node* child) { // 添加子节点
				children.push_back(child);
			}

			void print(int depth = 0) { // 打印语法树
				for (int i = 0; i < depth; ++i) cout << "  "; // 缩进

				cout << value << endl; // 打印当前节点的值

				for (node* child : children) { // 递归打印子节点
					child->print(depth + 1);
				}
			}

			node* findChild(const string& val) { // 查找子节点
				for (node* child : children) {
					if (child->value == val) return child;
				}
				return nullptr; // 没有找到
			}

			node* operator[](const string& val) { // 重载[]运算符，查找子节点
				return findChild(val);
			}

			node* operator[](int index) { // 重载[]运算符，按索引访问子节点
				if (index < 0 || index >= children.size()) return nullptr; // 索引越界
				return children[index];
			}

		};

		node* root = nullptr; // 语法树的根节点

	}

}

trecmake_end

#include "checkerScanner.h"


























#endif // !PARSER_H
