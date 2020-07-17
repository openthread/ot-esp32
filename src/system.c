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

#include <sys/select.h>
#include <unistd.h>

#include <esp_log.h>

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include <openthread/tasklet.h>
#include <openthread/platform/alarm-milli.h>
#include <openthread/platform/time.h>

#include <openthread/openthread-esp32.h>

extern bool gPlatformPseudoResetWasRequested;

void otSysInit(int argc, char *argv[])
{
    OT_UNUSED_VARIABLE(argc);
    OT_UNUSED_VARIABLE(argv);

    if (gPlatformPseudoResetWasRequested)
    {
        gPlatformPseudoResetWasRequested = false;
    }

    platformVfsEventInit();
    platformApiLockInit();
    platformCliUartInit();
    platformRadioInit(/* aResetRadio */ true, /* aRestoreDataSetFromNcp */ false);

    ESP_LOGI(OT_PLAT_LOG_TAG, "init radio done");
}

void otSysDeinit(void)
{
    platformRadioDeinit();
    platformCliUartDeinit();
    platformApiLockDeinit();
    platformVfsEventDeinit();
}

bool otSysPseudoResetWasRequested(void)
{
    return gPlatformPseudoResetWasRequested;
}

void otSysMainloopInit(otSysMainloopContext *aMainloop)
{
    FD_ZERO(&aMainloop->mReadFdSet);
    FD_ZERO(&aMainloop->mWriteFdSet);
    FD_ZERO(&aMainloop->mErrorFdSet);

    aMainloop->mMaxFd           = -1;
    aMainloop->mTimeout.tv_sec  = 10;
    aMainloop->mTimeout.tv_usec = 0;
}

void otSysMainloopUpdate(otInstance *aInstance, otSysMainloopContext *aMainloop)
{
    platformVfsEventUpdate(aMainloop);
    platformAlarmUpdate(aMainloop);
    platformCliUartUpdate(aMainloop);
    platformRadioUpdate(aMainloop);

    if (otTaskletsArePending(aInstance))
    {
        aMainloop->mTimeout.tv_sec  = 0;
        aMainloop->mTimeout.tv_usec = 0;
    }
}

int otSysMainloopPoll(otSysMainloopContext *aMainloop)
{
    return select(aMainloop->mMaxFd + 1, &aMainloop->mReadFdSet, &aMainloop->mWriteFdSet, &aMainloop->mErrorFdSet,
                  &aMainloop->mTimeout);
}

void otSysMainloopProcess(otInstance *aInstance, const otSysMainloopContext *aMainloop)
{
    platformVfsEventProcess(aInstance, aMainloop);
    platformCliUartProcess(aInstance, aMainloop);
    platformRadioProcess(aInstance, aMainloop);
    platformAlarmProcess(aInstance, aMainloop);
}

void otSysMainloopBreak(void)
{
    platformVfsEventActivate();
}
