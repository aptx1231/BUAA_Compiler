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
int tRegBusy[10] = {0,};  //有t3-t9共7个临时寄存器供分配 用于记录临时寄存器是否被占用
string tRegContent[10];   //记录每一个临时寄存器分配给了哪一个中间变量 #T0,#T1...
int debug = 1;

extern ofstream mipsCodefile;
extern vector<string> stringList;  //保存所有的字符串
extern vector<midCode> midCodeTable;
extern map<string, symbolItem> globalSymbolTable;
extern map<string, map<string, symbolItem>> allLocalSymbolTable;  //保存所有的局部符号表 用于保留变量的地址

int findEmptyTReg() {  //查找空闲的t寄存器
	for (int i = 3; i <= 9; i++) {
		if (!tRegBusy[i]) {  //找到了空闲寄存器
			return i;
		}
	}
	return -1; //没找到
}

int findNameHaveTReg(string& name) {  //判断当前中间变量是否被分配了t寄存器
	for (int i = 3; i <= 9; i++) {
		if (tRegBusy[i] && tRegContent[i]==name) {  //被占用的寄存器 存储着name
			return i;
		}
	}
	return -1; //没有被分配寄存器 需要lw取
}

void loadValue(string& name, string& regName, bool gene, int& va, bool& get) {
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
		else {  //var 局部变量或者中间变量
			if (name[0] == '#') {  //name是中间变量 看他是否被分配了寄存器
				int find = findNameHaveTReg(name);  //看name是否被分配了寄存器
				if (find != -1) {  //被分配了寄存器 直接用寄存器的值
					regName = "$t" + int2string(find);
					tRegBusy[find] = 0;  //取消标记
					tRegContent[find] = "";  //取消内容
					if (debug) {
						cout << "del " << regName << " = " << name << "\n";
					}
				}
				else {  //没有寄存器 需要lw
					addr = allLocalSymbolTable[curFuncName][name].addr;
					mipsCodeTable.push_back(mipsCode(lw, regName, "$fp", "", -4 * addr));
				}
			}
			else {  //name是局部变量
				addr = allLocalSymbolTable[curFuncName][name].addr;
				mipsCodeTable.push_back(mipsCode(lw, regName, "$fp", "", -4 * addr));
			}
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
		else {  //var 全局变量
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

void storeValue(string &name, string &regName) {
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
			string sx = "$t0", sy = "$t1", sz = "$t2";
			get1 = false;
			loadValue(mc.x, sx, false, va, get1);
			get2 = false;
			loadValue(mc.y, sy, false, va2, get2);
			int find;
			if (mc.z[0] == '#') {  //mc.z是中间变量 分配t寄存器
				find = findEmptyTReg();
				if (find != -1) {  //有空闲
					tRegBusy[find] = 1;  //打标记
					tRegContent[find] = mc.z;  //find这个寄存器保存了mc.z的值
					sz = "$t" + int2string(find);  //sz修改成$t(find) 直接给他赋值 而不需要move sz, $t(find)
					if (debug) {
						cout << sz << " = " << mc.z << "\n";
					}
				}
			}
			if (get1 && get2) {
				mipsCodeTable.push_back(mipsCode(li, sz, "", "", va + va2));
			}
			else if (get1 && !get2) {
				mipsCodeTable.push_back(mipsCode(addi, sz, sy, "", va));
			}
			else if (!get1 && get2) {
				mipsCodeTable.push_back(mipsCode(addi, sz, sx, "", va2));
			}
			else {
				mipsCodeTable.push_back(mipsCode(add, sz, sx, sy));
			}
			if (mc.z[0] == '#') {  //mc.z是中间变量
				if (find == -1) {  //没有空闲寄存器
					storeValue(mc.z, sz);
				} //有空闲寄存器的话 已经直接保存到它里边了
			}
			else {
				storeValue(mc.z, sz);
			}
			break;
		}
		case MINUOP: {
			string sx = "$t0", sy = "$t1", sz = "$t2";
			get1 = false;
			loadValue(mc.x, sx, false, va, get1);
			get2 = false;
			loadValue(mc.y, sy, false, va2, get2);
			int find;
			if (mc.z[0] == '#') {  //mc.z是中间变量 分配t寄存器
				find = findEmptyTReg();
				if (find != -1) {  //有空闲
					tRegBusy[find] = 1;  //打标记
					tRegContent[find] = mc.z;  //find这个寄存器保存了mc.z的值
					sz = "$t" + int2string(find);  //sz修改成$t(find) 直接给他赋值 而不需要move sz, $t(find)
					if (debug) {
						cout << sz << " = " << mc.z << "\n";
					}
				}
			}
			if (get1 && get2) {
				mipsCodeTable.push_back(mipsCode(li, sz, "", "", va - va2));
			}
			else if (get1 && !get2) {  //va - $t1
				mipsCodeTable.push_back(mipsCode(addi, sz, sy, "", -va));  //$t1-va
				mipsCodeTable.push_back(mipsCode(sub, sz, "$0", sz));      //0-$t2
			}
			else if (!get1 && get2) {  //$t0 - va2
				mipsCodeTable.push_back(mipsCode(addi, sz, sx, "", -va2));
			}
			else {
				mipsCodeTable.push_back(mipsCode(sub, sz, sx, sy));
			}
			if (mc.z[0] == '#') {  //mc.z是中间变量
				if (find == -1) {  //没有空闲寄存器
					storeValue(mc.z, sz);
				} //有空闲寄存器的话 已经直接保存到它里边了
			}
			else {
				storeValue(mc.z, sz);
			}
			break;
		}
		case MULTOP: {
			string sx = "$t0", sy = "$t1", sz = "$t2";
			get1 = false;
			loadValue(mc.x, sx, false, va, get1);
			get2 = false;
			loadValue(mc.y, sy, false, va2, get2);
			int find;
			if (mc.z[0] == '#') {  //mc.z是中间变量 分配t寄存器
				find = findEmptyTReg();
				if (find != -1) {  //有空闲
					tRegBusy[find] = 1;  //打标记
					tRegContent[find] = mc.z;  //find这个寄存器保存了mc.z的值
					sz = "$t" + int2string(find);  //sz修改成$t(find) 直接给他赋值 而不需要move sz, $t(find)
					if (debug) {
						cout << sz << " = " << mc.z << "\n";
					}
				}
			}
			if (get1 && get2) {
				mipsCodeTable.push_back(mipsCode(li, sz, "", "", va * va2));
			}
			else if (get1 && !get2) {
				if (va == 1) {  //sz=1*sy=sy
					mipsCodeTable.push_back(mipsCode(moveop, sz, sy, ""));
				}
				else if (va == 0) {  //sz=0*sy=0
					mipsCodeTable.push_back(mipsCode(li, sz, "", "", 0));
				}
				else {
					mipsCodeTable.push_back(mipsCode(li, sx, "", "", va));
					mipsCodeTable.push_back(mipsCode(mul, sz, sx, sy));
				}
			}
			else if (!get1 && get2) {
				if (va2 == 1) {  //sz=sx*1=sx
					mipsCodeTable.push_back(mipsCode(moveop, sz, sx, ""));
				}
				else if (va2 == 0) {  //sz=sx*0=0
					mipsCodeTable.push_back(mipsCode(li, sz, "", "", 0));
				}
				else {
					mipsCodeTable.push_back(mipsCode(li, sy, "", "", va2));
					mipsCodeTable.push_back(mipsCode(mul, sz, sx, sy));
				}
			}
			else {
				mipsCodeTable.push_back(mipsCode(mul, sz, sx, sy));
			}
			if (mc.z[0] == '#') {  //mc.z是中间变量
				if (find == -1) {  //没有空闲寄存器
					storeValue(mc.z, sz);
				} //有空闲寄存器的话 已经直接保存到它里边了
			}
			else {
				storeValue(mc.z, sz);
			}
			break;
		}
		case DIVOP: {
			string sx = "$t0", sy = "$t1", sz = "$t2";
			get1 = false;
			loadValue(mc.x, sx, false, va, get1);
			get2 = false;
			loadValue(mc.y, sy, false, va2, get2);
			int find;
			if (mc.z[0] == '#') {  //mc.z是中间变量 分配t寄存器
				find = findEmptyTReg();
				if (find != -1) {  //有空闲
					tRegBusy[find] = 1;  //打标记
					tRegContent[find] = mc.z;  //find这个寄存器保存了mc.z的值
					sz = "$t" + int2string(find);  //sz修改成$t(find) 直接给他赋值 而不需要move sz, $t(find)
					if (debug) {
						cout << sz << " = " << mc.z << "\n";
					}
				}
			}
			if (get1 && get2) {
				mipsCodeTable.push_back(mipsCode(li, sz, "", "", va / va2));
			}
			else if (get1 && !get2) {
				if (va == 0) {  //sz=0/sy=0
					mipsCodeTable.push_back(mipsCode(li, sz, "", "", 0));
				}
				else {
					mipsCodeTable.push_back(mipsCode(li, sx, "", "", va));
					mipsCodeTable.push_back(mipsCode(divop, sx, sy, ""));
					mipsCodeTable.push_back(mipsCode(mflo, sz, "", ""));
				}
			}
			else if (!get1 && get2) {
				if (va2 == 1) {  //sz=sx/1=sx
					mipsCodeTable.push_back(mipsCode(moveop, sz, sx, ""));
				}
				else {
					mipsCodeTable.push_back(mipsCode(li, sy, "", "", va2));
					mipsCodeTable.push_back(mipsCode(divop, sx, sy, ""));
					mipsCodeTable.push_back(mipsCode(mflo, sz, "", ""));
				}
			}
			else {
				mipsCodeTable.push_back(mipsCode(divop, sx, sy, ""));
				mipsCodeTable.push_back(mipsCode(mflo, sz, "", ""));
			}
			if (mc.z[0] == '#') {  //mc.z是中间变量
				if (find == -1) {  //没有空闲寄存器
					storeValue(mc.z, sz);
				} //有空闲寄存器的话 已经直接保存到它里边了
			}
			else {
				storeValue(mc.z, sz);
			}
			break;
		}
		case LSSOP: {  //<
			string sx = "$t0", sy = "$t1";
			get1 = false;
			loadValue(mc.x, sx, false, va, get1);
			get2 = false;
			loadValue(mc.y, sy, false, va2, get2);
			mcNext = midCodeTable[i + 1];
			if (mcNext.op == BZ) {  //0跳  x>=y跳
				if (get1 && get2) {
					if (va >= va2) {
						mipsCodeTable.push_back(mipsCode(j, mcNext.z, "", ""));
					}
				}
				else if (get1 && !get2) {  //va >= $t1
					mipsCodeTable.push_back(mipsCode(ble, sy, int2string(va), mcNext.z));  //ble <=跳转 代价4
				}
				else if (!get1 && get2) {  //$t0 >= va2
					mipsCodeTable.push_back(mipsCode(bge, sx, int2string(va2), mcNext.z));  //bge >=跳转 代价3
				}
				else {  //$t0 >= $t1
					mipsCodeTable.push_back(mipsCode(bge, sx, sy, mcNext.z));  //bge >=跳转 代价3
				}
			}
			else if (mcNext.op == BNZ) {  //1跳 x<y跳
				if (get1 && get2) {
					if (va < va2) {
						mipsCodeTable.push_back(mipsCode(j, mcNext.z, "", ""));
					}
				}
				else if (get1 && !get2) {  //va < $t1
					mipsCodeTable.push_back(mipsCode(bgt, sy, int2string(va), mcNext.z));  //bgt >跳转 代价4
				}
				else if (!get1 && get2) {  //$t0 < va2
					mipsCodeTable.push_back(mipsCode(blt, sx, int2string(va2), mcNext.z));  //blt <跳转 代价3
				}
				else { //$t0 < $t1
					mipsCodeTable.push_back(mipsCode(blt, sx, sy, mcNext.z));  //blt <跳转 代价3
				}
			}
			i++;
			break;
		}
		case LEQOP: {  //<=
			string sx = "$t0", sy = "$t1";
			get1 = false;
			loadValue(mc.x, sx, false, va, get1);
			get2 = false;
			loadValue(mc.y, sy, false, va2, get2);
			mcNext = midCodeTable[i + 1];
			if (mcNext.op == BZ) {  //0跳  x>y跳
				if (get1 && get2) {
					if (va > va2) {
						mipsCodeTable.push_back(mipsCode(j, mcNext.z, "", ""));
					}
				}
				else if (get1 && !get2) {  //va > $t1
					mipsCodeTable.push_back(mipsCode(blt, sy, int2string(va), mcNext.z));  //blt <跳转 代价3
				}
				else if (!get1 && get2) {  //$t0 > va2
					mipsCodeTable.push_back(mipsCode(bgt, sx, int2string(va2), mcNext.z));  //bgt >跳转 代价4
				}
				else {  //$t0 > $t1
					mipsCodeTable.push_back(mipsCode(bgt, sx, sy, mcNext.z));  //bgt >跳转 代价3
				}
			}
			else if (mcNext.op == BNZ) {  //1跳 x<=y跳
				if (get1 && get2) {
					if (va <= va2) {
						mipsCodeTable.push_back(mipsCode(j, mcNext.z, "", ""));
					}
				}
				else if (get1 && !get2) {  //va <= $t1
					mipsCodeTable.push_back(mipsCode(bge, sy, int2string(va), mcNext.z));  //bge >=跳转 代价3
				}
				else if (!get1 && get2) {  //$t0 <= va2
					mipsCodeTable.push_back(mipsCode(ble, sx, int2string(va2), mcNext.z));  //ble <=跳转 代价4
				}
				else { //$t0 <= $t1
					mipsCodeTable.push_back(mipsCode(ble, sx, sy, mcNext.z));  //ble <=跳转 代价3
				}
			}
			i++;
			break;
		}
		case GREOP: {  //>
			string sx = "$t0", sy = "$t1";
			get1 = false;
			loadValue(mc.x, sx, false, va, get1);
			get2 = false;
			loadValue(mc.y, sy, false, va2, get2);
			mcNext = midCodeTable[i + 1];
			if (mcNext.op == BZ) {  //0跳  x<=y跳
				if (get1 && get2) {
					if (va <= va2) {
						mipsCodeTable.push_back(mipsCode(j, mcNext.z, "", ""));
					}
				}
				else if (get1 && !get2) {  //va <= $t1
					mipsCodeTable.push_back(mipsCode(bge, sy, int2string(va), mcNext.z));  //bge >=跳转 代价3
				}
				else if (!get1 && get2) {  //$t0 <= va2
					mipsCodeTable.push_back(mipsCode(ble, sx, int2string(va2), mcNext.z));  //ble <=跳转 代价4
				}
				else { //$t0 <= $t1
					mipsCodeTable.push_back(mipsCode(ble, sx, sy, mcNext.z));  //ble <=跳转 代价3
				}
			}
			else if (mcNext.op == BNZ) {  //1跳 x>y跳
				if (get1 && get2) {
					if (va > va2) {
						mipsCodeTable.push_back(mipsCode(j, mcNext.z, "", ""));
					}
				}
				else if (get1 && !get2) {  //va > $t1
					mipsCodeTable.push_back(mipsCode(blt, sy, int2string(va), mcNext.z));  //blt <跳转 代价3
				}
				else if (!get1 && get2) {  //$t0 > va2
					mipsCodeTable.push_back(mipsCode(bgt, sx, int2string(va2), mcNext.z));  //bgt >跳转 代价4
				}
				else {  //$t0 > $t1
					mipsCodeTable.push_back(mipsCode(bgt, sx, sy, mcNext.z));  //bgt >跳转 代价3
				}
			}
			i++;
			break;
		}
		case GEQOP: {  //>=
			string sx = "$t0", sy = "$t1";
			get1 = false;
			loadValue(mc.x, sx, false, va, get1);
			get2 = false;
			loadValue(mc.y, sy, false, va2, get2);
			mcNext = midCodeTable[i + 1];
			if (mcNext.op == BZ) {  //0跳  x<y跳
				if (get1 && get2) {
					if (va < va2) {
						mipsCodeTable.push_back(mipsCode(j, mcNext.z, "", ""));
					}
				}
				else if (get1 && !get2) {  //va < $t1
					mipsCodeTable.push_back(mipsCode(bgt, sy, int2string(va), mcNext.z));  //bgt >跳转 代价4
				}
				else if (!get1 && get2) {  //$t0 < va2
					mipsCodeTable.push_back(mipsCode(blt, sx, int2string(va2), mcNext.z));  //blt <跳转 代价3
				}
				else { //$t0 < $t1
					mipsCodeTable.push_back(mipsCode(blt, sx, sy, mcNext.z));  //blt <跳转 代价3
				}
			}
			else if (mcNext.op == BNZ) {  //1跳 x>=y跳
				if (get1 && get2) {
					if (va >= va2) {
						mipsCodeTable.push_back(mipsCode(j, mcNext.z, "", ""));
					}
				}
				else if (get1 && !get2) {  //va >= $t1
					mipsCodeTable.push_back(mipsCode(ble, sy, int2string(va), mcNext.z));  //ble <=跳转 代价4
				}
				else if (!get1 && get2) {  //$t0 >= va2
					mipsCodeTable.push_back(mipsCode(bge, sx, int2string(va2), mcNext.z));  //bge >=跳转 代价3
				}
				else {  //$t0 >= $t1
					mipsCodeTable.push_back(mipsCode(bge, sx, sy, mcNext.z));  //bge >=跳转 代价3
				}
			}
			i++;
			break;
		}
		case EQLOP: {
			string sx = "$t0", sy = "$t1";
			get1 = false;
			loadValue(mc.x, sx, false, va, get1);
			get2 = false;
			loadValue(mc.y, sy, false, va2, get2);
			mcNext = midCodeTable[i + 1];
			if (mcNext.op == BZ) {  //0跳
				if (get1 && get2) {
					if (va != va2) {
						mipsCodeTable.push_back(mipsCode(j, mcNext.z, "", ""));
					}
				}
				else if (get1 && !get2) {
					mipsCodeTable.push_back(mipsCode(li, sx, "", "", va));
					mipsCodeTable.push_back(mipsCode(bne, sx, sy, mcNext.z));
				}
				else if (!get1 && get2) {
					mipsCodeTable.push_back(mipsCode(li, sy, "", "", va2));
					mipsCodeTable.push_back(mipsCode(bne, sx, sy, mcNext.z));
				}
				else {
					mipsCodeTable.push_back(mipsCode(bne, sx, sy, mcNext.z));
				}
			}
			else if (mcNext.op == BNZ) {  //1跳
				if (get1 && get2) {
					if (va == va2) {
						mipsCodeTable.push_back(mipsCode(j, mcNext.z, "", ""));
					}
				}
				else if (get1 && !get2) {
					mipsCodeTable.push_back(mipsCode(li, sx, "", "", va));
					mipsCodeTable.push_back(mipsCode(beq, sx, sy, mcNext.z));
				}
				else if (!get1 && get2) {
					mipsCodeTable.push_back(mipsCode(li, sy, "", "", va2));
					mipsCodeTable.push_back(mipsCode(beq, sx, sy, mcNext.z));
				}
				else {
					mipsCodeTable.push_back(mipsCode(beq, sx, sy, mcNext.z));
				}
			}
			i++;
			break;
		}
		case NEQOP: {
			string sx = "$t0", sy = "$t1";
			get1 = false;
			loadValue(mc.x, sx, false, va, get1);
			get2 = false;
			loadValue(mc.y, sy, false, va2, get2);
			mcNext = midCodeTable[i + 1];
			if (mcNext.op == BZ) {  //0跳
				if (get1 && get2) {
					if (va == va2) {
						mipsCodeTable.push_back(mipsCode(j, mcNext.z, "", ""));
					}
				}
				else if (get1 && !get2) {
					mipsCodeTable.push_back(mipsCode(li, sx, "", "", va));
					mipsCodeTable.push_back(mipsCode(beq, sx, sy, mcNext.z));
				}
				else if (!get1 && get2) {
					mipsCodeTable.push_back(mipsCode(li, sy, "", "", va2));
					mipsCodeTable.push_back(mipsCode(beq, sx, sy, mcNext.z));
				}
				else {
					mipsCodeTable.push_back(mipsCode(beq, sx, sy, mcNext.z));
				}
			}
			else if (mcNext.op == BNZ) {  //1跳
				if (get1 && get2) {
					if (va != va2) {
						mipsCodeTable.push_back(mipsCode(j, mcNext.z, "", ""));
					}
				}
				else if (get1 && !get2) {
					mipsCodeTable.push_back(mipsCode(li, sx, "", "", va));
					mipsCodeTable.push_back(mipsCode(bne, sx, sy, mcNext.z));
				}
				else if (!get1 && get2) {
					mipsCodeTable.push_back(mipsCode(li, sy, "", "", va2));
					mipsCodeTable.push_back(mipsCode(bne, sx, sy, mcNext.z));
				}
				else {
					mipsCodeTable.push_back(mipsCode(bne, sx, sy, mcNext.z));
				}
			}
			i++;
			break;
		}
		case ASSIGNOP: {
			//mc.z是局部的变量 或 全局的变量
			//mc.x可能是标识符也可能是数值 $t0
			string sx = "$t0", sz="";
			loadValue(mc.x, sx, true, va, get1);
			if (mc.z[0] == '#') {  //mc.z是中间变量 分配t寄存器
				int find = findEmptyTReg();
				if (find != -1) {  //有空闲
					tRegBusy[find] = 1;  //打标记
					tRegContent[find] = mc.z;  //find这个寄存器保存了mc.z的值
					sz = "$t" + int2string(find);
					mipsCodeTable.push_back(mipsCode(moveop, sz, sx, ""));
					if (debug) {
						cout << sz << " = " << mc.z << "\n";
					}
				}
				else {  //没有空闲的寄存器
					storeValue(mc.z, sx);
				}
			}
			else {
				storeValue(mc.z, sx);
			}
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
			int tRegNum = 0;
			for (int i = 3; i <= 9; i++) {
				if (tRegBusy[i]) {
					mipsCodeTable.push_back(mipsCode(sw, "$t" + int2string(i), "$sp", "", -4 * tRegNum));
					tRegNum++;
				}
			}
			string sx = "$t0";
			paramSize = globalSymbolTable[mc.z].parameterTable.size();
			while (paramSize) {
				paramSize--;
				if (pushOpStack.empty()) {
					cout << "ERROR!!!!!!!!\n";
				}
				midCode tmpMc = pushOpStack.top();
				pushOpStack.pop();
				loadValue(tmpMc.z, sx, true, va, get1);
				mipsCodeTable.push_back(mipsCode(sw, sx, "$sp", "", -4 * paramSize - 4 * tRegNum));
			}
			mipsCodeTable.push_back(mipsCode(addi, "$sp", "$sp", "", -4 * globalSymbolTable[mc.z].length - 4 * tRegNum - 8));
			mipsCodeTable.push_back(mipsCode(sw, "$ra", "$sp", "", 4));
			mipsCodeTable.push_back(mipsCode(sw, "$fp", "$sp", "", 8));
			mipsCodeTable.push_back(mipsCode(addi, "$fp", "$sp", "", 4 * globalSymbolTable[mc.z].length + 8));
			mipsCodeTable.push_back(mipsCode(jal, mc.z, "", ""));
			mipsCodeTable.push_back(mipsCode(lw, "$fp", "$sp", "", 8));
			mipsCodeTable.push_back(mipsCode(lw, "$ra", "$sp", "", 4));
			mipsCodeTable.push_back(mipsCode(addi, "$sp", "$sp", "", 4 * globalSymbolTable[mc.z].length + 4 * tRegNum + 8));
			tRegNum = 0;
			for (int i = 3; i <= 9; i++) {
				if (tRegBusy[i]) {
					mipsCodeTable.push_back(mipsCode(lw, "$t" + int2string(i), "$sp", "", -4 * tRegNum));
					tRegNum++;
				}
			}
			break;
		}
		case RET: {
			string sv = "$v0";
			loadValue(mc.z, sv, true, va, get1); 
			//这里只需要单纯的给v0赋值 如果mc.z是被分配了寄存器的中间变量
			//sv就会被修改为mc.z的那个寄存器 但是这时v0没有被赋值，只是知道了mc.z的值保存在寄存器sv中
			if (sv != "$v0") {
				mipsCodeTable.push_back(mipsCode(moveop, "$v0", sv, ""));
			}
			mipsCodeTable.push_back(mipsCode(jr, "$ra", "", ""));
			break;
		}
		case RETVALUE: {
			//mc.z 是产生的一个中间变量 需要把$v0的值赋给他 尝试分配寄存器
			if (mc.z[0] == '#') {  //mc.z是中间变量 分配t寄存器
				int find = findEmptyTReg();
				if (find != -1) {  //有空闲
					tRegBusy[find] = 1;  //打标记
					tRegContent[find] = mc.z;  //find这个寄存器保存了mc.z的值
					string sz = "$t" + int2string(find);
					mipsCodeTable.push_back(mipsCode(moveop, sz, "$v0", ""));
					if (debug) {
						cout << sz << " = " << mc.z << "\n";
					}
				}
				else {  //没有空闲寄存器
					if (allLocalSymbolTable[curFuncName].find(mc.z) != allLocalSymbolTable[curFuncName].end()
						&& allLocalSymbolTable[curFuncName][mc.z].kind == 1) {
						addr = allLocalSymbolTable[curFuncName][mc.z].addr;
						mipsCodeTable.push_back(mipsCode(sw, "$v0", "$fp", "", -4 * addr));
					}
				}
			}
			else {
				if (allLocalSymbolTable[curFuncName].find(mc.z) != allLocalSymbolTable[curFuncName].end()
					&& allLocalSymbolTable[curFuncName][mc.z].kind == 1) {
					addr = allLocalSymbolTable[curFuncName][mc.z].addr;
					mipsCodeTable.push_back(mipsCode(sw, "$v0", "$fp", "", -4 * addr));
				}
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
				string sa = "$a0";
				loadValue(mc.z, sa, true, va, get1);
				//这里只需要单纯的给a0赋值 如果mc.z是被分配了寄存器的中间变量
				//sa就会被修改为mc.z的那个寄存器 但是这时a0没有被赋值，只是知道了mc.z的值保存在寄存器sa中
				if (sa != "$a0") {
					mipsCodeTable.push_back(mipsCode(moveop, "$a0", sa, ""));
				}
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
			string sy = "$t0", sz = "$t1";
			//midCodefile << mc.z << " = " << mc.x << "[" << mc.y << "]\n";
			//mc.z是局部的变量 或 全局的变量
			//mc.x是数组名
			//mc.y可能是标识符也可能是数值 $t0--->数组的索引
			get1 = false;
			loadValue(mc.y, sy, false, va, get1);
			int find;
			if (mc.z[0] == '#') {  //mc.z是中间变量 分配t寄存器
				find = findEmptyTReg();
				if (find != -1) {  //有空闲
					tRegBusy[find] = 1;  //打标记
					tRegContent[find] = mc.z;  //find这个寄存器保存了mc.z的值
					sz = "$t" + int2string(find);  //sz修改成$t(find) 直接给他赋值 而不需要move sz, $t(find)
					if (debug) {
						cout << sz << " = " << mc.z << "\n";
					}
				}
			}
			if (allLocalSymbolTable[curFuncName].find(mc.x) != allLocalSymbolTable[curFuncName].end()
				&& allLocalSymbolTable[curFuncName][mc.x].kind == 4) {  //array
				addr = allLocalSymbolTable[curFuncName][mc.x].addr;
				if (!get1) {  //数组下标保存在sy寄存器
					mipsCodeTable.push_back(mipsCode(addi, "$t2", "$fp", "", -4 * addr));
					mipsCodeTable.push_back(mipsCode(sll, sy, sy, "", 2));
					mipsCodeTable.push_back(mipsCode(sub, "$t2", "$t2", sy));
					mipsCodeTable.push_back(mipsCode(lw, sz, "$t2", "", 0));
				}
				else {
					mipsCodeTable.push_back(mipsCode(lw, sz, "$fp", "", -4 * (addr + va)));
				}
			}
			else if (globalSymbolTable.find(mc.x) != globalSymbolTable.end()
				&& globalSymbolTable[mc.x].kind == 4) {  //array
				addr = globalSymbolTable[mc.x].addr;
				if (!get1) {  //数组下标保存在sy寄存器
					mipsCodeTable.push_back(mipsCode(addi, "$t2", "$gp", "", addr * 4));
					mipsCodeTable.push_back(mipsCode(sll, sy, sy, "", 2));
					mipsCodeTable.push_back(mipsCode(add, "$t2", "$t2", sy));
					mipsCodeTable.push_back(mipsCode(lw, sz, "$t2", "", 0));
				}
				else {
					mipsCodeTable.push_back(mipsCode(lw, sz, "$gp", "", (addr + va) * 4));
				}
			}
			if (mc.z[0] == '#') {  //mc.z是中间变量
				if (find == -1) {  //没有空闲寄存器
					storeValue(mc.z, sz);
				} //有空闲寄存器的话 已经直接保存到它里边了
			}
			else {
				storeValue(mc.z, sz);
			}
			break;
		}
		case PUTARRAY: {
			string sx = "$t0", sy = "$t1";
			//midCodefile << mc.z << "[" << mc.x << "]" << " = " << mc.y << "\n";
			//mc.x可能是标识符也可能是数值 数组下标 $t0
			//mc.z是数组名
			//mc.y可能是标识符也可能是数值 $t1
			loadValue(mc.y, sy, true, va, get1);
			get1 = false;
			loadValue(mc.x, sx, false, va, get1);
			if (allLocalSymbolTable[curFuncName].find(mc.z) != allLocalSymbolTable[curFuncName].end()
				&& allLocalSymbolTable[curFuncName][mc.z].kind == 4) {  //array
				addr = allLocalSymbolTable[curFuncName][mc.z].addr;
				if (!get1) {  //数组下标保存在sx寄存器
					mipsCodeTable.push_back(mipsCode(addi, "$t2", "$fp", "", -4 * addr));
					mipsCodeTable.push_back(mipsCode(sll, sx, sx, "", 2));
					mipsCodeTable.push_back(mipsCode(sub, "$t2", "$t2", sx));
					mipsCodeTable.push_back(mipsCode(sw, sy, "$t2", "", 0));
				}
				else { //拿到了数组下标 存在了va中
					mipsCodeTable.push_back(mipsCode(sw, sy, "$fp", "", -4 * (addr + va)));
				}
			}
			else if (globalSymbolTable.find(mc.z) != globalSymbolTable.end()
				&& globalSymbolTable[mc.z].kind == 4) {  //array
				addr = globalSymbolTable[mc.z].addr;
				if (!get1) {  //数组下标保存在sx寄存器
					mipsCodeTable.push_back(mipsCode(addi, "$t2", "$gp", "", addr * 4));
					mipsCodeTable.push_back(mipsCode(sll, sx, sx, "", 2));
					mipsCodeTable.push_back(mipsCode(add, "$t2", "$t2", sx));
					mipsCodeTable.push_back(mipsCode(sw, sy, "$t2", "", 0));
				}
				else {
					mipsCodeTable.push_back(mipsCode(sw, sy, "$gp", "", (addr + va) * 4));
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
		case mul:
			mipsCodefile << "mul " << mc.z << "," << mc.x << "," << mc.y << "\n";
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