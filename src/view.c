/*******************************************************************************
*   (c) 2016 Ledger
*   (c) 2018, 2019 ZondaX GmbH
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
#include <string.h>
#include "glyphs.h"
#include "view.h"
#include "app_main.h"
#include "apdu_codes.h"
#include "storage.h"
#include "view_templates.h"
#include "app_types.h"

#include "actions.h"

view_t viewdata;

#define ARRTOHEX(X, Y) array_to_hexstr(X, Y, sizeof(Y))
#define AMOUNT_TO_STR(OUTPUT, AMOUNT, DECIMALS) fpuint64_to_str(OUTPUT, uint64_from_BEarray(AMOUNT), DECIMALS)

void h_tree_init(unsigned int _) {
    actions_tree_init();
    view_update_state();
    view_idle_menu();
}

#if defined(TARGET_NANOX)

#include "ux.h"
ux_state_t G_ux;
bolos_ux_params_t G_ux_params;

UX_STEP_NOCB_INIT(ux_idle_flow_1_step, pbb, { view_update_state(); }, { &C_icon_app, viewdata.key, viewdata.value, });

UX_FLOW_DEF_VALID(ux_idle_flow_2init_step, pb, h_tree_init(0), { &C_icon_key, "Initialize",});
UX_FLOW_DEF_NOCB(ux_idle_flow_2pk_step, pb, { &C_icon_key, "Show Addr",});

UX_FLOW_DEF_NOCB(ux_idle_flow_4_step, bn, { "Version", APPVERSION, });
UX_FLOW_DEF_VALID(ux_idle_flow_5_step, pb, os_sched_exit(-1), { &C_icon_dashboard, "Quit",});


const ux_flow_step_t *const ux_idle_flow [] = {
  &ux_idle_flow_1_step,
  &ux_idle_flow_2pk_step,
  &ux_idle_flow_4_step,
  &ux_idle_flow_5_step,
  FLOW_END_STEP,
};

const ux_flow_step_t *const ux_idle_init_flow [] = {
  &ux_idle_flow_1_step,
  &ux_idle_flow_2init_step,
  &ux_idle_flow_4_step,
  &ux_idle_flow_5_step,
  FLOW_END_STEP,
};

UX_FLOW_DEF_VALID(ux_tx_flow_1_step, pbb, handler_view_tx(0), { &C_icon_eye, "Review", "Transaction" });
UX_FLOW_DEF_VALID(ux_tx_flow_2_step, pbb, handler_sign_tx(0), { &C_icon_validate_14, "Sign", "Transaction" });
UX_FLOW_DEF_VALID(ux_tx_flow_3_step, pbb, handler_reject_tx(0), { &C_icon_crossmark, "Reject", "Transaction" });
const ux_flow_step_t *const ux_tx_flow [] = {
  &ux_tx_flow_1_step,
  &ux_tx_flow_2_step,
  &ux_tx_flow_3_step,
  FLOW_END_STEP,
};

// TODO: ViewTX
// TODO: Set index
// TODO: ViewAddr
#else

#define COND_SCROLL_L2 0xF0

#define UIID_STATUS        UIID_LABEL+1
#define UIID_TREE_INIT     UIID_LABEL+3
#define UIID_TREE_PK       UIID_LABEL+4
#define UIID_TREE_SWITCH   UIID_LABEL+5

ux_state_t ux;

const ux_menu_entry_t menu_idle[] = {
        {NULL, NULL, UIID_STATUS, &C_icon_app, viewdata.key, viewdata.value, 32, 11},
        {NULL, NULL, UIID_TREE_PK, NULL, "Show Addr", NULL, 0, 0},
        {NULL, NULL, 0, NULL, "v"APPVERSION, NULL, 0, 0},
        {NULL, os_sched_exit, 0, &C_icon_dashboard, "Quit app", NULL, 50, 29},
        UX_MENU_END
};

const ux_menu_entry_t menu_idle_init[] = {
        {NULL, NULL, UIID_STATUS, &C_icon_app, viewdata.key, viewdata.value, 32, 11},
        {NULL, h_tree_init, UIID_TREE_INIT, NULL, "Init Tree", NULL, 0, 0},
        {NULL, NULL, 0, NULL, "QRL v"APPVERSION, NULL, 0, 0},
        {NULL, os_sched_exit, 0, &C_icon_dashboard, "Quit app", NULL, 50, 29},
        UX_MENU_END
};

const ux_menu_entry_t menu_sign[] = {
        {NULL, handler_view_tx, 0, NULL, "View transaction", NULL, 0, 0},
        {NULL, handler_sign_tx, 0, NULL, "Sign transaction", NULL, 0, 0},
        {NULL, handler_reject_tx, 0, &C_icon_back, "Reject", NULL, 60, 40},
        UX_MENU_END
};

static const bagl_element_t view_txinfo[] = {
        UI_BACKGROUND_LEFT_RIGHT_ICONS,
        UI_LabelLine(UIID_LABEL + 0, 0, 8, UI_SCREEN_WIDTH, UI_11PX, UI_WHITE, UI_BLACK, viewdata.title),
        UI_LabelLine(UIID_LABEL + 0, 0, 19, UI_SCREEN_WIDTH, UI_11PX, UI_WHITE, UI_BLACK, viewdata.key),
        UI_LabelLineScrolling(UIID_LABELSCROLL, 6, 30, 112, UI_11PX, UI_WHITE, UI_BLACK, viewdata.value),
};

static const bagl_element_t view_setidx[] = {
        UI_FillRectangle(0, 0, 0, UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 0x000000, 0xFFFFFF),
        UI_Icon(0, 0, 0, 7, 7, BAGL_GLYPH_ICON_CROSS),
        UI_Icon(0, 128 - 7, 0, 7, 7, BAGL_GLYPH_ICON_CHECK),
        UI_LabelLine(UIID_LABEL + 0, 0, 8, UI_SCREEN_WIDTH, UI_11PX, UI_WHITE, UI_BLACK, viewdata.title),
        UI_LabelLine(UIID_LABEL + 0, 0, 19, UI_SCREEN_WIDTH, UI_11PX, UI_WHITE, UI_BLACK, viewdata.key),
        UI_LabelLineScrolling(UIID_LABELSCROLL, 6, 30, 112, UI_11PX, UI_WHITE, UI_BLACK, viewdata.value),
};

static const bagl_element_t view_address[] = {
        UI_FillRectangle(0, 0, 0, UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 0x000000, 0xFFFFFF),
        UI_Icon(0, 128 - 7, 0, 7, 7, BAGL_GLYPH_ICON_CHECK),
        UI_LabelLine(UIID_LABEL + 0, 0, 8, UI_SCREEN_WIDTH, UI_11PX, UI_WHITE, UI_BLACK, viewdata.title),
        UI_LabelLine(UIID_LABEL + 0, 0, 19, UI_SCREEN_WIDTH, UI_11PX, UI_WHITE, UI_BLACK, viewdata.key),
        UI_LabelLineScrolling(UIID_LABELSCROLL, 6, 30, 112, UI_11PX, UI_WHITE, UI_BLACK, viewdata.value),
};

////////////////////////////////////////////////
//------ Event handlers

static unsigned int view_txinfo_button(unsigned int button_mask,
                                       unsigned int button_mask_counter) {
    switch (button_mask) {
        // Press both left and right buttons to quit
        case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT: {
            view_sign_menu();
            break;
        }

            // Press left to progress to the previous element
        case BUTTON_EVT_RELEASED | BUTTON_LEFT: {
            viewdata.idx--;
            view_txinfo_show();
            break;
        }

            // Press right to progress to the next element
        case BUTTON_EVT_RELEASED | BUTTON_RIGHT: {
            viewdata.idx++;
            view_txinfo_show();
            break;
        }

    }
    return 0;
}

static unsigned int view_setidx_button(unsigned int button_mask,
                                       unsigned int button_mask_counter) {
    switch (button_mask) {
        // Press both left and right buttons to quit
        case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT: {
            // DO NOTHING
            break;
        }

            // Press left to progress to cancel
        case BUTTON_EVT_RELEASED | BUTTON_LEFT: {
            // Cancel changing the index
            view_update_state();

            set_code(G_io_apdu_buffer, 0, APDU_CODE_COMMAND_NOT_ALLOWED);
            io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, 2);
            break;
        }

            // Press right to progress to accept
        case BUTTON_EVT_RELEASED | BUTTON_RIGHT: {
            // Accept changing the index
            app_setidx();
            view_update_state();

            set_code(G_io_apdu_buffer, 0, APDU_CODE_OK);
            io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, 2);
            break;
        }

    }
    return 0;
}

static unsigned int view_address_button(unsigned int button_mask,
                                       unsigned int button_mask_counter) {
    switch (button_mask) {
        // Press both left and right buttons to quit
        case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT: {
            // DO NOTHING
            break;
        }

            // Press left to progress to cancel
        case BUTTON_EVT_RELEASED | BUTTON_LEFT: {
            // DO NOTHING
            break;
        }

            // Press right to progress to accept
        case BUTTON_EVT_RELEASED | BUTTON_RIGHT: {
            // Go back to main menu
            view_update_state();

            set_code(G_io_apdu_buffer, 0, APDU_CODE_OK);
            io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, 2);
            break;
        }

    }
    return 0;
}

const bagl_element_t *view_prepro(const bagl_element_t *element) {
    switch (element->component.userid) {
        case UIID_ICONLEFT:
        case UIID_ICONRIGHT:
            UX_CALLBACK_SET_INTERVAL(2000);
            break;
        case UIID_LABELSCROLL:
            UX_CALLBACK_SET_INTERVAL(MAX(3000, 1000 + bagl_label_roundtrip_duration_ms(element, 7)));
            break;
    }
    return element;
}

////////////////////////////////////////////////
////////////////////////////////////////////////

#endif

void io_seproxyhal_display(const bagl_element_t *element) {
    io_seproxyhal_display_default((bagl_element_t *) element);
}

void view_init(void) {
    UX_INIT();
}

void view_idle_menu(void) {

#if defined(TARGET_NANOS)
    if (N_appdata.mode != APPMODE_READY) {
        UX_MENU_DISPLAY(0, menu_idle_init, NULL);
    } else {
        UX_MENU_DISPLAY(0, menu_idle, NULL);
    }
#elif defined(TARGET_NANOX)
    if(G_ux.stack_count == 0) {
        ux_stack_push();
    }
    if (N_appdata.mode != APPMODE_READY) {
        ux_flow_init(0, ux_idle_init_flow, NULL);
    } else {
        ux_flow_init(0, ux_idle_flow, NULL);
    }
#endif
}

void view_sign_menu(void) {
#if defined(TARGET_NANOS)
    UX_MENU_DISPLAY(0, menu_sign, NULL);
#elif defined(TARGET_NANOX)
    if(G_ux.stack_count == 0) {
        ux_stack_push();
    }
    ux_flow_init(0, ux_tx_flow, NULL);
#endif
}

void view_update_state() {
    #ifdef TESTING_ENABLED
        print_key("QRL");
    #else
        print_key("QRL (TEST)")
    #endif

    switch (N_appdata.mode) {
        case APPMODE_NOT_INITIALIZED: {
            print_status("not ready ");
        }
            break;
        case APPMODE_KEYGEN_RUNNING: {
            print_status("KEYGEN rem:%03d", 256 - N_appdata.xmss_index);
        }
            break;
        case APPMODE_READY: {
            if (N_appdata.xmss_index >= 256) {
                print_status( "NO SIGS LEFT");
                break;
            }

            if (N_appdata.xmss_index > 250) {
                print_status("WARN! rem:%03d", 256 - N_appdata.xmss_index);
                break;
            }

            print_status("READY rem:%03d", 256 - N_appdata.xmss_index);
        }
            break;
    }
}

void view_txinfo_show() {
#define EXIT_VIEW() {view_sign_menu(); return;}
#define PTR_DIST(p2, p1) ((char *)p2) - ((char *)p1)

    uint8_t
            elem_idx = 0;

    switch (ctx.qrltx.type) {

        case QRLTX_TX: {
            strcpy(viewdata.title, "TRANSFER");

            switch (viewdata.idx) {
                case 0: {
                    strcpy(viewdata.key, "Source Addr");
                    viewdata.value[0] = 'Q';
                    ARRTOHEX(viewdata.value + 1, ctx.qrltx.tx.master.address);
                    break;
                }
                case 1: {
                    strcpy(viewdata.key, "Fee (QRL)");

                    AMOUNT_TO_STR(viewdata.value,
                                  ctx.qrltx.tx.master.amount,
                                  QUANTA_DECIMALS);
                    break;
                }
                default: {
                    elem_idx = (viewdata.idx - 2) >> 1;
                    if (elem_idx >= QRLTX_SUBITEM_MAX) EXIT_VIEW();
                    if (elem_idx >= ctx.qrltx.subitem_count) EXIT_VIEW();

                    qrltx_addr_block *dst = &ctx.qrltx.tx.dst[elem_idx];

                    if (viewdata.idx % 2 == 0) {
                        snprintf(viewdata.key, sizeof(viewdata.key), "Dst %d", elem_idx);
                        viewdata.value[0] = 'Q';
                        ARRTOHEX(viewdata.value + 1, dst->address);
                    } else {
                        snprintf(viewdata.key, sizeof(viewdata.key), "Amount %d (QRL)", elem_idx);
                        AMOUNT_TO_STR(viewdata.value, dst->amount, QUANTA_DECIMALS);
                    }
                    break;
                }
            }
            break;
        }
#ifdef TXTOKEN_ENABLED
        case QRLTX_TXTOKEN: {
            strcpy(viewdata.title, "TRANSFER TOKEN");

            switch (viewdata.idx) {
                case 0: {
                    strcpy(viewdata.key, "Source Addr");
                    viewdata.value[0] = 'Q';
                    ARRTOHEX(viewdata.value + 1, ctx.qrltx.txtoken.master.address);
                    break;
                }
                case 1: {
                    strcpy(viewdata.key, "Fee (QRL)");
                    AMOUNT_TO_STR(viewdata.value,
                                  ctx.qrltx.txtoken.master.amount,
                                  QUANTA_DECIMALS);
                    break;
                }
                case 2: {
                    strcpy(viewdata.key, "Token Hash");
                    ARRTOHEX(viewdata.value, ctx.qrltx.txtoken.token_hash);
                    break;
                }
                default: {
                    elem_idx = (viewdata.idx - 3) >> 2;
                    if (elem_idx >= QRLTX_SUBITEM_MAX) EXIT_VIEW();
                    if (elem_idx >= ctx.qrltx.subitem_count) EXIT_VIEW();

                    qrltx_addr_block *dst = &ctx.qrltx.txtoken.dst[elem_idx];

                    if (view_idx % 2 == 0) {
                        snprintf(viewdata.key, sizeof(viewdata.key), "Dst %d", elem_idx);
                        viewdata.value[0] = 'Q';
                        ARRTOHEX(viewdata.value + 1, dst->address);
                    } else {
                        snprintf(viewdata.key, sizeof(viewdata.key), "Amount %d (QRL)", elem_idx);
                        // TODO: Decide what to do with token decimals
                        AMOUNT_TO_STR(viewdata.value, dst->amount, 0);
                    }
                    break;
                }
            }
            break;
        }
#endif
#ifdef SLAVE_ENABLED
        case QRLTX_SLAVE: {
            strcpy(view_title, "CREATE SLAVE");

            switch (view_idx) {
                case 0: {
                    strcpy(viewdata.key, "Master Addr");
                    viewdata.value[0] = 'Q';
                    ARRTOHEX(viewdata.value + 1, ctx.qrltx.slave.master.address);
                    break;
                }
                case 1: {
                    strcpy(viewdata.key, "Fee (QRL)");
                    AMOUNT_TO_STR(viewdata.value,
                                  ctx.qrltx.slave.master.amount,
                                  QUANTA_DECIMALS);
                    break;
                }
                default: {
                    elem_idx = (view_idx - 2) >> 1;
                    if (elem_idx >= QRLTX_SUBITEM_MAX) EXIT_VIEW();
                    if (elem_idx >= ctx.qrltx.subitem_count) EXIT_VIEW();

                    if (view_idx % 2 == 0) {
                        snprintf(viewdata.key, sizeof(viewdata.key), "Slave PK %d", elem_idx);
                        ARRTOHEX(viewdata.value, ctx.qrltx.slave.slaves[elem_idx].pk);
                    } else {
                        snprintf(viewdata.key, sizeof(viewdata.key), "Access Type %d", elem_idx);
                        ARRTOHEX(viewdata.value, ctx.qrltx.slave.slaves[elem_idx].access);
                    }
                    break;
                }
            }
            break;
        }
#endif
        case QRLTX_MESSAGE: {
            strcpy(viewdata.title, "MESSAGE");

            switch (viewdata.idx) {
                case 0: {
                    strcpy(viewdata.key, "Source Addr");
                    viewdata.value[0] = 'Q';
                    ARRTOHEX(viewdata.value + 1,
                             ctx.qrltx.msg.master.address);
                    break;
                }
                case 1: {
                    strcpy(viewdata.key, "Fee (QRL)");

                    AMOUNT_TO_STR(viewdata.value,
                                  ctx.qrltx.msg.master.amount,
                                  QUANTA_DECIMALS);
                    break;
                }
                case 2: {
                    strcpy(viewdata.key, "Message");
                    array_to_hexstr(viewdata.value,
                                    ctx.qrltx.msg.message,
                                    ctx.qrltx.subitem_count);
                    break;
                }
                default: {
                    EXIT_VIEW();
                }
            }
            break;
        }
        default: EXIT_VIEW()
    }

#if defined(TARGET_NANOS)
    UX_DISPLAY(view_txinfo, view_prepro);
#elif defined(TARGET_NANOX)
    // TODO: Complete
//    if(G_ux.stack_count == 0) {
//        ux_stack_push();
//    }
//    ux_flow_init(0, ux_tx_flow, NULL);
#endif
}

void view_setidx_show() {
    strcpy(viewdata.title, "WARNING!");
    strcpy(viewdata.key, "Set XMSS Index");
    print_status("New Value %d", ctx.new_idx);

#if defined(TARGET_NANOS)
    UX_DISPLAY(view_setidx, view_prepro);
#elif defined(TARGET_NANOX)
    // TODO: Complete
    //    if(G_ux.stack_count == 0) {
//        ux_stack_push();
//    }
//    ux_flow_init(0, ux_tx_flow, NULL);
#endif
}

void view_address_show() {
    // See https://docs.theqrl.org/developers/address/#format-sha256_2x
    // Add Ledger Nano S wallet address descriptor
    unsigned char desc[3];
    desc[0] = 0; // XMSS, SHA2-256
    desc[1] = 4; // Height 8
    desc[2] = 0; // SHA256_X

    // Copy raw pk into pk
    unsigned char pk[64];
    MEMCPY(pk, N_appdata.pk.raw,64);

    // Copy desc and pk to form extended_pubkey
    unsigned char extended_pubkey[67];
    MEMCPY(extended_pubkey, desc, 3);
    MEMCPY(extended_pubkey + 3, pk, 64);

    // Objects to store sha256 hashes in
    unsigned char hash_out[32];
    unsigned char verh_out[32];

    // Calculate HASH - sha2-256(DESC+PK)
    cx_hash_sha256(extended_pubkey, 67, hash_out, 32);

    // Copy desc and hash_out to create hashable bytes for verification checksum
    unsigned char verification_sum[35];
    MEMCPY(verification_sum, desc, 3);
    MEMCPY(verification_sum + 3, hash_out, 32);

    // Calculate VERH - sha2-256(DESC+hash_out)
    cx_hash_sha256(verification_sum, 35, verh_out, 32);

    // Now join desc, hash_out and last 4 bytes of verh_out to form address
    unsigned char address[39];
    MEMCPY(address, desc, 3);
    MEMCPY(address + 3, hash_out, 32);
    MEMCPY(address + 3 + 32, verh_out + 28, 4);

    // Prepare for display
    strcpy(viewdata.title, "VERIFY");
    strcpy(viewdata.key, "Your QRL Address");
    viewdata.value[0] = 'Q';
    ARRTOHEX(viewdata.value + 1, address);

#if defined(TARGET_NANOS)
    UX_DISPLAY(view_address, view_prepro);
#elif defined(TARGET_NANOX)
    // TODO: Complete
//    if(G_ux.stack_count == 0) {
//        ux_stack_push();
//    }
//    ux_flow_init(0, ux_tx_flow, NULL);
#endif
}

////////////////////////////////////////////////
////////////////////////////////////////////////


////////////////////////////////////////////////
////////////////////////////////////////////////

void handler_view_tx(unsigned int _) {
    UNUSED(_);
    // TODO: Create a better view
    viewdata.idx = 0;
    view_txinfo_show();
}

void handler_sign_tx(unsigned int _) {
    UNUSED(_);
    app_sign();

    view_update_state();

    set_code(G_io_apdu_buffer, 0, APDU_CODE_OK);
    io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, 2);
}

void handler_reject_tx(unsigned int _) {
    UNUSED(_);

    view_update_state();

    set_code(G_io_apdu_buffer, 0, APDU_CODE_COMMAND_NOT_ALLOWED);
    io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, 2);
}
