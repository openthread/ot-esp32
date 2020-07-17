/*
 *  Copyright (c) 2020, The OpenThread Authors.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the copyright holder nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef OT_ESP32_PLATFORM_ESP32_H_
#define OT_ESP32_PLATFORM_ESP32_H_

#include <stdint.h>
#include <time.h>

#include <driver/gpio.h>

#include <openthread/instance.h>

#include <openthread/openthread-esp32.h>

#include "openthread-core-esp32-config.h"

/**
 * The default SPI flash partition used by OpenThread settings.
 *
 */
#define OT_FLASH_PARTITION_NAME "ot_storage"

/**
 * The default page number of SPI flash partition used by OpenThread settings.
 *
 */
#define SETTINGS_CONFIG_PAGE_NUM 2

/**
 * The default page size of SPI flash partition used by OpenThread settings.
 *
 */
#define SETTINGS_CONFIG_PAGE_SIZE 4096

/**
 * The default platform logging tag.
 *
 */
#define OT_PLAT_LOG_TAG "OT_ESP32_PLAT"

/**
 * The default TXD pin of the radio uart.
 *
 */
#ifndef OT_RADIO_UART_TXD
#define OT_RADIO_UART_TXD (GPIO_NUM_2)
#endif

/**
 * The default RXD pin of the radio uart.
 *
 */
#ifndef OT_RADIO_UART_RXD
#define OT_RADIO_UART_RXD (GPIO_NUM_5)
#endif

/**
 * The uart used by radio spinel.
 *
 */
#ifndef OT_RADIO_UART_NUM
#define OT_RADIO_UART_NUM (UART_NUM_1)
#endif

/**
 * The uart used by OpenThread CLI.
 *
 */
#define OT_CLI_UART_NUM (UART_NUM_0)

/**
 * The uart receive buffer size for both CLI uart and radio uart.
 *
 */
#define OT_UART_RX_BUF_SIZE (UART_FIFO_LEN * 2)

/**
 * The minimum fd number reserved by the OpenThread platform driver.
 *
 * In case the application makes its VFS implementation, the fd range
 * [OT_RESERVED_FD_MIN, OT_RESERVED_FD_MAX) should not be overlapped.
 *
 */
#define OT_RESERVED_FD_MIN (8)

/**
 * The maximum fd number reserved by the OpenThread platform driver.
 *
 * In case the application makes its VFS implementation, the fd range
 * [OT_RESERVED_FD_MIN, OT_RESERVED_FD_MAX) should not be overlapped.
 *
 */
#define OT_RESERVED_FD_MAX (9)

/**
 * The prefix of event device path.
 *
 */
#define OT_EVENT_VFS_PREFIX "/dev/event"

/**
 * The truncated path of virtual event file.
 *
 * This is the virtual event file that used to
 * wake up a blocked select().
 *
 */
#define OT_EVENT_VFS_SHORT_PATH "/ot"

/**
 * Full path to the virtual event file.
 *
 */
#define OT_EVENT_VFS_PATH OT_EVENT_VFS_PREFIX OT_EVENT_VFS_SHORT_PATH

/**
 * Milliseconds per Second.
 *
 */
#ifndef OT_MS_PER_S
#define OT_MS_PER_S 1000
#endif

/**
 * Microseconds per Millisecond.
 *
 */
#ifndef OT_US_PER_MS
#define OT_US_PER_MS 1000
#endif

/**
 * Microseconds per Second.
 *
 */
#ifndef OT_US_PER_S
#define OT_US_PER_S (OT_MS_PER_S * OT_US_PER_MS)
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * This function updates OpenThread alarm events to the mainloop context.
 *
 * @param[inout]  aMainloop  The mainloop context;
 *
 */
void platformAlarmUpdate(otSysMainloopContext *aMainloop);

/**
 * This function process alarm events.
 *
 * @param[in] aInstance  The OpenThread instance.
 * @param[in] aMainloop  The mainloop context.
 *
 */
void platformAlarmProcess(otInstance *aInstance, const otSysMainloopContext *aMainloop);

/**
 * This function initialize the CLI UART driver.
 *
 */
void platformCliUartInit(void);

/**
 * This function deinitialize the CLI UART driver.
 *
 */
void platformCliUartDeinit(void);

/**
 * This function updates CLI UART events to the mainloop context.
 *
 * param[inout] aMainloop  The mainloop context.
 *
 */
void platformCliUartUpdate(otSysMainloopContext *aMainloop);

/**
 * This function process CLI UART events.
 *
 * @param[in] aInstance  The OpenThread instance.
 * @param[in] aMainloop  The mainloop context.
 *
 */
void platformCliUartProcess(otInstance *aInstance, const otSysMainloopContext *aMainloop);

/**
 * This function initializes the radio transceiver.
 *
 * @param[in]  aResetRadio            TRUE to reset on init, FALSE to not reset on init.
 * @param[in]  aRestoreDatasetFromNcp TRUE to restore dataset to host from non-volatile memory
 *                                    (only used when attempts to upgrade from NCP to RCP mode),
 *                                    FALSE otherwise.
 *
 */
void platformRadioInit(bool aResetRadio, bool aRestoreDataSetFromNcp);

/**
 * This function deinitialize the radio driver.
 *
 */
void platformRadioDeinit(void);

/**
 * This function updates spinel radio events to the mainloop context.
 *
 * param[inout] aMainloop  The mainloop context.
 *
 */
void platformRadioUpdate(otSysMainloopContext *aMainloop);

/**
 * This function process radio events.
 *
 * @param[in] aInstance  The OpenThread instance.
 * @param[in] aMainloop  The mainloop context.
 *
 */
void platformRadioProcess(otInstance *aInstance, const otSysMainloopContext *aMainloop);

/**
 * This function initializes the API lock.
 *
 */
void platformApiLockInit(void);

/**
 * This function deinitialize the API lock.
 *
 */
void platformApiLockDeinit(void);

/**
 * This function initializes VFS driver of event file.
 *
 */
void platformVfsEventInit(void);

/**
 * This function deinitialize VFS driver of event file.
 *
 */
void platformVfsEventDeinit(void);

/**
 * This function updates event file events to the mainloop context.
 *
 * param[inout] aMainloop  The mainloop context.
 *
 */
void platformVfsEventUpdate(otSysMainloopContext *aMainloop);

/**
 * This function process event file events.
 *
 * @param[in] aInstance  The OpenThread instance.
 * @param[in] aMainloop  The mainloop context.
 *
 */
void platformVfsEventProcess(otInstance *aInstance, const otSysMainloopContext *aMainloop);

/**
 * This function activates the event file.
 *
 */
void platformVfsEventActivate(void);

#ifdef __cplusplus
}
#endif

#endif // OT_ESP32_PLATFORM_ESP32_H_
