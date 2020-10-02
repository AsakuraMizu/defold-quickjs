extern "C"
{
#include "cutils.h"
}
#include "binder.h"
#include "Engine.h"
#include "LuaObject.h"
#include "LuaUtils.h"

namespace dmScript
{
    int Sys_LoadResource(lua_State *L);
}

static void dump_obj1(JSContext *ctx, JSValueConst val, dmLogSeverity severity)
{
    const char *str = JS_ToCString(ctx, val);
    if (str)
    {
        dmLogInternal(severity, DLIB_LOG_DOMAIN, "%s", str);
        JS_FreeCString(ctx, str);
    }
    else
    {
        dmLogInternal(severity, DLIB_LOG_DOMAIN, "%s", str);
    }
}

void dump_obj(JSContext *ctx, JSValueConst val)
{
    dump_obj1(ctx, val, DM_LOG_SEVERITY_INFO);
}

void dump_obj_error(JSContext *ctx, JSValueConst val)
{
    dump_obj1(ctx, val, DM_LOG_SEVERITY_ERROR);
}

static void dump_error1(JSContext *ctx, JSValueConst exception_val)
{
    JSValue val;
    bool is_error;

    is_error = JS_IsError(ctx, exception_val);
    dump_obj_error(ctx, exception_val);
    if (is_error)
    {
        val = JS_GetPropertyStr(ctx, exception_val, "stack");
        if (!JS_IsUndefined(val))
        {
            dump_obj_error(ctx, val);
        }
        JS_FreeValue(ctx, val);
    }
}

void dump_error(JSContext *ctx)
{
    JSValue exception_val;

    exception_val = JS_GetException(ctx);
    dump_error1(ctx, exception_val);
    JS_FreeValue(ctx, exception_val);
}

lua_State *get_lua_state(JSContext *ctx)
{
    return (lua_State *)JS_GetContextOpaque(ctx);
}

int module_set_import_meta(JSContext *ctx, JSValueConst func_val, bool is_main)
{
    JSModuleDef *m;
    char buf[PATH_MAX + 16];
    JSValue meta_obj;
    JSAtom module_name_atom;
    const char *module_name;

    assert(JS_VALUE_GET_TAG(func_val) == JS_TAG_MODULE);
    m = (JSModuleDef *)JS_VALUE_GET_PTR(func_val);

    module_name_atom = JS_GetModuleName(ctx, m);
    module_name = JS_AtomToCString(ctx, module_name_atom);
    JS_FreeAtom(ctx, module_name_atom);
    if (!module_name)
        return -1;
    pstrcpy(buf, sizeof(buf), module_name);
    JS_FreeCString(ctx, module_name);

    meta_obj = JS_GetImportMeta(ctx, m);
    if (JS_IsException(meta_obj))
        return -1;
    JS_DefinePropertyValueStr(ctx, meta_obj, "url", JS_NewString(ctx, buf), JS_PROP_C_W_E);
    JS_DefinePropertyValueStr(ctx, meta_obj, "main", JS_NewBool(ctx, is_main), JS_PROP_C_W_E);
    JS_FreeValue(ctx, meta_obj);
    return 0;
}

static uint8_t *load_file(JSContext *ctx, size_t *pbuf_len, const char *module_name)
{
    uint8_t *buf;
    size_t buf_len;

    CHECK_L2(L, ctx, nullptr)

    lua_pushstring(L, module_name);
    int ret = dmScript::Sys_LoadResource(L);
    if (ret == 1)
    {
        buf_len = lua_strlen(L, -1);
        if (ctx)
            buf = (uint8_t *)js_malloc(ctx, buf_len + 1);
        else
            buf = (uint8_t *)malloc(buf_len + 1);
        strcpy((char *)buf, lua_tolstring(L, -1, &buf_len));
        buf[buf_len] = '\0';
        *pbuf_len = buf_len;
        lua_pop(L, 2);
        return buf;
    }
    else
    {
        lua_pop(L, ret + 1);
        return nullptr;
    }
}

