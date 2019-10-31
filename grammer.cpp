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
using namespace std;

extern char token[100000];
extern int num;   //记录整形常量
extern char con_ch;  //记录字符型常量
extern enum typeId symbol;
extern ofstream outputfile;
extern int oldIndex;    //用于做恢复
extern ofstream errorfile;
extern int line;  //行号
extern string filecontent;  //文件的内容
extern map<string, symbolItem> globalSymbolTable;
extern map<string, symbolItem> localSymbolTable;

//＜字符串＞   ::=  "｛十进制编码为32,33,35-126的ASCII字符｝"
bool strings() {
	if (symbol == STRCON) {
		doOutput();
		outputfile << "<字符串>" << endl;
		getsym();  //预读一个 不管是啥
		return true;
	}
	else {
		return false;
	}
}

//＜程序＞  ::= ［＜常量说明＞］［＜变量说明＞］{＜有返回值函数定义＞|＜无返回值函数定义＞}＜主函数＞
bool procedure() {
	//尝试调用常量说明 因为是[] 是可以没有的 所以即便返回false也不能说程序分析失败,程序分析还要继续
	constDeclaration(true);
	//尝试调用变量说明
	variableDeclaration(true);
	while (true) {
		if (symbol == INTTK || symbol == CHARTK) {
			if (!haveReturnValueFunction()) {//尝试调用有返回值函数
				return false;
			}
		}
		else if (symbol == VOIDTK) {
			if (!noReturnValueFunction()) { //尝试调用无返回值函数
				break;
			}
		}
		else {
			return false;
		}
	}
	//分析主函数
	if (mainFunction()) {
		//看一下主函数之外剩没剩东西  主函数调用完 会预读一个
		if (isEOF()) {
			outputfile << "<程序>" << endl;
			return true;
		}
		else {
			return false;
		}
	}
	else {
		return false;
	}
}

//＜常量说明＞ ::=  const＜常量定义＞;{ const＜常量定义＞;}
bool constDeclaration(bool isglobal) {
	if (symbol == CONSTTK) {
		doOutput();
		int re = getsym();  //读下一个 然后进入常量定义
		if (re < 0) {  //没有下一个 出错
			return false;
		}
		else {
			if (!constDefinition(isglobal)) { //调用常量定义
				return false;
			}  //返回之后 会预读一个 不需要再读了
			else {
				if (isEOF()) {   //预读到了结尾
					return false;
				}
				if (symbol == SEMICN) {  //分号
					doOutput();
					//开始分析{ const＜常量定义＞;}
					while (true) {
						re = getsym();
						if (re < 0) {  //这个是可以没有的
							break;
						}
						if (symbol != CONSTTK) {  //这个是可以没有的
							break;
						}
						doOutput();
						//当前是const 继续向下看
						re = getsym();
						if (re < 0) {  //const 后缺少东西
							return false;
						}
						if (!constDefinition(isglobal)) { //调用常量定义
							return false;
						}
						if (isEOF()) {   //预读到了结尾
							return false;
						}
						//常量定义调用完成 预读了一个符号
						if (symbol != SEMICN) {  //缺少分号
							return false;
						}
						doOutput();
					}
					outputfile << "<常量说明>" << endl;
					return true;
				}
				else {
					return false;
				}
			}
		}
	}
	else {
		return false;
	}
}

//＜常量定义＞ ::= int＜标识符＞＝＜整数＞{,＜标识符＞＝＜整数＞}
//                  | char＜标识符＞＝＜字符＞{,＜标识符＞＝＜字符＞}
bool constDefinition(bool isglobal) {
	string name;
	if (symbol == INTTK) {
		doOutput();
		int re = getsym();  //读下一个 然后进入标识符
		if (re < 0) {  //没有下一个 出错
			return false;
		}
		else {
			if (symbol == IDENFR) {  //读到标识符
				name = string(token);
				doOutput();
				re = getsym();  //读下一个 进入=
				if (re < 0) {
					return false;
				}
				else {
					if (symbol == ASSIGN) {  //读到=
						doOutput();
						re = getsym();  //再读一个 进入整数
						if (re < 0) {
							return false;
						}
						else {
							if (integer()) {  //整数  预读了下一个单词
								//开始分析{,＜标识符＞＝＜整数＞}部分
								if (isglobal) {
									if (globalSymbolTable.find(name) == globalSymbolTable.end()) {  //没找到
										globalSymbolTable.insert(make_pair(name, symbolItem(name, 2, 1, num)));
									}
									else {  //找到了 说明重定义了
										errorfile << line << " b\n";
									}
								}
								else {
									if (localSymbolTable.find(name) == localSymbolTable.end()) {  //没找到
										localSymbolTable.insert(make_pair(name, symbolItem(name, 2, 1, num)));
									}
									else {  //找到了 说明重定义了
										errorfile << line << " b\n";
									}
								}
								while (true) {
									if (isEOF()) {  //预读到了结尾
										break;
									}
									if (symbol != COMMA) {  //这个是可以没有的
										break;
									}
									doOutput();
									//当前是,逗号 继续向下看
									re = getsym();
									if (re < 0) {  //, 后缺少东西
										return false;
									}
									if (symbol != IDENFR) { //,后不是标识符
										return false;
									}
									doOutput();
									name = string(token);
									//当前是标识符
									re = getsym();
									if (re < 0) {  //标识符后缺少东西
										return false;
									}
									if (symbol != ASSIGN) { //标识符后不是=
										return false;
									}
									doOutput();
									//当前是=
									re = getsym();
									if (re < 0) {  //=后缺少东西
										return false;
									}
									if (!integer()) { //=后不是整数
										return false;
									}
									if (isglobal) {
										if (globalSymbolTable.find(name) == globalSymbolTable.end()) {  //没找到
											globalSymbolTable.insert(make_pair(name, symbolItem(name, 2, 1, num)));
										}
										else {  //找到了 说明重定义了
											errorfile << line << " b\n";
										}
									}
									else {
										if (localSymbolTable.find(name) == localSymbolTable.end()) {  //没找到
											localSymbolTable.insert(make_pair(name, symbolItem(name, 2, 1, num)));
										}
										else {  //找到了 说明重定义了
											errorfile << line << " b\n";
										}
									}
								}
								outputfile << "<常量定义>" << endl;
								return true;
							}
							else {
								return false;
							}
						}
					}
					else {
						return false;
					}
				}
			}
			else {  //不是标识符
				return false;
			}
		}
	}
	else if (symbol == CHARTK) {  //char＜标识符＞＝＜字符＞{,＜标识符＞＝＜字符＞}
		doOutput();
		int re = getsym();  //读下一个 然后进入标识符
		if (re < 0) {  //没有下一个 出错
			return false;
		}
		else {
			if (symbol == IDENFR) {  //读到标识符
				doOutput();
				name = string(token);
				re = getsym();  //读下一个 进入=
				if (re < 0) {
					return false;
				}
				else {
					if (symbol == ASSIGN) {  //读到=
						doOutput();
						re = getsym();  //再读一个 进入字符
						if (re < 0) {
							return false;
						}
						else {
							if (symbol == CHARCON) {  //字符常量
								doOutput();
								if (isglobal) {
									if (globalSymbolTable.find(name) == globalSymbolTable.end()) {  //没找到
										globalSymbolTable.insert(make_pair(name, symbolItem(name, 2, 2, 0, con_ch)));
									}
									else {  //找到了 说明重定义了
										errorfile << line << " b\n";
									}
								}
								else {
									if (localSymbolTable.find(name) == localSymbolTable.end()) {  //没找到
										localSymbolTable.insert(make_pair(name, symbolItem(name, 2, 2, 0, con_ch)));
									}
									else {  //找到了 说明重定义了
										errorfile << line << " b\n";
									}
								}
								//开始分析{,＜标识符＞＝＜字符＞}部分
								while (true) {
									re = getsym();
									if (re < 0) {  //这个是可以没有的
										break;
									}
									if (symbol != COMMA) {  //这个是可以没有的
										break;
									}
									doOutput();
									//当前是,逗号 继续向下看
									re = getsym();
									if (re < 0) {  //, 后缺少东西
										return false;
									}
									if (symbol != IDENFR) { //,后不是标识符
										return false;
									}
									doOutput();
									name = string(token);
									//当前是标识符
									re = getsym();
									if (re < 0) {  //标识符后缺少东西
										return false;
									}
									if (symbol != ASSIGN) { //标识符后不是=
										return false;
									}
									doOutput();
									//当前是=
									re = getsym();
									if (re < 0) {  //=后缺少东西
										return false;
									}
									if (symbol != CHARCON) { //=后不是字符
										return false;
									}
									doOutput();
									if (isglobal) {
										if (globalSymbolTable.find(name) == globalSymbolTable.end()) {  //没找到
											globalSymbolTable.insert(make_pair(name, symbolItem(name, 2, 2, 0, con_ch)));
										}
										else {  //找到了 说明重定义了
											errorfile << line << " b\n";
										}
									}
									else {
										if (localSymbolTable.find(name) == localSymbolTable.end()) {  //没找到
											localSymbolTable.insert(make_pair(name, symbolItem(name, 2, 2, 0, con_ch)));
										}
										else {  //找到了 说明重定义了
											errorfile << line << " b\n";
										}
									}
								}
								outputfile << "<常量定义>" << endl;
								return true;
							}
							else {
								return false;
							}
						}
					}
					else {
						return false;
					}
				}
			}
			else {  //不是标识符
				return false;
			}
		}
	}
	else {
		return false;
	}
}

