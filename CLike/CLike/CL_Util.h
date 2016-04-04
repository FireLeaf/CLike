/*******************************************************************************
	FILE:		CL_Util.h
	
	DESCRIPTTION:	
	
	CREATED BY: YangCao, 2015/05/13

	Copyright (C) - All Rights Reserved with Coconat
*******************************************************************************/

#ifndef __COCONAT_CL_UTIL_H_
#define __COCONAT_CL_UTIL_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "CL_CompDef.h"

namespace Util{

	void handle_exception(int stage, int level, const char* fmt, va_list ap);

	void warning(const char* fmt, ...);

	void error(const char* fmt, ...);

	void expect(const char* msg);

	//const char* get_tkstr(int v);

	void link_error(const char* fmt, ...);


	// not a digit but alphabet or underline
	bool is_nodigit(char c);

	// is a digit
	bool is_digit(char c);

	// @Function malloc memory and clear the memory region
	// @Param size allocate count
	void* mallocz(int size);

	// @Function calculate string hash key
	// @Param key the string
	int elf_hash(const char* key);

	// @Function fill 0
	void fpad(FILE* fp, int new_pos);

	// @Function get type size
	// @Param t data type
	// @Param a align pointer
	int type_size(Type *t, int *a);

	// @Function calculate struct align 
	// @Param n unalign value
	// @Param align
	int calc_align(int n, int align);

	// @Function get file extension name
	char* get_file_ext(char * fname);

	// @Function get library path
	char* get_lib_path();

	/***********************************************************
	* 功能:	返回t所指向的数据类型
	* t:		指针类型
	**********************************************************/
	Type *pointed_type(Type *t);

	/***********************************************************
	* 功能:	返回t所指向的数据类型尺寸
	* t:		指针类型
	**********************************************************/
	int pointed_size(Type *t);
}

#endif