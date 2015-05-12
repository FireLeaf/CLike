/*******************************************************************************
	FILE:		CompDef.h
	
	DESCRIPTTION:	
	
	CREATED BY: YangCao, 2015/05/12

	Copyright (C) - All Rights Reserved with Coconat
*******************************************************************************/

#ifndef __COCONAT_COMPDEF_H_
#define __COCONAT_COMPDEF_H_

enum e_TokenCode
{
	//运算符及分隔符
	TK_PLUS,		//+加号
	TK_MINUS,		//-减号
	TK_STAR,		// * 星号
	TK_DIVIDE,		// /除号
	TK_MOD,			// % 求余
	TK_EQ,			// == 等于
	TK_NEQ,			// != 不等于
	TK_LT,			// < 小于号
	TK_LEQ,			// <= 小于等于
	TK_GT,			// > 大于号
	TK_GEQ,			// >= 大于等于
	TK_ASSIGN,		// = 赋值运算符
	TK_POINTSTO,	// -> 指向结构体成员运算符
	TK_DOT,			// .结构体成员运算符
	TK_AND,			// & 地址、与运算符
	TK_OPENPA,		// ( 左圆括号
	TK_CLOSEPA,		// ) 右圆括号
	TK_OPENBR,		// [ 左中括号
	TK_CLOSEBR,		// ] 右中括号
	TK_BEGIN,		// { 左大括号
	TK_END,			// } 右大括号
	TK_SEMICOLON,	// ; 分号
	TK_COMMA,		// , 逗号
	TK_ELLIPSIS,	// ...省略号
	TK_EOF,			// 文件结束符

	//常量
	TK_CINT,		// 整型常量
	TK_CCHAR,		// 字符常量
	TK_CSTR,		//字符串常量

	//关键字
	KW_CHAR,		// char
	KW_SHORT,		// short
	KW_INT,			// int
	KW_VOID,		// void
	KW_STRUCT,		// struct
	KW_IF,			// if
	KW_ELSE,		// else
	KW_FOR,			// for
	KW_CONTINUE,	// continue
	KW_RETURN,		// return
	KW_SIZEOF,		// sizeof

	KW_ALIGN,		// __align
	KW_CDECL,		// __cdecl
	KW_STDCALL,		// __stdcall

	//标识符
	TK_IDENT,
};

#endif