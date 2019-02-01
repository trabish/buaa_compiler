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


//��Ӧ�����������ɺ���
int tn_index = 0;//get_tn
int lable_index = 0;//get_lable
int string_index = 0;//get_string

int midcode_index = 0;
int mipscode_index = 0;


static int SymbolTable_index = 0;//��ǰbtab
static int now_para_num = 0;//�������в�������
static string para_list_array[Max_para];//��Ų���������  pushʱ������˳����  callʱ ����˳���para_list��ȡ������������ջ

map<operater, string> operater_map = { {op_ADD,"ADD"},{op_SUB,"SUB"},{op_MULT,"MULT"},{op_DIV,"DIV"},{op_int,"int"},{op_char,"char"},{op_void,"void"},
	{op_call,"call"},{op_push,"push"},{op_RETvalue,"RETvalue"},{op_ret,"ret"},{op_end,"end"},{op_assign,"assign"},
	{op_addreq,"[]="},{op_eqaddr,"=[]"},{op_printf,"printf"},{op_scanf,"scanf"},{op_lable,"lable"},{op_GOTO,"GOTO"},
	{op_BNZ,"BNZ"},{op_BZ,"BZ"},{op_EQ,"EQ"},{op_NOTEQ,"NOTEQ"},{op_LESS,"LESS"},{op_LESSEQ,"LESSEQ"},
	{op_MORE,"MORE"},{op_MOREEQ,"MOREEQ"}
};
////////////////////////////////////////////////
/////////////�Ĵ�����//////////////////////////
////////////////////////////////////////////////

register_pool reg_pool[10];
bool search_reg_pool(string name, string & temp_reg) {//����reg_pool 
	for (int i = 0; i < 10; i++) {
		if (reg_pool[i].ID == name) {
			temp_reg = "$t" + to_string(i);//�õ��Ǹ��Ĵ���
			reg_pool[i].order = now_order++;//����order ˵�����ʹ��
			return true;
		}
	}
	return false;//û���ҵ�
}


void get_next_temp_reg(string name, string &temp_reg) {//������һ����ʱ�Ĵ���
	int min_index = 0;
	int min_order = reg_pool[0].order;
	for (int i = 1; i < 10; i++) {
		if (reg_pool[i].order < min_order) {
			min_index = i;
			min_order = reg_pool[i].order;
		}
	}
	//����Сorder���Ǹ��Ĵ��������ȥ
	//��� �Ǹ��Ĵ�������� д���ڴ�
	if (reg_pool[min_index].dirty_mark == true) {
		int offset;
		if (find_addr(reg_pool[min_index].ID, offset) == true) {//�ֲ�����
			enter_mipscode("sw $t" + to_string(min_index) + " " + to_string(offset) + "($sp)");  //sw $ti offset($sp)
		}
		else {//ȫ�ֱ���
			enter_mipscode("sw $t" + to_string(min_index) + " " + to_string(offset));  //sw $ti offset
		}
	}
	temp_reg = "$t" + to_string(min_index);
	reg_pool[min_index].ID = name;
	reg_pool[min_index].order = now_order++;
	reg_pool[min_index].dirty_mark = false;
}
void initail_reg_pool() {//���������֮ǰ ��ʼ�� �Ĵ�����
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
void empty_reg_pool() {//�������� �����ݷŻ��ڴ�
	for (int i = 0; i < 10; i++) {
		if (reg_pool[i].ID.empty()) {

		}
		else if (reg_pool[i].dirty_mark == true) {//��Ϊ�� ���� ��Ҫ�Ż��ڴ�
			int offset;
			if (find_addr(reg_pool[i].ID, offset) == true) {//�ֲ�����
				enter_mipscode("sw $t" + to_string(i) + " " + to_string(offset) + "($sp)");  //sw $ti offset($sp)
			}
			else {//ȫ�ֱ���
				enter_mipscode("sw $t" + to_string(i) + " " + to_string(offset));  //sw $ti offset
			}
		}
	}
	initail_reg_pool();//Ϊ��һ�γ�ʼ��
}
bool alloc_reg_pool(string name, string & temp_reg) {//������ʱ�Ĵ����� true �����Ѿ��ڼĴ�������
	if (search_reg_pool(name, temp_reg) == true) {
		return true;//�������
	}
	else {
		get_next_temp_reg(name, temp_reg);
		return false;
	}
}

void alloc_temp_reg(string &temp_reg) {//����һ����ʱ�Ĵ��� ����¼����Ϣ
	int min_index = 0;
	int min_order = reg_pool[0].order;
	for (int i = 0; i < 10; i++) {
		if (reg_pool[i].order < min_order) {
			min_index = i;
			min_order = reg_pool[i].order;
		}
	}
	//����Сorder���Ǹ��Ĵ��������ȥ
	//��� �Ǹ��Ĵ�������� д���ڴ�
	if (reg_pool[min_index].dirty_mark == true) {
		int offset;
		if (find_addr(reg_pool[min_index].ID, offset) == true) {//�ֲ�����
			enter_mipscode("sw $t" + to_string(min_index) + " " + to_string(offset) + "($sp)");  //sw $ti offset($sp)
		}
		else {//ȫ�ֱ���
			enter_mipscode("sw $t" + to_string(min_index) + " " + to_string(offset));  //sw $ti offset
		}
	}
	temp_reg = "$t" + to_string(min_index);
	reg_pool[min_index].ID = "";
	reg_pool[min_index].dirty_mark = false;
	reg_pool[min_index].order = 0;//��Ϊ����һ����ʱ�ļĴ��� �ᱻ���ȷ����ȥ
}


////////////////////////////////////////////
void get_tn(string &Tn) { //��ʱ���� �����������ű�
	stringstream ss;
	ss << tn_index++;
	Tn = "@T" + ss.str();
	register_temp(Tn);//������ű�
}
void get_lable(string &Tn) { //��ת��lable
	stringstream ss;
	ss << lable_index++;
	Tn = "lable" + ss.str();
}

void get_string(string &key,string value) {
	stringstream ss;
	ss << string_index++;
	key = "$string" + ss.str(); // $xxx ������Ϊmips�ı�ʶ�� ��������Ϊ���Ǳ������ı�ʶ��
	register_string(key, value);
}

int str2int( const string &string_temp)
{
	int int_temp;
	stringstream stream(string_temp);
	stream >> int_temp;
	return int_temp;
}

//����м����
int enter_midcode(operater op, string src1, string src2, string des) {
	if (midcode_index>=MAX_midcode) {
		error("midcodes �����");//midcodes �����
		return 0;
	}
	midcodes[midcode_index].op = op;
	midcodes[midcode_index].src1 = src1;
	midcodes[midcode_index].src2 = src2;
	midcodes[midcode_index].des = des;
	midcode_index++;
	return midcode_index - 1;//����ָ���±�
}

//���mips �����
int enter_mipscode(string instruction) {
	if (mipscode_index >= MAX_mipscode) {
		error("mipscodes �����");
		return 0;
	}
	mipscodes[mipscode_index] = instruction;
	mipscode_index++;
	return mipscode_index - 1;//����ָ���±�
}

//����м���
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
		case op_ADD://��Ԫ����
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
			//����һ
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
			//����һ
			fout << operater_map[midcodes[i].op] << "  " << midcodes[i].src1 << "  " << midcodes[i].src2 << "  " << midcodes[i].des << endl;
			break;


		default:
			error("�м�����bug");
			return ;
			break;
		}
		
	}
}
//���Ŀ����
void out_mips() {
	ofstream fout;
	fout.open("mips.txt");
	for (int i = 0; i < mipscode_index; i++) {
		//cout << mipscodes[i] << endl;
		fout << mipscodes[i] << endl;
	}
}
void register_temp(string name) {//ע����ʱ����
	if (symbolTable[which_Table].maptable.count(name)!=0) {
		return; //��� ����Ҫ�ٴ���д
	}
	table_item temp;
	temp.name = name;// �����int һ���Ŀռ� 
	temp.obj = obj_var;
	temp.type = type_int;//ȫ������δint  !!!������
	symbolTable[which_Table].maptable[name] = temp;
}

void register_string(string key,string value) {
	if (stringTable.count(key) != 0) {
		error("��ô�����أ�");
		return ;
	}
	else {
		stringTable[key] = value;
	}
}
 