JSModuleDef *module_loader(JSContext *ctx, const char *module_name, void *opaque)
{
    JSModuleDef *m;

    JSValue func_val;

    size_t buf_len;
    uint8_t *buf;

    buf = load_file(ctx, &buf_len, module_name);
    if (!buf)
    {
        JS_ThrowReferenceError(ctx, "could not load module filename '%s'", module_name);
        return nullptr;
    }

    func_val = JS_Eval(ctx, (char *)buf, buf_len, module_name, JS_EVAL_TYPE_MODULE | JS_EVAL_FLAG_COMPILE_ONLY);
    if (JS_IsException(func_val))
        return nullptr;

    module_set_import_meta(ctx, func_val, false);

    m = (JSModuleDef *)JS_VALUE_GET_PTR(func_val);
    JS_FreeValue(ctx, func_val);
    return m;
}

void pushJSValue(JSContext *ctx, JSValueConst val)
{
    CHECK_L2(L, ctx, )

    switch (JS_VALUE_GET_NORM_TAG(val))
    {
    case JS_TAG_STRING:
    {
        size_t l;
        const char *s = JS_ToCStringLen(ctx, &l, val);
        lua_pushlstring(L, s, l);
        JS_FreeCString(ctx, s);
        break;
    }

    case JS_TAG_INT:
    {
        int64_t ni;
        JS_ToInt64(ctx, &ni, val);
        lua_pushinteger(L, ni);
        break;
    }

    case JS_TAG_BOOL:
    {
        bool b = JS_ToBool(ctx, val);
        lua_pushboolean(L, b);
        break;
    }

    case JS_TAG_FLOAT64:
    {
        double nf;
        JS_ToFloat64(ctx, &nf, val);
        lua_pushnumber(L, nf);
        break;
    }

    case JS_TAG_OBJECT:
    {
        if (!LuaObject::push(ctx, val))
        {
            JSValue _global = JS_GetGlobalObject(ctx);
            JSValue _object = JS_GetPropertyStr(ctx, _global, "Object");
            JSValue _keys = JS_GetPropertyStr(ctx, _object, "keys");
            JSValue keys = JS_Call(ctx, _keys, _object, 1, &val);
            uint32_t length, i;
            JSValue _key;
            const char *key;
            JS_ToUint32(ctx, &length, JS_GetPropertyStr(ctx, keys, "length"));
            lua_newtable(L);
            for (i = 0; i < length; ++i)
            {
                _key = JS_GetPropertyUint32(ctx, keys, i);
                key = JS_ToCString(ctx, _key);
                lua_pushstring(L, key);
                pushJSValue(ctx, JS_GetPropertyStr(ctx, val, key));
                lua_settable(L, -3);
                JS_FreeCString(ctx, key);
            }
            JS_FreeValue(ctx, keys);
        }
        break;
    }

    default:
    {
        lua_pushnil(L);
        break;
    }
    };
}

JSValue toJSValue(JSContext *ctx, int idx)
{
    CHECK_L1(L, ctx)

    JSValue ret;

    switch (lua_type(L, idx))
    {
    case LUA_TNIL:
    {
        ret = JS_NULL;
        break;
    }

    case LUA_TBOOLEAN:
    {
        bool b = lua_toboolean(L, idx);
        ret = JS_NewBool(ctx, b);
        break;
    }

    case LUA_TNUMBER:
    {
        double n = lua_tonumber(L, idx);
        ret = JS_NewFloat64(ctx, n);
        break;
    }

    case LUA_TSTRING:
    {
        size_t len;
        const char *str = lua_tolstring(L, idx, &len);
        ret = JS_NewStringLen(ctx, str, len);
        break;
    }

    default:
    {
        lua_pushvalue(L, idx);
        ret = LuaObject::create(ctx);
        break;
    }
    }

    return ret;
}