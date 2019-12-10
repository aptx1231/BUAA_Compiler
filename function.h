#pragma once

#include <string>
using namespace std;

void showGlobal();

void showLocal();

void showAll();

void showString();

string int2string(int t);  //修改

int string2int(string s);  //修改

string genLabel(string app="");

string genTmp();

string genName();

void showFuncMidCode();
