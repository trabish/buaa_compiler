// ConsoleApplication1.cpp : ���ļ����� "main" ����������ִ�н��ڴ˴���ʼ��������
//

#include "pch.h"
#include "Lexer.h"
#include "error.h"
#include "Parser.h"
#include <iostream>
#include <cstdio>
#include <sstream>
#include <fstream>//�ļ�����

using namespace std;
const char* RESERVE_SY[13] = {
"const","int","char","void","main","if","while","switch","case",
"default","scanf","printf","return"
};
/*const char* RESERVE_SY[13] = {
"const","int","char","void","main","if","while","switch","case",
"default","scanf","printf","return"
};*/

type_enum new_type;//��¼���� ������type   ֻ�ñ�ʾ  char int void ���ڷ��ű���д ��reserver��ʵ��
string new_ID;//��¼���¶����� ��ʶ�� ��reserver��ʵ��
 

char now_char;//�ַ���ȫ�ֱ�������ŵ�ǰ�������ַ�
string Token;//ȫ�ֱ������ַ����飬��ŵ��ʵ��ַ���
string Token2;//���ڳ�ǰ��
string Token3;//���ڳ�ǰ��
int num;//����ȫ�ֱ�������ŵ�ǰ�����������ֵ
enum Sym symbol;//ö����ȫ�ֱ��������浱ǰ��ʶ�𵥴ʵ�����
enum Sym symbol2;//���ڳ�ǰ��
enum Sym symbol3;//���ڳ�ǰ��

//char S[LEN_S];//Դ����
string S;
int index;//�±�
int line=1;//�ڼ���
//�����һ���ַ�
void nextchar(void) {
	while (S[index] < 0 || S[index]>255) {
		cout <<"line:" <<line <<" �޷�ʶ����ַ�" << endl;
		index++;
	}
	now_char = S[index++];

}
//�����ַ�ָ�����һ���ַ�
void retract(void) {
	index--;
}
//���ұ����֣�����0��Ϊ��ʶ��������Ϊ�����ֱ���
int reserver() {
	//��¼���� ������type 
	if (Token == "char") {
		new_type = type_char;
	}
	else if (Token == "int") {
		new_type = type_int;
	}
	else if (Token=="void") {
		new_type = type_void;
	}

	for (int i = 0; i < 13; i++) {
		if (Token == RESERVE_SY[i]) {
			return (i + 1);//0��ID
		}
	}

	new_ID = Token; //��¼���¶�����ID
	return 0;
}
//���token�ַ�����
void clearToken() {
	Token.clear();
	return;
}
//��token�е��ַ���ת����������ֵ
void transNum() {
	stringstream ss;
	if (Token.length() > 9) {
		error(number_too_big, line);
	}
	ss << Token;
	ss >> num;
}

