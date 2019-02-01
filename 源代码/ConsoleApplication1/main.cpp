#include "pch.h"
#include "Lexer.h"
#include "Parser.h"
#include "error.h"
#include "midcode.h"
#include "opt.h"
#include <iostream>
#include <cstdio>
#include <sstream>
#include <fstream>//文件操作
using namespace std;
int main()
{
	/*char ch;
	index = 0;
	while ((ch = getchar()) != EOF) {//从控制台读入所有字符
		S[index++] = ch;
	/}*/
	int out_index = 1;//输出时的下标
	string inputFilePath;
	cout << "input file path:" << endl;
	cin >> inputFilePath;
	//inputFilePath = "16231197_test.txt";
	
	ifstream fin(inputFilePath);
	if (!fin.is_open()) {
		cout << "File NoT Exist!" << endl;
		return 0;
	}
	//将文件所有类容读入到数组中
	S = string((std::istreambuf_iterator<char>(fin)), std::istreambuf_iterator<char>());
	fin.close();



	//S[index] = '\0';
	//cout << "读入ok" << endl;
	index = 0;
	nextchar();
	
	getsym();
	program();//递归下降 语法语义处理

	if (error_num == 0) {
		//cout << "词法分析 语法语义分析阶段 无错误" << endl;
		//delete_Tn();//删除多余的Tn

		out_midcode();//输出中间码
		//Reference_count();//引用计数 分配全局寄存器  顺便做了函数没有return报错
		mips_init();
		midcode2mips();

		if (error_num == 0) {
			cout << "生成目标码成功" << endl;
		}

		leaf_function();//在目标码上删除叶函数的$ra
		out_mips();
	}
	else {
		cout << "词法分析 语法语义分析阶段 存在错误" << endl;
		cout << "停止运行" << endl;
	}
	

	return 0;
}
