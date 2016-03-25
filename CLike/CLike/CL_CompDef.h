/*******************************************************************************
	FILE:		CompDef.h
	
	DESCRIPTTION:	
	
	CREATED BY: YangCao, 2015/05/12

	Copyright (C) - All Rights Reserved with Coconat
*******************************************************************************/

#ifndef __COCONAT_COMPDEF_H_
#define __COCONAT_COMPDEF_H_

// @MACRO
#define ALIGN_SET 0x100

enum e_ErrorLevel
{
	LEVEL_WARNING,
	LEVEL_ERROR,
};

enum e_WorkStage
{
	STAGE_COMPILE,
	STAGE_LINK,
};

enum e_LexState
{
	LEX_NORMAL,
	LEX_SEP,
};

// @Enum 
enum e_StorageClass
{
	SC_GLOBAL		= 0x00f0,		// include integer const��character��string��global variable��function definition
	SC_LOCAL		= 0x00f1,		// variable in stack
	SC_LLOCAL		= 0x00f2,		// store in stack when register overflow
	SC_CMP			= 0x00f3,		// use flag register
	SC_VALMASK		= 0x00ff,		// storage class type mask(previous classes)
	SC_LVAL			= 0x0100,		// left value
	SC_SYM			= 0x0200,		// symbol

 	SC_ANOM			= 0x10000000,	// anonymous symbol
	SC_STRUCT		= 0x20000000,	// struct symbol
	SC_MEMBER		= 0x40000000,	// struct member symbol
	SC_PARAMS		= 0x80000000,	// function params
};

// @Enum syntax state
enum e_SyntaxState
{
	SNTX_NUL,		// nil state, 
	SNTX_SP,		// space state
	SNTX_LF_HT,		// change line and indent (each declaration.function.)
	SNTX_DELAY,		// confirm output format which delay get next token
};

// @Enum type code
enum e_TypeCode
{
	T_INT		= 0,		// integer
	T_CHAR		= 1,		// character
	T_SHORT		= 2,		// short integer
	T_VOID		= 3,		// void type
	T_PTR		= 4,		// pointer type
	T_FUNC		= 5,		// function type
	T_STRUCT	= 6,		// struct type

	T_BTYPE		= 0x000f,	// basic type mask
	T_ARRAY		= 0x0010,	// array
};

enum e_TokenCode
{
	//��������ָ���
	TK_PLUS,		//+�Ӻ�
	TK_MINUS,		//-����
	TK_STAR,		// * �Ǻ�
	TK_DIVIDE,		// /����
	TK_MOD,			// % ����
	TK_EQ,			// == ����
	TK_NEQ,			// != ������
	TK_LT,			// < С�ں�
	TK_LEQ,			// <= С�ڵ���
	TK_GT,			// > ���ں�
	TK_GEQ,			// >= ���ڵ���
	TK_ASSIGN,		// = ��ֵ�����
	TK_POINTSTO,	// -> ָ��ṹ���Ա�����
	TK_DOT,			// .�ṹ���Ա�����
	TK_AND,			// & ��ַ���������
	TK_OPENPA,		// ( ��Բ����
	TK_CLOSEPA,		// ) ��Բ����
	TK_OPENBR,		// [ ��������
	TK_CLOSEBR,		// ] ��������
	TK_BEGIN,		// { �������
	TK_END,			// } �Ҵ�����
	TK_SEMICOLON,	// ; �ֺ�
	TK_COMMA,		// , ����
	TK_ELLIPSIS,	// ...ʡ�Ժ�
	TK_EOF,			// �ļ�������

	//����
	TK_CINT,		// ���ͳ���
	TK_CCHAR,		// �ַ�����
	TK_CSTR,		//�ַ�������

	//�ؼ���
	KW_CHAR,		// char
	KW_SHORT,		// short
	KW_INT,			// int
	KW_VOID,		// void
	KW_STRUCT,		// struct
	KW_IF,			// if
	KW_ELSE,		// else
	KW_FOR,			// for
	KW_CONTINUE,	// continue
	KW_BREAK,		// break
	KW_RETURN,		// return
	KW_SIZEOF,		// sizeof

	KW_ALIGN,		// __align
	KW_CDECL,		// __cdecl
	KW_STDCALL,		// __stdcall

	//��ʶ��
	TK_IDENT,
};

struct Symbol;

struct Type
{
	e_TypeCode t;	// data type
	Symbol* ref;	// ref symbol
};

// @Structure
struct Symbol
{
	int v;				// symbol word code
	int r;				// symbol related register
	int c;				// symbol related value
	Type type;			// symbol data type
	Symbol *next;		// other related symbol 
	Symbol *prev_tok;	// pointer the previous same name symbol 
};

#endif