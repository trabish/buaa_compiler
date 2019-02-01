#include "pch.h"
#include "Parser.h"
#include "Lexer.h"
#include "error.h"
#include "midcode.h"
#include <iostream>


/////////////////////////////////////////////////////////////////////////////////////////
//                         符号表操作                                                  //
/////////////////////////////////////////////////////////////////////////////////////////
using namespace std;
Table symbolTable[500];//符号表

map<string, string> stringTable; //常量字符串表

int which_Table = 0;// symbolTable的下标 0表示这个全局表 其他的按顺序排
int usedindexof_table = 0;//来记录已经使用了的Table
table_item globle_table_item;//从符号表中查询到的当前符号项
void enter(string name, obj_enum obj, type_enum type, int adr, int length){//登录普通表项
	//map<string, table_item>::iterator key = symbolTable[which_Table].maptable.find(name);
	/*if (key != symbolTable[which_Table].maptable.end()) {//存在同样的表项
		error();//重复定义
	}*/
	if (symbolTable[which_Table].maptable.count(name) == 1) {
		error("标识符重复定义",0);
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
		error("标识符重复定义",0);
		return;
	}
	if (symbolTable[which_Table].paranum>=Max_para) {
		error("参数过多");
		return;
	}
	table_item temp_item;
	temp_item.name = name;
	temp_item.obj = obj;
	temp_item.type = type;
	temp_item.adr = adr;
	temp_item.length = length;

	symbolTable[which_Table].paralist[symbolTable[which_Table].paranum] = temp_item;//填入参数表
	symbolTable[which_Table].paranum++;//参数个数加1
	symbolTable[which_Table].maptable[name] = temp_item;
}
void enterfunction(string function_name) {
	usedindexof_table++;
	if (usedindexof_table >= 500) {
		error("btab 溢出");//btab 溢出
		return;
	}
	which_Table = usedindexof_table;
	symbolTable[which_Table].function_name = function_name;//登记名字
	symbolTable[which_Table].paranum = 0;//初始化参数个数
	map<string, table_item>::iterator key = symbolTable[0].maptable.find(function_name);//找到在全局表中的表项
	key->second.adr = usedindexof_table;//填入其在table中的位置
}



bool findItem(string name) {
	if (symbolTable[which_Table].maptable.count(name) == 0) {
		if (symbolTable[0].maptable.count(name) == 0) {
			return false;//没有这个表项
		}
		else {
			globle_table_item = symbolTable[0].maptable[name];
			return true;
		}
		
	}
	else {
		globle_table_item = symbolTable[which_Table].maptable[name];
		return true;//找到了这个表项 并将它赋值给 globle_table_item
	}
}




////////////////////////////////////////////////////////////////////////////////////////
int Parser_level = 0;//输出语句前面的空格
void out_Parser_level() {
	int temp = Parser_level;
	for (int i = 0;i<Parser_level; i++) {
		cout << " ";
	}
	return;
}
//＜程序＞    ::= ［＜常量说明＞］［＜变量说明＞］{＜有返回值函数定义＞|＜无返回值函数定义＞}＜主函数＞
void program()
{
	Parser_level++;

	//初始化 全局表
	which_Table = 0;
	usedindexof_table = 0;
	symbolTable[0].function_name = "globle";
	symbolTable[0].paranum = 0;

	if (symbol == CONST_SY) {
		constDec();//常量声明
	}
	//以下判断需要超前读
	int mark = 0;//变量声明介绍标准
	int varDecmark = 0;//是否进行了声明
	while (1) {
		if (symbol == CHAR_SY || symbol == INT_SY) {  //考虑 变量声明 和 有返回值函数定义
			symbol3 = symbol;
			Token3.assign(Token) ;//赋值
			getsym();
			if (symbol == ID) {//读到标识符 ，任无法区分
				symbol2 = symbol;
				Token2.assign(Token);//赋值
				getsym();
				//如果读到 [ , ; ，则是变量说明
				if (symbol == LM_BACKET || symbol == COMMA || symbol == SEMI) {
					//任在变量声明处
					if (mark == 0) {
						varDef(true);// 表示 program 直接推导出的varDec()  //这里先改为varDef();
						varDecmark = 1;//存在vardec（）
						if (symbol == SEMI)
						{
							getsym();
						}
						else {
							error("这里应该是;",0);//这里应该是;
							//return;
						}
					}
					else {
						error("变量说明与函数定义混合", INT_SY, false);
						//return;
					}
				}
				//不是变量说明部分 则是 有返回值函数定义  需要是 (
				else if(symbol==LS_BACKET){

					 
					if (varDecmark) {
						varDecmark = 0;
						//out_Parser_level();
						//cout << " This is a var declaration !" << endl;
					}
					mark = 1;
					enter(new_ID, obj_function, new_type, 0, 0);//现在全局表注册   enterfunction会回填地址
					enterfunction(new_ID);//注册函数 
					retfunctionDef();
					which_Table = 0;//回到全局表
				}
				else {
					error("应该是(", INT_SY, false);//应该是 (
					//return;
				}
			}
			else {
				error("应该是标识符",INT_SY,false);//应该是标识符

				//return;
			}
		}
		//void 无返回函数 或 void main
		else if (symbol == VOID_SY){
			mark = 1;//过了变量定义
			getsym();
			//无返回值函数定义
			if (symbol == ID) {
				enter(new_ID, obj_function, new_type, 0, 0);//现在全局表注册   enterfunction会回填地址
				enterfunction(new_ID);//注册函数 
				voidfunctionDef();//注意已经读过了 void
				which_Table = 0;//回到全局表
			}
			else if (symbol == MAIN_SY) {
				//读主函数

				enter("main", obj_function, new_type, 0, 0);//现在全局表注册   enterfunction会回填地址
				enterfunction("main");//注册函数 
				
				break;
			}
			else {
				error("应该是main");//应该是main
				//return;
			}
		}
		else {
			error("不存在这样的类型", INT_SY, false);
			if (symbol == FILEEND) {
				return;
			}
			//return;
		}
	}
	mainFunction();//注意已经读过了 void

	if (symbol!=FILEEND) {
		cout << "main函数的结束 应该是程序的结束" << endl;
		return;
	}
	//out_Parser_level();
	//cout << "This is a program !" << endl;

	Parser_level--;
}

