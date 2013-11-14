#include "http.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>

#define LUALIB
#include "lua.h"
#include "lauxlib.h"

struct buffer {
    char *string;
    size_t len;
};

void init_buffer(struct buffer *data) {
    data->len = 0;
    data->string = malloc(data->len+1);

    if (data->string == NULL) {
        fprintf(stderr, "malloc() failed\n");
        exit(EXIT_FAILURE);
    }
    data->string[0] = '\0';
}

static size_t write_data(void *string, size_t size, size_t nmemb, void *arg)
{
    struct buffer *data = (struct buffer *) arg;

    const size_t data_len = size*nmemb;
    const size_t new_len = data->len + data_len;

    data->string = realloc(data->string, new_len+1);
    if (data->string == NULL) {
        fprintf(stderr, "realloc failed\n");
    }

    memcpy(data->string + data->len, string, data_len);
    data->string[new_len] = '\0';
    data->len = new_len;

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

    /* create a buffer to keep our response */
    struct buffer data;
    init_buffer(&data);

    /* set URL to get */
    curl_easy_setopt(curl_handle, CURLOPT_URL, url);

    /* no progress meter please */
    curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);

    /* send all data to this function  */
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);

    /* pass the response string to the write function */
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &data);

    /* perform the request */
    res = curl_easy_perform(curl_handle);
    if (res != CURLE_OK) {
        curl_global_cleanup();
        lua_pushnil(L);
        lua_pushstring(L, "error performing the request");
        return 2;
    }

    printf("%d\n", data.len);
    printf("%s\n", data.string);
    /* lua_pushstring(L, data.string); */

    /* cleanup */
    free(data.string);
    curl_easy_cleanup(curl_handle);
    lua_pushnil(L);
    return 2;
}
