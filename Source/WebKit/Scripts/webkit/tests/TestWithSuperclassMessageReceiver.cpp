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
#include "TestWithSuperclass.h"

#include "ArgumentCoders.h"
#include "Decoder.h"
#include "HandleMessage.h"
#include "TestClassName.h"
#if ENABLE(TEST_FEATURE)
#include "TestTwoStateEnum.h"
#endif
#include "TestWithSuperclassMessages.h"
#include <wtf/Optional.h>
#include <wtf/text/WTFString.h>

namespace Messages {

namespace TestWithSuperclass {

#if ENABLE(TEST_FEATURE)

void TestAsyncMessage::callReply(IPC::Decoder& decoder, CompletionHandler<void(uint64_t&&)>&& completionHandler)
{
    Optional<uint64_t> result;
    decoder >> result;
    if (!result) {
        ASSERT_NOT_REACHED();
        cancelReply(WTFMove(completionHandler));
        return;
    }
    completionHandler(WTFMove(*result));
}

void TestAsyncMessage::cancelReply(CompletionHandler<void(uint64_t&&)>&& completionHandler)
{
    completionHandler(IPC::AsyncReplyError<uint64_t>::create());
}

void TestAsyncMessage::send(std::unique_ptr<IPC::Encoder>&& encoder, IPC::Connection& connection, uint64_t result)
{
    *encoder << result;
    connection.sendSyncReply(WTFMove(encoder));
}

#endif

#if ENABLE(TEST_FEATURE)

void TestAsyncMessageWithNoArguments::callReply(IPC::Decoder& decoder, CompletionHandler<void()>&& completionHandler)
{
    completionHandler();
}

void TestAsyncMessageWithNoArguments::cancelReply(CompletionHandler<void()>&& completionHandler)
{
    completionHandler();
}

void TestAsyncMessageWithNoArguments::send(std::unique_ptr<IPC::Encoder>&& encoder, IPC::Connection& connection)
{
    connection.sendSyncReply(WTFMove(encoder));
}

#endif

#if ENABLE(TEST_FEATURE)

void TestAsyncMessageWithMultipleArguments::callReply(IPC::Decoder& decoder, CompletionHandler<void(bool&&, uint64_t&&)>&& completionHandler)
{
    Optional<bool> flag;
    decoder >> flag;
    if (!flag) {
        ASSERT_NOT_REACHED();
        cancelReply(WTFMove(completionHandler));
        return;
    }
    Optional<uint64_t> value;
    decoder >> value;
    if (!value) {
        ASSERT_NOT_REACHED();
        cancelReply(WTFMove(completionHandler));
        return;
    }
    completionHandler(WTFMove(*flag), WTFMove(*value));
}

void TestAsyncMessageWithMultipleArguments::cancelReply(CompletionHandler<void(bool&&, uint64_t&&)>&& completionHandler)
{
    completionHandler(IPC::AsyncReplyError<bool>::create(), IPC::AsyncReplyError<uint64_t>::create());
}

void TestAsyncMessageWithMultipleArguments::send(std::unique_ptr<IPC::Encoder>&& encoder, IPC::Connection& connection, bool flag, uint64_t value)
{
    *encoder << flag;
    *encoder << value;
    connection.sendSyncReply(WTFMove(encoder));
}

#endif

#if ENABLE(TEST_FEATURE)

void TestAsyncMessageWithConnection::callReply(IPC::Decoder& decoder, CompletionHandler<void(bool&&)>&& completionHandler)
{
    Optional<bool> flag;
    decoder >> flag;
    if (!flag) {
        ASSERT_NOT_REACHED();
        cancelReply(WTFMove(completionHandler));
        return;
    }
    completionHandler(WTFMove(*flag));
}

void TestAsyncMessageWithConnection::cancelReply(CompletionHandler<void(bool&&)>&& completionHandler)
{
    completionHandler(IPC::AsyncReplyError<bool>::create());
}

void TestAsyncMessageWithConnection::send(std::unique_ptr<IPC::Encoder>&& encoder, IPC::Connection& connection, bool flag)
{
    *encoder << flag;
    connection.sendSyncReply(WTFMove(encoder));
}

#endif

void TestSyncMessage::send(std::unique_ptr<IPC::Encoder>&& encoder, IPC::Connection& connection, uint8_t reply)
{
    *encoder << reply;
    connection.sendSyncReply(WTFMove(encoder));
}

void TestSynchronousMessage::send(std::unique_ptr<IPC::Encoder>&& encoder, IPC::Connection& connection, const Optional<WebKit::TestClassName>& optionalReply)
{
    *encoder << optionalReply;
    connection.sendSyncReply(WTFMove(encoder));
}

} // namespace TestWithSuperclass

} // namespace Messages

