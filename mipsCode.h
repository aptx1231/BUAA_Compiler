#pragma once

#include <string>
using namespace std;

enum mipsOperation {
	add,
	addi,
	sub,
	mult,
	mul,
	divop,
	mflo,
	mfhi,
	sll,
	beq,
	bne,
	bgt, //扩展指令 相当于一条ALU类指令+一条branch指令
	bge, //扩展指令 相当于一条ALU类指令+一条branch指令
	blt, //扩展指令 相当于一条ALU类指令+一条branch指令
	ble, //扩展指令 相当于一条ALU类指令+一条branch指令
	j,
	jal,
	jr,
	lw,
	sw,
	syscall,
	li,
	la,
	moveop,
	dataSeg,  //.data
	textSeg,  //.text
	asciizSeg,  //.asciiz
	globlSeg,  //.globl
	label,  //产生标号
};

class mipsCode {
public:
	mipsOperation op; // 操作
	string z;     // 结果
	string x;     // 左操作数
	string y;     // 右操作数
	int imme;     // 立即数
	mipsCode(mipsOperation o, string zz, string xx, string yy, int i = 0) : op(o), z(zz), x(xx), y(yy), imme(i) {}
};

void genMips();

void outputMipsCode();

void loadValue(string& name, string& regName, bool gene, int& va, bool& get, bool assign);

void storeValue(string &name, string &regName);
