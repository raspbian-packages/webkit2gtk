/*
 * Copyright (C) 2019 Apple Inc. All rights reserved.
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

#include "AuthenticationChallengeDisposition.h"
#include "AuthenticationChallengeProxy.h"
#include "AuthenticationDecisionListener.h"
#include <wtf/CompletionHandler.h>

namespace WebCore {
struct SecurityOriginData;
}

namespace WebKit {

class WebPageProxy;

class WebsiteDataStoreClient {
    WTF_MAKE_FAST_ALLOCATED;
public:
    virtual ~WebsiteDataStoreClient() { }

    virtual void requestStorageSpace(const WebCore::SecurityOriginData& topOrigin, const WebCore::SecurityOriginData& frameOrigin, uint64_t quota, uint64_t currentSize, uint64_t spaceRequired, CompletionHandler<void(std::optional<uint64_t>)>&& completionHandler)
    {
        completionHandler({ });
    }

    virtual void didReceiveAuthenticationChallenge(Ref<AuthenticationChallengeProxy>&& challenge)
    {
        challenge->listener().completeChallenge(AuthenticationChallengeDisposition::PerformDefaultHandling);
    }

    virtual void openWindowFromServiceWorker(const String&, const WebCore::SecurityOriginData&, CompletionHandler<void(WebPageProxy*)>&& completionHandler)
    {
        completionHandler(nullptr);
    }
};

} // namespace WebKit
