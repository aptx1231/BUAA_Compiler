#pragma once
#ifndef LEXICAL_H
#define LEXICAL_H

bool isSpace();

bool isNewline();

bool isBlank();

bool isLetter();

bool isDigit();

bool isPlus();

bool isMinu();

bool isMult();

bool isDiv();

bool isChar();

bool isLss();

bool isGre();

bool isExcla();

bool isAssign();

bool isSemicn();

bool isComma();

bool isLparent();

bool isRparent();

bool isLbrack();

bool isRbrack();

bool isLbrace();

bool isRbrace();

bool isSquo();

bool isDquo();

bool isEOF();

bool isStringChar();

bool isValid();

bool isFactorFellow();

void clearToken();

void catToken();

void get_ch();

void retract();

void retractString(int oldIndex);

int reserver();

int transNum();

int getsym(int out=1);

void doOutput();

#endif // !LEXICAL_H

