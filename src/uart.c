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

#include "platform-esp32.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/select.h>

#include <openthread/platform/uart.h>

#include <driver/uart.h>
#include <esp_log.h>
#include <esp_vfs_dev.h>

#include <openthread/openthread-esp32.h>

#include "error_handling.h"

static int sCliUartFd;

otError otPlatUartEnable(void)
{
    return OT_ERROR_NONE;
}

otError otPlatUartDisable(void)
{
    return OT_ERROR_NONE;
}

otError otPlatUartFlush(void)
{
    return OT_ERROR_NONE;
}

otError otPlatUartSend(const uint8_t *aBuf, uint16_t aBufLength)
{
    otError error = OT_ERROR_NONE;

    int rval = write(sCliUartFd, aBuf, aBufLength);

    VerifyOrExit(rval == (int)aBufLength, error = OT_ERROR_FAILED);

    otPlatUartSendDone();

exit:
    return error;
}

void platformCliUartInit()
{
    char          uartPath[16];
    uart_config_t uart_config = {.baud_rate           = 115200,
                                 .data_bits           = UART_DATA_8_BITS,
                                 .parity              = UART_PARITY_DISABLE,
                                 .stop_bits           = UART_STOP_BITS_1,
                                 .flow_ctrl           = UART_HW_FLOWCTRL_DISABLE,
                                 .rx_flow_ctrl_thresh = 0,
                                 .use_ref_tick        = false};
    ESP_ERROR_CHECK(uart_param_config(OT_CLI_UART_NUM, &uart_config));

    // Disable IO buffer.
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);

    // Install UART driver for interrupt-driven reads and writes.
    ESP_ERROR_CHECK(uart_driver_install(OT_CLI_UART_NUM, OT_UART_RX_BUF_SIZE, 0, 0, NULL, 0));

    // Tell VFS to use UART driver.
    esp_vfs_dev_uart_use_driver(OT_CLI_UART_NUM);

    // A workaround to configure line-ending per UART for issue#5.
    s_ctx[OT_CLI_UART_NUM]->rx_mode = ESP_LINE_ENDINGS_CRLF;
    s_ctx[OT_CLI_UART_NUM]->tx_mode = ESP_LINE_ENDINGS_CRLF;

    sprintf(uartPath, "/dev/uart/%d", OT_CLI_UART_NUM);
    sCliUartFd = open(uartPath, O_RDWR | O_NONBLOCK);

    VerifyOrDie(sCliUartFd != -1, OT_EXIT_FAILURE);
}

void platformCliUartDeinit(void)
{
    if (sCliUartFd != -1)
    {
        close(sCliUartFd);
        sCliUartFd = -1;
    }
    uart_driver_delete(OT_CLI_UART_NUM);
}

void platformCliUartUpdate(otSysMainloopContext *aMainloop)
{
    FD_SET(sCliUartFd, &aMainloop->mReadFdSet);
    if (sCliUartFd > aMainloop->mMaxFd)
    {
        aMainloop->mMaxFd = sCliUartFd;
    }
}

void platformCliUartProcess(otInstance *aInstance, const otSysMainloopContext *aMainloop)
{
    (void)aInstance;

    if (FD_ISSET(sCliUartFd, &aMainloop->mReadFdSet))
    {
        uint8_t buffer[256];

        int rval = read(sCliUartFd, buffer, sizeof(buffer));

        if (rval > 0)
        {
            otPlatUartReceived(buffer, (uint16_t)rval);
        }
        else if (rval > 0)
        {
            VerifyOrDie(errno == EAGAIN || errno == EINTR, OT_EXIT_FAILURE);
        }
    }
}