void calculate_offset(int now_table) {//����ñ��з�������ռ�� offset  ע��ջ��������
	int offset = 12+4*8;//��ҪΪ $ra �� ret ����ռ� ����һ�� $fp   //Ϊ$s�Ĵ����������ÿռ�
	map<string, table_item>::iterator iter;
	iter = symbolTable[now_table].maptable.begin();
	while (iter != symbolTable[now_table].maptable.end()) {
		if (iter->second.obj == obj_var|| iter->second.obj==obj_para) {//���� �� ���� �ŷ���ռ�
			iter->second.offset = offset;
			if (iter->second.type == type_int_group || iter->second.type == type_char_group) {//����
				offset = offset +4 * iter->second.length;
			}
			else {
				offset = offset + 4;//int char �ռ��Сһ��
			}
		}
		iter++;
	}

	symbolTable[now_table].vsize = offset; //��С����offset
}
void handle_table() {//������ű�   ���Գ��� ����offset  

	int now_table=1;//��ʼ�ǵ�һ������
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
void mips_init() { //����offset    �����ַ������� ȫ�ֱ���
	
	{//Ϊ�˽������ֵ������ δ��������ȫ�ֱ���
		table_item temp_item;
		string name = "$para" ;
		temp_item.name = name;
		temp_item.obj = obj_var;
		temp_item.type = type_int_group;

		temp_item.length = Max_para;
		symbolTable[0].maptable[name] = temp_item;

	}

	handle_table();//����offset

	//Ϊȫ�ֱ�������ռ�
	enter_mipscode(".data");//������ ��ų����ַ��� �� ȫ�ֱ���(��ǰ�����ȫ�ֱ�����λ��)
	int globle_offset = 0x10010000;//ȫ�ֱ����� ƫ��λ��   ��Ϊdefault
	map<string, table_item>::iterator iter;
	iter = symbolTable[0].maptable.begin();
	while (iter != symbolTable[0].maptable.end()) {
		if (iter->second.obj == obj_var) {//ֻΪ��������ռ� 
			switch (iter->second.type) {
			case type_char :
			case type_int:
				iter->second.offset = globle_offset;
				globle_offset = globle_offset + 4;
				enter_mipscode(iter->second.name+": .word 0");//�������0 �������ռ�
				break;
			case type_int_group: 
			case type_char_group:
				iter->second.offset = globle_offset;
				globle_offset =globle_offset+ 4 * iter->second.length;
				enter_mipscode(iter->second.name + ": .word 0:" + to_string(iter->second.length));//Ϊ����Ƿ���ռ� 
			}
		}
		iter++;
	}

	//Ϊ�����ַ��� ����ռ� ���ַ�������
	map<string, string>::iterator string_iter;
	string_iter = stringTable.begin();
	while (string_iter!=stringTable.end()) {
		enter_mipscode(string_iter->first+": .asciiz "+"\""+string_iter->second+"\\n\"");  //ע��Ҫ���˫���� 
		string_iter++;
	}

	//��ʼ�� $sp $fp  ����ת��������
	enter_mipscode(".text");//�����
	//enter_mipscode("move $fp $sp");//$sp�ʼ��ֵ  
	enter_mipscode("subi $sp $sp "+to_string(symbolTable[symbolTable[0].maptable["main"].adr].vsize));//Ϊmain ����ռ�
	enter_mipscode("j main");
}

bool find_addr(string name,int &offset) {//�ӷ��ű��в��� �����ĵ�ַ    �����ȫ�ֱ�������false ����Ǿֲ�������true
	if (symbolTable[SymbolTable_index].maptable.count(name) != 0) {
		//�ֲ�����
		offset = symbolTable[SymbolTable_index].maptable[name].offset;
		return true;
	}
	else if (symbolTable[0].maptable.count(name) != 0) {
		offset = symbolTable[0].maptable[name].offset;
		return false;
	}
	else {
		cout << name << endl;

		error("��϶���bug");

		return false;
	}
}

int  is_num(string name) { // -1 ���ַ���    0 ������     1 ����
	if (name.size() == 0) {
		return -1;//��string
	}
	else if(name.size()==1){
		if (name[0] >= '0'&&name[0] <= '9')
		{
			return 1;//����
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
			return 0;//������
		}
		else {
			return 1;//����
		}
	}

}
/////////////////////////////////////////////////////////////////////////////////
////                              ����ÿһ��midcode                          ////
/////////////////////////////////////////////////////////////////////////////////
void midcode_ADD(midcode instruction) { //add $t0 $t0 $t1
	int offset_src1;
	int offset_src2;
	int offset_des;

	string reg_1;
	string reg_2;
	string reg_3;
	if (is_num(instruction.src2) == 1) {//���� ������addi
	//src1
		if (is_num(instruction.src1) == 1) {//����
			if (alloc_reg_pool(instruction.src1, reg_1) == false) {//û�ڼĴ�������
				enter_mipscode("li " + reg_1 + " " + instruction.src1);
			}
		}
		else {//������
			if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.src1) != 0)//�Ѿ�������ȫ�ּĴ���
			{
				reg_1 = function_Reference_data[SymbolTable_index].map_reg[instruction.src1];//��ø�ȫ�ּĴ���	
			}
			else {//������ʱ�Ĵ�������
				if (alloc_reg_pool(instruction.src1, reg_1) == false) {//û�ڼĴ�������
					if (find_addr(instruction.src1, offset_src1) == true) {//�ֲ�����
						enter_mipscode("lw " + reg_1 + " " + to_string(offset_src1) + "($sp)");  //lw $ti offset($sp)
					}
					else {//ȫ�ֱ���
						enter_mipscode("lw " + reg_1 + " " + to_string(offset_src1));  //lw $ti offset
					}
				}
			}

		}
		

		//des
		if (is_num(instruction.des) == 0) {//������ �ǿ�
			if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.des) != 0)//�Ѿ�������ȫ�ּĴ���
			{
				reg_3 = function_Reference_data[SymbolTable_index].map_reg[instruction.des];//��ø�ȫ�ּĴ���

			}
			else {//������ʱ�Ĵ�������
				alloc_reg_pool(instruction.des, reg_3);
				//������Ϊ�� ��Ϊ��des
				dirty_reg_pool(reg_3);
			}
		}
		else {
			error("�Ҳ������ܵ�����");
			return ;
		}
		enter_mipscode("addi " + reg_3 + " " + reg_1 + " " + instruction.src2);
	}
	else {
		if (is_num(instruction.src1) == 1) {//����
			if (alloc_reg_pool(instruction.src1, reg_1) == false) {//û�ڼĴ�������
				enter_mipscode("li " + reg_1 + " " + instruction.src1);
			}
		}
		else {//������
			if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.src1) != 0)//�Ѿ�������ȫ�ּĴ���
			{
				reg_1 = function_Reference_data[SymbolTable_index].map_reg[instruction.src1];//��ø�ȫ�ּĴ���	
			}
			else {//������ʱ�Ĵ�������
				if (alloc_reg_pool(instruction.src1, reg_1) == false) {//û�ڼĴ�������
					if (find_addr(instruction.src1, offset_src1) == true) {//�ֲ�����
						enter_mipscode("lw " + reg_1 + " " + to_string(offset_src1) + "($sp)");  //lw $ti offset($sp)
					}
					else {//ȫ�ֱ���
						enter_mipscode("lw " + reg_1 + " " + to_string(offset_src1));  //lw $ti offset
					}
				}
			}

		}
		//src2
		if (is_num(instruction.src2) == 1) {//����
			if (alloc_reg_pool(instruction.src2, reg_2) == false) {//û�ڼĴ�������
				enter_mipscode("li " + reg_2 + " " + instruction.src2);
			}
		}
		else {//������
			if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.src2) != 0)//�Ѿ�������ȫ�ּĴ���
			{
				reg_2 = function_Reference_data[SymbolTable_index].map_reg[instruction.src2];//��ø�ȫ�ּĴ���

			}
			else {//������ʱ�Ĵ�������
				if (alloc_reg_pool(instruction.src2, reg_2) == false) {//û�ڼĴ�������
					if (find_addr(instruction.src2, offset_src2) == true) {//�ֲ�����
						enter_mipscode("lw " + reg_2 + " " + to_string(offset_src2) + "($sp)");  //lw $ti offset($sp)
					}
					else {//ȫ�ֱ���
						enter_mipscode("lw " + reg_2 + " " + to_string(offset_src2));  //lw $ti offset
					}
				}
			}
		}

		//des
		if (is_num(instruction.des) == 0) {//������ �ǿ�
			if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.des) != 0)//�Ѿ�������ȫ�ּĴ���
			{
				reg_3 = function_Reference_data[SymbolTable_index].map_reg[instruction.des];//��ø�ȫ�ּĴ���

			}
			else {//������ʱ�Ĵ�������
				alloc_reg_pool(instruction.des, reg_3);
				//������Ϊ�� ��Ϊ��des
				dirty_reg_pool(reg_3);
			}
		}
		else {
			error("�Ҳ������ܵ�����");
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


	if (is_num(instruction.src2) == 1) {//���� ������subi

		if (is_num(instruction.src1) == 1) {//����
			if (alloc_reg_pool(instruction.src1, reg_1) == false) {//û�ڼĴ�������
				enter_mipscode("li " + reg_1 + " " + instruction.src1);
			}
		}
		else {//������
			if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.src1) != 0)//�Ѿ�������ȫ�ּĴ���
			{
				reg_1 = function_Reference_data[SymbolTable_index].map_reg[instruction.src1];//��ø�ȫ�ּĴ���	
			}
			else {//������ʱ�Ĵ�������
				if (alloc_reg_pool(instruction.src1, reg_1) == false) {//û�ڼĴ�������
					if (find_addr(instruction.src1, offset_src1) == true) {//�ֲ�����
						enter_mipscode("lw " + reg_1 + " " + to_string(offset_src1) + "($sp)");  //lw $ti offset($sp)
					}
					else {//ȫ�ֱ���
						enter_mipscode("lw " + reg_1 + " " + to_string(offset_src1));  //lw $ti offset
					}
				}
			}

		}
		

		//des
		if (is_num(instruction.des) == 0) {//������ �ǿ�
			if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.des) != 0)//�Ѿ�������ȫ�ּĴ���
			{
				reg_3 = function_Reference_data[SymbolTable_index].map_reg[instruction.des];//��ø�ȫ�ּĴ���

			}
			else {//������ʱ�Ĵ�������
				alloc_reg_pool(instruction.des, reg_3);
				//������Ϊ�� ��Ϊ��des
				dirty_reg_pool(reg_3);
			}
		}
		else {
			error("�Ҳ������ܵ�����");
			return;
		}
		enter_mipscode("subi " + reg_3 + " " + reg_1 + " " + instruction.src2);
	}
	else {//src2 ������
		if (is_num(instruction.src1) == 1) {//����
			if (alloc_reg_pool(instruction.src1, reg_1) == false) {//û�ڼĴ�������
				enter_mipscode("li " + reg_1 + " " + instruction.src1);
			}
		}
		else {//������
			if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.src1) != 0)//�Ѿ�������ȫ�ּĴ���
			{
				reg_1 = function_Reference_data[SymbolTable_index].map_reg[instruction.src1];//��ø�ȫ�ּĴ���	
			}
			else {//������ʱ�Ĵ�������
				if (alloc_reg_pool(instruction.src1, reg_1) == false) {//û�ڼĴ�������
					if (find_addr(instruction.src1, offset_src1) == true) {//�ֲ�����
						enter_mipscode("lw " + reg_1 + " " + to_string(offset_src1) + "($sp)");  //lw $ti offset($sp)
					}
					else {//ȫ�ֱ���
						enter_mipscode("lw " + reg_1 + " " + to_string(offset_src1));  //lw $ti offset
					}
				}
			}

		}
		//src2
		if (is_num(instruction.src2) == 1) {//����
			if (alloc_reg_pool(instruction.src2, reg_2) == false) {//û�ڼĴ�������
				enter_mipscode("li " + reg_2 + " " + instruction.src2);
			}
		}
		else {//������
			if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.src2) != 0)//�Ѿ�������ȫ�ּĴ���
			{
				reg_2 = function_Reference_data[SymbolTable_index].map_reg[instruction.src2];//��ø�ȫ�ּĴ���

			}
			else {//������ʱ�Ĵ�������
				if (alloc_reg_pool(instruction.src2, reg_2) == false) {//û�ڼĴ�������
					if (find_addr(instruction.src2, offset_src2) == true) {//�ֲ�����
						enter_mipscode("lw " + reg_2 + " " + to_string(offset_src2) + "($sp)");  //lw $ti offset($sp)
					}
					else {//ȫ�ֱ���
						enter_mipscode("lw " + reg_2 + " " + to_string(offset_src2));  //lw $ti offset
					}
				}
			}
		}

		//des
		if (is_num(instruction.des) == 0) {//������ �ǿ�
			if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.des) != 0)//�Ѿ�������ȫ�ּĴ���
			{
				reg_3 = function_Reference_data[SymbolTable_index].map_reg[instruction.des];//��ø�ȫ�ּĴ���

			}
			else {//������ʱ�Ĵ�������
				alloc_reg_pool(instruction.des, reg_3);
				//������Ϊ�� ��Ϊ��des
				dirty_reg_pool(reg_3);
			}
		}
		else {
			error("�Ҳ������ܵ�����");
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
	if (is_num(instruction.src1) == 1) {//����
		if (alloc_reg_pool(instruction.src1, reg_1) == false) {//û�ڼĴ�������
			enter_mipscode("li " + reg_1 + " " + instruction.src1);
		}
	}
	else {//������
		if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.src1) != 0)//�Ѿ�������ȫ�ּĴ���
		{
			reg_1 = function_Reference_data[SymbolTable_index].map_reg[instruction.src1];//��ø�ȫ�ּĴ���	
		}
		else {//������ʱ�Ĵ�������
			if (alloc_reg_pool(instruction.src1, reg_1) == false) {//û�ڼĴ�������
				if (find_addr(instruction.src1, offset_src1) == true) {//�ֲ�����
					enter_mipscode("lw " + reg_1 + " " + to_string(offset_src1) + "($sp)");  //lw $ti offset($sp)
				}
				else {//ȫ�ֱ���
					enter_mipscode("lw " + reg_1 + " " + to_string(offset_src1));  //lw $ti offset
				}
			}
		}

	}
	//src2
	if (is_num(instruction.src2) == 1) {//����
		if (alloc_reg_pool(instruction.src2, reg_2) == false) {//û�ڼĴ�������
			enter_mipscode("li " + reg_2 + " " + instruction.src2);
		}
	}
	else {//������
		if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.src2) != 0)//�Ѿ�������ȫ�ּĴ���
		{
			reg_2 = function_Reference_data[SymbolTable_index].map_reg[instruction.src2];//��ø�ȫ�ּĴ���

		}
		else {//������ʱ�Ĵ�������
			if (alloc_reg_pool(instruction.src2, reg_2) == false) {//û�ڼĴ�������
				if (find_addr(instruction.src2, offset_src2) == true) {//�ֲ�����
					enter_mipscode("lw " + reg_2 + " " + to_string(offset_src2) + "($sp)");  //lw $ti offset($sp)
				}
				else {//ȫ�ֱ���
					enter_mipscode("lw " + reg_2 + " " + to_string(offset_src2));  //lw $ti offset
				}
			}
		}
	}

	//des
	if (is_num(instruction.des) == 0) {//������ �ǿ�
		if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.des) != 0)//�Ѿ�������ȫ�ּĴ���
		{
			reg_3 = function_Reference_data[SymbolTable_index].map_reg[instruction.des];//��ø�ȫ�ּĴ���

		}
		else {//������ʱ�Ĵ�������
			alloc_reg_pool(instruction.des, reg_3);
			//������Ϊ�� ��Ϊ��des
			dirty_reg_pool(reg_3);
		}
	}
	else {
		error("�Ҳ������ܵ�����");
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
	if (is_num(instruction.src1) == 1) {//����
		if (alloc_reg_pool(instruction.src1, reg_1) == false) {//û�ڼĴ�������
			enter_mipscode("li " + reg_1 + " " + instruction.src1);
		}
	}
	else {//������
		if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.src1) != 0)//�Ѿ�������ȫ�ּĴ���
		{
			reg_1 = function_Reference_data[SymbolTable_index].map_reg[instruction.src1];//��ø�ȫ�ּĴ���	
		}
		else {//������ʱ�Ĵ�������
			if (alloc_reg_pool(instruction.src1, reg_1) == false) {//û�ڼĴ�������
				if (find_addr(instruction.src1, offset_src1) == true) {//�ֲ�����
					enter_mipscode("lw " + reg_1 + " " + to_string(offset_src1) + "($sp)");  //lw $ti offset($sp)
				}
				else {//ȫ�ֱ���
					enter_mipscode("lw " + reg_1 + " " + to_string(offset_src1));  //lw $ti offset
				}
			}
		}

	}
	//src2
	if (is_num(instruction.src2) == 1) {//����
		if (alloc_reg_pool(instruction.src2, reg_2) == false) {//û�ڼĴ�������
			enter_mipscode("li " + reg_2 + " " + instruction.src2);
		}
	}
	else {//������
		if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.src2) != 0)//�Ѿ�������ȫ�ּĴ���
		{
			reg_2 = function_Reference_data[SymbolTable_index].map_reg[instruction.src2];//��ø�ȫ�ּĴ���

		}
		else {//������ʱ�Ĵ�������
			if (alloc_reg_pool(instruction.src2, reg_2) == false) {//û�ڼĴ�������
				if (find_addr(instruction.src2, offset_src2) == true) {//�ֲ�����
					enter_mipscode("lw " + reg_2 + " " + to_string(offset_src2) + "($sp)");  //lw $ti offset($sp)
				}
				else {//ȫ�ֱ���
					enter_mipscode("lw " + reg_2 + " " + to_string(offset_src2));  //lw $ti offset
				}
			}
		}
	}

	//des
	if (is_num(instruction.des) == 0) {//������ �ǿ�
		if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.des) != 0)//�Ѿ�������ȫ�ּĴ���
		{
			reg_3 = function_Reference_data[SymbolTable_index].map_reg[instruction.des];//��ø�ȫ�ּĴ���

		}
		else {//������ʱ�Ĵ�������
			alloc_reg_pool(instruction.des, reg_3);
			//������Ϊ�� ��Ϊ��des
			dirty_reg_pool(reg_3);
		}
	}
	else {
		error("�Ҳ������ܵ�����");
		return;
	}
	enter_mipscode("div " + reg_1 + " " + reg_2);
	enter_mipscode("mflo " + reg_3);
}


