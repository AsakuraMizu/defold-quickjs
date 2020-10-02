#define LIB_NAME "quickjs"

#include <dmsdk/sdk.h>

#include "Engine.h"
#include "LuaUtils.h"

static int load(lua_State *L)
{
    checkArgCount(L, 1);
    checkString(L, 1);
    return Engine::getInstance()->script(L, nullptr);
}

static int init(lua_State *L)
{
    checkArgCount(L, 2);
    checkString(L, 1);
    return Engine::getInstance()->script(L, "init");
}

static const luaL_reg Module_methods[] =
{
    {"load", load},
    {"init", init},
    {0, 0}
};

static void LuaInit(lua_State *L)
{
    int top = lua_gettop(L);

    luaL_register(L, LIB_NAME, Module_methods);

    lua_pop(L, 1);
    assert(top == lua_gettop(L));
}

dmExtension::Result AppInitializeQuickJS(dmExtension::AppParams *params)
{
    return dmExtension::RESULT_OK;
}

dmExtension::Result InitializeQuickJS(dmExtension::Params *params)
{
    LuaInit(params->m_L);
    return dmExtension::RESULT_OK;
}

dmExtension::Result AppFinalizeQuickJS(dmExtension::AppParams *params)
{
    return dmExtension::RESULT_OK;
}

dmExtension::Result FinalizeQuickJS(dmExtension::Params *params)
{
    return dmExtension::RESULT_OK;
}

dmExtension::Result OnUpdateQuickJS(dmExtension::Params *params)
{
    return dmExtension::RESULT_OK;
}

DM_DECLARE_EXTENSION(quickjs, LIB_NAME, AppInitializeQuickJS, AppFinalizeQuickJS, InitializeQuickJS, OnUpdateQuickJS, 0, FinalizeQuickJS)