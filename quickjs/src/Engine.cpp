#include <string>
#include "Engine.h"
#include "binder.h"
#include "LuaObject.h"

Engine *Engine::instance = 0;

Engine::Engine()
{
    rt = JS_NewRuntime();
    lastId = 0;

    JS_SetModuleLoaderFunc(rt, 0, module_loader, 0);
    LuaObject::init(rt);
}

Engine::~Engine()
{
    JS_FreeRuntime(rt);
}

Engine *Engine::getInstance()
{
    if (!instance)
    {
        instance = new Engine;
    }
    return instance;
}

int Engine::script(lua_State *L, const char *call)
{
    std::string code, eval_name;
    JSValue global, val, func, *args = nullptr;
    int ret, top = lua_gettop(L);
    const char *filename = lua_tostring(L, 1);
    JSContext *ctx = getCtx(++lastId, L);

    eval_name = "<script:";
    eval_name += filename;
    eval_name += ">";

    code = "import * as script from '";
    code += filename;
    code += "'\n";
    if (call)
    {
        code += "globalThis.func = script.";
        code += call;
        code += " || function() {};";
    }
    val = JS_Eval(ctx, code.c_str(), code.length(), eval_name.c_str(), JS_EVAL_TYPE_MODULE);
    if (JS_IsException(val))
    {
        dump_error(ctx);
        JS_FreeValue(ctx, val);
        JS_FreeContext(ctx);
        return 0;
    }
    JS_FreeValue(ctx, val);

    if (!call)
    {
        return 0;
    }

    if (top > 1)
    {
        args = (JSValue *)js_malloc(ctx, sizeof(JSValue) * (top - 1));
        for (int i = 2; i <= top; ++i)
        {
            args[i - 2] = toJSValue(ctx, i);
        }
    }

    global = JS_GetGlobalObject(ctx);
    func = JS_GetPropertyStr(ctx, global, "func");
    val = JS_Call(ctx, func, global, top - 1, args);
    if (JS_IsException(val))
    {
        dump_error(ctx);
        ret = 0;
    }
    else
    {
        pushJSValue(ctx, val);
        ret = 1;
    }

    js_free(ctx, args);
    JS_FreeValue(ctx, val);
    JS_FreeValue(ctx, func);
    JS_FreeValue(ctx, global);
    JS_FreeContext(ctx);

    return ret;
}

JSContext *Engine::getCtx(int id, lua_State *L)
{
    JSContext *ctx = JS_NewContext(rt);
    JS_SetContextOpaque(ctx, L);
    JSValue global = JS_GetGlobalObject(ctx);
    lua_pushvalue(L, LUA_GLOBALSINDEX);
    JS_SetPropertyStr(ctx, global, "lua", LuaObject::create(ctx));
    JS_FreeValue(ctx, global);
    return ctx;
}