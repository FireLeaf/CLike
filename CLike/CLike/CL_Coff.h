/*******************************************************************************
FILE:		CL_Coff.h

DESCRIPTTION:

CREATED BY: YangCao, 2016/03/26

Copyright (C) - All Rights Reserved with Coconat
*******************************************************************************/

#ifndef _CL_COFF_H_
#define _CL_COFF_H_

#include <vector>

#include "CL_CompDef.h"

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

class CodeGen;

// @Class COFF file generate
class Coff
{
	friend class CodeGen;
public:
	Coff() : ptr_lex(NULL), ptr_syntax(NULL), nsec_image(0), sec_text(NULL), sec_data(NULL),
			 sec_bss(NULL), sec_idata(NULL), sec_rdata(NULL), sec_rel(NULL), sec_symtab(NULL),
			 sec_dynsymtab(NULL){}

	~Coff();

	// @Function initialized coff
	// @Param lex Lexer
	// @Param syntax Syntax
	void InitCoff(Lexer* lex, Syntax* syntax);

	// @Function write to file
	// @Param name file name
	void write_obj(char* name);

public:
	// @Function create a new section
	// @Return the created section pointer
	// @Param name section name
	// @Param Characteristics section sign
	Section* section_new(char* name, int Characteristics);

	// @Function reserve at less 'increment' size memory to section data
	// @Return reserve memory pointer
	// @Param sec the section
	// @Param increment need increase data length
	void* section_ptr_add(Section* sec, int increment);

	// @Function realloc the section memory
	// @Param sec the section
	// @Param new_size the section new size
	void section_realloc(Section* sec, int new_size);
public:

	// @Function create coff symbol section
	// @Return the symbol section pointer
	// @Param symtab_name symbol table name
	// @Param Characteristics the section sign
	// @Param strtab_name related string table name
	Section *new_coffsym_section(char* symtab_name, int Characteristics, char* strtab_name);

	// @Function add coff symbol
	// @Return index where the symbol in the coff symbol table
	// @Param symtab the coff symbol section
	// @Param name the symbol name
	// @Param val the symbol related value
	// @Param sec_index the section index what define the symbol
	// @Param type coff symbol type
	// @Param StorageClass coff symbol storage class
	int coffsym_add(Section* symtab, char* name, int val, int sec_index, short type, char StorgeClass);

	// @Function add or update( only apply to function declare first and definition in other) coff symbol, 
	// @Param s the symbol
	// @Param val the symbol related value
	// @Param sec_index the the section index what define the symbol
	// @Param type coff symbol type
	// @Param StorageClass coff symbol storage class
	void coffsym_add_update(Symbol* s, int val, int sec_index, short type, char StorageClass);

	// @Function add coff symbol name
	// @Return added name string
	// @Param strtab the string section what save coff string table
	// @Param name symbol name string
	char* coffstr_add(Section* strtab, char* name);

	// @Function find coff symbol by symbol name
	// @Return the symbol index in symbol table
	// @Param symtab the symbol section
	// @Param name symbol name
	int coffsym_search(Section* symtab, char* name);

public:
	// @Function add relocation item
	// @Param section where the symbol in section
	// @Param sym symbol
	// @Param offset the offset which need to relocated code and data int their section
	// @Param type relocation type
	void coffreloc_add(Section* sec, Symbol* sym, int offset, char type);

	// @Function add relocation information
	// @Param offset the offset which need to relocated code and data int their section
	// @Param cfsym symbol table index
	// @Param section symbol where locate section
	// @Param type relocation type
	void coffreloc_direct_add(int offset, int cfsym, char section, char type);
protected:

	// @Property
	Lexer* ptr_lex;			// lex pointer
	Syntax* ptr_syntax;		// syntax pointer

	// @Property all sections
	std::vector < Section* > sections;

	// @Property current section image count
	int nsec_image;

	// Property some section
	Section* sec_text;		// code section
	Section* sec_data;		// initialized data section
	Section* sec_bss;		// uninitialized data section
	Section* sec_idata;		// input table section
	Section* sec_rdata;		// read only data section
	Section* sec_rel;		// relocation section
	Section* sec_symtab;	// coff Symbol section
	Section* sec_dynsymtab;	// dynamic link symbol section
};

#endif