void getsym() {
	clearToken();
	
	while (isspace(now_char) || now_char == '\n' || now_char == '\t') {
		if (now_char == '\n') {
			line++;
		}
		nextchar();//��ȡ�ַ��������ո񣬻��к�tab
	}
	//��ʶ����ͷ ��������»���
	if (isalpha(now_char) || now_char == '_') {
		while (isalnum(now_char) || now_char == '_') {
			Token = Token + now_char;
			nextchar();
		}
		retract();
		int resultValue = reserver();
		if (resultValue == 0) {

			symbol = ID;
		}
		else {
			symbol = (Sym)resultValue;//��֤��ö������λ����ͬ
		}
	}
	else if (isdigit(now_char)) {
		//if (now_char == '0')
		//	error(zero_head,line);
		while (isdigit(now_char)) {
			Token = Token + now_char;
			nextchar();
		}
		retract();
		transNum();
		symbol = UINT;
	}
	else if (now_char == '+') {
		symbol = ADD;
	}
	else if (now_char == '-') {
		symbol = SUB;
	}
	else if (now_char == '*') {
		symbol = MULT;
	}
	else if (now_char == '/') {
		symbol = DIV;
	}
	else if (now_char == '<') {
		nextchar();
		if (now_char == '=') {
			symbol = LESSEQ;
		}
		else {
			retract();
			symbol = LESS;
		}
	}
	else if (now_char == '>') {
		nextchar();
		if (now_char == '=') {
			symbol = MOREEQ;
		}
		else {
			retract();
			symbol = MORE;
		}
	}
	else if (now_char == '!') {
		nextchar();
		if (now_char == '=') {
			symbol = NOTEQ;
		}
		else {
			error(noteq_loss,line);
		}
	}
	else if (now_char == '=') {
		nextchar();
		if (now_char == '=') {
			symbol = EQ;
		}
		else {
			retract();
			symbol = ASSIGN;
		}
	}
	else if (now_char == ',') {
		symbol = COMMA;
	}
	else if (now_char == ';') {
		symbol = SEMI;
	}
	else if (now_char == ':') {
		symbol = COLON;
	}
	else if (now_char == '(') {
		symbol = LS_BACKET;
	}
	else if (now_char == ')') {
		symbol = RS_BACKET;
	}
	else if (now_char == '[') {
		symbol = LM_BACKET;
	}
	else if (now_char == ']') {
		symbol = RM_BACKET;
	}
	else if (now_char == '{') {
		symbol = LL_BACKET;
	}
	else if (now_char == '}') {
		symbol = RL_BACKET;
	}
	else if (now_char == '\'') {
		nextchar();
		if (now_char == '+' || now_char == '-' || now_char == '*' || now_char == '/' || now_char == '_' || isalnum(now_char)) {
			Token = Token + now_char;
			nextchar();
			if (now_char == '\'') {
				symbol = CHAR;
			}
			else {
				error(mult_char,line);
			}
		}
		else {
			if (now_char == '\'') {
				error(empty_char, line);
			}
			error(error_char,line);
		}
	}
	else if (now_char == '"') {
		nextchar();
		while (now_char == 32 || now_char == 33 || (now_char >= 35 && now_char <= 126)) {
			Token = Token + now_char;
			nextchar();
		}
		if (now_char == '"') {
			symbol = STRING;
		}
		else {
			error(error_string,line);
		}
	}
	else if (now_char == '\0') {
		symbol = FILEEND;
		retract();
	}
	else {
		error(unknow_char,line);
	}
	nextchar();
}
/*
int main()
{

	int out_index=1;//���ʱ���±�
	string inputFilePath;
	cout << "input file path:" << endl;
	cin >> inputFilePath;
	ifstream fin(inputFilePath);
	if (!fin.is_open()) {
		cout << "File NoT Exist!" << endl;
		return 0;
	}
	//���ļ��������ݶ��뵽������
	S=string((std::istreambuf_iterator<char>(fin)),std::istreambuf_iterator<char>());
	fin.close();



	//S[index] = '\0';
	//cout << "����ok" << endl;
	index = 0;
	nextchar();
	while (now_char != '\0') {

		getsym();
		if (symbol == ID) {
			cout << out_index++<<" "<< "ID" << " " << Token << endl;

		}
		else if(symbol>= CONST_SY && symbol <= RETURN_SY){
			cout << out_index++ << " " << RESERVE_SY[symbol - 1] << " " << Token << endl;//ע�����һλ
		}
		else if (symbol==ADD){
			cout << out_index++ << " " <<"ADD"<<" "<< "+"<< endl;
		}
		else if (symbol == SUB) {
			cout << out_index++ << " " <<"SUB"<<" "<< "-" <<  endl;
		}
		else if (symbol == MULT) {
			cout << out_index++ << " " <<"MULT"<<" "<< "*"<< endl;
		}
		else if (symbol == DIV) {
			cout << out_index++ << " " <<"DIV"<<" "<< "/"<< endl;
		}
		else if (symbol == LESS) {
			cout << out_index++ << " " <<"LESS" <<" "<< "<" << endl;
		}
		else if (symbol == LESSEQ) {
			cout << out_index++ << " " << "LESSEQ" << " " << "<=" << endl;
		}
		else if (symbol == MORE) {
			cout << out_index++ << " " << "MORE" << " " << ">" << endl;
		}
		else if (symbol == MOREEQ) {
			cout << out_index++ << " " << "MOREEQ" << " " << ">=" << endl;
		}
		else if (symbol == NOTEQ) {
			cout << out_index++ << " " << "NOTEQ" << " " << "!=" << endl;
		}
		else if (symbol == EQ) {
			cout << out_index++ << " " << "EQ" << " " << "==" << endl;
		}
		else if (symbol == ASSIGN) {
			cout << out_index++ << " " << "ASSIGN" << " " << "=" << endl;
		}
		else if (symbol == UINT) {
			cout << out_index++ << " " << "UINT" << " " << num << endl;
		}
		else if (symbol == CHAR) {
			cout << out_index++ << " " << "CHAR" << " " << Token << endl;
		}
		else if (symbol == STRING) {
			cout << out_index++ << " " << "STRING" << " " << Token << endl;
		}
		else if (symbol == COMMA) {
			cout << out_index++ << " " << "COMMA" << " " << "," << endl;
		}
		else if (symbol == SEMI) {
			cout << out_index++ << " " << "SEMI" << " " << ";" << endl;
		}
		else if (symbol == COLON) {
			cout << out_index++ << " " << "COLON" << " " << ":" << endl;
		}
		else if (symbol == LS_BACKET) {
			cout << out_index++ << " " << "LS_BACKET" << " " << "(" << endl;
		}
		else if (symbol == RS_BACKET) {
			cout << out_index++ << " " << "RS_BACKET" << " " << ")" << endl;
		}
		else if (symbol == LM_BACKET) {
			cout << out_index++ << " " << "LM_BACKET" << " " << "[" << endl;
		}
		else if (symbol == RM_BACKET) {
			cout << out_index++ << " " << "RM_BACKET" << " " << "]" << endl;
		}
		else if (symbol == LL_BACKET) {
			cout << out_index++ << " " << "LL_BACKET" << " " << "{" << endl;
		}
		else if (symbol == RL_BACKET) {
			cout << out_index++ << " " << "RL_BACKET" << " " << "}" << endl;
		}
		else if (symbol==FILEEND) {
			break;
		}
		else {
			error();
		}
		//nextchar();

	}
	cout << "��������" << endl;
	return 0;
}
*/
// ���г���: Ctrl + F5 ����� >����ʼִ��(������)���˵�
// ���Գ���: F5 ����� >����ʼ���ԡ��˵�

// ������ʾ: 
//   1. ʹ�ý��������Դ�������������/�����ļ�
//   2. ʹ���Ŷ���Դ�������������ӵ�Դ�������
//   3. ʹ��������ڲ鿴���������������Ϣ
//   4. ʹ�ô����б��ڲ鿴����
//   5. ת������Ŀ��>���������Դ����µĴ����ļ�����ת������Ŀ��>�����������Խ����д����ļ���ӵ���Ŀ
//   6. ��������Ҫ�ٴδ򿪴���Ŀ����ת�����ļ���>���򿪡�>����Ŀ����ѡ�� .sln �ļ�
