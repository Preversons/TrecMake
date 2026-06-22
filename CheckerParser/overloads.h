#ifndef OVERLOADS_H
#define OVERLOADS_H

#include <iostream>
#include <vector>
using namespace std;

template<typename T>
ostream& operator<<(ostream& os, const vector<T>& vec) {
	os << "[\n";
	for (int i = 0; i < vec.size(); i++) {
		os << "\t\"" << vec[i] << "\"";
		if (i != vec.size() - 1)os << ",\n";
	}
	os << "]";

	return os;
}






#endif // !OVERLOADS_H
