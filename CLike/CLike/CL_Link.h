/*******************************************************************************
FILE:		CL_Link.h

DESCRIPTTION:

CREATED BY: YangCao, 2016/03/30

Copyright (C) - All Rights Reserved with Coconat
*******************************************************************************/

#ifndef _CL_LINK_H_
#define _CL_LINK_H_

#include "CL_CompDef.h"

class CodeGen;
class Coff;

// @Class PE file linker
class Linker
{
public:
	Linker():ptr_coff(NULL){}

	bool init();
public:
	// @Function load coff file
	// @Return it will return whether load success
	// @Param fname the obj file name
	int load_obj_file(char* fname);

	// @Function load static library
	// @Param name the static library name
	int pe_load_lib_file(char* name);

	// @Function get dll name by libfile
	// @Return dll name
	// @Param libfile the library file(.lib)
	char* get_dllname(char* libfile);

	// @Function get line from static library file, and delete useless character
	// @Return a line character
	// @Param line character buffer
	// @Param size read max character
	// @Param fp the file pointer what be opened static library file
	char *get_line(char* line, int size, FILE* fp);

	// @Function load static library what need be linked
	void add_runtime_libs();
protected:
	// @Function resolve external symbol
	// @Return whether resolve success
	// @Param pe the pe information pointer
	int resolve_coffsyms(PEInfo* pe);

	// @Function find import function
	// @Return the symbol location section index
	// @Param symbol the symbol name
	int pe_find_import(char* symbol);

	// @Function add import function
	// @Return added symbol
	// @Param pe the pe information pointer
	// @Param sym_index the symbol location section index
	// @Param name the symbol name
	ImportSym* pe_add_import(PEInfo* pe, int sym_index, char* name);

	// @Function 
	// @Return ...
	// @Param pe pe information pointer
	int pe_assign_address(PEInfo* pe);

	// @Function calculate memory align position
	// @Param n current unalign position
	DWORD pe_virtual_align(DWORD n);

	// @Function create import information (import directory table and import symbol table)
	// @Param pe the pe information pointer
	void pe_build_imports(PEInfo* pe);

	// @Function write symbol name to import symbol table
	// @Return the symbol in import symbol table offset
	// @Param sec import symbol location section
	// @Param sym symbol name
	int put_import_str(Section * sec, char* sym);

	// @Function relocation symbol address
	void relocate_syms();

	// @Function fix up need relocate code or data address
	void coffrelocs_fixup();

	// @Function write pe file
	// @Param pe pe information pointer
	int pe_write(PEInfo* pe);

	// @Function calculate file align address
	// @Return aligned address
	// @Param n unalign address
	DWORD pe_file_align(DWORD n);

	// @Function set data directory
	// @Param dir directory type
	// @Param addr table rva
	// @Param size table size(byte)
	void pe_set_datadir(int dir, DWORD addr, DWORD size);

	// @Function calculate program entry point
	// @Param pe the pe information pointer
	void get_entry_addr(PEInfo* pe);
public:
	// @Function generate exe file
	// @Param filename the exe name
	int pe_output_file(char * filename);
protected:
	// @Property code gen pointer
	//CodeGen* ptr_codegen;

	// @Property coff pointer
	Coff* ptr_coff;
};

#endif