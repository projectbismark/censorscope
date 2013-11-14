#include "http.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>

#define LUALIB
#include "lua.h"
#include "lauxlib.h"

static size_t write_data(void *ptr, size_t size, size_t nmemb, void *arg)
{
    lua_State *L = arg;
    const size_t data_len = size*nmemb;
    char *response = malloc(data_len +2);

    memcpy(response, ptr, data_len);
    response[data_len] = '\0';

    printf("%s\n",response);
    /* lua_pushstring(L, "foo"); */

    free(response);
    return data_len;
}

int l_http_get(lua_State *L) {
    CURL *curl_handle;
    CURLcode res;
    const char *url = luaL_checkstring(L, 1);
    fprintf(stderr, "URL - %s\n", url);

    /* init the curl session */
    curl_handle = curl_easy_init();
    if(!curl_handle) {
        curl_global_cleanup();
        lua_pushnil(L);
        lua_pushstring(L, "error creating handle");
        return 2;
    }

    /* set URL to get */
    curl_easy_setopt(curl_handle, CURLOPT_URL, url);

    /* no progress meter please */
    curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);

    /* send all data to this function  */
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);

    /* pass the Lua stack to the write function */
    curl_easy_setopt(curl_handle, CURLOPT_WRITEHEADER, L);

    /* perform the request */
    res = curl_easy_perform(curl_handle);
    if (res != CURLE_OK) {
        curl_global_cleanup();
        lua_pushnil(L);
        lua_pushstring(L, "error performing the request");
        return 2;
    }

    /* cleanup */
    curl_easy_cleanup(curl_handle);
    lua_pushnil(L);

    return 2;
}
