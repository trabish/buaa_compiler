#include "pch.h"
#include "Parser.h"
#include "Lexer.h"
#include "error.h"
#include "midcode.h"
#include <iostream>


/////////////////////////////////////////////////////////////////////////////////////////
//                         ���ű����                                                  //
/////////////////////////////////////////////////////////////////////////////////////////
using namespace std;
Table symbolTable[500];//���ű�

map<string, string> stringTable; //�����ַ�����

int which_Table = 0;// symbolTable���±� 0��ʾ���ȫ�ֱ� �����İ�˳����
int usedindexof_table = 0;//����¼�Ѿ�ʹ���˵�Table
table_item globle_table_item;//�ӷ��ű��в�ѯ���ĵ�ǰ������
void enter(string name, obj_enum obj, type_enum type, int adr, int length){//��¼��ͨ����
	//map<string, table_item>::iterator key = symbolTable[which_Table].maptable.find(name);
	/*if (key != symbolTable[which_Table].maptable.end()) {//����ͬ���ı���
		error();//�ظ�����
	}*/
	if (symbolTable[which_Table].maptable.count(name) == 1) {
		error("��ʶ���ظ�����",0);
		return;
	}
	table_item temp_item;
	temp_item.name = name;
	temp_item.obj = obj;
	temp_item.type = type;
	temp_item.adr = adr;
	temp_item.length = length;
	symbolTable[which_Table].maptable[name] = temp_item;
}
void enterparalist(string name, obj_enum obj, type_enum type, int adr, int length) {
	if (symbolTable[which_Table].maptable.count(name) == 1) {
		error("��ʶ���ظ�����",0);
		return;
	}
	if (symbolTable[which_Table].paranum>=Max_para) {
		error("��������");
		return;
	}
	table_item temp_item;
	temp_item.name = name;
	temp_item.obj = obj;
	temp_item.type = type;
	temp_item.adr = adr;
	temp_item.length = length;

	symbolTable[which_Table].paralist[symbolTable[which_Table].paranum] = temp_item;//���������
	symbolTable[which_Table].paranum++;//����������1
	symbolTable[which_Table].maptable[name] = temp_item;
}
void enterfunction(string function_name) {
	usedindexof_table++;
	if (usedindexof_table >= 500) {
		error("btab ���");//btab ���
		return;
	}
	which_Table = usedindexof_table;
	symbolTable[which_Table].function_name = function_name;//�Ǽ�����
	symbolTable[which_Table].paranum = 0;//��ʼ����������
	map<string, table_item>::iterator key = symbolTable[0].maptable.find(function_name);//�ҵ���ȫ�ֱ��еı���
	key->second.adr = usedindexof_table;//��������table�е�λ��
}



bool findItem(string name) {
	if (symbolTable[which_Table].maptable.count(name) == 0) {
		if (symbolTable[0].maptable.count(name) == 0) {
			return false;//û���������
		}
		else {
			globle_table_item = symbolTable[0].maptable[name];
			return true;
		}
		
	}
	else {
		globle_table_item = symbolTable[which_Table].maptable[name];
		return true;//�ҵ���������� ��������ֵ�� globle_table_item
	}
}




////////////////////////////////////////////////////////////////////////////////////////
int Parser_level = 0;//������ǰ��Ŀո�
void out_Parser_level() {
	int temp = Parser_level;
	for (int i = 0;i<Parser_level; i++) {
		cout << " ";
	}
	return;
}
//������    ::= �ۣ�����˵�����ݣۣ�����˵������{���з���ֵ�������壾|���޷���ֵ�������壾}����������
void program()
{
	Parser_level++;

	//��ʼ�� ȫ�ֱ�
	which_Table = 0;
	usedindexof_table = 0;
	symbolTable[0].function_name = "globle";
	symbolTable[0].paranum = 0;

	if (symbol == CONST_SY) {
		constDec();//��������
	}
	//�����ж���Ҫ��ǰ��
	int mark = 0;//�����������ܱ�׼
	int varDecmark = 0;//�Ƿ����������
	while (1) {
		if (symbol == CHAR_SY || symbol == INT_SY) {  //���� �������� �� �з���ֵ��������
			symbol3 = symbol;
			Token3.assign(Token) ;//��ֵ
			getsym();
			if (symbol == ID) {//������ʶ�� �����޷�����
				symbol2 = symbol;
				Token2.assign(Token);//��ֵ
				getsym();
				//������� [ , ; �����Ǳ���˵��
				if (symbol == LM_BACKET || symbol == COMMA || symbol == SEMI) {
					//���ڱ���������
					if (mark == 0) {
						varDef(true);// ��ʾ program ֱ���Ƶ�����varDec()  //�����ȸ�ΪvarDef();
						varDecmark = 1;//����vardec����
						if (symbol == SEMI)
						{
							getsym();
						}
						else {
							error("����Ӧ����;",0);//����Ӧ����;
							//return;
						}
					}
					else {
						error("����˵���뺯��������", INT_SY, false);
						//return;
					}
				}
				//���Ǳ���˵������ ���� �з���ֵ��������  ��Ҫ�� (
				else if(symbol==LS_BACKET){

					 
					if (varDecmark) {
						varDecmark = 0;
						//out_Parser_level();
						//cout << " This is a var declaration !" << endl;
					}
					mark = 1;
					enter(new_ID, obj_function, new_type, 0, 0);//����ȫ�ֱ�ע��   enterfunction������ַ
					enterfunction(new_ID);//ע�ắ�� 
					retfunctionDef();
					which_Table = 0;//�ص�ȫ�ֱ�
				}
				else {
					error("Ӧ����(", INT_SY, false);//Ӧ���� (
					//return;
				}
			}
			else {
				error("Ӧ���Ǳ�ʶ��",INT_SY,false);//Ӧ���Ǳ�ʶ��

				//return;
			}
		}
		//void �޷��غ��� �� void main
		else if (symbol == VOID_SY){
			mark = 1;//���˱�������
			getsym();
			//�޷���ֵ��������
			if (symbol == ID) {
				enter(new_ID, obj_function, new_type, 0, 0);//����ȫ�ֱ�ע��   enterfunction������ַ
				enterfunction(new_ID);//ע�ắ�� 
				voidfunctionDef();//ע���Ѿ������� void
				which_Table = 0;//�ص�ȫ�ֱ�
			}
			else if (symbol == MAIN_SY) {
				//��������

				enter("main", obj_function, new_type, 0, 0);//����ȫ�ֱ�ע��   enterfunction������ַ
				enterfunction("main");//ע�ắ�� 
				
				break;
			}
			else {
				error("Ӧ����main");//Ӧ����main
				//return;
			}
		}
		else {
			error("����������������", INT_SY, false);
			if (symbol == FILEEND) {
				return;
			}
			//return;
		}
	}
	mainFunction();//ע���Ѿ������� void

	if (symbol!=FILEEND) {
		cout << "main�����Ľ��� Ӧ���ǳ���Ľ���" << endl;
		return;
	}
	//out_Parser_level();
	//cout << "This is a program !" << endl;

	Parser_level--;
}

