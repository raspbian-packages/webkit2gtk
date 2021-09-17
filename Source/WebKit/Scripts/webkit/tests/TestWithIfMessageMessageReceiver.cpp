/*
 * Copyright (C) 2010-2021 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "TestWithIfMessage.h"

#if PLATFORM(COCOA) || PLATFORM(GTK)
#include "ArgumentCoders.h"
#endif
#include "Decoder.h"
#include "HandleMessage.h"
#include "TestWithIfMessageMessages.h"
#if PLATFORM(COCOA) || PLATFORM(GTK)
#include <wtf/text/WTFString.h>
#endif

namespace WebKit {

void TestWithIfMessage::didReceiveMessage(IPC::Connection& connection, IPC::Decoder& decoder)
{
    auto protectedThis = makeRef(*this);
#if PLATFORM(COCOA)
    if (decoder.messageName() == Messages::TestWithIfMessage::LoadURL::name()) {
        IPC::handleMessage<Messages::TestWithIfMessage::LoadURL>(decoder, this, &TestWithIfMessage::loadURL);
        return;
    }
#endif
#if PLATFORM(GTK)
    if (decoder.messageName() == Messages::TestWithIfMessage::LoadURL::name()) {
        IPC::handleMessage<Messages::TestWithIfMessage::LoadURL>(decoder, this, &TestWithIfMessage::loadURL);
        return;
    }
#endif
    UNUSED_PARAM(connection);
    UNUSED_PARAM(decoder);
    ASSERT_NOT_REACHED();
}

} // namespace WebKit
