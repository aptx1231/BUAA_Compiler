#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstring>
#include <string>
#include <set>
#include <map>
#include "lexical.h"
#include "grammer.h"
#include "main.h"
using namespace std;

char ch;
char token[100000];
int tokenI = 0;
int num;   //记录整形常量
char con_ch;  //记录字符型常量
char s[100000];  //记录字符串常量
enum typeId symbol;
int len_reservedWord = 13;
char reservedWord[20][10] = {
		"const", "int", "char", "void", "main", "if", "else", "do", "while", "for", "scanf", "printf", "return",
};
string filecontent;  //文件的内容
ifstream inputfile;
ofstream outputfile;
ofstream errorfile;
int indexs = 0;  //文件的索引
int oldIndex;    //用于做恢复
set<string> haveReturnValueFunctionSet;
set<string> noReturnValueFunctionSet;
int line = 1;  //记录行号
map<string, symbolItem> globalSymbolTable;
map<string, symbolItem> localSymbolTable;

int main() {
	inputfile.open("testfile.txt", ios::in);
	outputfile.open("output.txt", ios::out);
	errorfile.open("error.txt", ios::out);
	indexs = 0;
	string tmpIn;
	while (getline(inputfile, tmpIn)) {  //读取文件内容
		filecontent.append(tmpIn);
		filecontent.append("\n");
	}
	int re = getsym();
	if (re < 0) {
		//error()
	}
	else {
		if (procedure()) {
			//success
		}
		else {
			//error()
		}
	}
	inputfile.close();
	outputfile.close();
	showGlobal();
	return 0;
}

void showGlobal() {
	cout << "----------------\n";
	for (map<string, symbolItem>::iterator it = globalSymbolTable.begin(); it != globalSymbolTable.end(); it++) {
		(*it).second.output();
	}
	cout << "----------------\n";
}

void showLocal() {
	cout << "----------------\n";
	for (map<string, symbolItem>::iterator it = localSymbolTable.begin(); it != localSymbolTable.end(); it++) {
		(*it).second.output();
	}
	cout << "----------------\n";
}
