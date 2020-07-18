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
#include <sys/lock.h>
#include <sys/select.h>

#include <esp_log.h>
#include <esp_vfs_dev.h>

#include <openthread/openthread-esp32.h>

#include "error_handling.h"

typedef struct Event
{
    int     mFd;
    bool    mIsOpen;
    bool    mCounter;
    _lock_t mLock;
} Event;

static Event sEvent = {
    .mFd      = -1,
    .mIsOpen  = false,
    .mCounter = false,
    .mLock    = 0, // lazy initialized
};

static SemaphoreHandle_t *sSignalSemaphore = NULL;

static esp_err_t event_start_select(int                nfds,
                                    fd_set *           readfds,
                                    fd_set *           writefds,
                                    fd_set *           exceptfds,
                                    SemaphoreHandle_t *signal_sem)
{
    esp_err_t error = ESP_OK;

    (void)writefds;
    (void)exceptfds;

    _lock_acquire_recursive(&sEvent.mLock);

    VerifyOrExit(sEvent.mIsOpen && nfds > 0 && FD_ISSET(sEvent.mFd, readfds), _lock_release_recursive(&sEvent.mLock));

    _lock_release_recursive(&sEvent.mLock);

    sSignalSemaphore = signal_sem;

    if (sEvent.mCounter > 0)
    {
        esp_vfs_select_triggered(sSignalSemaphore);
    }

exit:
    return error;
}

static void event_end_select()
{
    sSignalSemaphore = NULL;
}

static int event_open(const char *path, int flags, int mode)
{
    int fd = -1;

    (void)flags;
    (void)mode;

    _lock_acquire_recursive(&sEvent.mLock);

    VerifyOrExit(strcmp(path, OT_EVENT_VFS_SHORT_PATH) == 0, OT_NOOP);
    VerifyOrExit(!sEvent.mIsOpen, OT_NOOP);

    sEvent.mFd     = OT_RESERVED_FD_MIN;
    sEvent.mIsOpen = true;
    fd             = sEvent.mFd;

exit:
    _lock_release_recursive(&sEvent.mLock);
    return fd;
}

static ssize_t event_write(int fd, const void *data, size_t size)
{
    ssize_t ret = -1;

    (void)data;

    _lock_acquire_recursive(&sEvent.mLock);

    VerifyOrExit(fd == sEvent.mFd && sEvent.mIsOpen, _lock_release_recursive(&sEvent.mLock));

    ++sEvent.mCounter;
    _lock_release_recursive(&sEvent.mLock);

    if (sSignalSemaphore != NULL)
    {
        esp_vfs_select_triggered(sSignalSemaphore);
    }

    ret = size;

exit:
    return ret;
}

static ssize_t event_read(int fd, void *data, size_t size)
{
    int ret = -1;

    (void)data;
    (void)size;

    _lock_acquire_recursive(&sEvent.mLock);

    VerifyOrExit(fd == sEvent.mFd && sEvent.mIsOpen, OT_NOOP);

    ret             = sEvent.mCounter;
    sEvent.mCounter = 0;

exit:
    _lock_release_recursive(&sEvent.mLock);
    return ret;
}

static int event_close(int fd)
{
    int ret = -1;

    _lock_acquire_recursive(&sEvent.mLock);

    VerifyOrExit(fd == sEvent.mFd && sEvent.mIsOpen, OT_NOOP);

    sEvent.mIsOpen  = false;
    sEvent.mCounter = 0;

    ret = 0;

exit:
    _lock_release_recursive(&sEvent.mLock);
    return ret;
}

static void event_register(void)
{
    esp_vfs_t vfs = {
        .flags        = ESP_VFS_FLAG_DEFAULT,
        .write        = &event_write,
        .open         = &event_open,
        .fstat        = NULL,
        .close        = &event_close,
        .read         = &event_read,
        .fcntl        = NULL,
        .fsync        = NULL,
        .access       = NULL,
        .start_select = &event_start_select,
        .end_select   = &event_end_select,
#ifdef CONFIG_SUPPORT_TERMIOS
        .tcsetattr = NULL,
        .tcgetattr = NULL,
        .tcdrain   = NULL,
        .tcflush   = NULL,
#endif // CONFIG_SUPPORT_TERMIOS
    };
    ESP_ERROR_CHECK(esp_vfs_register(OT_EVENT_VFS_PREFIX, &vfs, NULL));
}

/**
 * The single event fd used by the platform driver.
 */
static int sEventFd = -1;

void platformVfsEventInit(void)
{
    event_register();
    sEventFd = open(OT_EVENT_VFS_PATH, 0, 0);

    assert(sEventFd != -1);
}

void platformVfsEventDeinit(void)
{
    if (sEventFd != -1)
    {
        close(sEventFd);
        sEventFd = -1;
    }
}

void platformVfsEventUpdate(otSysMainloopContext *aMainloop)
{
    FD_SET(sEventFd, &aMainloop->mReadFdSet);
    if (sEventFd > aMainloop->mMaxFd)
    {
        aMainloop->mMaxFd = sEventFd;
    }
}

void platformVfsEventProcess(otInstance *aInstance, const otSysMainloopContext *aMainloop)
{
    (void)aInstance;

    if (FD_ISSET(sEventFd, &aMainloop->mReadFdSet))
    {
        // Consume the event.
        int cnt = read(sEventFd, NULL, 0);

        ESP_LOGD(OT_PLAT_LOG_TAG, "external event received, count: %d", cnt);
    }
}

void platformVfsEventActivate(void)
{
    assert(sEventFd != -1);

    // Write to the event fd to activate select().
    write(sEventFd, NULL, 0);
}
