#include <iostream>
#include <fstream>
#include <vector>
#include "midCode.h"
using namespace std;

extern vector<midCode> midCodeTable;
extern ofstream midCodefile;

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
			midCodefile << mc.z << " = " << mc.x << "\n";
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
			midCodefile << "PUSH " << mc.z << "(" << mc.y << ")" << "\n";
			break;
		case CALL:
			midCodefile << "CALL " << mc.z << "\n";
			break;
		case RET:
			midCodefile << "RET " << mc.z << "\n";
			break;
		case INLINERET:
			midCodefile << "INLINERET " << mc.z << "\n";
			break;
		case RETVALUE:
			midCodefile << "RETVALUE " << mc.z << " = " << mc.x << "\n";
			break;
		case SCAN:
			midCodefile << "SCAN " << mc.z << "\n";
			break;
		case PRINT:
			midCodefile << "PRINT " << mc.z << " " << mc.x << "\n";
			break;
		case LABEL:
			midCodefile << mc.z << ": \n";
			break;
			/*case CONST:
				midCodefile << "CONST " << mc.z << " " << mc.x << " = " << mc.y << endl;
				break;
			case ARRAY:
				midCodefile << "ARRAY " << mc.z << " " << mc.x << "[" << mc.y << "]" << endl;
				break;
			case VAR:
				midCodefile << "VAR " << mc.z << " " << mc.x << endl;
				break;*/
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
		case EXIT:
			midCodefile << "EXIT\n";
			break;
		default:
			break;
		}
	}
}