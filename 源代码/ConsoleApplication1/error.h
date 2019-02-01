#pragma once
#include<string>
#include "Lexer.h"
using namespace std;
enum errortype {
	zero_head, noteq_loss, empty_char, error_char, mult_char, error_string, unknow_char, number_too_big
};
void error(errortype err, int line);
void error();
void error(int i);
void error(string i);
void error(string i, int mark);

void error(string i, Sym pig, bool hh);
extern int error_num;