void midcode_function(midcode instruction) { //int function  ,char function ,void function
	if (instruction.src1 != "function") {
		error("������ �Ҳ����������bug");
		return;
	 }
	else {

	

		empty_reg_pool();//��ռĴ�����
		

		SymbolTable_index = symbolTable[0].maptable[instruction.des].adr; //�л�btab  ͬʱҲ�л���ȫ�ּĴ�������ı�

		enter_mipscode(symbolTable[SymbolTable_index].function_name + ":"); //Ϊ�������� lable

		//�����м�ֵ��$s
		map<string, string>::iterator iter;//������
		iter = function_Reference_data[SymbolTable_index].map_reg.begin();
		int i = 12;
		while (iter != function_Reference_data[SymbolTable_index].map_reg.end()) {
			enter_mipscode("sw " + iter->second + " " + to_string(i) + "($sp)");
			iter++;
			i = i + 4;
		}
		//��ʼ��ȫ�ּĴ������� ���ز�����ֵ  ��Ϊֻ�в����г�ֵ�� ȫ�ֱ���δ����ȫ�ּĴ�������
		//��Ҫ���л�btab���� 
		map<string, string>::iterator iter2;//������
		iter2 = function_Reference_data[SymbolTable_index].map_reg.begin();
		while (iter2 != function_Reference_data[SymbolTable_index].map_reg.end()) {
			int offset;
			//����ID ���Ƿ��ǲ��� 
			string now_name = iter2->first;
			if (symbolTable[SymbolTable_index].maptable.count(now_name) != 0 && symbolTable[SymbolTable_index].maptable[now_name].obj == obj_para) {//����ǲ���
			   //�ǲ���
				find_addr(now_name, offset);
				enter_mipscode("lw " + iter2->second + " " + to_string(offset) + "($sp)");
			}


			iter2++;
		}

		
		//���ﲻ��Ҫ��ȡ���� ��ȡ������call function ����		
		enter_mipscode("sw $ra 0($sp)");//��Ҫ����$ra 
	}
}

