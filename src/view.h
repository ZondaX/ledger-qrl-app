/*******************************************************************************
*   (c) 2016 Ledger
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
#pragma once

#include "os.h"
#include "cx.h"

#define MAX_CHARS_PER_KEY_LINE      32
#define MAX_CHARS_PER_VALUE_LINE    128

typedef struct {
    char title[16];
    char key[MAX_CHARS_PER_KEY_LINE];
    char value[MAX_CHARS_PER_VALUE_LINE];
    int8_t idx;
} view_t;

extern view_t viewdata;

void view_init(void);
void view_idle_menu(void);
void view_sign_menu(void);
void view_txinfo_show();
void view_setidx_show();
void view_address_show();

void handler_view_tx(unsigned int unused);
void handler_sign_tx(unsigned int unused);
void handler_reject_tx(unsigned int unused);
void handler_init_device(unsigned int);
void handler_menu_idle_select(unsigned int);

#define print_key(...) snprintf(viewdata.key, sizeof(viewdata.key), __VA_ARGS__);
#define print_status(...) snprintf(viewdata.value, sizeof(viewdata.value), __VA_ARGS__);
void view_update_state();
