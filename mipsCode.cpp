#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <stack>
#include "mipsCode.h"
#include "midCode.h"
#include "symbolItem.h"
#include "function.h"
using namespace std;

vector<mipsCode> mipsCodeTable;
string curFuncName = "";

extern ofstream mipsCodefile;
extern vector<string> stringList;  //保存所有的字符串
extern vector<midCode> midCodeTable;
extern map<string, symbolItem> globalSymbolTable;
extern map<string, map<string, symbolItem>> allLocalSymbolTable;  //保存所有的局部符号表 用于保留变量的地址

void loadValue(string name, string regName, bool gene, int& va, bool& get) {
	int addr;
	if (allLocalSymbolTable[curFuncName].find(name) != allLocalSymbolTable[curFuncName].end()) {
		if (allLocalSymbolTable[curFuncName][name].kind == 2) {  //const
			va = allLocalSymbolTable[curFuncName][name].type == 1 ?
				allLocalSymbolTable[curFuncName][name].constInt : allLocalSymbolTable[curFuncName][name].constChar;
			if (gene) {
				mipsCodeTable.push_back(mipsCode(li, regName, "", "", va));
			}
			get = true;
		}
		else {  //var
			addr = allLocalSymbolTable[curFuncName][name].addr;
			mipsCodeTable.push_back(mipsCode(lw, regName, "$fp", "", -4 * addr));
		}
	}
	else if (globalSymbolTable.find(name) != globalSymbolTable.end()) {
		if (globalSymbolTable[name].kind == 2) {  //const
			va = globalSymbolTable[name].type == 1 ? globalSymbolTable[name].constInt : globalSymbolTable[name].constChar;
			if (gene) {
				mipsCodeTable.push_back(mipsCode(li, regName, "", "", va));
			}
			get = true;
		}
		else {  //var
			addr = globalSymbolTable[name].addr;
			mipsCodeTable.push_back(mipsCode(lw, regName, "$gp", "", addr * 4));
		}
	}
	else {  //值
		if (name.size() > 0) {  //不能是空串
			if (gene) {
				mipsCodeTable.push_back(mipsCode(li, regName, "", "", string2int(name)));
			}
			va = string2int(name);
			get = true;
		}
	}
}

void storeValue(string name, string regName) {
	int addr;
	if (allLocalSymbolTable[curFuncName].find(name) != allLocalSymbolTable[curFuncName].end()
		&& allLocalSymbolTable[curFuncName][name].kind == 1) {
		addr = allLocalSymbolTable[curFuncName][name].addr;
		mipsCodeTable.push_back(mipsCode(sw, regName, "$fp", "", -4 * addr));
	}
	else if (globalSymbolTable.find(name) != globalSymbolTable.end()
		&& globalSymbolTable[name].kind == 1) {
		addr = globalSymbolTable[name].addr;
		mipsCodeTable.push_back(mipsCode(sw, regName, "$gp", "", addr * 4));
	}
}