//＜常量说明＞ :: = const＜常量定义＞; { const＜常量定义＞; }
//前面已经读了 const
void constDec() {
	Parser_level++;
	do {
		getsym();
		constDef();
		if (symbol == SEMI) {
			getsym();
		}
		else {
			error("缺少分号",0);//缺少分号
			//return;
		}
	} while (symbol == CONST_SY);//根据文法 至少一次const
	//out_Parser_level();
	//cout << "This is a const declaration !" << endl;
	Parser_level--;
}
//＜常量定义＞   :: = int＜标识符＞＝＜整数＞{ ,＜标识符＞＝＜整数＞ }| char＜标识符＞＝＜字符＞{ ,＜标识符＞＝＜字符＞ }
void constDef() {//写复杂了 可以先识别标识符，再do while
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
							//不操作
						}
						else {
							getsym();
							num = -num;//改变符号
						}
						//getsym();
					}
					 if (symbol == UINT) {
			
						enter(new_ID, obj_const, type_int, num, 0);//enter中又重复定义的处理
						getsym();
					}
					else {
						error("应该是UINT");//应该是 UINT
						return;
					}
				}
				else {
					error("应该是等号");//应该是等号
					return;
				}
			}
			else {
				error("应该是标识符");//应该是 ID
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
						error("应该是CHAR");//应该是 char
						return;
					}
				}
				else {
					error("应该是=");//应该是等号
					return;
				}
			}
			else {
				error("应该是标识符");//应该是 ID
				return;
			}

		} while (symbol == COMMA);
	}
	else {
		error("应该是int 或者char");//应该是int_sy or char_sy
		return;
	}
	
	//out_Parser_level();
	//cout << "This is a const definition !" << endl;
	Parser_level--;
}

//＜变量说明＞  ::= ＜变量定义＞;{＜变量定义＞;}
void varDec(bool program_mark){
	Parser_level++;
	varDef(program_mark);

	if (symbol == SEMI) {
		getsym();
		while (symbol==INT_SY||symbol==CHAR_SY) { //循环判断 判断错误不报错  这里出错 下面如果是函数定义
			varDef(false);

			if (symbol == SEMI) {
				getsym();
			}
			else {
				error("应该是;",0);//应该是;
				return;
			}
		}
	}
	else {
		error("应该是;",0);//应该是 ;
		return;
	}
	//out_Parser_level();
	//cout << "This is a var declaration !" << endl;
	Parser_level--;
}

