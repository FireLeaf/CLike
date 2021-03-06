/*******************************************************************************
	FILE:		CompDef.h
	
	DESCRIPTTION:	
	
	CREATED BY: YangCao, 2015/05/12

	Copyright (C) - All Rights Reserved with Coconat
*******************************************************************************/

#ifndef __COCONAT_COMPDEF_H_
#define __COCONAT_COMPDEF_H_

#include <windows.h>
#include <vector>

// @MACRO
#define ALIGN_SET	0x100	// force align mask

// @Macro - use to CoffSym struct Type field
#define CST_FUNC	0x20	// function
#define CST_NOTFUNC	0x00	// no function

#define FUNC_PROLOG_SIZE 9

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
	SC_GLOBAL		= 0x00f0,		// include integer const、character、string、global variable、function definition
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

// @Enum register
enum e_Register
{
	REG_EAX,
	REG_ECX,
	REG_EDX,
	REG_EBX,
	REG_ESP,
	REG_EBP,
	REG_ESI,
	REG_EDI,
	REG_ANY,
};

#define REG_IRET  REG_EAX	// 存放函数返回值的寄存器

// @Enum addressing mode
enum e_AddrForm
{
	ADDR_OTHER,			// register indirect addressing
	ADDR_REG = 3,		// register direct addressing
};

// @Enum output type
enum e_OutType
{
	OUTPUT_OBJ,			// obj file
	OUPUT_EXE,			// execute file
};

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
	KW_BREAK,		// break
	KW_RETURN,		// return
	KW_SIZEOF,		// sizeof

	KW_ALIGN,		// __align
	KW_CDECL,		// __cdecl
	KW_STDCALL,		// __stdcall

	//标识符
	TK_IDENT,
};

struct Symbol;

struct Type
{
	int t;	// data type
	Symbol* ref;	// ref symbol
	Type() : t(0), ref(NULL){}
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

// @Struct section definition
struct Section
{
	int data_offset;			// current data offset
	char *data;					// section data
	int data_allocated;			// need allocate data count
	char index;					// section index
	Section* link;				// related other section,like symbol section relation string section
	int* hashtab;				// hash table, only use to store symbol table
	IMAGE_SECTION_HEADER sh;	// section header
};

// @Struct coff symbol item
struct CoffSym
{
	DWORD Name;					// Symbol name, offset in string table
	DWORD Next;					// hash collision, string 
	DWORD Value;				// 
	short SectionNumber;		// 
	WORD Type;					// 
	BYTE StorageClass;			// 
	BYTE NumberOfAuxSymbols;	// 
};

// @Struct coff relocation
struct CoffReloc
{
	DWORD offset;
	DWORD cfsym;
	BYTE section;
	BYTE type;
};

// @Structure import symbol 
struct ImportSym
{
	int iat_index;
	int thk_offset;
	IMAGE_IMPORT_BY_NAME imp_sym;
};

// @Structure import module info
struct ImportInfo
{
	int dll_index;
	std::vector<ImportSym*> imp_syms;
	IMAGE_IMPORT_DESCRIPTOR imphdr;
};

// @Structure PE information
struct PEInfo
{
	Section* thunk;				// section
	const char* filename;		// PE file name
	DWORD entry_addr;			// the entry point address
	DWORD imp_offs;				//
	DWORD imp_size;				//
	DWORD iat_offs;				// Import address table data offset
	DWORD iat_size;				// Import address table size
	Section** secs;
	int sec_size;
	std::vector<ImportInfo*> imps;// import symbol 

};

#endif