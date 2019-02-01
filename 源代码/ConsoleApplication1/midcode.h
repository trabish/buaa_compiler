#pragma once
#include "pch.h"
#include "Parser.h"
#include "Lexer.h"
#include "error.h"
#include <map>
#include <iostream>
using namespace std;
enum operater {
	op_ADD,op_SUB,op_MULT,op_DIV,op_int,op_char,op_void,op_call,
	op_push,op_RETvalue,op_ret,op_end,op_assign,op_addreq,op_eqaddr,
	op_printf,op_scanf,op_lable,op_GOTO,op_BNZ,op_BZ,op_EQ,op_NOTEQ,op_LESS,
	op_LESSEQ,op_MORE,op_MOREEQ,op_none  
};//op_BNZ 和op_BZ已经没有利用价值了  op_none代表空

//注意op_RET 是函数调用时的返回值   op_ret 函数内部的返回
//op_addreq  []=, op_eqaddr  =[]
struct midcode{
	operater op; //操作符
	string src1; //操作数1
	string src2;	//操作数2
	string des; //结果
};

bool find_addr(string name, int &offset);//查找标识符的偏移
/////////////////////////////////
/////   寄存器池相关   //////////
/////////////////////////////////
static int now_order = 1;//当前order
struct register_pool {
	string ID; //标识符 或者 是 常数
	bool dirty_mark;//这个值是否改变
 int order;//使用顺序  以一种简单的方式进行opt分配 初始为0
};//十个临时寄存器





extern midcode midcodes[];//中间指令最大条数

extern int midcode_index;//中间代码数量 全局变量

extern int mipscode_index;

extern string mipscodes[];

int  is_num(string name);
int str2int(const string &string_temp);
void get_tn(string &Tn);
void get_lable(string &Tn);
void get_string(string &key, string value);
int enter_midcode(operater op,string src1,string src2,string des);//添加中间码
int enter_mipscode(string instruction);//添加目标码
void out_midcode();//输出中间码
void out_mips();//输出目标码
void register_temp(string name);//在符号表中注册临时变量
void register_string(string key,string value);//在字符常量表中 注册字符串常量
void handle_table();
void mips_init();
//////主函数
void midcode2mips();

//每一条转换语句
void midcode_ADD(midcode instruction);
void midcode_SUB(midcode instruction);
void midcode_MULT(midcode instruction);
void midcode_DIV(midcode instruction);
void midcode_function(midcode instruction); //int function  ,char function ,void function
void midcode_call(midcode instruction);
void midcode_push(midcode instruction);
void midcode_RETvalue(midcode instruction);
void midcode_ret(midcode instruction);
void midcode_end(midcode instruction);
void midcode_assign(midcode instruction);
void midcode_addreq(midcode instruction);//"[] ="
void midcode_eqaddr(midcode instruction);//" =[]"
void midcode_printf(midcode instruction);
void midcode_scanf(midcode instruction);
void midcode_lable(midcode instruction);
void midcode_GOTO(midcode instruction);
//void midcode_BNZ(midcode instruction);
//void midcode_BZ(midcode instruction);

void midcode_EQ(midcode instruction);
void midcode_NOTEQ(midcode instruction);
void midcode_LESSEQ(midcode instruction);
void midcode_LESS(midcode instruction);
void midcode_MOREEQ(midcode instruction);
void midcode_MORE(midcode instruction);





