#pragma once
#ifndef MAIN_H
#define MAIN_H
#include <string>
#include <vector>
using namespace std;

enum typeId {
	CONSTTK, INTTK, CHARTK, VOIDTK, MAINTK, IFTK, ELSETK, DOTK, WHILETK, FORTK, SCANFTK, PRINTFTK, RETURNTK,  //保留字
	IDENFR, INTCON, CHARCON, STRCON,
	PLUS, MINU, MULT, DIV, LSS, LEQ, GRE, GEQ, EQL, NEQ, ASSIGN, SEMICN, COMMA, LPARENT, RPARENT, LBRACK, RBRACK, LBRACE, RBRACE
};

class symbolItem {
	string name;
	int kind; //var const function array
	int type; //int char void
	int constInt;
	char constChar;
	int length;  //数组长度
	vector<int> parameterType;  //参数类型
	symbolItem(string s, int k, int t, int ci, char cc, int l) :
		name(s), kind(k), type(t), constInt(ci), constChar(cc), length(l) {
		parameterType = vector<int>();
	}
};

#endif // !MAIN_H
