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
#      This file is the component file builds ot-esp32 project as an esp-idf component.
#

COMPONENT_ADD_INCLUDEDIRS :=       \
    include                        \
    third_party/openthread/include

COMPONENT_PRIV_INCLUDEDIRS :=                                    \
    src                                                          \
    third_party/openthread/src                                   \
    third_party/openthread/src/core                              \
    third_party/openthread/src/lib/hdlc                          \
    third_party/openthread/src/lib/spinel                        \
    third_party/openthread/src/ncp                               \
    third_party/openthread/third_party/jlink/SEGGER_RTT_V640/RTT

COMPONENT_SRCDIRS :=                        \
    src                                     \
    third_party/openthread/src/cli          \
    third_party/openthread/src/core         \
    third_party/openthread/src/core/api     \
    third_party/openthread/src/core/coap    \
    third_party/openthread/src/core/common  \
    third_party/openthread/src/core/crypto  \
    third_party/openthread/src/core/mac     \
    third_party/openthread/src/core/meshcop \
    third_party/openthread/src/core/net     \
    third_party/openthread/src/core/radio   \
    third_party/openthread/src/core/thread  \
    third_party/openthread/src/core/utils   \
    third_party/openthread/src/lib/hdlc     \
    third_party/openthread/src/lib/spinel   \
    third_party/openthread/src/ncp

COMPONENT_OBJEXCLUDE := \
    third_party/openthread/src/core/common/extension_example.o

COMMON_FLAGS :=                                                              \
    -D_GNU_SOURCE                                                            \
    -DOPENTHREAD_CONFIG_FILE=\<openthread-core-esp32-config.h\>              \
    -DOPENTHREAD_FTD=1                                                       \
    -DOPENTHREAD_SPINEL_CONFIG_OPENTHREAD_MESSAGE_ENABLE=1                   \
    -DOPENTHREAD_PROJECT_CORE_CONFIG_FILE=\"openthread-core-esp32-config.h\" \
    -DSPINEL_PLATFORM_HEADER=\"spinel_platform.h\"                           \
    -Wno-error=non-virtual-dtor

CFLAGS += $(COMMON_FLAGS)

CXXFLAGS += $(COMMON_FLAGS)

CPPFLAGS += $(COMMON_FLAGS)