//������˵���� :: = const���������壾; { const���������壾; }
//ǰ���Ѿ����� const
void constDec() {
	Parser_level++;
	do {
		getsym();
		constDef();
		if (symbol == SEMI) {
			getsym();
		}
		else {
			error("ȱ�ٷֺ�",0);//ȱ�ٷֺ�
			//return;
		}
	} while (symbol == CONST_SY);//�����ķ� ����һ��const
	//out_Parser_level();
	//cout << "This is a const declaration !" << endl;
	Parser_level--;
}
//���������壾   :: = int����ʶ��������������{ ,����ʶ�������������� }| char����ʶ���������ַ���{ ,����ʶ���������ַ��� }
void constDef() {//д������ ������ʶ���ʶ������do while
	Parser_level++;
	if (symbol == INT_SY) {
	
		do {
			getsym();
			if (symbol == ID) {
				getsym();
				if (symbol == ASSIGN) {
					getsym();
					if (symbol == ADD || symbol == SUB) {
						if (symbol == ADD) {
							getsym();
							//������
						}
						else {
							getsym();
							num = -num;//�ı����
						}
						//getsym();
					}
					 if (symbol == UINT) {
			
						enter(new_ID, obj_const, type_int, num, 0);//enter�����ظ�����Ĵ���
						getsym();
					}
					else {
						error("Ӧ����UINT");//Ӧ���� UINT
						return;
					}
				}
				else {
					error("Ӧ���ǵȺ�");//Ӧ���ǵȺ�
					return;
				}
			}
			else {
				error("Ӧ���Ǳ�ʶ��");//Ӧ���� ID
				return;
			}

		} while (symbol==COMMA);
	}
	else if(symbol==CHAR_SY){
		do {
			getsym();
			if (symbol == ID) {
				getsym();
				if (symbol == ASSIGN) {
					getsym();
					
					if (symbol == CHAR) {

						
						enter(new_ID, obj_const, type_char,Token[0], 0);
						getsym();
					}
					else {
						error("Ӧ����CHAR");//Ӧ���� char
						return;
					}
				}
				else {
					error("Ӧ����=");//Ӧ���ǵȺ�
					return;
				}
			}
			else {
				error("Ӧ���Ǳ�ʶ��");//Ӧ���� ID
				return;
			}

		} while (symbol == COMMA);
	}
	else {
		error("Ӧ����int ����char");//Ӧ����int_sy or char_sy
		return;
	}
	
	//out_Parser_level();
	//cout << "This is a const definition !" << endl;
	Parser_level--;
}

//������˵����  ::= ���������壾;{���������壾;}
void varDec(bool program_mark){
	Parser_level++;
	varDef(program_mark);

	if (symbol == SEMI) {
		getsym();
		while (symbol==INT_SY||symbol==CHAR_SY) { //ѭ���ж� �жϴ��󲻱���  ������� ��������Ǻ�������
			varDef(false);

			if (symbol == SEMI) {
				getsym();
			}
			else {
				error("Ӧ����;",0);//Ӧ����;
				return;
			}
		}
	}
	else {
		error("Ӧ����;",0);//Ӧ���� ;
		return;
	}
	//out_Parser_level();
	//cout << "This is a var declaration !" << endl;
	Parser_level--;
}

//���������壾  ::= �����ͱ�ʶ����(����ʶ����|����ʶ����'['���޷���������']'){,(����ʶ����|����ʶ����'['���޷���������']' )}
//���޷�����������ʾ����Ԫ�صĸ�������ֵ�����0
void varDef(bool program_mark) {
	Parser_level++;
	if (program_mark) {//�Ѿ���ȡ��  �����ͱ�ʶ���� �� ����ʶ���� ����ȷ���������� �� ,[ ;
		if (symbol == COMMA) { //��Ϊ�У� ��֤���ٻ���һ�� ����ʶ����|����ʶ����'['���޷���������']'
			enter(new_ID, obj_var, new_type, 0, 0);//new_type new_ID
			do {
				getsym();
				if (symbol == ID) {//<��ʶ��>
					getsym();
					if (symbol == LM_BACKET) {//����ʶ����'['���޷���������']'
						getsym();

						if (symbol == UINT) {
							if (num == 0) {
								error("���������������Ϊ��",0);
								//return;
							}
							getsym();
							if (symbol == RM_BACKET) {
								getsym();
								if (new_type == type_char) {
									enter(new_ID, obj_var, type_char_group, 0, num);//char A[10]
								}
								else {
									enter(new_ID, obj_var, type_int_group, 0, num);//int A[10]
								}
							}
							else {
								error("Ӧ����]");//Ӧ���� ]
								return;
							}
						}
						else {
							error("Ӧ����int");//Ӧ���� int
							return;
						}
					}
					else {//��ʶ��
						enter(new_ID, obj_var, new_type, 0, 0);//int a �� char a
					}
				}
				else {
					error("Ӧ���Ǳ�ʶ��");//Ӧ���� ��ʶ��
					return;
				}

			} while (symbol == COMMA);
		}
		else if (symbol == LM_BACKET) {
			getsym();

			if (symbol == UINT) {
				if (num == 0) {
					error("���������������Ϊ��",0);//�±�Ӧ�ô���0
					//return;
				}
				getsym();
				if (symbol == RM_BACKET) {
					getsym();
					if (new_type == type_char) {
						enter(new_ID, obj_var, type_char_group, 0, num);//char A[10]
					}
					else {
						enter(new_ID, obj_var, type_int_group, 0, num);//int A[10]
					}
				}
				else {
					error("Ӧ����]");//Ӧ���� ]
					return;
				}

			}
			else {
				error("Ӧ����int");//Ӧ���� int 
				return;
			}
			while (symbol == COMMA) {//û�б�֤һ����    ,(����ʶ����|����ʶ����'['���޷���������']' )
				getsym();
				if (symbol == ID) {
					getsym();
					if (symbol == LM_BACKET) {
						getsym();
						if (symbol == UINT) {
							if (num == 0) {
								error("���������������Ϊ��",0);//�±�Ӧ�ô���0
								//return;
							}
							getsym();
							if (symbol == RM_BACKET) {
								getsym();
								if (new_type == type_char) {
									enter(new_ID, obj_var, type_char_group, 0, num);//char A[10]
								}
								else {
									enter(new_ID, obj_var, type_int_group, 0, num);//int A[10]
								}
							}
							else {
								error("Ӧ����]");//Ӧ���� ]
								return;
							}

						}
						else {
							error("Ӧ����int");//Ӧ���� int
							return;
						}
					}
					else {//��ʶ��
						enter(new_ID, obj_var, new_type, 0, 0);//int a �� char a
					}
				}
				else {
					error("Ӧ���Ǳ�ʶ��");//Ӧ���� ��ʶ��
					return;
				}
			}
		}
		else {
			//����; ���ù�

			enter(new_ID, obj_var, new_type, 0, 0);
		}

	}
	else {
		if (symbol == CHAR_SY || INT_SY) {
			do {
				getsym();
				if (symbol == ID) {
					getsym();
					if (symbol == LM_BACKET) {
						getsym();
						if (symbol == UINT) {
							if (num == 0) {
								error("���������������Ϊ��",0);//�±�Ӧ�ô���0
								//return;
							}
							getsym();
							if (symbol == RM_BACKET) {
								getsym();
								if (new_type == type_char) {
									enter(new_ID, obj_var, type_char_group, 0, num);//char A[10]
								}
								else {
									enter(new_ID, obj_var, type_int_group, 0, num);//int A[10]
								}
							}
							else {
								error("Ӧ����]");//Ӧ���� ]
								return;
							}

						}
						else {
							error("Ӧ����int");//Ӧ���� int
							return;
						}
					}
					else {
						enter(new_ID, obj_var, new_type, 0, 0);//int a �� char a
					}
				}
				else {
					error("Ӧ���Ǳ�ʶ��");//Ӧ���� ��ʶ��
					return;
				}
			} while (symbol == COMMA);//��֤������һ�� ����ʶ����|����ʶ����'['���޷���������']'
		}
		else {
			error("Ӧ����CHAR_SY��INT_SY");//Ӧ���� CHAR_SY or INT_SY
			return;
		}
	}
	//out_Parser_level();
	//cout << " This is a var definition !" << endl;
	Parser_level--;
}



