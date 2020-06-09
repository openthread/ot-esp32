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

#include "spinel_hdlc.hpp"

#include "platform-esp32.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/unistd.h>

#include <driver/uart.h>
#include <esp_heap_caps.h>
#include <esp_log.h>
#include <esp_vfs_dev.h>

#include <openthread/platform/time.h>

#include <openthread/openthread-esp32.h>

#include "error_handling.h"

namespace ot {

namespace Esp32 {

HdlcInterface::HdlcInterface(ot::Spinel::SpinelInterface::ReceiveFrameCallback aCallback,
                             void *                                            aCallbackContext,
                             ot::Spinel::SpinelInterface::RxFrameBuffer &      aFrameBuffer)
    : mReceiveFrameCallback(aCallback)
    , mReceiveFrameContext(aCallbackContext)
    , mReceiveFrameBuffer(aFrameBuffer)
    , mHdlcDecoder(aFrameBuffer, HandleHdlcFrame, this)
{
}

HdlcInterface::~HdlcInterface(void)
{
}

void HdlcInterface::Init(void)
{
    mUartRxBuffer = static_cast<uint8_t *>(heap_caps_malloc(kMaxFrameSize, MALLOC_CAP_8BIT));
    VerifyOrDie(mUartRxBuffer != NULL, OT_EXIT_FAILURE);

    InitUart();
}

void HdlcInterface::Deinit(void)
{
    DeinitUart();

    heap_caps_free(mUartRxBuffer);
    mUartRxBuffer = NULL;
}

otError HdlcInterface::SendFrame(const uint8_t *aFrame, uint16_t aLength)
{
    otError                              error = OT_ERROR_NONE;
    ot::Hdlc::FrameBuffer<kMaxFrameSize> encoderBuffer;
    ot::Hdlc::Encoder                    hdlcEncoder(encoderBuffer);

    SuccessOrExit(error = hdlcEncoder.BeginFrame());
    SuccessOrExit(error = hdlcEncoder.Encode(aFrame, aLength));
    SuccessOrExit(error = hdlcEncoder.EndFrame());

    SuccessOrExit(error = Write(encoderBuffer.GetFrame(), encoderBuffer.GetLength()));

exit:
    if (error != OT_ERROR_NONE)
    {
        ESP_LOGE(OT_PLAT_LOG_TAG, "send radio frame failed");
    }
    else
    {
        ESP_LOGD(OT_PLAT_LOG_TAG, "sent radio frame\n");
    }

    return error;
}

void HdlcInterface::Process(const otSysMainloopContext &aMainloop)
{
    if (FD_ISSET(mUartFd, &aMainloop.mReadFdSet))
    {
        ESP_LOGD(OT_PLAT_LOG_TAG, "radio uart read event");
        TryReadAndDecode();
    }
}

void HdlcInterface::Update(otSysMainloopContext &aMainloop)
{
    // Register only READ events for radio UART and always wait
    // for a radio WRITE to complete.
    FD_SET(mUartFd, &aMainloop.mReadFdSet);
    if (mUartFd > aMainloop.mMaxFd)
    {
        aMainloop.mMaxFd = mUartFd;
    }
}

int HdlcInterface::TryReadAndDecode(void)
{
    uint8_t buffer[kMaxFrameSize];
    ssize_t rval;

    rval = read(mUartFd, buffer, sizeof(buffer));

    if (rval > 0)
    {
        mHdlcDecoder.Decode(buffer, static_cast<uint16_t>(rval));
    }
    else if ((rval < 0) && (errno != EAGAIN) && (errno != EINTR))
    {
        abort();
    }

    return rval;
}

otError HdlcInterface::WaitForWritable(void)
{
    otError        error   = OT_ERROR_NONE;
    struct timeval timeout = {kMaxWaitTime / OT_MS_PER_S, (kMaxWaitTime % OT_MS_PER_S) * OT_US_PER_MS};
    uint64_t       now     = otPlatTimeGet();
    uint64_t       end     = now + kMaxWaitTime * OT_US_PER_MS;
    fd_set         writeFds;
    fd_set         errorFds;
    int            rval;

    while (true)
    {
        FD_ZERO(&writeFds);
        FD_ZERO(&errorFds);
        FD_SET(mUartFd, &writeFds);
        FD_SET(mUartFd, &errorFds);

        rval = select(mUartFd + 1, NULL, &writeFds, &errorFds, &timeout);

        if (rval > 0)
        {
            if (FD_ISSET(mUartFd, &writeFds))
            {
                ExitNow();
            }
            else if (FD_ISSET(mUartFd, &errorFds))
            {
                VerifyOrDie(false, OT_EXIT_FAILURE);
            }
            else
            {
                assert(false);
            }
        }
        else if ((rval < 0) && (errno != EINTR))
        {
            VerifyOrDie(false, OT_EXIT_FAILURE);
        }

        now = otPlatTimeGet();

        if (end > now)
        {
            uint64_t remain = end - now;

            timeout.tv_sec  = static_cast<time_t>(remain / 1000000);
            timeout.tv_usec = static_cast<suseconds_t>(remain % 1000000);
        }
        else
        {
            break;
        }
    }

    error = OT_ERROR_FAILED;

exit:
    return error;
}

otError HdlcInterface::Write(const uint8_t *aFrame, uint16_t aLength)
{
    otError error = OT_ERROR_NONE;

    while (aLength)
    {
        ssize_t rval;

        // Configure ESP-IDF UART to never convert "\n" to "\r\n" just before
        // writing radio frames through UART. This is a workaround for issue:
        // https://github.com/openthread/ot-esp32/issues/5.
        esp_vfs_dev_uart_set_tx_line_endings(ESP_LINE_ENDINGS_LF);
        rval = write(mUartFd, aFrame, aLength);
        esp_vfs_dev_uart_set_tx_line_endings(ESP_LINE_ENDINGS_CRLF);

        if (rval > 0)
        {
            assert(rval <= aLength);
            aLength -= static_cast<uint16_t>(rval);
            aFrame += static_cast<uint16_t>(rval);
            continue;
        }

        if ((rval < 0) && (errno != EAGAIN) && (errno != EWOULDBLOCK) && (errno != EINTR))
        {
            abort();
        }

        SuccessOrExit(error = WaitForWritable());
    }

exit:
    return error;
}

otError HdlcInterface::WaitForFrame(uint64_t aTimeoutUs)
{
    otError        error = OT_ERROR_NONE;
    struct timeval timeout;
    fd_set         read_fds;
    fd_set         error_fds;
    int            rval;

    FD_ZERO(&read_fds);
    FD_ZERO(&error_fds);
    FD_SET(mUartFd, &read_fds);
    FD_SET(mUartFd, &error_fds);

    timeout.tv_sec  = static_cast<time_t>(aTimeoutUs / OT_US_PER_S);
    timeout.tv_usec = static_cast<suseconds_t>(aTimeoutUs % OT_US_PER_S);

    rval = select(mUartFd + 1, &read_fds, NULL, &error_fds, &timeout);

    if (rval > 0)
    {
        if (FD_ISSET(mUartFd, &read_fds))
        {
            TryReadAndDecode();
        }
        else if (FD_ISSET(mUartFd, &error_fds))
        {
            abort();
        }
        else
        {
            abort();
        }
    }
    else if (rval == 0)
    {
        ExitNow(error = OT_ERROR_RESPONSE_TIMEOUT);
    }
    else if (errno != EINTR)
    {
        abort();
    }

exit:
    return error;
}

void HdlcInterface::HandleHdlcFrame(void *aContext, otError aError)
{
    static_cast<HdlcInterface *>(aContext)->HandleHdlcFrame(aError);
}

void HdlcInterface::HandleHdlcFrame(otError aError)
{
    if (aError == OT_ERROR_NONE)
    {
        ESP_LOGD(OT_PLAT_LOG_TAG, "received hdlc radio frame\n");
        mReceiveFrameCallback(mReceiveFrameContext);
    }
    else
    {
        ESP_LOGE(OT_PLAT_LOG_TAG, "dropping radio frame: %s\n", otThreadErrorToString(aError));
        mReceiveFrameBuffer.DiscardFrame();
    }
}

void HdlcInterface::InitUart(void)
{
    char          uartPath[16];
    uart_config_t uart_config = {.baud_rate           = 115200,
                                 .data_bits           = UART_DATA_8_BITS,
                                 .parity              = UART_PARITY_DISABLE,
                                 .stop_bits           = UART_STOP_BITS_1,
                                 .flow_ctrl           = UART_HW_FLOWCTRL_DISABLE,
                                 .rx_flow_ctrl_thresh = 0,
                                 .use_ref_tick        = false};

    ESP_ERROR_CHECK(uart_param_config(OT_RADIO_UART_NUM, &uart_config));
    ESP_ERROR_CHECK(
        uart_set_pin(OT_RADIO_UART_NUM, OT_RADIO_UART_TXD, OT_RADIO_UART_RXD, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install(OT_RADIO_UART_NUM, OT_UART_RX_BUF_SIZE, 0, 0, NULL, 0));

    // We have a driver now installed so set up the read/write functions to use driver also.
    esp_vfs_dev_uart_use_driver(OT_RADIO_UART_NUM);

    sprintf(uartPath, "/dev/uart/%d", OT_RADIO_UART_NUM);
    mUartFd = open(uartPath, O_RDWR | O_NONBLOCK);

    VerifyOrDie(mUartFd != -1, OT_EXIT_FAILURE);
}

void HdlcInterface::DeinitUart(void)
{
    if (mUartFd != -1)
    {
        close(mUartFd);
        mUartFd = -1;
    }
    ESP_ERROR_CHECK(uart_driver_delete(OT_RADIO_UART_NUM));
}

} // namespace Esp32

} // namespace ot
