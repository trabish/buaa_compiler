#include "pch.h"
#include "midcode.h"
#include "Parser.h"
#include "Lexer.h"
#include "error.h"
#include "opt.h"
#include <map>
#include <iostream>
#include <sstream>
#include <stack>
#include <fstream>
#define MAX_midcode 5000
#define MAX_mipscode 5000
using namespace std;
midcode midcodes[MAX_midcode];
string mipscodes[MAX_mipscode];

////////
//-------$fp
//
//
//-------$last $fp
//-------$retvalue
//-------$ra
//-------$sp
//


//对应下面三个生成函数
int tn_index = 0;//get_tn
int lable_index = 0;//get_lable
int string_index = 0;//get_string

int midcode_index = 0;
int mipscode_index = 0;


static int SymbolTable_index = 0;//当前btab
static int now_para_num = 0;//现在已有参数数量
static string para_list_array[Max_para];//存放参数的数组  push时参数以顺序存放  call时 会以顺序从para_list中取出并存入运行栈

map<operater, string> operater_map = { {op_ADD,"ADD"},{op_SUB,"SUB"},{op_MULT,"MULT"},{op_DIV,"DIV"},{op_int,"int"},{op_char,"char"},{op_void,"void"},
	{op_call,"call"},{op_push,"push"},{op_RETvalue,"RETvalue"},{op_ret,"ret"},{op_end,"end"},{op_assign,"assign"},
	{op_addreq,"[]="},{op_eqaddr,"=[]"},{op_printf,"printf"},{op_scanf,"scanf"},{op_lable,"lable"},{op_GOTO,"GOTO"},
	{op_BNZ,"BNZ"},{op_BZ,"BZ"},{op_EQ,"EQ"},{op_NOTEQ,"NOTEQ"},{op_LESS,"LESS"},{op_LESSEQ,"LESSEQ"},
	{op_MORE,"MORE"},{op_MOREEQ,"MOREEQ"}
};
////////////////////////////////////////////////
/////////////寄存器池//////////////////////////
////////////////////////////////////////////////

register_pool reg_pool[10];
bool search_reg_pool(string name, string & temp_reg) {//搜索reg_pool 
	for (int i = 0; i < 10; i++) {
		if (reg_pool[i].ID == name) {
			temp_reg = "$t" + to_string(i);//得到那个寄存器
			reg_pool[i].order = now_order++;//更新order 说明最近使用
			return true;
		}
	}
	return false;//没有找到
}


void get_next_temp_reg(string name, string &temp_reg) {//分配下一个临时寄存器
	int min_index = 0;
	int min_order = reg_pool[0].order;
	for (int i = 1; i < 10; i++) {
		if (reg_pool[i].order < min_order) {
			min_index = i;
			min_order = reg_pool[i].order;
		}
	}
	//将最小order的那个寄存器分配出去
	//如果 那个寄存器是脏的 写回内存
	if (reg_pool[min_index].dirty_mark == true) {
		int offset;
		if (find_addr(reg_pool[min_index].ID, offset) == true) {//局部变量
			enter_mipscode("sw $t" + to_string(min_index) + " " + to_string(offset) + "($sp)");  //sw $ti offset($sp)
		}
		else {//全局变量
			enter_mipscode("sw $t" + to_string(min_index) + " " + to_string(offset));  //sw $ti offset
		}
	}
	temp_reg = "$t" + to_string(min_index);
	reg_pool[min_index].ID = name;
	reg_pool[min_index].order = now_order++;
	reg_pool[min_index].dirty_mark = false;
}
void initail_reg_pool() {//进入基本块之前 初始化 寄存器池
	for (int i = 0; i < 10; i++) {
		reg_pool[i].ID = "";
		reg_pool[i].order = 0;
		reg_pool[i].dirty_mark = false;
	}
	now_order = 1;
}
void dirty_reg_pool(string reg) {
	reg_pool[reg[2] - '0'].dirty_mark = true;
}
void empty_reg_pool() {//出基本块 将内容放回内存
	for (int i = 0; i < 10; i++) {
		if (reg_pool[i].ID.empty()) {

		}
		else if (reg_pool[i].dirty_mark == true) {//不为空 且脏 需要放回内存
			int offset;
			if (find_addr(reg_pool[i].ID, offset) == true) {//局部变量
				enter_mipscode("sw $t" + to_string(i) + " " + to_string(offset) + "($sp)");  //sw $ti offset($sp)
			}
			else {//全局变量
				enter_mipscode("sw $t" + to_string(i) + " " + to_string(offset));  //sw $ti offset
			}
		}
	}
	initail_reg_pool();//为下一次初始化
}
bool alloc_reg_pool(string name, string & temp_reg) {//分配临时寄存器器 true 代表已经在寄存器池中
	if (search_reg_pool(name, temp_reg) == true) {
		return true;//分配完成
	}
	else {
		get_next_temp_reg(name, temp_reg);
		return false;
	}
}

void alloc_temp_reg(string &temp_reg) {//分配一个临时寄存器 不记录其信息
	int min_index = 0;
	int min_order = reg_pool[0].order;
	for (int i = 0; i < 10; i++) {
		if (reg_pool[i].order < min_order) {
			min_index = i;
			min_order = reg_pool[i].order;
		}
	}
	//将最小order的那个寄存器分配出去
	//如果 那个寄存器是脏的 写回内存
	if (reg_pool[min_index].dirty_mark == true) {
		int offset;
		if (find_addr(reg_pool[min_index].ID, offset) == true) {//局部变量
			enter_mipscode("sw $t" + to_string(min_index) + " " + to_string(offset) + "($sp)");  //sw $ti offset($sp)
		}
		else {//全局变量
			enter_mipscode("sw $t" + to_string(min_index) + " " + to_string(offset));  //sw $ti offset
		}
	}
	temp_reg = "$t" + to_string(min_index);
	reg_pool[min_index].ID = "";
	reg_pool[min_index].dirty_mark = false;
	reg_pool[min_index].order = 0;//因为这是一个暂时的寄存器 会被优先分配出去
}


////////////////////////////////////////////
void get_tn(string &Tn) { //临时变量 并将其加入符号表
	stringstream ss;
	ss << tn_index++;
	Tn = "@T" + ss.str();
	register_temp(Tn);//加入符号表
}
void get_lable(string &Tn) { //跳转的lable
	stringstream ss;
	ss << lable_index++;
	Tn = "lable" + ss.str();
}

void get_string(string &key,string value) {
	stringstream ss;
	ss << string_index++;
	key = "$string" + ss.str(); // $xxx 可以作为mips的标识符 单不能作为我们编译器的标识符
	register_string(key, value);
}

int str2int( const string &string_temp)
{
	int int_temp;
	stringstream stream(string_temp);
	stream >> int_temp;
	return int_temp;
}

//添加中间代码
int enter_midcode(operater op, string src1, string src2, string des) {
	if (midcode_index>=MAX_midcode) {
		error("midcodes 表溢出");//midcodes 表溢出
		return 0;
	}
	midcodes[midcode_index].op = op;
	midcodes[midcode_index].src1 = src1;
	midcodes[midcode_index].src2 = src2;
	midcodes[midcode_index].des = des;
	midcode_index++;
	return midcode_index - 1;//返回指令下标
}

//添加mips 汇编码
int enter_mipscode(string instruction) {
	if (mipscode_index >= MAX_mipscode) {
		error("mipscodes 表溢出");
		return 0;
	}
	mipscodes[mipscode_index] = instruction;
	mipscode_index++;
	return mipscode_index - 1;//返回指令下标
}