//���з���ֵ�������壾  ::=  ������ͷ����'('��������')' '{'��������䣾'}'
//�Ѿ�����int|char �ͱ�ʶ�� ������ָ�� ( 
void retfunctionDef() {
	Parser_level++;

	if (new_type = type_char) {
		enter_midcode(op_char,"function","",new_ID);
	}
	else {
		enter_midcode(op_int, "function", "", new_ID);
	}

	if (symbol == LS_BACKET)
	{
		getsym();

		paraList();
		if (symbol == RS_BACKET) {
			getsym();
			if (symbol == LL_BACKET) {
				getsym();
				compoundStatement();
				if (symbol == RL_BACKET) {
					getsym();
				}
				else {
					error("Ӧ����}");//Ӧ���� }
					return;
				}
			}
			else {
				error("Ӧ����{");//Ӧ���� {
				return;
			}
		}
		else {
			error("Ӧ����)");//Ӧ���� )
			return;
		}
	}
	else {
		error("Ӧ����(");//Ӧ���ǣ�
		return;
	}
	enter_midcode(op_end,"","","");
	//out_Parser_level();
	//cout << "This is a return functionDef !" << endl;
	Parser_level--;
}

//�Ѿ����� void �� ��ʶ�� ����ָ���ʶ��
//���޷���ֵ�������壾  ::= void����ʶ����'('��������')''{'��������䣾'}'
void voidfunctionDef() {
	Parser_level++;
	enter_midcode(op_void, "function", "", new_ID);
	getsym();
	if (symbol == LS_BACKET) {
		getsym();
		paraList();
		if (symbol == RS_BACKET) {
			getsym();
			if (symbol == LL_BACKET) {
				getsym();
				compoundStatement();
				if (symbol == RL_BACKET) {
					getsym();
				}
				else {
					error("Ӧ����}");//Ӧ���� }
					return;
				}
			}
			else {
				error("Ӧ����{");//Ӧ���� {
				return;
			}
		}
		else {
			error("Ӧ����)");//Ӧ���� )
			return;
		}
	}
	else {
		error("Ӧ����(");//Ӧ���� (
		return;
	}
	enter_midcode(op_end, "", "", "");
	//out_Parser_level();
	//cout << "This is a void functionDef !" << endl;
	Parser_level--;
}

//����������    ::= void main'('')''{'��������䣾'}'
//�Ѿ����� void main ����ָ�� main
void mainFunction() {
	Parser_level++;
	enter_midcode(op_void, "function", "", "main");
	getsym();
	if (symbol == LS_BACKET) {
		getsym();
		if (symbol == RS_BACKET) {
			getsym();
			if (symbol == LL_BACKET) {
				getsym();
				compoundStatement();
				if (symbol == RL_BACKET) {
					getsym();
				}
				else {
					error("Ӧ����}");//Ӧ���� }
					return;
				}
			}
			else {
				error("Ӧ����{");//Ӧ���� {
				return;
			}
		}
		else {
			error("Ӧ����)");//Ӧ���� )
			return;
		}
	}
	else {
		error("Ӧ����(");//Ӧ���� (
		return;
	}
	enter_midcode(op_end, "", "", "");
	//out_Parser_level();
	//cout << "This is a mainfunction !" << endl;
	Parser_level--;
}

