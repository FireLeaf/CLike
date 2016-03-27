/*******************************************************************************
FILE:		CL_Util.cpp

DESCRIPTTION:

CREATED BY: YangCao, 2016/03/15

Copyright (C) - All Rights Reserved with Coconat
*******************************************************************************/

#include "CL_Util.h"
#include "CL_Lex.h"
#include "CL_CompDef.h"

#include <assert.h>

namespace Util{
	//output exception log
	void handle_exception(int stage, int level, const char* fmt, va_list ap)
	{
		char buf[1024];
		vsprintf(buf, fmt, ap);
		if ( STAGE_COMPILE == stage )
		{
			if ( LEVEL_WARNING == level)
			{
				printf("%s(line %d): compile warning : %s!\n", filename, linenum, buf);
			}
			else
			{
				printf("%s(line %d) : compile error : %s!\n", filename, linenum, buf);
				exit(-1);
			}
		}
		else
		{
			printf("link error: %s!\n", buf);
			exit(-1);
		}
	}

	void warning(const char* fmt, ...)
	{
		va_list ap;
		va_start(ap, fmt);
		handle_exception(STAGE_COMPILE, LEVEL_WARNING, fmt, ap);
		va_end(ap);
	}

	void error(const char* fmt, ...)
	{
		va_list ap;
		va_start(ap, fmt);
		handle_exception(STAGE_COMPILE, LEVEL_WARNING, fmt, ap);
		va_end(ap);
	}

	void expect(const char* msg)
	{
		error("miss %s", msg);
	}

	void link_error(const char* fmt, ...)
	{
		va_list ap;
		va_start(ap, fmt);
		handle_exception(STAGE_LINK, LEVEL_ERROR, fmt, ap);
		va_end(ap);
	}

	bool is_nodigit(char c)
	{
		return (c >= 'a' && c <= 'x') || (c >= 'A' && c <= 'Z') || (c == '_');
	}

	bool is_digit(char c)
	{
		return c >= '0' && c <= '9';
	}
	
	void* mallocz(int size)
	{
		void* ptr = malloc(size);
		if (!ptr)
			error("malloc memory failed!");

		memset(ptr, 0, size);
		return ptr;
	}

	int elf_hash(const char* key)
	{
		unsigned int h = 0, g = 0;
		while (*key)
		{
			h = (h << 4) + *key++;
			g = h & 0xf00000000;
			if (g)
				h ^= g >> 24;
			h &= ~g;
		}
		return h % MAXKEY;
	}

	void fpad(FILE* fp, int new_pos)
	{
		int curpos = ftell(fp);
		while (++curpos < new_pos)
		{
			fputc(0, fp);
		}
	}
}