#include "LuaObject.h"
#include "binder.h"

namespace LuaObject
{
    JSClassID class_id;

    struct Opaque {
        lua_State *L;   // This state is used only for finalizer
        int reference;
    };

    static int delete_property(JSContext *ctx, JSValueConst obj, JSAtom atom)
    {
        CHECK_L2(L, ctx, -1)
        JSValue prop = JS_AtomToValue(ctx, atom);
        push(ctx, obj);
        pushJSValue(ctx, prop);
        lua_pushnil(L);
        lua_settable(L, -3);
        lua_pop(L, 1);
        JS_FreeValue(ctx, prop);
        return true;
    }

    static int has_property(JSContext *ctx, JSValueConst obj, JSAtom atom)
    {
        CHECK_L2(L, ctx, -1)
        bool ret;
        JSValue prop = JS_AtomToValue(ctx, atom);;
        push(ctx, obj);
        pushJSValue(ctx, prop);
        lua_gettable(L, -2);
        ret = !lua_isnil(L, -1);
        lua_pop(L, 2);
        JS_FreeValue(ctx, prop);
        return ret;
    }

    static JSValue get_property(JSContext *ctx, JSValueConst obj, JSAtom atom, JSValueConst receiver)
    {
        CHECK_L1(L, ctx)
        JSValue ret, prop = JS_AtomToValue(ctx, atom);
        push(ctx, obj);
        pushJSValue(ctx, prop);
        lua_gettable(L, -2);
        ret = toJSValue(ctx, -1);
        lua_pop(L, 2);
        JS_FreeValue(ctx, prop);
        return ret;
    }

    static int set_property(JSContext *ctx, JSValueConst obj, JSAtom atom, JSValueConst value, JSValueConst receiver, int flags)
    {
        CHECK_L2(L, ctx, -1)
        JSValue prop = JS_AtomToValue(ctx, atom);
        push(ctx, obj);
        pushJSValue(ctx, prop);
        pushJSValue(ctx, value);
        lua_settable(L, -3);
        lua_pop(L, 1);
        JS_FreeValue(ctx, prop);
        return true;
    }

    static void finalizer(JSRuntime *rt, JSValue val)
    {
        Opaque *opaque = (Opaque *)JS_GetOpaque(val, class_id);
        dmScript::Unref(opaque->L, LUA_REGISTRYINDEX, opaque->reference);
        js_free_rt(rt, opaque);
    }

    static JSValue call(JSContext *ctx, JSValueConst func_obj, JSValueConst this_val, int argc, JSValueConst *argv, int flags)
    {
        CHECK_L1(L, ctx)
        uint32_t nresult;
        int i, err;
        JSValue ret;
        if (argc < 1 || !JS_IsNumber(argv[0]))
        {
            return JS_ThrowTypeError(ctx, "number excepted for param 1");
        }
        JS_ToUint32(ctx, &nresult, argv[0]);
        push(ctx, func_obj);
        for (i = 1; i < argc; ++i)
        {
            pushJSValue(ctx, argv[i]);
        }
        err = dmScript::PCall(L, argc - 1, nresult);
        if (err)
        {
            return JS_ThrowInternalError(ctx, "pcall failed");
        }
        if (nresult == 0)
        {
            ret = JS_UNDEFINED;
        }
        else if (nresult == 1)
        {
            ret = toJSValue(ctx, -1);
        }
        else
        {
            ret = JS_NewArray(ctx);
            for (i = 0; i < nresult; ++i)
            {
                JS_SetPropertyUint32(ctx, ret, i, toJSValue(ctx, i - nresult));
            }
        }
        lua_pop(L, nresult);
        return ret;
    }

    JSClassExoticMethods exotic = {
        .delete_property = delete_property,
        .has_property = has_property,
        .get_property = get_property,
        .set_property = set_property
    };

    JSClassDef class_def = {
        .class_name = "LuaObject",
        .finalizer = LuaObject::finalizer,
        .call = LuaObject::call,
        .exotic = &LuaObject::exotic
    };

    void init(JSRuntime *rt)
    {
        JS_NewClassID(&class_id);
        JS_NewClass(rt, class_id, &class_def);
    }

    JSValue create(JSContext *ctx)
    {
        CHECK_L1(L, ctx)
        JSValue obj = JS_NewObjectClass(ctx, class_id);
        Opaque *opaque = (Opaque *)js_malloc(ctx, sizeof(Opaque));
        opaque->L = L;
        opaque->reference = dmScript::Ref(L, LUA_REGISTRYINDEX);
        JS_SetOpaque(obj, opaque);
        return obj;
    }

    bool push(JSContext *ctx, JSValueConst val)
    {
        CHECK_L2(L, ctx, false)
        Opaque *opaque = (Opaque *)JS_GetOpaque(val, class_id);
        if (!opaque)
        {
            return false;
        }
        lua_rawgeti(L, LUA_REGISTRYINDEX, opaque->reference);
        return true;
    }
} // namespace LuaObject