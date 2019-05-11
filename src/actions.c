/*******************************************************************************
*   (c) 2019 ZondaX GmbH
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
********************************************************************************/
#include "actions.h"

#include "bolos_target.h"
#include "storage.h"
#include "view.h"
#include "apdu_codes.h"

#include "libxmss/xmss.h"
#include "libxmss/nvram.h"
#include "test_data/test_data.h"

app_ctx_t ctx;

const uint32_t bip32_path_tree1[5] = {
        0x80000000 | 44,
        0x80000000 | 238,
        0x80000000 | 0,
        0x80000000 | 0,
        0x80000000 | 0
};

const uint32_t bip32_path_tree2[5] = {
        0x80000000 | 44,
        0x80000000 | 238,
        0x80000000 | 0,
        0x80000000 | 0,
        0x80000000 | 1
};

void get_seed(uint8_t *seed) {
    union {
        unsigned char all[64];
        struct {
            unsigned char seed[32];
            unsigned char chain[32];
        };
    } u;

    MEMSET(seed, 0, 48);

#ifdef TESTING_MOCKSEED
    UNUSED(u);
    // Keep as all zeros for reproducible tests
#else
    unsigned char tmp_out[64];

    // TODO: Allow switching to another tree here
    os_memset(u.all, 0, 64);

    if (APP_TREE_IDX == 0) {
        os_perso_derive_node_bip32(CX_CURVE_SECP256K1, bip32_path_tree1, 5, u.seed, u.chain);
    } else {
        os_perso_derive_node_bip32(CX_CURVE_SECP256K1, bip32_path_tree2, 5, u.seed, u.chain);
    }

    cx_sha3_t hash_sha3;
    cx_sha3_init(&hash_sha3, 512);
    cx_hash(&hash_sha3.header, CX_LAST, u.all, 64, tmp_out, 64);
    memcpy(seed, tmp_out, 48);
#endif
}

void hash_tx(uint8_t hash[32]) {
    const int8_t ret = get_qrltx_hash(&ctx.qrltx, hash);
    if (ret < 0) {
        THROW(APDU_CODE_DATA_INVALID);
    }
}

char actions_tree_init_step() {
    if (APP_CURTREE_MODE != APPMODE_NOT_INITIALIZED && APP_CURTREE_MODE != APPMODE_KEYGEN_RUNNING) {
        return false;
    }

    if (APP_CURTREE_MODE == APPMODE_NOT_INITIALIZED) {
        uint8_t seed[48];
        get_seed(seed);

        xmss_gen_keys_1_get_seeds(&N_XMSS_DATA.sk, seed);

        app_set_mode_index(APPMODE_KEYGEN_RUNNING, 0);
        print_status("keygen start");
    }

    if (APP_CURTREE_XMSSIDX < 256) {
#ifdef TESTING_ENABLED
        for (int idx  = 0; idx < 256; idx +=4){
            nvm_write( (void *) (N_XMSS_DATA.xmss_nodes + 32 * idx),
                       (void *) test_xmss_leaves[idx],
                       128);
        }
        app_set_mode_index(APPMODE_KEYGEN_RUNNING, 256);
        print_status("TEST TREE");
#else
        const uint8_t *p = N_XMSS_DATA.xmss_nodes + 32 * APP_CURTREE_XMSSIDX;

        xmss_gen_keys_2_get_nodes((uint8_t * ) & N_XMSS_DATA.wots_buffer,
                                  (void *) p,
                                  &N_XMSS_DATA.sk,
        APP_CURTREE_XMSSIDX);

        app_set_mode_index(APPMODE_KEYGEN_RUNNING, APP_CURTREE_XMSSIDX + 1);
        print_status("keygen %03d/256", APP_CURTREE_XMSSIDX);
#endif
    } else {
        xmss_pk_t pk;
        memset(pk.raw, 0, 64);
        xmss_gen_keys_3_get_root(N_XMSS_DATA.xmss_nodes, &N_XMSS_DATA.sk);
        xmss_pk(&pk, &N_XMSS_DATA.sk);
        nvm_write(APP_CURTREE.pk.raw, pk.raw, 64);

        app_set_mode_index(APPMODE_READY, 0);
        print_status("keygen root");
    }

    return APP_CURTREE_MODE != APPMODE_READY;
}

void actions_tree_init() {
    while (actions_tree_init_step()) {
        view_idle_show();
        UX_WAIT();
    }
}
