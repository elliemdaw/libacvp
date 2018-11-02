/*****************************************************************************
* Copyright (c) 2017, Cisco Systems, Inc.
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

/*
 * After the test case has been processed by the DUT, the results
 * need to be JSON formated to be included in the vector set results
 * file that will be uploaded to the server.  This routine handles
 * the JSON processing for a single test case.
 */
static ACVP_RESULT acvp_kdf108_output_tc (ACVP_CTX *ctx,
                                          ACVP_KDF108_TC *stc,
                                          JSON_Object *tc_rsp) {
    ACVP_RESULT rv = ACVP_SUCCESS;
    char *tmp = NULL;

    tmp = calloc(ACVP_KDF108_KEYOUT_STR_MAX + 1, sizeof(char));
    if (!tmp) {
        ACVP_LOG_ERR("Unable to malloc");
        return ACVP_MALLOC_FAIL;
    }

    /*
     * Length check
     */
    if (stc->key_out_len > ACVP_KDF108_KEYOUT_BYTE_MAX) {
        ACVP_LOG_ERR("stc->key_out_len > ACVP_KDF108_KEYOUT_BYTE_MAX(%u)",
                     ACVP_KDF108_KEYOUT_BYTE_MAX);
        rv = ACVP_INVALID_ARG;
        goto end;
    }

    rv = acvp_bin_to_hexstr(stc->key_out, stc->key_out_len,
                            tmp, ACVP_KDF108_KEYOUT_STR_MAX);
    if (rv != ACVP_SUCCESS) {
        ACVP_LOG_ERR("hex conversion failure (key_out)");
        goto end;
    }
    json_object_set_string(tc_rsp, "keyOut", tmp);

    free(tmp);

    tmp = calloc(ACVP_KDF108_FIXED_DATA_STR_MAX + 1, sizeof(char));
    if (!tmp) {
        ACVP_LOG_ERR("Unable to malloc");
        return ACVP_MALLOC_FAIL;
    }

    /*
     * Length check
     */
    if (stc->fixed_data_len > ACVP_KDF108_FIXED_DATA_BYTE_MAX) {
        ACVP_LOG_ERR("stc->fixed_data_len > ACVP_KDF108_FIXED_DATA_BYTE_MAX(%u)",
                     ACVP_KDF108_FIXED_DATA_BYTE_MAX);
        rv = ACVP_INVALID_ARG;
        goto end;
    }

    rv = acvp_bin_to_hexstr(stc->fixed_data, stc->fixed_data_len,
                            tmp, ACVP_KDF108_FIXED_DATA_STR_MAX);
    if (rv != ACVP_SUCCESS) {
        ACVP_LOG_ERR("hex conversion failure (fixed_data)");
        goto end;
    }
    json_object_set_string(tc_rsp, "fixedData", tmp);

end:
    if (tmp) free(tmp);

    return rv;
}

static ACVP_RESULT acvp_kdf108_init_tc (ACVP_CTX *ctx,
                                        ACVP_KDF108_TC *stc,
                                        unsigned int tc_id,
                                        ACVP_KDF108_MODE kdf_mode,
                                        ACVP_KDF108_MAC_MODE_VAL mac_mode,
                                        ACVP_KDF108_FIXED_DATA_ORDER_VAL counter_location,
                                        const char *key_in,
                                        const char *iv,
                                        int key_in_len,
                                        int key_out_len,
                                        int iv_len,
                                        int counter_len,
                                        int deferred
) {
    ACVP_RESULT rv;
    memset(stc, 0x0, sizeof(ACVP_KDF108_TC));

    // Allocate space for the key_in (binary)
    stc->key_in = calloc(key_in_len, sizeof(unsigned char));
    if (!stc->key_in) { return ACVP_MALLOC_FAIL; }

    // Convert key_in from hex string to binary
    rv = acvp_hexstr_to_bin(key_in, stc->key_in, key_in_len, NULL);
    if (rv != ACVP_SUCCESS) return rv;

    if (iv != NULL) {
        /*
         * Feedback mode.
         * Allocate space for the iv.
         */
        stc->iv = calloc(iv_len, sizeof(unsigned char));
        if (!stc->iv) { return ACVP_MALLOC_FAIL; }

        // Convert iv from hex string to binary
        rv = acvp_hexstr_to_bin(iv, stc->iv, iv_len, NULL);
        if (rv != ACVP_SUCCESS) return rv;
    }

    /*
     * Allocate space for the key_out
     * User supplies the data.
     */
    stc->key_out = calloc(key_out_len, sizeof(unsigned char));
    if (!stc->key_out) { return ACVP_MALLOC_FAIL; }

    /*
     * Allocate space for the fixed_data.
     * User supplies the data.
     */
    stc->fixed_data = calloc(ACVP_KDF108_FIXED_DATA_BYTE_MAX,
                             sizeof(unsigned char));
    if (!stc->fixed_data) { return ACVP_MALLOC_FAIL; }
    
    stc->tc_id = tc_id;
    stc->cipher = ACVP_KDF108;
    stc->mode = kdf_mode;
    stc->mac_mode = mac_mode;
    stc->counter_location = counter_location;
    stc->key_in_len = key_in_len;
    stc->key_out_len = key_out_len;
    stc->counter_len = counter_len;
    stc->deferred = deferred;
    
    return ACVP_SUCCESS;
}

