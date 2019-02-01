#pragma once
#include "pch.h"
#include "Parser.h"
#include "Lexer.h"
#include "error.h"
#include <map>
#include <iostream>
#include "midcode.h"

//删除过多的临时变量  利用Tn先声明后使用的性质 Tn不会跨函数 
//程序大部分是顺序和分支结构唯一往回跳是while也会是从上到下的def 和use
//所有可以以一种极简 正确 高效的方式 删除Tn     思想类似于全局的复制传播+删除死代码
//主要矛盾是assign 和各个运算直接的矛盾
//且定义一次使用一次 这条性质可能出错
void delete_Tn();


struct Reference_data {
	map<string, string>  map_reg;//记录变量的全局寄存器
};
extern Reference_data function_Reference_data[500];//记录全局寄存器的分配
void Reference_count();//计算函数内的变量的引用 并为变量分配全局寄存器

void leaf_function();