void midcode_push(midcode instruction) {
	int offset;
	if (now_para_num >= Max_para) {
		error("������ ǰ��͸ñ���");
		return;
	}
	else {
		//�����ȫ�ֱ��� �ͽ���ֵ����$para������


		para_list_array[now_para_num++] = instruction.src1;

		if (is_num(instruction.src1) == 1) {//���� ����
			
		}
		else if(find_addr(instruction.src1, offset) == false){//ȫ�ֱ���

			find_addr("$para", offset);//�ҵ�$para��λ��

			string reg_1;
			int offset_src1;
			if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.src1) != 0)//�Ѿ�������ȫ�ּĴ���
			{
				reg_1 = function_Reference_data[SymbolTable_index].map_reg[instruction.src1];//��ø�ȫ�ּĴ���	
				//��ֵ������$para ������

				enter_mipscode("sw " + reg_1 + " " + to_string(offset + (now_para_num - 1) * 4));
				
			}
			else {//������ʱ�Ĵ�������
				if (alloc_reg_pool(instruction.src1, reg_1) == false) {//û�ڼĴ�������
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

void midcode_call(midcode instruction) {//1.$fg���� 2.$�����¿ռ� 3.��������������ջ 4.��ת������

	
	int offset;

	Table now_table = symbolTable[symbolTable[0].maptable[instruction.des].adr];
	int vsize = now_table.vsize;//�ú�������ջ��С
	



	//�������
	//λ�ð�˳��һ����Ӧ
	now_para_num = now_para_num - now_table.paranum;
	//for (int i = 0; i < now_para_num; i++) {
	for (int i = 0; i < now_table.paranum; i++) {
		string temp_para = para_list_array[i + now_para_num];
		if (is_num(temp_para) == 1) {//����
			enter_mipscode("li $a0 " + temp_para);//����������
			//�������ָ��λ��  ����֮ǰû�ڲ������д�� offset  ������Ҫͨ������������maptable�в�ѯ  ��
			enter_mipscode("sw $a0 " + to_string(now_table.maptable[now_table.paralist[i].name].offset- vsize) + "($sp)");
		}
		else if (find_addr(temp_para, offset) == false) {//ȫ�ֱ��� ��Ҫ��$para �л�ȡֵ

			find_addr("$para", offset);//�ҵ�$para��λ��
			enter_mipscode("lw $a0 " + to_string(offset + (i + now_para_num ) * 4));
			enter_mipscode("sw $a0 " + to_string(now_table.maptable[now_table.paralist[i].name].offset - vsize) + "($sp)");
		}
		else {//������ ��ʱ����
			string reg_1;
			int offset_src1;
			if (function_Reference_data[SymbolTable_index].map_reg.count(temp_para) != 0)//�Ѿ�������ȫ�ּĴ���
			{
				reg_1 = function_Reference_data[SymbolTable_index].map_reg[temp_para];//��ø�ȫ�ּĴ���	
				enter_mipscode("add $a0 "+reg_1+" $0");//��ֵ��ֵ��$a0
			}
			else {//������ʱ�Ĵ�������
				if (alloc_reg_pool(temp_para, reg_1) == false) {//û�ڼĴ�������
					if (find_addr(temp_para, offset_src1) == true) {//�ֲ�����
						enter_mipscode("lw $a0 " + to_string(offset_src1) + "($sp)");  //lw $ti offset($sp)
					}
					else {//ȫ�ֱ���
						enter_mipscode("lw $a0 " + to_string(offset_src1));  //lw $ti offset
					}
				}
				else {//��ȫ�ּĴ�������  addָ���loadָ��Ȩ����
					enter_mipscode("add $a0 " + reg_1 + " $0");//��ֵ��ֵ��$a0
				}
			}
			//�洢��Ŀ��λ��
			enter_mipscode("sw $a0 " + to_string(now_table.maptable[now_table.paralist[i].name].offset- vsize) + "($sp)");
		}

	}
	//now_para_num = 0;







	empty_reg_pool();//��ռĴ�����

	//�����м�ֵ��$s
	/*map<string, string>::iterator iter;//������
	iter = function_Reference_data[SymbolTable_index].map_reg.begin();
	int i = 12;
	while (iter != function_Reference_data[SymbolTable_index].map_reg.end()) {
		enter_mipscode("sw "+iter->second+" "+to_string(i)+"($sp)");
		iter++;
		i = i + 4;
	}*/

	//enter_mipscode("sw $fp -" + to_string(vsize - 8) + "($sp)"); //���Ǳ���fp��λ��
	//enter_mipscode("add $fp $sp 0");//�µ� $fp
	
	enter_mipscode("subi $sp $sp " + to_string(vsize));//�µ�$sp

	//��ת������
	enter_mipscode("jal " + instruction.des);//ʹ��jal ���ص�ַ������$ra  function �ڲ�����$ra�ı���

	//�ָ�$s
	/* i = 12;
	 iter = function_Reference_data[SymbolTable_index].map_reg.begin();
	while (iter != function_Reference_data[SymbolTable_index].map_reg.end()) {
		enter_mipscode("lw " + iter->second + " " + to_string(i) + "($sp)");//�����õ���$s  ע��������õ���$sp
		iter++;
		i = i + 4;
	}*/
}

void midcode_ret(midcode instruction) { //1.������ֵ����ָ��λ��  2.����$ra 3.��ջ  4.jr $ra
	 //1.������ֵ����ָ��λ��  (�ƺ�����ֱ�ӷ���$v0)
	string reg;
	int offset;
	if (instruction.src1 == "exp") {//�з���ֵ
		if (is_num(instruction.des) == 1) {//����
			enter_mipscode("li $v0 " + instruction.des);
		}
		else {//������

			if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.des) != 0)//�Ѿ�������ȫ�ּĴ���
			{
				reg = function_Reference_data[SymbolTable_index].map_reg[instruction.des];//��ø�ȫ�ּĴ���
				enter_mipscode("add $v0 " + reg + " $0");//��ֵ��ֵ��$a0
			}
			else {//������ʱ�Ĵ�������
				if (alloc_reg_pool(instruction.des, reg) == false) {//û�ڼĴ������� ��ʱ���ǻ����һ���Ĵ��� ���ǽ�����������ת ����ռĴ�����
					if (find_addr(instruction.des, offset) == true) {//�ֲ�����
						enter_mipscode("lw $v0 " + to_string(offset) + "($sp)");  
					}
					else {//ȫ�ֱ���
						enter_mipscode("lw $v0 " + to_string(offset) );
					}
				}
				else {//�ڼĴ�������
					enter_mipscode("add $v0 " + reg + " $0");//��ֵ��ֵ��$v0
				}
			}
		
		}
	}
	else if (instruction.src1 == "null") {//�޷���ֵ

	}
	else {
		error("���ڶ���");
		return;
	}
	//�����ʱ�Ĵ�����
	//empty_reg_pool();

	for (int i = 0; i < 10; i++) {
		if (reg_pool[i].ID.empty()) {

		}
		else if (reg_pool[i].dirty_mark == true) {//��Ϊ�� ���� ��Ҫ�Ż��ڴ�
			int offset;
			if (find_addr(reg_pool[i].ID, offset) == true) {//�ֲ�����
				//enter_mipscode("sw $t" + to_string(i) + " " + to_string(offset) + "($sp)");  //sw $ti offset($sp)
			}
			else {//ȫ�ֱ���
				enter_mipscode("sw $t" + to_string(i) + " " + to_string(offset));  //sw $ti offset
			}
		}
	}
	initail_reg_pool();//Ϊ��һ�γ�ʼ��




	map<string, string>::iterator iter;//������
	iter = function_Reference_data[SymbolTable_index].map_reg.begin();
	int i = 12;
	while (iter != function_Reference_data[SymbolTable_index].map_reg.end()) {
		enter_mipscode("lw " + iter->second + " " + to_string(i) + "($sp)");
		iter++;
		i = i + 4;
	}

	//2.����$ra
	enter_mipscode("lw $ra 0($sp)");
	//3.��ջ 
	//enter_mipscode("add $sp $fp 0");
	 
	int vsize = symbolTable[SymbolTable_index].vsize;//�ú�������ջ��С
	enter_mipscode("addi $sp $sp "+to_string(vsize));
	//
	//enter_mipscode("lw $fp -" + to_string(vsize - 8) + "($sp)"); //���Ǳ���fp��λ��  
	
	

	//4.jr $ra
	if (symbolTable[SymbolTable_index].function_name == "main") {//main ���� ֱ����end
		enter_mipscode("j end");
	}
	else {
		enter_mipscode("jr $ra");
	}

}

void midcode_RETvalue(midcode instruction) {  //���ǵ��ú������ ��һ�����
	if (is_num(instruction.des) == 1) {//����
		error("��ô����������");
		return;
	}
	else {//������  

		string reg_3;
		//des
		if (is_num(instruction.des) == 0) {//������ �ǿ�
			if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.des) != 0)//�Ѿ�������ȫ�ּĴ���
			{
				reg_3 = function_Reference_data[SymbolTable_index].map_reg[instruction.des];//��ø�ȫ�ּĴ���

			}
			else {//������ʱ�Ĵ�������
				alloc_reg_pool(instruction.des, reg_3);
				//������Ϊ�� ��Ϊ��des
				dirty_reg_pool(reg_3);
			}
		}
		else {
			error("�Ҳ������ܵ�����");
			return;
		}
		enter_mipscode("add " + reg_3 + " $v0 $0");//������ֵ�������
		/*int ret_offset;
		if (find_addr(instruction.des, ret_offset) == true) {//�ֲ�����
			enter_mipscode("sw $v0 " + to_string(ret_offset) + "($sp)");
		}
		else {//ȫ�ֱ���
			enter_mipscode("sw $v0 " + to_string(ret_offset));
		}*/
	}
}

