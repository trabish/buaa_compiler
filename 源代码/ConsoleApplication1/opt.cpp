#include "pch.h"
#include "opt.h"
#include "midcode.h"
#include "Parser.h"
#include "Lexer.h"
#include "error.h"
#include "midcode.h"
#include <algorithm>
#include <map>

//叶函数删除 保存$ra的指令
void leaf_function() {

	map<string, int> function_map;
	string now_function;
	for (int i = 0; i < midcode_index; i++) {//第一遍 找到叶函数

		if (midcodes[i].op == op_none) {//代表为空
			continue;
		}
		switch (midcodes[i].op)
		{
		case op_int:
		case op_char:
		case op_void:
			function_map[midcodes[i].des] = 1;//标记函数
			now_function = midcodes[i].des;
			break;
		case op_call://调用了其他函数
			function_map[now_function] = 0;//非叶函数
			break;

		}
	}


	int delete_ra_mark = 0;
	for (int i = 0; i < mipscode_index; i++) {
		string now_lable = mipscodes[i].substr(0,mipscodes[i].size() - 1);
		if (mipscodes[i][mipscodes[i].size() - 1] == ':') {
			if (function_map.count(now_lable) == 1) {//存在该函数
				if (function_map[now_lable] == 1) {//叶函数
					delete_ra_mark = 1;
				}
				else {
					delete_ra_mark = 0;
				}
			}
		}

		if (delete_ra_mark == 1&& (mipscodes[i]=="lw $ra 0($sp)"|| mipscodes[i] =="sw $ra 0($sp)" )) {
			mipscodes[i] = " ";
		}
	}
}

struct var_sort {
	string name;
	int num;
};

bool comp(var_sort x, var_sort y)
{
	return x.num > y.num;
}

