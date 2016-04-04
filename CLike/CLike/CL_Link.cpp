/*******************************************************************************
FILE:		CL_Link.cpp

DESCRIPTTION:

CREATED BY: YangCao, 2016/03/30

Copyright (C) - All Rights Reserved with Coconat
*******************************************************************************/

#include "CL_Link.h"
#include "CL_Util.h"
#include "CL_Coff.h"

using namespace Util;

extern std::vector< char* > array_dll;
extern std::vector < char* > array_lib;
extern char* entry_symbol;
extern short subsystem;
extern char* lib_path;


// @Variable pe file dos header 
IMAGE_DOS_HEADER dos_header = {
	0x5a4d,/*WORD e_maigc;				dos header magic number, 'MZ'*/
	0x0090,/*WORD e_cblp;				Bytes on last page of file*/
	0x0003,/*WORD e_cp;					Pages in file*/
	0x0000,/*WORD e_crlc;				Relocations*/

	0x0004,/*WORD e_cparhdr;			Size of header in paragraphs*/
	0x0000,/*WORD e_minalloc;			Minimum extra paragraphs needed*/
	0xffff,/*WORD e_minalloc;			Maximum extra paragraphs needed*/

	0x0000,/*WORD e_ss;					stack segment initialize value*/
	0x00B8,/*WORD e_sp;					stack pointer initialize value*/
	0x0000,/*WORD e_csum;				Checksum*/
	0x0000,/*WORD e_ip;					dos programm instruction pointer*/
	0x0000,/*WORD e_cs;					dos programm code segment inttialize value*/
	0x0040,/*WORD e_lfarlc;				File address of relocation table*/
	0x0000,/*WORD e_ovno;				Overlay number*/
	{ 0, 0, 0, 0 },/*WORD e_res[4];		Reserve words*/
	0x0000,/*WORD e_oemid;				OEM identifier(for e_oeminfo)*/
	0x0000,/*WORD e_oeminfo;			OEM information;*/
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },/*WORD e_res2[10];	reserve words*/
	0x00000080/*DWORD e_lfanew;			pointer PE Header(NT header)*/
};

