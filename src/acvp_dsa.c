/** @file */
/*****************************************************************************
* Copyright (c) 2016-2017, Cisco Systems, Inc.
* All rights reserved.

* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice,
*    this list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
* USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "acvp.h"
#include "acvp_lcl.h"
#include "parson.h"

static ACVP_RESULT acvp_dsa_set_sha (ACVP_DSA_TC *stc, unsigned char *sha) {

    if (!strncmp((char *) sha, "SHA2-1", 6)) {
        stc->sha = ACVP_DSA_SHA1;
        return ACVP_SUCCESS;
    }
    if (!strncmp((char *) sha, "SHA2-224", 8)) {
        stc->sha = ACVP_DSA_SHA224;
        return ACVP_SUCCESS;
    }
    if (!strncmp((char *) sha, "SHA2-256", 8)) {
        stc->sha = ACVP_DSA_SHA256;
        return ACVP_SUCCESS;
    }
    if (!strncmp((char *) sha, "SHA2-384", 8)) {
        stc->sha = ACVP_DSA_SHA384;
        return ACVP_SUCCESS;
    }
    if (!strncmp((char *) sha, "SHA2-512", 8)) {
        stc->sha = ACVP_DSA_SHA512;
        return ACVP_SUCCESS;
    }
    if (!strncmp((char *) sha, "SHA2-512/224", 12)) {
        stc->sha = ACVP_DSA_SHA512_224;
        return ACVP_SUCCESS;
    }
    if (!strncmp((char *) sha, "SHA2-512/256", 12)) {
        stc->sha = ACVP_DSA_SHA512_256;
        return ACVP_SUCCESS;
    }
    return ACVP_INVALID_ARG;
}
static ACVP_RESULT acvp_dsa_keygen_init_tc (ACVP_CTX *ctx,
                                     ACVP_DSA_TC *stc,
                                     unsigned int tc_id,
                                     ACVP_CIPHER alg_id,
                                     unsigned int num,
                                     unsigned char *index,
                                     int l,
                                     int n) {

    stc->l = l;
    stc->n = n;

    if (stc->l == 0) {
        return ACVP_INVALID_ARG;
    }
    if (stc->n == 0) {
        return ACVP_INVALID_ARG;
    }
    
    stc->p = calloc(1, ACVP_DSA_MAX_STRING);
    if (!stc->p) { return ACVP_MALLOC_FAIL; }
    stc->q = calloc(1, ACVP_DSA_MAX_STRING);
    if (!stc->q) { return ACVP_MALLOC_FAIL; }
    stc->g = calloc(1, ACVP_DSA_MAX_STRING);
    if (!stc->g) { return ACVP_MALLOC_FAIL; }
    stc->x = calloc(1, ACVP_DSA_MAX_STRING);
    if (!stc->x) { return ACVP_MALLOC_FAIL; }
    stc->y = calloc(1, ACVP_DSA_MAX_STRING);
    if (!stc->y) { return ACVP_MALLOC_FAIL; }

    return ACVP_SUCCESS;
}
static ACVP_RESULT acvp_dsa_siggen_init_tc (ACVP_CTX *ctx,
                                     ACVP_DSA_TC *stc,
                                     unsigned int tc_id,
                                     ACVP_CIPHER alg_id,
                                     unsigned int num,
                                     unsigned char *index,
                                     int l,
                                     int n,
                                     unsigned char *sha,
                                     char *msg) {
    ACVP_RESULT rv;

    stc->l = l;
    stc->n = n;
    
    stc->p = calloc(1, ACVP_DSA_MAX_STRING);
    if (!stc->p) { return ACVP_MALLOC_FAIL; }
    stc->q = calloc(1, ACVP_DSA_MAX_STRING);
    if (!stc->q) { return ACVP_MALLOC_FAIL; }
    stc->g = calloc(1, ACVP_DSA_MAX_STRING);
    if (!stc->g) { return ACVP_MALLOC_FAIL; }
    stc->r = calloc(1, ACVP_DSA_MAX_STRING);
    if (!stc->r) { return ACVP_MALLOC_FAIL; }
    stc->s = calloc(1, ACVP_DSA_MAX_STRING);
    if (!stc->s) { return ACVP_MALLOC_FAIL; }
    stc->y = calloc(1, ACVP_DSA_MAX_STRING);
    if (!stc->y) { return ACVP_MALLOC_FAIL; }

    if (stc->l == 0) {
        return ACVP_INVALID_ARG;
    }
    if (stc->n == 0) {
        return ACVP_INVALID_ARG;
    }

    rv = acvp_dsa_set_sha(stc, sha);
    if (rv != ACVP_SUCCESS) {
        ACVP_LOG_ERR("Bad SHA value");
        return rv;
    }

    stc->msg = calloc(1, ACVP_DSA_PQG_MAX);
    if (!stc->msg) { return ACVP_MALLOC_FAIL; }

    rv = acvp_hexstr_to_bin(msg, stc->msg, ACVP_DSA_PQG_MAX, &(stc->msglen));
    if (rv != ACVP_SUCCESS) {
        ACVP_LOG_ERR("Hex conversion failure (msg)");
        return rv;
    }
    return ACVP_SUCCESS;
}

static ACVP_RESULT acvp_dsa_sigver_init_tc (ACVP_CTX *ctx,
                                     ACVP_DSA_TC *stc,
                                     unsigned int tc_id,
                                     ACVP_CIPHER alg_id,
                                     unsigned int num,
                                     unsigned char *index,
                                     int l,
                                     int n,
                                     unsigned char *sha,
                                     char *p,
                                     char *q,
                                     char *g,
                                     char *r,
                                     char *s,
                                     char *y,
                                     char *msg) {
    ACVP_RESULT rv;

    stc->l = l;
    stc->n = n;

    if (stc->l == 0) {
        return ACVP_INVALID_ARG;
    }
    if (stc->n == 0) {
        return ACVP_INVALID_ARG;
    }

    rv = acvp_dsa_set_sha(stc, sha);
    if (rv != ACVP_SUCCESS) {
        ACVP_LOG_ERR("Bad SHA value");
        return rv;
    }
    stc->msg = calloc(1, ACVP_DSA_MAX_STRING);
    if (!stc->msg) { return ACVP_MALLOC_FAIL; }

    stc->p = calloc(1, ACVP_DSA_MAX_STRING);
    if (!stc->p) { return ACVP_MALLOC_FAIL; }
    stc->q = calloc(1, ACVP_DSA_MAX_STRING);
    if (!stc->q) { return ACVP_MALLOC_FAIL; }
    stc->g = calloc(1, ACVP_DSA_MAX_STRING);
    if (!stc->g) { return ACVP_MALLOC_FAIL; }
    stc->r = calloc(1, ACVP_DSA_MAX_STRING);
    if (!stc->r) { return ACVP_MALLOC_FAIL; }
    stc->s = calloc(1, ACVP_DSA_MAX_STRING);
    if (!stc->s) { return ACVP_MALLOC_FAIL; }
    stc->y = calloc(1, ACVP_DSA_MAX_STRING);
    if (!stc->y) { return ACVP_MALLOC_FAIL; }

    rv = acvp_hexstr_to_bin(msg, stc->msg, ACVP_DSA_MAX_STRING, &(stc->msglen));
    if (rv != ACVP_SUCCESS) {
        ACVP_LOG_ERR("Hex conversion failure (msg)");
        return rv;
    }
    rv = acvp_hexstr_to_bin(p, stc->p, ACVP_DSA_MAX_STRING, &(stc->p_len));
    if (rv != ACVP_SUCCESS) {
        ACVP_LOG_ERR("Hex conversion failure (p)");
        return rv;
    }
    rv = acvp_hexstr_to_bin(q, stc->q, ACVP_DSA_MAX_STRING, &(stc->q_len));
    if (rv != ACVP_SUCCESS) {
        ACVP_LOG_ERR("Hex conversion failure (q)");
        return rv;
    }
    rv = acvp_hexstr_to_bin(g, stc->g, ACVP_DSA_MAX_STRING, &(stc->g_len));
    if (rv != ACVP_SUCCESS) {
        ACVP_LOG_ERR("Hex conversion failure (g)");
        return rv;
    }
    rv = acvp_hexstr_to_bin(r, stc->r, ACVP_DSA_MAX_STRING, &(stc->r_len));
    if (rv != ACVP_SUCCESS) {
        ACVP_LOG_ERR("Hex conversion failure (r)");
        return rv;
    }
    rv = acvp_hexstr_to_bin(s, stc->s, ACVP_DSA_MAX_STRING, &(stc->s_len));
    if (rv != ACVP_SUCCESS) {
        ACVP_LOG_ERR("Hex conversion failure (s)");
        return rv;
    }
    rv = acvp_hexstr_to_bin(y, stc->y, ACVP_DSA_MAX_STRING, &(stc->y_len));
    if (rv != ACVP_SUCCESS) {
        ACVP_LOG_ERR("Hex conversion failure (y)");
        return rv;
    }

    return ACVP_SUCCESS;
}

static ACVP_RESULT acvp_dsa_pqgver_init_tc (ACVP_CTX *ctx,
                                     ACVP_DSA_TC *stc,
                                     unsigned int tc_id,
                                     ACVP_CIPHER alg_id,
                                     unsigned int num,
                                     int l,
                                     int n,
                                     int c,
                                     unsigned char *index,
                                     unsigned char *sha,
                                     char *p,
                                     char *q,
                                     char *g,
                                     char *seed,
                                     unsigned int pqg) {
    ACVP_RESULT rv;

    stc->l = l;
    stc->n = n;
    stc->c = c;
    stc->pqg = pqg;

    if (stc->l == 0) {
        return ACVP_INVALID_ARG;
    }
    if (stc->n == 0) {
        return ACVP_INVALID_ARG;
    }

    rv = acvp_dsa_set_sha(stc, sha);
    if (rv != ACVP_SUCCESS) {
        ACVP_LOG_ERR("Bad SHA value");
        return rv;
    }
    stc->seed = calloc(1, ACVP_DSA_MAX_STRING);
    if (!stc->seed) { return ACVP_MALLOC_FAIL; }

    stc->p = calloc(1, ACVP_DSA_MAX_STRING);
    if (!stc->p) { return ACVP_MALLOC_FAIL; }
    stc->q = calloc(1, ACVP_DSA_MAX_STRING);
    if (!stc->q) { return ACVP_MALLOC_FAIL; }
    stc->g = calloc(1, ACVP_DSA_MAX_STRING);
    if (!stc->g) { return ACVP_MALLOC_FAIL; }

    stc->r = calloc(1, ACVP_DSA_MAX_STRING);
    if (!stc->r) { return ACVP_MALLOC_FAIL; }
    stc->s = calloc(1, ACVP_DSA_MAX_STRING);
    if (!stc->s) { return ACVP_MALLOC_FAIL; }
    stc->y = calloc(1, ACVP_DSA_MAX_STRING);
    if (!stc->y) { return ACVP_MALLOC_FAIL; }

    stc->index = -1;
    if (index) {
        stc->index = strtol((char *) index, NULL, 16);
    }
    if (seed) {
        rv = acvp_hexstr_to_bin(seed, stc->seed, ACVP_DSA_MAX_STRING, &(stc->seedlen));
        if (rv != ACVP_SUCCESS) {
            ACVP_LOG_ERR("Hex conversion failure (seed)");
            return rv;
        }
    }
    
    rv = acvp_hexstr_to_bin(p, stc->p, ACVP_DSA_MAX_STRING, &(stc->p_len));
    if (rv != ACVP_SUCCESS) {
        ACVP_LOG_ERR("Hex conversion failure (p)");
        return rv;
    }
    rv = acvp_hexstr_to_bin(q, stc->q, ACVP_DSA_MAX_STRING, &(stc->q_len));
    if (rv != ACVP_SUCCESS) {
        ACVP_LOG_ERR("Hex conversion failure (q)");
        return rv;
    }

    if (g) {
        rv = acvp_hexstr_to_bin(g, stc->g, ACVP_DSA_MAX_STRING, &(stc->g_len));
        if (rv != ACVP_SUCCESS) {
            ACVP_LOG_ERR("Hex conversion failure (g)");
            return rv;
        }
    }
    return ACVP_SUCCESS;
}

static ACVP_RESULT acvp_dsa_pqggen_init_tc (ACVP_CTX *ctx,
                                     ACVP_DSA_TC *stc,
                                     unsigned int tc_id,
                                     ACVP_CIPHER alg_id,
                                     unsigned int gpq,
                                     unsigned char *index,
                                     int l,
                                     int n,
                                     unsigned char *sha,
                                     char *p,
                                     char *q,
                                     char *seed) {
    ACVP_RESULT rv;

    stc->l = l;
    stc->n = n;

    if (stc->l == 0) {
        return ACVP_INVALID_ARG;
    }
    if (stc->n == 0) {
        return ACVP_INVALID_ARG;
    }
    rv = acvp_dsa_set_sha(stc, sha);
    if (rv != ACVP_SUCCESS) {
        ACVP_LOG_ERR("Bad SHA value");
        return rv;
    }

    stc->p = calloc(1, ACVP_DSA_PQG_MAX);
    if (!stc->p) { return ACVP_MALLOC_FAIL; }
    stc->q = calloc(1, ACVP_DSA_PQG_MAX);
    if (!stc->q) { return ACVP_MALLOC_FAIL; }
    stc->g = calloc(1, ACVP_DSA_PQG_MAX);
    if (!stc->g) { return ACVP_MALLOC_FAIL; }
    stc->seed = calloc(1, ACVP_DSA_SEED_MAX);
    if (!stc->seed) { return ACVP_MALLOC_FAIL; }

    stc->gen_pq = gpq;
    switch (gpq) {
    case ACVP_DSA_CANONICAL:
        stc->index = strtol((char *) index, NULL, 16);
        rv = acvp_hexstr_to_bin(seed, stc->seed, ACVP_DSA_SEED_MAX, &(stc->seedlen));
        if (rv != ACVP_SUCCESS) {
            ACVP_LOG_ERR("Hex conversion failure (seed)");
            return rv;
        }
        rv = acvp_hexstr_to_bin(p, stc->p, ACVP_DSA_MAX_STRING, &(stc->p_len));
        if (rv != ACVP_SUCCESS) {
            ACVP_LOG_ERR("Hex conversion failure (p)");
            return rv;
        }
        rv = acvp_hexstr_to_bin(q, stc->q, ACVP_DSA_MAX_STRING, &(stc->q_len));
        if (rv != ACVP_SUCCESS) {
            ACVP_LOG_ERR("Hex conversion failure (q)");
            return rv;
        }
        break;
    case ACVP_DSA_UNVERIFIABLE:
    case ACVP_DSA_PROBABLE:
    case ACVP_DSA_PROVABLE:
        break;
    default:
        ACVP_LOG_ERR("Invalid GPQ argument %d", gpq);
        return ACVP_INVALID_ARG;
        break;
    }
    return ACVP_SUCCESS;
}

/*
 * After the test case has been processed by the DUT, the results
 * need to be JSON formated to be included in the vector set results
 * file that will be uploaded to the server.  This routine handles
 * the JSON processing for a single test case.
 */