void midcode_end(midcode instruction) {//�൱�� ret  null
	//�����ʱ�Ĵ�����
	empty_reg_pool();
	for (int i = 0; i < 10; i++) {
		if (reg_pool[i].ID.empty()) {

		}
		else if (reg_pool[i].dirty_mark == true) {//��Ϊ�� ���� ��Ҫ�Ż��ڴ�
			int offset;
			if (find_addr(reg_pool[i].ID, offset) == true) {//�ֲ�����
				//enter_mipscode("sw $t" + to_string(i) + " " + to_string(offset) + "($sp)");  //sw $ti offset($sp)
			}
			else {//ȫ�ֱ���
				enter_mipscode("sw $t" + to_string(i) + " " + to_string(offset));  //sw $ti offset
			}
		}
	}
	initail_reg_pool();//Ϊ��һ�γ�ʼ��

	map<string, string>::iterator iter;//������
	iter = function_Reference_data[SymbolTable_index].map_reg.begin();
	int i = 12;
	while (iter != function_Reference_data[SymbolTable_index].map_reg.end()) {
		enter_mipscode("lw " + iter->second + " " + to_string(i) + "($sp)");
		iter++;
		i = i + 4;
	}

	// 1.����$ra
	enter_mipscode("lw $ra 0($sp)");
	//2.��ջ 
	//enter_mipscode("add $sp $fp 0");

	//int vsize = symbolTable[SymbolTable_index].vsize;//�ú�������ջ��С
	//enter_mipscode("lw $fp -" + to_string(vsize - 8) + "($sp)"); //���Ǳ���fp��λ��  
	int vsize = symbolTable[SymbolTable_index].vsize;//�ú�������ջ��С
	enter_mipscode("addi $sp $sp " + to_string(vsize));
	//3.jr $ra
	
	

	if (symbolTable[SymbolTable_index].function_name == "main") {//main ���� ����end
		//enter_mipscode("j end");
		enter_mipscode("end:");
	}
	else {
		enter_mipscode("jr $ra");
	}
	//enter_mipscode("jr $ra");
	//4.�л�SymbolTable_index
	SymbolTable_index = 0;

	
}


