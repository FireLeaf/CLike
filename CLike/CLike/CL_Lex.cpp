/*******************************************************************************
	FILE:		CL_Lex.cpp
	
	DESCRIPTTION:	
	
	CREATED BY: YangCao, 2015/05/13

	Copyright (C) - All Rights Reserved with Coconat
*******************************************************************************/

#include <stdlib.h>
#include <math.h>

#include "CL_Lex.h"
#include "CL_Hash.h"
#include "CL_CompDef.h"
#include "CL_Util.h"

using namespace Util;

char* filename = "...";
int linenum = 0;
int token = TK_EOF;

e_SyntaxState syntax_state = SNTX_NUL;//current syntax state
int syntax_level = 0;
int tkvalue = 0;

static TKWord keywords[] = {
	{ TK_PLUS, NULL, "+", NULL, NULL },	//+加号
	{ TK_MINUS, NULL, "-", NULL, NULL },	//-减号
	{ TK_STAR, NULL, "*", NULL, NULL },	// * 星号
	{ TK_DIVIDE, NULL, "/", NULL, NULL },	// /除号
	{ TK_MOD, NULL, "%", NULL, NULL },	// % 求余
	{ TK_EQ, NULL, "==", NULL, NULL },	// == 等于
	{ TK_NEQ, NULL, "!=", NULL, NULL },	// != 不等于
	{ TK_LT, NULL, "<", NULL, NULL },	// < 小于号
	{ TK_LEQ, NULL, "<=", NULL, NULL },	// <= 小于等于
	{ TK_GT, NULL, ">", NULL, NULL },	// > 大于号
	{ TK_GEQ, NULL, ">=", NULL, NULL },	// >= 大于等于
	{ TK_ASSIGN, NULL, "=", NULL, NULL },	// = 赋值运算符
	{ TK_POINTSTO, NULL, "->", NULL, NULL },	// -> 指向结构体成员运算符
	{ TK_DOT, NULL, ".", NULL, NULL },	// .结构体成员运算符
	{ TK_AND, NULL, "&", NULL, NULL },	// & 地址、与运算符
	{ TK_OPENPA, NULL, "(", NULL, NULL },	// ( 左圆括号
	{ TK_CLOSEPA, NULL, ")", NULL, NULL },	// ) 右圆括号
	{ TK_OPENBR, NULL, "[", NULL, NULL },	// [ 左中括号
	{ TK_CLOSEBR, NULL, "]", NULL, NULL },	// ] 右中括号
	{ TK_BEGIN, NULL, "{", NULL, NULL },	// { 左大括号
	{ TK_END, NULL, "}", NULL, NULL },	// } 右大括号
	{ TK_SEMICOLON, NULL, ";", NULL, NULL },	// ; 分号
	{ TK_COMMA, NULL, ",", NULL, NULL },	// , 逗号
	{ TK_ELLIPSIS, NULL, "...", NULL, NULL },	// ...省略号
	{ TK_EOF, NULL, "", NULL, NULL },	// 文件结束符

	//常量
	{ TK_CINT, NULL, "整型常量", NULL, NULL },	// 整型常量
	{ TK_CCHAR, NULL, "字符常量", NULL, NULL },	// 字符常量
	{ TK_CSTR, NULL, "字符串常量", NULL, NULL },	//字符串常量

	//关键字
	{ KW_CHAR, NULL, "char", NULL, NULL },	// char
	{ KW_SHORT, NULL, "short", NULL, NULL },	// short
	{ KW_INT, NULL, "int", NULL, NULL },	// int
	{ KW_VOID, NULL, "void", NULL, NULL },	// void
	{ KW_STRUCT, NULL, "struct", NULL, NULL },	// struct
	{ KW_IF, NULL, "if", NULL, NULL },	// if
	{ KW_ELSE, NULL, "else", NULL, NULL },	// else
	{ KW_FOR, NULL, "for", NULL, NULL },	// for
	{ KW_CONTINUE, NULL, "continue", NULL, NULL },	// continue
	{ KW_BREAK, NULL, "break", NULL, NULL}, // break
	{ KW_RETURN, NULL, "return", NULL, NULL },	// return
	{ KW_SIZEOF, NULL, "sizeof", NULL, NULL },	// sizeof

	{ KW_ALIGN, NULL, "__align", NULL, NULL },	// __align
	{ KW_CDECL, NULL, "__cdecl", NULL, NULL },	// __cdecl
	{ KW_STDCALL, NULL, "__stdcall", NULL, NULL },	// __stdcall

	{ 0, NULL, NULL, NULL, NULL },
};