//��������䣾   ::=  �ۣ�����˵�����ݣۣ�����˵�����ݣ�����У�
void compoundStatement() {
	Parser_level++;
	if (symbol == CONST_SY) {
		constDec();
	}
	if (symbol == CHAR_SY || symbol == INT_SY) {
		varDec(false);//��program �Ƶ���
	}
	Statements();//�����
	//out_Parser_level();
	//cout << "This is a compoundStatement !" << endl;
	Parser_level--;
}
//��������    ::= ��������{,��������}| ����>
void paraList() {
	Parser_level++;
	if (symbol == RS_BACKET) { //���ֱ�Ӷ��� ) ˵��û�в���
		//out_Parser_level();
		//cout << "This is a paraList without para !" << endl;
		Parser_level--;
		return;
	}
	if (symbol == INT_SY || symbol == CHAR_SY) {
		getsym();
		if (symbol == ID) {
			enterparalist(new_ID, obj_para, new_type, 0, 0);//ע�����
			getsym();
			while (symbol == COMMA) {
				getsym();
				if (symbol == INT_SY || symbol == CHAR_SY) {
					getsym();
					if (symbol == ID) {
						enterparalist(new_ID, obj_para, new_type, 0, 0);//ע�����
						getsym();
					}
					else {
						error("Ӧ���Ǳ�ʶ��", RS_BACKET,true);//Ӧ���� ��ʶ��
						return;
					}
				}
				else {
					error("Ӧ����INT_SY or CHAR_SY" , RS_BACKET, true);//Ӧ����INT_SY or CHAR_SY
					return;
				}
			}
		}
		else {
			error("Ӧ���Ǳ�ʶ��", RS_BACKET, true);//Ӧ���Ǳ�ʶ��
			return;
		}
	}
	else {
		error("Ӧ���ǲ���", RS_BACKET, true);//Ӧ���ǲ���
		return;
	}
	//out_Parser_level();
	//cout << "This is a paraList with para !" << endl;
	Parser_level--;
}
//������У�   ::= ������䣾��
void Statements() {
	Parser_level++;
	while (symbol!=RL_BACKET) {//������Ҫע�� ��������н�������һ�� symbol һ���� }
		Statement();
	}
	//out_Parser_level();
	//cout << "This is a Statements !" << endl;
	Parser_level--;
}

//����䣾    ::= ��������䣾����ѭ����䣾| '{'������У�'}'| ���з���ֵ����������䣾; 
//  | ���޷���ֵ����������䣾; ������ֵ��䣾; ��������䣾; ����д��䣾; �����գ�; | �������䣾����������䣾;
void Statement() {
	Parser_level++;
	if (symbol == IF_SY) {//�������
		ifStatement();
	}
	else if (symbol==WHILE_SY) {//ѭ�����
		whileStatement();
	}
	else if (symbol == LL_BACKET) {//�����
		getsym();
		Statements();
		if (symbol == RL_BACKET) {
			getsym();
		}
		else {
			error("����Ӧ���� }");//����Ӧ���� }
			return;
		}
	}
	else if (symbol==SCANF_SY) {//�����
		readStatement();
		if (symbol == SEMI) {
			getsym();
		}
		else {
			error("Ӧ����;",0);//Ӧ����;
			return;
		}
	}
	else if (symbol == PRINTF_SY) {//д���
		writeStatement();
		if (symbol == SEMI) {
			getsym();
		}
		else {
			error("Ӧ����;",0);//Ӧ����;
			return;
		}
	}
	else if (symbol == SEMI) {
		getsym();
		//out_Parser_level();
		//cout << "This is a empty statement !" << endl;
	}
	else if (symbol == SWITCH_SY) {//������
		switchStatement();
	}
	else if (symbol == RETURN_SY) {//�������
		returnStatement();
		if (symbol == SEMI) {
			getsym();
		}
		else {
			error("Ӧ����;",0);//Ӧ����;
			return;
		}
	}
	else if (symbol == ID) {
		symbol2 = symbol;//��ǰ�� ���б���
		Token2.assign(Token);//��ǰ�� ���б���
		getsym();
		if (symbol == LS_BACKET) { //�з��� �� �޷���ֵ�ĺ�������
			//getsym();
			callFunction();//�Ѿ����� functionname �� (
		}
		else {//��ֵ���
			assignStatement();//�Ѿ����� ��ʶ��
		}

		if (symbol == SEMI) {
			getsym();
		}
		else {
			error("Ӧ����;",0);//Ӧ����;
			return;
		}
	}
	else {
		error("û�з���<���>�Ŀ�ʼ����");//û�з���<���>�Ŀ�ʼ����
		return;
	}
	//out_Parser_level();
	//cout << "This is a  statement !" << endl;
	Parser_level--;
}

//�����ʽ��    ::= �ۣ������ݣ��{���ӷ�����������}
bool expression(string &tn) {
	int calmark, value;

	Parser_level++;
	bool int_mark = false;//����ֵ�Ƿ�ʱint
	if (symbol == ADD||symbol==SUB) {
		Sym temp_symbol=symbol;

		getsym();
		term(tn, calmark, value);//��һ�� ֱ�Ӵ���tn
		if (temp_symbol == ADD) {
			//��������
		}
		else {
			enter_midcode(op_SUB,"0",tn,tn);//tn=0-tn
		}
		int_mark = true;
	}
	else {
		bool T_or_F=term(tn, calmark, value);
		if (T_or_F == true) {
			int_mark = true;
		}
	}
	
	while (symbol == ADD || symbol == SUB) {
		Sym temp_symbol = symbol;
		int_mark = true;
		getsym();
		string tn2;
	
		term(tn2, calmark, value);
		if (temp_symbol == ADD) {
			enter_midcode(op_ADD, tn, tn2, tn);//tn=tn+tn2
		}
		else {
			enter_midcode(op_SUB, tn, tn2, tn);//tn=tn-tn2
		}
	
	}
	//out_Parser_level();
	//cout << "This is a expression !" << endl;
	Parser_level--;
	return int_mark;
}
bool expression(string &tn, int &calmark, int &value) {//����experssion ר�����ھ�̬�����±���
	Parser_level++;
	bool int_mark = false;//����ֵ�Ƿ�ʱint
	if (symbol == ADD || symbol == SUB) {
		Sym temp_symbol = symbol;
	
		getsym();
		term(tn, calmark, value);//��һ�� ֱ�Ӵ���tn
		if (temp_symbol == ADD) {
			//��������
		}
		else {
			enter_midcode(op_SUB, "0", tn, tn);//tn=0-tn
			value = -value;
		}
		int_mark = true;
	}
	else {
		bool T_or_F = term(tn, calmark, value);
		if (T_or_F == true) {
			int_mark = true;
		}
	}

	while (symbol == ADD || symbol == SUB) {
		Sym temp_symbol = symbol;
		int_mark = true;
		getsym();
		string tn2;

		term(tn2, calmark, value);
		calmark = 1;//����������
		if (temp_symbol == ADD) {
			enter_midcode(op_ADD, tn, tn2, tn);//tn=tn+tn2
		}
		else {
			enter_midcode(op_SUB, tn, tn2, tn);//tn=tn-tn2
		}

	}
	//out_Parser_level();
	//cout << "This is a expression !" << endl;
	Parser_level--;
	return int_mark;
}
//���     :: = �����ӣ�{ ���˷�������������ӣ� }
bool term(string &tn, int &calmark, int &value) {
	Parser_level++;
	bool int_mark = false;//����ֵ�Ƿ�ʱint
	bool T_or_F=factor(tn, calmark, value);//��һ�� ֱ�Ӵ���tn
	if (T_or_F == true) {
		int_mark = true;
	}
	while (symbol == MULT || symbol == DIV) {
		Sym temp_symbol = symbol;
		int_mark = true;
		getsym();
		string tn2;

		factor(tn2 ,calmark, value);
		calmark = 1;//����������
		if (temp_symbol == MULT) {
			enter_midcode(op_MULT, tn, tn2, tn);//tn=tn*tn2
		}
		else {
			enter_midcode(op_DIV, tn, tn2, tn);//tn=tn/tn2
		}
	}
	//out_Parser_level();
	//cout << "This is a term !" << endl;
	Parser_level--;
	return int_mark;
}