void midcode_assign(midcode instruction) {
	string reg_src1;
	string reg_des;
	int offset_src1;

	//���des �ļĴ���
	if (is_num(instruction.des) == 0) {//������ �ǿ�
		if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.des) != 0)//�Ѿ�������ȫ�ּĴ���
		{
			reg_des = function_Reference_data[SymbolTable_index].map_reg[instruction.des];//��ø�ȫ�ּĴ���

		}
		else {//������ʱ�Ĵ�������
			alloc_reg_pool(instruction.des, reg_des);
			//������Ϊ�� ��Ϊ��des
			dirty_reg_pool(reg_des);
		}
	}
	else {
		error("�Ҳ������ܵ�����");
		return;
	}

	if (is_num(instruction.src1) == 1) {//����
		enter_mipscode("li "+reg_des+" " + instruction.src1);
	}
	else {//������

		if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.src1) != 0)//�Ѿ�������ȫ�ּĴ���
		{
			reg_src1 = function_Reference_data[SymbolTable_index].map_reg[instruction.src1];//��ø�ȫ�ּĴ���
			enter_mipscode("add "+reg_des+" " + reg_src1 + " $0");//��ֵ��ֵ��reg_des
		}
		else {//������ʱ�Ĵ�������
			if (search_reg_pool(instruction.src1, reg_src1) == false) {//û�ڼĴ�������
				if (find_addr(instruction.src1, offset_src1) == true) {//�ֲ�����
					enter_mipscode("lw "+reg_des+" " + to_string(offset_src1) + "($sp)");
				}
				else {//ȫ�ֱ���
					enter_mipscode("lw " + reg_des + " " + to_string(offset_src1));
				}
			}
			else {//�ڼĴ�������
				enter_mipscode("add " + reg_des + " "  + reg_src1 + " $0");//��ֵ��ֵ��reg_des
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
	//����yֵ
	//src2
	int offset_src2;
	int offset_src1;
	int offset_des;
	string reg_1;
	string reg_2;
	string temp_reg;//��ʱ�ļĴ��� ���ڼ���ƫ��

	//src2
	if (is_num(instruction.src2) == 1) {//����
		if (alloc_reg_pool(instruction.src2, reg_2) == false) {//û�ڼĴ�������
			enter_mipscode("li " + reg_2 + " " + instruction.src2);
		}
	}
	else {//������
		if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.src2) != 0)//�Ѿ�������ȫ�ּĴ���
		{
			reg_2 = function_Reference_data[SymbolTable_index].map_reg[instruction.src2];//��ø�ȫ�ּĴ���

		}
		else {//������ʱ�Ĵ�������
			if (alloc_reg_pool(instruction.src2, reg_2) == false) {//û�ڼĴ�������
				if (find_addr(instruction.src2, offset_src2) == true) {//�ֲ�����
					enter_mipscode("lw " + reg_2 + " " + to_string(offset_src2) + "($sp)");  //lw $ti offset($sp)
				}
				else {//ȫ�ֱ���
					enter_mipscode("lw " + reg_2 + " " + to_string(offset_src2));  //lw $ti offset
				}
			}
		}
	}

	if (is_num(instruction.src1) == 1) {//����  ���ֱȽϺô���
		int offset_des;//��Ϊdes������ ����һ��û�з���ȫ�ּĴ���
		if (find_addr(instruction.des, offset_des) == true) {//�ֲ�����
			enter_mipscode("sw "+reg_2+" " + to_string(4 * str2int(instruction.src1) + offset_des) + "($sp)");
		}
		else {//ȫ�ֱ���
			enter_mipscode("sw "+reg_2+" " + to_string(4 * str2int(instruction.src1) + offset_des));
		}
	}
	else {//������ ��Ҫ���üĴ��� ����offset

		if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.src1) != 0)//�Ѿ�������ȫ�ּĴ���
		{
			reg_1 = function_Reference_data[SymbolTable_index].map_reg[instruction.src1];//��ø�ȫ�ּĴ���	
		}
		else {//������ʱ�Ĵ�������
			if (alloc_reg_pool(instruction.src1, reg_1) == false) {//û�ڼĴ�������
				if (find_addr(instruction.src1, offset_src1) == true) {//�ֲ�����
					enter_mipscode("lw " + reg_1 + " " + to_string(offset_src1) + "($sp)");  //lw $ti offset($sp)
				}
				else {//ȫ�ֱ���
					enter_mipscode("lw " + reg_1 + " " + to_string(offset_src1));  //lw $ti offset
				}
			}
		}

		alloc_temp_reg(temp_reg);//�����ʱ�Ĵ���

		
		if (find_addr(instruction.des, offset_des) == true) {//�ֲ�����
			//��$reg_1 *4
			enter_mipscode("sll "+temp_reg+" "+reg_1+" 2");
			// add temp_reg temp_reg $sp
			enter_mipscode("add "+temp_reg+" "+temp_reg+" $sp");
			//sw reg_2  offset_des(temp_reg)
			enter_mipscode("sw "+ reg_2+" " + to_string(offset_des) + "("+temp_reg+")"); //���յĵ�ַ�� offset_des+$sp+4*reg_1
		}
		else {//ȫ�ֱ���
			//��reg_1 *4
			enter_mipscode("sll " + temp_reg + " " + reg_1 + " 2");

			enter_mipscode("sw " + reg_2 + " " + to_string(offset_des) + "(" + temp_reg + ")"); //���յĵ�ַ�� offset_des+4*reg_1
		}

	}
}
void midcode_eqaddr(midcode instruction) {//" =[]"eqaddr x y z   =>z=y[x]
	int offset_src2;
	int offset_src1;
	string reg_1;
	string reg_3;
	string temp_reg;//��ʱ�ļĴ��� ���ڼ���ƫ��

	//des  ���reg_3 
	if (is_num(instruction.des) == 0) {//������ �ǿ�
		if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.des) != 0)//�Ѿ�������ȫ�ּĴ���
		{
			reg_3 = function_Reference_data[SymbolTable_index].map_reg[instruction.des];//��ø�ȫ�ּĴ���

		}
		else {//������ʱ�Ĵ�������
			alloc_reg_pool(instruction.des, reg_3);
			//������Ϊ�� ��Ϊ��des
			dirty_reg_pool(reg_3);
		}
	}
	else {
		error("�Ҳ������ܵ�����");
		return;
	}

	if (is_num(instruction.src1) == 1) {//����  ���ֱȽϺô���

		if (find_addr(instruction.src2, offset_src2) == true) {//�ֲ�����
			enter_mipscode("lw " + reg_3 + " " + to_string(4 * str2int(instruction.src1) + offset_src2) + "($sp)");
		}
		else {//ȫ�ֱ���
			enter_mipscode("lw " + reg_3 + " " + to_string(4 * str2int(instruction.src1) + offset_src2));
		}
	}
	else {//������ ��Ҫ���üĴ��� ����offset

		if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.src1) != 0)//�Ѿ�������ȫ�ּĴ���
		{
			reg_1 = function_Reference_data[SymbolTable_index].map_reg[instruction.src1];//��ø�ȫ�ּĴ���	
		}
		else {//������ʱ�Ĵ�������
			if (alloc_reg_pool(instruction.src1, reg_1) == false) {//û�ڼĴ�������
				if (find_addr(instruction.src1, offset_src1) == true) {//�ֲ�����
					enter_mipscode("lw " + reg_1 + " " + to_string(offset_src1) + "($sp)");  //lw $ti offset($sp)
				}
				else {//ȫ�ֱ���
					enter_mipscode("lw " + reg_1 + " " + to_string(offset_src1));  //lw $ti offset
				}
			}
		}

		alloc_temp_reg(temp_reg);//�����ʱ�Ĵ���


		if (find_addr(instruction.src2, offset_src2) == true) {//�ֲ�����
			//��$reg_1 *4
			enter_mipscode("sll " + temp_reg + " " + reg_1 + " 2");
			// add temp_reg temp_reg $sp
			enter_mipscode("add " + temp_reg + " " + temp_reg + " $sp");
			//sw reg_2  offset_des(temp_reg)
			enter_mipscode("lw " + reg_3 + " " + to_string(offset_src2) + "(" + temp_reg + ")"); //���յĵ�ַ�� offset_des+$sp+4*reg_1
		}
		else {//ȫ�ֱ���
			//��reg_1 *4
			enter_mipscode("sll " + temp_reg + " " + reg_1 + " 2");

			enter_mipscode("lw " + reg_3 + " " + to_string(offset_src2) + "(" + temp_reg + ")"); //���յĵ�ַ�� offset_des+4*reg_1
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
		if (is_num(instruction.des) == 0) {//������ �ǿ�

			if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.des) != 0)//�Ѿ�������ȫ�ּĴ���
			{
				reg = function_Reference_data[SymbolTable_index].map_reg[instruction.des];//��ø�ȫ�ּĴ���
				enter_mipscode("add $a0 " + reg + " $0");//��ֵ��ֵ��$a0
			}
			else {//������ʱ�Ĵ���
				if (search_reg_pool(instruction.des, reg) == false) {//û����ʱ�Ĵ�����
					if (find_addr(instruction.des, offset_des) == true) {//�ֲ�����
						enter_mipscode("lw $a0 " + to_string(offset_des) + "($sp)");
					}
					else {//ȫ�ֱ���
						enter_mipscode("lw $a0 " + to_string(offset_des));
					}
				}
				else {//�ڼĴ�������
					alloc_reg_pool(instruction.des, reg);
					enter_mipscode("add $a0 " + reg + " $0");//��ֵ��ֵ��$a0
				}
			}
		}
		else {//����
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
		//���des �ļĴ���
		if (is_num(instruction.des) == 0) {//������ �ǿ�
			if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.des) != 0)//�Ѿ�������ȫ�ּĴ���
			{
				reg_des = function_Reference_data[SymbolTable_index].map_reg[instruction.des];//��ø�ȫ�ּĴ���

			}
			else {//������ʱ�Ĵ�������
				alloc_reg_pool(instruction.des, reg_des);
				//������Ϊ�� ��Ϊ��des
				dirty_reg_pool(reg_des);
			}
		}
		else {
			error("�Ҳ������ܵ�����");
			return;
		}
		enter_mipscode("add " + reg_des + " $v0 $0");
	}
	else {
		error("�о�������û����");
		return;
	}
}

