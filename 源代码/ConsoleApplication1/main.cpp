#include "pch.h"
#include "Lexer.h"
#include "Parser.h"
#include "error.h"
#include "midcode.h"
#include "opt.h"
#include <iostream>
#include <cstdio>
#include <sstream>
#include <fstream>//�ļ�����
using namespace std;
int main()
{
	/*char ch;
	index = 0;
	while ((ch = getchar()) != EOF) {//�ӿ���̨���������ַ�
		S[index++] = ch;
	/}*/
	int out_index = 1;//���ʱ���±�
	string inputFilePath;
	cout << "input file path:" << endl;
	cin >> inputFilePath;
	//inputFilePath = "16231197_test.txt";
	
	ifstream fin(inputFilePath);
	if (!fin.is_open()) {
		cout << "File NoT Exist!" << endl;
		return 0;
	}
	//���ļ��������ݶ��뵽������
	S = string((std::istreambuf_iterator<char>(fin)), std::istreambuf_iterator<char>());
	fin.close();



	//S[index] = '\0';
	//cout << "����ok" << endl;
	index = 0;
	nextchar();
	
	getsym();
	program();//�ݹ��½� �﷨���崦��

	if (error_num == 0) {
		//cout << "�ʷ����� �﷨��������׶� �޴���" << endl;
		//delete_Tn();//ɾ�������Tn

		out_midcode();//����м���
		//Reference_count();//���ü��� ����ȫ�ּĴ���  ˳�����˺���û��return����
		mips_init();
		midcode2mips();

		if (error_num == 0) {
			cout << "����Ŀ����ɹ�" << endl;
		}

		leaf_function();//��Ŀ������ɾ��Ҷ������$ra
		out_mips();
	}
	else {
		cout << "�ʷ����� �﷨��������׶� ���ڴ���" << endl;
		cout << "ֹͣ����" << endl;
	}
	

	return 0;
}
