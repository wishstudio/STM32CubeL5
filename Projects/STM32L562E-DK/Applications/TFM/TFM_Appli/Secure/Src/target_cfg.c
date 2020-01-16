/*
 * Copyright (c) 2018 Arm Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "cmsis.h"
#include "target_cfg.h"
#include "Driver_MPC.h"
#include "platform_retarget_dev.h"
#include "region_defs.h"
#include "tfm_secure_api.h"
#include "tfm_mbedcrypto_config.h"
#include "tfm_plat_defs.h"
/*  fix me to move to a CMSIS driver */
#include "stm32l5xx_hal.h"
#include <stdio.h>
/* Macros to pick linker symbols */
#define REGION(a, b, c) a##b##c
#define REGION_NAME(a, b, c) REGION(a, b, c)
#define REGION_DECLARE(a, b, c) extern uint32_t REGION_NAME(a, b, c)

/* The section names come from the scatter file */
REGION_DECLARE(Load$$LR$$, LR_NS_PARTITION, $$Base);
REGION_DECLARE(Load$$LR$$, LR_VENEER, $$Base);
REGION_DECLARE(Load$$LR$$, LR_VENEER, $$Limit);
REGION_DECLARE(Load$$LR$$, LR_SECONDARY_PARTITION, $$Base);


const struct memory_region_limits memory_regions =
{
  .non_secure_code_start =
  (uint32_t) &REGION_NAME(Load$$LR$$, LR_NS_PARTITION, $$Base),

  .non_secure_partition_base =
  (uint32_t) &REGION_NAME(Load$$LR$$, LR_NS_PARTITION, $$Base),

  .non_secure_partition_limit =
#if 0
  (uint32_t) &REGION_NAME(Load$$LR$$, LR_NS_PARTITION, $$Base) +
  NS_PARTITION_SIZE - 1,
#else /*  0 */
  (uint32_t)(FLASH_BASE_NS + FLASH_AREA_3_OFFSET + FLASH_NS_PARTITION_SIZE - 1),
#endif /* 0 */
  .veneer_base =
  (uint32_t) &REGION_NAME(Load$$LR$$, LR_VENEER, $$Base),

  .veneer_limit =
  (uint32_t) &REGION_NAME(Load$$LR$$, LR_VENEER, $$Limit),
};


/* Define Peripherals NS address range for the platform */
#define PERIPHERALS_BASE_NS_START (PERIPH_BASE_NS)
#define PERIPHERALS_BASE_NS_END   (0x4FFFFFFF)


enum tfm_plat_err_t enable_fault_handlers(void)
{
  NVIC_SetPriority(SecureFault_IRQn, 0);
  /* Enables BUS, MEM, USG and Secure faults */
  SCB->SHCSR |= SCB_SHCSR_USGFAULTENA_Msk
                | SCB_SHCSR_BUSFAULTENA_Msk
                | SCB_SHCSR_MEMFAULTENA_Msk
                | SCB_SHCSR_SECUREFAULTENA_Msk;
  return TFM_PLAT_ERR_SUCCESS;

}

/*----------------- NVIC interrupt target state to NS configuration ----------*/
void nvic_interrupt_target_state_cfg()
{
  /* Target every interrupt to NS; unimplemented interrupts will be WI */
  for (uint8_t i = 0; i < sizeof(NVIC->ITNS) / sizeof(NVIC->ITNS[0]); i++)
  {
    NVIC->ITNS[i] = 0xFFFFFFFF;
  }
}
void system_reset_cfg(void)
{
  /*  fix me : not implemented yet */

}

/*----------------- NVIC interrupt enabling for S peripherals ----------------*/
void nvic_interrupt_enable()
{
  /*  interrupt in s not supported at this stage */
}
/*----------------- RCC accessible for non secure --------------- */
/*  allow clock configuration from non secure */
void enable_ns_clk_config(void)
{
  /*  fix me : not implemented yet */

}
/*----------------- GPIO Pin mux configuration for non secure --------------- */
/*  set all pin mux to un-secure */
void pinmux_conf_init_ns(void)
{
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  GPIOA_S->SECCFGR = 0x0;
  GPIOB_S->SECCFGR = 0x0;
  GPIOC_S->SECCFGR = 0x0;
  GPIOD_S->SECCFGR = 0x0;
  GPIOE_S->SECCFGR = 0x0;
  GPIOF_S->SECCFGR = 0x0;
  GPIOG_S->SECCFGR = 0x0;
  GPIOH_S->SECCFGR = 0x0;

}
/*------------------- SAU/IDAU configuration functions -----------------------*/

