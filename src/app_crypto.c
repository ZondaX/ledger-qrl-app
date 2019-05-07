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
#include "app_crypto.h"
#include "os.h"
#include "apdu_codes.h"

#include "lib/qrl_types.h"
#include "libxmss/xmss.h"
#include "libxmss/nvram.h"

app_ctx_t ctx;

const uint32_t bip32_path[5] = {
        0x80000000 | 44,
        0x80000000 | 238,
        0x80000000 | 0,
        0x80000000 | 0,
        0x80000000 | 0
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
    os_perso_derive_node_bip32(CX_CURVE_SECP256K1, bip32_path, 5, u.seed, u.chain);

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
