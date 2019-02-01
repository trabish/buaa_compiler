#include "pch.h"
#include "error.h"
#include "Lexer.h"
#include <iostream>
#include <map> 
#include <string>
using namespace std;

int error_num = 0;

map<errortype, string> error_map = { {zero_head,"zero_head"},{noteq_loss,"!= loss ="},{empty_char,"empty_char"}, 
{error_char,"error_char"},{mult_char,"mult_char"},{ error_string," error_string"},{unknow_char,"unknow_char"},{number_too_big,"number_too_big"} };

void error(errortype err, int line) {
	error_num++;
	//cout << "词法分析出错" << endl;
	string temp = error_map[err];
	cout <<"line:"<<line<<" "<< temp << endl;
	
	while (1) {
		getsym();
		if (symbol == SEMI || symbol == FILEEND||symbol==RL_BACKET) {
			return;
		}
	}
	exit(EXIT_SUCCESS);
}
void error(string i) {
	error_num++;
	cout << "line:" << line << " " << i << endl;
	while (1) {
		getsym();
		if (symbol == SEMI || symbol == FILEEND || symbol == RL_BACKET) {
			if (symbol == FILEEND) {
				cout << "存在错误 程序结束" << endl;
				exit(EXIT_SUCCESS);
			}
			return;
		}
	}
	exit(EXIT_SUCCESS);
}


void error(string i,int mark) {
	error_num++;
	cout << "line:" << line << " " << i << endl;
	if (mark == 0) {//不进行跳读
		return;
	}
	while (1) {
		getsym();
		if (symbol == FILEEND) {
			cout << "存在错误 程序结束" << endl;
			exit(EXIT_SUCCESS);
		}
		if (symbol == SEMI || symbol == FILEEND || symbol == RL_BACKET) {
			getsym();
			return;
		}
	}

	exit(EXIT_SUCCESS);
}

void error(string i, Sym pig,bool hh) {
	error_num++;
	cout << "line:" << line << " " << i << endl;
	if (hh == true) {
		while (1) {
			getsym();
			if (symbol == FILEEND) {
				cout << "存在错误 程序结束" << endl;
				exit(EXIT_SUCCESS);
			}
			if (symbol == SEMI || symbol == pig || symbol == FILEEND || symbol == RL_BACKET) {
				getsym();
				return;
			}
		}
	}
	else {
		while (1) {//全局声明部分
			getsym();
			if (symbol == FILEEND) {
				cout << "存在错误 程序结束" << endl;
				exit(EXIT_SUCCESS);
			}
			if ( symbol ==INT_SY || symbol == FILEEND || symbol == CHAR_SY|| symbol == VOID_SY||symbol==CONST_SY) {
				//getsym();
				return;
			}
		}
	}

	exit(EXIT_SUCCESS);
}


void error() {
	error_num++;
	cout << "line:" << line << " " << "语法错误" << endl;
	while (1) {
		getsym();
		if (symbol == FILEEND) {
			cout << "存在错误 程序结束" << endl;
			exit(EXIT_SUCCESS);
		}
		if (symbol == SEMI || symbol == FILEEND || symbol == RL_BACKET) {
			getsym();
			return;
		}
	}
	exit(EXIT_SUCCESS);
}
void error(int i) {
	error_num++;
	cout << i<<"   " << "语法错误" << endl;
}
