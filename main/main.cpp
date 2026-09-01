#include <iostream>
#include <vector>
#include <trc/byte.h>

using namespace std;

const char* operator"" _upper(const char* str, size_t len) {
	char* result = new char[len + 1];
	memcpy(result, str, len);

	result[len] = '\0';

	for (size_t i = 0;i < len;i++) {
		if (result[i] <= 'z' && result[i] >= 'a') {
			result[i] &= 0b11011111;
		}
	}
	return result;
}

unsigned long long operator"" _add_one(unsigned long long value) {
	return value + 1;
}

int main(int argc, char* argv[]) {


	// 以vector<string>存储的参数列表，不可更改
	const vector<string> args = 
		[](int _argc, char* _argv[])->vector<string> {
		vector<string> temp;
		for (int i = 0;i < _argc;i++) {
			temp.push_back(_argv[i]);
		}
		return temp;
		}
	(argc, argv);

	trecmake::Byte b = 0x00;

	cout << b << endl;

	cin >> b;

	cout << b << endl;
	cout << static_cast<char>(static_cast<int>(b.toggle(5))) << endl;
	cout << b.toggle(5) << endl;
	cout << static_cast<trecmake::Byte>(b.toggle(5)) << endl;

	cout << "Hello, World!"_upper << endl;
	cout << 48_add_one << endl;

	return 0;

}