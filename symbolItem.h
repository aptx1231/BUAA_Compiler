#pragma once
#ifndef SYMBOLITEM_H
#define SYMBOLITEM_H

#include <iostream>
#include <string>
#include <vector>
#include <map>
using namespace std;

class symbolItem {
public:
	string name;
	int kind; //var const function array
	int type; //int char void
	int constInt;
	char constChar;
	int length;  //数组长度  对于函数用于记录这个函数有多少变量（参数+局部变量+临时)
	vector<int> parameterTable;  //参数类型
	int addr;   //地址 
	symbolItem(string s, int add = 0, int k = 0, int t = 0, int ci = 0, char cc = ' ', int l = 0) :
		name(s), kind(k), type(t), constInt(ci), constChar(cc), length(l), addr(add) {
		parameterTable = vector<int>();
	}
	symbolItem() {}
	void output() {
		cout << name << " ";
		switch (type) {
		case 1:
			cout << "int ";
			break;
		case 2:
			cout << "char ";
			break;
		case 3:
			cout << "void ";
			break;
		}
		switch (kind) {
		case 1:
			cout << "var ";
			break;
		case 2:
			cout << "const ";
			if (type == 1) {
				cout << constInt << " ";
			}
			else if (type == 2) {
				cout << constChar << " ";
			}
			break;
		case 3:
			cout << "func ";
			cout << "parameters: (";
			for (int i = 0; i < parameterTable.size(); i++) {
				if (parameterTable[i] == 1) {
					cout << "int ";
				}
				else {
					cout << "char ";
				}
			}
			cout << ")" << " 参数个数: " << length << " ";
			break;
		case 4:
			cout << "array ";
			cout << length << " ";
			break;
		}
		cout << "addr: " << addr;
		cout << "\n";
	}
	void insert(int t) {
		parameterTable.push_back(t);
	}
};

#endif // !SYMBOLITEM_H
