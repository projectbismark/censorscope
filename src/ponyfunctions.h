#ifndef CENSORSCOPE_PONYFUNCTIONS_H
#define CENSORSCOPE_PONYFUNCTIONS_H

#include "lua.h"

int pony_ping(lua_State *L);

int pony_traceroute(lua_State *L);

int pony_fasttraceroute(lua_State *L);

int pony_dns(lua_State *L);

int pony_gethttp(lua_State *L);

#endif