//输出中间码
void out_midcode() {
	ofstream fout;
	fout.open("midcodes.txt");
	/*for (int i = 0; i< midcode_index; i++) {
		if (midcodes[i].op == op_none) {
			continue;
		}
		fout << operater_map[midcodes[i].op] << "  " << midcodes[i].src1 << "  " << midcodes[i].src2 << "  " << midcodes[i].des << endl;
	}*/
	for (int i = 0; i < midcode_index; i++) {
		if (midcodes[i].op == op_none) {
			continue;
		}
		switch (midcodes[i].op)
		{
		case op_ADD://二元运算
			fout << midcodes[i].des << " = " << midcodes[i].src1 << " + " << midcodes[i].src2 << endl;
			break;
		case op_SUB:
			fout << midcodes[i].des << " = " << midcodes[i].src1 << " - " << midcodes[i].src2 << endl;
			break;
		case op_MULT:
			fout << midcodes[i].des << " = " << midcodes[i].src1 << " * " << midcodes[i].src2 << endl;
			break;
		case op_DIV:
			fout << midcodes[i].des << " = " << midcodes[i].src1 << " / " << midcodes[i].src2 << endl;

			break;
		case op_int:
		case op_char:
		case op_void:
			fout << endl;
			fout << operater_map[midcodes[i].op] << " " << midcodes[i].des << "()" << endl;
			break;
		case op_call:
			fout << "call " << midcodes[i].des  << endl;
			break;
		case op_push:
			fout << "push " << midcodes[i].src1 << endl;
			break;
		case op_RETvalue:
			fout << midcodes[i].des << " = RET" << endl;
			break;
		case op_ret:
			fout << "ret " << midcodes[i].des << endl;
			break;
		case op_end:
			fout << "end" << endl;
			break;
		case op_assign:
			
			fout << midcodes[i].des << " = " << midcodes[i].src1<<endl;
			

			break;
		case op_addreq://[]= 
			fout << midcodes[i].des << "[" << midcodes[i].src1<<"] = "<<midcodes[i].src2<<endl;

			break;
		case op_eqaddr:
			//参数一
			fout << midcodes[i].des <<" = " << midcodes[i].src2 << "[" << midcodes[i].src1 << "]"<<endl;
			break;

		case op_printf:
			fout << operater_map[midcodes[i].op] << "  " << midcodes[i].src1 << "  " << midcodes[i].src2 << "  " << midcodes[i].des << endl;
			break;
		case op_scanf:
			fout << operater_map[midcodes[i].op] << "  " << midcodes[i].src1 << "  " << midcodes[i].src2 << "  " << midcodes[i].des << endl;
			break;
		case op_lable:
			fout << operater_map[midcodes[i].op] << "  " << midcodes[i].src1 << "  " << midcodes[i].src2 << "  " << midcodes[i].des << endl;
			break;
		case op_GOTO:
			fout << operater_map[midcodes[i].op] << "  " << midcodes[i].src1 << "  " << midcodes[i].src2 << "  " << midcodes[i].des << endl;
			break;

		case op_EQ:
		case op_NOTEQ:
		case op_LESSEQ:
		case op_LESS:
		case op_MOREEQ:
		case op_MORE:
			//参数一
			fout << operater_map[midcodes[i].op] << "  " << midcodes[i].src1 << "  " << midcodes[i].src2 << "  " << midcodes[i].des << endl;
			break;


		default:
			error("中间码有bug");
			return ;
			break;
		}
		
	}
}
//输出目标码
void out_mips() {
	ofstream fout;
	fout.open("mips.txt");
	for (int i = 0; i < mipscode_index; i++) {
		//cout << mipscodes[i] << endl;
		fout << mipscodes[i] << endl;
	}
}
void register_temp(string name) {//注册临时变量
	if (symbolTable[which_Table].maptable.count(name)!=0) {
		return; //填过 不需要再次填写
	}
	table_item temp;
	temp.name = name;// 分配和int 一样的空间 
	temp.obj = obj_var;
	temp.type = type_int;//全部定义未int  !!!有问题
	symbolTable[which_Table].maptable[name] = temp;
}

void register_string(string key,string value) {
	if (stringTable.count(key) != 0) {
		error("怎么可能呢？");
		return ;
	}
	else {
		stringTable[key] = value;
	}
}
 
void calculate_offset(int now_table) {//计算该表中符号所需空间和 offset  注意栈向下生长
	int offset = 12+4*8;//需要为 $ra 和 ret 分配空间 和上一个 $fp   //为$s寄存器保存设置空间
	map<string, table_item>::iterator iter;
	iter = symbolTable[now_table].maptable.begin();
	while (iter != symbolTable[now_table].maptable.end()) {
		if (iter->second.obj == obj_var|| iter->second.obj==obj_para) {//变量 和 参量 才分配空间
			iter->second.offset = offset;
			if (iter->second.type == type_int_group || iter->second.type == type_char_group) {//数组
				offset = offset +4 * iter->second.length;
			}
			else {
				offset = offset + 4;//int char 空间大小一样
			}
		}
		iter++;
	}

	symbolTable[now_table].vsize = offset; //大小等于offset
}
void handle_table() {//处理符号表   忽略常量 计算offset  

	int now_table=1;//开始是第一个函数
	while (1) {
		int mark=0;
		if (symbolTable[now_table].function_name == "main") {
			mark = 1;
		}

		calculate_offset(now_table);

		if (mark == 1) {
			break;
		}
		now_table++;
	}
}
void mips_init() { //计算offset    设置字符串常量 全局变量
	
	{//为了解决参数值的问题 未参数申请全局变量
		table_item temp_item;
		string name = "$para" ;
		temp_item.name = name;
		temp_item.obj = obj_var;
		temp_item.type = type_int_group;

		temp_item.length = Max_para;
		symbolTable[0].maptable[name] = temp_item;

	}

	handle_table();//计算offset

	//为全局变量分配空间
	enter_mipscode(".data");//数据区 存放常量字符串 和 全局变量(提前计算出全局变量的位置)
	int globle_offset = 0x10010000;//全局变量的 偏移位置   换为default
	map<string, table_item>::iterator iter;
	iter = symbolTable[0].maptable.begin();
	while (iter != symbolTable[0].maptable.end()) {
		if (iter->second.obj == obj_var) {//只为变量分配空间 
			switch (iter->second.type) {
			case type_char :
			case type_int:
				iter->second.offset = globle_offset;
				globle_offset = globle_offset + 4;
				enter_mipscode(iter->second.name+": .word 0");//如果不加0 不会分配空间
				break;
			case type_int_group: 
			case type_char_group:
				iter->second.offset = globle_offset;
				globle_offset =globle_offset+ 4 * iter->second.length;
				enter_mipscode(iter->second.name + ": .word 0:" + to_string(iter->second.length));//为数组非分配空间 
			}
		}
		iter++;
	}

	//为常量字符串 分配空间 在字符串表中
	map<string, string>::iterator string_iter;
	string_iter = stringTable.begin();
	while (string_iter!=stringTable.end()) {
		enter_mipscode(string_iter->first+": .asciiz "+"\""+string_iter->second+"\\n\"");  //注意要输出双引号 
		string_iter++;
	}

	//初始化 $sp $fp  并跳转到主函数
	enter_mipscode(".text");//代码段
	//enter_mipscode("move $fp $sp");//$sp最开始有值  
	enter_mipscode("subi $sp $sp "+to_string(symbolTable[symbolTable[0].maptable["main"].adr].vsize));//为main 分配空间
	enter_mipscode("j main");
}

bool find_addr(string name,int &offset) {//从符号表中查找 变量的地址    如果是全局变量返回false 如果是局部量返回true
	if (symbolTable[SymbolTable_index].maptable.count(name) != 0) {
		//局部变量
		offset = symbolTable[SymbolTable_index].maptable[name].offset;
		return true;
	}
	else if (symbolTable[0].maptable.count(name) != 0) {
		offset = symbolTable[0].maptable[name].offset;
		return false;
	}
	else {
		cout << name << endl;

		error("你肯定有bug");

		return false;
	}
}

