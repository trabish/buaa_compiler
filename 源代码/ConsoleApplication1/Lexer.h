#pragma once
#include "pch.h"
#include "Parser.h"
#include <iostream>
#include <cstdio>
#include <sstream>
#include <fstream>//文件操作
using namespace std;

#define LEN_S 10000

enum Sym {
	ID, CONST_SY, INT_SY, CHAR_SY, VOID_SY, MAIN_SY, IF_SY, WHILE_SY,
	SWITCH_SY, CASE_SY, DEFAULT_SY, SCANF_SY, PRINTF_SY, RETURN_SY,
	ADD, SUB, MULT, DIV, LESS, LESSEQ, MORE, MOREEQ, NOTEQ, EQ, STRING, CHAR, UINT,
	COMMA, SEMI, COLON, LS_BACKET, RS_BACKET, LM_BACKET, RM_BACKET, LL_BACKET, RL_BACKET,
	ASSIGN, FILEEND
};
extern const char* RESERVE_SY[13];


void getsym();
void retract(void);
void nextchar(void);
int reserver();
void clearToken();
void transNum();
//void error();
extern char now_char;//字符型全局变量，存放当前读进的字符
extern string Token;//全局变量，字符数组，存放单词的字符串
extern string Token2;//用于超前读
extern string Token3;//用于超前读
extern int num;//整型全局变量，存放当前读入的整型数值
extern enum Sym symbol;//枚举型全局变量，保存当前所识别单词的类型
extern enum Sym symbol2;//用于超前读
extern enum Sym symbol3;//用于超前读
//char S[LEN_S];//源程序
extern string S;
extern int index;//下标
extern int line;

extern type_enum new_type;//记录最新 读到的type   只用表示  char int void 用于符号表填写 在reserver中实现
extern string new_ID;//记录最新读到的 标识符 在reserver中实现