void sau_and_idau_cfg(void)
{
  SCB->NSACR = (SCB->NSACR & ~(SCB_NSACR_CP10_Msk | SCB_NSACR_CP11_Msk)) |
               ((SCB_NSACR_CP10_11_VAL << SCB_NSACR_CP10_Pos) & (SCB_NSACR_CP10_Msk | SCB_NSACR_CP11_Msk));

  FPU->FPCCR = (FPU->FPCCR & ~(FPU_FPCCR_TS_Msk | FPU_FPCCR_CLRONRETS_Msk | FPU_FPCCR_CLRONRET_Msk)) |
               ((FPU_FPCCR_TS_VAL        << FPU_FPCCR_TS_Pos) & FPU_FPCCR_TS_Msk) |
               ((FPU_FPCCR_CLRONRETS_VAL << FPU_FPCCR_CLRONRETS_Pos) & FPU_FPCCR_CLRONRETS_Msk) |
               ((FPU_FPCCR_CLRONRET_VAL  << FPU_FPCCR_CLRONRET_Pos) & FPU_FPCCR_CLRONRET_Msk);

  /* Disable SAU */
  TZ_SAU_Disable();

  /* Configures SAU regions to be non-secure */
  SAU->RNR  = TFM_NS_REGION_CODE;
  SAU->RBAR = (memory_regions.non_secure_partition_base
               & SAU_RBAR_BADDR_Msk);
  SAU->RLAR = (memory_regions.non_secure_partition_limit
               & SAU_RLAR_LADDR_Msk)
              | SAU_RLAR_ENABLE_Msk;

  SAU->RNR  = TFM_NS_REGION_DATA;
  SAU->RBAR = (NS_DATA_START & SAU_RBAR_BADDR_Msk);
  SAU->RLAR = (NS_DATA_LIMIT & SAU_RLAR_LADDR_Msk) | SAU_RLAR_ENABLE_Msk;

  /* Configures veneers region to be non-secure callable */
  SAU->RNR  = TFM_NS_REGION_VENEER;
  SAU->RBAR = (memory_regions.veneer_base  & SAU_RBAR_BADDR_Msk);
  SAU->RLAR = (memory_regions.veneer_limit & SAU_RLAR_LADDR_Msk)
              | SAU_RLAR_ENABLE_Msk
              | SAU_RLAR_NSC_Msk;

  /* Configure the peripherals space */
  SAU->RNR  = TFM_NS_REGION_PERIPH_1;
  SAU->RBAR = (PERIPHERALS_BASE_NS_START & SAU_RBAR_BADDR_Msk);
  SAU->RLAR = (PERIPHERALS_BASE_NS_END & SAU_RLAR_LADDR_Msk)
              | SAU_RLAR_ENABLE_Msk;
  /* Configure the peripherals space 2 to access package information */
  SAU->RNR  = TFM_NS_REGION_PERIPH_2;
  SAU->RBAR = (PACKAGE_BASE & SAU_RBAR_BADDR_Msk);
  SAU->RLAR = ((PACKAGE_BASE + 0xfff) & SAU_RLAR_LADDR_Msk)
              | SAU_RLAR_ENABLE_Msk;
  /* Force memory writes before continuing */
  __DSB();
  /* Flush and refill pipeline with updated permissions */
  __ISB();

  /* Enable SAU */
  TZ_SAU_Enable();

  /* Lock SAU config */
  __HAL_RCC_SYSCFG_CLK_ENABLE();
  SYSCFG->CSLCKR |= SYSCFG_CSLCKR_LOCKSAU;
}
void  unsecure_sram1(uint32_t start, uint32_t end);
void  unsecure_sram2(uint32_t start, uint32_t end);
void  lock_sram_config(uint32_t MemBaseAddress);

/*------------------- Memory configuration functions -------------------------*/
void  unsecure_sram1(uint32_t start, uint32_t end)
{

  MPCBB_ConfigTypeDef MPCBB_desc;
  uint32_t regwrite = 0xffffffff;
  uint32_t index;
  uint32_t block_start = (start - 0x20000000) / (256);
  uint32_t block_end = block_start + ((end - start) + 1) / (256);

  if (start & 0xff)
    /*  Check alignment to avoid futher problem  */
    /*  FIX ME  */
    while (1);

  if (HAL_GTZC_MPCBB_GetConfigMem(SRAM1_BASE, &MPCBB_desc) != HAL_OK)
  {
    /* FIX ME */
    while (1);
  }

  for (index = 0; index < 768; index++)
  {
    /* clean register on index aligned */
    if (!(index & 0x1f))
    {
      regwrite = 0xffffffff;
    }
    if ((index >= block_start) && (index < block_end))
    {
      regwrite = regwrite & ~(1 << (index & 0x1f));
    }
    /* write register when 32 sub block are set  */
    if ((index & 0x1f) == 0x1f)
    {
      MPCBB_desc.AttributeConfig.MPCBB_SecConfig_array[index >> 5] = regwrite;
    }
  }
  if (HAL_GTZC_MPCBB_ConfigMem(SRAM1_BASE, &MPCBB_desc) != HAL_OK)
    /* FIX ME */
    while (1);

}