int  is_num(string name) { // -1 空字符串    0 非数组     1 数字
	if (name.size() == 0) {
		return -1;//空string
	}
	else if(name.size()==1){
		if (name[0] >= '0'&&name[0] <= '9')
		{
			return 1;//数字
		}
		else {
			return 0;
		}
	}
	else {
		int i = 0;
		if (name[0] == '-') {
			i++;
		}
		for (; i < name.size(); i++) {
			if (name[i]<'0' || name[i]>'9') {
				break;
			}
		}
		if (i != name.size()) {
			return 0;//非数字
		}
		else {
			return 1;//数字
		}
	}

}
/////////////////////////////////////////////////////////////////////////////////
////                              处理每一种midcode                          ////
/////////////////////////////////////////////////////////////////////////////////
void midcode_ADD(midcode instruction) { //add $t0 $t0 $t1
	int offset_src1;
	int offset_src2;
	int offset_des;

	string reg_1;
	string reg_2;
	string reg_3;
	if (is_num(instruction.src2) == 1) {//数字 可以用addi
	//src1
		if (is_num(instruction.src1) == 1) {//数字
			if (alloc_reg_pool(instruction.src1, reg_1) == false) {//没在寄存器池中
				enter_mipscode("li " + reg_1 + " " + instruction.src1);
			}
		}
		else {//非数字
			if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.src1) != 0)//已经分配了全局寄存器
			{
				reg_1 = function_Reference_data[SymbolTable_index].map_reg[instruction.src1];//获得该全局寄存器	
			}
			else {//进行临时寄存器分配
				if (alloc_reg_pool(instruction.src1, reg_1) == false) {//没在寄存器池中
					if (find_addr(instruction.src1, offset_src1) == true) {//局部变量
						enter_mipscode("lw " + reg_1 + " " + to_string(offset_src1) + "($sp)");  //lw $ti offset($sp)
					}
					else {//全局变量
						enter_mipscode("lw " + reg_1 + " " + to_string(offset_src1));  //lw $ti offset
					}
				}
			}

		}
		

		//des
		if (is_num(instruction.des) == 0) {//非数字 非空
			if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.des) != 0)//已经分配了全局寄存器
			{
				reg_3 = function_Reference_data[SymbolTable_index].map_reg[instruction.des];//获得该全局寄存器

			}
			else {//进行临时寄存器分配
				alloc_reg_pool(instruction.des, reg_3);
				//将其标记为脏 因为是des
				dirty_reg_pool(reg_3);
			}
		}
		else {
			error("我不相信能到这里");
			return ;
		}
		enter_mipscode("addi " + reg_3 + " " + reg_1 + " " + instruction.src2);
	}
	else {
		if (is_num(instruction.src1) == 1) {//数字
			if (alloc_reg_pool(instruction.src1, reg_1) == false) {//没在寄存器池中
				enter_mipscode("li " + reg_1 + " " + instruction.src1);
			}
		}
		else {//非数字
			if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.src1) != 0)//已经分配了全局寄存器
			{
				reg_1 = function_Reference_data[SymbolTable_index].map_reg[instruction.src1];//获得该全局寄存器	
			}
			else {//进行临时寄存器分配
				if (alloc_reg_pool(instruction.src1, reg_1) == false) {//没在寄存器池中
					if (find_addr(instruction.src1, offset_src1) == true) {//局部变量
						enter_mipscode("lw " + reg_1 + " " + to_string(offset_src1) + "($sp)");  //lw $ti offset($sp)
					}
					else {//全局变量
						enter_mipscode("lw " + reg_1 + " " + to_string(offset_src1));  //lw $ti offset
					}
				}
			}

		}
		//src2
		if (is_num(instruction.src2) == 1) {//数字
			if (alloc_reg_pool(instruction.src2, reg_2) == false) {//没在寄存器池中
				enter_mipscode("li " + reg_2 + " " + instruction.src2);
			}
		}
		else {//非数字
			if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.src2) != 0)//已经分配了全局寄存器
			{
				reg_2 = function_Reference_data[SymbolTable_index].map_reg[instruction.src2];//获得该全局寄存器

			}
			else {//进行临时寄存器分配
				if (alloc_reg_pool(instruction.src2, reg_2) == false) {//没在寄存器池中
					if (find_addr(instruction.src2, offset_src2) == true) {//局部变量
						enter_mipscode("lw " + reg_2 + " " + to_string(offset_src2) + "($sp)");  //lw $ti offset($sp)
					}
					else {//全局变量
						enter_mipscode("lw " + reg_2 + " " + to_string(offset_src2));  //lw $ti offset
					}
				}
			}
		}

		//des
		if (is_num(instruction.des) == 0) {//非数字 非空
			if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.des) != 0)//已经分配了全局寄存器
			{
				reg_3 = function_Reference_data[SymbolTable_index].map_reg[instruction.des];//获得该全局寄存器

			}
			else {//进行临时寄存器分配
				alloc_reg_pool(instruction.des, reg_3);
				//将其标记为脏 因为是des
				dirty_reg_pool(reg_3);
			}
		}
		else {
			error("我不相信能到这里");
			return;
		}
		enter_mipscode("addu " + reg_3 + " " + reg_1 + " " + reg_2);
	}
}

void midcode_SUB(midcode instruction) { //sub $t0 $t0 $t1
	int offset_src1;
	int offset_src2;
	int offset_des;

	string reg_1;
	string reg_2;
	string reg_3;
	//src1


	if (is_num(instruction.src2) == 1) {//数字 可以用subi

		if (is_num(instruction.src1) == 1) {//数字
			if (alloc_reg_pool(instruction.src1, reg_1) == false) {//没在寄存器池中
				enter_mipscode("li " + reg_1 + " " + instruction.src1);
			}
		}
		else {//非数字
			if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.src1) != 0)//已经分配了全局寄存器
			{
				reg_1 = function_Reference_data[SymbolTable_index].map_reg[instruction.src1];//获得该全局寄存器	
			}
			else {//进行临时寄存器分配
				if (alloc_reg_pool(instruction.src1, reg_1) == false) {//没在寄存器池中
					if (find_addr(instruction.src1, offset_src1) == true) {//局部变量
						enter_mipscode("lw " + reg_1 + " " + to_string(offset_src1) + "($sp)");  //lw $ti offset($sp)
					}
					else {//全局变量
						enter_mipscode("lw " + reg_1 + " " + to_string(offset_src1));  //lw $ti offset
					}
				}
			}

		}
		

		//des
		if (is_num(instruction.des) == 0) {//非数字 非空
			if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.des) != 0)//已经分配了全局寄存器
			{
				reg_3 = function_Reference_data[SymbolTable_index].map_reg[instruction.des];//获得该全局寄存器

			}
			else {//进行临时寄存器分配
				alloc_reg_pool(instruction.des, reg_3);
				//将其标记为脏 因为是des
				dirty_reg_pool(reg_3);
			}
		}
		else {
			error("我不相信能到这里");
			return;
		}
		enter_mipscode("subi " + reg_3 + " " + reg_1 + " " + instruction.src2);
	}
	else {//src2 非数字
		if (is_num(instruction.src1) == 1) {//数字
			if (alloc_reg_pool(instruction.src1, reg_1) == false) {//没在寄存器池中
				enter_mipscode("li " + reg_1 + " " + instruction.src1);
			}
		}
		else {//非数字
			if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.src1) != 0)//已经分配了全局寄存器
			{
				reg_1 = function_Reference_data[SymbolTable_index].map_reg[instruction.src1];//获得该全局寄存器	
			}
			else {//进行临时寄存器分配
				if (alloc_reg_pool(instruction.src1, reg_1) == false) {//没在寄存器池中
					if (find_addr(instruction.src1, offset_src1) == true) {//局部变量
						enter_mipscode("lw " + reg_1 + " " + to_string(offset_src1) + "($sp)");  //lw $ti offset($sp)
					}
					else {//全局变量
						enter_mipscode("lw " + reg_1 + " " + to_string(offset_src1));  //lw $ti offset
					}
				}
			}

		}
		//src2
		if (is_num(instruction.src2) == 1) {//数字
			if (alloc_reg_pool(instruction.src2, reg_2) == false) {//没在寄存器池中
				enter_mipscode("li " + reg_2 + " " + instruction.src2);
			}
		}
		else {//非数字
			if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.src2) != 0)//已经分配了全局寄存器
			{
				reg_2 = function_Reference_data[SymbolTable_index].map_reg[instruction.src2];//获得该全局寄存器

			}
			else {//进行临时寄存器分配
				if (alloc_reg_pool(instruction.src2, reg_2) == false) {//没在寄存器池中
					if (find_addr(instruction.src2, offset_src2) == true) {//局部变量
						enter_mipscode("lw " + reg_2 + " " + to_string(offset_src2) + "($sp)");  //lw $ti offset($sp)
					}
					else {//全局变量
						enter_mipscode("lw " + reg_2 + " " + to_string(offset_src2));  //lw $ti offset
					}
				}
			}
		}

		//des
		if (is_num(instruction.des) == 0) {//非数字 非空
			if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.des) != 0)//已经分配了全局寄存器
			{
				reg_3 = function_Reference_data[SymbolTable_index].map_reg[instruction.des];//获得该全局寄存器

			}
			else {//进行临时寄存器分配
				alloc_reg_pool(instruction.des, reg_3);
				//将其标记为脏 因为是des
				dirty_reg_pool(reg_3);
			}
		}
		else {
			error("我不相信能到这里");
			return;
		}
		enter_mipscode("sub " + reg_3 + " " + reg_1 + " " + reg_2);
	}
}

void midcode_MULT(midcode instruction) { //sub $t0 $t0 $t1
	int offset_src1;
	int offset_src2;
	int offset_des;

	string reg_1;
	string reg_2;
	string reg_3;
	//src1
	if (is_num(instruction.src1) == 1) {//数字
		if (alloc_reg_pool(instruction.src1, reg_1) == false) {//没在寄存器池中
			enter_mipscode("li " + reg_1 + " " + instruction.src1);
		}
	}
	else {//非数字
		if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.src1) != 0)//已经分配了全局寄存器
		{
			reg_1 = function_Reference_data[SymbolTable_index].map_reg[instruction.src1];//获得该全局寄存器	
		}
		else {//进行临时寄存器分配
			if (alloc_reg_pool(instruction.src1, reg_1) == false) {//没在寄存器池中
				if (find_addr(instruction.src1, offset_src1) == true) {//局部变量
					enter_mipscode("lw " + reg_1 + " " + to_string(offset_src1) + "($sp)");  //lw $ti offset($sp)
				}
				else {//全局变量
					enter_mipscode("lw " + reg_1 + " " + to_string(offset_src1));  //lw $ti offset
				}
			}
		}

	}
	//src2
	if (is_num(instruction.src2) == 1) {//数字
		if (alloc_reg_pool(instruction.src2, reg_2) == false) {//没在寄存器池中
			enter_mipscode("li " + reg_2 + " " + instruction.src2);
		}
	}
	else {//非数字
		if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.src2) != 0)//已经分配了全局寄存器
		{
			reg_2 = function_Reference_data[SymbolTable_index].map_reg[instruction.src2];//获得该全局寄存器

		}
		else {//进行临时寄存器分配
			if (alloc_reg_pool(instruction.src2, reg_2) == false) {//没在寄存器池中
				if (find_addr(instruction.src2, offset_src2) == true) {//局部变量
					enter_mipscode("lw " + reg_2 + " " + to_string(offset_src2) + "($sp)");  //lw $ti offset($sp)
				}
				else {//全局变量
					enter_mipscode("lw " + reg_2 + " " + to_string(offset_src2));  //lw $ti offset
				}
			}
		}
	}

	//des
	if (is_num(instruction.des) == 0) {//非数字 非空
		if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.des) != 0)//已经分配了全局寄存器
		{
			reg_3 = function_Reference_data[SymbolTable_index].map_reg[instruction.des];//获得该全局寄存器

		}
		else {//进行临时寄存器分配
			alloc_reg_pool(instruction.des, reg_3);
			//将其标记为脏 因为是des
			dirty_reg_pool(reg_3);
		}
	}
	else {
		error("我不相信能到这里");
		return;
	}
	enter_mipscode("mult " + reg_1 + " " + reg_2 );
	enter_mipscode("mflo " + reg_3);
	
}

