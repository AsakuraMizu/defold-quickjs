#ifndef __LUA_OBJECT_H__
#define __LUA_OBJECT_H__

#include <dmsdk/sdk.h>
#include "quickjs.h"

namespace LuaObject
{
    void init(JSRuntime *rt);
    JSValue create(JSContext *ctx);
    bool push(JSContext *ctx, JSValueConst val);
}; // namespace LuaObject

#endif // __LUA_OBJECT_H__