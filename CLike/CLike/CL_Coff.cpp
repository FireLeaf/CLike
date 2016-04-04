/*******************************************************************************
FILE:		CL_Coff.cpp

DESCRIPTTION:

CREATED BY: YangCao, 2016/03/26

Copyright (C) - All Rights Reserved with Coconat
*******************************************************************************/

#include "CL_Coff.h"
#include "CL_Util.h"
#include "CL_Lex.h"

using namespace Util;

Coff::~Coff()
{
	for (int i = 0; i < static_cast<int>(sections.size()); i++)
	{
		Section* sec = sections[i];
		if (sec->hashtab)
			free(sec->hashtab);
		free(sec->data);
	}
	sections.clear();
}

void Coff::Init()
{
}

void Coff::InitCoff(Lexer* lex, Syntax* syntax)
{
	ptr_lex = lex;
	ptr_syntax = syntax;

	sec_text = section_new(".text", IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_CNT_CODE);
	sec_data = section_new(".data", IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE | IMAGE_SCN_CNT_INITIALIZED_DATA);
	sec_rdata = section_new(".rdata", IMAGE_SCN_MEM_READ | IMAGE_SCN_CNT_INITIALIZED_DATA);
	sec_idata = section_new(".idata", IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE | IMAGE_SCN_CNT_INITIALIZED_DATA);
	sec_bss = section_new(".bss", IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE | IMAGE_SCN_CNT_UNINITIALIZED_DATA);
	sec_rel = section_new(".rel", IMAGE_SCN_LNK_REMOVE | IMAGE_SCN_MEM_READ);
	sec_symtab = new_coffsym_section(".symtab", IMAGE_SCN_LNK_REMOVE | IMAGE_SCN_MEM_READ, ".strtab");
	sec_dynsymtab = new_coffsym_section(".dynsym", IMAGE_SCN_LNK_REMOVE | IMAGE_SCN_MEM_READ, ".dynstr");

	coffsym_add(sec_symtab, "", 0, 0, 0, IMAGE_SYM_CLASS_NULL);
	coffsym_add(sec_symtab, ".data", 0, sec_data->index, 0, IMAGE_SYM_CLASS_STATIC);
	coffsym_add(sec_symtab, ".bss", 0, sec_bss->index, 0, IMAGE_SYM_CLASS_STATIC);
	coffsym_add(sec_symtab, ".rdata", 0, sec_rdata->index, 0, IMAGE_SYM_CLASS_STATIC);
	coffsym_add(sec_dynsymtab, "", 0, 0, 0, IMAGE_SYM_CLASS_NULL);
}

void Coff::write_obj(char* name)
{
	int file_offset = 0;
	FILE* fout = fopen(name, "wb");

	int i = 0, sh_size = 0, nsec_obj = 0;
	
	IMAGE_FILE_HEADER* fh = NULL;

	// reserve image file header + all section header except string talbe and du
	nsec_obj = sections.size() - 2;
	sh_size = sizeof(IMAGE_SECTION_HEADER);
	file_offset = sizeof(IMAGE_FILE_HEADER) + nsec_obj * sh_size;
	fpad(fout, file_offset);

	fh = reinterpret_cast<IMAGE_FILE_HEADER*>(mallocz(sizeof(IMAGE_FILE_HEADER)));
	for (int i = 0; i < nsec_obj; i++)
	{
		Section* sec = reinterpret_cast<Section*>(sections[i]);
		if (sec->data == NULL) continue;
		fwrite(sec->data, 1, sec->data_offset, fout);
		sec->sh.PointerToRawData = file_offset;
		sec->sh.SizeOfRawData = sec->data_offset;
		file_offset += sec->data_offset;
	}
	fseek(fout, SEEK_SET, 0);
	fh->Machine = IMAGE_FILE_MACHINE_I386;
	fh->NumberOfSections = nsec_obj;
	fh->PointerToSymbolTable = sec_symtab->sh.PointerToRawData;
	fh->NumberOfSymbols = sec_symtab->sh.SizeOfRawData / sizeof(CoffSym);
	fwrite(fh, 1, sizeof(IMAGE_FILE_HEADER), fout);

	for (int i = 0; i < nsec_obj; i++)
	{
		Section* sec = reinterpret_cast<Section*>(sections[i]);
		fwrite(sec->sh.Name, 1, sh_size, fout);
	}

	free(fh);
	fclose(fout);
}

Section* Coff::section_new(char* name, int Characteristics)
{
	Section *sec = NULL;
	int initsize = 8;
	sec = reinterpret_cast<Section*>(mallocz(sizeof(Section)));
	if ( sec == NULL )
	{
		return NULL;
	}

	strcpy(reinterpret_cast<char*>(sec->sh.Name), name);
	sec->sh.Characteristics = Characteristics;
	sec->index = sections.size() + 1;
	sec->data = reinterpret_cast<char*>(mallocz(sizeof(char) * initsize));
	sec->data_allocated = initsize;

	if (!(Characteristics & IMAGE_SCN_LNK_REMOVE))
		nsec_image++;

	sections.push_back(sec);
	return sec;
}

