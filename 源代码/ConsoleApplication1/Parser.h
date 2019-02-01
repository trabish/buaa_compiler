#pragma once
#include <iostream>
#include <map>
#define Max_para 100//最大参数个数 参数栈的大小
#define Max_table 100 //最大函数个数
using namespace std;
enum obj_enum {//种类标识符 常量 变量 函数 参数
	obj_const,obj_var,obj_function,obj_para
};
enum type_enum {//种类标识符 int char int[] char [] void
	type_int,type_int_group,type_char,type_char_group,type_void
};
struct table_item {
	string name;
	obj_enum obj;//种类标识符 常量 变量 函数 参数
	type_enum type;//种类标识符 int char int[] char[] void
	int adr;//对于函数，其为函数表的地址；对于常量，填入其值
	int length;//对于数组，其为数组长度
	int offset;//标识符的偏移位置，相对于所属函数（生成中间代码或注册符号表时填写）
};
struct Table {//包含该函数的信息 
	//bool Globle_mark;//true 为全局表，false 为函数表
	string function_name;//函数名，
	int paranum;//函数参数个数
	
	table_item paralist[Max_para];//参数存放位置  注意加入参数 时 既要加入 para 也要加入maptable;
	map<string, table_item>  maptable;//主要内容
	int vsize;//局部变量 参数及内务信息区在S栈中所占的存储单元总数
};
extern Table symbolTable[500];
extern int which_Table;// symbolTable的下标 0表示这个全局表 其他的按顺序排
//需要 usedindexof_table 来记录已经使用了的Table

extern map<string, string> stringTable; //常量字符表

void enter(string name, obj_enum obj, type_enum type, int adr, int length);
void enterfunction(string function_name);
void enterparalist(string name, obj_enum obj, type_enum type, int adr, int length);//填参数
void program();
void constDec();
void constDef();
void varDec(bool program_mark);//program_mark 表示 program 直接推导出的varDec()
void varDef(bool program_mark);//program_mark 表示 program 直接推导出的varDec()中 第一个变量定义
void retfunctionDef();
void voidfunctionDef();

void mainFunction();
void compoundStatement();//复合语句
void paraList();
void Statements();//语句列
void Statement();

bool expression(string &tn);//true 代表返回类型为int false为char
bool expression(string &tn, int &calmark, int &value);//true 代表返回类型为int false为char 为了静态数组越界检测
bool term(string &tn,  int &calmark, int &value);//true 代表返回类型为int false为char
bool factor(string &tn,  int &calmark, int &value);//true 代表返回类型为int false为char

void ifStatement();
void whileStatement();
void callFunction();
void assignStatement();
void readStatement();
void writeStatement();
void returnStatement();

void switchStatement();
void situationTable(string tn, string switch_end_lable,bool case_type);


