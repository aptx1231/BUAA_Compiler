#pragma once
#ifndef GRAMMER_H
#define GRAMMER_H

#include <string>
#include <vector>
#include <map>
#include "midCode.h"
using namespace std;

//＜字符串＞   ::=  "｛十进制编码为32,33,35-126的ASCII字符｝"
bool strings();
//＜程序＞  ::= ［＜常量说明＞］［＜变量说明＞］{＜有返回值函数定义＞|＜无返回值函数定义＞}＜主函数＞
bool procedure();
//＜常量说明＞ ::=  const＜常量定义＞;{ const＜常量定义＞;}
bool constDeclaration(bool isglobal);
//＜常量定义＞ ::= int＜标识符＞＝＜整数＞{,＜标识符＞＝＜整数＞}
//                  | char＜标识符＞＝＜字符＞{,＜标识符＞＝＜字符＞}
bool constDefinition(bool isglobal);
//＜无符号整数＞  ::= ＜非零数字＞｛＜数字＞｝| 0
bool unsignedInteger(int& value);
//＜整数＞ ::= ［＋｜－］＜无符号整数＞
bool integer(int& value);
//＜声明头部＞   ::=  int＜标识符＞ |char＜标识符＞
bool declarationHead(string& tmp, int& type);
//＜变量说明＞  ::= ＜变量定义＞;{＜变量定义＞;}
bool variableDeclaration(bool isglobal);
//＜变量定义＞  ::= ＜类型标识符＞(＜标识符＞|＜标识符＞'['＜无符号整数＞']')
//                              {,(＜标识符＞|＜标识符＞'['＜无符号整数＞']' )}
bool variableDefinition(bool isglobal);
//＜有返回值函数定义＞  ::=  ＜声明头部＞'('＜参数表＞')' '{'＜复合语句＞'}’
bool haveReturnValueFunction();
//＜无返回值函数定义＞  ::= void＜标识符＞'('＜参数表＞')''{'＜复合语句＞'}’
bool noReturnValueFunction();
//＜参数表＞    ::=  ＜类型标识符＞＜标识符＞{,＜类型标识符＞＜标识符＞}| ＜空＞
bool parameterTable(string funcName, bool isRedefine);
//＜复合语句＞  ::=  ［＜常量说明＞］［＜变量说明＞］＜语句列＞
bool compoundStatement();
//＜主函数＞    ::= void main‘(’‘)’ ‘{’＜复合语句＞‘}’
bool mainFunction();
//＜表达式＞    ::= ［＋｜－］＜项＞{＜加法运算符＞＜项＞}  
bool expression(int& type, string& ansTmp);
//＜项＞     ::= ＜因子＞{＜乘法运算符＞＜因子＞}
bool item(int& type, string& ansTmp);
//＜因子＞    ::= ＜标识符＞｜＜标识符＞'['＜表达式＞']'|'('＜表达式＞')'｜＜整数＞|＜字符＞｜＜有返回值函数调用语句＞
bool factor(int& type, string& ansTmp);
//＜语句＞    ::= ＜条件语句＞｜＜循环语句＞| '{'＜语句列＞'}'| ＜有返回值函数调用语句＞; 
//              |＜无返回值函数调用语句＞;｜＜赋值语句＞;｜＜读语句＞;｜＜写语句＞;｜＜空＞;|＜返回语句＞;
bool statement();
//＜赋值语句＞   ::=  ＜标识符＞＝＜表达式＞|＜标识符＞'['＜表达式＞']'=＜表达式＞
bool assignStatement();
//＜条件语句＞  ::= if '('＜条件＞')'＜语句＞［else＜语句＞］
bool conditionStatement();
//＜条件＞  ::=  ＜表达式＞＜关系运算符＞＜表达式＞｜＜表达式＞
bool condition();
//＜循环语句＞   ::=  while '('＜条件＞')'＜语句＞| do＜语句＞while '('＜条件＞')'
//              |for'('＜标识符＞＝＜表达式＞;＜条件＞;＜标识符＞＝＜标识符＞(+|-)＜步长＞')'＜语句＞
bool repeatStatement();
//＜步长＞::= ＜无符号整数＞
bool step(int& value);
//＜有返回值函数调用语句＞ ::= ＜标识符＞'('＜值参数表＞')’
bool callHaveReturnValueFunction();
//＜无返回值函数调用语句＞ ::= ＜标识符＞'('＜值参数表＞')’
bool callNoReturnValueFunction();
//＜值参数表＞  ::= ＜表达式＞{,＜表达式＞}｜＜空＞
bool valueParameterTable(string funcName);
//＜语句列＞   ::= ｛＜语句＞｝
bool statementList();
//＜读语句＞    ::=  scanf '('＜标识符＞{,＜标识符＞}')’
bool readStatement();
//＜写语句＞    ::= printf '(' ＜字符串＞,＜表达式＞ ')'| printf '('＜字符串＞ ')'| printf '('＜表达式＞')’
bool writeStatement();
//＜返回语句＞   ::=  return['('＜表达式＞')']  
bool returnStatement();

void checkBeforeFunc();

void fullNameMap(map<string, string>& nameMap, vector<midCode> ve, string funcName);

void dealInlineFunc(string name, int& begin, int& end);
#endif // !GRAMMER_H