Reference_data function_Reference_data[500];//每个函数的全局寄存器分配  确定好每个函数中变量对应的全局寄存器
void Reference_count() {
	bool globle_reg_close;//关闭全局寄存器分配
	bool function_globle_reg_close=false;//关闭调用函数过多的函数的全局寄存器分配 true是关闭
	int function_close_num=5;//关闭调用函数过多的函数 其中指定的数量

	////////////////////////
	/////////////////////// 还可以做一做  当引用次数与函数调用次数加权来判断是否使用这个全局变量
	
	string function_name;
	
	for (int i = 0; i < midcode_index; i++) {
		if (midcodes[i].op == op_none) {
			continue;
		}
		else if (midcodes[i].src1 == "function") {//进入某个函数

			bool return_num = false;//检测函数之中是否有return

			int call_function_num = 0;//调用函数的数量

			map<string, int>  count_ref;//记录每个变量的引用次数
			int while_mark = 0;//在while中的深度；
			int while_factor = 5;//在while中变量引用的权重 1+ while_mark*while_factor
			function_name = midcodes[i].des;//记录函数名
			int symbolTable_index = symbolTable[0].maptable[function_name].adr;
			while (midcodes[i].op != op_end) {//退出函数
				switch (midcodes[i].op) {
				case op_none:
					break;
				case op_ADD://二元运算
				case op_SUB:
				case op_MULT:
				case op_DIV:
					//src1
					if (midcodes[i].src1[0] == '-' || (midcodes[i].src1[0] >= '0'&&midcodes[i].src1[0] <= '9')) {//数字
					}
					else {//标识符
						if (count_ref.count(midcodes[i].src1) == 0) {//没有记录过
							count_ref[midcodes[i].src1] = while_mark*while_factor+1;
						}
						else {
							count_ref[midcodes[i].src1]= count_ref[midcodes[i].src1]+ while_mark * while_factor + 1;//加一次引用次数
						}
					}
					//src2
					if (midcodes[i].src2[0] == '-' || (midcodes[i].src2[0] >= '0'&&midcodes[i].src2[0] <= '9')) {//数字
					}
					else {//标识符
						if (count_ref.count(midcodes[i].src2) == 0) {//没有记录过
							count_ref[midcodes[i].src2] = while_mark * while_factor + 1;
						}
						else {
							count_ref[midcodes[i].src2]= count_ref[midcodes[i].src2]+ while_mark * while_factor + 1;//加一次引用次数
						}
					}
					//des
					if (midcodes[i].des[0] == '-' || (midcodes[i].des[0] >= '0'&&midcodes[i].des[0] <= '9')) {//数字
					}
					else {//标识符
						if (count_ref.count(midcodes[i].des) == 0) {//没有记录过
							count_ref[midcodes[i].des] = while_mark * while_factor + 1;
						}
						else {
							count_ref[midcodes[i].des]= count_ref[midcodes[i].des]+ while_mark * while_factor + 1;//加一次引用次数
						}
					}
					break;
				case op_int:
				case op_char:
				case op_void:
					break;
				case op_call:
					//call_function_num++;
					call_function_num = call_function_num + while_mark * while_factor + 1;
					break;
				case op_push:
					//src1
					if (midcodes[i].src1[0] == '-' || (midcodes[i].src1[0] >= '0'&&midcodes[i].src1[0] <= '9')) {//数字
					}
					else {//标识符
						if (count_ref.count(midcodes[i].src1) == 0) {//没有记录过
							count_ref[midcodes[i].src1] = while_mark * while_factor + 1;
						}
						else {
							count_ref[midcodes[i].src1] = count_ref[midcodes[i].src1] + while_mark * while_factor + 1;//加一次引用次数
						}
					}
					break;
				case op_RETvalue:
				case op_ret:
					if (midcodes[i].op == op_ret) {
						return_num = true;//检测到函数内存在return
					}
					//des
					if (midcodes[i].des.empty()||midcodes[i].des[0] == '-' || (midcodes[i].des[0] >= '0'&&midcodes[i].des[0] <= '9')) {//数字
					}
					else {//标识符
						if (count_ref.count(midcodes[i].des) == 0) {//没有记录过
							count_ref[midcodes[i].des] = while_mark * while_factor + 1;
						}
						else {
							count_ref[midcodes[i].des] = count_ref[midcodes[i].des] + while_mark * while_factor + 1;//加一次引用次数
						}
					}
					break;
				case op_end:
					error("怎么会是你");
					break;
				case op_eqaddr://=[]
				case op_assign:
					//src1
					if (midcodes[i].src1[0] == '-' || (midcodes[i].src1[0] >= '0'&&midcodes[i].src1[0] <= '9')) {//数字
					}
					else {//标识符
						if (count_ref.count(midcodes[i].src1) == 0) {//没有记录过
							count_ref[midcodes[i].src1] = while_mark * while_factor + 1;
						}
						else {
							count_ref[midcodes[i].src1] = count_ref[midcodes[i].src1] + while_mark * while_factor + 1;//加一次引用次数
						}
					}
					//des
					if (midcodes[i].des[0] == '-' || (midcodes[i].des[0] >= '0'&&midcodes[i].des[0] <= '9')) {//数字
					}
					else {//标识符
						if (count_ref.count(midcodes[i].des) == 0) {//没有记录过
							count_ref[midcodes[i].des] = while_mark * while_factor + 1;
						}
						else {
							count_ref[midcodes[i].des] = count_ref[midcodes[i].des] + while_mark * while_factor + 1;//加一次引用次数
						}
					}
					break;
				case op_addreq://[]=  数组元素不进行计数
					//src1
					if (midcodes[i].src1[0] == '-' || (midcodes[i].src1[0] >= '0'&&midcodes[i].src1[0] <= '9')) {//数字
					}
					else {//标识符
						if (count_ref.count(midcodes[i].src1) == 0) {//没有记录过
							count_ref[midcodes[i].src1] = while_mark * while_factor + 1;
						}
						else {
							count_ref[midcodes[i].src1] = count_ref[midcodes[i].src1] + while_mark * while_factor + 1;//加一次引用次数
						}
					}
					//src2
					if (midcodes[i].src2[0] == '-' || (midcodes[i].src2[0] >= '0'&&midcodes[i].src2[0] <= '9')) {//数字
					}
					else {//标识符
						if (count_ref.count(midcodes[i].src2) == 0) {//没有记录过
							count_ref[midcodes[i].src2] = while_mark * while_factor + 1;
						}
						else {
							count_ref[midcodes[i].src2] = count_ref[midcodes[i].src2] + while_mark * while_factor + 1;//加一次引用次数
						}
					}
					break;
				case op_printf:
				case op_scanf:
					//des
					if (midcodes[i].src1 == "string") {
						break;
					}
					//des
					if (midcodes[i].des[0] == '-' || (midcodes[i].des[0] >= '0'&&midcodes[i].des[0] <= '9')) {//数字
					}
					else {//标识符
						if (count_ref.count(midcodes[i].des) == 0) {//没有记录过
							count_ref[midcodes[i].des] = while_mark * while_factor + 1;
						}
						else {
							count_ref[midcodes[i].des] = count_ref[midcodes[i].des] + while_mark * while_factor + 1;//加一次引用次数
						}
					}
					break;
				case op_lable: //这两个注意
					if (midcodes[i].des[0] == 'w'){//while_lable
						while_mark++;
					}
					break;
				case op_GOTO:
					if (midcodes[i].des[0] == 'w') {//while_lable
						while_mark--;
					}
					break;

				case op_EQ:
				case op_NOTEQ:
				case op_LESSEQ:
				case op_LESS:
				case op_MOREEQ:
				case op_MORE:
					//src1
					if (midcodes[i].src1[0] == '-' || (midcodes[i].src1[0] >= '0'&&midcodes[i].src1[0] <= '9')) {//数字
					}
					else {//标识符
						if (count_ref.count(midcodes[i].src1) == 0) {//没有记录过
							count_ref[midcodes[i].src1] = while_mark * while_factor + 1;
						}
						else {
							count_ref[midcodes[i].src1] = count_ref[midcodes[i].src1] + while_mark * while_factor + 1;//加一次引用次数
						}
					}
					//src2
					if (midcodes[i].src2[0] == '-' || (midcodes[i].src2[0] >= '0'&&midcodes[i].src2[0] <= '9')) {//数字
					}
					else {//标识符
						if (count_ref.count(midcodes[i].src2) == 0) {//没有记录过
							count_ref[midcodes[i].src2] = while_mark * while_factor + 1;
						}
						else {
							count_ref[midcodes[i].src2] = count_ref[midcodes[i].src2] + while_mark * while_factor + 1;//加一次引用次数
						}
					}
					break;
				default:
					error("let me see see");
				}
				i++;
			}

			/*if (function_globle_reg_close==true&&call_function_num>= function_close_num) {
				continue;//不为该函数分配全局寄存器
			}*/


			//进行全局寄存器分配
			int var_num=0;//参数个数
			var_sort var_sort_group[500];//用于变量引用次数排序
			map<string, int>::iterator iter;//迭代器
			iter = count_ref.begin();
			while (iter != count_ref.end()) {//将计数数据导入 var_sort_group
				var_sort tem_var;
				tem_var.name = iter->first;

				tem_var.num = iter->second;
				if (symbolTable[symbolTable_index].maptable.count(tem_var.name) == 0) {//全局变量
					tem_var.num = 0;
				}
				var_sort_group[var_num++] = tem_var;
				iter++;
			}

			sort(var_sort_group, var_sort_group + var_num, comp);//以降序进行排序


			
			for (int j = 0; j < (var_num < 8 ? var_num : 8); j++) {//对应全局寄存器
				if (4>= var_sort_group[j].num) {
					continue;
				}
				if (symbolTable[symbolTable_index].maptable.count(var_sort_group[j].name) == 0) {//全局变量不分配全局寄存器
					continue;
				}
				function_Reference_data[symbolTable_index].map_reg[var_sort_group[j].name] = "$s"+to_string(j);//分配$si
				

			}
			if (return_num == false && symbolTable[0].maptable[function_name].type!=type_void) {
				error(function_name+"函数缺少return");
			}

		}
		else {
			error("你是什么四元式呀");
		}
	}
}