//＜变量定义＞  ::= ＜类型标识符＞(＜标识符＞|＜标识符＞'['＜无符号整数＞']'){,(＜标识符＞|＜标识符＞'['＜无符号整数＞']' )}
//＜无符号整数＞表示数组元素的个数，其值需大于0
void varDef(bool program_mark) {
	Parser_level++;
	if (program_mark) {//已经读取了  ＜类型标识符＞ 和 ＜标识符＞ 而且确定接下来的 是 ,[ ;
		if (symbol == COMMA) { //因为有， 保证至少还有一个 ＜标识符＞|＜标识符＞'['＜无符号整数＞']'
			enter(new_ID, obj_var, new_type, 0, 0);//new_type new_ID
			do {
				getsym();
				if (symbol == ID) {//<标识符>
					getsym();
					if (symbol == LM_BACKET) {//＜标识符＞'['＜无符号整数＞']'
						getsym();

						if (symbol == UINT) {
							if (num == 0) {
								error("定义数组个数不能为零",0);
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
								error("应该是]");//应该是 ]
								return;
							}
						}
						else {
							error("应该是int");//应该是 int
							return;
						}
					}
					else {//标识符
						enter(new_ID, obj_var, new_type, 0, 0);//int a 或 char a
					}
				}
				else {
					error("应该是标识符");//应该是 标识符
					return;
				}

			} while (symbol == COMMA);
		}
		else if (symbol == LM_BACKET) {
			getsym();

			if (symbol == UINT) {
				if (num == 0) {
					error("定义数组个数不能为零",0);//下标应该大于0
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
					error("应该是]");//应该是 ]
					return;
				}

			}
			else {
				error("应该是int");//应该是 int 
				return;
			}
			while (symbol == COMMA) {//没有保证一定有    ,(＜标识符＞|＜标识符＞'['＜无符号整数＞']' )
				getsym();
				if (symbol == ID) {
					getsym();
					if (symbol == LM_BACKET) {
						getsym();
						if (symbol == UINT) {
							if (num == 0) {
								error("定义数组个数不能为零",0);//下标应该大于0
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
								error("应该是]");//应该是 ]
								return;
							}

						}
						else {
							error("应该是int");//应该是 int
							return;
						}
					}
					else {//标识符
						enter(new_ID, obj_var, new_type, 0, 0);//int a 或 char a
					}
				}
				else {
					error("应该是标识符");//应该是 标识符
					return;
				}
			}
		}
		else {
			//读到; 不用管

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
								error("定义数组个数不能为零",0);//下标应该大于0
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
								error("应该是]");//应该是 ]
								return;
							}

						}
						else {
							error("应该是int");//应该是 int
							return;
						}
					}
					else {
						enter(new_ID, obj_var, new_type, 0, 0);//int a 或 char a
					}
				}
				else {
					error("应该是标识符");//应该是 标识符
					return;
				}
			} while (symbol == COMMA);//保证至少有一个 ＜标识符＞|＜标识符＞'['＜无符号整数＞']'
		}
		else {
			error("应该是CHAR_SY或INT_SY");//应该是 CHAR_SY or INT_SY
			return;
		}
	}
	//out_Parser_level();
	//cout << " This is a var definition !" << endl;
	Parser_level--;
}



//＜有返回值函数定义＞  ::=  ＜声明头部＞'('＜参数表＞')' '{'＜复合语句＞'}'
//已经读了int|char 和标识符 ，现在指向 ( 
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
					error("应该是}");//应该是 }
					return;
				}
			}
			else {
				error("应该是{");//应该是 {
				return;
			}
		}
		else {
			error("应该是)");//应该是 )
			return;
		}
	}
	else {
		error("应该是(");//应该是（
		return;
	}
	enter_midcode(op_end,"","","");
	//out_Parser_level();
	//cout << "This is a return functionDef !" << endl;
	Parser_level--;
}

//已经读了 void 和 标识符 现在指向标识符
//＜无返回值函数定义＞  ::= void＜标识符＞'('＜参数表＞')''{'＜复合语句＞'}'
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
					error("应该是}");//应该是 }
					return;
				}
			}
			else {
				error("应该是{");//应该是 {
				return;
			}
		}
		else {
			error("应该是)");//应该是 )
			return;
		}
	}
	else {
		error("应该是(");//应该是 (
		return;
	}
	enter_midcode(op_end, "", "", "");
	//out_Parser_level();
	//cout << "This is a void functionDef !" << endl;
	Parser_level--;
}

//＜主函数＞    ::= void main'('')''{'＜复合语句＞'}'
//已经读了 void main 现在指向 main
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
					error("应该是}");//应该是 }
					return;
				}
			}
			else {
				error("应该是{");//应该是 {
				return;
			}
		}
		else {
			error("应该是)");//应该是 )
			return;
		}
	}
	else {
		error("应该是(");//应该是 (
		return;
	}
	enter_midcode(op_end, "", "", "");
	//out_Parser_level();
	//cout << "This is a mainfunction !" << endl;
	Parser_level--;
}

