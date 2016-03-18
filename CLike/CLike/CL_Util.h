/*******************************************************************************
	FILE:		CL_Util.h
	
	DESCRIPTTION:	
	
	CREATED BY: YangCao, 2015/05/13

	Copyright (C) - All Rights Reserved with Coconat
*******************************************************************************/

#ifndef __COCONAT_CL_UTIL_H_
#define __COCONAT_CL_UTIL_H_

#include <stdarg.h>

namespace Util{

	void handle_exception(int stage, int level, const char* fmt, va_list ap);

	void warning(const char* fmt, ...);

	void error(const char* fmt, ...);

	void expect(const char* msg);

	void skip(int c);

	const char* get_tkstr(int v);

	void link_error(const char* fmt, ...);


	// not a digit but alphabet or underline
	bool is_nodigit(char c);

	// is a digit
	bool is_digit(char c);
}

#endif