void delete_Tn() {//1.替换tn 2.删除死代码
	bool delete_more = true;//对于 add i 1 @T     assign @T i 这种进行删除
	//解决 assign i  Tn 的情况

	bool G_delete_Tn = true;//全局变量的Tn的删除

	string Tn_def[2000];//存放tn的def 值
	for (int i = 0; i < 2000; i++) {
		Tn_def[i] = "#";//表示还未定义

	}
	//1.替换Tn
	for (int i = 0; i < midcode_index; i++) {//第一遍 替换Tn

		if (midcodes[i].op == op_none) {//代表为空
			continue;
		}
		switch (midcodes[i].op)
		{
		case op_ADD://二元运算
		case op_SUB:
		case op_MULT:
		case op_DIV:
			//参数一
			if (midcodes[i].src1[0] == '@')
			{
				int temp = str2int(midcodes[i].src1.substr(2));
				if (Tn_def[temp] != "#") {
					midcodes[i].src1 = Tn_def[temp];
				}
			}
			//参数二
			if (midcodes[i].src2[0] == '@')
			{
				int temp = str2int(midcodes[i].src2.substr(2));
				if (Tn_def[temp] != "#") {
					midcodes[i].src2 = Tn_def[temp];
				}
			}
			//des
			if (midcodes[i].des[0] == '@')
			{
				int temp = str2int(midcodes[i].des.substr(2));
				Tn_def[temp] = "#";//不能替代了
			}
			break;
		case op_int:
		case op_char:
		case op_void:
		case op_call:
			break;
		case op_push:
			if (midcodes[i].src1[0] == '@')
			{
				int temp = str2int(midcodes[i].src1.substr(2));
				if (Tn_def[temp] != "#") {
					midcodes[i].src1 = Tn_def[temp];
				}
			}
			break;
		case op_RETvalue:
			if (midcodes[i].des[0] == '@')
			{
				int temp = str2int(midcodes[i].des.substr(2));
				Tn_def[temp] = "#";//不能替代了
			}
			break;
		case op_ret:
			if (midcodes[i].des[0] == '@')
			{
				int temp = str2int(midcodes[i].des.substr(2));
				if (Tn_def[temp] != "#") {
					midcodes[i].des = Tn_def[temp];
				}
			}
			break;
		case op_end:
			
			break;
		case op_assign:
			//动不得全局变量

			if (midcodes[i].src1[0] == '@')
			{
				int temp = str2int(midcodes[i].src1.substr(2));
				if (Tn_def[temp] != "#") {
					midcodes[i].src1 = Tn_def[temp];
				}
			}
			if ((midcodes[i].src1[0]!='@')&&midcodes[i].des[0] == '@')
			{
				if (symbolTable[0].maptable.count(midcodes[i].src1) == 0) {//局部变量 可以操作


					int temp = str2int(midcodes[i].des.substr(2));
					Tn_def[temp] = midcodes[i].src1;
				}
				else {//全局变量 动不得
					int temp = str2int(midcodes[i].des.substr(2));
					Tn_def[temp] = "#";//不能替代了
				}
			}
			

			break;
		case op_addreq://[]= 
		case op_eqaddr:
			//参数一
			if (midcodes[i].src1[0] == '@')
			{
				int temp = str2int(midcodes[i].src1.substr(2));
				if (Tn_def[temp] != "#") {
					midcodes[i].src1 = Tn_def[temp];
				}
			}
			//参数二
			if (midcodes[i].src2[0] == '@')
			{
				int temp = str2int(midcodes[i].src2.substr(2));
				if (Tn_def[temp] != "#") {
					midcodes[i].src2 = Tn_def[temp];
				}
			}
			//des
			if (midcodes[i].des[0] == '@')
			{
				int temp = str2int(midcodes[i].des.substr(2));
				Tn_def[temp] = "#";//不能替代了
			}
			break;

		case op_printf:
			if (midcodes[i].des[0] == '@')
			{
				int temp = str2int(midcodes[i].des.substr(2));
				if (Tn_def[temp] != "#") {
					midcodes[i].des = Tn_def[temp];
				}
			}
			break;
		case op_scanf:
			break;
		case op_lable:
			break;
		case op_GOTO:
			break;

		case op_EQ:
		case op_NOTEQ:
		case op_LESSEQ:
		case op_LESS:
		case op_MOREEQ:
		case op_MORE:
			//参数一
			if (midcodes[i].src1[0] == '@')
			{
				int temp = str2int(midcodes[i].src1.substr(2));
				if (Tn_def[temp] != "#") {
					midcodes[i].src1 = Tn_def[temp];
				}
			}
			//参数二
			if (midcodes[i].src2[0] == '@')
			{
				int temp = str2int(midcodes[i].src2.substr(2));
				if (Tn_def[temp] != "#") {
					midcodes[i].src2 = Tn_def[temp];
				}
			}
			break;


		default:
			error("中间码有bug");
			break;
		}
	}
	//2.删除多余的tn
	for (int i = 0; i < midcode_index; i++) {
		if (midcodes[i].op == op_assign) {
			if ((midcodes[i].src1[0] != '@') && midcodes[i].des[0] == '@'&&symbolTable[0].maptable.count(midcodes[i].src1) == 0) {//全局变量不删
				midcodes[i].op = op_none;//为空
			}
		}
	}
////////////////////////////////////////////////
	////////////////////////////////////////////

	if (delete_more == true) {//存在问题
		for (int i = 1; i < midcode_index; i++) {
			if (midcodes[i].op == op_assign) {
				string temp;
				if (midcodes[i].src1[0] == '@') {
					switch (midcodes[i - 1].op) {
					case op_ADD://二元运算
					case op_SUB:
					case op_MULT:
					case op_DIV:
						if (midcodes[i - 1].des == midcodes[i].src1)
						{
							midcodes[i - 1].des = midcodes[i].des;
						}
						temp = midcodes[i].des;//交换@tn和变量的位置 在后面的删除死代码就可以进行删除
						midcodes[i].des = midcodes[i].src1;
						midcodes[i].src1 = temp;


						break;
					case op_RETvalue:
						if (midcodes[i - 1].des == midcodes[i].src1)
						{
							midcodes[i - 1].des = midcodes[i].des;
						}

						temp = midcodes[i].des;//交换@tn和变量的位置 在后面的删除死代码就可以进行删除
						midcodes[i].des = midcodes[i].src1;
						midcodes[i].src1 = temp;

						break;
					}
					

				}
			}
		}
	}
	//////////////////////////////////////////////////
	if (G_delete_Tn == true) {//进行基本块内的传播
		//处理全局变量@tn 的删除
		//在基本块中删除
		int GTn_def[2000];//存放tn的def 值
		int j_line = 0;//当前基本块开始的行
		for (int i = 0; i < 2000; i++) {
			GTn_def[i] = -1;//表示还未定义
			Tn_def[i] = "#";

		}
		for (int i = 0; i < midcode_index; i++) {//第一遍 替换Tn

			if (midcodes[i].op == op_none) {//代表为空
				continue;
			}
			switch (midcodes[i].op)
			{
			case op_ADD://二元运算
			case op_SUB:
			case op_MULT:
			case op_DIV:
				//参数一
				if (midcodes[i].src1[0] == '@')
				{
					int temp = str2int(midcodes[i].src1.substr(2));
					if (Tn_def[temp] != "#" && (GTn_def[temp] > j_line)) {
						midcodes[i].src1 = Tn_def[temp];
					}
				}
				//参数二
				if (midcodes[i].src2[0] == '@')
				{
					int temp = str2int(midcodes[i].src2.substr(2));
					if (Tn_def[temp] != "#" && (GTn_def[temp] > j_line)) {
						midcodes[i].src2 = Tn_def[temp];
					}
				}
				//des
				if (midcodes[i].des[0] == '@')
				{
					int temp = str2int(midcodes[i].des.substr(2));
					Tn_def[temp] = "#";//不能替代了
				}
				break;
			case op_int:
			case op_char:
			case op_void:
			case op_call:
				j_line = i;
				break;
			case op_push:
				if (midcodes[i].src1[0] == '@')
				{
					int temp = str2int(midcodes[i].src1.substr(2));
					if (Tn_def[temp] != "#" && (GTn_def[temp] > j_line)) {
						midcodes[i].src1 = Tn_def[temp];
					}
				}
				break;
			case op_RETvalue:
				if (midcodes[i].des[0] == '@')
				{
					int temp = str2int(midcodes[i].des.substr(2));
					Tn_def[temp] = "#";//不能替代了
				}
				break;
			case op_ret:
				j_line = i;
				if (midcodes[i].des[0] == '@')
				{
					int temp = str2int(midcodes[i].des.substr(2));
					if (Tn_def[temp] != "#" && (GTn_def[temp] > j_line)) {
						midcodes[i].des = Tn_def[temp];
					}
				}
				break;
			case op_end:
				j_line = i;
				break;
			case op_assign:
				//动全局变量

				if (midcodes[i].src1[0] == '@')
				{
					int temp = str2int(midcodes[i].src1.substr(2));
					if (Tn_def[temp] != "#" && (GTn_def[temp] > j_line)) {
						midcodes[i].src1 = Tn_def[temp];
					}
				}
				if ((midcodes[i].src1[0] != '@') && midcodes[i].des[0] == '@')
				{
					if (symbolTable[0].maptable.count(midcodes[i].src1) == 0) {//局部变量 
					}
					else {//全局变量 
						int temp = str2int(midcodes[i].des.substr(2));
						GTn_def[temp] = i;
						Tn_def[temp] = midcodes[i].src1;
					}
				}


				break;
			case op_addreq://[]= 
			case op_eqaddr:
				//参数一
				if (midcodes[i].src1[0] == '@')
				{
					int temp = str2int(midcodes[i].src1.substr(2));
					if (Tn_def[temp] != "#" && (GTn_def[temp] > j_line)) {
						midcodes[i].src1 = Tn_def[temp];
					}
				}
				//参数二
				if (midcodes[i].src2[0] == '@')
				{
					int temp = str2int(midcodes[i].src2.substr(2));
					if (Tn_def[temp] != "#" && (GTn_def[temp] > j_line)) {
						midcodes[i].src2 = Tn_def[temp];
					}
				}
				//des
				if (midcodes[i].des[0] == '@')
				{
					int temp = str2int(midcodes[i].des.substr(2));
					Tn_def[temp] = "#";//不能替代了
				}
				break;

			case op_printf:
				if (midcodes[i].des[0] == '@')
				{
					int temp = str2int(midcodes[i].des.substr(2));
					if (Tn_def[temp] != "#" && (GTn_def[temp] > j_line)) {
						midcodes[i].des = Tn_def[temp];
					}
				}
				break;
			case op_scanf:
				break;
			case op_lable:
				j_line = i;
				break;
			case op_GOTO:
				j_line = i;
				break;

			case op_EQ:
			case op_NOTEQ:
			case op_LESSEQ:
			case op_LESS:
			case op_MOREEQ:
			case op_MORE:
				//参数一
				if (midcodes[i].src1[0] == '@')
				{
					int temp = str2int(midcodes[i].src1.substr(2));
					if (Tn_def[temp] != "#" && (GTn_def[temp] > j_line)) {
						midcodes[i].src1 = Tn_def[temp];
					}
				}
				//参数二
				if (midcodes[i].src2[0] == '@')
				{
					int temp = str2int(midcodes[i].src2.substr(2));
					if (Tn_def[temp] != "#" && (GTn_def[temp] > j_line)) {
						midcodes[i].src2 = Tn_def[temp];
					}
				}
				j_line = i;
				break;


			default:
				error("中间码有bug");
				break;
			}
		}


		//删除死代码 只删除临时变量的定义
		map<string, int> var_map;//int -1代表使用过 大于0的值代表其定义处(没考虑重复定义的情况)
		for (int i = 0; i < midcode_index; i++) {//第一遍 替换Tn

			if (midcodes[i].op == op_none) {//代表为空
				continue;
			}
			switch (midcodes[i].op)
			{
			case op_ADD://二元运算
			case op_SUB:
			case op_MULT:
			case op_DIV:
				//参数一
				if (is_num(midcodes[i].src1)==0)
				{
					var_map[midcodes[i].src1] = -1;
				}
				//参数二
				if (is_num(midcodes[i].src2) == 0)
				{
					var_map[midcodes[i].src2] = -1;
				}
				//des
				if (is_num(midcodes[i].des) == 0)
				{
					if (var_map.count(midcodes[i].des) == 0) {
						var_map[midcodes[i].des] = i;
					}
				}
				break;
			case op_int:
			case op_char:
			case op_void:
			case op_call:
	
				break;
			case op_push:
				if (is_num(midcodes[i].src1)== 0)
				{
					var_map[midcodes[i].src1] = -1;
				}
				break;
			case op_RETvalue:
				if (is_num(midcodes[i].des) == 0)
				{
					if (var_map.count(midcodes[i].des) == 0) {
						var_map[midcodes[i].des] = i;
					}
				}
				break;
			case op_ret:
				if (is_num(midcodes[i].des) == 0)
				{
					//cout << var_map.count(midcodes[i].des) << endl;

					var_map[midcodes[i].des] = -1;
				
				}
				break;
			case op_end:

				break;
			case op_assign:
			

				if (is_num(midcodes[i].src1) == 0)
				{
					var_map[midcodes[i].src1] = -1;
				}
				if (is_num(midcodes[i].des) == 0)
				{
					if (var_map.count(midcodes[i].des) == 0) {
						var_map[midcodes[i].des] = i;
					}
				}
				break;
			case op_addreq://[]= 
				if (is_num(midcodes[i].src1) == 0)
				{
					var_map[midcodes[i].src1] = -1;
				}
				//参数二
				if (is_num(midcodes[i].src2) == 0)
				{
					var_map[midcodes[i].src2] = -1;
				}
				break;
			case op_eqaddr:
				//参数一
				if (is_num(midcodes[i].src1) == 0)
				{
					var_map[midcodes[i].src1] = -1;
				}
				
				if (is_num(midcodes[i].des) == 0)
				{
					if (var_map.count(midcodes[i].des) == 0) {
						var_map[midcodes[i].des] = i;
					}
				}
				break;

			case op_printf:
				if (is_num(midcodes[i].des) == 0&& midcodes[i].des[0]!='\"')
				{
					var_map[midcodes[i].des] = -1;
				}
				break;
			case op_scanf:
				if (is_num(midcodes[i].des) == 0)
				{
					if (var_map.count(midcodes[i].des) == 0) {
						var_map[midcodes[i].des] = i;
					}
				}
				break;
			case op_lable:

				break;
			case op_GOTO:

				break;

			case op_EQ:
			case op_NOTEQ:
			case op_LESSEQ:
			case op_LESS:
			case op_MOREEQ:
			case op_MORE:
				//参数一
				if (is_num(midcodes[i].src1) == 0)
				{
					var_map[midcodes[i].src1] = -1;
				}
				//参数二
				if (is_num(midcodes[i].src2) == 0)
				{
					var_map[midcodes[i].src2] = -1;
				}

				break;


			default:
				error("中间码有bug");
				break;
			}
		}
		//只删临时变量
		map<string, int>::iterator iter;//迭代器 
		iter = var_map.begin();
		while (iter !=var_map.end()) {
			if (iter->second != -1&& symbolTable[0].maptable.count(iter->first) == 0) {//非全局量
				if (midcodes[iter->second].op == op_assign) {
					midcodes[iter->second].op = op_none;
				}
				/*
				if (midcodes[iter->second].op != op_ret) {
					midcodes[iter->second].op = op_none;
				}*/
				
			}


			iter++;
		}
		
	}

	

	
}


