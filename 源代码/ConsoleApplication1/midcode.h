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
};//op_BNZ ��op_BZ�Ѿ�û�����ü�ֵ��  op_none�����

//ע��op_RET �Ǻ�������ʱ�ķ���ֵ   op_ret �����ڲ��ķ���
//op_addreq  []=, op_eqaddr  =[]
struct midcode{
	operater op; //������
	string src1; //������1
	string src2;	//������2
	string des; //���
};

bool find_addr(string name, int &offset);//���ұ�ʶ����ƫ��
/////////////////////////////////
/////   �Ĵ��������   //////////
/////////////////////////////////
static int now_order = 1;//��ǰorder
struct register_pool {
	string ID; //��ʶ�� ���� �� ����
	bool dirty_mark;//���ֵ�Ƿ�ı�
 int order;//ʹ��˳��  ��һ�ּ򵥵ķ�ʽ����opt���� ��ʼΪ0
};//ʮ����ʱ�Ĵ���





extern midcode midcodes[];//�м�ָ���������

extern int midcode_index;//�м�������� ȫ�ֱ���

extern int mipscode_index;

extern string mipscodes[];

int  is_num(string name);
int str2int(const string &string_temp);
void get_tn(string &Tn);
void get_lable(string &Tn);
void get_string(string &key, string value);
int enter_midcode(operater op,string src1,string src2,string des);//����м���
int enter_mipscode(string instruction);//���Ŀ����
void out_midcode();//����м���
void out_mips();//���Ŀ����
void register_temp(string name);//�ڷ��ű���ע����ʱ����
void register_string(string key,string value);//���ַ��������� ע���ַ�������
void handle_table();
void mips_init();
//////������
void midcode2mips();

//ÿһ��ת�����
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





