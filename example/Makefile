#
#  Copyright (c) 2020, The OpenThread Authors.
#  All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are met:
#  1. Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#  2. Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#  3. Neither the name of the copyright holder nor the
#     names of its contributors may be used to endorse or promote products
#     derived from this software without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
#  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
#  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
#  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
#  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
#  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
#  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
#  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
#  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
#  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
#  POSSIBILITY OF SUCH DAMAGE.
#
#    Description:
#      Make file of the example CLI application.
#

PROJECT_NAME := ot-cli

EXTRA_COMPONENT_DIRS += $(PROJECT_PATH)/..

COMMON_FLAGS :=                                 \
    -DMBEDTLS_KEY_EXCHANGE_ECJPAKE_ENABLED      \
    -DOPENTHREAD_CONFIG_PLATFORM_UDP_ENABLE=0   \
    -DOPENTHREAD_CONFIG_PLATFORM_NETIF_ENABLE=0 \
    -DOPENTHREAD_ENABLE_BUILTIN_MBEDTLS=0       \
    -DOT_RADIO_UART_TXD=GPIO_NUM_4              \
    -DMBEDTLS_ECJPAKE_C                         \
    -Wno-deprecated-declarations

EXTRA_CFLAGS=$(COMMON_FLAGS)

EXTRA_CXXFLAGS=$(COMMON_FLAGS)

EXTRA_CPPFLAGS=$(COMMON_FLAGS)

include $(IDF_PATH)/make/project.mk
