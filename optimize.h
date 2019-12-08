#pragma once
#include <vector>
#include "midCode.h"
using namespace std;

class Block {
public:
	int start;
	int end;
	int nextBlock1;
	int nextBlock2;
	vector<midCode> midCodeVector;

	Block(int s, int e, int n1, int n2) : start(s), end(e), nextBlock1(n1), nextBlock2(n2) {}

	void setnextBlock1(int n1) {
		nextBlock1 = n1;
	}

	void setnextBlock2(int n2) {
		nextBlock2 = n2;
	}

	void insert(midCode mc) {
		midCodeVector.push_back(mc);
	}

	void output() {
		cout << start << " " << end << " " << nextBlock1 << " " << nextBlock2 << "\n";
		for (int i = 0; i < midCodeVector.size(); i++) {
			cout << "(" << start + i << ") ";
			midCode mc = midCodeVector[i];
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
	}
};

void splitBlock();

void showFuncBlock();