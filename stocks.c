/*
 * MIT License
 *
 * Copyright (c) 2023 Davidson Francis <davidsondfgl@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "deps/cJSON/cJSON.h"
#include "stocks.h"
#include "log.h"

void stocks_free(struct stocks_info *si) {
    if (!si) return;
    free(si->symbol);
    free(si->longName);
    free(si->provider);
    free(si->currency);
    for (int i = 0; i < si->sub_count; ++i) {
        free(si->sub[i].symbol);
        free(si->sub[i].currency);
    }
}

static int json_parse_stocks(const char *json_str, struct stocks_info *si) {
    cJSON *root = cJSON_Parse(json_str);
    if (!root) return -1;

    cJSON *symbol = cJSON_GetObjectItem(root, "symbol");
    cJSON *name = cJSON_GetObjectItem(root, "name");
    cJSON *price = cJSON_GetObjectItem(root, "price");
    cJSON *percentChange = cJSON_GetObjectItem(root, "percentChange");
    cJSON *provider = cJSON_GetObjectItem(root, "provider");
    cJSON *sub = cJSON_GetObjectItem(root, "sub");
    cJSON *currency = cJSON_GetObjectItem(root, "currency");
    
    si->symbol = symbol && cJSON_IsString(symbol) ? strdup(symbol->valuestring) : NULL;
    si->longName = name && cJSON_IsString(name) ? strdup(name->valuestring) : NULL;
    si->price = price && cJSON_IsNumber(price) ? (float)price->valuedouble : 0.0f;
    si->percentChange = percentChange && cJSON_IsNumber(percentChange) ? (float)percentChange->valuedouble : 0.0f;
    si->provider = provider && cJSON_IsString(provider) ? strdup(provider->valuestring) : NULL;
    si->currency = currency && cJSON_IsString(currency) ? strdup(currency->valuestring) : NULL;

    int subidx = 0;
    cJSON *subitem = NULL;
    cJSON_ArrayForEach(subitem, sub) {
        if (subidx >= MAX_SUB) break;
        cJSON *ssymbol = cJSON_GetObjectItem(subitem, "symbol");
        cJSON *pchg = cJSON_GetObjectItem(subitem, "percentChange");
        cJSON *sprice = cJSON_GetObjectItem(subitem, "price");
        cJSON *scurrency = cJSON_GetObjectItem(subitem, "currency");

        si->sub[subidx].symbol = ssymbol && cJSON_IsString(ssymbol) ? strdup(ssymbol->valuestring) : NULL;
        si->sub[subidx].percentChange = pchg && cJSON_IsNumber(pchg) ? (float)pchg->valuedouble : 0.0f;
        si->sub[subidx].price = sprice && cJSON_IsNumber(sprice) ? (float)sprice->valuedouble : 0.0f;
        si->sub[subidx].currency = scurrency && cJSON_IsString(scurrency) ? strdup(scurrency->valuestring) : NULL;

        subidx++;
    }
    si->sub_count = subidx;

    cJSON_Delete(root);
    return 0;
}

int stocks_get(const char *command, struct stocks_info *si) {
    FILE *f = popen(command, "r");
    if (!f) return -1;

    char *json = NULL;
    size_t len = 0;
    char buf[512];
    while (fgets(buf, sizeof(buf), f)) {
        size_t buflen = strlen(buf);
        char *new_json = realloc(json, len + buflen + 1);
        if (!new_json) { free(json); pclose(f); return -1; }
        json = new_json;
        memcpy(json + len, buf, buflen);
        len += buflen;
        json[len] = '\0';
    }
    pclose(f);

    int ret = json_parse_stocks(json, si);
    free(json);
    return ret;
}