int Lexer::init_lex(const char* file_name)
{
	FILE* fp = fopen(file_name, "rb");
	if (!fp)
	{
		return -1;
	}

	fseek(fp, 0, SEEK_END);
	int len = static_cast<int>(ftell(fp));
	fseek(fp, 0, SEEK_SET);

	code_buf = reinterpret_cast<char*>(malloc((len + 1) * sizeof(char)));
	int read_count = 0;
	while (read_count < len)
	{
		read_count += fread(code_buf + read_count, sizeof(char), len - read_count, fp);
	}

	fclose(fp);

	code_len = read_count;

	TKWord* tp = NULL;

	for (tp = &keywords[0]; tp->words; tp++)
	{
		word_table.direct_insert(tp);
	}
	return 1;
}

void Lexer::skip(int c)
{
	if (token != c)
	{
		error("miss '%s'", get_tkstr(c));
	}
	get_token();
}

const char* Lexer::get_tkstr(int word_idx)
{
//	return word_table.tk_wordtable[word_idx];
	if (word_idx >= TK_CINT && word_idx <= TK_CSTR)
		return srcstr.c_str();
	return
		word_table.get_word_string(word_idx);
}

TKWord* Lexer::get_tkword(int word_idx)
{
	return word_table.tk_wordtable[word_idx];
}

void Lexer::get_char()
{
	if (idx_buf >= code_len)
	{
		cur_ch = EOF;
		return;
	}

	cur_ch = code_buf[idx_buf++];
}

void Lexer::unget_char()
{
	cur_ch = code_buf[--idx_buf];
}

void Lexer::parse_comment()
{
	get_char();
	do
	{
		do 
		{
			if (cur_ch == '\n' || cur_ch == '*' || cur_ch == EOF)
				break;
			else
				get_char();

		} while (1);

		if (cur_ch == '\n')
		{
			linenum++;
			get_char();
		}
		else if (cur_ch == '*')
		{
			get_char();
			if (cur_ch == '/')
			{
				get_char();
				return;
			}
		}
		else
		{
			error("unexcept file end");
			return;
		}
	} while (1);
}

void Lexer::skip_white_space()
{
	while ( cur_ch == ' ' || cur_ch == '\t' || cur_ch == '\r')
	{
		bool is_line_feed = false;
		if ( cur_ch == '\r' )
		{
			get_char();
			if (cur_ch != '\n')
			{
				return;
			}
			is_line_feed = true;
			linenum++;
		}
		if (!is_line_feed)
			printf("%c", cur_ch);
		get_char();
	}
}


void Lexer::preprocess()
{
	while (true)
	{
		if (cur_ch == ' ' || cur_ch == '\t' || cur_ch == '\r')
		{
			skip_white_space();
		}
		else if (cur_ch == '/')
		{
			// pre read a character judgment whether is comment head
			get_char();
			if ( cur_ch == '*' )
			{
				parse_comment();
			}
			else
			{
				unget_char();
				break;
			}
		}
		else
			break;
	}
}

void Lexer::get_token()
{
	preprocess();

	switch ( cur_ch )
	{
	case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
	case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
	case 'o': case 'p': case 'q': case 'r': case 's': case 't':
	case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
	case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
	case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
	case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T':
	case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
	case '_':
		{
			TKWord* tp = NULL;
			parse_identifier();//identify
			tp = word_table.insert(tkstr.c_str());
			token = tp->tkcode;
		}
		break;
	case '0': case '1': case '2':case '3': case '4':
	case '5':case '6': case '7': case '8': case '9':
		{
			parse_num();
			token = TK_CINT;
		}
		break;
	case '+':
		get_char();
		token = TK_PLUS;
		break;
	case '-':
		{
			get_char();
			if (cur_ch == '>')
			{
				token = TK_POINTSTO;
				get_char();
			}
			else
				token = TK_MINUS;
		}
		break;
	case '/':
		token = TK_DIVIDE;
		get_char();
		break;
	case '%':
		token = TK_MOD;
		get_char();
		break;
	case '=':
		get_char();
		if (cur_ch == '=')
		{
			token = TK_EQ;
			get_char();
		}
		else
			token = TK_ASSIGN;
		break;
	case '!':
		{
			get_char();
			if (cur_ch == '=')
			{
				token = TK_NEQ;
				get_char();
			}
			else
				error("nonsupport '!' none operation");
		}
		break;
	case '<':
		{
			get_char();
			if (cur_ch == '=')
			{
				token = TK_LEQ;
				get_char();
			}
			else
				token = TK_LT;
		}
		break;
	case '>':
		{
			get_char();
			if (cur_ch == '=')
			{
				token = TK_GEQ;
				get_char();
			}
			else
				token = TK_GT;
		}
		break;
	case '.':
		get_char();
		if (cur_ch == '.')
		{
			get_char();
			if (cur_ch != '.')
			{
				error("ellipsis spelling error");;
			}
			else
				token = TK_ELLIPSIS;
		}
		else
			token = TK_DOT;
		break;
	case '&':
		token = TK_AND;
		get_char();
		break;
	case ';':
		token = TK_SEMICOLON;
		get_char();
		break;
	case ']':
		token = TK_CLOSEBR;
		get_char();
		break;
	case '}':
		token = TK_END;
		get_char();
		break;
	case ')':
		token = TK_CLOSEPA;
		get_char();
		break;
	case '[':
		token = TK_OPENBR;
		get_char();
		break;
	case '{':
		token = TK_BEGIN;
		get_char();
		break;
	case ',':
		token = TK_COMMA;
		get_char();
		break;
	case '(':
		token = TK_OPENPA;
		get_char();
		break;
	case '*':
		token = TK_STAR;
		get_char();
		break;
	case '\'':
		parse_string(cur_ch);
		token = TK_CCHAR;
		//
		break;
	case '\"':
		parse_string(cur_ch);
		token = TK_CSTR;
		break;
	case EOF:
		token = TK_EOF;
		break;
	default:
		error("unknown character \\x%02x", cur_ch);
		get_char();
		break;
	}

	syntax_indent();
}