//＜无符号整数＞  ::= ＜非零数字＞｛＜数字＞｝| 0
bool unsignedInteger() {
	if (symbol == INTCON) {
		doOutput();
		getsym();    //预读一个单词 不管成功与否 都得返回true 因为预读成功与否不影响对“整数”的判断
		outputfile << "<无符号整数>" << endl;
		return true;
	}
	else {
		return false;
	}
}

//＜整数＞ ::= ［＋｜－］＜无符号整数＞
bool integer() {
	int re;
	if (symbol == PLUS || symbol == MINU) {
		doOutput();
		re = getsym();
		if (re < 0) {
			return false;
		}
		if (unsignedInteger()) {  //调用无符号整数
			outputfile << "<整数>" << endl;
			return true;
		}
		else {
			return false;
		}
	}
	else {
		if (unsignedInteger()) {  //直接调用无符号整数
			outputfile << "<整数>" << endl;
			return true;
		}
		else {
			return false;
		}
	}
}

//＜声明头部＞   ::=  int＜标识符＞ |char＜标识符＞
bool declarationHead(string& tmp, int& type) {
	if (symbol == INTTK || symbol == CHARTK) {
		if (symbol == INTTK) {
			type = 1;
		}
		else {
			type = 2;
		}
		doOutput();
		int re = getsym();
		if (re < 0) {
			return false;
		}
		if (symbol != IDENFR) {
			return false;
		}
		//当前是标识符 成功了
		doOutput();
		tmp = string(token);
		outputfile << "<声明头部>" << endl;
		re = getsym();  //预读 不管读到什么
		return true;
	}
	else {
		return false;
	}
}

