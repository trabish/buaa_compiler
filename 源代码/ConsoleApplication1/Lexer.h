#pragma once
#include "pch.h"
#include "Parser.h"
#include <iostream>
#include <cstdio>
#include <sstream>
#include <fstream>//�ļ�����
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
extern char now_char;//�ַ���ȫ�ֱ�������ŵ�ǰ�������ַ�
extern string Token;//ȫ�ֱ������ַ����飬��ŵ��ʵ��ַ���
extern string Token2;//���ڳ�ǰ��
extern string Token3;//���ڳ�ǰ��
extern int num;//����ȫ�ֱ�������ŵ�ǰ�����������ֵ
extern enum Sym symbol;//ö����ȫ�ֱ��������浱ǰ��ʶ�𵥴ʵ�����
extern enum Sym symbol2;//���ڳ�ǰ��
extern enum Sym symbol3;//���ڳ�ǰ��
//char S[LEN_S];//Դ����
extern string S;
extern int index;//�±�
extern int line;

extern type_enum new_type;//��¼���� ������type   ֻ�ñ�ʾ  char int void ���ڷ��ű���д ��reserver��ʵ��
extern string new_ID;//��¼���¶����� ��ʶ�� ��reserver��ʵ��
