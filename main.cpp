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
#include <sstream>
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
ofstream midCodefile;
int indexs = 0;  //文件的索引
int oldIndex;    //用于做恢复
int line = 1;  //记录行号
int labelId = 0; //标号的id
int tmpVarId = 0;  //中间变量的id
map<string, symbolItem> globalSymbolTable;
map<string, symbolItem> localSymbolTable;
map<string, map<string, symbolItem>> allLocalSymbolTable;  //
vector<midCode> midCodeTable;
int globalAddr = 0;
int localAddr = 0;

int main() {
	inputfile.open("testfile.txt", ios::in);
	outputfile.open("output.txt", ios::out);
	errorfile.open("error.txt", ios::out);
	midCodefile.open("midCode.txt", ios::out);
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
	outputMidCode();
	inputfile.close();
	outputfile.close();
	errorfile.close();
	midCodefile.close();
	showGlobal();
	showAll();
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

string int2string(int t) {
	stringstream ss;
	ss << t;
	return ss.str();
}

string genLabel() {
	labelId++;
	return "Label" + int2string(labelId);
}

string genTmp() {
	tmpVarId++;
	return "T" + int2string(tmpVarId);
}

void outputMidCode() {
	for (int i = 0; i < midCodeTable.size(); i++) {
		midCode mc = midCodeTable[i];
		switch (mc.op) {
			case PLUSOP:
				midCodefile << mc.z << " = " << mc.x << " + " << mc.y << "\n";
				break;
			case MINUOP:
				midCodefile << mc.z << " = " << mc.x << " - " << mc.y << "\n";
				break;
			case MULTOP:
				midCodefile << mc.z << " = " << mc.x << " * " << mc.y << "\n";
				break;
			case DIVOP:
				midCodefile << mc.z << " = " << mc.x << " / " << mc.y << "\n";
				break;
			case LSSOP:
				midCodefile << mc.z << " = (" << mc.x << " < " << mc.y << ")\n";
				break;
			case LEQOP:
				midCodefile << mc.z << " = (" << mc.x << " <= " << mc.y << ")\n";
				break;
			case GREOP:
				midCodefile << mc.z << " = (" << mc.x << " > " << mc.y << ")\n";
				break;
			case GEQOP:
				midCodefile << mc.z << " = (" << mc.x << " >= " << mc.y << ")\n";
				break;
			case EQLOP:
				midCodefile << mc.z << " = (" << mc.x << " == " << mc.y << ")\n";
				break;
			case NEQOP:
				midCodefile << mc.z << " = (" << mc.x << " != " << mc.y << ")\n";
				break;
			case ASSIGNOP:
				midCodefile << mc.z << " = " << mc.x <<  "\n";
				break;
			case GOTO:
				midCodefile << "GOTO " << mc.z << "\n";
				break;
			case BZ:
				midCodefile << "BZ " << mc.z << "(" << mc.x << "=0)" << "\n";
				break;
			case BNZ:
				midCodefile << "BNZ " << mc.z << "(" << mc.x << "=1)" << "\n";
				break;
			case PUSH:
				midCodefile << "PUSH " << mc.z << "\n";
				break;
			case CALL:
				midCodefile << "CALL " << mc.z << "\n";
				break;
			case RET:
				midCodefile << "RET " << mc.z << "\n";
				break;
			case RETVALUE:
				midCodefile << "RETVALUE " << mc.z << " = " << mc.x << "\n";
				break;
			case SCAN:
				midCodefile << "SCAN " << mc.z << "\n";
				break;
			case PRINT:
				midCodefile << "PRINT " << mc.z << "\n";
				break;
			case LABEL:
				midCodefile << mc.z << ": \n";
				break;
			case CONST:
				midCodefile << "CONST " << mc.z << " " << mc.x << " = " << mc.y << endl;
				break;
			case ARRAY:
				midCodefile << "ARRAY " << mc.z << " " << mc.x << "[" << mc.y << "]" << endl;
				break;
			case VAR:
				midCodefile << "VAR " << mc.z << " " << mc.x << endl;
				break;
			case FUNC:
				midCodefile << "FUNC " << mc.z << " " << mc.x << "()" << endl;
				break;
			case PARAM:
				midCodefile << "PARA " << mc.z << " " << mc.x << endl;
				break;
			case GETARRAY:
				midCodefile << mc.z << " = " << mc.x << "[" << mc.y << "]\n";
				break;
			case PUTARRAY:
				midCodefile << mc.z << "[" << mc.x << "]" << " = " << mc.y << "\n";
				break;
			default:
				break;
		}
	}
}