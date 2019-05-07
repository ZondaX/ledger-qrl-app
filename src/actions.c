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
#include "app_crypto.h"
#include "storage.h"
#include "view.h"

#include "libxmss/xmss.h"
#include "libxmss/nvram.h"
#include <zxmacros.h>

// TODO: move to app_crypto
#include "test_data/test_data.h"

char actions_tree_init_step() {
    if (N_appdata.mode != APPMODE_NOT_INITIALIZED && N_appdata.mode != APPMODE_KEYGEN_RUNNING) {
        return false;
    }
    appstorage_t tmp;

    // Generate all leaves
    if (N_appdata.mode == APPMODE_NOT_INITIALIZED) {
        uint8_t seed[48];
        get_seed(seed);

        xmss_gen_keys_1_get_seeds(&N_DATA.sk, seed);

        tmp.mode = APPMODE_KEYGEN_RUNNING;
        tmp.xmss_index = 0;

        nvm_write((void *) &N_appdata.raw, &tmp.raw, sizeof(tmp.raw));
    }

    if (N_appdata.xmss_index < 256) {
        print_status("keygen: %03d/256", N_appdata.xmss_index + 1);
        view_idle_menu();
        UX_WAIT();

#ifdef TESTING_ENABLED
        for (int idx  = 0; idx < 256; idx +=4){
            nvm_write( (void *) (N_DATA.xmss_nodes + 32 * idx),
                       (void *) test_xmss_leaves[idx],
                       128);
        }
        tmp.mode = APPMODE_KEYGEN_RUNNING;
        tmp.xmss_index = 256;
#else
        const uint8_t *p = N_DATA.xmss_nodes + 32 * N_appdata.xmss_index;
        xmss_gen_keys_2_get_nodes((uint8_t * ) & N_DATA.wots_buffer, (void *) p, &N_DATA.sk, N_appdata.xmss_index);
        tmp.mode = APPMODE_KEYGEN_RUNNING;
        tmp.xmss_index = N_appdata.xmss_index + 1;
#endif

    } else {
        print_status("keygen: root");
        view_idle_menu();
        UX_WAIT();

        xmss_pk_t pk;
        memset(pk.raw, 0, 64);

        xmss_gen_keys_3_get_root(N_DATA.xmss_nodes, &N_DATA.sk);
        xmss_pk(&pk, &N_DATA.sk);

        nvm_write(N_appdata.pk.raw, pk.raw, 64);

        tmp.mode = APPMODE_READY;
        tmp.xmss_index = 0;
    }

    nvm_write((void *) &N_appdata.raw, &tmp.raw, sizeof(tmp.raw));
    return N_appdata.mode != APPMODE_READY;
}

void actions_tree_init() {
    while (actions_tree_init_step());
}
