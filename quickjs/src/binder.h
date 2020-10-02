#ifndef __BINDER_H__
#define __BINDER_H__

#include <dmsdk/sdk.h>
#include "quickjs.h"

#define CHECK_L1(L, ctx) \
lua_State *L = get_lua_state(ctx); \
if (!L) \
{ \
    return JS_ThrowInternalError(ctx, "could not get lua state"); \
}

#define CHECK_L2(L, ctx, val) \
lua_State *L = get_lua_state(ctx); \
if (!L) \
{ \
    JS_ThrowInternalError(ctx, "could not get lua state"); \
    return val; \
}

void dump_obj(JSContext *ctx, JSValueConst val);
void dump_obj_error(JSContext *ctx, JSValueConst val);
void dump_error(JSContext *ctx);
lua_State *get_lua_state(JSContext *ctx);
int module_set_import_meta(JSContext *ctx, JSValueConst func_val, bool is_main);
JSModuleDef *module_loader(JSContext *ctx, const char *module_name, void *opaque);
void pushJSValue(JSContext *ctx, JSValueConst val);
JSValue toJSValue(JSContext *ctx, int idx);

#endif // __BINDER_H__