//�����ӣ�    ::= ����ʶ����������ʶ����'['�����ʽ��']'|'('�����ʽ��')'����������|���ַ��������з���ֵ����������䣾 
bool factor(string &tn, int &calmark,int &value) {//calmark ���ڱ�ʶ�Ƿ񾭹����� value��ʶ��ֵ
	calmark = 1;

	Parser_level++;
	bool int_mark = false;//����ֵ�Ƿ�ʱint
	if (symbol==SUB) {
		int_mark = true;
		getsym();
		if (symbol == UINT) {//��������
			num = -num;//ֵȡ��
			getsym();
			get_tn(tn);//��ȡ��ʱ����
			enter_midcode(op_assign, to_string(num), "int", tn);//tn=-num ֱ�Ӹ�ֵ

			value = num;
			calmark = 0;
		}
		else {
			error("Ӧ����uint");//Ӧ����uint
			return false;
		}
	}
	else if (symbol == ADD) {
		int_mark = true;
		getsym();
		if (symbol == UINT) {//��������
			getsym();
			get_tn(tn);//��ȡ��ʱ����
			enter_midcode(op_assign, to_string(num), "int", tn);//tn=num ֱ�Ӹ�ֵ

			value = num;
			calmark = 0;
		}
		else {
			error("Ӧ����uint");//Ӧ����uint
			return false;
		}
	}
	else if (symbol == UINT) {
		int_mark = true;
		getsym();
		get_tn(tn);//��ȡ��ʱ����
		enter_midcode(op_assign, to_string(num), "int", tn);//tn=num ֱ�Ӹ�ֵ

		value = num;
		calmark = 0;
	}
	else if (symbol == CHAR) {//���ַ���
		get_tn(tn);//��ȡ��ʱ����
		enter_midcode(op_assign,to_string((int)Token[0]), "char", tn);
		getsym();
	}
	else if (symbol==LS_BACKET) {//'('�����ʽ��')'
		int_mark = true;
		getsym();
		expression(tn);
		if (symbol == RS_BACKET) {
			getsym();
		}
		else {
			error("Ӧ���� )", RS_BACKET, true);//Ӧ���� )
			return false;
		}
	}
	else if (symbol == ID) {
		/*if (findItem(Token) == false) {
			error("��ʶ��δ����");
		 }*/

		symbol2 = symbol;//��ǰ�� ���б���
		Token2.assign(Token);//��ǰ�� ���б���
		getsym();
		if (symbol == LS_BACKET) {//���з���ֵ����������䣾
			//getsym();
			/*if (findItem(new_ID) == false) {
				error("��ʶ��δ����");
			}*/
			if (symbolTable[0].maptable.count(new_ID) == 0) {
				error("��ʶ��δ����",0);
				return false;
			}
			else {
				globle_table_item = symbolTable[0].maptable[new_ID];
			}
			if (globle_table_item.obj != obj_function|| globle_table_item.type==type_void) {
				error("�Ǻ��� ���� �޷���ֵ");
				return false;
			}
			if ( globle_table_item.type == type_int) {
				int_mark = true;
			}
			

			callFunction();//�Ѿ����� functionname �� (
			get_tn(tn);//��ȡ��ʱ����
			enter_midcode(op_RETvalue, "", "", tn); //������ֵ��ֵ�� tn=RET
		}
		else if (symbol == LM_BACKET) {//����ʶ����'['�����ʽ��']'
			string group_ID = new_ID;
			if (findItem(new_ID) == false) {
				error("��ʶ��δ����");
				return false;
			} 
			if (globle_table_item.type != type_int_group&& globle_table_item.type != type_char_group) {
				error("������");
				return false;
			}
			if (globle_table_item.type == type_int_group) {
				int_mark = true;
			}
			getsym();
			string tn2;
			get_tn(tn2);//��ȡһ���µ���ʱ����

			int calmark, value;


			bool T_or_F=expression(tn2, calmark, value);//�õ��±�ֵ
			if (calmark == 0) {//û��������
				findItem(group_ID);
				if (value >= globle_table_item.length||value<0) {//�������鳤��
					error("��̬�����±�Խ��",0);
					//return false;
				}
			}

			if (T_or_F == false) {//
				error("�±겻Ӧ��ʱ�ַ�����",0);
				//return false;
			}
		
			if (symbol == RM_BACKET) {
				getsym();
				get_tn(tn);//��ȡ��ʱ����
				enter_midcode(op_eqaddr, tn2, group_ID, tn); //������ֵ��ֵ�� tn=new_ID[tn2]
			}
			else {
				error("Ӧ���� ]", RM_BACKET, true);//Ӧ���� ]
				//return false;
			}
		}
		else {//����ʶ����
			//����getsym() 
			if (findItem(new_ID) == false) {
				error("��ʶ��δ����");
				return false;
			}
			if (globle_table_item.type == type_int) {
				if (globle_table_item.obj == obj_const) {//����

					value = globle_table_item.adr;
					calmark = 0;

					int_mark = true;
					get_tn(tn);
					enter_midcode(op_assign, to_string(globle_table_item.adr), "int", tn);
				}
				else {
					int_mark = true;
					get_tn(tn);
					//tn = globle_table_item.name;
					enter_midcode(op_assign, globle_table_item.name, "", tn);
				}
			}
			else if (globle_table_item.type == type_char) {
				get_tn(tn);
				//tn = globle_table_item.name;
				if (globle_table_item.obj == obj_const) {//�ַ�����
					enter_midcode(op_assign, to_string(globle_table_item.adr), "char", tn);
				}
				else {
					enter_midcode(op_assign, globle_table_item.name, "", tn);
				}
				
			}
			else {
				error("��ʶ�����Ͳ���",0);
				//return false;
			}
		}
	}
	else {
		error("factorû�к��ʵĿ�ʼ����");//û�к��ʵĿ�ʼ����
		return false;
	}
	//out_Parser_level();
	//cout << "This is a factor !" << endl;
	Parser_level--;
	return int_mark;
}
//��������䣾  ::=  if '('��������')'����䣾
void ifStatement() {
	Parser_level++;
	if (symbol == IF_SY) {
		getsym();
		if (symbol = LS_BACKET) {//��������    ::=  �����ʽ������ϵ������������ʽ���������ʽ��
			getsym();
			string tn;
			bool T_or_F_1=expression(tn);
			string temp_lable;//����lable
			get_lable(temp_lable);
			if (symbol == EQ || symbol == NOTEQ || symbol == LESS || symbol == LESSEQ || symbol == MORE || symbol == MOREEQ) {//tn:tn2
				Sym temp_symbol = symbol;//������׼
				getsym();
				string tn2;
				bool T_or_F_2=expression(tn2);
				if (T_or_F_1 != T_or_F_2|| T_or_F_1 !=true) {
					error("��ϵ���������ֻ��Ϊint��",0);
					//return;
				}
				switch (temp_symbol) {
				case EQ:
					enter_midcode(op_EQ,tn,tn2, temp_lable);
					break;
				case NOTEQ:
					enter_midcode(op_NOTEQ, tn, tn2, temp_lable);
					break;
				case LESS:
					enter_midcode(op_LESS, tn, tn2, temp_lable);
					break;
				case LESSEQ:
					enter_midcode(op_LESSEQ, tn, tn2, temp_lable);
					break;
				case MORE:
					enter_midcode(op_MORE, tn, tn2, temp_lable);
					break;
				case MOREEQ:
					enter_midcode(op_MOREEQ, tn, tn2, temp_lable);
					break;
				default:
					error("������");
					return;
			
				}
			}
			else {//tn :0
				if (T_or_F_1 == false) {
					error("�������ʹ���",0);
					//return;
				}
				enter_midcode(op_NOTEQ, tn, "0", temp_lable);
			}
			if (symbol == RS_BACKET) {
				//string temp_lable;
				//get_lable(temp_lable);
				//enter_midcode(op_BZ, "", "", temp_lable);//���û���

				getsym();
				Statement();
				enter_midcode(op_lable, "", "", temp_lable);
			}
			else {
				error("Ӧ���� )");//Ӧ���� )
				return;
			}
		}
		else {
			error("Ӧ���� (");//Ӧ���� (
			return;
		}
	}
	else {
		error("Ӧ���� if");//Ӧ���� if
		return;
	}
	//out_Parser_level();
	//cout << "This is a ifStatement !" << endl;
	Parser_level--;
}
//��ѭ����䣾   ::=  while '('��������')'����䣾
void whileStatement() {
	Parser_level++;
	string temp_lable1;
	get_lable(temp_lable1);
	//Ϊwhile ��������lable
	temp_lable1 = "while_" + temp_lable1;


	enter_midcode(op_lable, "", "", temp_lable1);//���ÿ�ͷlable
	if (symbol == WHILE_SY) {
		getsym();
		if (symbol = LS_BACKET) {//��������    ::=  �����ʽ������ϵ������������ʽ���������ʽ��
			getsym();
			string tn;
			bool T_or_F_1=expression(tn);
			string temp_lable2;//����lable
			get_lable(temp_lable2);
			if (symbol == EQ || symbol == NOTEQ || symbol == LESS || symbol == LESSEQ || symbol == MORE || symbol == MOREEQ) {//tn:tn2
				Sym temp_symbol = symbol;//������׼
				getsym();
				string tn2;
				bool T_or_F_2=expression(tn2);
				if (T_or_F_1 != T_or_F_2|| T_or_F_1 != true) {
					error("��ϵ���������ֻ��Ϊint��",0);
					//return;
				}
				switch (temp_symbol) {
				case EQ:
					enter_midcode(op_EQ, tn, tn2, temp_lable2);
					break;
				case NOTEQ:
					enter_midcode(op_NOTEQ, tn, tn2, temp_lable2);
					break;
				case LESS:
					enter_midcode(op_LESS, tn, tn2, temp_lable2);
					break;
				case LESSEQ:
					enter_midcode(op_LESSEQ, tn, tn2, temp_lable2);
					break;
				case MOREEQ:
					enter_midcode(op_MOREEQ, tn, tn2, temp_lable2);
					break;
				case MORE:
					enter_midcode(op_MORE, tn, tn2, temp_lable2);
					break;
				default:
					error("no");
					return;

				}
			}
			else {//tn :0
				if (T_or_F_1 == false) {
					error("�������ʹ���",0);
					//return;
				}
				enter_midcode(op_NOTEQ, tn, "0", temp_lable2);
			}
			if (symbol == RS_BACKET) {
				//string temp_lable2;
				//get_lable(temp_lable2);
				//enter_midcode(op_BZ, "", "", temp_lable2);//���û��� ������β

				getsym();
				Statement();

				enter_midcode(op_GOTO, "", "", temp_lable1);//������ͷ
				enter_midcode(op_lable, "", "", temp_lable2);//���ý�βlable
			}
			else {
				error("Ӧ���� )");//Ӧ���� )
				return;
			}
		}
		else {
			error("Ӧ���� (");//Ӧ���� (
			return;
		}
	}
	else {
		error("Ӧ����while");//Ӧ����while
		return;
	}
	//out_Parser_level();
	//cout << "This is a whileStatement !" << endl;
	Parser_level--;
}