void midcode_DIV(midcode instruction) { //sub $t0 $t0 $t1
	int offset_src1;
	int offset_src2;
	int offset_des;

	string reg_1;
	string reg_2;
	string reg_3;
	//src1
	if (is_num(instruction.src1) == 1) {//数字
		if (alloc_reg_pool(instruction.src1, reg_1) == false) {//没在寄存器池中
			enter_mipscode("li " + reg_1 + " " + instruction.src1);
		}
	}
	else {//非数字
		if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.src1) != 0)//已经分配了全局寄存器
		{
			reg_1 = function_Reference_data[SymbolTable_index].map_reg[instruction.src1];//获得该全局寄存器	
		}
		else {//进行临时寄存器分配
			if (alloc_reg_pool(instruction.src1, reg_1) == false) {//没在寄存器池中
				if (find_addr(instruction.src1, offset_src1) == true) {//局部变量
					enter_mipscode("lw " + reg_1 + " " + to_string(offset_src1) + "($sp)");  //lw $ti offset($sp)
				}
				else {//全局变量
					enter_mipscode("lw " + reg_1 + " " + to_string(offset_src1));  //lw $ti offset
				}
			}
		}

	}
	//src2
	if (is_num(instruction.src2) == 1) {//数字
		if (alloc_reg_pool(instruction.src2, reg_2) == false) {//没在寄存器池中
			enter_mipscode("li " + reg_2 + " " + instruction.src2);
		}
	}
	else {//非数字
		if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.src2) != 0)//已经分配了全局寄存器
		{
			reg_2 = function_Reference_data[SymbolTable_index].map_reg[instruction.src2];//获得该全局寄存器

		}
		else {//进行临时寄存器分配
			if (alloc_reg_pool(instruction.src2, reg_2) == false) {//没在寄存器池中
				if (find_addr(instruction.src2, offset_src2) == true) {//局部变量
					enter_mipscode("lw " + reg_2 + " " + to_string(offset_src2) + "($sp)");  //lw $ti offset($sp)
				}
				else {//全局变量
					enter_mipscode("lw " + reg_2 + " " + to_string(offset_src2));  //lw $ti offset
				}
			}
		}
	}

	//des
	if (is_num(instruction.des) == 0) {//非数字 非空
		if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.des) != 0)//已经分配了全局寄存器
		{
			reg_3 = function_Reference_data[SymbolTable_index].map_reg[instruction.des];//获得该全局寄存器

		}
		else {//进行临时寄存器分配
			alloc_reg_pool(instruction.des, reg_3);
			//将其标记为脏 因为是des
			dirty_reg_pool(reg_3);
		}
	}
	else {
		error("我不相信能到这里");
		return;
	}
	enter_mipscode("div " + reg_1 + " " + reg_2);
	enter_mipscode("mflo " + reg_3);
}


void midcode_function(midcode instruction) { //int function  ,char function ,void function
	if (instruction.src1 != "function") {
		error("不可能 我不相信这儿有bug");
		return;
	 }
	else {

	

		empty_reg_pool();//清空寄存器池
		

		SymbolTable_index = symbolTable[0].maptable[instruction.des].adr; //切换btab  同时也切换了全局寄存器分配的表

		enter_mipscode(symbolTable[SymbolTable_index].function_name + ":"); //为函数设置 lable

		//保存有价值的$s
		map<string, string>::iterator iter;//迭代器
		iter = function_Reference_data[SymbolTable_index].map_reg.begin();
		int i = 12;
		while (iter != function_Reference_data[SymbolTable_index].map_reg.end()) {
			enter_mipscode("sw " + iter->second + " " + to_string(i) + "($sp)");
			iter++;
			i = i + 4;
		}
		//初始化全局寄存器分配 加载参数的值  因为只有参数有初值的 全局变量未参与全局寄存器分配
		//需要在切换btab后做 
		map<string, string>::iterator iter2;//迭代器
		iter2 = function_Reference_data[SymbolTable_index].map_reg.begin();
		while (iter2 != function_Reference_data[SymbolTable_index].map_reg.end()) {
			int offset;
			//根据ID 看是否是参数 
			string now_name = iter2->first;
			if (symbolTable[SymbolTable_index].maptable.count(now_name) != 0 && symbolTable[SymbolTable_index].maptable[now_name].obj == obj_para) {//如果是参数
			   //是参数
				find_addr(now_name, offset);
				enter_mipscode("lw " + iter2->second + " " + to_string(offset) + "($sp)");
			}


			iter2++;
		}

		
		//这里不需要获取参数 获取参数在call function 里面		
		enter_mipscode("sw $ra 0($sp)");//需要保存$ra 
	}
}

void midcode_push(midcode instruction) {
	int offset;
	if (now_para_num >= Max_para) {
		error("讲道理 前面就该报错");
		return;
	}
	else {
		//如果是全局变量 就将其值存入$para数组中


		para_list_array[now_para_num++] = instruction.src1;

		if (is_num(instruction.src1) == 1) {//数字 不管
			
		}
		else if(find_addr(instruction.src1, offset) == false){//全局变量

			find_addr("$para", offset);//找到$para的位置

			string reg_1;
			int offset_src1;
			if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.src1) != 0)//已经分配了全局寄存器
			{
				reg_1 = function_Reference_data[SymbolTable_index].map_reg[instruction.src1];//获得该全局寄存器	
				//将值保存在$para 数组中

				enter_mipscode("sw " + reg_1 + " " + to_string(offset + (now_para_num - 1) * 4));
				
			}
			else {//进行临时寄存器分配
				if (alloc_reg_pool(instruction.src1, reg_1) == false) {//没在寄存器池中
					enter_mipscode("lw " + reg_1 + " " + instruction.src1);
					enter_mipscode("sw " + reg_1 + " " + to_string(offset + (now_para_num - 1) * 4));
				}
				else {
					enter_mipscode("sw " + reg_1 + " " + to_string(offset + (now_para_num - 1) * 4));
				}
			}

		}
	}
}

void midcode_call(midcode instruction) {//1.$fg保存 2.$开辟新空间 3.将参数存入运行栈 4.跳转到函数

	
	int offset;

	Table now_table = symbolTable[symbolTable[0].maptable[instruction.des].adr];
	int vsize = now_table.vsize;//该函数运行栈大小
	



	//存入参数
	//位置按顺序一样对应
	now_para_num = now_para_num - now_table.paranum;
	//for (int i = 0; i < now_para_num; i++) {
	for (int i = 0; i < now_table.paranum; i++) {
		string temp_para = para_list_array[i + now_para_num];
		if (is_num(temp_para) == 1) {//数字
			enter_mipscode("li $a0 " + temp_para);//设置立即数
			//放入参数指定位置  由于之前没在参数表中存放 offset  所有需要通过参数名字在maptable中查询  呵
			enter_mipscode("sw $a0 " + to_string(now_table.maptable[now_table.paralist[i].name].offset- vsize) + "($sp)");
		}
		else if (find_addr(temp_para, offset) == false) {//全局变量 需要从$para 中获取值

			find_addr("$para", offset);//找到$para的位置
			enter_mipscode("lw $a0 " + to_string(offset + (i + now_para_num ) * 4));
			enter_mipscode("sw $a0 " + to_string(now_table.maptable[now_table.paralist[i].name].offset - vsize) + "($sp)");
		}
		else {//非数字 临时变量
			string reg_1;
			int offset_src1;
			if (function_Reference_data[SymbolTable_index].map_reg.count(temp_para) != 0)//已经分配了全局寄存器
			{
				reg_1 = function_Reference_data[SymbolTable_index].map_reg[temp_para];//获得该全局寄存器	
				enter_mipscode("add $a0 "+reg_1+" $0");//将值赋值给$a0
			}
			else {//进行临时寄存器分配
				if (alloc_reg_pool(temp_para, reg_1) == false) {//没在寄存器池中
					if (find_addr(temp_para, offset_src1) == true) {//局部变量
						enter_mipscode("lw $a0 " + to_string(offset_src1) + "($sp)");  //lw $ti offset($sp)
					}
					else {//全局变量
						enter_mipscode("lw $a0 " + to_string(offset_src1));  //lw $ti offset
					}
				}
				else {//在全局寄存器池中  add指令比load指令权重少
					enter_mipscode("add $a0 " + reg_1 + " $0");//将值赋值给$a0
				}
			}
			//存储到目标位置
			enter_mipscode("sw $a0 " + to_string(now_table.maptable[now_table.paralist[i].name].offset- vsize) + "($sp)");
		}

	}
	//now_para_num = 0;







	empty_reg_pool();//清空寄存器池

	//保存有价值的$s
	/*map<string, string>::iterator iter;//迭代器
	iter = function_Reference_data[SymbolTable_index].map_reg.begin();
	int i = 12;
	while (iter != function_Reference_data[SymbolTable_index].map_reg.end()) {
		enter_mipscode("sw "+iter->second+" "+to_string(i)+"($sp)");
		iter++;
		i = i + 4;
	}*/

	//enter_mipscode("sw $fp -" + to_string(vsize - 8) + "($sp)"); //这是保留fp的位置
	//enter_mipscode("add $fp $sp 0");//新的 $fp
	
	enter_mipscode("subi $sp $sp " + to_string(vsize));//新的$sp

	//跳转到函数
	enter_mipscode("jal " + instruction.des);//使用jal 返回地址保留在$ra  function 内部进行$ra的保存

	//恢复$s
	/* i = 12;
	 iter = function_Reference_data[SymbolTable_index].map_reg.begin();
	while (iter != function_Reference_data[SymbolTable_index].map_reg.end()) {
		enter_mipscode("lw " + iter->second + " " + to_string(i) + "($sp)");//保存用到的$s  注意这里的用的是$sp
		iter++;
		i = i + 4;
	}*/
}