namespace WebKit {

void TestWithSuperclass::didReceiveMessage(IPC::Connection& connection, IPC::Decoder& decoder)
{
    auto protectedThis = makeRef(*this);
    if (decoder.messageName() == Messages::TestWithSuperclass::LoadURL::name()) {
        IPC::handleMessage<Messages::TestWithSuperclass::LoadURL>(decoder, this, &TestWithSuperclass::loadURL);
        return;
    }
#if ENABLE(TEST_FEATURE)
    if (decoder.messageName() == Messages::TestWithSuperclass::TestAsyncMessage::name()) {
        IPC::handleMessageAsync<Messages::TestWithSuperclass::TestAsyncMessage>(connection, decoder, this, &TestWithSuperclass::testAsyncMessage);
        return;
    }
#endif
#if ENABLE(TEST_FEATURE)
    if (decoder.messageName() == Messages::TestWithSuperclass::TestAsyncMessageWithNoArguments::name()) {
        IPC::handleMessageAsync<Messages::TestWithSuperclass::TestAsyncMessageWithNoArguments>(connection, decoder, this, &TestWithSuperclass::testAsyncMessageWithNoArguments);
        return;
    }
#endif
#if ENABLE(TEST_FEATURE)
    if (decoder.messageName() == Messages::TestWithSuperclass::TestAsyncMessageWithMultipleArguments::name()) {
        IPC::handleMessageAsync<Messages::TestWithSuperclass::TestAsyncMessageWithMultipleArguments>(connection, decoder, this, &TestWithSuperclass::testAsyncMessageWithMultipleArguments);
        return;
    }
#endif
#if ENABLE(TEST_FEATURE)
    if (decoder.messageName() == Messages::TestWithSuperclass::TestAsyncMessageWithConnection::name()) {
        IPC::handleMessageAsyncWantsConnection<Messages::TestWithSuperclass::TestAsyncMessageWithConnection>(connection, decoder, this, &TestWithSuperclass::testAsyncMessageWithConnection);
        return;
    }
#endif
    WebPageBase::didReceiveMessage(connection, decoder);
}

void TestWithSuperclass::didReceiveSyncMessage(IPC::Connection& connection, IPC::Decoder& decoder, std::unique_ptr<IPC::Encoder>& replyEncoder)
{
    auto protectedThis = makeRef(*this);
    if (decoder.messageName() == Messages::TestWithSuperclass::TestSyncMessage::name()) {
        IPC::handleMessageSynchronous<Messages::TestWithSuperclass::TestSyncMessage>(connection, decoder, replyEncoder, this, &TestWithSuperclass::testSyncMessage);
        return;
    }
    if (decoder.messageName() == Messages::TestWithSuperclass::TestSynchronousMessage::name()) {
        IPC::handleMessageSynchronous<Messages::TestWithSuperclass::TestSynchronousMessage>(connection, decoder, replyEncoder, this, &TestWithSuperclass::testSynchronousMessage);
        return;
    }
    UNUSED_PARAM(connection);
    UNUSED_PARAM(decoder);
    UNUSED_PARAM(replyEncoder);
    ASSERT_NOT_REACHED();
}

} // namespace WebKit
