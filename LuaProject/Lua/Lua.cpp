// Lua.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
extern "C"{
#include "lualib/lua.h"
#include "lualib/lualib.h"
#include "lualib/lauxlib.h"
}
extern "C"{
#include "pb.h"
}
int main(int argc, _TCHAR* argv[])
{
	lua_State* L = luaL_newstate();// lua_newstate(nullptr, nullptr);
	luaL_openlibs(L);
	luaopen_pb(L);
	if (luaL_dofile(L, "./lua/main.lua"))
	{
		printf("%s", lua_tostring(L, -1));
		lua_pop(L, 1);
	}
	getchar();
	return 0;
}