//＜变量说明＞  ::= ＜变量定义＞;{＜变量定义＞;}
bool variableDeclaration(bool isglobal) {
	//变量说明 变量定义 跟 有返回值的函数定义前缀有冲突
	//但是只是在＜程序＞::=［＜常量说明＞］［＜变量说明＞］{＜有返回值函数定义＞|＜无返回值函数定义＞}＜主函数＞
	//在程序这个文法出二者需要做区别
	//在＜复合语句＞  ::=  ［＜常量说明＞］［＜变量说明＞］＜语句列＞
	//复合语句这个文法中 二者不会出现同时出现 所以可以正常处理
	if (!isglobal) {  //不要考虑是不是有返回值的函数了
		if (symbol == INTTK || symbol == CHARTK) {
			if (!variableDefinition(isglobal)) {  //调用变量定义
				return false;
			}
			//变量定义成功 预读了一个符号
			if (isEOF()) {
				return false;
			}
			if (symbol == SEMICN) {  //读到分号  ＜变量定义＞;这部分结束了
				doOutput();
				//接下来分析{＜变量定义＞;}
				while (true) {
					int re = getsym();
					if (re < 0) {
						break;
					}
					if (!variableDefinition(isglobal)) {  //不是变量定义 可以break
						break;
					}
					//当前是变量定义 并且已经预读了
					if (isEOF()) {
						return false;
					}
					if (symbol != SEMICN) {  //变量定义后边不是分号
						return false;
					}
					doOutput();
				}
				outputfile << "<变量说明>" << endl;
				return true;
			}
			else {
				return false;
			}
		}
		else {
			return false;
		}
	}
	else {  //需要考虑 可能是函数  区分的位置在第三位 函数 int a (  而 变量定义是 int a ,[;这类的
		int old = oldIndex;   //记录读取完int/char之后保留的那个oldIndex 这个就是int/char的起始位置
		while (isspace(filecontent[old])) {
			old++;
		}
		if (symbol == INTTK || symbol == CHARTK) {
			int re = getsym(0);  //读下一个
			if (re < 0) {
				return false;
			}
			else {  //读到了 应该是标识符才对
				if (symbol == IDENFR) {
					re = getsym(0);  //读下一个
					if (re < 0) {
						return false;
					}
					else {  //通过他是(还是别的做出区别了
						if (symbol == LPARENT) {  //是( 说明这个应该是函数定义 不是变量定义  需要需要把多读的两个退回去 返回错误
							retractString(old);   //指针回到没有读取int/char之前的位置
							getsym(0);  //把int/char读出来
							return false;
						}
						else {  //应该试着正常调用变量定义了 注意也需要把多读的两个退回去
							retractString(old);   //指针回到没有读取int/char之前的位置
							getsym();  //把int/char读出来
							//开始调用变量定义
							if (symbol == INTTK || symbol == CHARTK) {
								if (!variableDefinition(isglobal)) {  //调用变量定义
									return false;
								}
								//变量定义成功 预读了一个符号
								if (isEOF()) {
									return false;
								}
								if (symbol == SEMICN) {  //读到分号  ＜变量定义＞;这部分结束了
									doOutput();
									//接下来分析{＜变量定义＞;}
									//注意的是在分析中 可能就遇到函数了  也就是每一次分析“＜变量定义＞;”的时候 他都有可能是函数
									while (true) {
										re = getsym();
										if (re < 0) {
											break;
										}
										else {
											old = oldIndex;  //记录读取完int/char之后保留的那个oldIndex 这个就是int/char的起始位置
											while (isspace(filecontent[old])) {
												old++;
											}
											if (symbol == INTTK || symbol == CHARTK) {  //拿到了int/char 说明有可能是变量定义了
												re = getsym(0);  //读下一个
												if (re < 0) {
													return false;   //???
												}
												else {
													if (symbol == IDENFR) {  //是标识符
														re = getsym(0);  //读下一个
														if (re < 0) {
															return false;   //???
														}
														else {  //通过他是(还是别的做出区别了
															if (symbol == LPARENT) {  //是( 说明这个应该是函数定义 不是变量定义  需要需要把多读的两个退回去 break
																retractString(old);   //指针回到没有读取int/char之前的位置
																getsym(0);  //把int/char读出来
																break;
															}
															else {  //应该试着正常调用变量定义了 注意也需要把多读的两个退回去
																retractString(old);   //指针回到没有读取int/char之前的位置
																getsym();  //把int/char读出来
																//开始调用变量定义
																if (symbol == INTTK || symbol == CHARTK) {
																	if (!variableDefinition(isglobal)) {  //调用变量定义
																		break;
																	}
																	//变量定义成功 预读了一个符号
																	if (isEOF()) {
																		return false;
																	}
																	if (symbol == SEMICN) {  //读到分号  ＜变量定义＞;这部分结束了
																		doOutput();
																	}
																	else {
																		return false;
																	}
																}
															}
														}
													}
													else {  //不是标识符
														return false;
													}
												}
											}
											else {  //如果不是的话 说明到此结束了
												break;
											}
										}
									}
									outputfile << "<变量说明>" << endl;
									return true;
								}
								else {
									return false;
								}
							}
							else {
								return false;
							}
						}
					}
				}
				else {
					return false;
				}
			}
		}
		else {
			return false;
		}
	}
}
//＜变量定义＞  ::= ＜类型标识符＞(＜标识符＞|＜标识符＞'['＜无符号整数＞']')
//                              {,(＜标识符＞|＜标识符＞'['＜无符号整数＞']' )}
bool variableDefinition(bool isglobal) {
	string name;
	int type;
	if (symbol == INTTK || symbol == CHARTK) {
		if (symbol == INTTK) {
			type = 1;
		}
		else {
			type = 2;
		}
		doOutput();
		int re = getsym();
		if (re < 0) {
			return false;
		}
		else {
			if (symbol == IDENFR) {   //标识符
				doOutput();
				name = string(token);
				re = getsym();
				if (symbol == LBRACK) {  //[
					doOutput();
					re = getsym();  //预读一个 然后进入判别 无符号整数
					if (re < 0) {
						return false;
					}
					else {
						if (!unsignedInteger()) {  //不是无符号整数
							return false;
						}
						//当前是无符号整数 已经预读了下一个 判断是不是]
						if (symbol != RBRACK) {
							return false;
						}
						else {  // ]
							doOutput();
							if (isglobal) {
								if (globalSymbolTable.find(name) == globalSymbolTable.end()) {  //没找到
									globalSymbolTable.insert(make_pair(name, symbolItem(name, 4, type, 0, ' ', num)));
								}
								else {  //找到了 说明重定义了
									errorfile << line << " b\n";
								}
							}
							else {
								if (localSymbolTable.find(name) == localSymbolTable.end()) {  //没找到
									localSymbolTable.insert(make_pair(name, symbolItem(name, 4, type, 0, ' ', num)));
								}
								else {  //找到了 说明重定义了
									errorfile << line << " b\n";
								}
							}
							re = getsym();  //多读一个 不管是啥 因为如果没有[的时候 也已经预读了一个
						}
					}
				}  //如果不是[ 就相当于预读了
				else {
					if (isglobal) {
						if (globalSymbolTable.find(name) == globalSymbolTable.end()) {  //没找到
							globalSymbolTable.insert(make_pair(name, symbolItem(name, 1, type)));
						}
						else {  //找到了 说明重定义了
							errorfile << line << " b\n";
						}
					}
					else {
						if (localSymbolTable.find(name) == localSymbolTable.end()) {  //没找到
							localSymbolTable.insert(make_pair(name, symbolItem(name, 1, type)));
						}
						else {  //找到了 说明重定义了
							errorfile << line << " b\n";
						}
					}
				}
				//不是[ 或者[]处理完 接下来分析 {,(＜标识符＞|＜标识符＞'['＜无符号整数＞']' )}
				//稍有不同 因为上一步已经读了一个单词 进入循环不要立刻读单词
				while (true) {
					if (symbol != COMMA) {  //不是 ,  说明没有这部分内容
						break;
					}
					doOutput();
					//当前是, 判断下一个是不是标识符
					re = getsym();
					if (re < 0) {
						return false;
					}
					else {
						if (symbol == IDENFR) {   //标识符
							doOutput();
							name = string(token);
							re = getsym();
							if (symbol == LBRACK) {  //[
								doOutput();
								re = getsym();  //预读一个 然后进入判别 无符号整数
								if (re < 0) {
									return false;
								}
								else {
									if (!unsignedInteger()) {  //不是无符号整数
										return false;
									}
									//当前是无符号整数 已经预读了下一个 判断是不是]
									if (symbol != RBRACK) {
										return false;
									}
									else {  // ]
										doOutput();
										if (isglobal) {
											if (globalSymbolTable.find(name) == globalSymbolTable.end()) {  //没找到
												globalSymbolTable.insert(make_pair(name, symbolItem(name, 4, type, 0, ' ', num)));
											}
											else {  //找到了 说明重定义了
												errorfile << line << " b\n";
											}
										}
										else {
											if (localSymbolTable.find(name) == localSymbolTable.end()) {  //没找到
												localSymbolTable.insert(make_pair(name, symbolItem(name, 4, type, 0, ' ', num)));
											}
											else {  //找到了 说明重定义了
												errorfile << line << " b\n";
											}
										}
										re = getsym();  //多读一个 不管是啥 因为如果没有[的时候 也已经预读了一个
									}
								}
							}  //如果不是[ 就相当于预读了
							else {
								if (isglobal) {
									if (globalSymbolTable.find(name) == globalSymbolTable.end()) {  //没找到
										globalSymbolTable.insert(make_pair(name, symbolItem(name, 1, type)));
									}
									else {  //找到了 说明重定义了
										errorfile << line << " b\n";
									}
								}
								else {
									if (localSymbolTable.find(name) == localSymbolTable.end()) {  //没找到
										localSymbolTable.insert(make_pair(name, symbolItem(name, 1, type)));
									}
									else {  //找到了 说明重定义了
										errorfile << line << " b\n";
									}
								}
							}
						}
						else {
							return false;
						}
					}
				}
				outputfile << "<变量定义>" << endl;
				return true;
				//}
			}
			else {
				return false;
			}
		}
	}
	else {
		return false;
	}
}

//＜有返回值函数定义＞  ::=  ＜声明头部＞'('＜参数表＞')' '{'＜复合语句＞'}’
bool haveReturnValueFunction() {
	string name;
	int type;
	if (!declarationHead(name, type)) {  //调用声明头部
		return false;
	}
	//调用声明头部成功 预读了一个符号
	bool isRedefine = false;
	if (globalSymbolTable.find(name) == globalSymbolTable.end()) {  //没找到
		globalSymbolTable.insert(make_pair(name, symbolItem(name, 3, type)));
	}
	else {  //找到了 说明重定义了
		errorfile << line << " b\n";
		isRedefine = true;
	}
	if (isEOF()) {
		return false;
	}
	if (symbol == LPARENT) {  //声明头部后边是(
		doOutput();
		int re = getsym();  //为分析参数表预读
		if (re < 0) {
			return false;
		}
		//开始分析参数表
		if (!parameterTable(name, isRedefine)) {
			return false;
		}
		//分析参数表成功 预读了一个符号
		if (isEOF()) {
			return false;
		}
		if (symbol == RPARENT) {  //参数表后边是)
			doOutput();
			re = getsym();
			if (re < 0) {
				return false;
			}
			if (symbol != LBRACE) {  //)后边不是{
				return false;
			}
			//当前是{  为分析复合语句预读
			doOutput();
			re = getsym();
			if (re < 0) {
				return false;
			}
			if (!compoundStatement()) {  //开始分析复合语句
				return false;
			}
			//分析复合语句成功 预读了一个符号
			if (isEOF()) {
				return false;
			}
			if (symbol == RBRACE) {  //复合语句后边是}
				doOutput();
				outputfile << "<有返回值函数定义>" << endl;
				re = getsym();  //预读 不管读到什么
				showLocal();
				localSymbolTable.clear();
				return true;
			}
			else {
				return false;
			}
		}
		else {
			return false;
		}
	}
	else {
		return false;
	}
}