void genMips() {
	mipsCodeTable.push_back(mipsCode(dataSeg, "", "", ""));  //.data
	//字符串.asciiz
	for (int i = 0; i < stringList.size(); i++) {
		mipsCodeTable.push_back(mipsCode(asciizSeg, "s_" + int2string(i), stringList[i], ""));
	}
	mipsCodeTable.push_back(mipsCode(asciizSeg, "nextLine", "\\n", ""));
	//全局变量从$gp向上加
	mipsCodeTable.push_back(mipsCode(textSeg, "", "", ""));  //.text
	mipsCodeTable.push_back(mipsCode(j, "main", "", ""));
	bool flag = false;
	int len = 0, addr = 0, va = 0, va2 = 0;
	bool get1 = false, get2 = false;
	int pushCnt = 0;
	int paramSize = 0;
	stack<midCode> pushOpStack;
	for (int i = 0; i < midCodeTable.size(); i++) {
		midCode mc = midCodeTable[i];
		midCode mcNext = mc;
		switch (mc.op) {
		case PLUSOP: {
			get1 = false;
			loadValue(mc.x, "$t0", false, va, get1);
			get2 = false;
			loadValue(mc.y, "$t1", false, va2, get2);
			if (get1 && get2) {
				mipsCodeTable.push_back(mipsCode(li, "$t2", "", "", va + va2));
			}
			else if (get1 && !get2) {
				mipsCodeTable.push_back(mipsCode(addi, "$t2", "$t1", "", va));
			}
			else if (!get1 && get2) {
				mipsCodeTable.push_back(mipsCode(addi, "$t2", "$t0", "", va2));
			}
			else {
				mipsCodeTable.push_back(mipsCode(add, "$t2", "$t0", "$t1"));
			}
			storeValue(mc.z, "$t2");
			break;
		}
		case MINUOP: {
			get1 = false;
			loadValue(mc.x, "$t0", false, va, get1);
			get2 = false;
			loadValue(mc.y, "$t1", false, va2, get2);
			if (get1 && get2) {
				mipsCodeTable.push_back(mipsCode(li, "$t2", "", "", va - va2));
			}
			else if (get1 && !get2) {  //va - $t1
				mipsCodeTable.push_back(mipsCode(addi, "$t2", "$t1", "", -va));  //$t1-va
				mipsCodeTable.push_back(mipsCode(sub, "$t2", "$0", "$t2"));      //0-$t2
			}
			else if (!get1 && get2) {  //$t0 - va2
				mipsCodeTable.push_back(mipsCode(addi, "$t2", "$t0", "", -va2));
			}
			else {
				mipsCodeTable.push_back(mipsCode(sub, "$t2", "$t0", "$t1"));
			}
			storeValue(mc.z, "$t2");
			break;
		}
		case MULTOP: {
			loadValue(mc.x, "$t0", true, va, get1);
			loadValue(mc.y, "$t1", true, va2, get2);
			mipsCodeTable.push_back(mipsCode(mult, "$t0", "$t1", ""));
			mipsCodeTable.push_back(mipsCode(mflo, "$t2", "", ""));
			storeValue(mc.z, "$t2");
			break;
		}
		case DIVOP: {
			loadValue(mc.x, "$t0", true, va, get1);
			loadValue(mc.y, "$t1", true, va2, get2);
			mipsCodeTable.push_back(mipsCode(divop, "$t0", "$t1", ""));
			mipsCodeTable.push_back(mipsCode(mflo, "$t2", "", ""));
			storeValue(mc.z, "$t2");
			break;
		}
		case LSSOP: {  //<
			get1 = false;
			loadValue(mc.x, "$t0", false, va, get1);
			get2 = false;
			loadValue(mc.y, "$t1", false, va2, get2);
			mcNext = midCodeTable[i + 1];
			if (mcNext.op == BZ) {  //0跳  x>=y跳
				if (get1 && get2) {
					if (va >= va2) {
						mipsCodeTable.push_back(mipsCode(j, mcNext.z, "", ""));
					}
				}
				else if (get1 && !get2) {  //va >= $t1
					mipsCodeTable.push_back(mipsCode(ble, "$t1", int2string(va), mcNext.z));  //ble <=跳转 代价4
				}
				else if (!get1 && get2) {  //$t0 >= va2
					mipsCodeTable.push_back(mipsCode(bge, "$t0", int2string(va2), mcNext.z));  //bge >=跳转 代价3
				}
				else {  //$t0 >= $t1
					mipsCodeTable.push_back(mipsCode(bge, "$t0", "$t1", mcNext.z));  //bge >=跳转 代价3
				}
			}
			else if (mcNext.op == BNZ) {  //1跳 x<y跳
				if (get1 && get2) {
					if (va < va2) {
						mipsCodeTable.push_back(mipsCode(j, mcNext.z, "", ""));
					}
				}
				else if (get1 && !get2) {  //va < $t1
					mipsCodeTable.push_back(mipsCode(bgt, "$t1", int2string(va), mcNext.z));  //bgt >跳转 代价4
				}
				else if (!get1 && get2) {  //$t0 < va2
					mipsCodeTable.push_back(mipsCode(blt, "$t0", int2string(va2), mcNext.z));  //blt <跳转 代价3
				}
				else { //$t0 < $t1
					mipsCodeTable.push_back(mipsCode(blt, "$t0", "$t1", mcNext.z));  //blt <跳转 代价3
				}
			}
			i++;
			break;
		}
		case LEQOP: {  //<=
			get1 = false;
			loadValue(mc.x, "$t0", false, va, get1);
			get2 = false;
			loadValue(mc.y, "$t1", false, va2, get2);
			mcNext = midCodeTable[i + 1];
			if (mcNext.op == BZ) {  //0跳  x>y跳
				if (get1 && get2) {
					if (va > va2) {
						mipsCodeTable.push_back(mipsCode(j, mcNext.z, "", ""));
					}
				}
				else if (get1 && !get2) {  //va > $t1
					mipsCodeTable.push_back(mipsCode(blt, "$t1", int2string(va), mcNext.z));  //blt <跳转 代价3
				}
				else if (!get1 && get2) {  //$t0 > va2
					mipsCodeTable.push_back(mipsCode(bgt, "$t0", int2string(va2), mcNext.z));  //bgt >跳转 代价4
				}
				else {  //$t0 > $t1
					mipsCodeTable.push_back(mipsCode(bgt, "$t0", "$t1", mcNext.z));  //bgt >跳转 代价3
				}
			}
			else if (mcNext.op == BNZ) {  //1跳 x<=y跳
				if (get1 && get2) {
					if (va <= va2) {
						mipsCodeTable.push_back(mipsCode(j, mcNext.z, "", ""));
					}
				}
				else if (get1 && !get2) {  //va <= $t1
					mipsCodeTable.push_back(mipsCode(bge, "$t1", int2string(va), mcNext.z));  //bge >=跳转 代价3
				}
				else if (!get1 && get2) {  //$t0 <= va2
					mipsCodeTable.push_back(mipsCode(ble, "$t0", int2string(va2), mcNext.z));  //ble <=跳转 代价4
				}
				else { //$t0 <= $t1
					mipsCodeTable.push_back(mipsCode(ble, "$t0", "$t1", mcNext.z));  //ble <=跳转 代价3
				}
			}
			i++;
			break;
		}
		case GREOP: {  //>
			get1 = false;
			loadValue(mc.x, "$t0", false, va, get1);
			get2 = false;
			loadValue(mc.y, "$t1", false, va2, get2);
			mcNext = midCodeTable[i + 1];
			if (mcNext.op == BZ) {  //0跳  x<=y跳
				if (get1 && get2) {
					if (va <= va2) {
						mipsCodeTable.push_back(mipsCode(j, mcNext.z, "", ""));
					}
				}
				else if (get1 && !get2) {  //va <= $t1
					mipsCodeTable.push_back(mipsCode(bge, "$t1", int2string(va), mcNext.z));  //bge >=跳转 代价3
				}
				else if (!get1 && get2) {  //$t0 <= va2
					mipsCodeTable.push_back(mipsCode(ble, "$t0", int2string(va2), mcNext.z));  //ble <=跳转 代价4
				}
				else { //$t0 <= $t1
					mipsCodeTable.push_back(mipsCode(ble, "$t0", "$t1", mcNext.z));  //ble <=跳转 代价3
				}
			}
			else if (mcNext.op == BNZ) {  //1跳 x>y跳
				if (get1 && get2) {
					if (va > va2) {
						mipsCodeTable.push_back(mipsCode(j, mcNext.z, "", ""));
					}
				}
				else if (get1 && !get2) {  //va > $t1
					mipsCodeTable.push_back(mipsCode(blt, "$t1", int2string(va), mcNext.z));  //blt <跳转 代价3
				}
				else if (!get1 && get2) {  //$t0 > va2
					mipsCodeTable.push_back(mipsCode(bgt, "$t0", int2string(va2), mcNext.z));  //bgt >跳转 代价4
				}
				else {  //$t0 > $t1
					mipsCodeTable.push_back(mipsCode(bgt, "$t0", "$t1", mcNext.z));  //bgt >跳转 代价3
				}
			}
			i++;
			break;
		}
		case GEQOP: {  //>=
			get1 = false;
			loadValue(mc.x, "$t0", false, va, get1);
			get2 = false;
			loadValue(mc.y, "$t1", false, va2, get2);
			mcNext = midCodeTable[i + 1];
			if (mcNext.op == BZ) {  //0跳  x<y跳
				if (get1 && get2) {
					if (va < va2) {
						mipsCodeTable.push_back(mipsCode(j, mcNext.z, "", ""));
					}
				}
				else if (get1 && !get2) {  //va < $t1
					mipsCodeTable.push_back(mipsCode(bgt, "$t1", int2string(va), mcNext.z));  //bgt >跳转 代价4
				}
				else if (!get1 && get2) {  //$t0 < va2
					mipsCodeTable.push_back(mipsCode(blt, "$t0", int2string(va2), mcNext.z));  //blt <跳转 代价3
				}
				else { //$t0 < $t1
					mipsCodeTable.push_back(mipsCode(blt, "$t0", "$t1", mcNext.z));  //blt <跳转 代价3
				}
			}
			else if (mcNext.op == BNZ) {  //1跳 x>=y跳
				if (get1 && get2) {
					if (va >= va2) {
						mipsCodeTable.push_back(mipsCode(j, mcNext.z, "", ""));
					}
				}
				else if (get1 && !get2) {  //va >= $t1
					mipsCodeTable.push_back(mipsCode(ble, "$t1", int2string(va), mcNext.z));  //ble <=跳转 代价4
				}
				else if (!get1 && get2) {  //$t0 >= va2
					mipsCodeTable.push_back(mipsCode(bge, "$t0", int2string(va2), mcNext.z));  //bge >=跳转 代价3
				}
				else {  //$t0 >= $t1
					mipsCodeTable.push_back(mipsCode(bge, "$t0", "$t1", mcNext.z));  //bge >=跳转 代价3
				}
			}
			i++;
			break;
		}
		case EQLOP: {
			get1 = false;
			loadValue(mc.x, "$t0", false, va, get1);
			get2 = false;
			loadValue(mc.y, "$t1", false, va2, get2);
			mcNext = midCodeTable[i + 1];
			if (mcNext.op == BZ) {  //0跳
				if (get1 && get2) {
					if (va != va2) {
						mipsCodeTable.push_back(mipsCode(j, mcNext.z, "", ""));
					}
				}
				else if (get1 && !get2) {
					mipsCodeTable.push_back(mipsCode(li, "$t0", "", "", va));
					mipsCodeTable.push_back(mipsCode(bne, "$t0", "$t1", mcNext.z));
				}
				else if (!get1 && get2) {
					mipsCodeTable.push_back(mipsCode(li, "$t1", "", "", va2));
					mipsCodeTable.push_back(mipsCode(bne, "$t0", "$t1", mcNext.z));
				}
				else {
					mipsCodeTable.push_back(mipsCode(bne, "$t0", "$t1", mcNext.z));
				}
			}
			else if (mcNext.op == BNZ) {  //1跳
				if (get1 && get2) {
					if (va == va2) {
						mipsCodeTable.push_back(mipsCode(j, mcNext.z, "", ""));
					}
				}
				else if (get1 && !get2) {
					mipsCodeTable.push_back(mipsCode(li, "$t0", "", "", va));
					mipsCodeTable.push_back(mipsCode(beq, "$t0", "$t1", mcNext.z));
				}
				else if (!get1 && get2) {
					mipsCodeTable.push_back(mipsCode(li, "$t1", "", "", va2));
					mipsCodeTable.push_back(mipsCode(beq, "$t0", "$t1", mcNext.z));
				}
				else {
					mipsCodeTable.push_back(mipsCode(beq, "$t0", "$t1", mcNext.z));
				}
			}
			i++;
			break;
		}
		case NEQOP: {
			get1 = false;
			loadValue(mc.x, "$t0", false, va, get1);
			get2 = false;
			loadValue(mc.y, "$t1", false, va2, get2);
			mcNext = midCodeTable[i + 1];
			if (mcNext.op == BZ) {  //0跳
				if (get1 && get2) {
					if (va == va2) {
						mipsCodeTable.push_back(mipsCode(j, mcNext.z, "", ""));
					}
				}
				else if (get1 && !get2) {
					mipsCodeTable.push_back(mipsCode(li, "$t0", "", "", va));
					mipsCodeTable.push_back(mipsCode(beq, "$t0", "$t1", mcNext.z));
				}
				else if (!get1 && get2) {
					mipsCodeTable.push_back(mipsCode(li, "$t1", "", "", va2));
					mipsCodeTable.push_back(mipsCode(beq, "$t0", "$t1", mcNext.z));
				}
				else {
					mipsCodeTable.push_back(mipsCode(beq, "$t0", "$t1", mcNext.z));
				}
			}
			else if (mcNext.op == BNZ) {  //1跳
				if (get1 && get2) {
					if (va != va2) {
						mipsCodeTable.push_back(mipsCode(j, mcNext.z, "", ""));
					}
				}
				else if (get1 && !get2) {
					mipsCodeTable.push_back(mipsCode(li, "$t0", "", "", va));
					mipsCodeTable.push_back(mipsCode(bne, "$t0", "$t1", mcNext.z));
				}
				else if (!get1 && get2) {
					mipsCodeTable.push_back(mipsCode(li, "$t1", "", "", va2));
					mipsCodeTable.push_back(mipsCode(bne, "$t0", "$t1", mcNext.z));
				}
				else {
					mipsCodeTable.push_back(mipsCode(bne, "$t0", "$t1", mcNext.z));
				}
			}
			i++;
			break;
		}
		case ASSIGNOP: {
			//mc.z是局部的变量 或 全局的变量
			//mc.x可能是标识符也可能是数值 $t0
			loadValue(mc.x, "$t0", true, va, get1);
			storeValue(mc.z, "$t0");
			break;
		}
		case GOTO: {
			mipsCodeTable.push_back(mipsCode(j, mc.z, "", ""));
			break;
		}
		case PUSH: {
			pushOpStack.push(mc);
			break;
		}
		case CALL: {
			paramSize = globalSymbolTable[mc.z].parameterTable.size();
			while (paramSize) {
				paramSize--;
				if (pushOpStack.empty()) {
					cout << "ERROR!!!!!!!!\n";
				}
				midCode tmpMc = pushOpStack.top();
				pushOpStack.pop();
				loadValue(tmpMc.z, "$t0", true, va, get1);
				mipsCodeTable.push_back(mipsCode(sw, "$t0", "$sp", "", -4 * paramSize));
			}
			mipsCodeTable.push_back(mipsCode(addi, "$sp", "$sp", "", -4 * globalSymbolTable[mc.z].length - 8));
			mipsCodeTable.push_back(mipsCode(sw, "$ra", "$sp", "", 4));
			mipsCodeTable.push_back(mipsCode(sw, "$fp", "$sp", "", 8));
			mipsCodeTable.push_back(mipsCode(addi, "$fp", "$sp", "", 4 * globalSymbolTable[mc.z].length + 8));
			mipsCodeTable.push_back(mipsCode(jal, mc.z, "", ""));
			mipsCodeTable.push_back(mipsCode(lw, "$fp", "$sp", "", 8));
			mipsCodeTable.push_back(mipsCode(lw, "$ra", "$sp", "", 4));
			mipsCodeTable.push_back(mipsCode(addi, "$sp", "$sp", "", 4 * globalSymbolTable[mc.z].length + 8));
			break;
		}
		case RET: {
			loadValue(mc.z, "$v0", true, va, get1);
			mipsCodeTable.push_back(mipsCode(jr, "$ra", "", ""));
			break;
		}
		case RETVALUE: {
			//mc.z 是产生的一个临时变量 需要把$v0的值赋给他
			if (allLocalSymbolTable[curFuncName].find(mc.z) != allLocalSymbolTable[curFuncName].end()
				&& allLocalSymbolTable[curFuncName][mc.z].kind == 1) {
				addr = allLocalSymbolTable[curFuncName][mc.z].addr;
				mipsCodeTable.push_back(mipsCode(sw, "$v0", "$fp", "", -4 * addr));
			}
			break;
		}
		case SCAN: {
			//mc.z是局部的变量 或 全局的变量
			if (allLocalSymbolTable[curFuncName].find(mc.z) != allLocalSymbolTable[curFuncName].end()
				&& allLocalSymbolTable[curFuncName][mc.z].kind == 1) {
				if (allLocalSymbolTable[curFuncName][mc.z].type == 1) {  //int
					mipsCodeTable.push_back(mipsCode(li, "$v0", "", "", 5));
				}
				else {  //char
					mipsCodeTable.push_back(mipsCode(li, "$v0", "", "", 12));
				}
				mipsCodeTable.push_back(mipsCode(syscall, "", "", ""));
				addr = allLocalSymbolTable[curFuncName][mc.z].addr;
				mipsCodeTable.push_back(mipsCode(sw, "$v0", "$fp", "", -4 * addr));
			}
			else if (globalSymbolTable.find(mc.z) != globalSymbolTable.end()
				&& globalSymbolTable[mc.z].kind == 1) {
				if (globalSymbolTable[mc.z].type == 1) {  //int
					mipsCodeTable.push_back(mipsCode(li, "$v0", "", "", 5));
				}
				else {  //char
					mipsCodeTable.push_back(mipsCode(li, "$v0", "", "", 12));
				}
				mipsCodeTable.push_back(mipsCode(syscall, "", "", ""));
				addr = globalSymbolTable[mc.z].addr;
				mipsCodeTable.push_back(mipsCode(sw, "$v0", "$gp", "", addr * 4));
			}
			break;
		}
		case PRINT: {
			if (mc.x[0] == '3') {  //string
				for (int i = 0; i < stringList.size(); i++) {
					if (stringList[i] == mc.z) {
						mipsCodeTable.push_back(mipsCode(la, "$a0", "s_" + int2string(i), ""));
						break;
					}
				}
				mipsCodeTable.push_back(mipsCode(li, "$v0", "", "", 4));
				mipsCodeTable.push_back(mipsCode(syscall, "", "", ""));
			}
			else if (mc.x[0] == '4') {  //换行
				mipsCodeTable.push_back(mipsCode(la, "$a0", "nextLine", ""));
				mipsCodeTable.push_back(mipsCode(li, "$v0", "", "", 4));
				mipsCodeTable.push_back(mipsCode(syscall, "", "", ""));
			}
			else { //int char
				loadValue(mc.z, "$a0", true, va, get1);
				mipsCodeTable.push_back(mipsCode(li, "$v0", "", "", mc.x[0] == '1' ? 1 : 11));
				mipsCodeTable.push_back(mipsCode(syscall, "", "", ""));
			}
			break;
		}
		case LABEL: {
			mipsCodeTable.push_back(mipsCode(label, mc.z, "", ""));
			break;
		}
		case FUNC: {
			//进入函数 首先产生标号 此时的$sp就是当前函数的栈顶
			//需要为前一个函数做jr $ra 注意第一个函数不用做
			if (flag) {
				mipsCodeTable.push_back(mipsCode(jr, "$ra", "", ""));
			}
			flag = true;
			mipsCodeTable.push_back(mipsCode(label, mc.x, "", ""));
			if (mc.x == "main") {
				len = globalSymbolTable[mc.x].length;
				mipsCodeTable.push_back(mipsCode(moveop, "$fp", "$sp", ""));
				mipsCodeTable.push_back(mipsCode(addi, "$sp", "$sp", "", -4 * len - 8));
			}
			curFuncName = mc.x;  //记录当前的函数名字
			break;
		}
		case GETARRAY: {
			//midCodefile << mc.z << " = " << mc.x << "[" << mc.y << "]\n";
			//mc.z是局部的变量 或 全局的变量
			//mc.x是数组名
			//mc.y可能是标识符也可能是数值 $t0--->数组的索引
			get1 = false;
			loadValue(mc.y, "$t0", false, va, get1);
			if (allLocalSymbolTable[curFuncName].find(mc.x) != allLocalSymbolTable[curFuncName].end()
				&& allLocalSymbolTable[curFuncName][mc.x].kind == 4) {  //array
				addr = allLocalSymbolTable[curFuncName][mc.x].addr;
				if (!get1) {
					mipsCodeTable.push_back(mipsCode(addi, "$t2", "$fp", "", -4 * addr));
					mipsCodeTable.push_back(mipsCode(sll, "$t0", "$t0", "", 2));
					mipsCodeTable.push_back(mipsCode(sub, "$t2", "$t2", "$t0"));
					mipsCodeTable.push_back(mipsCode(lw, "$t1", "$t2", "", 0));
				}
				else {
					mipsCodeTable.push_back(mipsCode(lw, "$t1", "$fp", "", -4 * (addr + va)));
				}
			}
			else if (globalSymbolTable.find(mc.x) != globalSymbolTable.end()
				&& globalSymbolTable[mc.x].kind == 4) {  //array
				addr = globalSymbolTable[mc.x].addr;
				if (!get1) {
					mipsCodeTable.push_back(mipsCode(addi, "$t2", "$gp", "", addr * 4));
					mipsCodeTable.push_back(mipsCode(sll, "$t0", "$t0", "", 2));
					mipsCodeTable.push_back(mipsCode(add, "$t2", "$t2", "$t0"));
					mipsCodeTable.push_back(mipsCode(lw, "$t1", "$t2", "", 0));
				}
				else {
					mipsCodeTable.push_back(mipsCode(lw, "$t1", "$gp", "", (addr + va) * 4));
				}
			}
			storeValue(mc.z, "$t1");
			break;
		}
		case PUTARRAY: {
			//midCodefile << mc.z << "[" << mc.x << "]" << " = " << mc.y << "\n";
			//mc.x可能是标识符也可能是数值 数组下标 $t0
			//mc.z是数组名
			//mc.y可能是标识符也可能是数值 $t1
			loadValue(mc.y, "$t1", true, va, get1);
			get1 = false;
			loadValue(mc.x, "$t0", false, va, get1);
			if (allLocalSymbolTable[curFuncName].find(mc.z) != allLocalSymbolTable[curFuncName].end()
				&& allLocalSymbolTable[curFuncName][mc.z].kind == 4) {  //array
				addr = allLocalSymbolTable[curFuncName][mc.z].addr;
				if (!get1) {
					mipsCodeTable.push_back(mipsCode(addi, "$t2", "$fp", "", -4 * addr));
					mipsCodeTable.push_back(mipsCode(sll, "$t0", "$t0", "", 2));
					mipsCodeTable.push_back(mipsCode(sub, "$t2", "$t2", "$t0"));
					mipsCodeTable.push_back(mipsCode(sw, "$t1", "$t2", "", 0));
				}
				else { //拿到了数组下标 存在了va中
					mipsCodeTable.push_back(mipsCode(sw, "$t1", "$fp", "", -4 * (addr + va)));
				}
			}
			else if (globalSymbolTable.find(mc.z) != globalSymbolTable.end()
				&& globalSymbolTable[mc.z].kind == 4) {  //array
				addr = globalSymbolTable[mc.z].addr;
				if (!get1) {
					mipsCodeTable.push_back(mipsCode(addi, "$t2", "$gp", "", addr * 4));
					mipsCodeTable.push_back(mipsCode(sll, "$t0", "$t0", "", 2));
					mipsCodeTable.push_back(mipsCode(add, "$t2", "$t2", "$t0"));
					mipsCodeTable.push_back(mipsCode(sw, "$t1", "$t2", "", 0));
				}
				else {
					mipsCodeTable.push_back(mipsCode(sw, "$t1", "$gp", "", (addr + va) * 4));
				}
			}
			break;
		}
		case EXIT: {
			mipsCodeTable.push_back(mipsCode(li, "$v0", "", "", 10));
			mipsCodeTable.push_back(mipsCode(syscall, "", "", ""));
		}
		default: {
			break;
		}
		}
	}
	mipsCodeTable.push_back(mipsCode(li, "$v0", "", "", 10));
	mipsCodeTable.push_back(mipsCode(syscall, "", "", ""));
}