void midcode_ret(midcode instruction) { //1.将返回值放入指定位置  2.设置$ra 3.退栈  4.jr $ra
	 //1.将返回值放入指定位置  (似乎可以直接放入$v0)
	string reg;
	int offset;
	if (instruction.src1 == "exp") {//有返回值
		if (is_num(instruction.des) == 1) {//数字
			enter_mipscode("li $v0 " + instruction.des);
		}
		else {//非数字

			if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.des) != 0)//已经分配了全局寄存器
			{
				reg = function_Reference_data[SymbolTable_index].map_reg[instruction.des];//获得该全局寄存器
				enter_mipscode("add $v0 " + reg + " $0");//将值赋值给$a0
			}
			else {//进行临时寄存器分配
				if (alloc_reg_pool(instruction.des, reg) == false) {//没在寄存器池中 这时还是会分配一个寄存器 但是接下来进行跳转 会清空寄存器池
					if (find_addr(instruction.des, offset) == true) {//局部变量
						enter_mipscode("lw $v0 " + to_string(offset) + "($sp)");  
					}
					else {//全局变量
						enter_mipscode("lw $v0 " + to_string(offset) );
					}
				}
				else {//在寄存器池中
					enter_mipscode("add $v0 " + reg + " $0");//将值赋值给$v0
				}
			}
		
		}
	}
	else if (instruction.src1 == "null") {//无返回值

	}
	else {
		error("你在逗我");
		return;
	}
	//清空临时寄存器池
	//empty_reg_pool();

	for (int i = 0; i < 10; i++) {
		if (reg_pool[i].ID.empty()) {

		}
		else if (reg_pool[i].dirty_mark == true) {//不为空 且脏 需要放回内存
			int offset;
			if (find_addr(reg_pool[i].ID, offset) == true) {//局部变量
				//enter_mipscode("sw $t" + to_string(i) + " " + to_string(offset) + "($sp)");  //sw $ti offset($sp)
			}
			else {//全局变量
				enter_mipscode("sw $t" + to_string(i) + " " + to_string(offset));  //sw $ti offset
			}
		}
	}
	initail_reg_pool();//为下一次初始化




	map<string, string>::iterator iter;//迭代器
	iter = function_Reference_data[SymbolTable_index].map_reg.begin();
	int i = 12;
	while (iter != function_Reference_data[SymbolTable_index].map_reg.end()) {
		enter_mipscode("lw " + iter->second + " " + to_string(i) + "($sp)");
		iter++;
		i = i + 4;
	}

	//2.设置$ra
	enter_mipscode("lw $ra 0($sp)");
	//3.退栈 
	//enter_mipscode("add $sp $fp 0");
	 
	int vsize = symbolTable[SymbolTable_index].vsize;//该函数运行栈大小
	enter_mipscode("addi $sp $sp "+to_string(vsize));
	//
	//enter_mipscode("lw $fp -" + to_string(vsize - 8) + "($sp)"); //这是保留fp的位置  
	
	

	//4.jr $ra
	if (symbolTable[SymbolTable_index].function_name == "main") {//main 函数 直接跳end
		enter_mipscode("j end");
	}
	else {
		enter_mipscode("jr $ra");
	}

}

void midcode_RETvalue(midcode instruction) {  //这是调用函数后的 第一条语句
	if (is_num(instruction.des) == 1) {//数字
		error("怎么可能是数字");
		return;
	}
	else {//非数字  

		string reg_3;
		//des
		if (is_num(instruction.des) == 0) {//非数字 非空
			if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.des) != 0)//已经分配了全局寄存器
			{
				reg_3 = function_Reference_data[SymbolTable_index].map_reg[instruction.des];//获得该全局寄存器

			}
			else {//进行临时寄存器分配
				alloc_reg_pool(instruction.des, reg_3);
				//将其标记为脏 因为是des
				dirty_reg_pool(reg_3);
			}
		}
		else {
			error("我不相信能到这里");
			return;
		}
		enter_mipscode("add " + reg_3 + " $v0 $0");//将返回值存入变量
		/*int ret_offset;
		if (find_addr(instruction.des, ret_offset) == true) {//局部变量
			enter_mipscode("sw $v0 " + to_string(ret_offset) + "($sp)");
		}
		else {//全局变量
			enter_mipscode("sw $v0 " + to_string(ret_offset));
		}*/
	}
}

void midcode_end(midcode instruction) {//相当于 ret  null
	//清空临时寄存器池
	empty_reg_pool();
	for (int i = 0; i < 10; i++) {
		if (reg_pool[i].ID.empty()) {

		}
		else if (reg_pool[i].dirty_mark == true) {//不为空 且脏 需要放回内存
			int offset;
			if (find_addr(reg_pool[i].ID, offset) == true) {//局部变量
				//enter_mipscode("sw $t" + to_string(i) + " " + to_string(offset) + "($sp)");  //sw $ti offset($sp)
			}
			else {//全局变量
				enter_mipscode("sw $t" + to_string(i) + " " + to_string(offset));  //sw $ti offset
			}
		}
	}
	initail_reg_pool();//为下一次初始化

	map<string, string>::iterator iter;//迭代器
	iter = function_Reference_data[SymbolTable_index].map_reg.begin();
	int i = 12;
	while (iter != function_Reference_data[SymbolTable_index].map_reg.end()) {
		enter_mipscode("lw " + iter->second + " " + to_string(i) + "($sp)");
		iter++;
		i = i + 4;
	}

	// 1.设置$ra
	enter_mipscode("lw $ra 0($sp)");
	//2.退栈 
	//enter_mipscode("add $sp $fp 0");

	//int vsize = symbolTable[SymbolTable_index].vsize;//该函数运行栈大小
	//enter_mipscode("lw $fp -" + to_string(vsize - 8) + "($sp)"); //这是保留fp的位置  
	int vsize = symbolTable[SymbolTable_index].vsize;//该函数运行栈大小
	enter_mipscode("addi $sp $sp " + to_string(vsize));
	//3.jr $ra
	
	

	if (symbolTable[SymbolTable_index].function_name == "main") {//main 函数 设置end
		//enter_mipscode("j end");
		enter_mipscode("end:");
	}
	else {
		enter_mipscode("jr $ra");
	}
	//enter_mipscode("jr $ra");
	//4.切换SymbolTable_index
	SymbolTable_index = 0;

	
}