//＜无返回值函数定义＞  ::= void＜标识符＞'('＜参数表＞')''{'＜复合语句＞'}’
bool noReturnValueFunction() {
	//无返回值函数和main函数前缀有冲突 需要多读一个做预判
	string name;
	int old = oldIndex;   //记录读取完void只有的oldIndex 是void的起始位置
	while (isspace(filecontent[old])) {
		old++;
	}
	if (symbol == VOIDTK) {
		//doOutput();
		int re = getsym(0);
		if (re < 0) {
			return false;
		}
		if (symbol != IDENFR && symbol != MAINTK) {  //不是标识符 也不是 main
			return false;
		}
		//doOutput();
		//当前是标识符 需要知道这个标识符是不是main
		if (symbol == MAINTK) {  //说明是main函数 需要恢复回去
			retractString(old);   //回退到void的起始位置
			getsym(0);  //重新读出来void
			return false;
		}
		//说明是无返回值函数了 token存的不是main
		symbol = VOIDTK;  //没有修改token 后边直接回复symbol就可以了
		doOutput();
		symbol = IDENFR;
		doOutput();
		name = string(token);
		bool isRedefine = false;
		if (globalSymbolTable.find(name) == globalSymbolTable.end()) {  //没找到
			globalSymbolTable.insert(make_pair(name, symbolItem(name, 3, 3)));
		}
		else {  //找到了 说明重定义了
			errorfile << line << " b\n";
			isRedefine = true;
		}
		//当前是标识符 看下一个是不是(
		re = getsym();
		if (re < 0) {
			return false;
		}
		if (symbol == LPARENT) {  //标识符后边是(
			doOutput();
			re = getsym();  //为分析参数表预读
			if (re < 0) {
				return false;
			}
			//开始分析参数表
			if (!parameterTable(name, isRedefine)) {
				return false;
			}
			//分析参数表成功 预读了一个符号
			if (isEOF()) {
				return false;
			}
			if (symbol == RPARENT) {  //参数表后边是)
				doOutput();
				re = getsym();
				if (re < 0) {
					return false;
				}
				if (symbol != LBRACE) {  //)后边不是{
					return false;
				}
				//当前是{  为分析复合语句预读
				doOutput();
				re = getsym();
				if (re < 0) {
					return false;
				}
				if (!compoundStatement()) {  //开始分析复合语句
					return false;
				}
				//分析复合语句成功 预读了一个符号
				if (isEOF()) {
					return false;
				}
				if (symbol == RBRACE) {  //复合语句后边是}
					doOutput();
					outputfile << "<无返回值函数定义>" << endl;
					re = getsym();  //预读 不管读到什么
					showLocal();
					localSymbolTable.clear();
					return true;
				}
				else {
					return false;
				}
			}
			else {
				return false;
			}
		}
		else {
			return false;
		}
	}
	else {
		return false;
	}
}

//＜参数表＞    ::=  ＜类型标识符＞＜标识符＞{,＜类型标识符＞＜标识符＞}| ＜空＞
bool parameterTable(string funcName, bool isRedefine) {
	//参数表可以为空  参数表为空时 当前字符就是)右括号
	string name;
	int type;
	if (symbol == RPARENT) {
		outputfile << "<参数表>" << endl;
		return true;
	}
	if (symbol == INTTK || symbol == CHARTK) {
		if (symbol == INTTK) {
			type = 1;
		}
		else {
			type = 2;
		}
		doOutput();
		int re = getsym();
		if (re < 0) {
			return false;
		}
		if (symbol != IDENFR) {
			return false;
		}
		//当前是标识符  开始分析{,＜类型标识符＞＜标识符＞}
		name = string(token);
		if (localSymbolTable.find(name) == localSymbolTable.end()) {  //没找到
			localSymbolTable.insert(make_pair(name, symbolItem(name, 1, type)));
			if (!isRedefine) {
				globalSymbolTable[funcName].insert(type);
			}
		}
		else {  //找到了 说明重定义了
			errorfile << line << " b\n";
		}
		doOutput();
		while (true) {
			re = getsym();
			if (re < 0) {
				break;
			}
			if (symbol != COMMA) {
				break;
			}
			//当前是, 看下一个是不是类型标识符
			doOutput();
			re = getsym();
			if (re < 0) {
				return false;
			}
			if (symbol != INTTK && symbol != CHARTK) {
				return false;
			}
			if (symbol == INTTK) {
				type = 1;
			}
			else {
				type = 2;
			}
			//当前是类型标识符 看下一个是不是标识符
			doOutput();
			re = getsym();
			if (re < 0) {
				return false;
			}
			if (symbol != IDENFR) {
				return false;
			}
			//当前是标识符
			doOutput();
			name = string(token);
			if (localSymbolTable.find(name) == localSymbolTable.end()) {  //没找到
				localSymbolTable.insert(make_pair(name, symbolItem(name, 1, type)));
				if (!isRedefine) {
					globalSymbolTable[funcName].insert(type);
				}
			}
			else {  //找到了 说明重定义了
				errorfile << line << " b\n";
			}
		}
		outputfile << "<参数表>" << endl;
		return true;
	}
	else {
		return false;
	}
}

//＜复合语句＞  ::=  ［＜常量说明＞］［＜变量说明＞］＜语句列＞
bool compoundStatement() {
	//尝试调用常量说明 因为是[] 是可以没有的 所以即便返回false也不能说程序分析失败,程序分析还要继续
	constDeclaration(false);
	//尝试调用变量说明
	variableDeclaration(false);  //在这调用 说明只可能是变量说明，不可能是有返回值的函数 不需要判别了
	//调用语句列
	if (statementList()) {
		outputfile << "<复合语句>" << endl;
		return true;
	}
	else {
		return false;
	}
}

//＜主函数＞    ::= void main‘(’‘)’ ‘{’＜复合语句＞‘}’
bool mainFunction() {
	if (symbol == VOIDTK) {
		doOutput();
		int re = getsym();
		if (re < 0) {
			return false;
		}
		if (symbol == MAINTK) {  //void后边是main
			//当前是main 看下一个是不是(
			doOutput();
			re = getsym();
			if (re < 0) {
				return false;
			}
			if (symbol == LPARENT) {  //main后边是(
				doOutput();
				re = getsym();  //看下一个是不是)
				if (re < 0) {
					return false;
				}
				if (symbol == RPARENT) {  //(后边是)
					doOutput();
					re = getsym();
					if (re < 0) {
						return false;
					}
					if (symbol == LBRACE) { //)后边是{
						//当前是{  为分析复合语句预读
						doOutput();
						re = getsym();
						if (re < 0) {
							return false;
						}
						if (!compoundStatement()) {  //开始分析复合语句
							return false;
						}
						//分析复合语句成功 预读了一个符号
						if (isEOF()) {
							return false;
						}
						if (symbol == RBRACE) {  //复合语句后边是}
							doOutput();
							outputfile << "<主函数>" << endl;
							re = getsym();  //预读 不管读到什么
							showLocal();
							localSymbolTable.clear();
							return true;
						}
						else {
							return false;
						}
					}
					else {
						return false;
					}
				}
				else {
					return false;
				}
			}
			else {
				return false;
			}
		}
		else {
			return false;
		}
	}
	else {
		return false;
	}
}

