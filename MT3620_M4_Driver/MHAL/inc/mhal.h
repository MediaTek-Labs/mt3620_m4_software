/*
 * MediaTek Inc. (C) 2019. All rights reserved.
 *
 * Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#ifndef __MHAL_H__
#define __MHAL_H__

/**
 * @addtogroup M-HAL
 * @{
 * This section introduces MT3620 CM4 common driver,
 * including terms and acronyms, software architecture,
 * file naming convention, API naming convention.
 * @section MHAL_Overview_1_Chapter    Terms and acronyms
 *
 * |Acronyms                         |Definition                              |
 * |------------------------------|-------------------------------------------|
 * |\b ADC                  | Analog-to-digital converter
 * |\b EINT                 | External interrupt
 * |\b DMA                 | Direct memory access
 * |\b GPIO                 | General-Purpose Inputs-Outputs
 * |\b GPIOIF               | General-Purpose Inputs-Outputs Interface
 * |\b GPT                  | Generic purpose timer
 * |\b I2C                  | Inter-integrated circuit
 * |\b I2S                  | Inter-integrated sound
 * |\b PWM                  | Pulse-Width Modulation
 * |\b SPI                  | Serial Peripheral Interface
 * |\b UART                 | Universal asynchronous receiver/transmitter
 * |\b WDT                  | Watchdog timer
 *
 * @section MHAL_Overview_2_Chapter    Overview of M-HAL SW Architecture
 *
 * - \b Introduction \n
 * MT3620 CM4 common driver has 5 layers as follows:\n
 *	  User Application -> Framework -> OS-HAL -> M-HAL -> HDL
 *
 *	- User Application:
 *	  Performs a group of coordinated functions, tasks,
 *        or activities for the benefit of the user.
 *	- Framework:
 *	  An abstraction in which software provides generic functionality
 *        to avoid HW design differences.
 *	- OS-HAL(OS Hardware Abstraction Layer):
 *	  Provides high level APIs to upper layer(defined as OS-related driver).
 *	- M-HAL(MediaTek Hardware Abstraction Layer):
 *	  Provides Fixed APIs(defined as Common Interface) to fully control
 *	  the MediaTek HW.
 *	- HDL(Hardware Driving Layer):
 *	  Handles low level related operations.
 *
 * - \b M-HAL \b driver \b architecture \n
 *	- The M-HAL driver architecture is shown below:\n
 *         @image html MT3620_M4_SW_Architecture.png
 *
 *	- The introduction of OSAI\n
 *       HDL and M-HAL are all defined to OS(operating system) disrelated,
 *       but these two layers still need to use OS interfaces, such as
 *       print log, read&write register, malloc/free buffer and so on.\n
 *       For these puposes, we define a software module and
 *       name it as OSAI(Operating System Abstract Interface).\n
 *       OSAI provides some fixed function group (which is OS disrelated)
 *       to help HW modules (such as SPI/I2C/UART/PWM...)
 *       to develop HDL and M-HAL SDK.\n
 *       Note:\n
 *       1. The function group which OSAI provided should not be
 *       called by OS-HAL and User Application.
 *       2. OSAI is the mapping of OS, it need to be reimplement
 *       if landing on a new OS.
 *
 * @}
 */

/**
 * @addtogroup M-HAL
 * @{
 * @section MHAL_3_Naming_Rule	  File and API Naming Convention
 *
 *  - \b File \b naming \b convention \n
 *	 - HDL\n
 *       The HDL header files are named as hdl_{module name}.h,
 *       such as hdl_spim.h.\n
 *       The HDL driver files are named as hdl_{module name}.c,
 *       such as hdl_spim.c.\n
 *       The HDL header and driver files are all in driver/chip/mtxxxx/inc
 *       and driver/chip/mtxxxx/src/common.
 *	 - M-HAL\n
 *       The M-HAL header files are named as mhal_{module name}.h,
 *       such as mhal_spim.h.\n
 *       The M-HAL driver files are named as mhal_{module name}.c,
 *       such as mhal_spim.c.\n
 *       The M-HAL header and driver files are all in driver/chip/mtxxxx/inc
 *       and driver/chip/mtxxxx/src/common.
 *	 - OS-HAL\n
 *       The OS-HAL header files are named as os_hal_{module name}.h,
 *       such as os_hal_spim.h.\n
 *       The OS-HAL driver files are named as os_hal_{module name}.c,
 *       such as os_hal_spim.c.\n
 *       The OS-HAL header and driver files are all in driver/chip/inc
 *       and driver/chip/mtxxxx/src.
 *
 *  - \b API \b naming \b convention \n
 *	 - External function names\n
 *	   HDL: mtk_hdl_{module name}_xxx, such as mtk_hdl_spim_prepare_hw.\n
 *	   M-HAL: mtk_mhal_{module name}_xxx,
 *         such as mtk_mhal_spim_prepare_hw.\n
 *	   OS-HAL: mtk_os_hal_{module name}_xxx,
 *         such as mtk_os_hal_spim_ctlr_init.\n
 *	 - Internal function names\n
 *	   HDL: \_mtk_hdl_{module name}_xxx.\n
 *	   M-HAL: \_mtk_mhal_{module name}_xxx.\n
 *	   OS-HAL: \_mtk_os_hal_{module name}_xxx.\n
 *
 * @}
 */

#endif /* __MHAL_H__ */

