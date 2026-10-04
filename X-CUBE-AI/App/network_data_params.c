/**
  ******************************************************************************
  * @file    network_data_params.c
  * @author  AST Embedded Analytics Research Platform
  * @date    2026-10-02T12:03:39+0530
  * @brief   AI Tool Automatic Code Generator for Embedded NN computing
  ******************************************************************************
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  ******************************************************************************
  */

#include "network_data_params.h"


/**  Activations Section  ****************************************************/
ai_handle g_network_activations_table[1 + 2] = {
  AI_HANDLE_PTR(AI_MAGIC_MARKER),
  AI_HANDLE_PTR(NULL),
  AI_HANDLE_PTR(AI_MAGIC_MARKER),
};




/**  Weights Section  ********************************************************/
AI_ALIGNED(32)
const ai_u64 s_network_weights_array_u64[58] = {
  0x817f7f817f817f7fU, 0x7f7f817f81817f7fU, 0x17aafffff612U, 0x14edfffffc7bU,
  0xfffff5f8fffffe09U, 0xfffff615fffff8aaU, 0x24b2ffffee71U, 0xffffc738000017dfU,
  0xffffb87bfffffca9U, 0xfffff3c900001b78U, 0x252d15ae6c3878U, 0x5582059de481a3a9U,
  0x75c9f20cb43069faU, 0xe9cdd2f7f9ac5c5U, 0xd2e8eef71ff015e4U, 0xd21af6e505051581U,
  0xeb2a2df175ea4e30U, 0x423bfa24f902197fU, 0xd0e9eff569e450e7U, 0xc53504f600052e81U,
  0xdad4dae413e430c6U, 0xc1250bedf70f2981U, 0xea1913ff7ff1431fU, 0x2a2ffa0904f92353U,
  0xdfe000a8107e0ffU, 0xfce9fc020107efffU, 0xf30f0bf57ff73107U, 0xc24fb0801011228U,
  0xd3e1dff00c020bdbU, 0xc90df9f0fd600f81U, 0x1be8eb15041203e6U, 0xdf0301fafc7f0ea3U,
  0xc9e5f7f400f301e4U, 0xd618f1f70b480581U, 0x38ebe125051f03e8U, 0xe60006f7fa7f0c9dU,
  0x869163abbc7f0925U, 0xa124e5c55588d673U, 0x38aebf59681b7ffU, 0x61e80bc2b92145cbU,
  0x23cd8b81ecbd0c86U, 0x289a279b420020efU, 0xfffff993fffffeaeU, 0x38d00000583U,
  0x4830000043fU, 0xffffff5900000312U, 0x5590000019cU, 0x6440000044cU,
  0x51aU, 0xfffffe07U, 0xd82b699818dff01U, 0xfeff03000301aaU,
  0x94fa6f28ef7ffcffU, 0xf403044a14340fU, 0xfe02f90003f70100U, 0x1010281898a9f02U,
  0x3bfffffffcU, 0xfffffffbU,
};


ai_handle g_network_weights_table[1 + 2] = {
  AI_HANDLE_PTR(AI_MAGIC_MARKER),
  AI_HANDLE_PTR(s_network_weights_array_u64),
  AI_HANDLE_PTR(AI_MAGIC_MARKER),
};

