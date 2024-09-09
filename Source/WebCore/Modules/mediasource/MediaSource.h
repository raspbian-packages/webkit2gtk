/*
 * Copyright (C) 2013 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#if ENABLE(MEDIA_SOURCE)

#include "ActiveDOMObject.h"
#include "EventTarget.h"
#include "ExceptionOr.h"
#include "HTMLMediaElement.h"
#include "MediaPlayer.h"
#include "MediaPromiseTypes.h"
#include "MediaSourcePrivateClient.h"
#include "URLRegistry.h"
#include <optional>
#include <wtf/LoggerHelper.h>
#include <wtf/NativePromise.h>
#include <wtf/RefCounted.h>
#include <wtf/UniqueRef.h>
#include <wtf/Vector.h>
#include <wtf/WeakPtr.h>

namespace WebCore {

class ContentType;
class SourceBuffer;
class SourceBufferList;
class SourceBufferPrivate;
class TimeRanges;

class MediaSource
    : public MediaSourcePrivateClient
    , public ActiveDOMObject
    , public EventTarget
    , public URLRegistrable
#if !RELEASE_LOG_DISABLED
    , private LoggerHelper
#endif
{
    WTF_MAKE_ISO_ALLOCATED(MediaSource);
public:
    static void setRegistry(URLRegistry*);
    static MediaSource* lookup(const String& url) { return s_registry ? static_cast<MediaSource*>(s_registry->lookup(url)) : nullptr; }

    static Ref<MediaSource> create(ScriptExecutionContext&);
    virtual ~MediaSource();

    void addedToRegistry();
    void removedFromRegistry();
    void openIfInEndedState();
    void openIfDeferredOpen();
    virtual bool isOpen() const;
    virtual void monitorSourceBuffers();
    bool isClosed() const;
    bool isEnded() const;
    void sourceBufferDidChangeActiveState(SourceBuffer&, bool);
    MediaTime duration() const;
    const PlatformTimeRanges& buffered() const;

    enum class EndOfStreamError { Network, Decode };
    void streamEndedWithError(std::optional<EndOfStreamError>);

    bool attachToElement(HTMLMediaElement&);
    void detachFromElement(HTMLMediaElement&);
    bool isSeeking() const { return !!m_pendingSeekTarget; }
    Ref<TimeRanges> seekable();
    ExceptionOr<void> setLiveSeekableRange(double start, double end);
    ExceptionOr<void> clearLiveSeekableRange();

    ExceptionOr<void> setDuration(double);
    ExceptionOr<void> setDurationInternal(const MediaTime&);
    MediaTime currentTime() const;

    enum class ReadyState { Closed, Open, Ended };
    ReadyState readyState() const;
    ExceptionOr<void> endOfStream(std::optional<EndOfStreamError>);

    HTMLMediaElement* mediaElement() const { return m_mediaElement.get(); }

    SourceBufferList* sourceBuffers() { return m_sourceBuffers.get(); }
    SourceBufferList* activeSourceBuffers() { return m_activeSourceBuffers.get(); }
    ExceptionOr<Ref<SourceBuffer>> addSourceBuffer(const String& type);
    ExceptionOr<void> removeSourceBuffer(SourceBuffer&);
    static bool isTypeSupported(ScriptExecutionContext&, const String& type);

    ScriptExecutionContext* scriptExecutionContext() const final;

    using MediaSourcePrivateClient::ref;
    using MediaSourcePrivateClient::deref;

    static const MediaTime& currentTimeFudgeFactor();
    static bool contentTypeShouldGenerateTimestamps(const ContentType&);

#if !RELEASE_LOG_DISABLED
    const Logger& logger() const final { return m_logger.get(); }
    const void* logIdentifier() const final { return m_logIdentifier; }
    const char* logClassName() const final { return "MediaSource"; }
    WTFLogChannel& logChannel() const final;
    void setLogIdentifier(const void*) final;
#endif

    void failedToCreateRenderer(RendererType) final;

    virtual bool isManaged() const { return false; }
    virtual bool streaming() const { return false; }
    void memoryPressure();

    void setAsSrcObject(bool);

    // Called by SourceBuffer.
    void sourceBufferBufferedChanged();
    void sourceBufferReceivedFirstInitializationSegmentChanged();
    void sourceBufferActiveTrackFlagChanged(bool);
    void setMediaPlayerReadyState(MediaPlayer::ReadyState);

protected:
    explicit MediaSource(ScriptExecutionContext&);

    bool hasBufferedTime(const MediaTime&);
    bool hasCurrentTime();
    bool hasFutureTime();

    void scheduleEvent(const AtomString& eventName);
    void notifyElementUpdateMediaState() const;

    RefPtr<MediaSourcePrivate> m_private;
private:
    // ActiveDOMObject.
    void stop() final;
    const char* activeDOMObjectName() const final;
    bool virtualHasPendingActivity() const final;
    static bool isTypeSupported(ScriptExecutionContext&, const String& type, Vector<ContentType>&& contentTypesRequiringHardwareSupport);

    void setPrivateAndOpen(Ref<MediaSourcePrivate>&&) final;
    Ref<MediaTimePromise> waitForTarget(const SeekTarget&) final;
    Ref<MediaPromise> seekToTime(const MediaTime&) final;

    void refEventTarget() final { ref(); }
    void derefEventTarget() final { deref(); }
    EventTargetInterface eventTargetInterface() const final;

    URLRegistry& registry() const final;

    void setReadyState(ReadyState);
    void onReadyStateChange(ReadyState oldState, ReadyState newState);

    Vector<PlatformTimeRanges> activeRanges() const;

    ExceptionOr<Ref<SourceBufferPrivate>> createSourceBufferPrivate(const ContentType&);

    void regenerateActiveSourceBuffers();
    void updateBufferedIfNeeded(bool forced = false);

    void completeSeek();

    static URLRegistry* s_registry;

    RefPtr<SourceBufferList> m_sourceBuffers;
    RefPtr<SourceBufferList> m_activeSourceBuffers;
    PlatformTimeRanges m_liveSeekable;
    WeakPtr<HTMLMediaElement, WeakPtrImplWithEventTargetData> m_mediaElement;
    std::optional<SeekTarget> m_pendingSeekTarget;
    std::optional<MediaTimePromise::Producer> m_seekTargetPromise;
    ReadyState m_readyState { ReadyState::Closed };
    bool m_openDeferred { false };
    bool m_sourceopenPending { false };
#if !RELEASE_LOG_DISABLED
    Ref<const Logger> m_logger;
    const void* m_logIdentifier { nullptr };
#endif
    uint64_t m_associatedRegistryCount { 0 };
};

String convertEnumerationToString(MediaSource::EndOfStreamError);
String convertEnumerationToString(MediaSource::ReadyState);

} // namespace WebCore

namespace WTF {

template<typename Type>
struct LogArgument;

template <>
struct LogArgument<WebCore::MediaSource::EndOfStreamError> {
    static String toString(const WebCore::MediaSource::EndOfStreamError error)
    {
        return convertEnumerationToString(error);
    }
};

template <>
struct LogArgument<WebCore::MediaSource::ReadyState> {
    static String toString(const WebCore::MediaSource::ReadyState state)
    {
        return convertEnumerationToString(state);
    }
};

} // namespace WTF

#endif