static ACVP_RESULT acvp_dsa_output_tc (ACVP_CTX *ctx, ACVP_DSA_TC *stc, JSON_Object *r_tobj) {
    ACVP_RESULT rv;
    char *tmp = NULL;

    switch (stc->mode) {
    case ACVP_DSA_MODE_PQGGEN:
        switch (stc->gen_pq) {
        case ACVP_DSA_CANONICAL:
        case ACVP_DSA_UNVERIFIABLE:
            tmp = calloc(ACVP_DSA_PQG_MAX+1, sizeof(char));
            if (!tmp) {
                ACVP_LOG_ERR("Unable to malloc in acvp_dsa_output_tc");
                return ACVP_MALLOC_FAIL;
            }
            rv = acvp_bin_to_hexstr(stc->g, stc->g_len, tmp, ACVP_DSA_PQG_MAX);
            if (rv != ACVP_SUCCESS) {
                ACVP_LOG_ERR("hex conversion failure (g)");
                goto err;
            }
            json_object_set_string(r_tobj, "g", (const char *)tmp);
            memset(tmp, 0x0, ACVP_DSA_PQG_MAX+1);
            break;
        case ACVP_DSA_PROBABLE:
        case ACVP_DSA_PROVABLE:
            tmp = calloc(ACVP_DSA_PQG_MAX+1, sizeof(char));
            if (!tmp) {
                ACVP_LOG_ERR("Unable to malloc in acvp_dsa_output_tc");
                return ACVP_MALLOC_FAIL;
            }
            rv = acvp_bin_to_hexstr(stc->p, stc->p_len, tmp, ACVP_DSA_PQG_MAX);
            if (rv != ACVP_SUCCESS) {
                ACVP_LOG_ERR("hex conversion failure (p)");
                goto err;
            }
            json_object_set_string(r_tobj, "p", (const char *)tmp);
            memset(tmp, 0x0, ACVP_DSA_PQG_MAX+1);
            
            rv = acvp_bin_to_hexstr(stc->q, stc->q_len, tmp, ACVP_DSA_PQG_MAX);
            if (rv != ACVP_SUCCESS) {
                ACVP_LOG_ERR("hex conversion failure (q)");
                goto err;
            }
            json_object_set_string(r_tobj, "q", (const char *)tmp);
            
            memset(tmp, 0x0, ACVP_DSA_SEED_MAX);
            rv = acvp_bin_to_hexstr(stc->seed, stc->seedlen, tmp, ACVP_DSA_SEED_MAX);
            if (rv != ACVP_SUCCESS) {
                ACVP_LOG_ERR("hex conversion failure (p)");
                return rv;
            }
            json_object_set_string(r_tobj, "domainSeed", tmp);
            json_object_set_number(r_tobj, "counter", stc->counter);
            break;
        default:
            ACVP_LOG_ERR("Invalid mode argument %d", stc->mode);
            return ACVP_INVALID_ARG;
            break;
        }
        break;
    case ACVP_DSA_MODE_SIGGEN:
        tmp = calloc(ACVP_DSA_PQG_MAX+1, sizeof(char));
        if (!tmp) {
            ACVP_LOG_ERR("Unable to malloc in acvp_dsa_output_tc");
            return ACVP_MALLOC_FAIL;
        }

        rv = acvp_bin_to_hexstr(stc->p, stc->p_len, tmp, ACVP_DSA_PQG_MAX);
        if (rv != ACVP_SUCCESS) {
            ACVP_LOG_ERR("hex conversion failure (p)");
            goto err;
        }
        json_object_set_string(r_tobj, "p", (const char *)tmp);
        memset(tmp, 0x0, ACVP_DSA_PQG_MAX);
        
        rv = acvp_bin_to_hexstr(stc->q, stc->q_len, tmp, ACVP_DSA_PQG_MAX);
        if (rv != ACVP_SUCCESS) {
            ACVP_LOG_ERR("hex conversion failure (q)");
            goto err;
        }
        json_object_set_string(r_tobj, "q", (const char *)tmp);
        memset(tmp, 0x0, ACVP_DSA_PQG_MAX);
        
        rv = acvp_bin_to_hexstr(stc->g, stc->g_len, tmp, ACVP_DSA_PQG_MAX);
        if (rv != ACVP_SUCCESS) {
            ACVP_LOG_ERR("hex conversion failure (g)");
            goto err;
        }
        json_object_set_string(r_tobj, "g", (const char *)tmp);
        memset(tmp, 0x0, ACVP_DSA_PQG_MAX);
        
        rv = acvp_bin_to_hexstr(stc->y, stc->y_len, tmp, ACVP_DSA_PQG_MAX);
        if (rv != ACVP_SUCCESS) {
            ACVP_LOG_ERR("hex conversion failure (y)");
            goto err;
        }
        json_object_set_string(r_tobj, "y", (const char *)tmp);
        memset(tmp, 0x0, ACVP_DSA_PQG_MAX);
        
        rv = acvp_bin_to_hexstr(stc->r, stc->r_len, tmp, ACVP_DSA_PQG_MAX);
        if (rv != ACVP_SUCCESS) {
            ACVP_LOG_ERR("hex conversion failure (r)");
            goto err;
        }
        json_object_set_string(r_tobj, "r", (const char *)tmp);
        memset(tmp, 0x0, ACVP_DSA_PQG_MAX);
        
        rv = acvp_bin_to_hexstr(stc->s, stc->s_len, tmp, ACVP_DSA_PQG_MAX);
        if (rv != ACVP_SUCCESS) {
            ACVP_LOG_ERR("hex conversion failure (s)");
            goto err;
        }
        json_object_set_string(r_tobj, "s", (const char *)tmp);
        memset(tmp, 0x0, ACVP_DSA_PQG_MAX);
        
        break;
    case ACVP_DSA_MODE_SIGVER:
        json_object_set_string(r_tobj, "result", stc->result > 0 ? "passed" : "failed");
        break;
    case ACVP_DSA_MODE_KEYGEN:
        tmp = calloc(ACVP_DSA_PQG_MAX+1, sizeof(char));
        if (!tmp) {
            ACVP_LOG_ERR("Unable to malloc in acvp_dsa_output_tc");
            return ACVP_MALLOC_FAIL;
        }
        rv = acvp_bin_to_hexstr(stc->p, stc->p_len, tmp, ACVP_DSA_PQG_MAX);
        if (rv != ACVP_SUCCESS) {
            ACVP_LOG_ERR("hex conversion failure (p)");
            goto err;
        }
        json_object_set_string(r_tobj, "p", (const char *)tmp);
        memset(tmp, 0x0, ACVP_DSA_PQG_MAX);
        
        rv = acvp_bin_to_hexstr(stc->q, stc->q_len, tmp, ACVP_DSA_PQG_MAX);
        if (rv != ACVP_SUCCESS) {
            ACVP_LOG_ERR("hex conversion failure (q)");
            goto err;
        }
        json_object_set_string(r_tobj, "q", (const char *)tmp);
        memset(tmp, 0x0, ACVP_DSA_PQG_MAX);
        
        rv = acvp_bin_to_hexstr(stc->g, stc->g_len, tmp, ACVP_DSA_PQG_MAX);
        if (rv != ACVP_SUCCESS) {
            ACVP_LOG_ERR("hex conversion failure (g)");
            goto err;
        }
        json_object_set_string(r_tobj, "g", (const char *)tmp);
        memset(tmp, 0x0, ACVP_DSA_PQG_MAX);
        
        rv = acvp_bin_to_hexstr(stc->y, stc->y_len, tmp, ACVP_DSA_PQG_MAX);
        if (rv != ACVP_SUCCESS) {
            ACVP_LOG_ERR("hex conversion failure (y)");
            goto err;
        }
        json_object_set_string(r_tobj, "y", (const char *)tmp);
        memset(tmp, 0x0, ACVP_DSA_PQG_MAX);
        
        rv = acvp_bin_to_hexstr(stc->x, stc->x_len, tmp, ACVP_DSA_PQG_MAX);
        if (rv != ACVP_SUCCESS) {
            ACVP_LOG_ERR("hex conversion failure (x)");
            goto err;
        }
        json_object_set_string(r_tobj, "x", (const char *)tmp);
        memset(tmp, 0x0, ACVP_DSA_PQG_MAX);

        break;
    case ACVP_DSA_MODE_PQGVER:
        json_object_set_string(r_tobj, "result", stc->result > 0 ? "passed" : "failed");
        break;
    default:
        break;
    }

err:
    free(tmp);
    return ACVP_SUCCESS;
}