/*
 * This function simply releases the data associated with
 * a test case.
 */
static ACVP_RESULT acvp_kdf108_release_tc (ACVP_KDF108_TC *stc) {
    if (stc->key_in) free(stc->key_in);
    if (stc->key_out) free(stc->key_out);
    if (stc->fixed_data) free(stc->fixed_data);
    if (stc->iv) free(stc->iv);
    
    memset(stc, 0x0, sizeof(ACVP_KDF108_TC));
    return ACVP_SUCCESS;
}

ACVP_RESULT acvp_kdf108_kat_handler (ACVP_CTX *ctx, JSON_Object *obj) {
    unsigned int tc_id;
    JSON_Value *groupval;
    JSON_Object *groupobj = NULL;
    JSON_Value *testval;
    JSON_Object *testobj = NULL;
    JSON_Array *groups;
    JSON_Array *tests;

    JSON_Value *reg_arry_val = NULL;
    JSON_Object *reg_obj = NULL;
    JSON_Array *reg_arry = NULL;

    int i, g_cnt;
    int j, t_cnt;

    JSON_Value *r_vs_val = NULL;
    JSON_Object *r_vs = NULL;
    JSON_Array *r_tarr = NULL, *r_garr = NULL; /* Response testarray, grouparray */
    JSON_Value *r_tval = NULL, *r_gval = NULL; /* Response testval, groupval */
    JSON_Object *r_tobj = NULL, *r_gobj = NULL; /* Response testobj, groupobj */

    ACVP_CAPS_LIST *cap;
    ACVP_KDF108_TC stc;
    ACVP_TEST_CASE tc;
    ACVP_RESULT rv;
    const char *alg_str = NULL;
    ACVP_CIPHER alg_id = 0;
    char *json_result;

    ACVP_KDF108_MODE kdf_mode = 0;
    ACVP_KDF108_MAC_MODE_VAL mac_mode = 0;
    ACVP_KDF108_FIXED_DATA_ORDER_VAL ctr_loc = 0;
    int key_out_bit_len = 0, key_out_len = 0, key_in_len = 0,
        iv_len = 0, ctr_len = 0, deferred = 0;
    const char *kdf_mode_str = NULL, *mac_mode_str = NULL, *key_in_str = NULL,
               *iv_str = NULL, *ctr_loc_str = NULL;

    if (!ctx) {
        ACVP_LOG_ERR("No ctx for handler operation");
        return ACVP_NO_CTX;
    }

    alg_str = json_object_get_string(obj, "algorithm");
    if (!alg_str) {
        ACVP_LOG_ERR("unable to parse 'algorithm' from JSON.");
        return (ACVP_MALFORMED_JSON);
    }
    if (strncmp(alg_str, "KDF", strlen("KDF"))) {
        ACVP_LOG_ERR("Invalid algorithm %s", alg_str);
        return ACVP_INVALID_ARG;
    }

    /*
     * Get a reference to the abstracted test case
     */
    tc.tc.kdf108 = &stc;
    alg_id = ACVP_KDF108;

    /*
     * Get the crypto module handler for this hash algorithm
     */
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
     */
    if (ctx->kat_resp) {
        json_value_free(ctx->kat_resp);
    }
    ctx->kat_resp = reg_arry_val;
    r_vs_val = json_value_init_object();
    r_vs = json_value_get_object(r_vs_val);

    json_object_set_number(r_vs, "vsId", ctx->vs_id);
    json_object_set_string(r_vs, "algorithm", alg_str);
    /*
     * create an array of response test groups
     */
    json_object_set_value(r_vs, "testGroups", json_value_init_array());
    r_garr = json_object_get_array(r_vs, "testGroups");

    groups = json_object_get_array(obj, "testGroups");
    g_cnt = json_array_get_count(groups);
    for (i = 0; i < g_cnt; i++) {
        int tgId = 0;
        groupval = json_array_get_value(groups, i);
        groupobj = json_value_get_object(groupval);
    
        /*
         * Create a new group in the response with the tgid
         * and an array of tests
         */
        r_gval = json_value_init_object();
        r_gobj = json_value_get_object(r_gval);
        tgId = json_object_get_number(groupobj, "tgId");
        if (!tgId) {
            ACVP_LOG_ERR("Missing tgid from server JSON groub obj");
            return ACVP_MALFORMED_JSON;
        }
        json_object_set_number(r_gobj, "tgId", tgId);
        json_object_set_value(r_gobj, "tests", json_value_init_array());
        r_tarr = json_object_get_array(r_gobj, "tests");

        kdf_mode_str = json_object_get_string(groupobj, "kdfMode");
        if (!kdf_mode_str) {
            ACVP_LOG_ERR("Failed to include kdfMode");
            return ACVP_MISSING_ARG;
        }

        /*
         * Determine the KDF108 mode to operate.
         * Compare using protocol specified strings.
         */
        if (strncmp(kdf_mode_str, ACVP_MODE_COUNTER,
                    strlen(ACVP_MODE_COUNTER)) == 0) {
            kdf_mode = ACVP_KDF108_MODE_COUNTER;
        } else if (strncmp(kdf_mode_str, ACVP_MODE_FEEDBACK,
                           strlen(ACVP_MODE_FEEDBACK)) == 0) {
            kdf_mode = ACVP_KDF108_MODE_FEEDBACK;
        } else if (strncmp(kdf_mode_str, ACVP_MODE_DPI,
                           strlen(ACVP_MODE_DPI)) == 0) {
            kdf_mode = ACVP_KDF108_MODE_DPI;
        } else {
            ACVP_LOG_ERR("Server JSON invalid kdfMode");
            return (ACVP_INVALID_ARG);
        }

        mac_mode_str = json_object_get_string(groupobj, "macMode");
        if (!mac_mode_str) {
            ACVP_LOG_ERR("Server JSON missing macMode");
            return ACVP_MISSING_ARG;
        }

        /*
         * Determine the mac mode to operate.
         * Compare using protocol specified strings.
         */
        if (strncmp(mac_mode_str, ACVP_ALG_HMAC_SHA1,
                    strlen(ACVP_ALG_HMAC_SHA1)) == 0) {
            mac_mode = ACVP_KDF108_MAC_MODE_HMAC_SHA1;
        } else if (strncmp(mac_mode_str, ACVP_ALG_HMAC_SHA2_224,
                           strlen(ACVP_ALG_HMAC_SHA2_224)) == 0) {
            mac_mode = ACVP_KDF108_MAC_MODE_HMAC_SHA224;
        } else if (strncmp(mac_mode_str, ACVP_ALG_HMAC_SHA2_256,
                           strlen(ACVP_ALG_HMAC_SHA2_256)) == 0) {
            mac_mode = ACVP_KDF108_MAC_MODE_HMAC_SHA256;
        } else if (strncmp(mac_mode_str, ACVP_ALG_HMAC_SHA2_384,
                           strlen(ACVP_ALG_HMAC_SHA2_384)) == 0) {
            mac_mode = ACVP_KDF108_MAC_MODE_HMAC_SHA384;
        } else if (strncmp(mac_mode_str, ACVP_ALG_HMAC_SHA2_512,
                           strlen(ACVP_ALG_HMAC_SHA2_512)) == 0) {
            mac_mode = ACVP_KDF108_MAC_MODE_HMAC_SHA512;
        } else if (strncmp(mac_mode_str, ACVP_ALG_CMAC_AES_128,
                           strlen(ACVP_ALG_CMAC_AES_128)) == 0) {
            mac_mode = ACVP_KDF108_MAC_MODE_CMAC_AES128;
        } else if (strncmp(mac_mode_str, ACVP_ALG_CMAC_AES_192,
                           strlen(ACVP_ALG_CMAC_AES_192)) == 0) {
            mac_mode = ACVP_KDF108_MAC_MODE_CMAC_AES192;
        } else if (strncmp(mac_mode_str, ACVP_ALG_CMAC_AES_256,
                           strlen(ACVP_ALG_CMAC_AES_256)) == 0) {
            mac_mode = ACVP_KDF108_MAC_MODE_CMAC_AES256;
        } else if (strncmp(mac_mode_str, ACVP_ALG_CMAC_TDES,
                           strlen(ACVP_ALG_CMAC_TDES)) == 0) {
            mac_mode = ACVP_KDF108_MAC_MODE_CMAC_TDES;
        } else {
            ACVP_LOG_ERR("Server JSON invalid macMode");
            return (ACVP_INVALID_ARG);
        }

        key_out_bit_len = json_object_get_number(groupobj, "keyOutLength");
        if (!key_out_bit_len || key_out_bit_len > ACVP_KDF108_KEYOUT_BIT_MAX) {
            ACVP_LOG_ERR("Server JSON invalid keyOutLength, (%d)", key_out_len);
            return (ACVP_INVALID_ARG);
        }
        // Get the keyout byte length  (+1 for overflow bits)
        key_out_len = (key_out_bit_len + 7) / 8;

        ctr_len = json_object_get_number(groupobj, "counterLength");
        if (kdf_mode == ACVP_KDF108_MODE_COUNTER) {
            /* Only check during counter mode */
            if (ctr_len != 8 && ctr_len != 16
                && ctr_len != 24 && ctr_len != 32) {
                    ACVP_LOG_ERR("Server JSON invalid counterLength, (%d)", ctr_len);
                    return ACVP_INVALID_ARG;
            }
        }

        ctr_loc_str = json_object_get_string(groupobj, "counterLocation");
        if (!ctr_loc_str) {
            ACVP_LOG_ERR("Server JSON missing counterLocation");
            return ACVP_MISSING_ARG;
        }

        /*
         * Determine the counter location.
         * Compare using protocol specified strings.
         */
        if (strncmp(ctr_loc_str, ACVP_FIXED_DATA_ORDER_AFTER_STR,
                    strlen(ACVP_FIXED_DATA_ORDER_AFTER_STR)) == 0) {
            ctr_loc = ACVP_KDF108_FIXED_DATA_ORDER_AFTER;
        } else if (strncmp(ctr_loc_str, ACVP_FIXED_DATA_ORDER_BEFORE_STR,
                           strlen(ACVP_FIXED_DATA_ORDER_BEFORE_STR)) == 0) {
            ctr_loc = ACVP_KDF108_FIXED_DATA_ORDER_BEFORE;
        } else if (strncmp(ctr_loc_str, ACVP_FIXED_DATA_ORDER_MIDDLE_STR,
                           strlen(ACVP_FIXED_DATA_ORDER_MIDDLE_STR)) == 0) {
            ctr_loc = ACVP_KDF108_FIXED_DATA_ORDER_MIDDLE;
        } else if (strncmp(ctr_loc_str, ACVP_FIXED_DATA_ORDER_NONE_STR,
                           strlen(ACVP_FIXED_DATA_ORDER_NONE_STR)) == 0) {
            ctr_loc = ACVP_KDF108_FIXED_DATA_ORDER_NONE;
        } else if (strncmp(ctr_loc_str, ACVP_FIXED_DATA_ORDER_BEFORE_ITERATOR_STR,
                           strlen(ACVP_FIXED_DATA_ORDER_BEFORE_ITERATOR_STR)) == 0) {
            ctr_loc = ACVP_KDF108_FIXED_DATA_ORDER_BEFORE_ITERATOR;
        } else {
            ACVP_LOG_ERR("Server JSON invalid counterLocation.");
            return (ACVP_INVALID_ARG);
        }

        /*
         * Log Test Group information...
         */
        ACVP_LOG_INFO("    Test group: %d", i);
        ACVP_LOG_INFO("       kdfMode: %s", kdf_mode_str);
        ACVP_LOG_INFO("       macMode: %s", mac_mode_str);
        ACVP_LOG_INFO("     keyOutLen: %d", key_out_bit_len);
        ACVP_LOG_INFO("    counterLen: %d", ctr_len);
        ACVP_LOG_INFO("    counterLoc: %s", ctr_loc_str);

        tests = json_object_get_array(groupobj, "tests");
        t_cnt = json_array_get_count(tests);
        for (j = 0; j < t_cnt; j++) {
            ACVP_LOG_INFO("Found new kdf108 test vector...");
            testval = json_array_get_value(tests, j);
            testobj = json_value_get_object(testval);

            tc_id = (unsigned int) json_object_get_number(testobj, "tcId");

            key_in_str = json_object_get_string(testobj, "keyIn");
            if (!key_in_str) {
                ACVP_LOG_ERR("Server JSON missing keyIn");
                return ACVP_MISSING_ARG;
            }

            key_in_len = strnlen(key_in_str, ACVP_KDF108_KEYIN_STR_MAX + 1);
            if (key_in_len > ACVP_KDF108_KEYIN_STR_MAX) {
                ACVP_LOG_ERR("keyIn too long, max allowed=(%d)",
                             ACVP_KDF108_KEYIN_STR_MAX);
                return ACVP_INVALID_ARG;
            }
            // Convert to byte length
            key_in_len = key_in_len / 2;

            if (kdf_mode == ACVP_KDF108_MODE_FEEDBACK) {
                iv_str = json_object_get_string(testobj, "iv");
                if (!iv_str) {
                    ACVP_LOG_ERR("Server JSON missing iv");
                    return ACVP_MISSING_ARG;
                }

                iv_len = strnlen(iv_str, ACVP_KDF108_IV_STR_MAX + 1);
                if (iv_len > ACVP_KDF108_IV_STR_MAX) {
                    ACVP_LOG_ERR("iv too long, max allowed=(%d)",
                                 ACVP_KDF108_IV_STR_MAX);
                    return ACVP_INVALID_ARG;
                }

                // Convert to byte length
                iv_len = iv_len / 2;
            }

            deferred = json_object_get_boolean(testobj, "deferred");
            if (deferred == -1) {
                ACVP_LOG_ERR("Server JSON missing deferred");
                return ACVP_MISSING_ARG;
            }

            /*
             * Log Test Case information...
             */
            ACVP_LOG_INFO("        Test case: %d", j);
            ACVP_LOG_INFO("             tcId: %d", tc_id);
            ACVP_LOG_INFO("            keyIn: %s", key_in_str);
            ACVP_LOG_INFO("         deferred: %d", deferred);

            /*
             * Create a new test case in the response
             */
            r_tval = json_value_init_object();
            r_tobj = json_value_get_object(r_tval);

            json_object_set_number(r_tobj, "tcId", tc_id);

            /*
             * Setup the test case data that will be passed down to
             * the crypto module.
             * TODO: this does mallocs, we can probably do the mallocs once for
             *       the entire vector set to be more efficient
             */
            acvp_kdf108_init_tc(ctx, &stc, tc_id, kdf_mode, mac_mode,
                                ctr_loc, key_in_str, iv_str, key_in_len,
                                key_out_len, iv_len, ctr_len, deferred);

            /* Process the current test vector... */
            rv = (cap->crypto_handler)(&tc);
            if (rv != ACVP_SUCCESS) {
                ACVP_LOG_ERR("crypto module failed the operation");
                return ACVP_CRYPTO_MODULE_FAIL;
            }

            /*
             * Output the test case results using JSON
            */
            rv = acvp_kdf108_output_tc(ctx, &stc, r_tobj);
            if (rv != ACVP_SUCCESS) {
                ACVP_LOG_ERR("JSON output failure in kdf135 tpm module");
                return rv;
            }
            /*
             * Release all the memory associated with the test case
             */
            acvp_kdf108_release_tc(&stc);

            /* Append the test response value to array */
            json_array_append_value(r_tarr, r_tval);
        }
        json_array_append_value(r_garr, r_gval);
    }

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
