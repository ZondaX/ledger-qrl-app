// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
#pragma once

#ifdef  __cplusplus
extern "C" {
#endif

#include "xmss_types.h"

typedef struct {
  xmss_sk_t sk;
  xmss_signature_t signature;
  uint8_t wots_buffer[WOTS_LEN * WOTS_N];
  uint8_t xmss_nodes[XMSS_NODES_BUFSIZE];
} xmss_data_t;

extern NV_CONST xmss_data_t N_xmss_data_impl NV_ALIGN;
#define N_XMSS_DATA (*(xmss_data_t *)PIC(&N_xmss_data_impl))

#ifdef  __cplusplus
}
#endif