/*
 * This function simply releases the data associated with
 * a test case.
 */
static ACVP_RESULT acvp_dsa_release_tc (ACVP_DSA_TC *stc) {

    if (stc->p) free(stc->p);
    if (stc->q) free(stc->q);
    if (stc->g) free(stc->g);
    if (stc->x) free(stc->x);
    if (stc->y) free(stc->y);
    if (stc->r) free(stc->r);
    if (stc->s) free(stc->s);
    if (stc->seed) free(stc->seed);
    if (stc->msg) free(stc->msg);
    return ACVP_SUCCESS;
}

ACVP_RESULT acvp_dsa_keygen_handler (ACVP_CTX *ctx, ACVP_TEST_CASE tc, ACVP_CAPS_LIST *cap,
                                     JSON_Array *r_tarr, JSON_Object *groupobj)
{
    unsigned char *index = NULL;
    JSON_Array *tests;
    JSON_Value *testval;
    JSON_Object *testobj = NULL;
    JSON_Value *r_tval = NULL; /* Response testval */
    int j, t_cnt, tc_id, l, n;
    ACVP_RESULT rv = ACVP_SUCCESS;
    JSON_Value *mval;
    JSON_Object *mobj = NULL;
    unsigned int num = 0;
    ACVP_DSA_TC *stc;

    l = json_object_get_number(groupobj, "l");
    if (!l) {
        ACVP_LOG_ERR("Failed to include l. ");
        return ACVP_MISSING_ARG;
    }

    n = json_object_get_number(groupobj, "n");
    if (!n) {
        ACVP_LOG_ERR("Failed to include n. ");
        return ACVP_MISSING_ARG;
    }

    ACVP_LOG_INFO("             l: %d", l);
    ACVP_LOG_INFO("             n: %d", n);

    tests = json_object_get_array(groupobj, "tests");
    if (!tests) {
        ACVP_LOG_ERR("Failed to include tests. ");
        return ACVP_MISSING_ARG;
    }

    t_cnt = json_array_get_count(tests);
    if (!t_cnt) {
        ACVP_LOG_ERR("Failed to include tests in array. ");
        return ACVP_MISSING_ARG;
    }

    stc = tc.tc.dsa;

    for (j = 0; j < t_cnt; j++) {
        ACVP_LOG_INFO("Found new DSA KeyGen test vector...");
        testval = json_array_get_value(tests, j);
        testobj = json_value_get_object(testval);

        tc_id = (unsigned int) json_object_get_number(testobj, "tcId");
        if (!tc_id) {
            ACVP_LOG_ERR("Failed to include tc_id. ");
            return ACVP_MISSING_ARG;
        }

        ACVP_LOG_INFO("       Test case: %d", j);
        ACVP_LOG_INFO("            tcId: %d", tc_id);

        /*
         * Setup the test case data that will be passed down to
         * the crypto module.
         * TODO: this does mallocs, we can probably do the mallocs once for
         *       the entire vector set to be more efficient
         */
        acvp_dsa_keygen_init_tc(ctx, stc, tc_id, stc->cipher, num, index, l, n);

        /* Process the current DSA test vector... */
        rv = (cap->crypto_handler)(&tc);
        if (rv != ACVP_SUCCESS) {
            ACVP_LOG_ERR("crypto module failed the operation");
            return ACVP_CRYPTO_MODULE_FAIL;
        }

        mval = json_value_init_object();
        mobj = json_value_get_object(mval);
        json_object_set_number(mobj, "tcId", tc_id);
        /*
         * Output the test case results using JSON
         */
        rv = acvp_dsa_output_tc(ctx, stc, mobj);
        if (rv != ACVP_SUCCESS) {
            ACVP_LOG_ERR("JSON output failure in DSA module");
            return rv;
        }

        /* Append the test response value to array */
        json_array_append_value(r_tarr, mval);
        acvp_dsa_release_tc(stc);
    }
    /* Append the test response value to array */
    json_array_append_value(r_tarr, r_tval);
    return rv;
    return ACVP_SUCCESS;
}