void outputMipsCode() {
	for (int i = 0; i < mipsCodeTable.size(); i++) {
		mipsCode mc = mipsCodeTable[i];
		switch (mc.op) {
		case add:
			mipsCodefile << "add " << mc.z << "," << mc.x << "," << mc.y << "\n";
			break;
		case sub:
			mipsCodefile << "sub " << mc.z << "," << mc.x << "," << mc.y << "\n";
			break;
		case mult:
			mipsCodefile << "mult " << mc.z << "," << mc.x << "\n";
			break;
		case divop:
			mipsCodefile << "div " << mc.z << "," << mc.x << "\n";
			break;
		case addi:
			mipsCodefile << "addi " << mc.z << "," << mc.x << "," << mc.imme << "\n";
			break;
		case sll:
			mipsCodefile << "sll " << mc.z << "," << mc.x << "," << mc.imme << "\n";
			break;
		case mflo:
			mipsCodefile << "mflo " << mc.z << "\n";
			break;
		case mfhi:
			mipsCodefile << "mfhi " << mc.z << "\n";
		case beq:
			mipsCodefile << "beq " << mc.z << "," << mc.x << "," << mc.y << "\n";
			break;
		case bne:
			mipsCodefile << "bne " << mc.z << "," << mc.x << "," << mc.y << "\n";
			break;
		case bgt:
			mipsCodefile << "bgt " << mc.z << "," << mc.x << "," << mc.y << "\n";
			break;
		case bge:
			mipsCodefile << "bge " << mc.z << "," << mc.x << "," << mc.y << "\n";
			break;
		case blt:
			mipsCodefile << "blt " << mc.z << "," << mc.x << "," << mc.y << "\n";
			break;
		case ble:
			mipsCodefile << "ble " << mc.z << "," << mc.x << "," << mc.y << "\n";
			break;
		case j:
			mipsCodefile << "j " << mc.z << "\n";
			break;
		case jal:
			mipsCodefile << "jal " << mc.z << "\n";
			break;
		case jr:
			mipsCodefile << "jr " << mc.z << "\n";
			break;
		case lw:
			mipsCodefile << "lw " << mc.z << "," << mc.imme << "(" << mc.x << ")\n";
			break;
		case sw:
			mipsCodefile << "sw " << mc.z << "," << mc.imme << "(" << mc.x << ")\n";
			break;
		case syscall:
			mipsCodefile << "syscall\n";
			break;
		case li:
			mipsCodefile << "li " << mc.z << "," << mc.imme << "\n";
			break;
		case la:
			mipsCodefile << "la " << mc.z << "," << mc.x << "\n";
			break;
		case moveop:
			mipsCodefile << "move " << mc.z << "," << mc.x << "\n";
			break;
		case dataSeg:
			mipsCodefile << ".data\n";
			break;
		case textSeg:
			mipsCodefile << ".text\n";
			break;
		case asciizSeg:
			mipsCodefile << mc.z << ": .asciiz \"" << mc.x << "\"\n";
			break;
		case globlSeg:
			mipsCodefile << ".globl main\n";
			break;
		case label:
			mipsCodefile << mc.z << ":\n";
			break;
		default:
			break;
		}
	}
}