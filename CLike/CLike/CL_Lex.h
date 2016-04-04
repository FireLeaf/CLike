/*******************************************************************************
	FILE:		CL_Lex.h
	
	DESCRIPTTION:	
	
	CREATED BY: YangCao, 2015/05/13

	Copyright (C) - All Rights Reserved with Coconat
*******************************************************************************/

#ifndef __COCONAT_CL_LEX_H_
#define __COCONAT_CL_LEX_H_

#include "CL_Hash.h"

class Lexer
{
	friend class Syntax;
public:

	Lexer()
	{
		code_buf = nullptr;
		idx_buf = 0;
		str_token = "";
		cur_ch = 0;
	}

	~Lexer()
	{
		if (code_buf != nullptr)
		{
			free(code_buf);
			code_buf = nullptr;
		}
	}

	static Lexer& instance()
	{
		static Lexer lex;
		return lex;
	}
public:
	// @Function initialize lex
	// @Param file_name the source code file path
	int init_lex(const char* file_name);
	
	// get the word_idx correspond word name string
	const char *get_tkstr(int word_idx);

	// @Function get word
	// @Param word_idx the word index
	TKWord* get_tkword(int word_idx);

	// get a character
	void get_char();

	// go back a character
	void unget_char();

	// preprocess the source code
	void preprocess();

	// get a token
	void get_token();

	// @Function syntax indent
	void syntax_indent();

	// @Function color token
	void color_token(int lex_state);

	// skip a token what is must be need
	void skip(int c);

	// parse the source code comment
	void parse_comment();

	// parse the identifier from source code
	void parse_identifier();

	// parse the string or a character from source code
	void parse_string(char sep);

	// parse the number from source code
	void parse_num();

	// skip the space¡¢table and other needless character
	void skip_white_space();

public:
	TKHashtable	word_table;

	int			code_len;
	char*		code_buf;
	int			idx_buf;
	char		cur_ch;

	char*		str_token;
	//int			token;

	std::string	tkstr;// current word string
	std::string srcstr;// current word source string


};

extern char* filename;
extern int linenum;
extern int token;
extern e_SyntaxState syntax_state;//current syntax state
extern int syntax_level;
extern int tkvalue;

#endif