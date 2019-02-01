// ConsoleApplication1.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "Lexer.h"
#include "error.h"
#include "Parser.h"
#include <iostream>
#include <cstdio>
#include <sstream>
#include <fstream>//文件操作

using namespace std;
const char* RESERVE_SY[13] = {
"const","int","char","void","main","if","while","switch","case",
"default","scanf","printf","return"
};
/*const char* RESERVE_SY[13] = {
"const","int","char","void","main","if","while","switch","case",
"default","scanf","printf","return"
};*/

type_enum new_type;//记录最新 读到的type   只用表示  char int void 用于符号表填写 在reserver中实现
string new_ID;//记录最新读到的 标识符 在reserver中实现
 

char now_char;//字符型全局变量，存放当前读进的字符
string Token;//全局变量，字符数组，存放单词的字符串
string Token2;//用于超前读
string Token3;//用于超前读
int num;//整型全局变量，存放当前读入的整型数值
enum Sym symbol;//枚举型全局变量，保存当前所识别单词的类型
enum Sym symbol2;//用于超前读
enum Sym symbol3;//用于超前读

//char S[LEN_S];//源程序
string S;
int index;//下标
int line=1;//第几行
//获得下一个字符
void nextchar(void) {
	while (S[index] < 0 || S[index]>255) {
		cout <<"line:" <<line <<" 无法识别该字符" << endl;
		index++;
	}
	now_char = S[index++];

}
//将读字符指针后退一个字符
void retract(void) {
	index--;
}
//查找保留字，返回0则为标识符，否则为保留字编码
int reserver() {
	//记录最新 读到的type 
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
			return (i + 1);//0是ID
		}
	}

	new_ID = Token; //记录最新读到的ID
	return 0;
}
//清空token字符数组
void clearToken() {
	Token.clear();
	return;
}
//将token中的字符串转换成整型数值
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
		nextchar();//读取字符，跳过空格，换行和tab
	}
	//标识符开头 字面或者下划线
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
			symbol = (Sym)resultValue;//保证与枚举类型位置相同
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

	int out_index=1;//输出时的下标
	string inputFilePath;
	cout << "input file path:" << endl;
	cin >> inputFilePath;
	ifstream fin(inputFilePath);
	if (!fin.is_open()) {
		cout << "File NoT Exist!" << endl;
		return 0;
	}
	//将文件所有类容读入到数组中
	S=string((std::istreambuf_iterator<char>(fin)),std::istreambuf_iterator<char>());
	fin.close();



	//S[index] = '\0';
	//cout << "读入ok" << endl;
	index = 0;
	nextchar();
	while (now_char != '\0') {

		getsym();
		if (symbol == ID) {
			cout << out_index++<<" "<< "ID" << " " << Token << endl;

		}
		else if(symbol>= CONST_SY && symbol <= RETURN_SY){
			cout << out_index++ << " " << RESERVE_SY[symbol - 1] << " " << Token << endl;//注意错了一位
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
	cout << "正常结束" << endl;
	return 0;
}
*/
// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门提示: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
