#include <stdio.h>
#include <windows.h>

#include "CL_CompDef.h"
#include "CL_Syntax.h"

Syntax syntax;

int main(int argc, char** argv)
{
	if (-1 == syntax.init_syntax("test_code.cpp"))
	{
		return -1;
	}

	syntax.translation_unit();

	getchar();

	return 0;
}