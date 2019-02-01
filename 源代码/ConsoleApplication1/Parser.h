#pragma once
#include <iostream>
#include <map>
#define Max_para 100//���������� ����ջ�Ĵ�С
#define Max_table 100 //���������
using namespace std;
enum obj_enum {//�����ʶ�� ���� ���� ���� ����
	obj_const,obj_var,obj_function,obj_para
};
enum type_enum {//�����ʶ�� int char int[] char [] void
	type_int,type_int_group,type_char,type_char_group,type_void
};
struct table_item {
	string name;
	obj_enum obj;//�����ʶ�� ���� ���� ���� ����
	type_enum type;//�����ʶ�� int char int[] char[] void
	int adr;//���ں�������Ϊ������ĵ�ַ�����ڳ�����������ֵ
	int length;//�������飬��Ϊ���鳤��
	int offset;//��ʶ����ƫ��λ�ã���������������������м�����ע����ű�ʱ��д��
};
struct Table {//�����ú�������Ϣ 
	//bool Globle_mark;//true Ϊȫ�ֱ�false Ϊ������
	string function_name;//��������
	int paranum;//������������
	
	table_item paralist[Max_para];//�������λ��  ע�������� ʱ ��Ҫ���� para ҲҪ����maptable;
	map<string, table_item>  maptable;//��Ҫ����
	int vsize;//�ֲ����� ������������Ϣ����Sջ����ռ�Ĵ洢��Ԫ����
};
extern Table symbolTable[500];
extern int which_Table;// symbolTable���±� 0��ʾ���ȫ�ֱ� �����İ�˳����
//��Ҫ usedindexof_table ����¼�Ѿ�ʹ���˵�Table

extern map<string, string> stringTable; //�����ַ���

void enter(string name, obj_enum obj, type_enum type, int adr, int length);
void enterfunction(string function_name);
void enterparalist(string name, obj_enum obj, type_enum type, int adr, int length);//�����
void program();
void constDec();
void constDef();
void varDec(bool program_mark);//program_mark ��ʾ program ֱ���Ƶ�����varDec()
void varDef(bool program_mark);//program_mark ��ʾ program ֱ���Ƶ�����varDec()�� ��һ����������
void retfunctionDef();
void voidfunctionDef();

void mainFunction();
void compoundStatement();//�������
void paraList();
void Statements();//�����
void Statement();

bool expression(string &tn);//true ����������Ϊint falseΪchar
bool expression(string &tn, int &calmark, int &value);//true ����������Ϊint falseΪchar Ϊ�˾�̬����Խ����
bool term(string &tn,  int &calmark, int &value);//true ����������Ϊint falseΪchar
bool factor(string &tn,  int &calmark, int &value);//true ����������Ϊint falseΪchar

void ifStatement();
void whileStatement();
void callFunction();
void assignStatement();
void readStatement();
void writeStatement();
void returnStatement();

void switchStatement();
void situationTable(string tn, string switch_end_lable,bool case_type);


