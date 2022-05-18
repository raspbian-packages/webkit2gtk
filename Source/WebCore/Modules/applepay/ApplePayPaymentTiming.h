/*
 * Copyright (C) 2021 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include <wtf/Forward.h>

#if USE(APPLE_INTERNAL_SDK)
#include <WebKitAdditions/ApplePayPaymentTimingAdditions.h>
#endif

namespace WebCore {

enum class ApplePayPaymentTiming : uint8_t {
    Immediate,
#if ENABLE(APPLE_PAY_RECURRING_LINE_ITEM)
    Recurring,
#endif
#if ENABLE(APPLE_PAY_DEFERRED_LINE_ITEM)
    Deferred,
#endif
#if defined(ApplePayPaymentTimingAdditions_members)
    ApplePayPaymentTimingAdditions_members
#endif
};

} // namespace WebCore

namespace WTF {

template<> struct EnumTraits<WebCore::ApplePayPaymentTiming> {
    using values = EnumValues<
        WebCore::ApplePayPaymentTiming,
        WebCore::ApplePayPaymentTiming::Immediate
#if ENABLE(APPLE_PAY_RECURRING_LINE_ITEM)
        , WebCore::ApplePayPaymentTiming::Recurring
#endif
#if ENABLE(APPLE_PAY_DEFERRED_LINE_ITEM)
        , WebCore::ApplePayPaymentTiming::Deferred
#endif
#if defined(ApplePayPaymentTimingAdditions_EnumTraits)
    ApplePayPaymentTimingAdditions_EnumTraits
#endif
    >;
};

} // namespace WTF