void midcode_assign(midcode instruction) {
	string reg_src1;
	string reg_des;
	int offset_src1;

	//获得des 的寄存器
	if (is_num(instruction.des) == 0) {//非数字 非空
		if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.des) != 0)//已经分配了全局寄存器
		{
			reg_des = function_Reference_data[SymbolTable_index].map_reg[instruction.des];//获得该全局寄存器

		}
		else {//进行临时寄存器分配
			alloc_reg_pool(instruction.des, reg_des);
			//将其标记为脏 因为是des
			dirty_reg_pool(reg_des);
		}
	}
	else {
		error("我不相信能到这里");
		return;
	}

	if (is_num(instruction.src1) == 1) {//数字
		enter_mipscode("li "+reg_des+" " + instruction.src1);
	}
	else {//非数字

		if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.src1) != 0)//已经分配了全局寄存器
		{
			reg_src1 = function_Reference_data[SymbolTable_index].map_reg[instruction.src1];//获得该全局寄存器
			enter_mipscode("add "+reg_des+" " + reg_src1 + " $0");//将值赋值给reg_des
		}
		else {//进行临时寄存器分配
			if (search_reg_pool(instruction.src1, reg_src1) == false) {//没在寄存器池中
				if (find_addr(instruction.src1, offset_src1) == true) {//局部变量
					enter_mipscode("lw "+reg_des+" " + to_string(offset_src1) + "($sp)");
				}
				else {//全局变量
					enter_mipscode("lw " + reg_des + " " + to_string(offset_src1));
				}
			}
			else {//在寄存器池中
				enter_mipscode("add " + reg_des + " "  + reg_src1 + " $0");//将值赋值给reg_des
			}
		}

	}

}
////////////////////////
////////////////////////
////////////////////////
////////////////////////
////////////////////////
////////////////////////
////////////////////////
////////////////////////
void midcode_addreq(midcode instruction) {//"[] ="addreq x y z   =>z[x]=y
	//加载y值
	//src2
	int offset_src2;
	int offset_src1;
	int offset_des;
	string reg_1;
	string reg_2;
	string temp_reg;//临时的寄存器 用于计算偏移

	//src2
	if (is_num(instruction.src2) == 1) {//数字
		if (alloc_reg_pool(instruction.src2, reg_2) == false) {//没在寄存器池中
			enter_mipscode("li " + reg_2 + " " + instruction.src2);
		}
	}
	else {//非数字
		if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.src2) != 0)//已经分配了全局寄存器
		{
			reg_2 = function_Reference_data[SymbolTable_index].map_reg[instruction.src2];//获得该全局寄存器

		}
		else {//进行临时寄存器分配
			if (alloc_reg_pool(instruction.src2, reg_2) == false) {//没在寄存器池中
				if (find_addr(instruction.src2, offset_src2) == true) {//局部变量
					enter_mipscode("lw " + reg_2 + " " + to_string(offset_src2) + "($sp)");  //lw $ti offset($sp)
				}
				else {//全局变量
					enter_mipscode("lw " + reg_2 + " " + to_string(offset_src2));  //lw $ti offset
				}
			}
		}
	}

	if (is_num(instruction.src1) == 1) {//数字  数字比较好处理
		int offset_des;//因为des是数组 所有一定没有分配全局寄存器
		if (find_addr(instruction.des, offset_des) == true) {//局部变量
			enter_mipscode("sw "+reg_2+" " + to_string(4 * str2int(instruction.src1) + offset_des) + "($sp)");
		}
		else {//全局变量
			enter_mipscode("sw "+reg_2+" " + to_string(4 * str2int(instruction.src1) + offset_des));
		}
	}
	else {//非数字 需要利用寄存器 计算offset

		if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.src1) != 0)//已经分配了全局寄存器
		{
			reg_1 = function_Reference_data[SymbolTable_index].map_reg[instruction.src1];//获得该全局寄存器	
		}
		else {//进行临时寄存器分配
			if (alloc_reg_pool(instruction.src1, reg_1) == false) {//没在寄存器池中
				if (find_addr(instruction.src1, offset_src1) == true) {//局部变量
					enter_mipscode("lw " + reg_1 + " " + to_string(offset_src1) + "($sp)");  //lw $ti offset($sp)
				}
				else {//全局变量
					enter_mipscode("lw " + reg_1 + " " + to_string(offset_src1));  //lw $ti offset
				}
			}
		}

		alloc_temp_reg(temp_reg);//获得临时寄存器

		
		if (find_addr(instruction.des, offset_des) == true) {//局部变量
			//将$reg_1 *4
			enter_mipscode("sll "+temp_reg+" "+reg_1+" 2");
			// add temp_reg temp_reg $sp
			enter_mipscode("add "+temp_reg+" "+temp_reg+" $sp");
			//sw reg_2  offset_des(temp_reg)
			enter_mipscode("sw "+ reg_2+" " + to_string(offset_des) + "("+temp_reg+")"); //最终的地址是 offset_des+$sp+4*reg_1
		}
		else {//全局变量
			//将reg_1 *4
			enter_mipscode("sll " + temp_reg + " " + reg_1 + " 2");

			enter_mipscode("sw " + reg_2 + " " + to_string(offset_des) + "(" + temp_reg + ")"); //最终的地址是 offset_des+4*reg_1
		}

	}
}
void midcode_eqaddr(midcode instruction) {//" =[]"eqaddr x y z   =>z=y[x]
	int offset_src2;
	int offset_src1;
	string reg_1;
	string reg_3;
	string temp_reg;//临时的寄存器 用于计算偏移

	//des  获得reg_3 
	if (is_num(instruction.des) == 0) {//非数字 非空
		if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.des) != 0)//已经分配了全局寄存器
		{
			reg_3 = function_Reference_data[SymbolTable_index].map_reg[instruction.des];//获得该全局寄存器

		}
		else {//进行临时寄存器分配
			alloc_reg_pool(instruction.des, reg_3);
			//将其标记为脏 因为是des
			dirty_reg_pool(reg_3);
		}
	}
	else {
		error("我不相信能到这里");
		return;
	}

	if (is_num(instruction.src1) == 1) {//数字  数字比较好处理

		if (find_addr(instruction.src2, offset_src2) == true) {//局部变量
			enter_mipscode("lw " + reg_3 + " " + to_string(4 * str2int(instruction.src1) + offset_src2) + "($sp)");
		}
		else {//全局变量
			enter_mipscode("lw " + reg_3 + " " + to_string(4 * str2int(instruction.src1) + offset_src2));
		}
	}
	else {//非数字 需要利用寄存器 计算offset

		if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.src1) != 0)//已经分配了全局寄存器
		{
			reg_1 = function_Reference_data[SymbolTable_index].map_reg[instruction.src1];//获得该全局寄存器	
		}
		else {//进行临时寄存器分配
			if (alloc_reg_pool(instruction.src1, reg_1) == false) {//没在寄存器池中
				if (find_addr(instruction.src1, offset_src1) == true) {//局部变量
					enter_mipscode("lw " + reg_1 + " " + to_string(offset_src1) + "($sp)");  //lw $ti offset($sp)
				}
				else {//全局变量
					enter_mipscode("lw " + reg_1 + " " + to_string(offset_src1));  //lw $ti offset
				}
			}
		}

		alloc_temp_reg(temp_reg);//获得临时寄存器


		if (find_addr(instruction.src2, offset_src2) == true) {//局部变量
			//将$reg_1 *4
			enter_mipscode("sll " + temp_reg + " " + reg_1 + " 2");
			// add temp_reg temp_reg $sp
			enter_mipscode("add " + temp_reg + " " + temp_reg + " $sp");
			//sw reg_2  offset_des(temp_reg)
			enter_mipscode("lw " + reg_3 + " " + to_string(offset_src2) + "(" + temp_reg + ")"); //最终的地址是 offset_des+$sp+4*reg_1
		}
		else {//全局变量
			//将reg_1 *4
			enter_mipscode("sll " + temp_reg + " " + reg_1 + " 2");

			enter_mipscode("lw " + reg_3 + " " + to_string(offset_src2) + "(" + temp_reg + ")"); //最终的地址是 offset_des+4*reg_1
		}

	}

}

void midcode_printf(midcode instruction) {
	if (instruction.src1 == "string") {
		enter_mipscode("li $v0 4");
		enter_mipscode("la $a0 " + instruction.des);
		enter_mipscode("syscall");
	}
	else if (instruction.src1 == "char"||instruction.src1=="int") {
		if (instruction.src1 == "char") {
			enter_mipscode("li $v0 11");
		}
		else {
			enter_mipscode("li $v0 1");
		}
		//des
		int offset_des;
		string reg;
		if (is_num(instruction.des) == 0) {//非数字 非空

			if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.des) != 0)//已经分配了全局寄存器
			{
				reg = function_Reference_data[SymbolTable_index].map_reg[instruction.des];//获得该全局寄存器
				enter_mipscode("add $a0 " + reg + " $0");//将值赋值给$a0
			}
			else {//查找临时寄存器
				if (search_reg_pool(instruction.des, reg) == false) {//没在临时寄存器中
					if (find_addr(instruction.des, offset_des) == true) {//局部变量
						enter_mipscode("lw $a0 " + to_string(offset_des) + "($sp)");
					}
					else {//全局变量
						enter_mipscode("lw $a0 " + to_string(offset_des));
					}
				}
				else {//在寄存器池中
					alloc_reg_pool(instruction.des, reg);
					enter_mipscode("add $a0 " + reg + " $0");//将值赋值给$a0
				}
			}
		}
		else {//数字
			enter_mipscode("li $a0 "+instruction.des);
		}
		enter_mipscode("syscall");
	}
	else {
		error("can't believe");
		return;
	}
}

void midcode_scanf(midcode instruction) {
	string reg_des;
	if (instruction.src1 == "int"|| instruction.src1 == "char") {
		if (instruction.src1 == "char") {
			enter_mipscode("li $v0 12");
		}
		else {//int
			enter_mipscode("li $v0 5");
		}
		enter_mipscode("syscall");
		//获得des 的寄存器
		if (is_num(instruction.des) == 0) {//非数字 非空
			if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.des) != 0)//已经分配了全局寄存器
			{
				reg_des = function_Reference_data[SymbolTable_index].map_reg[instruction.des];//获得该全局寄存器

			}
			else {//进行临时寄存器分配
				alloc_reg_pool(instruction.des, reg_des);
				//将其标记为脏 因为是des
				dirty_reg_pool(reg_des);
			}
		}
		else {
			error("我不相信能到这里");
			return;
		}
		enter_mipscode("add " + reg_des + " $v0 $0");
	}
	else {
		error("感觉这样好没意义");
		return;
	}
}