//＜表达式＞    ::= ［＋｜－］＜项＞{＜加法运算符＞＜项＞}  
bool expression(int& type) {
	bool first = false;
	int re;
	if (symbol == PLUS || symbol == MINU) {
		doOutput();
		re = getsym();
		if (re < 0) {
			return false;
		}
		first = true;
	}
	//进入项的分析
	if (!item(type)) {
		return false;
	}
	//项分析成功 并预读了一个单词
	bool flag = false;
	//开始分析{＜加法运算符＞＜项＞}  
	while (true) {
		if (isEOF()) {
			break;
		}
		if (symbol != PLUS && symbol != MINU) {  //不是+不是-
			break;
		}
		//项中包括了加法减法 就一定是int了
		flag = true;
		doOutput();
		re = getsym();
		if (re < 0) {
			return false;
		}
		//进入项的分析
		if (!item(type)) {
			return false;
		}
	}
	if (first) {  //带有最前边的+-号 一定是int
		type = 1;
	}
	else {
		if (flag) {  //项中包括了加法减法 就一定是int
			type = 1;
		}
	}
	outputfile << "<表达式>" << endl;
	return true;
}

//＜项＞     ::= ＜因子＞{＜乘法运算符＞＜因子＞}
bool item(int& type) {
	if (!factor(type)) {  //直接分析因子
		return false;
	}
	//因子分析成功 并预读了一个单词
	//开始分析 {＜乘法运算符＞＜因子＞}
	bool flag = false;
	while (true) {
		if (isEOF()) {
			break;
		}
		if (symbol != MULT && symbol != DIV) {  //不是* 也不是/
			break;
		}
		doOutput();
		//项中包括了乘法除法 就一定是int了
		flag = true;
		int re = getsym();
		if (re < 0) {
			return false;
		}
		//进入因子的分析
		if (!factor(type)) {
			return false;
		}
	}
	if (flag) {  //项中包括了乘法除法 就一定是int了 
		type = 1;
	}
	outputfile << "<项>" << endl;
	return true;
}

//＜因子＞    ::= ＜标识符＞｜＜标识符＞'['＜表达式＞']'|'('＜表达式＞')'｜＜整数＞|＜字符＞｜＜有返回值函数调用语句＞
//注意 ＜有返回值函数调用语句＞ ::= ＜标识符＞'('＜值参数表＞')’跟 ＜标识符＞｜＜标识符＞'['＜表达式＞']'|'('＜表达式＞')' 有前缀的冲突
bool factor(int& type) {
	int re;
	int old = oldIndex;  //记录读取完标识符之后的oldIndex 是标识符的起始位置
	while (isspace(filecontent[old])) {
		old++;
	}
	if (symbol == IDENFR) {  //当前是标识符  对应文法 ＜标识符＞｜＜标识符＞'['＜表达式＞']' 也可能是 ＜有返回值函数调用语句＞
		re = getsym(0);
		if (re < 0) {
			return false;
		}
		if (symbol == LBRACK) {  //是[
			symbol = IDENFR;
			string name = string(token);
			if (localSymbolTable.find(name) != localSymbolTable.end() && localSymbolTable[name].kind != 3) {
				type = localSymbolTable[name].type;
			}
			else {
				if (globalSymbolTable.find(name) != globalSymbolTable.end() && globalSymbolTable[name].kind != 3) {
					type = globalSymbolTable[name].type;
				}
				else {
					errorfile << line << " c\n";  //未定义的名字
				}
			}
			doOutput();  //因为[不会修改token 只需要改一下symbol 就能输出刚才的标识符了
			symbol = LBRACK;
			doOutput();
			re = getsym();
			if (re < 0) {
				return false;
			}
			//进入到对表达式的分析
			int t; 
			//＜标识符＞'['＜表达式＞']'的类型取决于标识符 而不是后边的数组下标 所以不能用type 否则type被修改了
			if (!expression(t)) {
				return false;
			}
			//表达式分析成功 并预读了一个单词
			if (symbol == RBRACK) {  //是]
				doOutput();
				re = getsym();   //为下一个预读 不管是啥
				outputfile << "<因子>" << endl;
				return true;
			}
			else {
				return false;
			}
		}
		else if (symbol == LPARENT) {  //是( 说明这个有可能是  ＜有返回值函数调用语句＞  需要恢复
			retractString(old);   //回退到标识符的起始位置
			getsym(0);   //把标识符重新读出来
			//开始调用 有返回值的函数调用语句
			string name = string(token);
			if (globalSymbolTable.find(name) != globalSymbolTable.end()
				&& globalSymbolTable[name].kind == 3
				&& (globalSymbolTable[name].type == 1 || globalSymbolTable[name].type == 2)
				&& localSymbolTable.find(name) == localSymbolTable.end()
				) {
				if (!callHaveReturnValueFunction()) {
					return false;
				}
				//调用有返回值的函数调用语句成功 并预读了一个单词
				type = globalSymbolTable[name].type;
				outputfile << "<因子>" << endl;
				return true;
			}
			else {
				errorfile << line << " c\n";  //未定义的名字
				while (1) {
					get_ch();
					if (isRparent()) {  //)  作为因子出现的 不能一直读到;了 )就得停
						break;
					}
				}
				outputfile << "<因子>" << endl;
				getsym();  //预读一个 不管是啥
				return true;
			}
		}
		else {  //标识符后边不是[ 也不是(  对应文法＜标识符＞  直接返回 这个单词就是为下一个预读的单词了
			//但是如果直接返回 那么这个标识符就没有输出出来
			retractString(old);   //回退到标识符的起始位置
			getsym(0);   //把标识符重新读出来
			string name = string(token);
			if (localSymbolTable.find(name) != localSymbolTable.end() && localSymbolTable[name].kind != 3) {
				type = localSymbolTable[name].type;
			}
			else {
				if (globalSymbolTable.find(name) != globalSymbolTable.end() && globalSymbolTable[name].kind != 3) {
					type = globalSymbolTable[name].type;
				}
				else {
					errorfile << line << " c\n";  //未定义的名字
				}
			}
			doOutput();
			outputfile << "<因子>" << endl;
			getsym();  //预读一个
			return true;
		}
	}
	else if (symbol == LPARENT) {  //当前是(  对应文法 '('＜表达式＞')'
		doOutput();
		re = getsym();
		if (re < 0) {
			return false;
		}
		//进入到对表达式的分析
		if (!expression(type)) {
			return false;
		}
		type = 1;  //(表达式)这个的类型就是int
		//表达式分析成功 并预读了一个单词
		if (symbol == RPARENT) {  //是)
			doOutput();
			re = getsym();   //为下一个预读 不管是啥
			outputfile << "<因子>" << endl;
			return true;
		}
		else {
			return false;
		}
	}
	else if (symbol == CHARCON) {  //当前是字符 对应文法 ＜字符＞
		type = 2;  //字符常量类型是char 
		doOutput();
		re = getsym();   //为下一个预读 不管是啥
		outputfile << "<因子>" << endl;
		return true;
	}
	else if (integer()) {  //当前是整数   并预读了一个单词
		type = 1;  //整形常量 类型是int
		outputfile << "<因子>" << endl;
		return true;
	}
	else {
		type = 0;
		return false;
	}
}

