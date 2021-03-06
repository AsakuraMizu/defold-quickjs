#include "LuaUtils.h"

void printStack(lua_State *L)
{
    int n = lua_gettop(L);
    for (int i = 1; i <= n; i++)
    {
        switch (lua_type(L, i))
        {
        case LUA_TBOOLEAN:
            printf("STACK %d %d\n", i, lua_toboolean(L, i));
            break;

        case LUA_TNIL:
            printf("STACK %d nil\n", i);
            break;

        case LUA_TNUMBER:
            printf("STACK %d %.14g\n", i, lua_tonumber(L, i));
            break;

        case LUA_TSTRING:
            printf("STACK %d %s\n", i, lua_tostring(L, i));
            break;

        default:
            printf("STACK %d [%s]\n", i, luaL_typename(L, i));
            break;
        }
    }
}

void checkArgCount(lua_State *L, int countExact)
{
    int count = lua_gettop(L);
    if (count != countExact)
    {
        luaL_error(L, "This function requires %d argument(s). Got %d.", countExact, count);
    }
}

void checkArgCount(lua_State *L, int countFrom, int countTo)
{
    int count = lua_gettop(L);
    if ((count < countFrom) || (count > countTo))
    {
        luaL_error(L, "This function requires from %d to %d argument(s). Got %d.", countFrom, countTo, count);
    }
}

void checkString(lua_State *L, int index)
{
    int luaType = lua_type(L, index);
    if (luaType != LUA_TSTRING)
    {
        luaL_error(L, "Wrong type. String expected at %d. Got %s.", index, lua_typename(L, luaType));
    }
}

void checkNumber(lua_State *L, int index)
{
    int luaType = lua_type(L, index);
    if (luaType != LUA_TNUMBER)
    {
        luaL_error(L, "Wrong type. Number expected at %d. Got %s.", index, lua_typename(L, luaType));
    }
}

void checkBoolean(lua_State *L, int index)
{
    int luaType = lua_type(L, index);
    if (luaType != LUA_TBOOLEAN)
    {
        luaL_error(L, "Wrong type. Boolean expected at %d. Got %s.", index, lua_typename(L, luaType));
    }
}

void checkTable(lua_State *L, int index)
{
    int luaType = lua_type(L, index);
    if (luaType != LUA_TTABLE)
    {
        luaL_error(L, "Wrong type. Table expected at %d. Got %s.", index, lua_typename(L, luaType));
    }
}