//＜复合语句＞   ::=  ［＜常量说明＞］［＜变量说明＞］＜语句列＞
void compoundStatement() {
	Parser_level++;
	if (symbol == CONST_SY) {
		constDec();
	}
	if (symbol == CHAR_SY || symbol == INT_SY) {
		varDec(false);//非program 推导出
	}
	Statements();//语句列
	//out_Parser_level();
	//cout << "This is a compoundStatement !" << endl;
	Parser_level--;
}
//＜参数表＞    ::= ＜参数＞{,＜参数＞}| ＜空>
void paraList() {
	Parser_level++;
	if (symbol == RS_BACKET) { //如果直接读到 ) 说明没有参数
		//out_Parser_level();
		//cout << "This is a paraList without para !" << endl;
		Parser_level--;
		return;
	}
	if (symbol == INT_SY || symbol == CHAR_SY) {
		getsym();
		if (symbol == ID) {
			enterparalist(new_ID, obj_para, new_type, 0, 0);//注册参数
			getsym();
			while (symbol == COMMA) {
				getsym();
				if (symbol == INT_SY || symbol == CHAR_SY) {
					getsym();
					if (symbol == ID) {
						enterparalist(new_ID, obj_para, new_type, 0, 0);//注册参数
						getsym();
					}
					else {
						error("应该是标识符", RS_BACKET,true);//应该是 标识符
						return;
					}
				}
				else {
					error("应该是INT_SY or CHAR_SY" , RS_BACKET, true);//应该是INT_SY or CHAR_SY
					return;
				}
			}
		}
		else {
			error("应该是标识符", RS_BACKET, true);//应该是标识符
			return;
		}
	}
	else {
		error("应该是参数", RS_BACKET, true);//应该是参数
		return;
	}
	//out_Parser_level();
	//cout << "This is a paraList with para !" << endl;
	Parser_level--;
}
//＜语句列＞   ::= ｛＜语句＞｝
void Statements() {
	Parser_level++;
	while (symbol!=RL_BACKET) {//这里需要注意 所有语句列结束后下一个 symbol 一定是 }
		Statement();
	}
	//out_Parser_level();
	//cout << "This is a Statements !" << endl;
	Parser_level--;
}

//＜语句＞    ::= ＜条件语句＞｜＜循环语句＞| '{'＜语句列＞'}'| ＜有返回值函数调用语句＞; 
//  | ＜无返回值函数调用语句＞; ｜＜赋值语句＞; ｜＜读语句＞; ｜＜写语句＞; ｜＜空＞; | ＜情况语句＞｜＜返回语句＞;
void Statement() {
	Parser_level++;
	if (symbol == IF_SY) {//条件语句
		ifStatement();
	}
	else if (symbol==WHILE_SY) {//循环语句
		whileStatement();
	}
	else if (symbol == LL_BACKET) {//语句列
		getsym();
		Statements();
		if (symbol == RL_BACKET) {
			getsym();
		}
		else {
			error("这里应该是 }");//这里应该是 }
			return;
		}
	}
	else if (symbol==SCANF_SY) {//读语句
		readStatement();
		if (symbol == SEMI) {
			getsym();
		}
		else {
			error("应该是;",0);//应该是;
			return;
		}
	}
	else if (symbol == PRINTF_SY) {//写语句
		writeStatement();
		if (symbol == SEMI) {
			getsym();
		}
		else {
			error("应该是;",0);//应该是;
			return;
		}
	}
	else if (symbol == SEMI) {
		getsym();
		//out_Parser_level();
		//cout << "This is a empty statement !" << endl;
	}
	else if (symbol == SWITCH_SY) {//情况语句
		switchStatement();
	}
	else if (symbol == RETURN_SY) {//返回语句
		returnStatement();
		if (symbol == SEMI) {
			getsym();
		}
		else {
			error("应该是;",0);//应该是;
			return;
		}
	}
	else if (symbol == ID) {
		symbol2 = symbol;//超前读 进行保留
		Token2.assign(Token);//超前读 进行保留
		getsym();
		if (symbol == LS_BACKET) { //有返回 和 无返回值的函数调用
			//getsym();
			callFunction();//已经读了 functionname 和 (
		}
		else {//赋值语句
			assignStatement();//已经读了 标识符
		}

		if (symbol == SEMI) {
			getsym();
		}
		else {
			error("应该是;",0);//应该是;
			return;
		}
	}
	else {
		error("没有符合<语句>的开始符号");//没有符合<语句>的开始符号
		return;
	}
	//out_Parser_level();
	//cout << "This is a  statement !" << endl;
	Parser_level--;
}

