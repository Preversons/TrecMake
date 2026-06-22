#pragma once

#include <sstream>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <stdexcept>

#include "macros.h"
using namespace std;

#define stream_processer_begin namespace compiler { namespace stream_processer {
#define stream_processer_end   }}

trecmake_begin

stream_processer_begin

// in: trecmake::compiler::stream_processer






vector<string> key_words = {
	"array",  "as",   "bool",  "break",  "case",  "char",   "const",  "continue",
	"else",   "enum", "error", "export", "false", "float",  "for",    "from",
	"func",   "if",   "impl",  "import", "in",    "int",    "let",    "list",    "map",
	"match",  "mut",  "new",   "none",   "ok",    "option", "own",    "result",
	"return", "self", "set",   "shared", "some",  "str",    "struct", "true",
	"type",   "uint", "var",   "where",  "while"
};

void read_kw_from(string path) {
	string word = "";
	fstream file(path.c_str());

	if (!file.is_open()) {
		throw 
	}

	while (!file.eof()) {

	}
}











stream_processer_end

trecmake_end