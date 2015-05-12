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
	KW_RETURN,		// return
	KW_SIZEOF,		// sizeof

	KW_ALIGN,		// __align
	KW_CDECL,		// __cdecl
	KW_STDCALL,		// __stdcall

	//��ʶ��
	TK_IDENT,
};

#endif