void print_tab(int n)
{
	for (int i = 0; i < n; i++)
	{
		printf("\t");
	}
}

void Lexer::syntax_indent()
{
	//return;
	switch (syntax_state)
	{
	case SNTX_NUL:
		color_token(LEX_NORMAL);
		break;
	case SNTX_SP:
		printf(" ");
		color_token(LEX_NORMAL);
		break;
	case SNTX_LF_HT:
		{
			if (token == TK_END)
				syntax_level--;
			printf("\n");
			print_tab(syntax_level);
			color_token(LEX_NORMAL);
		}
		break;
	case SNTX_DELAY:
		break;
	}
	syntax_state = SNTX_NUL;
}
#include <windows.h>
// lex colour
void Lexer::color_token(int lex_state)
{
	HANDLE hHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	char *p;
	switch (lex_state)
	{
	case LEX_NORMAL:
		{
			// set the identifier foreground color is gray
			if (token >= TK_IDENT)
				SetConsoleTextAttribute(hHandle, FOREGROUND_INTENSITY);
			else if (token >= KW_CHAR) // set the keyword foreground color is green
				SetConsoleTextAttribute(hHandle, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
			else if (token > TK_CINT)
				SetConsoleTextAttribute(hHandle, FOREGROUND_RED | FOREGROUND_GREEN);
			else if (token >= TK_PLUS && token <= TK_COMMA)
				SetConsoleTextAttribute(hHandle, FOREGROUND_RED | FOREGROUND_BLUE);
			else
				SetConsoleTextAttribute(hHandle, FOREGROUND_RED | FOREGROUND_INTENSITY);
			p = const_cast<char*>(get_tkstr(token));
			printf("%s", p);
		}
		break;
	case LEX_SEP:
		printf("%c", cur_ch);
		break;
	}
}

void Lexer::parse_identifier()
{
	tkstr = "";

	do 
	{
		tkstr += cur_ch;
		get_char();
	} while ( is_nodigit(cur_ch) || is_digit(cur_ch) );
	
	tkstr += '\0';
}

void Lexer::parse_num()
{
	tkstr = "";
	srcstr = "";

	do 
	{
		tkstr += cur_ch;
		srcstr += cur_ch;
		get_char();
	} while (is_digit(cur_ch));
	
	if ( cur_ch == '.')
	{
		do 
		{
			tkstr += cur_ch;
			srcstr += cur_ch;
		} while (is_digit(cur_ch));
	}

	tkstr += '\0';
	srcstr += '\0';

	long long value = atol(tkstr.c_str());

	tkvalue = static_cast<int>(value);
}

void Lexer::parse_string(char sep)
{
	char c;
	
	tkstr = "";
	srcstr = sep;

	get_char();

	while (1)
	{
		if (cur_ch == sep)// null string
		{
			break;
		}
		else if (cur_ch == '\\')
		{
			srcstr += cur_ch;
			get_char();
			switch (cur_ch)//parse translation letter
			{
			case '0':
				c = '\0';
				break;
			case 'a':
				c = '\a';
				break;
			case 'b':
				c = '\b';
				break;
			case 't':
				c = '\t';
				break;
			case 'n':
				c = '\n';
				break;
			case'v':
				c = '\v';
				break;
			case 'f':
				c = '\f';
				break;
			case 'r':
				c = '\r';
				break;
			case '\"':
				c = '\"';
				break;
			case'\'':
				c = '\'';
				break;
			case '\\':
				c = '\\';
				break;
			default:
				c = cur_ch;
				if (c >= '!' && c <= '~')
				{
					warning("illegal translation character:\'\\%c\'", c);
				}
				else
				{
					warning("illegal translation character:\'\\0x%x\'", c);
				}
				break;
			}
			tkstr += c;
			srcstr += cur_ch;
		}
		else
		{
			tkstr += cur_ch;
			srcstr += cur_ch;
			get_char();
		}
	}

	tkstr += '\0';
	srcstr += sep;
	srcstr += '\0';
	get_char();
}