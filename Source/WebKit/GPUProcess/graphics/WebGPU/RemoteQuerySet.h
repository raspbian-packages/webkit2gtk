/*
 * Copyright (C) 2021-2023 Apple Inc. All rights reserved.
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

#if ENABLE(GPU_PROCESS)

#include "StreamMessageReceiver.h"
#include "WebGPUIdentifier.h"
#include <wtf/Ref.h>
#include <wtf/text/WTFString.h>

namespace WebCore::WebGPU {
class QuerySet;
}

namespace IPC {
class StreamServerConnection;
}

namespace WebKit {

namespace WebGPU {
class ObjectHeap;
}

class RemoteQuerySet final : public IPC::StreamMessageReceiver {
    WTF_MAKE_FAST_ALLOCATED;
public:
    static Ref<RemoteQuerySet> create(WebCore::WebGPU::QuerySet& querySet, WebGPU::ObjectHeap& objectHeap, Ref<IPC::StreamServerConnection>&& streamConnection, WebGPUIdentifier identifier)
    {
        return adoptRef(*new RemoteQuerySet(querySet, objectHeap, WTFMove(streamConnection), identifier));
    }

    virtual ~RemoteQuerySet();

    void stopListeningForIPC();

private:
    friend class WebGPU::ObjectHeap;

    RemoteQuerySet(WebCore::WebGPU::QuerySet&, WebGPU::ObjectHeap&, Ref<IPC::StreamServerConnection>&&, WebGPUIdentifier);

    RemoteQuerySet(const RemoteQuerySet&) = delete;
    RemoteQuerySet(RemoteQuerySet&&) = delete;
    RemoteQuerySet& operator=(const RemoteQuerySet&) = delete;
    RemoteQuerySet& operator=(RemoteQuerySet&&) = delete;

    WebCore::WebGPU::QuerySet& backing() { return m_backing; }

    void didReceiveStreamMessage(IPC::StreamServerConnection&, IPC::Decoder&) final;

    void destroy();
    void destruct();

    void setLabel(String&&);

    Ref<WebCore::WebGPU::QuerySet> m_backing;
    WebGPU::ObjectHeap& m_objectHeap;
    Ref<IPC::StreamServerConnection> m_streamConnection;
    WebGPUIdentifier m_identifier;
};

} // namespace WebKit

#endif // ENABLE(GPU_PROCESS)