void midcode_lable(midcode instruction) {
	empty_reg_pool();
	enter_mipscode(instruction.des+":"); //设置lable
}

void midcode_GOTO(midcode instruction) {
	empty_reg_pool();
	enter_mipscode("j " + instruction.des);

}

void midcode_BNZ(midcode instruction) {//满足条件跳转 $t9 作为跳转寄存器   满足条件其值为1 不满足条件其值为0
	//enter_mipscode("beq $t9 1 "+instruction.des);
}//该函数已经没有利用价值了

void midcode_BZ(midcode instruction) {//不满足条件跳转 $t9 作为跳转寄存器   满足条件其值为1 不满足条件其值为0
	//enter_mipscode("beq $t9 0 " + instruction.des);
}//该函数已经没有利用价值了

void midcode_EQ(midcode instruction) {//相等不跳转  和 不等跳转(bne) 等价  加载src1 和src2
	int offset_src1;
	int offset_src2;

	string reg_1;
	string reg_2;
	//src1
	if (is_num(instruction.src1) == 1) {//数字
		if (alloc_reg_pool(instruction.src1, reg_1) == false) {//没在寄存器池中
			enter_mipscode("li " + reg_1 + " " + instruction.src1);
		}
	}
	else {//非数字
		if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.src1) != 0)//已经分配了全局寄存器
		{
			reg_1 = function_Reference_data[SymbolTable_index].map_reg[instruction.src1];//获得该全局寄存器	
		}
		else {//进行临时寄存器分配
			if (alloc_reg_pool(instruction.src1, reg_1) == false) {//没在寄存器池中
				if (find_addr(instruction.src1, offset_src1) == true) {//局部变量
					enter_mipscode("lw " + reg_1 + " " + to_string(offset_src1) + "($sp)");  //lw $ti offset($sp)
				}
				else {//全局变量
					enter_mipscode("lw " + reg_1 + " " + to_string(offset_src1));  //lw $ti offset
				}
			}
		}

	}
	//src2
	if (is_num(instruction.src2) == 1) {//数字
		if (alloc_reg_pool(instruction.src2, reg_2) == false) {//没在寄存器池中
			enter_mipscode("li " + reg_2 + " " + instruction.src2);
		}
	}
	else {//非数字
		if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.src2) != 0)//已经分配了全局寄存器
		{
			reg_2 = function_Reference_data[SymbolTable_index].map_reg[instruction.src2];//获得该全局寄存器

		}
		else {//进行临时寄存器分配
			if (alloc_reg_pool(instruction.src2, reg_2) == false) {//没在寄存器池中
				if (find_addr(instruction.src2, offset_src2) == true) {//局部变量
					enter_mipscode("lw " + reg_2 + " " + to_string(offset_src2) + "($sp)");  //lw $ti offset($sp)
				}
				else {//全局变量
					enter_mipscode("lw " + reg_2 + " " + to_string(offset_src2));  //lw $ti offset
				}
			}
		}
	}
	empty_reg_pool();
	enter_mipscode("bne "+reg_1+" "+reg_2+" "+instruction.des);

}
void midcode_NOTEQ(midcode instruction) {//不相等不跳转  和 等于跳转(beq) 等价  加载src1 和src2
	int offset_src1;
	int offset_src2;

	string reg_1;
	string reg_2;
	//src1
	if (is_num(instruction.src1) == 1) {//数字
		if (alloc_reg_pool(instruction.src1, reg_1) == false) {//没在寄存器池中
			enter_mipscode("li " + reg_1 + " " + instruction.src1);
		}
	}
	else {//非数字
		if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.src1) != 0)//已经分配了全局寄存器
		{
			reg_1 = function_Reference_data[SymbolTable_index].map_reg[instruction.src1];//获得该全局寄存器	
		}
		else {//进行临时寄存器分配
			if (alloc_reg_pool(instruction.src1, reg_1) == false) {//没在寄存器池中
				if (find_addr(instruction.src1, offset_src1) == true) {//局部变量
					enter_mipscode("lw " + reg_1 + " " + to_string(offset_src1) + "($sp)");  //lw $ti offset($sp)
				}
				else {//全局变量
					enter_mipscode("lw " + reg_1 + " " + to_string(offset_src1));  //lw $ti offset
				}
			}
		}

	}
	//src2
	if (is_num(instruction.src2) == 1) {//数字
		if (alloc_reg_pool(instruction.src2, reg_2) == false) {//没在寄存器池中
			enter_mipscode("li " + reg_2 + " " + instruction.src2);
		}
	}
	else {//非数字
		if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.src2) != 0)//已经分配了全局寄存器
		{
			reg_2 = function_Reference_data[SymbolTable_index].map_reg[instruction.src2];//获得该全局寄存器

		}
		else {//进行临时寄存器分配
			if (alloc_reg_pool(instruction.src2, reg_2) == false) {//没在寄存器池中
				if (find_addr(instruction.src2, offset_src2) == true) {//局部变量
					enter_mipscode("lw " + reg_2 + " " + to_string(offset_src2) + "($sp)");  //lw $ti offset($sp)
				}
				else {//全局变量
					enter_mipscode("lw " + reg_2 + " " + to_string(offset_src2));  //lw $ti offset
				}
			}
		}
	}
	empty_reg_pool();
	enter_mipscode("beq "+reg_1+" "+reg_2+" " + instruction.des);

}
void midcode_LESSEQ(midcode instruction) {//<= src1 <= src2 不跳转  src1>src2跳转
	int offset_src1;
	int offset_src2;

	string reg_1;
	string reg_2;
	string reg_temp;
	//src1
	if (is_num(instruction.src1) == 1) {//数字
		if (alloc_reg_pool(instruction.src1, reg_1) == false) {//没在寄存器池中
			enter_mipscode("li " + reg_1 + " " + instruction.src1);
		}
	}
	else {//非数字
		if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.src1) != 0)//已经分配了全局寄存器
		{
			reg_1 = function_Reference_data[SymbolTable_index].map_reg[instruction.src1];//获得该全局寄存器	
		}
		else {//进行临时寄存器分配
			if (alloc_reg_pool(instruction.src1, reg_1) == false) {//没在寄存器池中
				if (find_addr(instruction.src1, offset_src1) == true) {//局部变量
					enter_mipscode("lw " + reg_1 + " " + to_string(offset_src1) + "($sp)");  //lw $ti offset($sp)
				}
				else {//全局变量
					enter_mipscode("lw " + reg_1 + " " + to_string(offset_src1));  //lw $ti offset
				}
			}
		}

	}
	//src2
	if (is_num(instruction.src2) == 1) {//数字
		if (alloc_reg_pool(instruction.src2, reg_2) == false) {//没在寄存器池中
			enter_mipscode("li " + reg_2 + " " + instruction.src2);
		}
	}
	else {//非数字
		if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.src2) != 0)//已经分配了全局寄存器
		{
			reg_2 = function_Reference_data[SymbolTable_index].map_reg[instruction.src2];//获得该全局寄存器

		}
		else {//进行临时寄存器分配
			if (alloc_reg_pool(instruction.src2, reg_2) == false) {//没在寄存器池中
				if (find_addr(instruction.src2, offset_src2) == true) {//局部变量
					enter_mipscode("lw " + reg_2 + " " + to_string(offset_src2) + "($sp)");  //lw $ti offset($sp)
				}
				else {//全局变量
					enter_mipscode("lw " + reg_2 + " " + to_string(offset_src2));  //lw $ti offset
				}
			}
		}
	}
	alloc_temp_reg(reg_temp);
	enter_mipscode("sub "+reg_temp+" "+reg_1+" "+reg_2);//src1-src2>0 跳转
	empty_reg_pool();
	enter_mipscode("bgtz "+reg_temp +" "+ instruction.des);

}
void midcode_LESS(midcode instruction) {//<= src1 < src2 不跳转  src1>=src2跳转
	int offset_src1;
	int offset_src2;

	string reg_1;
	string reg_2;
	string reg_temp;
	//src1
	if (is_num(instruction.src1) == 1) {//数字
		if (alloc_reg_pool(instruction.src1, reg_1) == false) {//没在寄存器池中
			enter_mipscode("li " + reg_1 + " " + instruction.src1);
		}
	}
	else {//非数字
		if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.src1) != 0)//已经分配了全局寄存器
		{
			reg_1 = function_Reference_data[SymbolTable_index].map_reg[instruction.src1];//获得该全局寄存器	
		}
		else {//进行临时寄存器分配
			if (alloc_reg_pool(instruction.src1, reg_1) == false) {//没在寄存器池中
				if (find_addr(instruction.src1, offset_src1) == true) {//局部变量
					enter_mipscode("lw " + reg_1 + " " + to_string(offset_src1) + "($sp)");  //lw $ti offset($sp)
				}
				else {//全局变量
					enter_mipscode("lw " + reg_1 + " " + to_string(offset_src1));  //lw $ti offset
				}
			}
		}

	}
	//src2
	if (is_num(instruction.src2) == 1) {//数字
		if (alloc_reg_pool(instruction.src2, reg_2) == false) {//没在寄存器池中
			enter_mipscode("li " + reg_2 + " " + instruction.src2);
		}
	}
	else {//非数字
		if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.src2) != 0)//已经分配了全局寄存器
		{
			reg_2 = function_Reference_data[SymbolTable_index].map_reg[instruction.src2];//获得该全局寄存器

		}
		else {//进行临时寄存器分配
			if (alloc_reg_pool(instruction.src2, reg_2) == false) {//没在寄存器池中
				if (find_addr(instruction.src2, offset_src2) == true) {//局部变量
					enter_mipscode("lw " + reg_2 + " " + to_string(offset_src2) + "($sp)");  //lw $ti offset($sp)
				}
				else {//全局变量
					enter_mipscode("lw " + reg_2 + " " + to_string(offset_src2));  //lw $ti offset
				}
			}
		}
	}
	alloc_temp_reg(reg_temp);
	enter_mipscode("sub " + reg_temp + " " + reg_1 + " " + reg_2);//src1-src2>=0 跳转
	empty_reg_pool();
	enter_mipscode("bgez " + reg_temp + " " + instruction.des);

	//enter_mipscode("sub $t0 $t0 $t1"); //src1-src2>=0 跳转
	//enter_mipscode("bgez $t0 " + instruction.des);
}
void midcode_MOREEQ(midcode instruction) {//<= src1 >= src2 不跳转  src1<src2跳转
	int offset_src1;
	int offset_src2;

	string reg_1;
	string reg_2;
	string reg_temp;
	//src1
	if (is_num(instruction.src1) == 1) {//数字
		if (alloc_reg_pool(instruction.src1, reg_1) == false) {//没在寄存器池中
			enter_mipscode("li " + reg_1 + " " + instruction.src1);
		}
	}
	else {//非数字
		if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.src1) != 0)//已经分配了全局寄存器
		{
			reg_1 = function_Reference_data[SymbolTable_index].map_reg[instruction.src1];//获得该全局寄存器	
		}
		else {//进行临时寄存器分配
			if (alloc_reg_pool(instruction.src1, reg_1) == false) {//没在寄存器池中
				if (find_addr(instruction.src1, offset_src1) == true) {//局部变量
					enter_mipscode("lw " + reg_1 + " " + to_string(offset_src1) + "($sp)");  //lw $ti offset($sp)
				}
				else {//全局变量
					enter_mipscode("lw " + reg_1 + " " + to_string(offset_src1));  //lw $ti offset
				}
			}
		}

	}
	//src2
	if (is_num(instruction.src2) == 1) {//数字
		if (alloc_reg_pool(instruction.src2, reg_2) == false) {//没在寄存器池中
			enter_mipscode("li " + reg_2 + " " + instruction.src2);
		}
	}
	else {//非数字
		if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.src2) != 0)//已经分配了全局寄存器
		{
			reg_2 = function_Reference_data[SymbolTable_index].map_reg[instruction.src2];//获得该全局寄存器

		}
		else {//进行临时寄存器分配
			if (alloc_reg_pool(instruction.src2, reg_2) == false) {//没在寄存器池中
				if (find_addr(instruction.src2, offset_src2) == true) {//局部变量
					enter_mipscode("lw " + reg_2 + " " + to_string(offset_src2) + "($sp)");  //lw $ti offset($sp)
				}
				else {//全局变量
					enter_mipscode("lw " + reg_2 + " " + to_string(offset_src2));  //lw $ti offset
				}
			}
		}
	}
	alloc_temp_reg(reg_temp);
	enter_mipscode("sub " + reg_temp + " " + reg_1 + " " + reg_2);//src1-src2<0 跳转
	empty_reg_pool();
	enter_mipscode("bltz " + reg_temp + " " + instruction.des);
	
	//enter_mipscode("sub $t0 $t0 $t1"); //src1-src2<0 跳转
	//enter_mipscode("bltz $t0 " + instruction.des);
}
void midcode_MORE(midcode instruction) {//<= src1 > src2 不跳转  src1<=src2跳转
	int offset_src1;
	int offset_src2;

	string reg_1;
	string reg_2;
	string reg_temp;
	//src1
	if (is_num(instruction.src1) == 1) {//数字
		if (alloc_reg_pool(instruction.src1, reg_1) == false) {//没在寄存器池中
			enter_mipscode("li " + reg_1 + " " + instruction.src1);
		}
	}
	else {//非数字
		if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.src1) != 0)//已经分配了全局寄存器
		{
			reg_1 = function_Reference_data[SymbolTable_index].map_reg[instruction.src1];//获得该全局寄存器	
		}
		else {//进行临时寄存器分配
			if (alloc_reg_pool(instruction.src1, reg_1) == false) {//没在寄存器池中
				if (find_addr(instruction.src1, offset_src1) == true) {//局部变量
					enter_mipscode("lw " + reg_1 + " " + to_string(offset_src1) + "($sp)");  //lw $ti offset($sp)
				}
				else {//全局变量
					enter_mipscode("lw " + reg_1 + " " + to_string(offset_src1));  //lw $ti offset
				}
			}
		}

	}
	//src2
	if (is_num(instruction.src2) == 1) {//数字
		if (alloc_reg_pool(instruction.src2, reg_2) == false) {//没在寄存器池中
			enter_mipscode("li " + reg_2 + " " + instruction.src2);
		}
	}
	else {//非数字
		if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.src2) != 0)//已经分配了全局寄存器
		{
			reg_2 = function_Reference_data[SymbolTable_index].map_reg[instruction.src2];//获得该全局寄存器

		}
		else {//进行临时寄存器分配
			if (alloc_reg_pool(instruction.src2, reg_2) == false) {//没在寄存器池中
				if (find_addr(instruction.src2, offset_src2) == true) {//局部变量
					enter_mipscode("lw " + reg_2 + " " + to_string(offset_src2) + "($sp)");  //lw $ti offset($sp)
				}
				else {//全局变量
					enter_mipscode("lw " + reg_2 + " " + to_string(offset_src2));  //lw $ti offset
				}
			}
		}
	}
	alloc_temp_reg(reg_temp);
	enter_mipscode("sub " + reg_temp + " " + reg_1 + " " + reg_2);//src1-src2<=0 跳转
	empty_reg_pool();
	enter_mipscode("blez " + reg_temp + " " + instruction.des);

	//enter_mipscode("sub $t0 $t0 $t1"); //src1-src2<=0 跳转
	//enter_mipscode("blez $t0 " + instruction.des);
}