//＜语句＞    ::= ＜条件语句＞｜＜循环语句＞| '{'＜语句列＞'}'| ＜有返回值函数调用语句＞; 
//              |＜无返回值函数调用语句＞;｜＜赋值语句＞;｜＜读语句＞;｜＜写语句＞;｜＜空＞;|＜返回语句＞;
bool statement() {
	if (symbol == SEMICN) {  //;分号  一个分号直接就是一个语句
		doOutput();
		outputfile << "<语句>" << endl;
		getsym();  //预读一个 不管是啥
		return true;
	}
	else if (symbol == RETURNTK) {  //＜返回语句＞;
		if (returnStatement()) {
			//分析返回语句成功 并预读了一个符号
			if (symbol == SEMICN) {  //;分号
				doOutput();
				outputfile << "<语句>" << endl;
				getsym();  //预读一个 不管是啥
				return true;
			}
			else {
				return false;
			}
		}
		else {
			return false;
		}
	}
	else if (symbol == SCANFTK) {  //＜读语句＞;
		if (readStatement()) {
			//分析读语句成功 并预读了一个符号
			if (symbol == SEMICN) {  //;分号
				doOutput();
				outputfile << "<语句>" << endl;
				getsym();  //预读一个 不管是啥
				return true;
			}
			else {
				return false;
			}
		}
		else {
			return false;
		}
	}
	else if (symbol == PRINTFTK) {  //＜写语句＞;
		if (writeStatement()) {
			//分析写语句成功 并预读了一个符号
			if (symbol == SEMICN) {  //;分号
				doOutput();
				outputfile << "<语句>" << endl;
				getsym();  //预读一个 不管是啥
				return true;
			}
			else {
				return false;
			}
		}
		else {
			return false;
		}
	}
	else if (symbol == IFTK) {  //＜条件语句＞
		if (conditionStatement()) {
			//分析条件语句成功 并预读了一个符号
			outputfile << "<语句>" << endl;
			return true;
		}
		else {
			return false;
		}
	}
	else if (symbol == WHILETK || symbol == DOTK || symbol == FORTK) {  //＜循环语句＞
		if (repeatStatement()) {
			//分析循环语句成功 并预读了一个符号
			outputfile << "<语句>" << endl;
			return true;
		}
		else {
			return false;
		}
	}
	else if (symbol == LBRACE) {  //'{'＜语句列＞'}'
		doOutput();
		int re = getsym();
		if (re < 0) {
			return false;
		}
		if (!statementList()) {  //分析语句列
			return false;
		}
		//分析语句列 并预读了一个符号
		if (symbol == RBRACE) {  //}
			doOutput();
			outputfile << "<语句>" << endl;
			getsym();  //预读一个 不管是啥
			return true;
		}
		else {
			return false;
		}
	}
	else if (symbol == IDENFR) {  //＜有返回值函数调用语句＞; |＜无返回值函数调用语句＞;｜＜赋值语句＞;
		int old = oldIndex;  //记录下读取完标识符之后的oldIndex 就是这个标识符的起始位置
		while (isspace(filecontent[old])) {
			old++;
		}
		int re = getsym(0);
		if (re < 0) {
			return false;
		}
		if (symbol == LBRACK || symbol == ASSIGN) {  //[ = 说明是赋值语句
			retractString(old);
			getsym(0);
			if (!assignStatement()) {
				return false;
			}
			//分析赋值语句成功 并预读了一个单词
			if (symbol == SEMICN) {  //;分号
				doOutput();
				outputfile << "<语句>" << endl;
				getsym();  //预读一个 不管是啥
				return true;
			}
			else {
				return false;
			}
		}
		else if (symbol == LPARENT) {  //( 说明是函数调用语句
			retractString(old);
			getsym(0);
			string name = string(token);
			if (globalSymbolTable.find(name) != globalSymbolTable.end()
				&& globalSymbolTable[name].kind == 3
				&& localSymbolTable.find(name) == localSymbolTable.end()
				) {
				if (globalSymbolTable[name].type == 3) {  //func void
					if (!callNoReturnValueFunction()) {
						return false;
					}
					//分析无返回值函数调用语句成功 并预读了一个单词
					if (symbol == SEMICN) {  //;分号
						doOutput();
						outputfile << "<语句>" << endl;
						getsym();  //预读一个 不管是啥
						return true;
					}
					else {
						return false;
					}
				}
				else {  //func int char
					if (!callHaveReturnValueFunction()) {
						return false;
					}
					//分析有返回值函数调用语句成功 并预读了一个单词
					if (symbol == SEMICN) {  //;分号
						doOutput();
						outputfile << "<语句>" << endl;
						getsym();  //预读一个 不管是啥
						return true;
					}
					else {
						return false;
					}
				}
			}
			else {
				errorfile << line << " c\n";  //未定义的名字
				while (1) {
					get_ch();
					if (isSemicn()) {  //;
						break;
					}
				}
				outputfile << "<语句>" << endl;
				getsym();  //预读一个 不管是啥
				return true;
			}
		}
		else {
			return false;
		}
	}
	else {
		return false;
	}
}

//＜赋值语句＞   ::=  ＜标识符＞＝＜表达式＞|＜标识符＞'['＜表达式＞']'=＜表达式＞
bool assignStatement() {
	if (symbol == IDENFR) {  //是标识符
		string name = string(token);
		if (!((localSymbolTable.find(name) != localSymbolTable.end() && localSymbolTable[name].kind != 3)
			|| (globalSymbolTable.find(name) != globalSymbolTable.end() && globalSymbolTable[name].kind != 3))
			) {
			errorfile << line << " c\n";  //未定义的名字
		}
		doOutput();
		int re = getsym();
		if (re < 0) {
			return false;
		}
		if (symbol == LBRACK) {  //[   对应文法＜标识符＞'['＜表达式＞']'=＜表达式＞
			doOutput();
			re = getsym();
			if (re < 0) {
				return false;
			}
			//开始分析表达式
			int t;
			if (!expression(t)) {  //t是数组下标的类型
				return false;
			}
			//分析表达式成功 并预读了一个单词
			if (symbol == RBRACK) {  //]
				doOutput();
				re = getsym();
				if (re < 0) {
					return false;
				}
				if (symbol == ASSIGN) {  // =
					doOutput();
					re = getsym();
					if (re < 0) {
						return false;
					}
					//开始分析表达式
					int tt;
					if (!expression(tt)) {
						return false;
					}
					//分析表达式成功 并预读了一个单词
					outputfile << "<赋值语句>" << endl;
					return true;
				}
				else {
					return false;
				}
			}
			else {
				return false;
			}
		}
		else  if (symbol == ASSIGN) {  //= 对应文法 ＜标识符＞＝＜表达式＞
			doOutput();
			re = getsym();
			if (re < 0) {
				return false;
			}
			//开始分析表达式
			int t;
			if (!expression(t)) {
				return false;
			}
			//分析表达式成功 并预读了一个单词
			outputfile << "<赋值语句>" << endl;
			return true;
		}
		else {
			return false;
		}
	}
	else {
		return false;
	}
}

//＜条件语句＞  ::= if '('＜条件＞')'＜语句＞［else＜语句＞］
bool conditionStatement() {
	if (symbol == IFTK) {  //if
		doOutput();
		int re = getsym();
		if (re < 0) {
			return false;
		}
		if (symbol == LPARENT) {  //(
			doOutput();
			re = getsym();
			if (re < 0) {
				return false;
			}
			if (!condition()) {  //分析条件
				return false;
			}
			//分析条件成功 并预读了一个单词
			if (symbol == RPARENT) {  //)
				doOutput();
				re = getsym();
				if (re < 0) {
					return false;
				}
				if (!statement()) {  //分析语句
					return false;
				}
				//分析语句成功 并预读了一个单词
				//开始分析［else＜语句＞］
				if (symbol == ELSETK) {  //else
					doOutput();
					re = getsym();
					if (re < 0) {
						return false;
					}
					if (!statement()) {  //分析语句
						return false;
					}
					//分析语句成功 并预读了一个单词
					//return true;
				} //不是else 说明没有这部分 是可以的
				outputfile << "<条件语句>" << endl;
				return true;
			}
			else {
				return false;
			}
		}
		else {
			return false;
		}
	}
	else {
		return false;
	}
}