ACVP_RESULT acvp_dsa_pqggen_handler (ACVP_CTX *ctx, ACVP_TEST_CASE tc, ACVP_CAPS_LIST *cap,
                                     JSON_Array *r_tarr, JSON_Object *groupobj) {
    unsigned char *gen_pq = NULL, *sha = NULL, *index = NULL, *gen_g = NULL;
    JSON_Array *tests;
    JSON_Value *testval;
    JSON_Object *testobj = NULL;
    JSON_Value *r_tval = NULL; /* Response testval */
    JSON_Object *r_tobj = NULL; /* Response testobj */
    int j, t_cnt, tc_id;
    ACVP_RESULT rv = ACVP_SUCCESS;
    unsigned gpq = 0, n, l;
    char *p = NULL, *q = NULL, *seed = NULL;
    ACVP_DSA_TC *stc;

    gen_pq = (unsigned char *) json_object_get_string(groupobj, "pqMode");
    gen_g = (unsigned char *) json_object_get_string(groupobj, "gMode");
    if (!gen_pq && !gen_g) {
        ACVP_LOG_ERR("Failed to include either gen_pq or gen_g. ");
        return ACVP_MISSING_ARG;
    }
    l = json_object_get_number(groupobj, "l");
    if (!l) {
        ACVP_LOG_ERR("Failed to include l. ");
        return ACVP_MISSING_ARG;
    }

    n = json_object_get_number(groupobj, "n");
    if (!n) {
        ACVP_LOG_ERR("Failed to include n. ");
        return ACVP_MISSING_ARG;
    }
    sha = (unsigned char *) json_object_get_string(groupobj, "hashAlg");
    if (!sha) {
        ACVP_LOG_ERR("Failed to include hashAlg. ");
        return ACVP_MISSING_ARG;
    }

    if (gen_pq) {
        ACVP_LOG_INFO("         genPQ: %s", gen_pq);
    }
    if (gen_g) {
        ACVP_LOG_INFO("          genG: %s", gen_g);
    }
    ACVP_LOG_INFO("             l: %d", l);
    ACVP_LOG_INFO("             n: %d", n);
    ACVP_LOG_INFO("           sha: %s", sha);

    tests = json_object_get_array(groupobj, "tests");
    if (!tests) {
        ACVP_LOG_ERR("Failed to include tests. ");
        return ACVP_MISSING_ARG;
    }

    t_cnt = json_array_get_count(tests);
    if (!t_cnt) {
        ACVP_LOG_ERR("Failed to include tests in array. ");
        return ACVP_MISSING_ARG;
    }

    stc = tc.tc.dsa;

    for (j = 0; j < t_cnt; j++) {
        ACVP_LOG_INFO("Found new DSA PQGGen test vector...");
        testval = json_array_get_value(tests, j);
        testobj = json_value_get_object(testval);

        tc_id = (unsigned int) json_object_get_number(testobj, "tcId");
        if (!tc_id) {
            ACVP_LOG_ERR("Failed to include tc_id. ");
            return ACVP_MISSING_ARG;
        }

        ACVP_LOG_INFO("       Test case: %d", j);
        ACVP_LOG_INFO("            tcId: %d", tc_id);
        if (gen_g) {
            if (!strncmp((char *) gen_g, "canonical", 9)) {
                p = (char *) json_object_get_string(testobj, "p");
                if (!p) {
                    ACVP_LOG_ERR("Failed to include p. ");
                    return ACVP_MISSING_ARG;
                }

                q = (char *) json_object_get_string(testobj, "q");
                if (!q) {
                    ACVP_LOG_ERR("Failed to include q. ");
                    return ACVP_MISSING_ARG;
                }

                seed = (char *) json_object_get_string(testobj, "domainSeed");
                if (!seed) {
                    ACVP_LOG_ERR("Failed to include domainSeed. ");
                    return ACVP_MISSING_ARG;
                }

                index = (unsigned char *) json_object_get_string(testobj, "index");
                if (!index) {
                    ACVP_LOG_ERR("Failed to include index. ");
                    return ACVP_MISSING_ARG;
                }

                gpq = ACVP_DSA_CANONICAL;
                ACVP_LOG_INFO("               p: %s", p);
                ACVP_LOG_INFO("               q: %s", q);
                ACVP_LOG_INFO("            seed: %s", seed);
                ACVP_LOG_INFO("           index: %s", index);
            }
        }

        /* find the mode */
        if (gen_g) {
            if (!strncmp((char *) gen_g, "unverifiable", 12)) {
                p = (char *) json_object_get_string(testobj, "p");
                if (!p) {
                    ACVP_LOG_ERR("Failed to include p. ");
                    return ACVP_MISSING_ARG;
                }

                q = (char *) json_object_get_string(testobj, "q");
                if (!q) {
                    ACVP_LOG_ERR("Failed to include q. ");
                    return ACVP_MISSING_ARG;
                }

                gpq = ACVP_DSA_UNVERIFIABLE;
                ACVP_LOG_INFO("               p: %s", p);
                ACVP_LOG_INFO("               q: %s", q);
            }
        }
        if (gen_pq) {
            if (!strncmp((char *) gen_pq, "probable", 8)) {
                gpq = ACVP_DSA_PROBABLE;
            }
        }
        if (gen_pq) {
            if (!strncmp((char *) gen_pq, "provable", 8)) {
                gpq = ACVP_DSA_PROVABLE;
            }
        }
        if (gpq == 0) {
            ACVP_LOG_ERR("Failed to include valid gen_pq. ");
            return ACVP_UNSUPPORTED_OP;
        }

        /*
         * Setup the test case data that will be passed down to
         * the crypto module.
         * TODO: this does mallocs, we can probably do the mallocs once for
         *       the entire vector set to be more efficient
         */

        switch (gpq) {
        case ACVP_DSA_PROBABLE:
        case ACVP_DSA_PROVABLE:
            /*
             * Create a new test case in the response
             */
            r_tval = json_value_init_object();
            r_tobj = json_value_get_object(r_tval);
            json_object_set_number(r_tobj, "tcId", tc_id);

            acvp_dsa_pqggen_init_tc(ctx, stc, tc_id, stc->cipher, gpq, index, l, n, sha, p, q, seed);

            /* Process the current DSA test vector... */
            rv = (cap->crypto_handler)(&tc);
            if (rv != ACVP_SUCCESS) {
                ACVP_LOG_ERR("crypto module failed the operation");
                return ACVP_CRYPTO_MODULE_FAIL;
            }

            /*
             * Output the test case results using JSON
             */
            rv = acvp_dsa_output_tc(ctx, stc, r_tobj);
            if (rv != ACVP_SUCCESS) {
                ACVP_LOG_ERR("JSON output failure in DSA module");
                return rv;
            }

            stc->seedlen = 0;
            stc->counter = 0;
            stc->seed = 0;
            break;

        case ACVP_DSA_CANONICAL:
        case ACVP_DSA_UNVERIFIABLE:
            /*
             * Create a new test case in the response
             */
            r_tval = json_value_init_object();
            r_tobj = json_value_get_object(r_tval);
            json_object_set_number(r_tobj, "tcId", tc_id);

            /* Process the current DSA test vector... */
            acvp_dsa_pqggen_init_tc(ctx, stc, tc_id, stc->cipher, gpq, index, l, n, sha, p, q, seed);
            rv = (cap->crypto_handler)(&tc);
            if (rv != ACVP_SUCCESS) {
                ACVP_LOG_ERR("crypto module failed the operation");
                return ACVP_CRYPTO_MODULE_FAIL;
            }

            /*
             * Output the test case results using JSON
             */
            rv = acvp_dsa_output_tc(ctx, stc, r_tobj);
            if (rv != ACVP_SUCCESS) {
                ACVP_LOG_ERR("JSON output failure in DSA module");
                return rv;
            }
            break;
        default:
            ACVP_LOG_ERR("Invalid DSA PQGGen mode");
            rv = ACVP_INVALID_ARG;
            break;
        }
        json_array_append_value(r_tarr, r_tval);
        acvp_dsa_release_tc(stc);
    }
    return rv;
}

