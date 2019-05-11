/*******************************************************************************
*   (c) 2018 ZondaX GmbH
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
#include "storage.h"

NV_CONST app_data_t N_appdata_impl NV_ALIGN;

void app_data_init() {
    if (N_appdata.initialized){
        return;
    }
    uint8_t tmp[] = {1, 0};
    MEMCPY_NV(&N_appdata.initialized, tmp, sizeof(tmp));
}

void app_set_tree(uint8_t tree_index){
    SET_NV(&APP_TREE_IDX, uint8_t, tree_index);
}

void app_set_mode_index(uint8_t mode, uint16_t xmss_index){
    xmms_tree_t tmp;
    tmp.mode = mode;
    tmp.xmss_index = xmss_index;
    nvm_write((void *) &APP_CURTREE.raw, &tmp.raw, sizeof(tmp.raw));
}
