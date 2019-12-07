#pragma once

#include <string>
using namespace std;

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
	INLINERET,  //函数内联之后的返回语句
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
	EXIT,  //退出 main最后
};

class midCode {  //z = x op y
public:
	operation op; // 操作
	string z;     // 结果
	string x;     // 左操作数
	string y;     // 右操作数
	midCode(operation o, string zz, string xx, string yy) : op(o), z(zz), x(xx), y(yy) {}
};

void outputMidCode();