//���з���ֵ����������䣾 ::= ����ʶ����'('��ֵ������')'
//���޷���ֵ����������䣾 ::= ����ʶ����'('��ֵ������')'
//�ѽ����� ������ �� ( ����ָ��
void callFunction() {
	string functionName = new_ID;//�Ƚ���������������
	//Token2 = new_ID;//�Ƚ���������������
	//enter_midcode(op_call, "", "", new_ID);//call new_ID
	if (symbolTable[0].maptable.count(new_ID) == 0) {
		error("����δ����");
		return;
	}
	Table temp_table = symbolTable[symbolTable[0].maptable[new_ID].adr];//��ȡ�������ڵ�btab
	

	Parser_level++;
	getsym();
	if (symbol == RS_BACKET) {
		if (temp_table.paranum!=0) {
			error("����̫��",0);
			//return;
		}
		getsym();
		//out_Parser_level();
		//cout << "This is a empty valueparalist !" << endl;
	}
	else {
		int index_of_para = 0;
		string tn;
		bool T_or_F=expression(tn);

		if (index_of_para > temp_table.paranum) {
			error("��������",0);
			//return;
		}
		else {
			if ((temp_table.paralist[index_of_para].type == type_char && T_or_F == true)||(temp_table.paralist[index_of_para].type == type_int && T_or_F == false)) {//��Ҫ�Ĳ���Ϊchar �������int
				error("�������ʹ��� ",0);
				//return;
			}
			else {
				enter_midcode(op_push, tn, "", "");
			}
			index_of_para++;
		}
		while (symbol == COMMA) {
			getsym();
			string tn2;
			T_or_F = expression(tn2);
			if (index_of_para >= temp_table.paranum) {
				error("��������",0);
				//return;
			}
			else {
				if ((temp_table.paralist[index_of_para].type == type_char && T_or_F == true)|| (temp_table.paralist[index_of_para].type == type_int && T_or_F == false)) {//��Ҫ�Ĳ���Ϊchar �������int
					error("�������ʹ���",0);
					//return;
				}
				else {
					enter_midcode(op_push, tn2, "", "");
				}
				index_of_para++;
			}
		}
		if (index_of_para < temp_table.paranum) {
			error("��������",0);
			//return;
		}
		//out_Parser_level();
		//cout << "This is a valueparalist !" << endl;
		if (symbol == RS_BACKET) {
			getsym();
		}
		else {
			error("Ӧ����)" , RS_BACKET, true);//Ӧ����)
			return;
		}
	}

	enter_midcode(op_call, "", "",functionName);//���ú���
	//out_Parser_level();
	//cout << "This is a callFunction !" << endl;
	Parser_level--;
}