void* Coff::section_ptr_add(Section* sec, int increment)
{
	int offset = 0, offset_new = 0;

	offset = sec->data_offset;
	offset_new = offset + increment;

	if (offset_new > sec->data_allocated)
		section_realloc(sec, offset_new);
	
	sec->data_offset = offset_new;
	return sec->data + offset;
}

void Coff::section_realloc(Section* sec, int new_size)
{
	int size = 0;
	char* data = NULL;

	size = sec->data_allocated;
	// increase more than need
	while (size < new_size)
		size = size * 2;

	data = reinterpret_cast<char*>(realloc(sec->data, size));
	if (!data)
		error("memory realloc failed!");

	memset(data + sec->data_allocated, 0, size - sec->data_allocated);
	sec->data = data;
	sec->data_allocated = size;
}

Section* Coff::new_coffsym_section(char* symtab_name, int Characteristics, char* strtab_name)
{
	Section *sec = NULL;
	sec = section_new(symtab_name, Characteristics);
	sec->link = section_new(strtab_name, Characteristics);
	sec->hashtab = reinterpret_cast<int*>(mallocz(sizeof(int) * MAXKEY));
	return sec;
}

void Coff::coffsym_add_update(Symbol* s, int val, int sec_index, short type, char StorageClass)
{
	char *name = NULL;
	CoffSym *cfsym = NULL;
	
	if (s->c == NULL)
	{
		TKWord* word = ptr_lex->get_tkword(s->v);
		name = word->words;//...
		s->c = coffsym_add(sec_symtab, name, val, sec_index, type, StorageClass);
	}
	else // the function declare fisrt and definition in other
	{
		CoffSym* symbols = reinterpret_cast<CoffSym*>(sec_symtab->data);
		cfsym = &symbols[s->c];//
		cfsym->Value = val;
		cfsym->SectionNumber = sec_index;
	}
}

int Coff::coffsym_add(Section* symtab, char* name, int val, int sec_index, short type, char StorageClass)
{
	CoffSym* cfsym = NULL;
	int cs = 0, keyno = 0;
	char* csname = NULL;
	Section* strtab = symtab->link;
	int *hashtab = NULL;
	hashtab = symtab->hashtab;
	cs = coffsym_search(symtab, name);
	if (cs == NULL)
	{
		cfsym = reinterpret_cast<CoffSym*>(section_ptr_add(symtab, sizeof(CoffSym)));
		csname = reinterpret_cast<char*>(coffstr_add(strtab, name));
		cfsym->Name = csname - strtab->data;
		cfsym->Value = val;
		cfsym->SectionNumber = sec_index;
		cfsym->Type = type;
		cfsym->StorageClass = StorageClass;
		keyno = elf_hash(name);
		cfsym->Next = hashtab[keyno];

		cs = cfsym - reinterpret_cast<CoffSym*>(symtab->data);
		hashtab[keyno] = cs;
	}
	return cs;
}

char* Coff::coffstr_add(Section* strtab, char* name)
{
	int len = 0;
	char *pstr = NULL;

	len = strlen(name);
	pstr = reinterpret_cast<char*>(section_ptr_add(strtab, len + 1));
	memcpy(pstr, name, len);
	return pstr;
}

int Coff::coffsym_search(Section* symtab, char* name)
{
	CoffSym *cfsym = NULL;
	int cs = 0, keyno = 0;
	char *csname = NULL;
	Section *strtab = NULL;

	keyno = elf_hash(name);
	strtab = symtab->link;
	cs = symtab->hashtab[keyno];
	// find in the collision list
	while (cs)
	{
		cfsym = (CoffSym*)symtab->data + cs;
		csname = strtab->data + cfsym->Name;

		if (strcmp(csname, name) == 0)
			return cs;

		cs = cfsym->Next;
	}

	return cs;
}

void Coff::coffreloc_add(Section* sec, Symbol* sym, int offset, char type)
{
	int cfsym = 0;
	char *name = NULL;
	if (sym->c == NULL)
		coffsym_add_update(sym, 0, IMAGE_SYM_UNDEFINED, CST_FUNC, IMAGE_SYM_CLASS_EXTERNAL);

	TKWord* word = ptr_lex->get_tkword(sym->v);
	name = word->words;
	cfsym = coffsym_search(sec_symtab, name);
	coffreloc_direct_add(offset, cfsym, sec->index, type);
}

void Coff::coffreloc_direct_add(int offset, int cfsym, char section, char type)
{
	CoffReloc *rel = NULL;
	rel = reinterpret_cast<CoffReloc*>(section_ptr_add(sec_rel, sizeof(CoffReloc)));
	rel->offset = offset;
	rel->cfsym = cfsym;
	rel->section = section;
	rel->type = type;
}