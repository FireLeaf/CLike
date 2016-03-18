#include <stdio.h>
#include <windows.h>

#include "CL_CompDef.h"
#include "CL_Lex.h"

Lexer lex;

// lex colour
void color_token(int lex_state)
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
			p = const_cast<char*>(lex.get_tkstr(token));
			printf("%s", p);
		}
		break;
	case LEX_SEP:
		printf("%c", lex.cur_ch);
		break;
	}
}

int main(int argc, char** argv)
{
	lex.init_lex();

	FILE* fp = fopen("CL_Util.cpp", "rb");
	if (!fp)
	{
		return -1;
	}

	fseek(fp, 0, SEEK_END);
	int len = static_cast<int>(ftell(fp));
	fseek(fp, 0, SEEK_SET);

	char* code_buf = reinterpret_cast<char*>(malloc( (len + 1) * sizeof(char)));
	int read_count = 0;
	while (read_count < len)
	{
		read_count += fread(code_buf + read_count, sizeof(char), len - read_count, fp);
	}

	lex.code_len = read_count;
	lex.code_buf = code_buf;

	lex.get_char();
	do 
	{
		lex.get_token();
		color_token(LEX_NORMAL);
	} while (token != TK_EOF);

	getchar();

	return 0;
}