#include <ldns/ldns.h>

#define LUALIB
#include "lua.h"
#include "lauxlib.h"

int l_dns_lookup(lua_State *L) {
    const char *domain_string = luaL_checkstring(L, 1);
    const char *resolver_string = luaL_checkstring(L, 2);

    ldns_rdf *domain = ldns_dname_new_frm_str(domain_string);
    if (!domain) {
        lua_pushnil(L);
        lua_pushstring(L, "error parsing domain name");
        return 2;
    }

    ldns_resolver *resolver;
    if (strlen(resolver_string) == 0) {
        ldns_status status = ldns_resolver_new_frm_file(&resolver, NULL);
        if (status != LDNS_STATUS_OK) {
            ldns_rdf_deep_free(domain);
            lua_pushnil(L);
            lua_pushstring(L, "error creating new resolver");
            return 2;
        }
    } else {
        resolver = ldns_resolver_new();
        if (!resolver) {
            ldns_rdf_deep_free(domain);
            lua_pushnil(L);
            lua_pushstring(L, "error creating new resolver");
            return 2;
        }
        ldns_rdf *nameserver = ldns_rdf_new_frm_str(LDNS_RDF_TYPE_A, resolver_string);
        if (!nameserver) {
            ldns_rdf_deep_free(domain);
            ldns_resolver_deep_free(resolver);
            lua_pushnil(L);
            lua_pushstring(L, "error parsing nameserver address");
            return 2;
        }
        ldns_status status = ldns_resolver_push_nameserver(resolver, nameserver);
        if (status != LDNS_STATUS_OK) {
            ldns_rdf_deep_free(domain);
            ldns_resolver_deep_free(resolver);
            lua_pushnil(L);
            lua_pushstring(L, "error pushing nameserver address");
            return 2;
        }
    }

    ldns_pkt *pkt = ldns_resolver_query(resolver,
                                        domain,
                                        LDNS_RR_TYPE_A,
                                        LDNS_RR_CLASS_IN,
                                        LDNS_RD);
    ldns_rdf_deep_free(domain);
    if (!pkt) {
        ldns_resolver_deep_free(resolver);
        lua_pushnil(L);
        lua_pushstring(L, "error issuing query");
        return 2;
    }

    ldns_rr_list *results = ldns_pkt_rr_list_by_type(pkt,
                                                     LDNS_RR_TYPE_A,
                                                     LDNS_SECTION_ANSWER);
    if (!results) {
        ldns_pkt_free(pkt);
        ldns_resolver_deep_free(resolver);
        lua_pushnil(L);
        lua_pushstring(L, "error extracting result");
        return 2;
    }

    if (ldns_rr_list_rr_count(results) == 0) {
        lua_pushnil(L);
    } else {
        ldns_rr *result = ldns_rr_list_rr(results, 0);
        ldns_rdf *a_record = ldns_rr_a_address(result);
        char *ip_address = ldns_rdf2str(a_record);
        lua_pushstring(L, ip_address);
    }
    ldns_pkt_free(pkt);
    ldns_resolver_deep_free(resolver);
    lua_pushnil(L);
    return 2;
}
