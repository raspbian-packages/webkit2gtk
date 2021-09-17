/*
 * Copyright (C) 2020 Apple Inc. All rights reserved.
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

#include "config.h"
#include "WindowOrWorkerGlobalScopeFetch.h"

#include "DOMWindow.h"
#include "Document.h"
#include "FetchResponse.h"
#include "JSDOMPromiseDeferred.h"
#include "JSFetchResponse.h"
#include "UserGestureIndicator.h"
#include "WorkerGlobalScope.h"

namespace WebCore {

using FetchResponsePromise = DOMPromiseDeferred<IDLInterface<FetchResponse>>;

void WindowOrWorkerGlobalScopeFetch::fetch(DOMWindow& window, FetchRequest::Info&& input, FetchRequest::Init&& init, Ref<DeferredPromise>&& deferred)
{
    FetchResponsePromise promise = WTFMove(deferred);

    auto* document = window.document();
    if (!document) {
        promise.reject(InvalidStateError);
        return;
    }

    auto request = FetchRequest::create(*document, WTFMove(input), WTFMove(init));
    if (request.hasException()) {
        promise.reject(request.releaseException());
        return;
    }

    FetchResponse::fetch(*document, request.releaseReturnValue(), [promise = WTFMove(promise), userGestureToken = UserGestureIndicator::currentUserGesture()](ExceptionOr<FetchResponse&>&& result) mutable {
        if (!userGestureToken || userGestureToken->hasExpired(UserGestureToken::maximumIntervalForUserGestureForwardingForFetch()) || !userGestureToken->processingUserGesture()) {
            promise.settle(WTFMove(result));
            return;
        }

        UserGestureIndicator gestureIndicator(userGestureToken, UserGestureToken::GestureScope::MediaOnly, UserGestureToken::IsPropagatedFromFetch::Yes);
        promise.settle(WTFMove(result));
    });
}

void WindowOrWorkerGlobalScopeFetch::fetch(WorkerGlobalScope& scope, FetchRequest::Info&& input, FetchRequest::Init&& init, Ref<DeferredPromise>&& deferred)
{
    FetchResponsePromise promise = WTFMove(deferred);

    auto request = FetchRequest::create(scope, WTFMove(input), WTFMove(init));
    if (request.hasException()) {
        promise.reject(request.releaseException());
        return;
    }

    FetchResponse::fetch(scope, request.releaseReturnValue().get(), [promise = WTFMove(promise)](ExceptionOr<FetchResponse&>&& result) mutable {
        promise.settle(WTFMove(result));
    });
}

}