//＜条件＞  ::=  ＜表达式＞＜关系运算符＞＜表达式＞｜＜表达式＞
bool condition() {
	int typeLeft, typeRight;
	if (!expression(typeLeft)) {  //直接调用表达式
		return false;
	}
	//分析表达式成功 并预读一个单词
	if (symbol == LSS || symbol == LEQ || symbol == GRE || symbol == GEQ || symbol == EQL || symbol == NEQ) {  //关系运算符
		doOutput();
		int re = getsym();
		if (re < 0) {
			return false;
		}
		//开始分析表达式
		if (!expression(typeRight)) {
			return false;
		}
		//分析表达式成功 并预读了一个单词
		if (typeLeft != 1 || typeRight != 1) {
			errorfile << line << " f\n";  //条件判断中出现不合法的类型 要求全是int才行
		}
		outputfile << "<条件>" << endl;
		return true;
	}
	else {
		if (typeLeft != 1) {
			errorfile << line << " f\n";  //条件判断中出现不合法的类型 要求是int才行
		}
		outputfile << "<条件>" << endl;
		return true;
	}
}

//＜循环语句＞   ::=  while '('＜条件＞')'＜语句＞| do＜语句＞while '('＜条件＞')'
//              |for'('＜标识符＞＝＜表达式＞;＜条件＞;＜标识符＞＝＜标识符＞(+|-)＜步长＞')'＜语句＞
bool repeatStatement() {
	if (symbol == WHILETK) {  //while '('＜条件＞')'＜语句＞
		doOutput();
		int re = getsym();
		if (re < 0) {
			return false;
		}
		if (symbol == LPARENT) {  //(
			doOutput();
			re = getsym();
			if (re < 0) {
				return false;
			}
			if (!condition()) {  //分析条件
				return false;
			}
			//分析条件成功 并预读了一个单词
			if (symbol == RPARENT) {  //)
				doOutput();
				re = getsym();
				if (re < 0) {
					return false;
				}
				if (!statement()) {  //分析语句
					return false;
				}
				//分析语句成功 并预读了一个单词
				outputfile << "<循环语句>" << endl;
				return true;
			}
			else {
				return false;
			}
		}
		else {
			return false;
		}
	}
	else if (symbol == DOTK) {  // do＜语句＞while '('＜条件＞')'
		doOutput();
		int re = getsym();
		if (re < 0) {
			return false;
		}
		if (!statement()) {  //分析语句
			return false;
		}
		//分析语句成功 并预读了一个单词
		if (symbol == WHILETK) {
			doOutput();
			re = getsym();
			if (re < 0) {
				return false;
			}
			if (symbol == LPARENT) {  //(
				doOutput();
				re = getsym();
				if (re < 0) {
					return false;
				}
				if (!condition()) {  //分析条件
					return false;
				}
				//分析条件成功 并预读了一个单词
				if (symbol == RPARENT) {  //)
					doOutput();
					outputfile << "<循环语句>" << endl;
					getsym(); //预读一个 不管是啥
					return true;
				}
				else {
					return false;
				}
			}
			else {
				return false;
			}
		}
		else {
			return false;
		}
	}
	else if (symbol == FORTK) {  //for'('＜标识符＞＝＜表达式＞;＜条件＞;＜标识符＞＝＜标识符＞(+|-)＜步长＞')'＜语句＞
		doOutput();
		int re = getsym();
		if (re < 0) {
			return false;
		}
		if (symbol != LPARENT) {  //(
			return false;
		}
		doOutput();  //是(
		re = getsym();
		if (re < 0) {
			return false;
		}
		if (symbol != IDENFR) {  //不是标识符
			return false;
		}
		doOutput();   //是标识符
		string name = string(token);
		if (!( (localSymbolTable.find(name) != localSymbolTable.end() && localSymbolTable[name].kind != 3)
			|| (globalSymbolTable.find(name) != globalSymbolTable.end() && globalSymbolTable[name].kind != 3) )
			) {
			errorfile << line << " c\n";  //未定义的名字
		}
		re = getsym();
		if (re < 0) {
			return false;
		}
		if (symbol != ASSIGN) {  //不是=
			return false;
		}
		doOutput();   //是=
		re = getsym();
		if (re < 0) {
			return false;
		}
		int t;
		if (!expression(t)) {  //分析表达式
			return false;
		}
		//分析表达式成功 并预读了一个单词
		if (symbol != SEMICN) {  //不是;
			return false;
		}
		doOutput(); //是;
		re = getsym();
		if (re < 0) {
			return false;
		}
		if (!condition()) {  //分析条件
			return false;
		}
		//分析条件成功 并预读了一个单词
		if (symbol != SEMICN) {  //不是;
			return false;
		}
		doOutput();  //是;
		re = getsym();
		if (re < 0) {
			return false;
		}
		if (symbol != IDENFR) {  //不是标识符
			return false;
		}
		doOutput();  //是标识符
		name = string(token);
		if (!((localSymbolTable.find(name) != localSymbolTable.end() && localSymbolTable[name].kind != 3)
			|| (globalSymbolTable.find(name) != globalSymbolTable.end() && globalSymbolTable[name].kind != 3))
			) {
			errorfile << line << " c\n";  //未定义的名字
		}
		re = getsym();
		if (re < 0) {
			return false;
		}
		if (symbol != ASSIGN) {  //不是=
			return false;
		}
		doOutput();   //是=
		re = getsym();
		if (re < 0) {
			return false;
		}
		if (symbol != IDENFR) {  //不是标识符
			return false;
		}
		doOutput();  //是标识符
		name = string(token);
		if (!((localSymbolTable.find(name) != localSymbolTable.end() && localSymbolTable[name].kind != 3)
			|| (globalSymbolTable.find(name) != globalSymbolTable.end() && globalSymbolTable[name].kind != 3))
			) {
			errorfile << line << " c\n";  //未定义的名字
		}
		re = getsym();
		if (re < 0) {
			return false;
		}
		if (symbol != PLUS && symbol != MINU) { //不是+ -
			return false;
		}
		doOutput();  //是+-
		re = getsym();
		if (re < 0) {
			return false;
		}
		if (!step()) {  //分析步长
			return false;
		}
		//分析步长成功 并预读了一个单词
		if (symbol != RPARENT) {  //不是)
			return false;
		}
		doOutput();  //是)
		re = getsym();
		if (re < 0) {
			return false;
		}
		if (!statement()) {  //分析语句
			return false;
		}
		//分析语句成功 并预读了一个单词
		outputfile << "<循环语句>" << endl;
		return true;
	}
	else {
		return false;
	}
}

//＜步长＞::= ＜无符号整数＞
bool step() {
	if (unsignedInteger()) {
		outputfile << "<步长>" << endl;
		return true;
	}
	else {
		return false;
	}
}

//＜有返回值函数调用语句＞ ::= ＜标识符＞'('＜值参数表＞')’
//现在的写法 一旦进入了这个函数 那么这个函数名就是合法的 即存在于全局表 不存在与局部表 且 返回值是int/char
bool callHaveReturnValueFunction() {
	if (symbol == IDENFR) {  //标识符是函数名 需要查全局符号表
		string name = string(token);
		doOutput();
		int re = getsym();
		if (re < 0) {
			return false;
		}
		if (symbol == LPARENT) {  //是(
			doOutput();
			re = getsym();
			if (re < 0) {
				return false;
			}
			//开始调用值参数表
			if (!valueParameterTable(name)) {
				return false;
			}
			//调用值参数表成功 并预读了一个单词
			if (symbol == RPARENT) {  //是)
				doOutput();
				outputfile << "<有返回值函数调用语句>" << endl;
				getsym();  //预读一个 不管是啥
				return true;
			}
			else {
				return false;
			}
		}
		else {
			return false;
		}
	}
	else {
		return false;
	}
}