ACVP_RESULT acvp_dsa_siggen_handler (ACVP_CTX *ctx, ACVP_TEST_CASE tc, ACVP_CAPS_LIST *cap,
                                     JSON_Array *r_tarr, JSON_Object *groupobj) {
    unsigned char *sha = NULL, *index = NULL;
    char *msg = NULL;
    JSON_Array *tests;
    JSON_Value *testval;
    JSON_Object *testobj = NULL;
    JSON_Value *r_tval = NULL; /* Response testval */
    int j, t_cnt, tc_id, l, n;
    ACVP_RESULT rv = ACVP_SUCCESS;
    JSON_Value *mval;
    JSON_Object *mobj = NULL;
    unsigned int num = 0;
    ACVP_DSA_TC *stc;

    l = json_object_get_number(groupobj, "l");
    if (!l) {
        ACVP_LOG_ERR("Failed to include l. ");
        return ACVP_MISSING_ARG;
    }

    n = json_object_get_number(groupobj, "n");
    if (!n) {
        ACVP_LOG_ERR("Failed to include n. ");
        return ACVP_MISSING_ARG;
    }

    sha = (unsigned char *) json_object_get_string(groupobj, "hashAlg");
    if (!sha) {
        ACVP_LOG_ERR("Failed to include hashAlg. ");
        return ACVP_MISSING_ARG;
    }

    ACVP_LOG_INFO("             l: %d", l);
    ACVP_LOG_INFO("             n: %d", n);
    ACVP_LOG_INFO("           sha: %s", sha);

    tests = json_object_get_array(groupobj, "tests");
    if (!tests) {
        ACVP_LOG_ERR("Failed to include tests. ");
        return ACVP_MISSING_ARG;
    }

    t_cnt = json_array_get_count(tests);
    if (!t_cnt) {
        ACVP_LOG_ERR("Failed to include tests in array. ");
        return ACVP_MISSING_ARG;
    }

    stc = tc.tc.dsa;

    for (j = 0; j < t_cnt; j++) {
        ACVP_LOG_INFO("Found new DSA SigGen test vector...");
        testval = json_array_get_value(tests, j);
        testobj = json_value_get_object(testval);

        tc_id = (unsigned int) json_object_get_number(testobj, "tcId");
        if (!tc_id) {
            ACVP_LOG_ERR("Failed to include tc_id. ");
            return ACVP_MISSING_ARG;
        }

        msg = (char *) json_object_get_string(testobj, "message");
        if (!msg) {
            ACVP_LOG_ERR("Failed to include message. ");
            return ACVP_MISSING_ARG;
        }

        ACVP_LOG_INFO("       Test case: %d", j);
        ACVP_LOG_INFO("            tcId: %d", tc_id);
        ACVP_LOG_INFO("             msg: %s", msg);

        /*
         * Setup the test case data that will be passed down to
         * the crypto module.
         * TODO: this does mallocs, we can probably do the mallocs once for
         *       the entire vector set to be more efficient
         */
        acvp_dsa_siggen_init_tc(ctx, stc, tc_id, stc->cipher, num, index, l, n, sha, msg);

        /* Process the current DSA test vector... */
        rv = (cap->crypto_handler)(&tc);
        if (rv != ACVP_SUCCESS) {
            ACVP_LOG_ERR("crypto module failed the operation");
            return ACVP_CRYPTO_MODULE_FAIL;
        }

        mval = json_value_init_object();
        mobj = json_value_get_object(mval);
        json_object_set_number(mobj, "tcId", tc_id);
        /*
         * Output the test case results using JSON
         */
        rv = acvp_dsa_output_tc(ctx, stc, mobj);
        if (rv != ACVP_SUCCESS) {
            ACVP_LOG_ERR("JSON output failure in DSA module");
            return rv;
        }
        acvp_dsa_release_tc(stc);
        /* Append the test response value to array */
        json_array_append_value(r_tarr, mval);
    }
    /* Append the test response value to array */
    json_array_append_value(r_tarr, r_tval);
    return rv;
}

ACVP_RESULT acvp_dsa_pqgver_handler (ACVP_CTX *ctx, ACVP_TEST_CASE tc, ACVP_CAPS_LIST *cap,
                                     JSON_Array *r_tarr, JSON_Object *groupobj) {
    unsigned char *sha = NULL, *index = NULL;
    char *g = NULL, *pqmode = NULL, *gmode = NULL, *seed = NULL;
    JSON_Array *tests;
    JSON_Value *testval;
    JSON_Object *testobj = NULL;
    JSON_Value *r_tval = NULL; /* Response testval */
    int j, t_cnt, tc_id, l, n, c, gpq = 0;
    ACVP_RESULT rv = ACVP_SUCCESS;
    JSON_Value *mval;
    JSON_Object *mobj = NULL;
    unsigned int num = 0;
    char *p = NULL, *q = NULL;
    ACVP_DSA_TC *stc;

    l = json_object_get_number(groupobj, "l");
    if (!l) {
        ACVP_LOG_ERR("Failed to include l. ");
        return ACVP_MISSING_ARG;
    }

    n = json_object_get_number(groupobj, "n");
    if (!n) {
        ACVP_LOG_ERR("Failed to include n. ");
        return ACVP_MISSING_ARG;
    }

    sha = (unsigned char *) json_object_get_string(groupobj, "hashAlg");
    if (!sha) {
        ACVP_LOG_ERR("Failed to include hashAlg. ");
        return ACVP_MISSING_ARG;
    }
    gmode = (char *) json_object_get_string(groupobj, "gMode");
    pqmode = (char *) json_object_get_string(groupobj, "pqMode");
    if (!pqmode && !gmode) {
        ACVP_LOG_ERR("Failed to include either pqMode or gMode. ");
        return ACVP_MISSING_ARG;
    }

    ACVP_LOG_INFO("             l: %d", l);
    ACVP_LOG_INFO("             n: %d", n);
    ACVP_LOG_INFO("           sha: %s", sha);
    ACVP_LOG_INFO("         gmode: %s", gmode);
    ACVP_LOG_INFO("        pqmode: %s", pqmode);

    tests = json_object_get_array(groupobj, "tests");
    if (!tests) {
        ACVP_LOG_ERR("Failed to include tests. ");
        return ACVP_MISSING_ARG;
    }

    t_cnt = json_array_get_count(tests);
    if (!t_cnt) {
        ACVP_LOG_ERR("Failed to include tests in array. ");
        return ACVP_MISSING_ARG;
    }

    stc = tc.tc.dsa;

    for (j = 0; j < t_cnt; j++) {
        ACVP_LOG_INFO("Found new DSA PQGVer test vector...");
        testval = json_array_get_value(tests, j);
        testobj = json_value_get_object(testval);

        tc_id = (unsigned int) json_object_get_number(testobj, "tcId");
        if (!tc_id) {
            ACVP_LOG_ERR("Failed to include tc_id. ");
            return ACVP_MISSING_ARG;
        }

        seed = (char *) json_object_get_string(testobj, "domainSeed");
        c = json_object_get_number(testobj, "counter");
        index = (unsigned char *) json_object_get_string(testobj, "index");

        p = (char *) json_object_get_string(testobj, "p");
        if (!p) {
            ACVP_LOG_ERR("Failed to include p. ");
            return ACVP_MISSING_ARG;
        }

        q = (char *) json_object_get_string(testobj, "q");
        if (!q) {
            ACVP_LOG_ERR("Failed to include q. ");
            return ACVP_MISSING_ARG;
        }

        g = (char *) json_object_get_string(testobj, "g");

        ACVP_LOG_INFO("       Test case: %d", j);
        ACVP_LOG_INFO("            tcId: %d", tc_id);
        ACVP_LOG_INFO("            seed: %s", seed);
        ACVP_LOG_INFO("               p: %s", p);
        ACVP_LOG_INFO("               q: %s", q);
        ACVP_LOG_INFO("               g: %s", g);
        ACVP_LOG_INFO("          pqMode: %s", pqmode);
        ACVP_LOG_INFO("           gMode: %s", gmode);
        ACVP_LOG_INFO("               c: %d", c);
        ACVP_LOG_INFO("           index: %s", index);

        /* find the mode */
        if (gmode) {
            if (!strncmp(gmode, "canonical", 9)) {
                gpq = ACVP_DSA_CANONICAL;
            }
        }
        if (pqmode) {
            if (!strncmp(pqmode, "probable", 8)) {
                gpq = ACVP_DSA_PROBABLE;
            }
        }
        if (gpq == 0) {
            ACVP_LOG_ERR("Failed to include valid gen_pq. ");
            return ACVP_UNSUPPORTED_OP;
        }

        if (gpq == ACVP_DSA_CANONICAL) {
            if (!index) {
                ACVP_LOG_ERR("Failed to include index. ");
                return ACVP_MISSING_ARG;
            }
            if (!g) {
                ACVP_LOG_ERR("Failed to include q. ");
                return ACVP_MISSING_ARG;
            }
        }

        if (gpq == ACVP_DSA_PROBABLE) {
            if (!seed) {
                ACVP_LOG_ERR("Failed to include seed. ");
                return ACVP_MISSING_ARG;
            }
            if (!c) {
                ACVP_LOG_ERR("Failed to include counter. ");
                return ACVP_MISSING_ARG;
            }
        }

        /*
         * Setup the test case data that will be passed down to
         * the crypto module.
         * TODO: this does mallocs, we can probably do the mallocs once for
         *       the entire vector set to be more efficient
         */
        acvp_dsa_pqgver_init_tc(ctx, stc, tc_id, stc->cipher, num, 
                                l, n, c, index, sha, p, q, g, seed, gpq);

        /* Process the current DSA test vector... */
        rv = (cap->crypto_handler)(&tc);
        if (rv != ACVP_SUCCESS) {
            ACVP_LOG_ERR("crypto module failed the operation");
            return ACVP_CRYPTO_MODULE_FAIL;
        }

        mval = json_value_init_object();
        mobj = json_value_get_object(mval);
        json_object_set_number(mobj, "tcId", tc_id);
        /*
         * Output the test case results using JSON
         */
        rv = acvp_dsa_output_tc(ctx, stc, mobj);
        if (rv != ACVP_SUCCESS) {
            ACVP_LOG_ERR("JSON output failure in DSA module");
            return rv;
        }
        acvp_dsa_release_tc(stc);

        /* Append the test response value to array */
        json_array_append_value(r_tarr, mval);
    }
    /* Append the test response value to array */
    json_array_append_value(r_tarr, r_tval);
    return rv;
}