// @Constant ms dos-stub 
const unsigned char dos_stub[0x40] = {
	/* 14 code bytes + This program cannot be run inDOS mode.\n$' + 7 * 0x00*/
	0x0e, 0x1f, 0xba, 0x0e, 0x00, 0xb4, 0x09, 0xcd, 0x21, 0xb8, 0x01, 0x4c, 0xcd, 0x21,
	0x54, 0x68, 0x69, 0x73, 0x20, 0x70, 0x72, 0x6f, 0x67, 0x72, 0x61, 0x6d, 0x20, 0x63,
	0x61, 0x6e, 0x6e, 0x6f, 0x74, 0x20, 0x62, 0x65, 0x20, 0x72, 0x75, 0x6e, 0x20, 0x69,
	0x6e, 0x20, 0x44, 0x4f, 0x53, 0x20, 0x6d, 0x6f, 0x64, 0x65, 0x2e, 0x0d, 0x0d, 0x0a, 0x24,

	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// @Variable nt header
IMAGE_NT_HEADERS32 nt_header = {
	0x00004550,			/* DWORD Signature = IMAGE_NT_SIGNATURE PE file singature*/
	{
		// IMAGE_FILE_HEADER	FileHeader;
		0x014C,			/*WORD Machine;	run machine platform*/
		0x0003,			/*WORD NumberOfSections;*/
		0x00000000,		/*DWORD TimeDateStamp; pe file create time*/
		0x00000000,		/*DWORD PointerToSymbolTable; symbol table */
		0x00000000,		/*DWORD NumberOfSymbols; symbol count*/
		0x00E0,			/*WORD SizeOfOptionalHeader; optional header structure size*/
		0x030F			/*WORD Characteristics; file attribute*/
	},
	{
		// IMAGE_OPTIONAL_HEADER OptionalHeader;
		// standard fields
		0x010B,			/*WORD Magic;	magic number*/
		0x06,			/*BYTE MajorLinkerVersion;*/
		0x00,			/*BYTE MinorLinkerVersion;*/
		0x00000000,		/*DWORD SizeofCode;*/
		0x00000000,		/*DWORD SizeOfInitializedData;*/
		0x00000000,		/*DWORD SizeOfUninitializedData;*/
		0x00000000,		/*DWORD AddressOfEntryPoint; programm entry point rva*/
		0x00000000,		/*DWORD BaseOfCode; code segment start RVA*/
		0x00000000,		/*DWORD BaseOfData; data segment start RVA*/

		// NT extra fields
		0x00400000,		/*DWORD ImageBase; suggest image load to virtual address*/
		0x00001000,		/*DWORD SectionAlignment; section alignment in memory*/
		0x00000200,		/*DWORD FileAlignment; section alignment in file*/
		0x0004,			/*WORD MajorOperatingSystemVersion;*/
		0x0000,			/*WORD MinorOperatingSystemVersion;*/
		0x0000,			/*WORD MajorImageVersion; major programm version*/
		0x0000,			/*WORD MinorImageVersion; minor programm version*/
		0x0004,			/*WORD MajorSubsystemVersion;*/
		0x0000,			/*WORD MinorSubsystemVersion;*/
		0x00000000,		/*DWORD Win32VersionValue; reserve 0*/
		0x00000000,		/*DWORD SizeOfImage; the pe image size in memory*/
		0x00000200,		/*DWORD SizeOfHeader; all section header and section table size*/
		0x00000000,		/*DWORD CheckSum;*/
		0x0003,			/*WORD Subsystem;  file subsystem*/
		0x0000,			/*WORD DllCharacteristic;*/
		0x00100000,		/*DWORD SizeOfStackReserve; reserve stack size in inilitialize*/
		0x00001000,		/*DWORD SizeOfStackCommit; commit stack size in inilitialize*/
		0x00100000,		/*DWORD SizeOfHeapReserve;*/
		0x00001000,		/*DWORD SizeofHeapCommit;*/
		0x00000000,		/*DWORD LoaderFlags; reserve*/
		0x00000010,		/*DWORD NumberOfRvaAndSizes; data directory count*/

		// IMAGE_DATA_DIRECTORY DataDirectory[16]; 
		{
			{ 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
			{ 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
		}
	},
};

bool Linker::init()
{
	ptr_coff = new Coff;
	return true;
}

int Linker::load_obj_file(char* fname)
{
	IMAGE_FILE_HEADER fh = { 0 };
	ptr_coff->InitCoff(NULL, NULL);
	std::vector< Section* >& secs = ptr_coff->sections;
	int sh_size = 0, nsec_obj = 0, nsym = 0;
	FILE *fobj = NULL;
	CoffSym* cfsyms = NULL;
	CoffSym* cfsym = NULL;
	char * strs = NULL, *csname = NULL;
	void *ptr = NULL;
	int cur_text_offset = 0;
	int cur_rel_offset = 0;
	int *old_to_new_syms;
	int cfsym_index;
	CoffReloc *rel = NULL, *rel_end = NULL;

	sh_size = sizeof(IMAGE_SECTION_HEADER);
	fobj = fopen(fname, "rb");
	if (!fobj)
		return 0;

	fread(&fh, 1, sizeof(IMAGE_FILE_HEADER), fobj);
	nsec_obj = fh.NumberOfSections;

	for (int i = 0; i < nsec_obj; i++)
	{
		fread(secs[i]->sh.Name, 1, sh_size, fobj);
	}

	cur_text_offset = ptr_coff->sec_text->data_offset;
	cur_rel_offset = ptr_coff->sec_rel->data_offset;

	for (int i = 0; i < nsec_obj; i++)
	{
		if (!strcmp(reinterpret_cast<const char*>(secs[i]->sh.Name), ".symtab"))
		{
			cfsyms = reinterpret_cast<CoffSym*>(mallocz(secs[i]->sh.SizeOfRawData));
			fseek(fobj, SEEK_SET, secs[i]->sh.PointerToRawData);
			fread(cfsyms, 1, secs[i]->sh.SizeOfRawData, fobj);
			nsym = secs[i]->sh.SizeOfRawData / sizeof(CoffSym);
			continue;
		}
		if (!strcmp(reinterpret_cast<const char*>(secs[i]->sh.Name), ".strtab"))
		{
			strs = reinterpret_cast<char*>(mallocz(secs[i]->sh.SizeOfRawData));
			fseek(fobj, SEEK_SET, secs[i]->sh.PointerToRawData);
			fread(strs, 1, secs[i]->sh.SizeOfRawData, fobj);
			continue;
		}

		if (!strcmp(reinterpret_cast<const char*>(secs[i]->sh.Name), ".dynsym")
			|| !strcmp(reinterpret_cast<const char*>(secs[i]->sh.Name), ".dynstr"))
		{
			continue;
		}

		fseek(fobj, SEEK_SET, secs[i]->sh.PointerToRawData);
		ptr = ptr_coff->section_ptr_add(secs[i], secs[i]->sh.SizeOfRawData);
		fread(ptr, 1, secs[i]->sh.SizeOfRawData, fobj);
	}

	old_to_new_syms = reinterpret_cast<int*>(mallocz(sizeof(int)* nsym));

	for (int i = 0; i < nsym; i++)
	{
		cfsym = &cfsyms[i];
		csname = strs + cfsym->Name;
		cfsym_index = ptr_coff->coffsym_add(ptr_coff->sec_symtab, csname, cfsym->Value
								, cfsym->SectionNumber, cfsym->Type, cfsym->StorageClass);
		old_to_new_syms[i] = cfsym_index;
	}

	rel = reinterpret_cast<CoffReloc*>(ptr_coff->sec_rel->data + cur_rel_offset);
	rel_end = reinterpret_cast<CoffReloc*>(ptr_coff->sec_rel->data + ptr_coff->sec_rel->data_offset);

	for (; rel < rel_end; rel++)
	{
		cfsym_index = rel->cfsym;
		rel->cfsym = old_to_new_syms[cfsym_index];
		rel->offset += cur_text_offset;
	}

	free(cfsyms);
	free(strs);
	free(old_to_new_syms);
	fclose(fobj);
	return 1;
}

int Linker::pe_load_lib_file(char* name)
{
	char libfile[MAX_PATH];
	int ret = -1;
	char line[400], *dllname = NULL, *p = NULL;
	FILE *fp = NULL;
	sprintf_s(libfile, "%s%s.slib", lib_path, name);
	fp = fopen(libfile, "rb");
	if (fp)
	{
		dllname = get_dllname(libfile);
		array_dll.push_back(dllname);//...
		while (true)
		{
			p = get_line(line, sizeof(line), fp);
			if (p == NULL)
				break;
			if (0 == *p || ';' == *p)
				continue;
			ptr_coff->coffsym_add(ptr_coff->sec_dynsymtab, p, array_dll.size()
								, ptr_coff->sec_text->index, CST_FUNC, IMAGE_SYM_CLASS_EXTERNAL);
		}
		ret = 0;
		fclose(fp);
	}
	else
	{
		link_error("open file failed : \"%s\"", libfile);
	}
	return ret;
}

char* Linker::get_dllname(char* libfile)
{
	int n1 = 0, n2 = 0;
	char *libname = NULL, *dllname = NULL, *p = NULL;
	n1 = strlen(libfile);
	libname = libfile;
	for (p = libfile + n1; n1 > 0; p--)
	{
		if (*p == '\\')
		{
			libname = p + 1;
			break;
		}
	}

	n2 = strlen(libname);
	dllname = reinterpret_cast<char*>(mallocz(sizeof(char)* n2));
	memcpy(dllname, libname, n2 - 4);
	memcpy(dllname + n2 - 4, "dll", 3);
	return dllname;
}

char* Linker::get_line(char* line, int size, FILE* fp)
{
	char *p = NULL, *p1 = NULL;
	if (NULL == fgets(line, size, fp))
	{
		return NULL;
	}

	// delete left blank
	p = line;
	while (*p && isspace(*p))
		p++;
	//delete right blank and enter character
	p1 = strchr(p, '\0');
	while (p1 > p && p1[-1] <= ' ')
		p1--;
	*p1 = '\0';
	return p;
}

void Linker::add_runtime_libs()
{
	int i = 0, pe_type = 0;

	for (int i = 0; i < static_cast<int>(array_lib.size()); i++)
	{
		pe_load_lib_file(array_lib[i]);
	}
}

int Linker::resolve_coffsyms(PEInfo* pe)
{
	CoffSym * sym = NULL;
	int sym_index = 0, sym_end = 0;
	int ret = 0;
	int found = 1;

	sym_end = ptr_coff->sec_symtab->data_offset / sizeof(CoffSym);
	for (sym_index = 1; sym_index < sym_end; sym_index++)
	{
		sym = reinterpret_cast<CoffSym*>(ptr_coff->sec_symtab) + sym_index;
		if (sym->SectionNumber  == IMAGE_SYM_UNDEFINED)
		{
			char* name = ptr_coff->sec_symtab->link->data + sym->Name;
			unsigned int type = sym->Type;
			int imp_sym = pe_find_import(name);
			ImportSym *is = NULL;
			if ( 0 == imp_sym)
			{
				found = 0;
			}
			is = pe_add_import(pe, imp_sym, name);

			if (!is)
				found = 0;

			if (found && type == CST_FUNC)
			{
				int offset = is->thk_offset;
				char buffer[100];
				offset = ptr_coff->sec_text->data_offset;
				// FF /4 JMP r/m32 Jump near, absolute indirect, address given in r/m32
				*reinterpret_cast<WORD*>(ptr_coff->section_ptr_add(ptr_coff->sec_text, 6)) = 0x25ff;

				sprintf_s(buffer, "IAT.%s", name);
				is->iat_index = ptr_coff->coffsym_add(ptr_coff->sec_symtab, buffer, 0, ptr_coff->sec_idata->index
					, CST_FUNC, IMAGE_SYM_CLASS_EXTERNAL);
				ptr_coff->coffreloc_direct_add(offset + 2, is->iat_index, ptr_coff->sec_text->index, IMAGE_REL_I386_DIR32);

				is->thk_offset = offset;

				sym = reinterpret_cast<CoffSym*>(ptr_coff->sec_symtab->data + sym_index);
				sym->Value = offset;
				sym->SectionNumber = ptr_coff->sec_text->index;
			}
			else
			{
				link_error("'%s' undefined", name);
				ret = 1;
			}
		}
	}

	return ret;
}

int Linker::pe_find_import(char* symbol)
{
	int sym_index = 0;
	sym_index = ptr_coff->coffsym_search(ptr_coff->sec_dynsymtab, symbol);
	return sym_index;
}

ImportSym* Linker::pe_add_import(PEInfo* pe, int sym_index, char* name)
{
	int i = 0, dll_index = 0;
	ImportInfo *p = NULL;
	ImportSym * s = NULL;

	dll_index = (reinterpret_cast<CoffSym*>(ptr_coff->sec_dynsymtab->data) + sym_index)->Value;
	if (0 == dll_index)
		return NULL;

	for (int info_idx = 0; info_idx < static_cast<int>(pe->imps.size()); info_idx++)
	{
		if (pe->imps[info_idx]->dll_index == dll_index)
		{
			p = pe->imps[info_idx];
		}
	}
	if (!p)
	{
		p = new ImportInfo;
		p->dll_index = dll_index;
		pe->imps.push_back(p);
	}

	
	
	for (int info_idx = 0; info_idx < static_cast<int>(p->imp_syms.size()); info_idx++)
	{
		if (p->imp_syms[info_idx]->iat_index == sym_index)
		{
			return p->imp_syms[info_idx];
		}
	}

	s = reinterpret_cast<ImportSym*>(mallocz(sizeof(ImportSym) + strlen(name)));
	strcpy(reinterpret_cast<char*>(&s->imp_sym.Name), name);
	p->imp_syms.push_back(s);

	return s;
}

int Linker::pe_assign_address(PEInfo* pe)
{
	int i = 0;
	DWORD addr = 0;
	Section* sec = NULL, **ps = NULL;
	
	pe->thunk = ptr_coff->sec_idata;

	pe->secs = reinterpret_cast<Section**>(mallocz(ptr_coff->nsec_image * sizeof(Section*)));
	addr = nt_header.OptionalHeader.ImageBase + 1;

	for (i = 0; i < ptr_coff->nsec_image; i++)
	{
		sec = ptr_coff->sections[i];
		ps = &(pe->secs[pe->sec_size]);
		*ps = sec;
		sec->sh.VirtualAddress = pe_virtual_align(addr);
	
		if (sec == pe->thunk)
		{
			pe_build_imports(pe);
		}

		if (sec->data_offset)
		{
			addr += sec->data_offset;
			++pe->sec_size;
		}
	}

	return 0;
}

DWORD Linker::pe_virtual_align(DWORD n)
{
	DWORD SectionAlignment = nt_header.OptionalHeader.SectionAlignment;
	return calc_align(n, SectionAlignment);
}

void Linker::pe_build_imports(PEInfo* pe)
{
	int thk_ptr = 0, ent_ptr = 0, dll_ptr = 0, sym_cnt = 0, i = 0;
	DWORD rva_base = pe->thunk->sh.VirtualAddress - nt_header.OptionalHeader.ImageBase;
	int ndlls = pe->imps.size();

	for (sym_cnt = i = 0; i < ndlls; i++)
		sym_cnt += pe->imps[i]->imp_syms.size();

	if (0 == sym_cnt)
		return;
	
	pe->imp_offs = dll_ptr = pe->thunk->data_offset;
	pe->imp_size = (ndlls + 1) * sizeof(IMAGE_IMPORT_DESCRIPTOR);
	pe->iat_offs = dll_ptr + pe->imp_size;
	pe->iat_size = (sym_cnt + ndlls) * sizeof(DWORD);

	// Reserve dll export table and iat table space
	ptr_coff->section_ptr_add(pe->thunk, pe->imp_size + 2 * pe->iat_size);

	thk_ptr = pe->iat_offs;
	ent_ptr = pe->iat_offs + pe->iat_size;

	for (i = 0; i < static_cast<int>(pe->imps.size()); i++)
	{
		int k, n, v;
		ImportInfo * p = pe->imps[i];
		char* name = array_dll[p->dll_index - 1];

		// write dll name
		v = put_import_str(pe->thunk, name);

		p->imphdr.FirstThunk = thk_ptr + rva_base;
		p->imphdr.OriginalFirstThunk = ent_ptr + rva_base;
		p->imphdr.Name = v + rva_base;
		memcpy(pe->thunk->data + dll_ptr, &p->imphdr, sizeof(IMAGE_IMPORT_DESCRIPTOR));

		for (k = 0, n = p->imp_syms.size(); k < n; ++k)
		{
			if (k < n)
			{
				ImportSym * is = p->imp_syms[k];
				DWORD iat_index = is->iat_index;
				CoffSym* org_sym = reinterpret_cast<CoffSym*>(ptr_coff->sec_symtab) + iat_index;

				org_sym->Value = thk_ptr;
				org_sym->SectionNumber = pe->thunk->index;
				v = reinterpret_cast<int>(pe->thunk->data + rva_base);

				ptr_coff->section_ptr_add(pe->thunk, sizeof(is->imp_sym.Hint));
				put_import_str(pe->thunk, is->imp_sym.Name);
			}
			else
			{
				v = 0;
			}

			*reinterpret_cast<DWORD*>(pe->thunk->data + thk_ptr) =
			*reinterpret_cast<DWORD*>(pe->thunk->data + ent_ptr) = static_cast<DWORD>(v);
			thk_ptr += sizeof(DWORD);
			ent_ptr += sizeof(DWORD);
		}

		dll_ptr += sizeof(IMAGE_IMPORT_DESCRIPTOR);
		for (int sym_index = 0; sym_index < static_cast<int>(p->imp_syms.size()); sym_index++)
		{
			delete p->imp_syms[sym_index];
		}
		p->imp_syms.clear();
	}
	for (int imps_index = 0; imps_index < static_cast<int>(pe->imps.size()); imps_index++)
	{
		delete pe->imps[imps_index];
	}
	pe->imps.clear();
}

int Linker::put_import_str(Section * sec, char* sym)
{
	int offset = 0, len = 0;
	char* ptr = nullptr;
	len = strlen(sym) + 1;
	offset = sec->data_offset;
	ptr = reinterpret_cast<char*>(ptr_coff->section_ptr_add(sec, len));
	memcpy(ptr, sym, len);
	return offset;
}

void Linker::relocate_syms()
{
	CoffSym *sym, *sym_end;
	Section *sec;
	sym_end = reinterpret_cast<CoffSym*>(ptr_coff->sec_symtab->data + ptr_coff->sec_symtab->data_offset);
	for (sym = reinterpret_cast<CoffSym*>(ptr_coff->sec_symtab->data_offset) + 1;
		sym < sym_end; sym++
		)
	{
		sec = ptr_coff->sections[sym->SectionNumber - 1];
		sym->Value += sec->sh.VirtualAddress;
	}
}

void Linker::coffrelocs_fixup()
{
	Section *sec, *sr;
	CoffReloc *rel, *rel_end, *qrel;
	CoffSym *sym;
	int type, sym_index;
	char *ptr;
	unsigned long val, addr;
	char *name;

	sr = ptr_coff->sec_rel;
	rel_end = reinterpret_cast<CoffReloc*>(sr->data + sr->data_offset);
	qrel = reinterpret_cast<CoffReloc*>(sr->data);
	for (rel = qrel; rel < rel_end; rel++)
	{
		sec = ptr_coff->sections[rel->section - 1];

		sym_index = rel->cfsym;
		sym = &(reinterpret_cast<CoffSym*>(ptr_coff->sec_symtab->data))[sym_index];
		name = ptr_coff->sec_symtab->data + sym->Name;

		val = sym->Value;
		type = rel->type;
		addr = sec->sh.VirtualAddress + rel->offset;
		ptr = sec->data + rel->offset;
		switch (type)
		{
		case IMAGE_REL_I386_DIR32: // global variable
			*reinterpret_cast<int*>(ptr) += val;
			break;
		case IMAGE_REL_I386_REL32:
			*reinterpret_cast<int*>(ptr) += val - addr;
			break;
		}
	}

}

int Linker::pe_write(PEInfo* pe)
{
	FILE *op = nullptr;
	DWORD file_offset = 0, r = 0;
	int sizeofheaders;
	op = fopen(pe->filename, "wb");
	if (op == nullptr)
	{
		link_error("'%s' generate failed!", pe->filename);
		return 1;
	}

	sizeofheaders = pe_file_align(sizeof(dos_header) + sizeof(dos_stub)
								+ sizeof(nt_header) + pe->sec_size * sizeof(IMAGE_SECTION_HEADER));

	file_offset = sizeofheaders;
	fpad(op, file_offset);
	for (int sec_idx = 0; sec_idx < pe->sec_size; sec_idx++)
	{
		Section* sec = pe->secs[sec_idx];
		char * sh_name = reinterpret_cast<char*>(sec->sh.Name);
		unsigned long addr = sec->sh.VirtualAddress - nt_header.OptionalHeader.ImageBase;
		unsigned long size = sec->data_offset;
		IMAGE_SECTION_HEADER* psh = &(sec->sh);

		if (!strcmp(reinterpret_cast<const char*>(sec->sh.Name), ".text"))
		{
			nt_header.OptionalHeader.BaseOfCode = addr;
			nt_header.OptionalHeader.AddressOfEntryPoint = addr + pe->entry_addr;
		}
		else if (!strcmp(reinterpret_cast<const char*>(sec->sh.Name), ".data"))
		{
			nt_header.OptionalHeader.BaseOfCode = addr;
		}
		else if (!strcmp(reinterpret_cast<const char*>(sec->sh.Name), ".idata"))
		{
			if (pe->imp_size)
			{
				pe_set_datadir(IMAGE_DIRECTORY_ENTRY_IMPORT, pe->imp_offs + addr, pe->imp_size);
				pe_set_datadir(IMAGE_DIRECTORY_ENTRY_IAT, pe->iat_offs + addr, pe->iat_size);
			}
		}

		strcpy(reinterpret_cast<char*>(psh->Name), sh_name);

		psh->VirtualAddress = addr;
		psh->Misc.VirtualSize = size;
		nt_header.OptionalHeader.SizeOfImage = pe_virtual_align(size + addr);

		if (sec->data_offset)
		{
			psh->PointerToRawData = r = file_offset;
			if (!strcmp(reinterpret_cast<const char*>(sec->sh.Name), ".bss"))
			{
				sec->sh.SizeOfRawData = 0;
				continue;
			}

			fwrite(sec->data, 1, sec->data_offset, op);
			file_offset = pe_file_align(file_offset + sec->data_offset);
			psh->SizeOfRawData = file_offset - r;
			fpad(op, file_offset);
		}
	}

	nt_header.FileHeader.NumberOfSections = pe->sec_size;
	nt_header.OptionalHeader.SizeOfHeaders = sizeofheaders;

	nt_header.OptionalHeader.Subsystem = subsystem;

	fseek(op, SEEK_SET, 0);
	fwrite(&dos_header, 1, sizeof(dos_header), op);
	fwrite(&dos_stub, 1, sizeof(dos_stub), op);
	fwrite(&nt_header, 1, sizeof(nt_header), op);

	for (int sec_idx = 0; sec_idx < pe->sec_size; sec_idx++)
	{
		fwrite(&pe->secs[sec_idx]->sh, 1, sizeof(IMAGE_SECTION_HEADER), op);
	}
	fclose(op);
	return 0;
}

DWORD Linker::pe_file_align(DWORD n)
{
	DWORD FileAlignment = nt_header.OptionalHeader.FileAlignment;
	return calc_align(n, FileAlignment);
}

void Linker::pe_set_datadir(int dir, DWORD addr, DWORD size)
{
	nt_header.OptionalHeader.DataDirectory[dir].VirtualAddress = addr;
	nt_header.OptionalHeader.DataDirectory[dir].Size = size;
}

int Linker::pe_output_file(char * filename)
{
	int ret = 0;
	PEInfo pe = {0};
	
	pe.filename = filename;

	// Load static library
	add_runtime_libs();

	// Get entry point
	get_entry_addr(&pe);

	// Resolve external symbol
	ret = resolve_coffsyms(&pe);

	if (0 == ret)
	{
		// 
		pe_assign_address(&pe);
		//
		relocate_syms();

		coffrelocs_fixup();

		ret = pe_write(&pe);

		free(pe.secs);
	}
	return ret;
}

void Linker::get_entry_addr(PEInfo* pe)
{
	unsigned long addr = 0;
	int cs = 0;
	CoffSym* cfsym_entry = NULL;

	cs = ptr_coff->coffsym_search(ptr_coff->sec_symtab, entry_symbol);
	cfsym_entry = reinterpret_cast<CoffSym*>(ptr_coff->sec_symtab->data) + cs;
	addr = cfsym_entry->Value;
	pe->entry_addr = addr;
}