#include <stdio.h>
#include <windows.h>

#include "CL_CompDef.h"
#include "CL_Syntax.h"
#include "CL_Link.h"

Syntax syntax;
Linker linker;

// @Global source code file pointer
FILE* fin = NULL;

// @Global output file name
char* outfile;

// @Global source file array
std::vector < char* > src_files;

// @Global dll array
std::vector< char* > array_dll;

// @Global library array
std::vector < char* > array_lib;

// @Global output file type
e_OutType output_type;

// @Global scc compile version
float scc_version = 1.00f;

// @Global program entry symbol
char* entry_symbol = "_entry";

// @Global library path
char* lib_path;

// @Global subsystem
short subsystem;

// @Function parser command line
int process_command(int argc, char **argv)
{
	for (int argc_idx = 1; argc_idx < argc; argc_idx++)
	{
		if (argv[argc_idx][0] == '-')
		{
			char *p = &argv[argc_idx][1];
			int c = *p;

			switch (c)
			{
			case 'o':
				outfile = argv[++argc_idx];
				break;
			case 'c':
				src_files.push_back(argv[++argc_idx]);
				output_type = OUTPUT_OBJ;
				return 1;
				break;
			case 'l':
				array_lib.push_back(&argv[argc_idx][2]);
				break;
			case 'G':
				subsystem = IMAGE_SUBSYSTEM_WINDOWS_GUI;
				break;
			case'v':
				printf("SCC Version%.2f", scc_version);
				break;
			case 'h':
				printf("usage: scc [-c infile] [-o outfile] [-lib] [infile] infile2...\n");
				return 0;
				break;
			default:
				printf("unsupport command line option");
				break;
			}
		}
		else
		{
			src_files.push_back(argv[argc_idx]);
		}
	}

	return 1;
}

// @Function compile file
void compile(char* fname)
{
	if (-1 == syntax.init_syntax(fname))
	{
		return;
	}

	syntax.translation_unit();
	printf("%s compile success", fname);
}

void init()
{
	output_type = OUPUT_EXE;
	subsystem = IMAGE_SUBSYSTEM_WINDOWS_CUI;

	lib_path = get_lib_path();

	linker.init();
}

int main(int argc, char** argv)
{
	
	int opind;
	char *ext;
	init();
	opind = process_command(argc, argv);
	if (opind == 0)
	{
		return 0;
	}

	for (int src_idx = 0; src_idx < static_cast<int>(src_files.size()); src_idx++)
	{
		filename = src_files[src_idx];
		ext = get_file_ext(filename);
		if (!strcmp(ext, "c"))
		{
			compile(filename);
		}
		
		if (!strcmp(ext, "obj"))
		{
			linker.load_obj_file(filename);
		}
	}

	if (!outfile)
	{
		printf("usage: scc [-c infile] [-o outfile] [-lib] [infile] infile2...\n");
		return -1;
	}

	if (output_type == OUTPUT_OBJ)
	{
		syntax.coff.write_obj(outfile);
	}
	else
		linker.pe_output_file(outfile);

	getchar();

	return 0;
}