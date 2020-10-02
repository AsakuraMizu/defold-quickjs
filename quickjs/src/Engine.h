#ifndef __ENGINE_H__
#define __ENGINE_H__

#include <dmsdk/sdk.h>
#include "quickjs.h"

class Engine
{
private:
    static Engine *instance;

    JSRuntime *rt;
    int lastId;

    Engine();
    ~Engine();

public:
    static Engine *getInstance();

    int script(lua_State *L, const char *call);
    JSContext *getCtx(int id, lua_State *L);
};

#endif // __ENGINE_H__