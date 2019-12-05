#include <string>
#include <map>
#include <sstream>
#include "symbolItem.h"
#include "function.h"
using namespace std;

extern map<string, symbolItem> globalSymbolTable;
extern map<string, symbolItem> localSymbolTable;
extern map<string, map<string, symbolItem>> allLocalSymbolTable;  //保存所有的局部符号表 用于保留变量的地址
extern vector<string> stringList;  //保存所有的字符串
int labelId = 0; //标号的id
int tmpVarId = 0;  //中间变量的id

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