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

/**
 * @file
 * @brief
 *   This file defines the platform-specific functions needed by OpenThread's example applications.
 */

#ifndef OT_ESP32_OPENTHREAD_ESP32_H_
#define OT_ESP32_OPENTHREAD_ESP32_H_

#include <stdbool.h>
#include <stdint.h>
#include <sys/select.h>

#include <openthread/error.h>
#include <openthread/instance.h>

/**
 * The minimum FreeRTOS stack size required by the OT stack.
 *
 * It is not recommended to run the OT stack in tasks with
 * stack size smaller than this value.
 *
 */
#define OT_MIN_RTOS_STACK_SIZE (10 * 1024)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * This structure represents a context for a select() based mainloop.
 *
 */
typedef struct otSysMainloopContext
{
    fd_set         mReadFdSet;  ///< The read file descriptors.
    fd_set         mWriteFdSet; ///< The write file descriptors.
    fd_set         mErrorFdSet; ///< The error file descriptors.
    int            mMaxFd;      ///< The max file descriptor.
    struct timeval mTimeout;    ///< The timeout.
} otSysMainloopContext;

/**
 * This function performs all platform-specific initialization of OpenThread's drivers.
 *
 * @note This function is not called by the OpenThread library. Instead, the system/RTOS should call this function
 *       when initialization of OpenThread's drivers is most appropriate.
 *
 * @param[in]  argc  Number of arguments in @p argv.
 * @param[in]  argv  Argument vector.
 *
 */
void otSysInit(int argc, char *argv[]);

/**
 * This function performs all platform-specific deinitialization for OpenThread's drivers.
 *
 * @note This function is not called by the OpenThread library. Instead, the system/RTOS should call this function
 *       when deinitialization of OpenThread's drivers is most appropriate.
 *
 */
void otSysDeinit(void);

/**
 * This function returns true if a pseudo-reset was requested.
 *
 * In such a case, the main loop should shut down and re-initialize the OpenThread instance.
 *
 * @note This function is not called by the OpenThread library. Instead, the system/RTOS should call this function
 *       in the main loop to determine when to shut down and re-initialize the OpenThread instance.
 *
 */
bool otSysPseudoResetWasRequested(void);

/**
 * This function initialize a mainloop context.
 *
 * @param[inout] aMainloop  A pointer to the mainloop to be initialized.
 *
 */
void otSysMainloopInit(otSysMainloopContext *aMainloop);

/**
 * This function updates the mainloop context.
 *
 * @param[in]       aInstance   The OpenThread instance structure.
 * @param[inout]    aMainloop   A pointer to the mainloop context.
 *
 */
void otSysMainloopUpdate(otInstance *aInstance, otSysMainloopContext *aMainloop);

/**
 * This function polls OpenThread's mainloop.
 *
 * @param[inout]    aMainloop   A pointer to the mainloop context.
 *
 * @returns value returned from select().
 *
 */
int otSysMainloopPoll(otSysMainloopContext *aMainloop);

/**
 * This function performs all platform-specific processing for OpenThread's example applications.
 *
 * @note This function is not called by the OpenThread library. Instead, the system/RTOS should call this function
 *       in the main loop when processing OpenThread's drivers is most appropriate.
 *
 * @param[in]   aInstance   The OpenThread instance structure.
 * @param[in]   aMainloop   A pointer to the mainloop context.
 *
 */
void otSysMainloopProcess(otInstance *aInstance, const otSysMainloopContext *aMainloop);

/**
 * This function breaks the mainloop.
 *
 * @note This function is designed to break the OpenThread mainloop in case there are
 *       external events which should be handled immediately.
 *
 */
void otSysMainloopBreak(void);

/**
 * This functions locks the OpenThread API lock.
 *
 * Every OT APIs that takes an otInstance argument
 * MUST be protected with this API lock except that
 * the call site is in OT callbacks.
 *
 */
void otSysApiLock(void);

/**
 * This function unlocks the OpenThread API lock.
 *
 */
void otSysApiUnlock(void);

#ifdef __cplusplus
} // end of extern "C"
#endif

#endif // OT_ESP32_OPENTHREAD_ESP32_H_
