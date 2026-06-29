#include <iostream>
#include <iomanip>
#include <string>
#include <Windows.h>
#include "checkerScanner.h"
#include "overloads.h"
#include "macros.h"
using namespace std;
using namespace trecmake::checker;

int main(int argc, char* argv[]) {

	// 是否开启调试模式
	bool isDebugging;

	isDebugging = false;

	if (argc < 2 && !isDebugging) {
		cerr << "Usage: " << argv[0] << " <configFilePath>" << endl;
		return 1;
	}

	enable_colors; // 开启颜色输出支持

	// 配置文件的文件名
	string configFilePath = argv[1];

	Scanner input(configFilePath);

	auto tokens = input.tokenize().results;
	vector<ScannerExceptions::ScannerProcessingException*> errors = input.tokenize().errors;
	
	cout << "tokens = " << endl;
	for (int i = 0;i < tokens.size();i++) {
		for (int j = 0;j < tokens[i].size();j++) {
			cout << setw(13) << left << (string("|") + tokens[i][j] + string("| "));
		}
		cout << endl;
	}

	cout << "errors = " << endl;
	for (int i = 0;i < errors.size();i++) {
		cout << (errors[i])->what() << endl << endl;
	}


	system("pause");

	return 0;
}