void  unsecure_sram2(uint32_t start, uint32_t end)
{

  MPCBB_ConfigTypeDef MPCBB_desc;
  uint32_t regwrite = 0xffffffff;
  uint32_t index;
  uint32_t block_start = (start - 0x30030000) / (256);
  uint32_t block_end = block_start + ((end - start) + 1) / (256);

  if (start & 0xff)
    /*  Check alignment to avoid futher problem  */
    /*  FIX ME  */
    while (1);

  if (HAL_GTZC_MPCBB_GetConfigMem(SRAM2_BASE, &MPCBB_desc) != HAL_OK)
  {
    /* FIX ME */
    while (1);
  }

  for (index = 0; index < 256; index++)
  {
    /* clean register on index aligned */
    if (!(index & 0x1f))
    {
      regwrite = 0xffffffff;
    }
    if ((index >= block_start) && (index < block_end))
    {
      regwrite = regwrite & ~(1 << (index & 0x1f));
    }
    /* write register when 32 sub block are set  */
    if ((index & 0x1f) == 0x1f)
    {
      MPCBB_desc.AttributeConfig.MPCBB_SecConfig_array[index >> 5] = regwrite;
    }
  }
  if (HAL_GTZC_MPCBB_ConfigMem(SRAM2_BASE, &MPCBB_desc) != HAL_OK)
    /* FIX ME */
    while (1);

}

void  lock_sram_config(uint32_t MemBaseAddress)
{
  MPCBB_ConfigTypeDef MPCBB_desc;
  uint32_t index;
  if (HAL_GTZC_MPCBB_GetConfigMem(MemBaseAddress, &MPCBB_desc) != HAL_OK)
    /* FIX ME */
    while (1);

  for (index = 0; index < GTZC_MCPBB_NB_LCK_VCTR_REG_MAX; index ++)
  {
    MPCBB_desc.AttributeConfig.MPCBB_LockConfig_array[index] = 0xffffffff;
  }
  if (HAL_GTZC_MPCBB_ConfigMem(MemBaseAddress, &MPCBB_desc) != HAL_OK)
    /* FIX ME */
    while (1);
}

void mpc_init_cfg(void)
{
  /* Enable GTZC clock */
  __HAL_RCC_GTZC_CLK_ENABLE();
  /* assume non secure ram is only in SRAM 1 , SRAM 2 is reserved for secure */
  unsecure_sram1(NS_DATA_START, NS_DATA_START + NS_DATA_SIZE - 1);
  lock_sram_config(SRAM1_BASE);
  lock_sram_config(SRAM2_BASE);

  /*  use sticky bit to lock all SRAM config  */
  /*  enable interruption on illegal access on FLASH */
  HAL_GTZC_TZIC_EnableIT(GTZC_PERIPH_FLASH);

  /* Configure Secure peripherals */
  HAL_GTZC_TZSC_ConfigPeriphAttributes(GTZC_PERIPH_RNG, GTZC_TZSC_PERIPH_SEC | GTZC_TZSC_PERIPH_NPRIV);
#if (defined (MBEDTLS_SHA256_C) && defined (MBEDTLS_SHA256_ALT)) || \
    (defined (MBEDTLS_SHA1_C) && defined (MBEDTLS_SHA1_ALT)) || \
    (defined (MBEDTLS_MD5_C) && defined (MBEDTLS_MD5_ALT))
  HAL_GTZC_TZSC_ConfigPeriphAttributes(GTZC_PERIPH_HASH, GTZC_TZSC_PERIPH_SEC | GTZC_TZSC_PERIPH_NPRIV);
#endif
#if (defined (MBEDTLS_ECP_C) && defined (MBEDTLS_ECP_ALT)) || \
    (defined (MBEDTLS_ECDSA_C) && (defined (MBEDTLS_ECDSA_SIGN_ALT) || defined (MBEDTLS_ECDSA_SIGN_ALT)))
  HAL_GTZC_TZSC_ConfigPeriphAttributes(GTZC_PERIPH_PKA, GTZC_TZSC_PERIPH_SEC | GTZC_TZSC_PERIPH_NPRIV);
#endif
#if (defined (MBEDTLS_AES_C) && defined (MBEDTLS_AES_ALT)) || \
    (defined (MBEDTLS_GCM_C) && defined (MBEDTLS_GCM_ALT)) || \
    (defined (MBEDTLS_CCM_C) && defined (MBEDTLS_CCM_ALT))
  HAL_GTZC_TZSC_ConfigPeriphAttributes(GTZC_PERIPH_AES, GTZC_TZSC_PERIPH_SEC | GTZC_TZSC_PERIPH_NPRIV);
#endif

  /* Add barriers to assure the MPC configuration is done before continue
   * the execution.
   */
  __DSB();
  __ISB();
}


/*---------------------- PPC configuration functions -------------------------*/

void ppc_init_cfg(void)
{
  /*  pinmux is secured after reset : configure all pin mux as non secure*/
  pinmux_conf_init_ns();
  /*  all peripheral  are non secured after reset */
}


void ppc_clear_irq(void)
{
  /*  fix me : not implemented yet */

}

void tfm_spm_hal_init_debug(void)
{
  /*  debug is available  only with RDP 0 for secure*/
}