ACVP_RESULT acvp_dsa_sigver_handler (ACVP_CTX *ctx, ACVP_TEST_CASE tc, ACVP_CAPS_LIST *cap,
                                     JSON_Array *r_tarr, JSON_Object *groupobj) {
    unsigned char *sha = NULL, *index = NULL;
    char *msg = NULL, *r = NULL, *s = NULL, *y = NULL, *g = NULL;
    JSON_Array *tests;
    JSON_Value *testval;
    JSON_Object *testobj = NULL;
    JSON_Value *r_tval = NULL; /* Response testval */
    int j, t_cnt, tc_id, l, n;
    ACVP_RESULT rv = ACVP_SUCCESS;
    JSON_Value *mval;
    JSON_Object *mobj = NULL;
    unsigned int num = 0;
    char *p = NULL, *q = NULL;
    ACVP_DSA_TC *stc;

    l = json_object_get_number(groupobj, "l");
    if (!l) {
        ACVP_LOG_ERR("Failed to include l. ");
        return ACVP_MISSING_ARG;
    }

    n = json_object_get_number(groupobj, "n");
    if (!n) {
        ACVP_LOG_ERR("Failed to include n. ");
        return ACVP_MISSING_ARG;
    }

    sha = (unsigned char *) json_object_get_string(groupobj, "hashAlg");
    if (!sha) {
        ACVP_LOG_ERR("Failed to include hashAlg. ");
        return ACVP_MISSING_ARG;
    }

    ACVP_LOG_INFO("             l: %d", l);
    ACVP_LOG_INFO("             n: %d", n);
    ACVP_LOG_INFO("           sha: %s", sha);

    tests = json_object_get_array(groupobj, "tests");
    if (!tests) {
        ACVP_LOG_ERR("Failed to include tests. ");
        return ACVP_MISSING_ARG;
    }

    t_cnt = json_array_get_count(tests);
    if (!t_cnt) {
        ACVP_LOG_ERR("Failed to include tests in array. ");
        return ACVP_MISSING_ARG;
    }

    stc = tc.tc.dsa;

    p = (char *)json_object_get_string(groupobj, "p");
    if (!p) {
        ACVP_LOG_ERR("Failed to include p. ");
        return ACVP_MISSING_ARG;
    }

    q = (char *)json_object_get_string(groupobj, "q");
    if (!q) {
        ACVP_LOG_ERR("Failed to include q. ");
        return ACVP_MISSING_ARG;
    }

    g = (char *)json_object_get_string(groupobj, "g");
    if (!g) {
        ACVP_LOG_ERR("Failed to include g. ");
        return ACVP_MISSING_ARG;
    }

    for (j = 0; j < t_cnt; j++) {
        ACVP_LOG_INFO("Found new DSA SigVer test vector...");
        testval = json_array_get_value(tests, j);
        testobj = json_value_get_object(testval);

        tc_id = json_object_get_number(testobj, "tcId");
        if (!tc_id) {
            ACVP_LOG_ERR("Failed to include tc_id. ");
            return ACVP_MISSING_ARG;
        }

        msg = (char *)json_object_get_string(testobj, "message");
        if (!msg) {
            ACVP_LOG_ERR("Failed to include message. ");
            return ACVP_MISSING_ARG;
        }
        r = (char *)json_object_get_string(testobj, "r");
        if (!r) {
            ACVP_LOG_ERR("Failed to include r. ");
            return ACVP_MISSING_ARG;
        }
        s = (char *)json_object_get_string(testobj, "s");
        if (!s) {
            ACVP_LOG_ERR("Failed to include s. ");
            return ACVP_MISSING_ARG;
        }
        y = (char *)json_object_get_string(testobj, "y");
        if (!y) {
            ACVP_LOG_ERR("Failed to include y. ");
            return ACVP_MISSING_ARG;
        }

        ACVP_LOG_INFO("       Test case: %d", j);
        ACVP_LOG_INFO("            tcId: %d", tc_id);
        ACVP_LOG_INFO("             msg: %s", msg);
        ACVP_LOG_INFO("               p: %s", p);
        ACVP_LOG_INFO("               q: %s", q);
        ACVP_LOG_INFO("               g: %s", g);
        ACVP_LOG_INFO("               r: %s", r);
        ACVP_LOG_INFO("               s: %s", s);
        ACVP_LOG_INFO("               y: %s", y);

        /*
         * Setup the test case data that will be passed down to
         * the crypto module.
         * TODO: this does mallocs, we can probably do the mallocs once for
         *       the entire vector set to be more efficient
         */
        acvp_dsa_sigver_init_tc(ctx, stc, tc_id, stc->cipher, num, index, 
                                l, n, sha, p, q, g, r, s, y, msg);

        /* Process the current DSA test vector... */
        rv = (cap->crypto_handler)(&tc);
        if (rv != ACVP_SUCCESS) {
            ACVP_LOG_ERR("crypto module failed the operation");
            return ACVP_CRYPTO_MODULE_FAIL;
        }

        mval = json_value_init_object();
        mobj = json_value_get_object(mval);
        json_object_set_number(mobj, "tcId", tc_id);
        /*
         * Output the test case results using JSON
         */
        rv = acvp_dsa_output_tc(ctx, stc, mobj);
        if (rv != ACVP_SUCCESS) {
            ACVP_LOG_ERR("JSON output failure in DSA module");
            return rv;
        }
        acvp_dsa_release_tc(stc);

        /* Append the test response value to array */
        json_array_append_value(r_tarr, mval);
    }
    /* Append the test response value to array */
    json_array_append_value(r_tarr, r_tval);
    return rv;
}


ACVP_RESULT acvp_dsa_pqgver_kat_handler (ACVP_CTX *ctx, JSON_Object *obj)
{
    JSON_Value *groupval;
    JSON_Object *groupobj = NULL;
    JSON_Value *r_vs_val = NULL;
    JSON_Object *r_vs = NULL;
    JSON_Array *r_tarr = NULL; /* Response testarray */
    JSON_Value *reg_arry_val = NULL;
    JSON_Array *reg_arry = NULL;
    JSON_Object *reg_obj = NULL;
    JSON_Array *groups;
    ACVP_CAPS_LIST *cap;
    ACVP_DSA_TC stc;
    ACVP_TEST_CASE tc;
    ACVP_RESULT rv;
    const char *alg_str = json_object_get_string(obj, "algorithm");
    ACVP_CIPHER alg_id;
    char *json_result;
    unsigned int g_cnt, i;

    if (!alg_str) {
        ACVP_LOG_ERR("unable to parse 'algorithm' from JSON");
        return (ACVP_MALFORMED_JSON);
    }

    /*
     * Get a reference to the abstracted test case
     */
    tc.tc.dsa = &stc;
    memset(&stc, 0x0, sizeof(ACVP_DSA_TC));

    /*
     * Get the crypto module handler for DSA mode
     */
    alg_id = ACVP_DSA_PQGVER;
    cap = acvp_locate_cap_entry(ctx, alg_id);
    if (!cap) {
        ACVP_LOG_ERR("ACVP server requesting unsupported capability");
        return (ACVP_UNSUPPORTED_OP);
    }

    /*
     * Create ACVP array for response
     */
    rv = acvp_create_array(&reg_obj, &reg_arry_val, &reg_arry);
    if (rv != ACVP_SUCCESS) {
        ACVP_LOG_ERR("Failed to create JSON response struct. ");
        return (rv);
    }

    /*
     * Start to build the JSON response
     * TODO: This code will likely be common to all the algorithms, need to move this
     */
    if (ctx->kat_resp) {
        json_value_free(ctx->kat_resp);
    }
    ctx->kat_resp = reg_arry_val;
    r_vs_val = json_value_init_object();
    r_vs = json_value_get_object(r_vs_val);

    json_object_set_number(r_vs, "vsId", ctx->vs_id);
    json_object_set_string(r_vs, "algorithm", alg_str);
    json_object_set_value(r_vs, "testResults", json_value_init_array());
    r_tarr = json_object_get_array(r_vs, "testResults");

    groups = json_object_get_array(obj, "testGroups");
    if (!groups) {
        ACVP_LOG_ERR("Failed to include testGroups. ");
        return ACVP_MISSING_ARG;
    }

    g_cnt = json_array_get_count(groups);

    stc.cipher = alg_id;
    for (i = 0; i < g_cnt; i++) {
        groupval = json_array_get_value(groups, i);
        groupobj = json_value_get_object(groupval);

        stc.mode = ACVP_DSA_MODE_PQGVER;

        ACVP_LOG_INFO("    Test group: %d", i);

        rv = acvp_dsa_pqgver_handler(ctx, tc, cap, r_tarr, groupobj);
        if (rv != ACVP_SUCCESS) {
            return (rv);
        }
    }
    memset(&stc, 0x0, sizeof(ACVP_DSA_TC));
    json_array_append_value(reg_arry, r_vs_val);
    json_result = json_serialize_to_string_pretty(ctx->kat_resp);
    if (!json_result) {
        ACVP_LOG_ERR("JSON unable to be serialized");
        return ACVP_JSON_ERR;
    }

    if (ctx->debug == ACVP_LOG_LVL_VERBOSE) {
        printf("\n\n%s\n\n", json_result);
    } else {
        ACVP_LOG_INFO("\n\n%s\n\n", json_result);
    }
    json_free_serialized_string(json_result);

    return ACVP_SUCCESS;
}

