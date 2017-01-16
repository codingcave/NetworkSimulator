#include <iostream>
#include <unordered_map>
#include <vector>
#include "lua.hpp"

#include "Naming.hpp"
#include "ParameterType.hpp"
#include "def.hpp"
#include "ParameterTypeSystem.hpp"
#include "types/LuaFunctionWrapper.hpp"
#include "types/lua_basetypes.hpp"

int lua_UserData_gc(lua_State* L)
{
    if(luaL_getmetafield(L, 1, "__type")) {
        int type = lua_tointeger(L, 2);
        void* value = *((void**)lua_touserdata(L, 1));
        ParameterTypeSystem::deleteValue({type, value});
    }
    return 0;
}

void luat_UserData_push(lua_State* const L, void * const value, const int& type)
{
    const void **item = (const void **)lua_newuserdata(L, sizeof(void *));
    *item = value;
}

void luat_nil_push(lua_State* const L, void * const value, const int& type)
{
    lua_pushnil(L);
}

void luat_boolean_push(lua_State* const L, void * const value, const int& type)
{
    lua_pushboolean(L, *((bool*)value));
}

void luat_boolean_delete(lua_State* const L, void * const value, const int& type)
{
    delete ((bool*)value);
}

void luat_real_push(lua_State* const L, void * const value, const int& type)
{
    lua_pushnumber(L, *((double*)value));
}

void luat_real_delete(lua_State* const L, void * const value, const int& type)
{
    delete ((double*)value);
}

void luat_string_push(lua_State* const L, void * const value, const int& type)
{
    std::string* s = (std::string*)value;
    lua_pushlstring(L, s->c_str(), s->size());
}

void luat_string_delete(lua_State* const L, void * const value, const int& type)
{
    delete ((std::string*)value);
}

void luat_function_push(lua_State* const L, void * const value, const int& type)
{
    lua_pushcfunction(L, (lua_CFunction)value);
}

int luat_luafunction_call(lua_State* L)
{
    LuaFunctionWrapper* obj = *((LuaFunctionWrapper**)lua_touserdata(L, 1));
    vec_t_LuaItem items;
    for(int i = 2; i <= lua_gettop(L); i++)
    {
        items.push_back(ParameterTypeSystem::getValue(i));
    }

    const struct T_Parameter p = (*obj)(items);
    ParameterTypeSystem::pushValue(p);

    return 1;
}

void luat_luafunction_init(lua_State* const L, const int& type)
{
    lua_pushstring(L, Naming::Lua_reg_functions);
    lua_newtable(L);
    lua_rawset(L, LUA_REGISTRYINDEX);
    std::string metaName("meta:");
    metaName += Naming::Type_UserFunction;
    luaL_getmetatable (L, metaName.c_str());
    lua_pushcfunction(L, luat_luafunction_call);
    lua_setfield(L, -2, "__call");
}

void luat_luafunction_push(lua_State* const L, void * const value, const int& type)
{
    lua_pushstring(L, Naming::Lua_reg_functions);
    lua_rawget(L, LUA_REGISTRYINDEX);
    lua_pushlightuserdata(L, value);

    //LuaFunctionWrapper* obj = (LuaFunctionWrapper*)value;
    if(lua_rawget(L, -2))
    {
        lua_pop(L, 1);
        LuaFunctionWrapper **item = (LuaFunctionWrapper **)lua_newuserdata(L, sizeof(LuaFunctionWrapper *));
        *item = (LuaFunctionWrapper*)value;
        std::string metaName("meta:");
        metaName += Naming::Type_UserFunction;
        luaL_setmetatable (L, metaName.c_str());
        //lua_pushlightuserdata(L, value);
        //lua_pushvalue(L, -2);
        //lua_rawset(L, -4);
    }
    lua_remove(L, -2);
}

void luat_luafunction_delete(lua_State* const L, void * const value, const int& type)
{
    LuaFunctionWrapper* fw = (LuaFunctionWrapper*)value;
    if(fw->intern()) {
        lua_pushstring(L, Naming::Lua_reg_functions);
        lua_rawget(L, LUA_REGISTRYINDEX);
        lua_pushlightuserdata(L, value);
        lua_pushnil(L);
        lua_rawset(L, -3);
        lua_pop(L, 1);
    }
    delete fw;
}