//＜无返回值函数调用语句＞ ::= ＜标识符＞'('＜值参数表＞')’
//现在的写法 一旦进入了这个函数 那么这个函数名就是合法的 即存在于全局表 不存在与局部表 且 返回值是void
bool callNoReturnValueFunction() {
	if (symbol == IDENFR) {  //标识符
		string name = string(token);
		doOutput();
		int re = getsym();
		if (re < 0) {
			return false;
		}
		if (symbol == LPARENT) {  //是(
			doOutput();
			re = getsym();
			if (re < 0) {
				return false;
			}
			//开始调用值参数表
			if (!valueParameterTable(name)) {
				return false;
			}
			//调用值参数表成功 并预读了一个单词
			if (symbol == RPARENT) {  //是)
				doOutput();
				outputfile << "<无返回值函数调用语句>" << endl;
				getsym();  //预读一个 不管是啥
				return true;
			}
			else {
				return false;
			}
		}
		else {
			return false;
		}
	}
	else {
		return false;
	}
}

//＜值参数表＞  ::= ＜表达式＞{,＜表达式＞}｜＜空＞
bool valueParameterTable(string funcName) {
	//值参数表可以为空  值参数表为空时 当前字符就是)右括号
	if (symbol == RPARENT) {
		if (globalSymbolTable[funcName].parameterTable.size() != 0) {
			errorfile << line << " d\n";  //参数个数不匹配
		}
		outputfile << "<值参数表>" << endl;
		return true;
	}
	vector<int> typeList;  //值参数类型
	int type;
	if (!expression(type)) {  //调用分析表达式
		return false;
	}
	typeList.push_back(type);
	//分析表达式成功 并预读了一个单词
	//开始分析{,＜表达式＞}
	while (true) {
		if (isEOF()) {
			break;
		}
		if (symbol != COMMA) {  //不是逗号
			break;
		}
		//当前是逗号 看下一个是不是表达式
		doOutput();
		int re = getsym();
		if (re < 0) {
			return false;
		}
		if (!expression(type)) {  //调用分析表达式
			return false;
		}
		//分析表达式成功 并预读了一个单词
		typeList.push_back(type);
	}
	if (typeList.size() != globalSymbolTable[funcName].parameterTable.size()) {
		errorfile << line << " d\n";  //参数个数不匹配
	}
	else {
		for (int i = 0; i < typeList.size(); i++) {
			if (typeList[i] != globalSymbolTable[funcName].parameterTable[i]) {
				errorfile << line << " e\n";  //参数类型不匹配
				break;
			}
		}
	}
	outputfile << "<值参数表>" << endl;
	return true;
}

//＜语句列＞   ::= ｛＜语句＞｝
bool statementList() {
	while (true) {
		if (!statement()) {  //判断是不是语句
			break;
		}
	}
	outputfile << "<语句列>" << endl;
	return true;   //?????如果第一次进while就不是语句 会不会有问题？
}

//＜读语句＞    ::=  scanf '('＜标识符＞{,＜标识符＞}')’
bool readStatement() {
	if (symbol == SCANFTK) {
		doOutput();
		int re = getsym();
		if (re < 0) {
			return false;
		}
		if (symbol == LPARENT) {  //是(
			doOutput();
			re = getsym();
			if (re < 0) {
				return false;
			}
			if (symbol == IDENFR) {
				string name = string(token);
				if (!((localSymbolTable.find(name) != localSymbolTable.end() && localSymbolTable[name].kind != 3)
					|| (globalSymbolTable.find(name) != globalSymbolTable.end() && globalSymbolTable[name].kind != 3))
					) {
					errorfile << line << " c\n";  //未定义的名字
				}
				doOutput();
				//开始分析{,＜标识符＞}
				while (true) {
					re = getsym();
					if (re < 0) {
						return false;
					}
					if (symbol != COMMA) {  //不是,
						break;
					}
					//当前是逗号 看下一个是不是标识符
					doOutput();
					re = getsym();
					if (re < 0) {
						return false;
					}
					if (symbol != IDENFR) {//标识符
						return false;
					}
					//当前是标识符
					name = string(token);
					if (!((localSymbolTable.find(name) != localSymbolTable.end() && localSymbolTable[name].kind != 3)
						|| (globalSymbolTable.find(name) != globalSymbolTable.end() && globalSymbolTable[name].kind != 3))
						) {
						errorfile << line << " c\n";  //未定义的名字
					}
					doOutput();
				}
				if (symbol == RPARENT) {  //)
					doOutput();
					outputfile << "<读语句>" << endl;
					getsym();  //预读一个 不管是啥
					return true;
				}
				else {
					return false;
				}
			}
			else {
				return false;
			}
		}
		else {
			return false;
		}
	}
	else {
		return false;
	}
}

//＜写语句＞    ::= printf '(' ＜字符串＞,＜表达式＞ ')'| printf '('＜字符串＞ ')'| printf '('＜表达式＞')’
bool writeStatement() {
	if (symbol == PRINTFTK) {
		doOutput();
		int re = getsym();
		if (re < 0) {
			return false;
		}
		if (symbol == LPARENT) {  //是(
			doOutput();
			re = getsym();
			if (re < 0) {
				return false;
			}
			if (strings()) {  //字符串常量  预读一个单词
				if (symbol == COMMA) {  //,  printf '(' ＜字符串＞,＜表达式＞ ')'
					doOutput();
					re = getsym();
					if (re < 0) {
						return false;
					}
					int t;
					if (!expression(t)) {  //分析表达式
						return false;
					}
					//分析表达式成功 并预读了一个单词
					if (symbol == RPARENT) {  //)
						doOutput();
						outputfile << "<写语句>" << endl;
						getsym();  //预读一个 不管是啥
						return true;
					}
					else {
						return false;
					}
				}
				else if (symbol == RPARENT) {  //)  printf '('＜字符串＞ ')'
					doOutput();
					outputfile << "<写语句>" << endl;
					getsym();  //预读一个 不管是啥
					return true;
				}
				else {
					return false;
				}
			}
			else {  //判断是不是表达式  printf '('＜表达式＞')’
				int t;
				if (!expression(t)) {  //分析表达式
					return false;
				}
				//分析表达式成功 并预读了一个单词
				if (symbol == RPARENT) {  //)
					doOutput();
					outputfile << "<写语句>" << endl;
					getsym();  //预读一个 不管是啥
					return true;
				}
				else {
					return false;
				}
			}
		}
		else {
			return false;
		}
	}
	else {
		return false;
	}
}

//＜返回语句＞   ::=  return['('＜表达式＞')']  
bool returnStatement() {
	if (symbol == RETURNTK) {  //return 
		doOutput();
		//开始分析 ['('＜表达式＞')']    可有可无的
		int re = getsym();
		if (re < 0) {   // 后边可以没有
			outputfile << "<返回语句>" << endl;
			return true;
		}
		if (symbol == LPARENT) {  //是(
			doOutput();
			re = getsym();
			if (re < 0) {
				return false;
			}
			int t;
			if (!expression(t)) {  //分析表达式
				return false;
			}
			//分析表达式成功 并预读了一个单词
			if (symbol == RPARENT) {  //)
				doOutput();
				outputfile << "<返回语句>" << endl;
				getsym();  //预读一个 不管是啥
				return true;
			}
			else {
				return false;
			}
		}
		else {  //return 后边不是( 也可以 
			outputfile << "<返回语句>" << endl;
			return true;
		}
	}
	else {
		return false;
	}
}