ACVP_RESULT acvp_dsa_pqggen_kat_handler (ACVP_CTX *ctx, JSON_Object *obj) {
    JSON_Value *groupval;
    JSON_Object *groupobj = NULL;
    JSON_Value *r_vs_val = NULL;
    JSON_Object *r_vs = NULL;
    JSON_Array *r_tarr = NULL; /* Response testarray */
    JSON_Value *reg_arry_val = NULL;
    JSON_Array *reg_arry = NULL;
    JSON_Object *reg_obj = NULL;
    JSON_Array *groups;
    ACVP_CAPS_LIST *cap;
    ACVP_DSA_TC stc;
    ACVP_TEST_CASE tc;
    ACVP_RESULT rv;
    const char *alg_str = json_object_get_string(obj, "algorithm");
    ACVP_CIPHER alg_id;
    char *json_result;
    unsigned int g_cnt, i;

    if (!alg_str) {
        ACVP_LOG_ERR("unable to parse 'algorithm' from JSON");
        return (ACVP_MALFORMED_JSON);
    }

    /*
     * Get a reference to the abstracted test case
     */
    tc.tc.dsa = &stc;
    memset(&stc, 0x0, sizeof(ACVP_DSA_TC));

    /*
     * Get the crypto module handler for DSA mode
     */
    alg_id = ACVP_DSA_PQGGEN;
    cap = acvp_locate_cap_entry(ctx, alg_id);
    if (!cap) {
        ACVP_LOG_ERR("ACVP server requesting unsupported capability");
        return (ACVP_UNSUPPORTED_OP);
    }

    /*
     * Create ACVP array for response
     */
    rv = acvp_create_array(&reg_obj, &reg_arry_val, &reg_arry);
    if (rv != ACVP_SUCCESS) {
        ACVP_LOG_ERR("Failed to create JSON response struct. ");
        return (rv);
    }

    /*
     * Start to build the JSON response
     * TODO: This code will likely be common to all the algorithms, need to move this
     */
    if (ctx->kat_resp) {
        json_value_free(ctx->kat_resp);
    }
    ctx->kat_resp = reg_arry_val;
    r_vs_val = json_value_init_object();
    r_vs = json_value_get_object(r_vs_val);

    json_object_set_number(r_vs, "vsId", ctx->vs_id);
    json_object_set_string(r_vs, "algorithm", alg_str);
    json_object_set_value(r_vs, "testResults", json_value_init_array());
    r_tarr = json_object_get_array(r_vs, "testResults");

    groups = json_object_get_array(obj, "testGroups");
    if (!groups) {
        ACVP_LOG_ERR("Failed to include testGroups. ");
        return ACVP_MISSING_ARG;
    }
    g_cnt = json_array_get_count(groups);

    stc.cipher = alg_id;
    for (i = 0; i < g_cnt; i++) {
        groupval = json_array_get_value(groups, i);
        groupobj = json_value_get_object(groupval);

        stc.mode = ACVP_DSA_MODE_PQGGEN;

        ACVP_LOG_INFO("    Test group: %d", i);

         rv = acvp_dsa_pqggen_handler(ctx, tc, cap, r_tarr, groupobj);
         if (rv != ACVP_SUCCESS) {
            return (rv);
         }
    }

    memset(&stc, 0x0, sizeof(ACVP_DSA_TC));
    json_array_append_value(reg_arry, r_vs_val);
    json_result = json_serialize_to_string_pretty(ctx->kat_resp);
    if (ctx->debug == ACVP_LOG_LVL_VERBOSE) {
        printf("\n\n%s\n\n", json_result);
    } else {
        ACVP_LOG_INFO("\n\n%s\n\n", json_result);
    }
    json_free_serialized_string(json_result);

    return ACVP_SUCCESS;
}

ACVP_RESULT acvp_dsa_siggen_kat_handler (ACVP_CTX *ctx, JSON_Object *obj)
{
    JSON_Value *groupval;
    JSON_Object *groupobj = NULL;
    JSON_Value *r_vs_val = NULL;
    JSON_Object *r_vs = NULL;
    JSON_Array *r_tarr = NULL; /* Response testarray */
    JSON_Value *reg_arry_val = NULL;
    JSON_Array *reg_arry = NULL;
    JSON_Object *reg_obj = NULL;
    JSON_Array *groups;
    ACVP_CAPS_LIST *cap;
    ACVP_DSA_TC stc;
    ACVP_TEST_CASE tc;
    ACVP_RESULT rv;
    const char *alg_str = json_object_get_string(obj, "algorithm");
    ACVP_CIPHER alg_id;
    char *json_result;
    unsigned int g_cnt, i;

    if (!alg_str) {
        ACVP_LOG_ERR("unable to parse 'algorithm' from JSON");
        return (ACVP_MALFORMED_JSON);
    }

    /*
     * Get a reference to the abstracted test case
     */
    tc.tc.dsa = &stc;
    memset(&stc, 0x0, sizeof(ACVP_DSA_TC));

    /*
     * Get the crypto module handler for DSA mode
     */
    alg_id = ACVP_DSA_SIGGEN;
    cap = acvp_locate_cap_entry(ctx, alg_id);
    if (!cap) {
        ACVP_LOG_ERR("ACVP server requesting unsupported capability");
        return (ACVP_UNSUPPORTED_OP);
    }

    /*
     * Create ACVP array for response
     */
    rv = acvp_create_array(&reg_obj, &reg_arry_val, &reg_arry);
    if (rv != ACVP_SUCCESS) {
        ACVP_LOG_ERR("Failed to create JSON response struct. ");
        return (rv);
    }

    /*
     * Start to build the JSON response
     * TODO: This code will likely be common to all the algorithms, need to move this
     */
    if (ctx->kat_resp) {
        json_value_free(ctx->kat_resp);
    }
    ctx->kat_resp = reg_arry_val;
    r_vs_val = json_value_init_object();
    r_vs = json_value_get_object(r_vs_val);

    json_object_set_number(r_vs, "vsId", ctx->vs_id);
    json_object_set_string(r_vs, "algorithm", alg_str);
    json_object_set_value(r_vs, "testResults", json_value_init_array());
    r_tarr = json_object_get_array(r_vs, "testResults");

    groups = json_object_get_array(obj, "testGroups");
    if (!groups) {
        ACVP_LOG_ERR("Failed to include testGroups. ");
        return ACVP_MISSING_ARG;
    }
    g_cnt = json_array_get_count(groups);

    stc.cipher = alg_id;
    for (i = 0; i < g_cnt; i++) {
        groupval = json_array_get_value(groups, i);
        groupobj = json_value_get_object(groupval);

        stc.mode = ACVP_DSA_MODE_SIGGEN;

        ACVP_LOG_INFO("    Test group: %d", i);

        rv = acvp_dsa_siggen_handler(ctx, tc, cap, r_tarr, groupobj);
        if (rv != ACVP_SUCCESS) {
            return (rv);
        }
    }

    memset(&stc, 0x0, sizeof(ACVP_DSA_TC));
    json_array_append_value(reg_arry, r_vs_val);
    json_result = json_serialize_to_string_pretty(ctx->kat_resp);

/* TODO: we should check the return code */
    if (ctx->debug == ACVP_LOG_LVL_VERBOSE) {
        printf("\n\n%s\n\n", json_result);
    } else {
        ACVP_LOG_INFO("\n\n%s\n\n", json_result);
    }
    json_free_serialized_string(json_result);

    return ACVP_SUCCESS;
}

