#include <iostream>
#include <iomanip>
#include <string>
#include <Windows.h>
#include "checkerScanner.h"
#include "checkerParser.h"
#include "checkerComparer.h"
#include "overloads.h"
#include "macros.h"
using namespace std;
using namespace trecmake::checker;

int main(int argc, char* argv[]) {

	// 是否开启调试模式
	bool isDebugging;

	isDebugging = true;

	if (argc < 2 && !isDebugging) {
		cerr << "Usage: " << argv[0] << " <configFilePath>" << endl;
		return 1;
	}

	enable_colors; // 开启颜色输出支持

	// 配置文件的文件名
	string configFilePath = "D:\\my folder\\TrecMake\\x64\\Debug\\test1.txt";

	Scanner input(configFilePath);

	auto tokens = input.tokenize().results;
	vector<ScannerExceptions::ScannerProcessingException*> errors = input.tokenize().errors;
	
	cout << "ScannerTokens = " << endl;
	for (int i = 0;i < tokens.size();i++) {
		for (int j = 0;j < tokens[i].size();j++) {
			cout << setw(13) << left << tokens[i][j];
		}
		cout << endl;
	}

	cout << "ScannerErrors = " << endl;
	for (int i = 0;i < errors.size();i++) {
		cout << (errors[i])->what() << endl << endl;
	}

	Parser parser(tokens);
	auto result = parser.init();

	auto parseErrors = result.errors;
	cout << "ParserErrors = " << endl;
	for (int i = 0;i < parseErrors.size();i++) {
		cout << (parseErrors[i])->what() << endl << endl;
	}

	Comparator comparer;
	comparer.setConfig(result.results);
	const auto& compareResult = comparer.match({
		"import",
		"module",
		".",
		"function"
		},

		{
			{ "function", [](const string& s) -> bool {
						if (s == "aExampleFunctionName")return true;
						else return false;
						}}
		}
	);
	cout << "CompareResult.isMatch = " << (compareResult.isMatch ? "true" : "false") << endl;
	if (!compareResult.isMatch) {
		for (const auto& errorPtr : compareResult.errors) {
			cout << errorPtr->what() << endl << endl;
		}
	}
	
	system("pause");
	return 0;
}