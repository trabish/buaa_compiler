#pragma once
#include "pch.h"
#include "Parser.h"
#include "Lexer.h"
#include "error.h"
#include <map>
#include <iostream>
#include "midcode.h"

//ɾ���������ʱ����  ����Tn��������ʹ�õ����� Tn����纯�� 
//����󲿷���˳��ͷ�֧�ṹΨһ��������whileҲ���Ǵ��ϵ��µ�def ��use
//���п�����һ�ּ��� ��ȷ ��Ч�ķ�ʽ ɾ��Tn     ˼��������ȫ�ֵĸ��ƴ���+ɾ��������
//��Ҫì����assign �͸�������ֱ�ӵ�ì��
//�Ҷ���һ��ʹ��һ�� �������ʿ��ܳ���
void delete_Tn();


struct Reference_data {
	map<string, string>  map_reg;//��¼������ȫ�ּĴ���
};
extern Reference_data function_Reference_data[500];//��¼ȫ�ּĴ����ķ���
void Reference_count();//���㺯���ڵı��������� ��Ϊ��������ȫ�ּĴ���

void leaf_function();