ACVP_RESULT acvp_dsa_keygen_kat_handler (ACVP_CTX *ctx, JSON_Object *obj)
{
    JSON_Value *groupval;
    JSON_Object *groupobj = NULL;
    JSON_Value *r_vs_val = NULL;
    JSON_Object *r_vs = NULL;
    JSON_Array *r_tarr = NULL; /* Response testarray */
    JSON_Value *reg_arry_val = NULL;
    JSON_Array *reg_arry = NULL;
    JSON_Object *reg_obj = NULL;
    JSON_Array *groups;
    ACVP_CAPS_LIST *cap;
    ACVP_DSA_TC stc;
    ACVP_TEST_CASE tc;
    ACVP_RESULT rv;
    const char *alg_str = json_object_get_string(obj, "algorithm");
    ACVP_CIPHER alg_id;
    char *json_result;
    unsigned int g_cnt, i;

    if (!alg_str) {
        ACVP_LOG_ERR("unable to parse 'algorithm' from JSON");
        return (ACVP_MALFORMED_JSON);
    }

    /*
     * Get a reference to the abstracted test case
     */
    tc.tc.dsa = &stc;
    memset(&stc, 0x0, sizeof(ACVP_DSA_TC));

    /*
     * Get the crypto module handler for DSA mode
     */
    alg_id = ACVP_DSA_KEYGEN;
    cap = acvp_locate_cap_entry(ctx, alg_id);
    if (!cap) {
        ACVP_LOG_ERR("ACVP server requesting unsupported capability");
        return (ACVP_UNSUPPORTED_OP);
    }

    /*
     * Create ACVP array for response
     */
    rv = acvp_create_array(&reg_obj, &reg_arry_val, &reg_arry);
    if (rv != ACVP_SUCCESS) {
        ACVP_LOG_ERR("Failed to create JSON response struct. ");
        return (rv);
    }

    /*
     * Start to build the JSON response
     * TODO: This code will likely be common to all the algorithms, need to move this
     */
    if (ctx->kat_resp) {
        json_value_free(ctx->kat_resp);
    }
    ctx->kat_resp = reg_arry_val;
    r_vs_val = json_value_init_object();
    r_vs = json_value_get_object(r_vs_val);

    json_object_set_number(r_vs, "vsId", ctx->vs_id);
    json_object_set_string(r_vs, "algorithm", alg_str);
    json_object_set_value(r_vs, "testResults", json_value_init_array());
    r_tarr = json_object_get_array(r_vs, "testResults");

    groups = json_object_get_array(obj, "testGroups");
    if (!groups) {
        ACVP_LOG_ERR("Failed to include testGroups. ");
        return ACVP_MISSING_ARG;
    }
    g_cnt = json_array_get_count(groups);

    stc.cipher = alg_id;
    for (i = 0; i < g_cnt; i++) {
        groupval = json_array_get_value(groups, i);
        groupobj = json_value_get_object(groupval);

        stc.mode = ACVP_DSA_MODE_KEYGEN;

        ACVP_LOG_INFO("    Test group: %d", i);

        rv = acvp_dsa_keygen_handler(ctx, tc, cap, r_tarr, groupobj);
        if (rv != ACVP_SUCCESS) {
            return (rv);
        }
    }

    memset(&stc, 0x0, sizeof(ACVP_DSA_TC));
    json_array_append_value(reg_arry, r_vs_val);
    json_result = json_serialize_to_string_pretty(ctx->kat_resp);

/* TODO: we should check the return code */
    if (ctx->debug == ACVP_LOG_LVL_VERBOSE) {
        printf("\n\n%s\n\n", json_result);
    } else {
        ACVP_LOG_INFO("\n\n%s\n\n", json_result);
    }
    json_free_serialized_string(json_result);

    return ACVP_SUCCESS;
}

ACVP_RESULT acvp_dsa_sigver_kat_handler (ACVP_CTX *ctx, JSON_Object *obj)
{
    JSON_Value *groupval;
    JSON_Object *groupobj = NULL;
    JSON_Value *r_vs_val = NULL;
    JSON_Object *r_vs = NULL;
    JSON_Array *r_tarr = NULL; /* Response testarray */
    JSON_Value *reg_arry_val = NULL;
    JSON_Array *reg_arry = NULL;
    JSON_Object *reg_obj = NULL;
    JSON_Array *groups;
    ACVP_CAPS_LIST *cap;
    ACVP_DSA_TC stc;
    ACVP_TEST_CASE tc;
    ACVP_RESULT rv;
    const char *alg_str = json_object_get_string(obj, "algorithm");
    ACVP_CIPHER alg_id;
    char *json_result;
    unsigned int g_cnt, i;

    if (!alg_str) {
        ACVP_LOG_ERR("unable to parse 'algorithm' from JSON");
        return (ACVP_MALFORMED_JSON);
    }

    /*
     * Get a reference to the abstracted test case
     */
    tc.tc.dsa = &stc;
    memset(&stc, 0x0, sizeof(ACVP_DSA_TC));

    /*
     * Get the crypto module handler for DSA mode
     */
    alg_id = ACVP_DSA_SIGVER;
    cap = acvp_locate_cap_entry(ctx, alg_id);
    if (!cap) {
        ACVP_LOG_ERR("ACVP server requesting unsupported capability");
        return (ACVP_UNSUPPORTED_OP);
    }

    /*
     * Create ACVP array for response
     */
    rv = acvp_create_array(&reg_obj, &reg_arry_val, &reg_arry);
    if (rv != ACVP_SUCCESS) {
        ACVP_LOG_ERR("Failed to create JSON response struct. ");
        return (rv);
    }

    /*
     * Start to build the JSON response
     * TODO: This code will likely be common to all the algorithms, need to move this
     */
    if (ctx->kat_resp) {
        json_value_free(ctx->kat_resp);
    }
    ctx->kat_resp = reg_arry_val;
    r_vs_val = json_value_init_object();
    r_vs = json_value_get_object(r_vs_val);

    json_object_set_number(r_vs, "vsId", ctx->vs_id);
    json_object_set_string(r_vs, "algorithm", alg_str);
    json_object_set_value(r_vs, "testResults", json_value_init_array());
    r_tarr = json_object_get_array(r_vs, "testResults");

    groups = json_object_get_array(obj, "testGroups");
    if (!groups) {
        ACVP_LOG_ERR("Failed to include testGroups. ");
        return ACVP_MISSING_ARG;
    }
    g_cnt = json_array_get_count(groups);

    stc.cipher = alg_id;
    for (i = 0; i < g_cnt; i++) {
        groupval = json_array_get_value(groups, i);
        groupobj = json_value_get_object(groupval);

        stc.mode = ACVP_DSA_MODE_SIGVER;

        ACVP_LOG_INFO("    Test group: %d", i);

        rv = acvp_dsa_sigver_handler(ctx, tc, cap, r_tarr, groupobj);
        if (rv != ACVP_SUCCESS) {
            return (rv);
        }
    }

    memset(&stc, 0x0, sizeof(ACVP_DSA_TC));
    json_array_append_value(reg_arry, r_vs_val);
    json_result = json_serialize_to_string_pretty(ctx->kat_resp);
    if (!json_result) {
        ACVP_LOG_ERR("JSON unable to be serialized");
        return ACVP_JSON_ERR;
    }

    if (ctx->debug == ACVP_LOG_LVL_VERBOSE) {
        printf("\n\n%s\n\n", json_result);
    } else {
        ACVP_LOG_INFO("\n\n%s\n\n", json_result);
    }
    json_free_serialized_string(json_result);

    return ACVP_SUCCESS;
}

ACVP_RESULT acvp_dsa_kat_handler (ACVP_CTX *ctx, JSON_Object *obj)
{
    const char *mode = json_object_get_string(obj, "mode");

    if (!ctx) {
        ACVP_LOG_ERR("CTX is NULL. ");
        return ACVP_NO_CTX;
    }

    if (!obj) {
        ACVP_LOG_ERR("OBJ is NULL. ");
        return ACVP_MALFORMED_JSON;
    }

    if (!mode) {
        ACVP_LOG_ERR("Failed to include mode. ");
        return ACVP_MISSING_ARG;
    }

    if (!strncmp(mode, ACVP_ALG_DSA_PQGGEN, 6)) {
        return (acvp_dsa_pqggen_kat_handler(ctx, obj));
    }
    if (!strncmp(mode, ACVP_ALG_DSA_PQGVER, 6)) {
        return (acvp_dsa_pqgver_kat_handler(ctx, obj));
    }
    if (!strncmp(mode, ACVP_ALG_DSA_SIGGEN, 6)) {
        return (acvp_dsa_siggen_kat_handler(ctx, obj));
    }
    if (!strncmp(mode, ACVP_ALG_DSA_SIGVER, 6)) {
        return (acvp_dsa_sigver_kat_handler(ctx, obj));
    }
    if (!strncmp(mode, ACVP_ALG_DSA_KEYGEN, 6)) {
        return (acvp_dsa_keygen_kat_handler(ctx, obj));
    }
    return ACVP_INVALID_ARG;
}