//����ֵ��䣾   ::=  ����ʶ�����������ʽ��|����ʶ����'['�����ʽ��']'=�����ʽ��
//�Ѿ����� ��ʶ�� ����Ӧ��ָ�� =���� [
void assignStatement() {
	Parser_level++;
	table_item temp_item;
	string tn;
	string ID_name = new_ID;
	if (symbol == LM_BACKET) {
		if (findItem(new_ID)==false) {
			error("��ʶ��δ����");//��ʶ��δ����
			return;
		}
		temp_item = globle_table_item;

		if (globle_table_item.type != type_int_group && globle_table_item.type != type_char_group) {
			error("������");
			return;
		}
		getsym();

		int calmark, value;

		bool T_or_F=expression(tn, calmark, value);
		if (calmark == 0) {//δ��������
			findItem(ID_name);
			if (value >= globle_table_item.length||value<0) {
				//cout << value << endl;
				error("��̬�����±�Խ��",0);
				//return;
			}
		}
		if (T_or_F == false) {//
			error("�±겻Ӧ�����ַ�����",0);
			//return;
		}
		if (symbol == RM_BACKET) {
			
			getsym();
		}
		else {
			error("Ӧ���� ]", RM_BACKET, true);//Ӧ���� ]
			return;
		}
	}
	else {
		if (findItem(new_ID) == false) {
			error("��ʶ��δ����");//��ʶ��δ����
			return;
		}
		temp_item = globle_table_item;
	}
	if (symbol == ASSIGN) {
		if (temp_item.obj == obj_const) {
			error("����������ֵ",0);
			//return;
		}
		getsym();
		string tn2;
		bool T_or_F=expression(tn2);//���ܻ�ı� globle_table_item
		if (T_or_F == true && (temp_item.type == type_char || temp_item.type == type_char_group)) {//int���ܸ���char
			error("���ʹ��� char=int",0);
			//return;
		}
		if (T_or_F == false && (temp_item.type == type_int || temp_item.type == type_int_group)) {//char���ܸ���int
			error("���ʹ��� int=char",0);
			//return;
		}
		if (temp_item.type != type_int_group && temp_item.type != type_char_group) {//������
			enter_midcode(op_assign, tn2, "", ID_name);
		}
		else {//����
			enter_midcode(op_addreq, tn, tn2, ID_name);
		}
	}
	else {
		error("Ӧ���� =");//Ӧ���� =
		return;
	}
	//out_Parser_level();
	//cout << "This is a assignStatement !" << endl;
	Parser_level--;
}
//������䣾    ::=  scanf '('����ʶ����{,����ʶ����}')'
void readStatement() {
	Parser_level++;
	if (symbol == SCANF_SY) {
		getsym();
		if (symbol == LS_BACKET) {
			do {
				getsym();
				if (symbol == ID) {
					if (findItem(new_ID) == false) {
						error("��ʶ��δ����");
						return;
					}
			
					if (globle_table_item.obj!=obj_var&&(globle_table_item.type!=type_int&& globle_table_item.type != type_char)) {
						error("scanf �б�ʶ�����ͳ���",0);
						//return;
					}
					if (globle_table_item.type == type_int) {//scanf int 
						enter_midcode(op_scanf, "int", "", globle_table_item.name);
					}
					else {//scanf char
						enter_midcode(op_scanf, "char", "", globle_table_item.name);
					}
					getsym();
				}
				else {
					error("Ӧ���Ǳ�ʶ��");//Ӧ���Ǳ�ʶ��
					return;
				}

			} while (symbol == COMMA);//����һ����ʶ��
			if (symbol == RS_BACKET) {
				getsym();
			}
			else {
				error("Ӧ���� )", RS_BACKET, true);//Ӧ���� )
				return;
			}
		}
		else {
			error("Ӧ���� (", RS_BACKET, true);//Ӧ���� (
			return;
		}
	}
	else {
		error("Ӧ���� scanf");//Ӧ���� scanf
		return;
	}
	//out_Parser_level();
	///cout << "This is a readStatement !" << endl;
	Parser_level--;
}
//��д��䣾    ::= printf '(' ���ַ�����,�����ʽ�� ')'| printf '('���ַ����� ')'| printf '('�����ʽ��')'
void writeStatement() {
	Parser_level++;
	if (symbol == PRINTF_SY) {
		getsym();
		if (symbol == LS_BACKET) {
			getsym();
			if (symbol == STRING) {
				string temp_string;
				get_string(temp_string,Token);
				enter_midcode(op_printf,"string","", temp_string);
				getsym();
				if (symbol == COMMA) {
					getsym();
					string tn;
					bool T_or_F=expression(tn);
					if (T_or_F == true) {
						enter_midcode(op_printf, "int", "", tn);
					}
					else {
						enter_midcode(op_printf, "char", "", tn);
					}
				}
				if (symbol == RS_BACKET) {
					getsym();
				}
				else {
					error("Ӧ����)", RS_BACKET, true);//Ӧ����)
					return;
				}
				
			}
			else {
				string tn;
				bool T_or_F = expression(tn);
				if (T_or_F == true) {
					enter_midcode(op_printf, "int", "", tn);
				}
				else {
					enter_midcode(op_printf, "char", "", tn);
				}
				if (symbol == RS_BACKET) {
					getsym();
				}
				else {
					error("Ӧ����)", RS_BACKET, true);//Ӧ����)
					return;
				}
			}
		}
		else {
			error("Ӧ���� (", RS_BACKET, true);//Ӧ���� (
			return;
		}
	}
	else {
		error("Ӧ������ printf");//Ӧ������ printf
		return;
	}
	//out_Parser_level();
	//cout << "This is a writeStatement !" << endl;
	Parser_level--;
}
//��������䣾   ::=  return['('�����ʽ��')']  
void returnStatement() {
	Parser_level++;
	if (symbol == RETURN_SY) {
		getsym();
		if (symbol == LS_BACKET) {
			getsym();
			string tn;
			bool T_or_F=expression(tn);
			if ((T_or_F == false && symbolTable[0].maptable[symbolTable[which_Table].function_name].type == type_char)
				|| (T_or_F == true && symbolTable[0].maptable[symbolTable[which_Table].function_name].type == type_int)
				) {

			}
			else {
				error("�����������ʹ���",0);
				//return;
			}
			if (symbol == RS_BACKET) {
				getsym();
				enter_midcode(op_ret, "exp", "", tn);
			}
			else {
				error("Ӧ����)", RS_BACKET, true);//Ӧ����)
				return;
			}
		}
		else {
			enter_midcode(op_ret, "null", "", "");
		}
	}
	else {
		error("Ӧ����return");//Ӧ����return
		return;
	}
	//out_Parser_level();
	//cout << "This is a returnStatement !" << endl;
	Parser_level--;
}
//�������䣾  ::=  switch '('�����ʽ��')' '{'���������ȱʡ�� '}'   ��ȱʡ��   ::=  default : ����䣾|���գ�
void switchStatement() {
	Parser_level++;
	string tn;
	bool T_or_F;
	string switch_end_lable;
	get_lable(switch_end_lable);
	//string default_lable;
	//get_lable(default_lable);
	if (symbol == SWITCH_SY) {
		getsym();
		if (symbol == LS_BACKET) {
			getsym();
			T_or_F=expression(tn);
			if (symbol == RS_BACKET) {
				getsym();
				if (symbol == LL_BACKET) {
					getsym();
					situationTable(tn,switch_end_lable,T_or_F);
					if (symbol == DEFAULT_SY) {
						//enter_midcode(op_lable,"","",default_lable);
						
						getsym();
						if (symbol == COLON) {
							getsym();
							Statement();
						}
						else {
							error("Ӧ���� ð��");//Ӧ���� ð��
							return;
						}
						enter_midcode(op_lable, "", "", switch_end_lable); //����switch ��end
						//out_Parser_level();
						//cout << "This is a defaultStatement !" << endl;
					}
					else {
						//enter_midcode(op_lable, "", "", default_lable);//default lable �� switch_end_lable ��ͬһ���ط�
						enter_midcode(op_lable, "", "", switch_end_lable); //����switch ��end
						//out_Parser_level();
						//cout << "This is a  empty defaultStatement !" << endl;
					}
					if (symbol == RL_BACKET) {
						getsym();
					}
					else {
						error("Ӧ���� }");//Ӧ���� }
						return;
					}
				}
				else {
					error("Ӧ����{");//Ӧ����{
					return;
				}
			}
			else {
				error("Ӧ����)");//Ӧ����)
				return;
			}
		}
		else {
			error("Ӧ���� (");//Ӧ���� (
			return;
		}
	}
	else {
		error("Ӧ����switch");//Ӧ����switch
		return;
	}
	//out_Parser_level();
	//cout << "This is a switchStatement !" << endl;
	Parser_level--;
}
//�������   ::=  ���������䣾{���������䣾}
//���������䣾  :: = case��������������䣾
void situationTable(string tn,string switch_end_lable,bool case_type) {
	Parser_level++;
	string next_lable;

	if (symbol == CASE_SY) {
		do {
			getsym();
			if ( symbol == CHAR) {//����
				if (case_type == true) {
					error("switch���ʹ���",0);
					//return;
				}
				get_lable(next_lable);
				enter_midcode(op_EQ, tn,to_string( (int)Token[0]), next_lable);//�Ƚ�
				//enter_midcode(op_BZ,"","", next_lable);//���������� ����ת
				getsym();
				if (symbol == COLON) {
					getsym();
					Statement();
					enter_midcode(op_GOTO, "", "", switch_end_lable);
					enter_midcode(op_lable, "", "", next_lable);
				}
				else {
					error("Ӧ���� ð��");//Ӧ���� ð��
					return;
				}
			}
			else if (symbol == SUB||symbol==ADD) {
				if (case_type == false) {
					error("switch���ʹ���",0);
					//return;
				}
				if (symbol == SUB)
				{
					getsym();
					num = -num;
				}
				else {
					getsym();
				}
				if (symbol == UINT) {
					get_lable(next_lable);
					enter_midcode(op_EQ, tn, to_string(num), next_lable);//�Ƚ�
					//enter_midcode(op_BZ, "", "", next_lable);//���������� ����ת
					getsym();
					if (symbol == COLON) {
						getsym();
						Statement();
						enter_midcode(op_GOTO, "", "", switch_end_lable);
						enter_midcode(op_lable, "", "", next_lable);
					}
					else {
						error("Ӧ���� ð��");//Ӧ���� ð��
						return;
					}
				}
				else {
					error("Ӧ����UINT");//Ӧ����UINT
					return;
				}
			}
			else if(symbol == UINT) {
				if (case_type == false) {
					error("switch���ʹ���",0);
					//return;
				}
				get_lable(next_lable);
				enter_midcode(op_EQ, tn, to_string(num), next_lable);//�Ƚ�
				//enter_midcode(op_BZ, "", "", next_lable);//���������� ����ת
				getsym();
				if (symbol == COLON) {
					getsym();
					Statement();
					enter_midcode(op_GOTO, "", "", switch_end_lable);
					enter_midcode(op_lable, "", "", next_lable);
				}
				else {
					error("Ӧ���� ð��");//Ӧ���� ð��
					return;
				}
			}
			else {
				error("Ӧ���ǳ���");//Ӧ���ǳ���
				return;
			}
		} while (symbol == CASE_SY);
	}
	else {
		error("Ӧ����case");//Ӧ����case
		return;
	}
	//out_Parser_level();
	//cout << "This is a situationTable !" << endl;
	Parser_level--;
}