void midcode_lable(midcode instruction) {
	empty_reg_pool();
	enter_mipscode(instruction.des+":"); //����lable
}

void midcode_GOTO(midcode instruction) {
	empty_reg_pool();
	enter_mipscode("j " + instruction.des);

}

void midcode_BNZ(midcode instruction) {//����������ת $t9 ��Ϊ��ת�Ĵ���   ����������ֵΪ1 ������������ֵΪ0
	//enter_mipscode("beq $t9 1 "+instruction.des);
}//�ú����Ѿ�û�����ü�ֵ��

void midcode_BZ(midcode instruction) {//������������ת $t9 ��Ϊ��ת�Ĵ���   ����������ֵΪ1 ������������ֵΪ0
	//enter_mipscode("beq $t9 0 " + instruction.des);
}//�ú����Ѿ�û�����ü�ֵ��

void midcode_EQ(midcode instruction) {//��Ȳ���ת  �� ������ת(bne) �ȼ�  ����src1 ��src2
	int offset_src1;
	int offset_src2;

	string reg_1;
	string reg_2;
	//src1
	if (is_num(instruction.src1) == 1) {//����
		if (alloc_reg_pool(instruction.src1, reg_1) == false) {//û�ڼĴ�������
			enter_mipscode("li " + reg_1 + " " + instruction.src1);
		}
	}
	else {//������
		if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.src1) != 0)//�Ѿ�������ȫ�ּĴ���
		{
			reg_1 = function_Reference_data[SymbolTable_index].map_reg[instruction.src1];//��ø�ȫ�ּĴ���	
		}
		else {//������ʱ�Ĵ�������
			if (alloc_reg_pool(instruction.src1, reg_1) == false) {//û�ڼĴ�������
				if (find_addr(instruction.src1, offset_src1) == true) {//�ֲ�����
					enter_mipscode("lw " + reg_1 + " " + to_string(offset_src1) + "($sp)");  //lw $ti offset($sp)
				}
				else {//ȫ�ֱ���
					enter_mipscode("lw " + reg_1 + " " + to_string(offset_src1));  //lw $ti offset
				}
			}
		}

	}
	//src2
	if (is_num(instruction.src2) == 1) {//����
		if (alloc_reg_pool(instruction.src2, reg_2) == false) {//û�ڼĴ�������
			enter_mipscode("li " + reg_2 + " " + instruction.src2);
		}
	}
	else {//������
		if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.src2) != 0)//�Ѿ�������ȫ�ּĴ���
		{
			reg_2 = function_Reference_data[SymbolTable_index].map_reg[instruction.src2];//��ø�ȫ�ּĴ���

		}
		else {//������ʱ�Ĵ�������
			if (alloc_reg_pool(instruction.src2, reg_2) == false) {//û�ڼĴ�������
				if (find_addr(instruction.src2, offset_src2) == true) {//�ֲ�����
					enter_mipscode("lw " + reg_2 + " " + to_string(offset_src2) + "($sp)");  //lw $ti offset($sp)
				}
				else {//ȫ�ֱ���
					enter_mipscode("lw " + reg_2 + " " + to_string(offset_src2));  //lw $ti offset
				}
			}
		}
	}
	empty_reg_pool();
	enter_mipscode("bne "+reg_1+" "+reg_2+" "+instruction.des);

}
void midcode_NOTEQ(midcode instruction) {//����Ȳ���ת  �� ������ת(beq) �ȼ�  ����src1 ��src2
	int offset_src1;
	int offset_src2;

	string reg_1;
	string reg_2;
	//src1
	if (is_num(instruction.src1) == 1) {//����
		if (alloc_reg_pool(instruction.src1, reg_1) == false) {//û�ڼĴ�������
			enter_mipscode("li " + reg_1 + " " + instruction.src1);
		}
	}
	else {//������
		if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.src1) != 0)//�Ѿ�������ȫ�ּĴ���
		{
			reg_1 = function_Reference_data[SymbolTable_index].map_reg[instruction.src1];//��ø�ȫ�ּĴ���	
		}
		else {//������ʱ�Ĵ�������
			if (alloc_reg_pool(instruction.src1, reg_1) == false) {//û�ڼĴ�������
				if (find_addr(instruction.src1, offset_src1) == true) {//�ֲ�����
					enter_mipscode("lw " + reg_1 + " " + to_string(offset_src1) + "($sp)");  //lw $ti offset($sp)
				}
				else {//ȫ�ֱ���
					enter_mipscode("lw " + reg_1 + " " + to_string(offset_src1));  //lw $ti offset
				}
			}
		}

	}
	//src2
	if (is_num(instruction.src2) == 1) {//����
		if (alloc_reg_pool(instruction.src2, reg_2) == false) {//û�ڼĴ�������
			enter_mipscode("li " + reg_2 + " " + instruction.src2);
		}
	}
	else {//������
		if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.src2) != 0)//�Ѿ�������ȫ�ּĴ���
		{
			reg_2 = function_Reference_data[SymbolTable_index].map_reg[instruction.src2];//��ø�ȫ�ּĴ���

		}
		else {//������ʱ�Ĵ�������
			if (alloc_reg_pool(instruction.src2, reg_2) == false) {//û�ڼĴ�������
				if (find_addr(instruction.src2, offset_src2) == true) {//�ֲ�����
					enter_mipscode("lw " + reg_2 + " " + to_string(offset_src2) + "($sp)");  //lw $ti offset($sp)
				}
				else {//ȫ�ֱ���
					enter_mipscode("lw " + reg_2 + " " + to_string(offset_src2));  //lw $ti offset
				}
			}
		}
	}
	empty_reg_pool();
	enter_mipscode("beq "+reg_1+" "+reg_2+" " + instruction.des);

}
void midcode_LESSEQ(midcode instruction) {//<= src1 <= src2 ����ת  src1>src2��ת
	int offset_src1;
	int offset_src2;

	string reg_1;
	string reg_2;
	string reg_temp;
	//src1
	if (is_num(instruction.src1) == 1) {//����
		if (alloc_reg_pool(instruction.src1, reg_1) == false) {//û�ڼĴ�������
			enter_mipscode("li " + reg_1 + " " + instruction.src1);
		}
	}
	else {//������
		if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.src1) != 0)//�Ѿ�������ȫ�ּĴ���
		{
			reg_1 = function_Reference_data[SymbolTable_index].map_reg[instruction.src1];//��ø�ȫ�ּĴ���	
		}
		else {//������ʱ�Ĵ�������
			if (alloc_reg_pool(instruction.src1, reg_1) == false) {//û�ڼĴ�������
				if (find_addr(instruction.src1, offset_src1) == true) {//�ֲ�����
					enter_mipscode("lw " + reg_1 + " " + to_string(offset_src1) + "($sp)");  //lw $ti offset($sp)
				}
				else {//ȫ�ֱ���
					enter_mipscode("lw " + reg_1 + " " + to_string(offset_src1));  //lw $ti offset
				}
			}
		}

	}
	//src2
	if (is_num(instruction.src2) == 1) {//����
		if (alloc_reg_pool(instruction.src2, reg_2) == false) {//û�ڼĴ�������
			enter_mipscode("li " + reg_2 + " " + instruction.src2);
		}
	}
	else {//������
		if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.src2) != 0)//�Ѿ�������ȫ�ּĴ���
		{
			reg_2 = function_Reference_data[SymbolTable_index].map_reg[instruction.src2];//��ø�ȫ�ּĴ���

		}
		else {//������ʱ�Ĵ�������
			if (alloc_reg_pool(instruction.src2, reg_2) == false) {//û�ڼĴ�������
				if (find_addr(instruction.src2, offset_src2) == true) {//�ֲ�����
					enter_mipscode("lw " + reg_2 + " " + to_string(offset_src2) + "($sp)");  //lw $ti offset($sp)
				}
				else {//ȫ�ֱ���
					enter_mipscode("lw " + reg_2 + " " + to_string(offset_src2));  //lw $ti offset
				}
			}
		}
	}
	alloc_temp_reg(reg_temp);
	enter_mipscode("sub "+reg_temp+" "+reg_1+" "+reg_2);//src1-src2>0 ��ת
	empty_reg_pool();
	enter_mipscode("bgtz "+reg_temp +" "+ instruction.des);

}
void midcode_LESS(midcode instruction) {//<= src1 < src2 ����ת  src1>=src2��ת
	int offset_src1;
	int offset_src2;

	string reg_1;
	string reg_2;
	string reg_temp;
	//src1
	if (is_num(instruction.src1) == 1) {//����
		if (alloc_reg_pool(instruction.src1, reg_1) == false) {//û�ڼĴ�������
			enter_mipscode("li " + reg_1 + " " + instruction.src1);
		}
	}
	else {//������
		if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.src1) != 0)//�Ѿ�������ȫ�ּĴ���
		{
			reg_1 = function_Reference_data[SymbolTable_index].map_reg[instruction.src1];//��ø�ȫ�ּĴ���	
		}
		else {//������ʱ�Ĵ�������
			if (alloc_reg_pool(instruction.src1, reg_1) == false) {//û�ڼĴ�������
				if (find_addr(instruction.src1, offset_src1) == true) {//�ֲ�����
					enter_mipscode("lw " + reg_1 + " " + to_string(offset_src1) + "($sp)");  //lw $ti offset($sp)
				}
				else {//ȫ�ֱ���
					enter_mipscode("lw " + reg_1 + " " + to_string(offset_src1));  //lw $ti offset
				}
			}
		}

	}
	//src2
	if (is_num(instruction.src2) == 1) {//����
		if (alloc_reg_pool(instruction.src2, reg_2) == false) {//û�ڼĴ�������
			enter_mipscode("li " + reg_2 + " " + instruction.src2);
		}
	}
	else {//������
		if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.src2) != 0)//�Ѿ�������ȫ�ּĴ���
		{
			reg_2 = function_Reference_data[SymbolTable_index].map_reg[instruction.src2];//��ø�ȫ�ּĴ���

		}
		else {//������ʱ�Ĵ�������
			if (alloc_reg_pool(instruction.src2, reg_2) == false) {//û�ڼĴ�������
				if (find_addr(instruction.src2, offset_src2) == true) {//�ֲ�����
					enter_mipscode("lw " + reg_2 + " " + to_string(offset_src2) + "($sp)");  //lw $ti offset($sp)
				}
				else {//ȫ�ֱ���
					enter_mipscode("lw " + reg_2 + " " + to_string(offset_src2));  //lw $ti offset
				}
			}
		}
	}
	alloc_temp_reg(reg_temp);
	enter_mipscode("sub " + reg_temp + " " + reg_1 + " " + reg_2);//src1-src2>=0 ��ת
	empty_reg_pool();
	enter_mipscode("bgez " + reg_temp + " " + instruction.des);

	//enter_mipscode("sub $t0 $t0 $t1"); //src1-src2>=0 ��ת
	//enter_mipscode("bgez $t0 " + instruction.des);
}
void midcode_MOREEQ(midcode instruction) {//<= src1 >= src2 ����ת  src1<src2��ת
	int offset_src1;
	int offset_src2;

	string reg_1;
	string reg_2;
	string reg_temp;
	//src1
	if (is_num(instruction.src1) == 1) {//����
		if (alloc_reg_pool(instruction.src1, reg_1) == false) {//û�ڼĴ�������
			enter_mipscode("li " + reg_1 + " " + instruction.src1);
		}
	}
	else {//������
		if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.src1) != 0)//�Ѿ�������ȫ�ּĴ���
		{
			reg_1 = function_Reference_data[SymbolTable_index].map_reg[instruction.src1];//��ø�ȫ�ּĴ���	
		}
		else {//������ʱ�Ĵ�������
			if (alloc_reg_pool(instruction.src1, reg_1) == false) {//û�ڼĴ�������
				if (find_addr(instruction.src1, offset_src1) == true) {//�ֲ�����
					enter_mipscode("lw " + reg_1 + " " + to_string(offset_src1) + "($sp)");  //lw $ti offset($sp)
				}
				else {//ȫ�ֱ���
					enter_mipscode("lw " + reg_1 + " " + to_string(offset_src1));  //lw $ti offset
				}
			}
		}

	}
	//src2
	if (is_num(instruction.src2) == 1) {//����
		if (alloc_reg_pool(instruction.src2, reg_2) == false) {//û�ڼĴ�������
			enter_mipscode("li " + reg_2 + " " + instruction.src2);
		}
	}
	else {//������
		if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.src2) != 0)//�Ѿ�������ȫ�ּĴ���
		{
			reg_2 = function_Reference_data[SymbolTable_index].map_reg[instruction.src2];//��ø�ȫ�ּĴ���

		}
		else {//������ʱ�Ĵ�������
			if (alloc_reg_pool(instruction.src2, reg_2) == false) {//û�ڼĴ�������
				if (find_addr(instruction.src2, offset_src2) == true) {//�ֲ�����
					enter_mipscode("lw " + reg_2 + " " + to_string(offset_src2) + "($sp)");  //lw $ti offset($sp)
				}
				else {//ȫ�ֱ���
					enter_mipscode("lw " + reg_2 + " " + to_string(offset_src2));  //lw $ti offset
				}
			}
		}
	}
	alloc_temp_reg(reg_temp);
	enter_mipscode("sub " + reg_temp + " " + reg_1 + " " + reg_2);//src1-src2<0 ��ת
	empty_reg_pool();
	enter_mipscode("bltz " + reg_temp + " " + instruction.des);
	
	//enter_mipscode("sub $t0 $t0 $t1"); //src1-src2<0 ��ת
	//enter_mipscode("bltz $t0 " + instruction.des);
}
void midcode_MORE(midcode instruction) {//<= src1 > src2 ����ת  src1<=src2��ת
	int offset_src1;
	int offset_src2;

	string reg_1;
	string reg_2;
	string reg_temp;
	//src1
	if (is_num(instruction.src1) == 1) {//����
		if (alloc_reg_pool(instruction.src1, reg_1) == false) {//û�ڼĴ�������
			enter_mipscode("li " + reg_1 + " " + instruction.src1);
		}
	}
	else {//������
		if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.src1) != 0)//�Ѿ�������ȫ�ּĴ���
		{
			reg_1 = function_Reference_data[SymbolTable_index].map_reg[instruction.src1];//��ø�ȫ�ּĴ���	
		}
		else {//������ʱ�Ĵ�������
			if (alloc_reg_pool(instruction.src1, reg_1) == false) {//û�ڼĴ�������
				if (find_addr(instruction.src1, offset_src1) == true) {//�ֲ�����
					enter_mipscode("lw " + reg_1 + " " + to_string(offset_src1) + "($sp)");  //lw $ti offset($sp)
				}
				else {//ȫ�ֱ���
					enter_mipscode("lw " + reg_1 + " " + to_string(offset_src1));  //lw $ti offset
				}
			}
		}

	}
	//src2
	if (is_num(instruction.src2) == 1) {//����
		if (alloc_reg_pool(instruction.src2, reg_2) == false) {//û�ڼĴ�������
			enter_mipscode("li " + reg_2 + " " + instruction.src2);
		}
	}
	else {//������
		if (function_Reference_data[SymbolTable_index].map_reg.count(instruction.src2) != 0)//�Ѿ�������ȫ�ּĴ���
		{
			reg_2 = function_Reference_data[SymbolTable_index].map_reg[instruction.src2];//��ø�ȫ�ּĴ���

		}
		else {//������ʱ�Ĵ�������
			if (alloc_reg_pool(instruction.src2, reg_2) == false) {//û�ڼĴ�������
				if (find_addr(instruction.src2, offset_src2) == true) {//�ֲ�����
					enter_mipscode("lw " + reg_2 + " " + to_string(offset_src2) + "($sp)");  //lw $ti offset($sp)
				}
				else {//ȫ�ֱ���
					enter_mipscode("lw " + reg_2 + " " + to_string(offset_src2));  //lw $ti offset
				}
			}
		}
	}
	alloc_temp_reg(reg_temp);
	enter_mipscode("sub " + reg_temp + " " + reg_1 + " " + reg_2);//src1-src2<=0 ��ת
	empty_reg_pool();
	enter_mipscode("blez " + reg_temp + " " + instruction.des);

	//enter_mipscode("sub $t0 $t0 $t1"); //src1-src2<=0 ��ת
	//enter_mipscode("blez $t0 " + instruction.des);
}

/////////////////////////////////////////////////////////////
///////////              ������                //////////////
////////////////////////////////////////////////////////////
void midcode2mips() {
	initail_reg_pool();//��ʼ���Ĵ�����
	SymbolTable_index = 0;
	for (int i = 0; i < midcode_index; i++) {
		if (midcodes[i].op == op_none) {//����Ϊ��
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
			error("�м�����bug");

			break;
		}
	}
}