/////////////////////////////////////////////////////////////
///////////              主函数                //////////////
////////////////////////////////////////////////////////////
void midcode2mips() {
	initail_reg_pool();//初始化寄存器池
	SymbolTable_index = 0;
	for (int i = 0; i < midcode_index; i++) {
		if (midcodes[i].op == op_none) {//代表为空
			continue;
		}

		switch (midcodes[i].op)
		{
		case op_ADD:
			midcode_ADD(midcodes[i]);
			break;
		case op_SUB:
			midcode_SUB(midcodes[i]);
			break;
		case op_MULT:
			midcode_MULT(midcodes[i]);
			break;
		case op_DIV:
			midcode_DIV(midcodes[i]);
			break;
		case op_int:
		case op_char:
		case op_void:
			midcode_function(midcodes[i]);
			break;
		case op_call:
			midcode_call(midcodes[i]);
			break;
		case op_push:
			midcode_push(midcodes[i]);
			break;
		case op_RETvalue:
			midcode_RETvalue(midcodes[i]);
			break;
		case op_ret:
			midcode_ret(midcodes[i]);
			break;
		case op_end:
			midcode_end(midcodes[i]);
			break;
		case op_assign:
			midcode_assign(midcodes[i]);
			break;
		case op_addreq:
			midcode_addreq(midcodes[i]);
			break;
		case op_eqaddr:
			midcode_eqaddr(midcodes[i]);
			break;
		case op_printf:
			midcode_printf(midcodes[i]);
			break;
		case op_scanf:
			midcode_scanf(midcodes[i]);
			break;
		case op_lable:
			midcode_lable(midcodes[i]);
			break;
		case op_GOTO:
			midcode_GOTO(midcodes[i]);
			break;
		case op_EQ:
			midcode_EQ(midcodes[i]);
			break;
		case op_NOTEQ:
			midcode_NOTEQ(midcodes[i]);
			break;
		case op_LESSEQ:
			midcode_LESSEQ(midcodes[i]);
			break;
		case op_LESS:
			midcode_LESS(midcodes[i]);
			break;
		case op_MOREEQ:
			midcode_MOREEQ(midcodes[i]);
			break;
		case op_MORE:
			midcode_MORE(midcodes[i]);
			break;


		default:
			error("中间码有bug");

			break;
		}
	}
}