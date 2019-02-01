#include "pch.h"
#include "opt.h"
#include "midcode.h"
#include "Parser.h"
#include "Lexer.h"
#include "error.h"
#include "midcode.h"
#include <algorithm>
#include <map>

//Ҷ����ɾ�� ����$ra��ָ��
void leaf_function() {

	map<string, int> function_map;
	string now_function;
	for (int i = 0; i < midcode_index; i++) {//��һ�� �ҵ�Ҷ����

		if (midcodes[i].op == op_none) {//����Ϊ��
			continue;
		}
		switch (midcodes[i].op)
		{
		case op_int:
		case op_char:
		case op_void:
			function_map[midcodes[i].des] = 1;//��Ǻ���
			now_function = midcodes[i].des;
			break;
		case op_call://��������������
			function_map[now_function] = 0;//��Ҷ����
			break;

		}
	}


	int delete_ra_mark = 0;
	for (int i = 0; i < mipscode_index; i++) {
		string now_lable = mipscodes[i].substr(0,mipscodes[i].size() - 1);
		if (mipscodes[i][mipscodes[i].size() - 1] == ':') {
			if (function_map.count(now_lable) == 1) {//���ڸú���
				if (function_map[now_lable] == 1) {//Ҷ����
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

Reference_data function_Reference_data[500];//ÿ��������ȫ�ּĴ�������  ȷ����ÿ�������б�����Ӧ��ȫ�ּĴ���
void Reference_count() {
	bool globle_reg_close;//�ر�ȫ�ּĴ�������
	bool function_globle_reg_close=false;//�رյ��ú�������ĺ�����ȫ�ּĴ������� true�ǹر�
	int function_close_num=5;//�رյ��ú�������ĺ��� ����ָ��������

	////////////////////////
	/////////////////////// ��������һ��  �����ô����뺯�����ô�����Ȩ���ж��Ƿ�ʹ�����ȫ�ֱ���
	
	string function_name;
	
	for (int i = 0; i < midcode_index; i++) {
		if (midcodes[i].op == op_none) {
			continue;
		}
		else if (midcodes[i].src1 == "function") {//����ĳ������

			bool return_num = false;//��⺯��֮���Ƿ���return

			int call_function_num = 0;//���ú���������

			map<string, int>  count_ref;//��¼ÿ�����������ô���
			int while_mark = 0;//��while�е���ȣ�
			int while_factor = 5;//��while�б������õ�Ȩ�� 1+ while_mark*while_factor
			function_name = midcodes[i].des;//��¼������
			int symbolTable_index = symbolTable[0].maptable[function_name].adr;
			while (midcodes[i].op != op_end) {//�˳�����
				switch (midcodes[i].op) {
				case op_none:
					break;
				case op_ADD://��Ԫ����
				case op_SUB:
				case op_MULT:
				case op_DIV:
					//src1
					if (midcodes[i].src1[0] == '-' || (midcodes[i].src1[0] >= '0'&&midcodes[i].src1[0] <= '9')) {//����
					}
					else {//��ʶ��
						if (count_ref.count(midcodes[i].src1) == 0) {//û�м�¼��
							count_ref[midcodes[i].src1] = while_mark*while_factor+1;
						}
						else {
							count_ref[midcodes[i].src1]= count_ref[midcodes[i].src1]+ while_mark * while_factor + 1;//��һ�����ô���
						}
					}
					//src2
					if (midcodes[i].src2[0] == '-' || (midcodes[i].src2[0] >= '0'&&midcodes[i].src2[0] <= '9')) {//����
					}
					else {//��ʶ��
						if (count_ref.count(midcodes[i].src2) == 0) {//û�м�¼��
							count_ref[midcodes[i].src2] = while_mark * while_factor + 1;
						}
						else {
							count_ref[midcodes[i].src2]= count_ref[midcodes[i].src2]+ while_mark * while_factor + 1;//��һ�����ô���
						}
					}
					//des
					if (midcodes[i].des[0] == '-' || (midcodes[i].des[0] >= '0'&&midcodes[i].des[0] <= '9')) {//����
					}
					else {//��ʶ��
						if (count_ref.count(midcodes[i].des) == 0) {//û�м�¼��
							count_ref[midcodes[i].des] = while_mark * while_factor + 1;
						}
						else {
							count_ref[midcodes[i].des]= count_ref[midcodes[i].des]+ while_mark * while_factor + 1;//��һ�����ô���
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
					if (midcodes[i].src1[0] == '-' || (midcodes[i].src1[0] >= '0'&&midcodes[i].src1[0] <= '9')) {//����
					}
					else {//��ʶ��
						if (count_ref.count(midcodes[i].src1) == 0) {//û�м�¼��
							count_ref[midcodes[i].src1] = while_mark * while_factor + 1;
						}
						else {
							count_ref[midcodes[i].src1] = count_ref[midcodes[i].src1] + while_mark * while_factor + 1;//��һ�����ô���
						}
					}
					break;
				case op_RETvalue:
				case op_ret:
					if (midcodes[i].op == op_ret) {
						return_num = true;//��⵽�����ڴ���return
					}
					//des
					if (midcodes[i].des.empty()||midcodes[i].des[0] == '-' || (midcodes[i].des[0] >= '0'&&midcodes[i].des[0] <= '9')) {//����
					}
					else {//��ʶ��
						if (count_ref.count(midcodes[i].des) == 0) {//û�м�¼��
							count_ref[midcodes[i].des] = while_mark * while_factor + 1;
						}
						else {
							count_ref[midcodes[i].des] = count_ref[midcodes[i].des] + while_mark * while_factor + 1;//��һ�����ô���
						}
					}
					break;
				case op_end:
					error("��ô������");
					break;
				case op_eqaddr://=[]
				case op_assign:
					//src1
					if (midcodes[i].src1[0] == '-' || (midcodes[i].src1[0] >= '0'&&midcodes[i].src1[0] <= '9')) {//����
					}
					else {//��ʶ��
						if (count_ref.count(midcodes[i].src1) == 0) {//û�м�¼��
							count_ref[midcodes[i].src1] = while_mark * while_factor + 1;
						}
						else {
							count_ref[midcodes[i].src1] = count_ref[midcodes[i].src1] + while_mark * while_factor + 1;//��һ�����ô���
						}
					}
					//des
					if (midcodes[i].des[0] == '-' || (midcodes[i].des[0] >= '0'&&midcodes[i].des[0] <= '9')) {//����
					}
					else {//��ʶ��
						if (count_ref.count(midcodes[i].des) == 0) {//û�м�¼��
							count_ref[midcodes[i].des] = while_mark * while_factor + 1;
						}
						else {
							count_ref[midcodes[i].des] = count_ref[midcodes[i].des] + while_mark * while_factor + 1;//��һ�����ô���
						}
					}
					break;
				case op_addreq://[]=  ����Ԫ�ز����м���
					//src1
					if (midcodes[i].src1[0] == '-' || (midcodes[i].src1[0] >= '0'&&midcodes[i].src1[0] <= '9')) {//����
					}
					else {//��ʶ��
						if (count_ref.count(midcodes[i].src1) == 0) {//û�м�¼��
							count_ref[midcodes[i].src1] = while_mark * while_factor + 1;
						}
						else {
							count_ref[midcodes[i].src1] = count_ref[midcodes[i].src1] + while_mark * while_factor + 1;//��һ�����ô���
						}
					}
					//src2
					if (midcodes[i].src2[0] == '-' || (midcodes[i].src2[0] >= '0'&&midcodes[i].src2[0] <= '9')) {//����
					}
					else {//��ʶ��
						if (count_ref.count(midcodes[i].src2) == 0) {//û�м�¼��
							count_ref[midcodes[i].src2] = while_mark * while_factor + 1;
						}
						else {
							count_ref[midcodes[i].src2] = count_ref[midcodes[i].src2] + while_mark * while_factor + 1;//��һ�����ô���
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
					if (midcodes[i].des[0] == '-' || (midcodes[i].des[0] >= '0'&&midcodes[i].des[0] <= '9')) {//����
					}
					else {//��ʶ��
						if (count_ref.count(midcodes[i].des) == 0) {//û�м�¼��
							count_ref[midcodes[i].des] = while_mark * while_factor + 1;
						}
						else {
							count_ref[midcodes[i].des] = count_ref[midcodes[i].des] + while_mark * while_factor + 1;//��һ�����ô���
						}
					}
					break;
				case op_lable: //������ע��
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
					if (midcodes[i].src1[0] == '-' || (midcodes[i].src1[0] >= '0'&&midcodes[i].src1[0] <= '9')) {//����
					}
					else {//��ʶ��
						if (count_ref.count(midcodes[i].src1) == 0) {//û�м�¼��
							count_ref[midcodes[i].src1] = while_mark * while_factor + 1;
						}
						else {
							count_ref[midcodes[i].src1] = count_ref[midcodes[i].src1] + while_mark * while_factor + 1;//��һ�����ô���
						}
					}
					//src2
					if (midcodes[i].src2[0] == '-' || (midcodes[i].src2[0] >= '0'&&midcodes[i].src2[0] <= '9')) {//����
					}
					else {//��ʶ��
						if (count_ref.count(midcodes[i].src2) == 0) {//û�м�¼��
							count_ref[midcodes[i].src2] = while_mark * while_factor + 1;
						}
						else {
							count_ref[midcodes[i].src2] = count_ref[midcodes[i].src2] + while_mark * while_factor + 1;//��һ�����ô���
						}
					}
					break;
				default:
					error("let me see see");
				}
				i++;
			}

			/*if (function_globle_reg_close==true&&call_function_num>= function_close_num) {
				continue;//��Ϊ�ú�������ȫ�ּĴ���
			}*/


			//����ȫ�ּĴ�������
			int var_num=0;//��������
			var_sort var_sort_group[500];//���ڱ������ô�������
			map<string, int>::iterator iter;//������
			iter = count_ref.begin();
			while (iter != count_ref.end()) {//���������ݵ��� var_sort_group
				var_sort tem_var;
				tem_var.name = iter->first;

				tem_var.num = iter->second;
				if (symbolTable[symbolTable_index].maptable.count(tem_var.name) == 0) {//ȫ�ֱ���
					tem_var.num = 0;
				}
				var_sort_group[var_num++] = tem_var;
				iter++;
			}

			sort(var_sort_group, var_sort_group + var_num, comp);//�Խ����������


			
			for (int j = 0; j < (var_num < 8 ? var_num : 8); j++) {//��Ӧȫ�ּĴ���
				if (4>= var_sort_group[j].num) {
					continue;
				}
				if (symbolTable[symbolTable_index].maptable.count(var_sort_group[j].name) == 0) {//ȫ�ֱ���������ȫ�ּĴ���
					continue;
				}
				function_Reference_data[symbolTable_index].map_reg[var_sort_group[j].name] = "$s"+to_string(j);//����$si
				

			}
			if (return_num == false && symbolTable[0].maptable[function_name].type!=type_void) {
				error(function_name+"����ȱ��return");
			}

		}
		else {
			error("����ʲô��Ԫʽѽ");
		}
	}
}

void delete_Tn() {//1.�滻tn 2.ɾ��������
	bool delete_more = true;//���� add i 1 @T     assign @T i ���ֽ���ɾ��
	//��� assign i  Tn �����

	bool G_delete_Tn = true;//ȫ�ֱ�����Tn��ɾ��

	string Tn_def[2000];//���tn��def ֵ
	for (int i = 0; i < 2000; i++) {
		Tn_def[i] = "#";//��ʾ��δ����

	}
	//1.�滻Tn
	for (int i = 0; i < midcode_index; i++) {//��һ�� �滻Tn

		if (midcodes[i].op == op_none) {//����Ϊ��
			continue;
		}
		switch (midcodes[i].op)
		{
		case op_ADD://��Ԫ����
		case op_SUB:
		case op_MULT:
		case op_DIV:
			//����һ
			if (midcodes[i].src1[0] == '@')
			{
				int temp = str2int(midcodes[i].src1.substr(2));
				if (Tn_def[temp] != "#") {
					midcodes[i].src1 = Tn_def[temp];
				}
			}
			//������
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
				Tn_def[temp] = "#";//���������
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
				Tn_def[temp] = "#";//���������
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
			//������ȫ�ֱ���

			if (midcodes[i].src1[0] == '@')
			{
				int temp = str2int(midcodes[i].src1.substr(2));
				if (Tn_def[temp] != "#") {
					midcodes[i].src1 = Tn_def[temp];
				}
			}
			if ((midcodes[i].src1[0]!='@')&&midcodes[i].des[0] == '@')
			{
				if (symbolTable[0].maptable.count(midcodes[i].src1) == 0) {//�ֲ����� ���Բ���


					int temp = str2int(midcodes[i].des.substr(2));
					Tn_def[temp] = midcodes[i].src1;
				}
				else {//ȫ�ֱ��� ������
					int temp = str2int(midcodes[i].des.substr(2));
					Tn_def[temp] = "#";//���������
				}
			}
			

			break;
		case op_addreq://[]= 
		case op_eqaddr:
			//����һ
			if (midcodes[i].src1[0] == '@')
			{
				int temp = str2int(midcodes[i].src1.substr(2));
				if (Tn_def[temp] != "#") {
					midcodes[i].src1 = Tn_def[temp];
				}
			}
			//������
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
				Tn_def[temp] = "#";//���������
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
			//����һ
			if (midcodes[i].src1[0] == '@')
			{
				int temp = str2int(midcodes[i].src1.substr(2));
				if (Tn_def[temp] != "#") {
					midcodes[i].src1 = Tn_def[temp];
				}
			}
			//������
			if (midcodes[i].src2[0] == '@')
			{
				int temp = str2int(midcodes[i].src2.substr(2));
				if (Tn_def[temp] != "#") {
					midcodes[i].src2 = Tn_def[temp];
				}
			}
			break;


		default:
			error("�м�����bug");
			break;
		}
	}
	//2.ɾ�������tn
	for (int i = 0; i < midcode_index; i++) {
		if (midcodes[i].op == op_assign) {
			if ((midcodes[i].src1[0] != '@') && midcodes[i].des[0] == '@'&&symbolTable[0].maptable.count(midcodes[i].src1) == 0) {//ȫ�ֱ�����ɾ
				midcodes[i].op = op_none;//Ϊ��
			}
		}
	}
////////////////////////////////////////////////
	////////////////////////////////////////////

	if (delete_more == true) {//��������
		for (int i = 1; i < midcode_index; i++) {
			if (midcodes[i].op == op_assign) {
				string temp;
				if (midcodes[i].src1[0] == '@') {
					switch (midcodes[i - 1].op) {
					case op_ADD://��Ԫ����
					case op_SUB:
					case op_MULT:
					case op_DIV:
						if (midcodes[i - 1].des == midcodes[i].src1)
						{
							midcodes[i - 1].des = midcodes[i].des;
						}
						temp = midcodes[i].des;//����@tn�ͱ�����λ�� �ں����ɾ��������Ϳ��Խ���ɾ��
						midcodes[i].des = midcodes[i].src1;
						midcodes[i].src1 = temp;


						break;
					case op_RETvalue:
						if (midcodes[i - 1].des == midcodes[i].src1)
						{
							midcodes[i - 1].des = midcodes[i].des;
						}

						temp = midcodes[i].des;//����@tn�ͱ�����λ�� �ں����ɾ��������Ϳ��Խ���ɾ��
						midcodes[i].des = midcodes[i].src1;
						midcodes[i].src1 = temp;

						break;
					}
					

				}
			}
		}
	}
	//////////////////////////////////////////////////
	if (G_delete_Tn == true) {//���л������ڵĴ���
		//����ȫ�ֱ���@tn ��ɾ��
		//�ڻ�������ɾ��
		int GTn_def[2000];//���tn��def ֵ
		int j_line = 0;//��ǰ�����鿪ʼ����
		for (int i = 0; i < 2000; i++) {
			GTn_def[i] = -1;//��ʾ��δ����
			Tn_def[i] = "#";

		}
		for (int i = 0; i < midcode_index; i++) {//��һ�� �滻Tn

			if (midcodes[i].op == op_none) {//����Ϊ��
				continue;
			}
			switch (midcodes[i].op)
			{
			case op_ADD://��Ԫ����
			case op_SUB:
			case op_MULT:
			case op_DIV:
				//����һ
				if (midcodes[i].src1[0] == '@')
				{
					int temp = str2int(midcodes[i].src1.substr(2));
					if (Tn_def[temp] != "#" && (GTn_def[temp] > j_line)) {
						midcodes[i].src1 = Tn_def[temp];
					}
				}
				//������
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
					Tn_def[temp] = "#";//���������
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
					Tn_def[temp] = "#";//���������
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
				//��ȫ�ֱ���

				if (midcodes[i].src1[0] == '@')
				{
					int temp = str2int(midcodes[i].src1.substr(2));
					if (Tn_def[temp] != "#" && (GTn_def[temp] > j_line)) {
						midcodes[i].src1 = Tn_def[temp];
					}
				}
				if ((midcodes[i].src1[0] != '@') && midcodes[i].des[0] == '@')
				{
					if (symbolTable[0].maptable.count(midcodes[i].src1) == 0) {//�ֲ����� 
					}
					else {//ȫ�ֱ��� 
						int temp = str2int(midcodes[i].des.substr(2));
						GTn_def[temp] = i;
						Tn_def[temp] = midcodes[i].src1;
					}
				}


				break;
			case op_addreq://[]= 
			case op_eqaddr:
				//����һ
				if (midcodes[i].src1[0] == '@')
				{
					int temp = str2int(midcodes[i].src1.substr(2));
					if (Tn_def[temp] != "#" && (GTn_def[temp] > j_line)) {
						midcodes[i].src1 = Tn_def[temp];
					}
				}
				//������
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
					Tn_def[temp] = "#";//���������
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
				//����һ
				if (midcodes[i].src1[0] == '@')
				{
					int temp = str2int(midcodes[i].src1.substr(2));
					if (Tn_def[temp] != "#" && (GTn_def[temp] > j_line)) {
						midcodes[i].src1 = Tn_def[temp];
					}
				}
				//������
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
				error("�м�����bug");
				break;
			}
		}


		//ɾ�������� ֻɾ����ʱ�����Ķ���
		map<string, int> var_map;//int -1����ʹ�ù� ����0��ֵ�����䶨�崦(û�����ظ���������)
		for (int i = 0; i < midcode_index; i++) {//��һ�� �滻Tn

			if (midcodes[i].op == op_none) {//����Ϊ��
				continue;
			}
			switch (midcodes[i].op)
			{
			case op_ADD://��Ԫ����
			case op_SUB:
			case op_MULT:
			case op_DIV:
				//����һ
				if (is_num(midcodes[i].src1)==0)
				{
					var_map[midcodes[i].src1] = -1;
				}
				//������
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
				//������
				if (is_num(midcodes[i].src2) == 0)
				{
					var_map[midcodes[i].src2] = -1;
				}
				break;
			case op_eqaddr:
				//����һ
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
				//����һ
				if (is_num(midcodes[i].src1) == 0)
				{
					var_map[midcodes[i].src1] = -1;
				}
				//������
				if (is_num(midcodes[i].src2) == 0)
				{
					var_map[midcodes[i].src2] = -1;
				}

				break;


			default:
				error("�м�����bug");
				break;
			}
		}
		//ֻɾ��ʱ����
		map<string, int>::iterator iter;//������ 
		iter = var_map.begin();
		while (iter !=var_map.end()) {
			if (iter->second != -1&& symbolTable[0].maptable.count(iter->first) == 0) {//��ȫ����
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


