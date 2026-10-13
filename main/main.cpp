#include <iostream>
#include <vector>

using namespace std;

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



	return 0;

}