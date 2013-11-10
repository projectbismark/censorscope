#ifndef CENSORSCOPE_TRANSPORT_H
#define CENSORSCOPE_TRANSPORT_H

#include "lua.h"

typedef struct {
    lua_State *L;
} transport_t;

int transport_init(transport_t *transport, const char *module);

int transport_destroy(transport_t *transport);

int transport_download(transport_t *transport);

int transport_upload(transport_t *transport);

#endif
