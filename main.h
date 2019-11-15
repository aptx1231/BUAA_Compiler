#pragma once
#ifndef MAIN_H
#define MAIN_H
#include <iostream>
#include <string>
#include <vector>
#include <map>
using namespace std;

enum typeId {
	CONSTTK, INTTK, CHARTK, VOIDTK, MAINTK, IFTK, ELSETK, DOTK, WHILETK, FORTK, SCANFTK, PRINTFTK, RETURNTK,  //保留字
	IDENFR, INTCON, CHARCON, STRCON,
	PLUS, MINU, MULT, DIV, LSS, LEQ, GRE, GEQ, EQL, NEQ, ASSIGN, SEMICN, COMMA, LPARENT, RPARENT, LBRACK, RBRACK, LBRACE, RBRACE
};

enum operation {
	PLUSOP, //+
	MINUOP, //-
	MULTOP, //*
	DIVOP,  // /
	LSSOP,  //<
	LEQOP,  //<=
	GREOP,  //>
	GEQOP,  //>=
	EQLOP,  //==
	NEQOP,  //!=
	ASSIGNOP,  //=
	GOTO,  //无条件跳转
	BZ,    //不满足条件跳转
	BNZ,   //满足条件跳转
	PUSH,  //函数调用时参数传递
	CALL,  //函数调用
	RET,   //函数返回语句
	RETVALUE, //有返回值函数返回的结果
	SCAN,  //读
	PRINT, //写
	LABEL, //标号
	CONST, //常量
	ARRAY, //数组
	VAR,   //变量
	FUNC,  //函数定义
	PARAM, //函数参数
	GETARRAY,  //取数组的值  t = a[]
	PUTARRAY,  //给数组赋值  a[] = t
};

class symbolItem {
public:
	string name;
	int kind; //var const function array
	int type; //int char void
	int constInt;
	char constChar;
	int length;  //数组长度  对于函数用于记录这个函数有多少变量（参数+局部+临时)
	vector<int> parameterTable;  //参数类型
	int addr;   //地址 
	symbolItem(string s, int add = 0, int k=0, int t=0, int ci=0, char cc=' ', int l=0) :
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

void showGlobal();

void showLocal();

void showAll();

class midCode {  //z = x op y
public:
	operation op; // 操作
	string z;     // 结果
	string x;     // 左操作数
	string y;     // 右操作数
	midCode(operation o, string zz, string xx, string yy) : op(o), z(zz), x(xx), y(yy) {}
};

string int2string(int t);

string genLabel();

string genTmp();

void outputMidCode();
#endif // !MAIN_H
