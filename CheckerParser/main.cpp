#include <iostream>
#include <string>
#include <stdexcept>
#include "checkerScanner.h"
#include "overloads.h"
using namespace std;
using namespace trecmake::checker;

int main(int argc, char* argv[]) {
	bool isDebugging; // 是否开启调试模式

	isDebugging = false;

	if (argc < 2 && !isDebugging) {
		cerr << "Usage: " << argv[0] << " <configFilePath>" << endl;
		return 1;
	}

	string configFilePath = argv[1];

	// 测试字符串
	string __TestString = "\"Hello\" ->\", World\";"; // "Hello" ->", World";

	Scanner scr(configFilePath);
	auto tokens = scr.tokenize(__TestString, 0).results;

	for (const auto& token : tokens) {
		cout << "|" << token << "|" << endl;
	}


	system("pause");

	return 0;
}