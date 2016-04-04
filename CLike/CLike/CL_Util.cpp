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
		return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_');
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

	int calc_align(int n, int align)
	{
		return ((n + align - 1) & (align - 1));
	}

	char* get_lib_path()
	{
		// Static library must location in at the same directory with compiler's 'lib' directory
		char path[MAX_PATH];
		char *p = nullptr;
		GetModuleFileNameA(NULL, path, sizeof(path));
		p = strrchr(path, '\\');
		*p = '\0';
		strcat(path, "\\lib\\");
		return strdup(path);
	}

	char* get_file_ext(char * fname)
	{
		char* p = nullptr;
		p = strrchr(fname, '.');
		return p + 1;
	}

	int type_size(Type *t, int *a)
	{
		Symbol *s;
		int bt;
		//pointer type length is 4
		const int PTR_SIZE = 4;

		bt = t->t & T_BTYPE;

		switch (bt)
		{
		case T_STRUCT:
			s = t->ref;
			*a = s->r;
			return s->c;
		case T_PTR:
			if (t->t & T_ARRAY)
			{
				s = t->ref;
				return type_size(&s->type, a) * s->c;
			}
			else
			{
				*a = PTR_SIZE;
				return PTR_SIZE;
			}
		case T_INT:
			*a = 4;
			return 4;
		case T_SHORT:
			*a = 2;
			return 2;
		default://char,void, function
			*a = 1;
			return 1;
		}
	}

	Type *pointed_type(Type *t)
	{
		return &t->ref->type;
	}

	int pointed_size(Type *t)
	{
		int align;
		return type_size(pointed_type(t), &align);
	}
}