// RBTree.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "RBTree.h"

#include <string>

bool operator < (const std::string& l, const std::string& r)
{
	return (strcmp(l.c_str(), r.c_str()) < 0);
}

int _tmain(int argc, _TCHAR* argv[])
{
	RBTree < std::string, std::string > dictional;
	dictional.insert_node("34", "wor879ld");
	dictional.insert_node("788", "456");
	dictional.insert_node("4557", "dfdsaf");

	RBTree<int, std::string> dictional2;
	dictional2.insert_node();
	return 0;
}

