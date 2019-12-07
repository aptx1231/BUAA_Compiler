#include <string>
#include <map>
#include <sstream>
#include <iostream>
#include "symbolItem.h"
#include "function.h"
#include "midCode.h"
using namespace std;

extern map<string, symbolItem> globalSymbolTable;
extern map<string, symbolItem> localSymbolTable;
extern map<string, map<string, symbolItem>> allLocalSymbolTable;  //保存所有的局部符号表 用于保留变量的地址
extern vector<string> stringList;  //保存所有的字符串
extern map<string, vector<midCode> > funcMidCodeTable;
int labelId = 0; //标号的id
int tmpVarId = 0;  //中间变量的id
int nameId = 0;  //函数内联新产生的变量名常量名数组名

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

void showAll() {
	cout << "----------------\n";
	for (map<string, map<string, symbolItem>>::iterator it = allLocalSymbolTable.begin(); it != allLocalSymbolTable.end(); it++) {
		cout << "Func: " << (*it).first << "\n";
		map<string, symbolItem> ss = (*it).second;
		for (map<string, symbolItem>::iterator i = ss.begin(); i != ss.end(); i++) {
			(*i).second.output();
		}
	}
	cout << "----------------\n";
}

void showString() {
	cout << "Show strings:\n";
	for (int i = 0; i < stringList.size(); i++) {
		cout << stringList[i] << "\n";
	}
}

string int2string(int t) {
	stringstream ss;
	ss << t;
	return ss.str();
}

int string2int(string s) {
	stringstream ss;
	ss << s;
	int t;
	ss >> t;
	return t;
}

string genLabel() {
	labelId++;
	return "Label" + int2string(labelId);
}

string genTmp() {
	tmpVarId++;
	return "#T" + int2string(tmpVarId); //#开头 跟正常的变量区分开 
}

string genName() {
	nameId++;
	return "%INLINE_" + int2string(nameId); //%开头 跟正常的变量区分开 
}

void showFuncMidCode() {
	cout << "*********************\n";
	for (map<string, vector<midCode> >::iterator it = funcMidCodeTable.begin(); it != funcMidCodeTable.end(); it++) {
		vector<midCode> ve = (*it).second;
		for (int i = 0; i < ve.size(); i++) {
			midCode mc = ve[i];
			switch (mc.op) {
			case PLUSOP:
				cout << mc.z << " = " << mc.x << " + " << mc.y << "\n";
				break;
			case MINUOP:
				cout << mc.z << " = " << mc.x << " - " << mc.y << "\n";
				break;
			case MULTOP:
				cout << mc.z << " = " << mc.x << " * " << mc.y << "\n";
				break;
			case DIVOP:
				cout << mc.z << " = " << mc.x << " / " << mc.y << "\n";
				break;
			case LSSOP:
				cout << mc.z << " = (" << mc.x << " < " << mc.y << ")\n";
				break;
			case LEQOP:
				cout << mc.z << " = (" << mc.x << " <= " << mc.y << ")\n";
				break;
			case GREOP:
				cout << mc.z << " = (" << mc.x << " > " << mc.y << ")\n";
				break;
			case GEQOP:
				cout << mc.z << " = (" << mc.x << " >= " << mc.y << ")\n";
				break;
			case EQLOP:
				cout << mc.z << " = (" << mc.x << " == " << mc.y << ")\n";
				break;
			case NEQOP:
				cout << mc.z << " = (" << mc.x << " != " << mc.y << ")\n";
				break;
			case ASSIGNOP:
				cout << mc.z << " = " << mc.x << "\n";
				break;
			case GOTO:
				cout << "GOTO " << mc.z << "\n";
				break;
			case BZ:
				cout << "BZ " << mc.z << "(" << mc.x << "=0)" << "\n";
				break;
			case BNZ:
				cout << "BNZ " << mc.z << "(" << mc.x << "=1)" << "\n";
				break;
			case PUSH:
				cout << "PUSH " << mc.z << "\n";
				break;
			case CALL:
				cout << "CALL " << mc.z << "\n";
				break;
			case RET:
				cout << "RET " << mc.z << "\n";
				break;
			case RETVALUE:
				cout << "RETVALUE " << mc.z << " = " << mc.x << "\n";
				break;
			case SCAN:
				cout << "SCAN " << mc.z << "\n";
				break;
			case PRINT:
				cout << "PRINT " << mc.z << " " << mc.x << "\n";
				break;
			case LABEL:
				cout << mc.z << ": \n";
				break;
			case CONST:
				cout << "CONST " << mc.z << " " << mc.x << " = " << mc.y << endl;
				break;
			case ARRAY:
				cout << "ARRAY " << mc.z << " " << mc.x << "[" << mc.y << "]" << endl;
				break;
			case VAR:
				cout << "VAR " << mc.z << " " << mc.x << endl;
				break;
			case FUNC:
				cout << "FUNC " << mc.z << " " << mc.x << "()" << endl;
				break;
			case PARAM:
				cout << "PARA " << mc.z << " " << mc.x << endl;
				break;
			case GETARRAY:
				cout << mc.z << " = " << mc.x << "[" << mc.y << "]\n";
				break;
			case PUTARRAY:
				cout << mc.z << "[" << mc.x << "]" << " = " << mc.y << "\n";
				break;
			case EXIT:
				cout << "EXIT\n";
				break;
			default:
				break;
			}
		}
		cout << "\n";
	}
	cout << "*********************\n";
}