//＜表达式＞    ::= ［＋｜－］＜项＞{＜加法运算符＞＜项＞}
bool expression(string &tn) {
	int calmark, value;

	Parser_level++;
	bool int_mark = false;//返回值是否时int
	if (symbol == ADD||symbol==SUB) {
		Sym temp_symbol=symbol;

		getsym();
		term(tn, calmark, value);//第一个 直接传入tn
		if (temp_symbol == ADD) {
			//不做操作
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
bool expression(string &tn, int &calmark, int &value) {//重载experssion 专门用于静态数组下标检测
	Parser_level++;
	bool int_mark = false;//返回值是否时int
	if (symbol == ADD || symbol == SUB) {
		Sym temp_symbol = symbol;
	
		getsym();
		term(tn, calmark, value);//第一个 直接传入tn
		if (temp_symbol == ADD) {
			//不做操作
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
		calmark = 1;//经过运算了
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
//＜项＞     :: = ＜因子＞{ ＜乘法运算符＞＜因子＞ }
bool term(string &tn, int &calmark, int &value) {
	Parser_level++;
	bool int_mark = false;//返回值是否时int
	bool T_or_F=factor(tn, calmark, value);//第一个 直接传入tn
	if (T_or_F == true) {
		int_mark = true;
	}
	while (symbol == MULT || symbol == DIV) {
		Sym temp_symbol = symbol;
		int_mark = true;
		getsym();
		string tn2;

		factor(tn2 ,calmark, value);
		calmark = 1;//经过计算了
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

//＜因子＞    ::= ＜标识符＞｜＜标识符＞'['＜表达式＞']'|'('＜表达式＞')'｜＜整数＞|＜字符＞｜＜有返回值函数调用语句＞ 
bool factor(string &tn, int &calmark,int &value) {//calmark 用于标识是否经过计算 value标识其值
	calmark = 1;

	Parser_level++;
	bool int_mark = false;//返回值是否时int
	if (symbol==SUB) {
		int_mark = true;
		getsym();
		if (symbol == UINT) {//＜整数＞
			num = -num;//值取负
			getsym();
			get_tn(tn);//获取临时变量
			enter_midcode(op_assign, to_string(num), "int", tn);//tn=-num 直接赋值

			value = num;
			calmark = 0;
		}
		else {
			error("应该是uint");//应该是uint
			return false;
		}
	}
	else if (symbol == ADD) {
		int_mark = true;
		getsym();
		if (symbol == UINT) {//＜整数＞
			getsym();
			get_tn(tn);//获取临时变量
			enter_midcode(op_assign, to_string(num), "int", tn);//tn=num 直接赋值

			value = num;
			calmark = 0;
		}
		else {
			error("应该是uint");//应该是uint
			return false;
		}
	}
	else if (symbol == UINT) {
		int_mark = true;
		getsym();
		get_tn(tn);//获取临时变量
		enter_midcode(op_assign, to_string(num), "int", tn);//tn=num 直接赋值

		value = num;
		calmark = 0;
	}
	else if (symbol == CHAR) {//＜字符＞
		get_tn(tn);//获取临时变量
		enter_midcode(op_assign,to_string((int)Token[0]), "char", tn);
		getsym();
	}
	else if (symbol==LS_BACKET) {//'('＜表达式＞')'
		int_mark = true;
		getsym();
		expression(tn);
		if (symbol == RS_BACKET) {
			getsym();
		}
		else {
			error("应该是 )", RS_BACKET, true);//应该是 )
			return false;
		}
	}
	else if (symbol == ID) {
		/*if (findItem(Token) == false) {
			error("标识符未定义");
		 }*/

		symbol2 = symbol;//超前读 进行保留
		Token2.assign(Token);//超前读 进行保留
		getsym();
		if (symbol == LS_BACKET) {//＜有返回值函数调用语句＞
			//getsym();
			/*if (findItem(new_ID) == false) {
				error("标识符未定义");
			}*/
			if (symbolTable[0].maptable.count(new_ID) == 0) {
				error("标识符未定义",0);
				return false;
			}
			else {
				globle_table_item = symbolTable[0].maptable[new_ID];
			}
			if (globle_table_item.obj != obj_function|| globle_table_item.type==type_void) {
				error("非函数 或者 无返回值");
				return false;
			}
			if ( globle_table_item.type == type_int) {
				int_mark = true;
			}
			

			callFunction();//已经读了 functionname 和 (
			get_tn(tn);//获取临时变量
			enter_midcode(op_RETvalue, "", "", tn); //将返回值赋值给 tn=RET
		}
		else if (symbol == LM_BACKET) {//＜标识符＞'['＜表达式＞']'
			string group_ID = new_ID;
			if (findItem(new_ID) == false) {
				error("标识符未定义");
				return false;
			} 
			if (globle_table_item.type != type_int_group&& globle_table_item.type != type_char_group) {
				error("非数组");
				return false;
			}
			if (globle_table_item.type == type_int_group) {
				int_mark = true;
			}
			getsym();
			string tn2;
			get_tn(tn2);//获取一个新的临时变量

			int calmark, value;


			bool T_or_F=expression(tn2, calmark, value);//得到下标值
			if (calmark == 0) {//没经过计算
				findItem(group_ID);
				if (value >= globle_table_item.length||value<0) {//大于数组长度
					error("静态数组下标越界",0);
					//return false;
				}
			}

			if (T_or_F == false) {//
				error("下标不应该时字符类型",0);
				//return false;
			}
		
			if (symbol == RM_BACKET) {
				getsym();
				get_tn(tn);//获取临时变量
				enter_midcode(op_eqaddr, tn2, group_ID, tn); //将返回值赋值给 tn=new_ID[tn2]
			}
			else {
				error("应该是 ]", RM_BACKET, true);//应该是 ]
				//return false;
			}
		}
		else {//＜标识符＞
			//不需getsym() 
			if (findItem(new_ID) == false) {
				error("标识符未定义");
				return false;
			}
			if (globle_table_item.type == type_int) {
				if (globle_table_item.obj == obj_const) {//常量

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
				if (globle_table_item.obj == obj_const) {//字符常量
					enter_midcode(op_assign, to_string(globle_table_item.adr), "char", tn);
				}
				else {
					enter_midcode(op_assign, globle_table_item.name, "", tn);
				}
				
			}
			else {
				error("标识符类型不对",0);
				//return false;
			}
		}
	}
	else {
		error("factor没有合适的开始符号");//没有合适的开始符号
		return false;
	}
	//out_Parser_level();
	//cout << "This is a factor !" << endl;
	Parser_level--;
	return int_mark;
}
//＜条件语句＞  ::=  if '('＜条件＞')'＜语句＞
void ifStatement() {
	Parser_level++;
	if (symbol == IF_SY) {
		getsym();
		if (symbol = LS_BACKET) {//＜条件＞    ::=  ＜表达式＞＜关系运算符＞＜表达式＞｜＜表达式＞
			getsym();
			string tn;
			bool T_or_F_1=expression(tn);
			string temp_lable;//申请lable
			get_lable(temp_lable);
			if (symbol == EQ || symbol == NOTEQ || symbol == LESS || symbol == LESSEQ || symbol == MORE || symbol == MOREEQ) {//tn:tn2
				Sym temp_symbol = symbol;//保留标准
				getsym();
				string tn2;
				bool T_or_F_2=expression(tn2);
				if (T_or_F_1 != T_or_F_2|| T_or_F_1 !=true) {
					error("关系运算符两边只能为int型",0);
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
					error("不可能");
					return;
			
				}
			}
			else {//tn :0
				if (T_or_F_1 == false) {
					error("条件类型错误",0);
					//return;
				}
				enter_midcode(op_NOTEQ, tn, "0", temp_lable);
			}
			if (symbol == RS_BACKET) {
				//string temp_lable;
				//get_lable(temp_lable);
				//enter_midcode(op_BZ, "", "", temp_lable);//不用回填

				getsym();
				Statement();
				enter_midcode(op_lable, "", "", temp_lable);
			}
			else {
				error("应该是 )");//应该是 )
				return;
			}
		}
		else {
			error("应该是 (");//应该是 (
			return;
		}
	}
	else {
		error("应该是 if");//应该是 if
		return;
	}
	//out_Parser_level();
	//cout << "This is a ifStatement !" << endl;
	Parser_level--;
}
//＜循环语句＞   ::=  while '('＜条件＞')'＜语句＞
void whileStatement() {
	Parser_level++;
	string temp_lable1;
	get_lable(temp_lable1);
	//为while 设置特殊lable
	temp_lable1 = "while_" + temp_lable1;


	enter_midcode(op_lable, "", "", temp_lable1);//设置开头lable
	if (symbol == WHILE_SY) {
		getsym();
		if (symbol = LS_BACKET) {//＜条件＞    ::=  ＜表达式＞＜关系运算符＞＜表达式＞｜＜表达式＞
			getsym();
			string tn;
			bool T_or_F_1=expression(tn);
			string temp_lable2;//申请lable
			get_lable(temp_lable2);
			if (symbol == EQ || symbol == NOTEQ || symbol == LESS || symbol == LESSEQ || symbol == MORE || symbol == MOREEQ) {//tn:tn2
				Sym temp_symbol = symbol;//保留标准
				getsym();
				string tn2;
				bool T_or_F_2=expression(tn2);
				if (T_or_F_1 != T_or_F_2|| T_or_F_1 != true) {
					error("关系运算符两边只能为int型",0);
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
					error("条件类型错误",0);
					//return;
				}
				enter_midcode(op_NOTEQ, tn, "0", temp_lable2);
			}
			if (symbol == RS_BACKET) {
				//string temp_lable2;
				//get_lable(temp_lable2);
				//enter_midcode(op_BZ, "", "", temp_lable2);//不用回填 跳到结尾

				getsym();
				Statement();

				enter_midcode(op_GOTO, "", "", temp_lable1);//跳到开头
				enter_midcode(op_lable, "", "", temp_lable2);//设置结尾lable
			}
			else {
				error("应该是 )");//应该是 )
				return;
			}
		}
		else {
			error("应该是 (");//应该是 (
			return;
		}
	}
	else {
		error("应该是while");//应该是while
		return;
	}
	//out_Parser_level();
	//cout << "This is a whileStatement !" << endl;
	Parser_level--;
}


//＜有返回值函数调用语句＞ ::= ＜标识符＞'('＜值参数表＞')'
//＜无返回值函数调用语句＞ ::= ＜标识符＞'('＜值参数表＞')'
//已近读了 函数名 和 ( 现在指向（
void callFunction() {
	string functionName = new_ID;//先将函数名保存起来
	//Token2 = new_ID;//先将函数名保存起来
	//enter_midcode(op_call, "", "", new_ID);//call new_ID
	if (symbolTable[0].maptable.count(new_ID) == 0) {
		error("函数未定义");
		return;
	}
	Table temp_table = symbolTable[symbolTable[0].maptable[new_ID].adr];//获取函数对于的btab
	

	Parser_level++;
	getsym();
	if (symbol == RS_BACKET) {
		if (temp_table.paranum!=0) {
			error("参数太少",0);
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
			error("参数过多",0);
			//return;
		}
		else {
			if ((temp_table.paralist[index_of_para].type == type_char && T_or_F == true)||(temp_table.paralist[index_of_para].type == type_int && T_or_F == false)) {//需要的参数为char 传入的是int
				error("参数类型错误 ",0);
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
				error("参数过多",0);
				//return;
			}
			else {
				if ((temp_table.paralist[index_of_para].type == type_char && T_or_F == true)|| (temp_table.paralist[index_of_para].type == type_int && T_or_F == false)) {//需要的参数为char 传入的是int
					error("参数类型错误",0);
					//return;
				}
				else {
					enter_midcode(op_push, tn2, "", "");
				}
				index_of_para++;
			}
		}
		if (index_of_para < temp_table.paranum) {
			error("参数过少",0);
			//return;
		}
		//out_Parser_level();
		//cout << "This is a valueparalist !" << endl;
		if (symbol == RS_BACKET) {
			getsym();
		}
		else {
			error("应该是)" , RS_BACKET, true);//应该是)
			return;
		}
	}

	enter_midcode(op_call, "", "",functionName);//调用函数
	//out_Parser_level();
	//cout << "This is a callFunction !" << endl;
	Parser_level--;
}

//＜赋值语句＞   ::=  ＜标识符＞＝＜表达式＞|＜标识符＞'['＜表达式＞']'=＜表达式＞
//已经读了 标识符 现在应该指向 =或者 [
void assignStatement() {
	Parser_level++;
	table_item temp_item;
	string tn;
	string ID_name = new_ID;
	if (symbol == LM_BACKET) {
		if (findItem(new_ID)==false) {
			error("标识符未定义");//标识符未定义
			return;
		}
		temp_item = globle_table_item;

		if (globle_table_item.type != type_int_group && globle_table_item.type != type_char_group) {
			error("非数组");
			return;
		}
		getsym();

		int calmark, value;

		bool T_or_F=expression(tn, calmark, value);
		if (calmark == 0) {//未经过计算
			findItem(ID_name);
			if (value >= globle_table_item.length||value<0) {
				//cout << value << endl;
				error("静态数组下标越界",0);
				//return;
			}
		}
		if (T_or_F == false) {//
			error("下标不应该是字符类型",0);
			//return;
		}
		if (symbol == RM_BACKET) {
			
			getsym();
		}
		else {
			error("应该是 ]", RM_BACKET, true);//应该是 ]
			return;
		}
	}
	else {
		if (findItem(new_ID) == false) {
			error("标识符未定义");//标识符未定义
			return;
		}
		temp_item = globle_table_item;
	}
	if (symbol == ASSIGN) {
		if (temp_item.obj == obj_const) {
			error("常量不允许赋值",0);
			//return;
		}
		getsym();
		string tn2;
		bool T_or_F=expression(tn2);//可能会改变 globle_table_item
		if (T_or_F == true && (temp_item.type == type_char || temp_item.type == type_char_group)) {//int不能赋给char
			error("类型错误 char=int",0);
			//return;
		}
		if (T_or_F == false && (temp_item.type == type_int || temp_item.type == type_int_group)) {//char不能赋给int
			error("类型错误 int=char",0);
			//return;
		}
		if (temp_item.type != type_int_group && temp_item.type != type_char_group) {//非数组
			enter_midcode(op_assign, tn2, "", ID_name);
		}
		else {//数组
			enter_midcode(op_addreq, tn, tn2, ID_name);
		}
	}
	else {
		error("应该是 =");//应该是 =
		return;
	}
	//out_Parser_level();
	//cout << "This is a assignStatement !" << endl;
	Parser_level--;
}
//＜读语句＞    ::=  scanf '('＜标识符＞{,＜标识符＞}')'
void readStatement() {
	Parser_level++;
	if (symbol == SCANF_SY) {
		getsym();
		if (symbol == LS_BACKET) {
			do {
				getsym();
				if (symbol == ID) {
					if (findItem(new_ID) == false) {
						error("标识符未定义");
						return;
					}
			
					if (globle_table_item.obj!=obj_var&&(globle_table_item.type!=type_int&& globle_table_item.type != type_char)) {
						error("scanf 中标识符类型出错",0);
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
					error("应该是标识符");//应该是标识符
					return;
				}

			} while (symbol == COMMA);//至少一个标识符
			if (symbol == RS_BACKET) {
				getsym();
			}
			else {
				error("应该是 )", RS_BACKET, true);//应该是 )
				return;
			}
		}
		else {
			error("应该是 (", RS_BACKET, true);//应该是 (
			return;
		}
	}
	else {
		error("应该是 scanf");//应该是 scanf
		return;
	}
	//out_Parser_level();
	///cout << "This is a readStatement !" << endl;
	Parser_level--;
}
//＜写语句＞    ::= printf '(' ＜字符串＞,＜表达式＞ ')'| printf '('＜字符串＞ ')'| printf '('＜表达式＞')'
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
					error("应该是)", RS_BACKET, true);//应该是)
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
					error("应该是)", RS_BACKET, true);//应该是)
					return;
				}
			}
		}
		else {
			error("应该是 (", RS_BACKET, true);//应该是 (
			return;
		}
	}
	else {
		error("应该是是 printf");//应该是是 printf
		return;
	}
	//out_Parser_level();
	//cout << "This is a writeStatement !" << endl;
	Parser_level--;
}
//＜返回语句＞   ::=  return['('＜表达式＞')']  
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
				error("函数返回类型错误",0);
				//return;
			}
			if (symbol == RS_BACKET) {
				getsym();
				enter_midcode(op_ret, "exp", "", tn);
			}
			else {
				error("应该是)", RS_BACKET, true);//应该是)
				return;
			}
		}
		else {
			enter_midcode(op_ret, "null", "", "");
		}
	}
	else {
		error("应该是return");//应该是return
		return;
	}
	//out_Parser_level();
	//cout << "This is a returnStatement !" << endl;
	Parser_level--;
}
//＜情况语句＞  ::=  switch '('＜表达式＞')' '{'＜情况表＞＜缺省＞ '}'   ＜缺省＞   ::=  default : ＜语句＞|＜空＞
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
							error("应该是 冒号");//应该是 冒号
							return;
						}
						enter_midcode(op_lable, "", "", switch_end_lable); //设置switch 的end
						//out_Parser_level();
						//cout << "This is a defaultStatement !" << endl;
					}
					else {
						//enter_midcode(op_lable, "", "", default_lable);//default lable 和 switch_end_lable 在同一个地方
						enter_midcode(op_lable, "", "", switch_end_lable); //设置switch 的end
						//out_Parser_level();
						//cout << "This is a  empty defaultStatement !" << endl;
					}
					if (symbol == RL_BACKET) {
						getsym();
					}
					else {
						error("应该是 }");//应该是 }
						return;
					}
				}
				else {
					error("应该是{");//应该是{
					return;
				}
			}
			else {
				error("应该是)");//应该是)
				return;
			}
		}
		else {
			error("应该是 (");//应该是 (
			return;
		}
	}
	else {
		error("应该是switch");//应该是switch
		return;
	}
	//out_Parser_level();
	//cout << "This is a switchStatement !" << endl;
	Parser_level--;
}
//＜情况表＞   ::=  ＜情况子语句＞{＜情况子语句＞}
//＜情况子语句＞  :: = case＜常量＞：＜语句＞
void situationTable(string tn,string switch_end_lable,bool case_type) {
	Parser_level++;
	string next_lable;

	if (symbol == CASE_SY) {
		do {
			getsym();
			if ( symbol == CHAR) {//常量
				if (case_type == true) {
					error("switch类型错误",0);
					//return;
				}
				get_lable(next_lable);
				enter_midcode(op_EQ, tn,to_string( (int)Token[0]), next_lable);//比较
				//enter_midcode(op_BZ,"","", next_lable);//不满足条件 就跳转
				getsym();
				if (symbol == COLON) {
					getsym();
					Statement();
					enter_midcode(op_GOTO, "", "", switch_end_lable);
					enter_midcode(op_lable, "", "", next_lable);
				}
				else {
					error("应该是 冒号");//应该是 冒号
					return;
				}
			}
			else if (symbol == SUB||symbol==ADD) {
				if (case_type == false) {
					error("switch类型错误",0);
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
					enter_midcode(op_EQ, tn, to_string(num), next_lable);//比较
					//enter_midcode(op_BZ, "", "", next_lable);//不满足条件 就跳转
					getsym();
					if (symbol == COLON) {
						getsym();
						Statement();
						enter_midcode(op_GOTO, "", "", switch_end_lable);
						enter_midcode(op_lable, "", "", next_lable);
					}
					else {
						error("应该是 冒号");//应该是 冒号
						return;
					}
				}
				else {
					error("应该是UINT");//应该是UINT
					return;
				}
			}
			else if(symbol == UINT) {
				if (case_type == false) {
					error("switch类型错误",0);
					//return;
				}
				get_lable(next_lable);
				enter_midcode(op_EQ, tn, to_string(num), next_lable);//比较
				//enter_midcode(op_BZ, "", "", next_lable);//不满足条件 就跳转
				getsym();
				if (symbol == COLON) {
					getsym();
					Statement();
					enter_midcode(op_GOTO, "", "", switch_end_lable);
					enter_midcode(op_lable, "", "", next_lable);
				}
				else {
					error("应该是 冒号");//应该是 冒号
					return;
				}
			}
			else {
				error("应该是常量");//应该是常量
				return;
			}
		} while (symbol == CASE_SY);
	}
	else {
		error("应该是case");//应该是case
		return;
	}
	//out_Parser_level();
	//cout << "This is a situationTable !" << endl;
	Parser_level--;
}