/*
 * Copyright (C) 2014-2021 Apple Inc. All rights reserved.
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

#if ENABLE(VIDEO)

#include "MediaElementSession.h"

#include "Chrome.h"
#include "ChromeClient.h"
#include "DocumentInlines.h"
#include "DocumentLoader.h"
#include "ElementInlines.h"
#include "FullscreenManager.h"
#include "HTMLAudioElement.h"
#include "HTMLMediaElement.h"
#include "HTMLNames.h"
#include "HTMLVideoElement.h"
#include "HitTestResult.h"
#include "LocalFrame.h"
#include "LocalFrameView.h"
#include "Logging.h"
#include "MediaUsageInfo.h"
#include "Navigator.h"
#include "NowPlayingInfo.h"
#include "Page.h"
#include "PlatformMediaSessionManager.h"
#include "Quirks.h"
#include "RenderMedia.h"
#include "RenderView.h"
#include "RuntimeApplicationChecks.h"
#include "ScriptController.h"
#include "Settings.h"
#include "SourceBuffer.h"
#include <wtf/text/StringBuilder.h>

#if ENABLE(MEDIA_SESSION)
#include "MediaMetadata.h"
#include "MediaPositionState.h"
#include "MediaSession.h"
#include "MediaSessionPlaybackState.h"
#include "NavigatorMediaSession.h"
#endif

#if PLATFORM(IOS_FAMILY)
#include "AudioSession.h"
#include "RuntimeApplicationChecks.h"
#include <wtf/cocoa/RuntimeApplicationChecksCocoa.h>
#endif

namespace WebCore {

static const Seconds clientDataBufferingTimerThrottleDelay { 100_ms };
static const Seconds elementMainContentCheckInterval { 250_ms };

static bool isElementRectMostlyInMainFrame(const HTMLMediaElement&);
static bool isElementLargeEnoughForMainContent(const HTMLMediaElement&, MediaSessionMainContentPurpose);
static bool isElementMainContentForPurposesOfAutoplay(const HTMLMediaElement&, bool shouldHitTestMainFrame);
static bool isElementLongEnoughForMainContent(const HTMLMediaElement&);

#if !RELEASE_LOG_DISABLED

static String restrictionNames(MediaElementSession::BehaviorRestrictions restriction)
{
    StringBuilder restrictionBuilder;
#define CASE(restrictionType) \
    if (restriction & MediaElementSession::restrictionType) { \
        if (!restrictionBuilder.isEmpty()) \
            restrictionBuilder.append(", "); \
        restrictionBuilder.append(#restrictionType); \
    } \

    CASE(NoRestrictions)
    CASE(RequireUserGestureForLoad)
    CASE(RequireUserGestureForVideoRateChange)
    CASE(RequireUserGestureForAudioRateChange)
    CASE(RequireUserGestureForFullscreen)
    CASE(RequirePageConsentToLoadMedia)
    CASE(RequirePageConsentToResumeMedia)
    CASE(RequireUserGestureToShowPlaybackTargetPicker)
    CASE(WirelessVideoPlaybackDisabled)
    CASE(RequireUserGestureToAutoplayToExternalDevice)
    CASE(AutoPreloadingNotPermitted)
    CASE(InvisibleAutoplayNotPermitted)
    CASE(OverrideUserGestureRequirementForMainContent)
    CASE(RequireUserGestureToControlControlsManager)
    CASE(RequirePlaybackToControlControlsManager)
    CASE(RequireUserGestureForVideoDueToLowPowerMode)

    return restrictionBuilder.toString();
}

#endif

static bool pageExplicitlyAllowsElementToAutoplayInline(const HTMLMediaElement& element)
{
    Document& document = element.document();
    Page* page = document.page();
    return document.isMediaDocument() && !document.ownerElement() && page && page->allowsMediaDocumentInlinePlayback();
}

#if ENABLE(MEDIA_SESSION)
class MediaSessionObserver : public MediaSession::Observer {
    WTF_MAKE_FAST_ALLOCATED;

public:
    MediaSessionObserver(MediaElementSession& session, const Ref<MediaSession>& mediaSession)
        : m_session(session), m_mediaSession(mediaSession)
    {
        m_mediaSession->addObserver(*this);
    }
    ~MediaSessionObserver()
    {
        m_mediaSession->removeObserver(*this);
    }
    void metadataChanged(const RefPtr<MediaMetadata>& metadata) final
    {
        if (m_session)
            m_session->metadataChanged(metadata);
    }
    void positionStateChanged(const std::optional<MediaPositionState>& state) final
    {
        if (m_session)
            m_session->positionStateChanged(state);
    }
    void playbackStateChanged(MediaSessionPlaybackState state) final
    {
        if (m_session)
            m_session->playbackStateChanged(state);
    }
    void actionHandlersChanged()
    {
        if (m_session)
            m_session->actionHandlersChanged();
    }
private:
    WeakPtr<MediaElementSession> m_session;
    Ref<MediaSession> m_mediaSession;
};
#endif

MediaElementSession::MediaElementSession(HTMLMediaElement& element)
    : PlatformMediaSession(PlatformMediaSessionManager::sharedManager(), element)
    , m_element(element)
    , m_restrictions(NoRestrictions)
#if ENABLE(WIRELESS_PLAYBACK_TARGET)
    , m_targetAvailabilityChangedTimer(*this, &MediaElementSession::targetAvailabilityChangedTimerFired)
#endif
    , m_mainContentCheckTimer(*this, &MediaElementSession::mainContentCheckTimerFired)
    , m_clientDataBufferingTimer(*this, &MediaElementSession::clientDataBufferingTimerFired)
#if !RELEASE_LOG_DISABLED
    , m_logIdentifier(element.logIdentifier())
#endif
{
}

MediaElementSession::~MediaElementSession()
{
#if ENABLE(MEDIA_USAGE)
    auto page = m_element.document().page();
    if (page && m_haveAddedMediaUsageManagerSession)
        page->chrome().client().removeMediaUsageManagerSession(mediaSessionIdentifier());
#endif
}

void MediaElementSession::addMediaUsageManagerSessionIfNecessary()
{
#if ENABLE(MEDIA_USAGE)
    if (m_haveAddedMediaUsageManagerSession)
        return;

    auto page = m_element.document().page();
    if (!page)
        return;

    m_haveAddedMediaUsageManagerSession = true;
    page->chrome().client().addMediaUsageManagerSession(mediaSessionIdentifier(), m_element.sourceApplicationIdentifier(), m_element.document().url());
#endif
}

void MediaElementSession::registerWithDocument(Document& document)
{
#if ENABLE(WIRELESS_PLAYBACK_TARGET)
    document.addPlaybackTargetPickerClient(*this);
#else
    UNUSED_PARAM(document);
#endif
    ensureIsObservingMediaSession();
}

void MediaElementSession::unregisterWithDocument(Document& document)
{
#if ENABLE(WIRELESS_PLAYBACK_TARGET)
    document.removePlaybackTargetPickerClient(*this);
#else
    UNUSED_PARAM(document);
#endif
#if ENABLE(MEDIA_SESSION)
    m_observer = nullptr;
#endif
}

void MediaElementSession::clientWillBeginAutoplaying()
{
    PlatformMediaSession::clientWillBeginAutoplaying();
    m_elementIsHiddenBecauseItWasRemovedFromDOM = false;
    updateClientDataBuffering();
}

bool MediaElementSession::clientWillBeginPlayback()
{
    if (!PlatformMediaSession::clientWillBeginPlayback())
        return false;

    m_elementIsHiddenBecauseItWasRemovedFromDOM = false;
    updateClientDataBuffering();

#if ENABLE(MEDIA_SESSION)
    if (auto* session = mediaSession())
        session->willBeginPlayback();
#endif

    return true;
}

bool MediaElementSession::clientWillPausePlayback()
{
    if (!PlatformMediaSession::clientWillPausePlayback())
        return false;

    updateClientDataBuffering();

#if ENABLE(MEDIA_SESSION)
    if (auto* session = mediaSession())
        session->willPausePlayback();
#endif

    return true;
}

void MediaElementSession::visibilityChanged()
{
    scheduleClientDataBufferingCheck();

    bool elementIsHidden = m_element.elementIsHidden();

    if (elementIsHidden)
        m_elementIsHiddenUntilVisibleInViewport = true;
    else if (m_element.isVisibleInViewport())
        m_elementIsHiddenUntilVisibleInViewport = false;

    bool isPlayingAudio = m_element.isPlaying() && m_element.hasAudio() && !m_element.muted() && m_element.volume();
    if (!isPlayingAudio) {
        if (elementIsHidden) {
            ALWAYS_LOG(LOGIDENTIFIER, "Suspending silent playback after page visibility: hidden");
            beginInterruption(PlatformMediaSession::InterruptionType::EnteringBackground);
        } else {
            ALWAYS_LOG(LOGIDENTIFIER, "Resuming silent playback after page visibility: showing");
            endInterruption(PlatformMediaSession::EndInterruptionFlags::MayResumePlaying);
        }
        return;
    }

    if (hasBehaviorRestriction(RequirePageVisibilityToPlayAudio)) {
        if (elementIsHidden) {
            ALWAYS_LOG(LOGIDENTIFIER, "Suspending audible playback after page visibility: hidden");
            beginInterruption(PlatformMediaSession::InterruptionType::EnteringBackground);
        } else {
            ALWAYS_LOG(LOGIDENTIFIER, "Resuming audible playback after page visibility: showing");
            endInterruption(PlatformMediaSession::EndInterruptionFlags::MayResumePlaying);
        }
    }
}

void MediaElementSession::isVisibleInViewportChanged()
{
    scheduleClientDataBufferingCheck();

    if (m_element.isFullscreen() || m_element.isVisibleInViewport())
        m_elementIsHiddenUntilVisibleInViewport = false;

#if PLATFORM(COCOA) && !HAVE(CGS_FIX_FOR_RADAR_97530095)
    PlatformMediaSessionManager::sharedManager().scheduleSessionStatusUpdate();
#endif
}

void MediaElementSession::inActiveDocumentChanged()
{
    m_elementIsHiddenBecauseItWasRemovedFromDOM = !m_element.inActiveDocument();
    scheduleClientDataBufferingCheck();
}

void MediaElementSession::scheduleClientDataBufferingCheck()
{
    if (!m_clientDataBufferingTimer.isActive())
        m_clientDataBufferingTimer.startOneShot(clientDataBufferingTimerThrottleDelay);
}

void MediaElementSession::clientDataBufferingTimerFired()
{
    INFO_LOG(LOGIDENTIFIER, "visible = ", m_element.elementIsHidden());

    updateClientDataBuffering();

    if (state() != PlatformMediaSession::State::Playing || !m_element.elementIsHidden())
        return;

    PlatformMediaSessionManager::SessionRestrictions restrictions = PlatformMediaSessionManager::sharedManager().restrictions(mediaType());
    if ((restrictions & PlatformMediaSessionManager::BackgroundTabPlaybackRestricted) == PlatformMediaSessionManager::BackgroundTabPlaybackRestricted)
        pauseSession();
}

void MediaElementSession::updateClientDataBuffering()
{
    if (m_clientDataBufferingTimer.isActive())
        m_clientDataBufferingTimer.stop();

    m_element.setBufferingPolicy(preferredBufferingPolicy());

#if PLATFORM(IOS_FAMILY)
    PlatformMediaSessionManager::sharedManager().configureWirelessTargetMonitoring();
#endif
}

void MediaElementSession::addBehaviorRestriction(BehaviorRestrictions restrictions)
{
    if (restrictions & ~m_restrictions)
        INFO_LOG(LOGIDENTIFIER, "adding ", restrictionNames(restrictions & ~m_restrictions));

    m_restrictions |= restrictions;

    if (restrictions & OverrideUserGestureRequirementForMainContent)
        m_mainContentCheckTimer.startRepeating(elementMainContentCheckInterval);
}

void MediaElementSession::removeBehaviorRestriction(BehaviorRestrictions restriction)
{
    if (restriction & RequireUserGestureToControlControlsManager) {
        m_mostRecentUserInteractionTime = MonotonicTime::now();
        if (auto page = m_element.document().page())
            page->setAllowsPlaybackControlsForAutoplayingAudio(true);
    }

    if (!(m_restrictions & restriction))
        return;

    INFO_LOG(LOGIDENTIFIER, "removed ", restrictionNames(m_restrictions & restriction));
    m_restrictions &= ~restriction;
}

Expected<void, MediaPlaybackDenialReason> MediaElementSession::playbackStateChangePermitted(MediaPlaybackState state) const
{
    INFO_LOG(LOGIDENTIFIER, "state = ", state);
    if (m_element.isSuspended()) {
        ALWAYS_LOG(LOGIDENTIFIER, "Returning FALSE because element is suspended");
        return makeUnexpected(MediaPlaybackDenialReason::InvalidState);
    }

    auto& document = m_element.document();
    auto* page = document.page();
    if (!page || page->mediaPlaybackIsSuspended()) {
        ALWAYS_LOG(LOGIDENTIFIER, "Returning FALSE because media playback is suspended");
        return makeUnexpected(MediaPlaybackDenialReason::PageConsentRequired);
    }

    if (document.isMediaDocument() && !document.ownerElement())
        return { };

    if (pageExplicitlyAllowsElementToAutoplayInline(m_element))
        return { };

    if (requiresFullscreenForVideoPlayback() && !fullscreenPermitted()) {
        ALWAYS_LOG(LOGIDENTIFIER, "Returning FALSE because of fullscreen restriction");
        return makeUnexpected(MediaPlaybackDenialReason::FullscreenRequired);
    }

    if (m_restrictions & OverrideUserGestureRequirementForMainContent && updateIsMainContent())
        return { };

#if ENABLE(MEDIA_STREAM)
    if (m_element.hasMediaStreamSrcObject()) {
        if (document.isCapturing())
            return { };
        if (document.mediaState() & MediaProducerMediaState::IsPlayingAudio)
            return { };
    }
#endif

    // FIXME: Why are we checking top-level document only for PerDocumentAutoplayBehavior?
    const auto& topDocument = document.topDocument();
    if (topDocument.quirks().requiresUserGestureToPauseInPictureInPicture()
        && m_element.fullscreenMode() & HTMLMediaElementEnums::VideoFullscreenModePictureInPicture
        && !m_element.paused() && state == MediaPlaybackState::Paused
        && !document.processingUserGestureForMedia()) {
        ALWAYS_LOG(LOGIDENTIFIER, "Returning FALSE because a quirk requires a user gesture to pause while in Picture-in-Picture");
        return makeUnexpected(MediaPlaybackDenialReason::UserGestureRequired);
    }

    if (topDocument.mediaState() & MediaProducerMediaState::HasUserInteractedWithMediaElement && topDocument.quirks().needsPerDocumentAutoplayBehavior())
        return { };

    if (m_restrictions & RequireUserGestureForVideoRateChange && m_element.isVideo() && !document.processingUserGestureForMedia()) {
        ALWAYS_LOG(LOGIDENTIFIER, "Returning FALSE because a user gesture is required for video rate change restriction");
        return makeUnexpected(MediaPlaybackDenialReason::UserGestureRequired);
    }

    if (m_restrictions & RequireUserGestureForAudioRateChange && (!m_element.isVideo() || m_element.hasAudio()) && !m_element.muted() && m_element.volume() && !document.processingUserGestureForMedia()) {
        ALWAYS_LOG(LOGIDENTIFIER, "Returning FALSE because a user gesture is required for audio rate change restriction");
        return makeUnexpected(MediaPlaybackDenialReason::UserGestureRequired);
    }

    if (m_restrictions & RequirePageVisibilityToPlayAudio && (!m_element.isVideo() || m_element.hasAudio()) && !m_element.muted() && m_element.volume() && m_element.elementIsHidden()) {
        ALWAYS_LOG(LOGIDENTIFIER, "Returning FALSE because page visibility required for audio rate change restriction");
        return makeUnexpected(MediaPlaybackDenialReason::UserGestureRequired);
    }

    if (m_restrictions & RequireUserGestureForVideoDueToLowPowerMode && m_element.isVideo() && !document.processingUserGestureForMedia()) {
        ALWAYS_LOG(LOGIDENTIFIER, "Returning FALSE because of video low power mode restriction");
        return makeUnexpected(MediaPlaybackDenialReason::UserGestureRequired);
    }

    return { };
}

bool MediaElementSession::autoplayPermitted() const
{
    const Document& document = m_element.document();
    if (document.backForwardCacheState() != Document::NotInBackForwardCache)
        return false;
    if (document.activeDOMObjectsAreSuspended())
        return false;

    if (!hasBehaviorRestriction(MediaElementSession::InvisibleAutoplayNotPermitted))
        return true;

    // If the media element is audible, allow autoplay even when not visible as pausing it would be observable by the user.
    if ((!m_element.isVideo() || m_element.hasAudio()) && !m_element.muted() && m_element.volume())
        return true;

    auto* renderer = m_element.renderer();
    if (!renderer) {
        ALWAYS_LOG(LOGIDENTIFIER, "Returning FALSE because element has no renderer");
        return false;
    }
    if (renderer->style().visibility() != Visibility::Visible) {
        ALWAYS_LOG(LOGIDENTIFIER, "Returning FALSE because element is not visible");
        return false;
    }
    if (renderer->view().frameView().isOffscreen()) {
        ALWAYS_LOG(LOGIDENTIFIER, "Returning FALSE because frame is offscreen");
        return false;
    }
    if (renderer->visibleInViewportState() != VisibleInViewportState::Yes) {
        ALWAYS_LOG(LOGIDENTIFIER, "Returning FALSE because element is not visible in the viewport");
        return false;
    }
    return true;
}

bool MediaElementSession::dataLoadingPermitted() const
{
    if (m_restrictions & OverrideUserGestureRequirementForMainContent && updateIsMainContent())
        return true;

    if (m_restrictions & RequireUserGestureForLoad && !m_element.document().processingUserGestureForMedia()) {
        INFO_LOG(LOGIDENTIFIER, "returning FALSE");
        return false;
    }

    return true;
}

MediaPlayer::BufferingPolicy MediaElementSession::preferredBufferingPolicy() const
{
    if (isSuspended())
        return MediaPlayer::BufferingPolicy::MakeResourcesPurgeable;

    if (bufferingSuspended())
        return MediaPlayer::BufferingPolicy::LimitReadAhead;

    if (state() == PlatformMediaSession::State::Playing)
        return MediaPlayer::BufferingPolicy::Default;

    if (shouldOverrideBackgroundLoadingRestriction())
        return MediaPlayer::BufferingPolicy::Default;

#if ENABLE(WIRELESS_PLAYBACK_TARGET)
    if (m_shouldPlayToPlaybackTarget)
        return MediaPlayer::BufferingPolicy::Default;
#endif

    if (m_elementIsHiddenUntilVisibleInViewport || m_elementIsHiddenBecauseItWasRemovedFromDOM || m_element.elementIsHidden())
        return MediaPlayer::BufferingPolicy::MakeResourcesPurgeable;

    return MediaPlayer::BufferingPolicy::Default;
}

bool MediaElementSession::fullscreenPermitted() const
{
    if (m_restrictions & RequireUserGestureForFullscreen && !m_element.document().processingUserGestureForMedia()) {
        INFO_LOG(LOGIDENTIFIER, "returning FALSE");
        return false;
    }

    return true;
}

bool MediaElementSession::pageAllowsDataLoading() const
{
    Page* page = m_element.document().page();
    if (m_restrictions & RequirePageConsentToLoadMedia && page && !page->canStartMedia()) {
        INFO_LOG(LOGIDENTIFIER, "returning FALSE");
        return false;
    }

    return true;
}

bool MediaElementSession::pageAllowsPlaybackAfterResuming() const
{
    Page* page = m_element.document().page();
    if (m_restrictions & RequirePageConsentToResumeMedia && page && !page->canStartMedia()) {
        INFO_LOG(LOGIDENTIFIER, "returning FALSE");
        return false;
    }

    return true;
}

bool MediaElementSession::canShowControlsManager(PlaybackControlsPurpose purpose) const
{
    if (m_element.isSuspended() || !m_element.inActiveDocument()) {
        INFO_LOG(LOGIDENTIFIER, "returning FALSE: isSuspended()");
        return false;
    }

    if (m_element.isFullscreen()) {
        INFO_LOG(LOGIDENTIFIER, "returning TRUE: is fullscreen");
        return true;
    }

    if (m_element.muted()) {
        INFO_LOG(LOGIDENTIFIER, "returning FALSE: muted");
        return false;
    }

    if (m_element.document().isMediaDocument() && (m_element.document().frame() && m_element.document().frame()->isMainFrame())) {
        INFO_LOG(LOGIDENTIFIER, "returning TRUE: is media document");
        return true;
    }

    if (client().presentationType() == MediaType::Audio
        && purpose == PlaybackControlsPurpose::NowPlaying
        && !isLongEnoughForMainContent()) {
        INFO_LOG(LOGIDENTIFIER, "returning FALSE: audio too short for NowPlaying");
        return false;
    }

    if (client().presentationType() == MediaType::Audio && (purpose == PlaybackControlsPurpose::ControlsManager || purpose == PlaybackControlsPurpose::MediaSession)) {
        if (!hasBehaviorRestriction(RequireUserGestureToControlControlsManager) || m_element.document().processingUserGestureForMedia()) {
            INFO_LOG(LOGIDENTIFIER, "returning TRUE: audio element with user gesture");
            return true;
        }

        if (m_element.isPlaying() && allowsPlaybackControlsForAutoplayingAudio()) {
            INFO_LOG(LOGIDENTIFIER, "returning TRUE: user has played media before");
            return true;
        }

        INFO_LOG(LOGIDENTIFIER, "returning FALSE: audio element is not suitable");
        return false;
    }

    if (purpose == PlaybackControlsPurpose::ControlsManager && !isElementRectMostlyInMainFrame(m_element)) {
        INFO_LOG(LOGIDENTIFIER, "returning FALSE: not in main frame");
        return false;
    }

    if (!m_element.hasAudio() && !m_element.hasEverHadAudio()) {
        INFO_LOG(LOGIDENTIFIER, "returning FALSE: no audio");
        return false;
    }

    if (!playbackStateChangePermitted(MediaPlaybackState::Playing)) {
        INFO_LOG(LOGIDENTIFIER, "returning FALSE: playback not permitted");
        return false;
    }

    if (!hasBehaviorRestriction(RequireUserGestureToControlControlsManager) || m_element.document().processingUserGestureForMedia()) {
        INFO_LOG(LOGIDENTIFIER, "returning TRUE: no user gesture required");
        return true;
    }

    if (purpose == PlaybackControlsPurpose::ControlsManager && hasBehaviorRestriction(RequirePlaybackToControlControlsManager) && !m_element.isPlaying()) {
        INFO_LOG(LOGIDENTIFIER, "returning FALSE: needs to be playing");
        return false;
    }

    if (purpose != PlaybackControlsPurpose::MediaSession && !m_element.hasEverNotifiedAboutPlaying()) {
        INFO_LOG(LOGIDENTIFIER, "returning FALSE: hasn't fired playing notification");
        return false;
    }

#if ENABLE(FULLSCREEN_API)
    // Elements which are not descendants of the current fullscreen element cannot be main content.
    if (CheckedPtr fullsreenManager = m_element.document().fullscreenManagerIfExists()) {
        auto* fullscreenElement = fullsreenManager->currentFullscreenElement();
        if (fullscreenElement && !m_element.isDescendantOf(*fullscreenElement)) {
            INFO_LOG(LOGIDENTIFIER, "returning FALSE: outside of full screen");
            return false;
        }
    }
#endif

    // Only allow the main content heuristic to forbid videos from showing up if our purpose is the controls manager.
    if (purpose == PlaybackControlsPurpose::ControlsManager && m_element.isVideo()) {
        if (!m_element.renderer()) {
            INFO_LOG(LOGIDENTIFIER, "returning FALSE: no renderer");
            return false;
        }

        if (!m_element.hasVideo() && !m_element.hasEverHadVideo()) {
            INFO_LOG(LOGIDENTIFIER, "returning FALSE: no video");
            return false;
        }

        if (isLargeEnoughForMainContent(MediaSessionMainContentPurpose::MediaControls)) {
            INFO_LOG(LOGIDENTIFIER, "returning TRUE: is main content");
            return true;
        }
    }

    if (purpose == PlaybackControlsPurpose::NowPlaying || purpose == PlaybackControlsPurpose::MediaSession) {
        INFO_LOG(LOGIDENTIFIER, "returning TRUE: potentially plays audio");
        return true;
    }

    INFO_LOG(LOGIDENTIFIER, "returning FALSE: no user gesture");
    return false;
}

bool MediaElementSession::isLargeEnoughForMainContent(MediaSessionMainContentPurpose purpose) const
{
    return isElementLargeEnoughForMainContent(m_element, purpose);
}

bool MediaElementSession::isLongEnoughForMainContent() const
{
    return isElementLongEnoughForMainContent(m_element);
}

bool MediaElementSession::isMainContentForPurposesOfAutoplayEvents() const
{
    return isElementMainContentForPurposesOfAutoplay(m_element, false);
}

MonotonicTime MediaElementSession::mostRecentUserInteractionTime() const
{
    return m_mostRecentUserInteractionTime;
}

bool MediaElementSession::wantsToObserveViewportVisibilityForMediaControls() const
{
    return isLargeEnoughForMainContent(MediaSessionMainContentPurpose::MediaControls);
}

bool MediaElementSession::wantsToObserveViewportVisibilityForAutoplay() const
{
    return m_element.isVideo();
}

#if ENABLE(WIRELESS_PLAYBACK_TARGET)
void MediaElementSession::showPlaybackTargetPicker()
{
    ALWAYS_LOG(LOGIDENTIFIER);

    auto& document = m_element.document();
    if (m_restrictions & RequireUserGestureToShowPlaybackTargetPicker && !document.processingUserGestureForMedia()) {
        ALWAYS_LOG(LOGIDENTIFIER, "returning early because of permissions");
        return;
    }

    if (!document.page()) {
        ALWAYS_LOG(LOGIDENTIFIER, "returning early because page is NULL");
        return;
    }

#if !PLATFORM(IOS_FAMILY)
    if (m_element.readyState() < HTMLMediaElementEnums::HAVE_METADATA) {
        ALWAYS_LOG(LOGIDENTIFIER, "returning early because element is not playable");
        return;
    }
#endif

    auto& audioSession = AudioSession::sharedSession();
    document.showPlaybackTargetPicker(*this, is<HTMLVideoElement>(m_element), audioSession.routeSharingPolicy(), audioSession.routingContextUID());
}

bool MediaElementSession::hasWirelessPlaybackTargets() const
{
    INFO_LOG(LOGIDENTIFIER, "returning ", m_hasPlaybackTargets);

    return m_hasPlaybackTargets;
}

bool MediaElementSession::wirelessVideoPlaybackDisabled() const
{
    if (!m_element.document().settings().allowsAirPlayForMediaPlayback()) {
        INFO_LOG(LOGIDENTIFIER, "returning TRUE because of settings");
        return true;
    }

    if (m_element.hasAttributeWithoutSynchronization(HTMLNames::webkitwirelessvideoplaybackdisabledAttr)) {
        INFO_LOG(LOGIDENTIFIER, "returning TRUE because of attribute");
        return true;
    }

#if PLATFORM(IOS_FAMILY)
    auto& legacyAirplayAttributeValue = m_element.attributeWithoutSynchronization(HTMLNames::webkitairplayAttr);
    if (equalLettersIgnoringASCIICase(legacyAirplayAttributeValue, "deny"_s)) {
        INFO_LOG(LOGIDENTIFIER, "returning TRUE because of legacy attribute");
        return true;
    }
    if (equalLettersIgnoringASCIICase(legacyAirplayAttributeValue, "allow"_s)) {
        INFO_LOG(LOGIDENTIFIER, "returning FALSE because of legacy attribute");
        return false;
    }
#endif

    if (m_element.document().settings().remotePlaybackEnabled() && m_element.hasAttributeWithoutSynchronization(HTMLNames::disableremoteplaybackAttr)) {
        LOG(Media, "MediaElementSession::wirelessVideoPlaybackDisabled - returning TRUE because of RemotePlayback attribute");
        return true;
    }

    auto player = m_element.player();
    if (!player)
        return true;

    bool disabled = player->wirelessVideoPlaybackDisabled();
    INFO_LOG(LOGIDENTIFIER, "returning ", disabled, " because media engine says so");

    return disabled;
}

void MediaElementSession::setWirelessVideoPlaybackDisabled(bool disabled)
{
    if (disabled)
        addBehaviorRestriction(WirelessVideoPlaybackDisabled);
    else
        removeBehaviorRestriction(WirelessVideoPlaybackDisabled);

    auto player = m_element.player();
    if (!player)
        return;

    INFO_LOG(LOGIDENTIFIER, disabled);
    player->setWirelessVideoPlaybackDisabled(disabled);
}

void MediaElementSession::setHasPlaybackTargetAvailabilityListeners(bool hasListeners)
{
    INFO_LOG(LOGIDENTIFIER, hasListeners);

#if PLATFORM(IOS_FAMILY)
    m_hasPlaybackTargetAvailabilityListeners = hasListeners;
    PlatformMediaSessionManager::sharedManager().configureWirelessTargetMonitoring();
#else
    UNUSED_PARAM(hasListeners);
    m_element.document().playbackTargetPickerClientStateDidChange(*this, m_element.mediaState());
#endif
}

void MediaElementSession::setPlaybackTarget(Ref<MediaPlaybackTarget>&& device)
{
    m_playbackTarget = WTFMove(device);
    client().setWirelessPlaybackTarget(*m_playbackTarget.copyRef());
}

void MediaElementSession::targetAvailabilityChangedTimerFired()
{
    client().wirelessRoutesAvailableDidChange();
}

void MediaElementSession::externalOutputDeviceAvailableDidChange(bool hasTargets)
{
    if (m_hasPlaybackTargets == hasTargets)
        return;

    INFO_LOG(LOGIDENTIFIER, hasTargets);

    m_hasPlaybackTargets = hasTargets;
    m_targetAvailabilityChangedTimer.startOneShot(0_s);
}

bool MediaElementSession::isPlayingToWirelessPlaybackTarget() const
{
#if !PLATFORM(IOS_FAMILY)
    if (!m_playbackTarget || !m_playbackTarget->hasActiveRoute())
        return false;
#endif

    return client().isPlayingToWirelessPlaybackTarget();
}

void MediaElementSession::setShouldPlayToPlaybackTarget(bool shouldPlay)
{
    INFO_LOG(LOGIDENTIFIER, shouldPlay);
    m_shouldPlayToPlaybackTarget = shouldPlay;
    updateClientDataBuffering();
    client().setShouldPlayToPlaybackTarget(shouldPlay);
}

void MediaElementSession::playbackTargetPickerWasDismissed()
{
    INFO_LOG(LOGIDENTIFIER);
    client().playbackTargetPickerWasDismissed();
}

void MediaElementSession::mediaStateDidChange(MediaProducerMediaStateFlags state)
{
    m_element.document().playbackTargetPickerClientStateDidChange(*this, state);
}
#endif

MediaPlayer::Preload MediaElementSession::effectivePreloadForElement() const
{
    MediaPlayer::Preload preload = m_element.preloadValue();

    if (pageExplicitlyAllowsElementToAutoplayInline(m_element))
        return preload;

    if (m_restrictions & AutoPreloadingNotPermitted) {
        if (preload > MediaPlayer::Preload::MetaData)
            return MediaPlayer::Preload::MetaData;
    }

    return preload;
}

bool MediaElementSession::requiresFullscreenForVideoPlayback() const
{
    if (pageExplicitlyAllowsElementToAutoplayInline(m_element))
        return false;

    if (is<HTMLAudioElement>(m_element))
        return false;

    if (m_element.document().isMediaDocument()) {
        const HTMLVideoElement& videoElement = *downcast<const HTMLVideoElement>(&m_element);
        if (m_element.readyState() < HTMLVideoElement::HAVE_METADATA || !videoElement.hasEverHadVideo())
            return false;
    }

    if (m_element.isTemporarilyAllowingInlinePlaybackAfterFullscreen())
        return false;

    if (!m_element.document().settings().allowsInlineMediaPlayback())
        return true;

    if (!m_element.document().settings().inlineMediaPlaybackRequiresPlaysInlineAttribute())
        return false;

#if PLATFORM(IOS_FAMILY)
    if (CocoaApplication::isIBooks())
        return !m_element.hasAttributeWithoutSynchronization(HTMLNames::webkit_playsinlineAttr) && !m_element.hasAttributeWithoutSynchronization(HTMLNames::playsinlineAttr);
    if (!linkedOnOrAfterSDKWithBehavior(SDKAlignedBehavior::UnprefixedPlaysInlineAttribute))
        return !m_element.hasAttributeWithoutSynchronization(HTMLNames::webkit_playsinlineAttr);
#endif

    if (m_element.document().isMediaDocument() && m_element.document().ownerElement())
        return false;

    return !m_element.hasAttributeWithoutSynchronization(HTMLNames::playsinlineAttr);
}

bool MediaElementSession::allowsAutomaticMediaDataLoading() const
{
    if (pageExplicitlyAllowsElementToAutoplayInline(m_element))
        return true;

    if (m_element.document().settings().mediaDataLoadsAutomatically())
        return true;

    return false;
}

void MediaElementSession::mediaEngineUpdated()
{
    INFO_LOG(LOGIDENTIFIER);

#if ENABLE(WIRELESS_PLAYBACK_TARGET)
    if (m_restrictions & WirelessVideoPlaybackDisabled)
        setWirelessVideoPlaybackDisabled(true);
    if (m_playbackTarget)
        client().setWirelessPlaybackTarget(*m_playbackTarget.copyRef());
    if (m_shouldPlayToPlaybackTarget)
        client().setShouldPlayToPlaybackTarget(true);
#endif

}

void MediaElementSession::resetPlaybackSessionState()
{
    m_mostRecentUserInteractionTime = MonotonicTime();
    addBehaviorRestriction(RequireUserGestureToControlControlsManager | RequirePlaybackToControlControlsManager);
}

void MediaElementSession::suspendBuffering()
{
    updateClientDataBuffering();
}

void MediaElementSession::resumeBuffering()
{
    updateClientDataBuffering();
}

bool MediaElementSession::bufferingSuspended() const
{
    if (auto* page = m_element.document().page())
        return page->mediaBufferingIsSuspended();
    return true;
}

bool MediaElementSession::allowsPictureInPicture() const
{
    return m_element.document().settings().allowsPictureInPictureMediaPlayback();
}

#if PLATFORM(IOS_FAMILY)
bool MediaElementSession::requiresPlaybackTargetRouteMonitoring() const
{
    return m_hasPlaybackTargetAvailabilityListeners && !m_element.elementIsHidden();
}
#endif

#if ENABLE(MEDIA_SOURCE)
size_t MediaElementSession::maximumMediaSourceBufferSize(const SourceBuffer& buffer) const
{
    // A good quality 1080p video uses 8,000 kbps and stereo audio uses 384 kbps, so assume 95% for video and 5% for audio.
    const float bufferBudgetPercentageForVideo = .95;
    const float bufferBudgetPercentageForAudio = .05;

    size_t maximum = buffer.document().settings().maximumSourceBufferSize();

    // Allow a SourceBuffer to buffer as though it is audio-only even if it doesn't have any active tracks (yet).
    size_t bufferSize = static_cast<size_t>(maximum * bufferBudgetPercentageForAudio);
    if (buffer.hasVideo())
        bufferSize += static_cast<size_t>(maximum * bufferBudgetPercentageForVideo);

    // FIXME: we might want to modify this algorithm to:
    // - decrease the maximum size for background tabs
    // - decrease the maximum size allowed for inactive elements when a process has more than one
    //   element, eg. so a page with many elements which are played one at a time doesn't keep
    //   everything buffered after an element has finished playing.

    return bufferSize;
}
#endif

static bool isElementMainContentForPurposesOfAutoplay(const HTMLMediaElement& element, bool shouldHitTestMainFrame)
{
    Document& document = element.document();
    if (!document.hasLivingRenderTree() || document.activeDOMObjectsAreStopped() || element.isSuspended() || !element.hasAudio() || !element.hasVideo())
        return false;

    // Elements which have not yet been laid out, or which are not yet in the DOM, cannot be main content.
    auto* renderer = element.renderer();
    if (!renderer)
        return false;

    if (!isElementLargeEnoughForMainContent(element, MediaSessionMainContentPurpose::Autoplay))
        return false;

    // Elements which are hidden by style, or have been scrolled out of view, cannot be main content.
    // But elements which have audio & video and are already playing should not stop playing because
    // they are scrolled off the page.
    if (renderer->style().visibility() != Visibility::Visible)
        return false;
    if (renderer->visibleInViewportState() != VisibleInViewportState::Yes && !element.isPlaying())
        return false;

    // Main content elements must be in the main frame.
    if (!document.frame() || !document.frame()->isMainFrame())
        return false;

    auto* localFrame = dynamicDowncast<LocalFrame>(document.frame()->mainFrame());
    if (!localFrame)
        return false;

    auto& mainFrame = *localFrame;
    if (!mainFrame.view() || !mainFrame.view()->renderView())
        return false;

    if (!shouldHitTestMainFrame)
        return true;

    if (!mainFrame.document())
        return false;

    // Hit test the area of the main frame where the element appears, to determine if the element is being obscured.
    // Elements which are obscured by other elements cannot be main content.
    IntRect rectRelativeToView = element.boundingBoxInRootViewCoordinates();
    ScrollPosition scrollPosition = mainFrame.view()->documentScrollPositionRelativeToViewOrigin();
    IntRect rectRelativeToTopDocument(rectRelativeToView.location() + scrollPosition, rectRelativeToView.size());
    OptionSet<HitTestRequest::Type> hitType { HitTestRequest::Type::ReadOnly, HitTestRequest::Type::Active, HitTestRequest::Type::AllowChildFrameContent, HitTestRequest::Type::IgnoreClipping, HitTestRequest::Type::DisallowUserAgentShadowContent };
    HitTestResult result(rectRelativeToTopDocument.center());

    mainFrame.document()->hitTest(hitType, result);
    result.setToNonUserAgentShadowAncestor();
    RefPtr<Element> hitElement = result.targetElement();
    if (hitElement != &element)
        return false;

    return true;
}

static bool isElementRectMostlyInMainFrame(const HTMLMediaElement& element)
{
    if (!element.renderer())
        return false;

    RefPtr documentFrame = element.document().frame();
    if (!documentFrame)
        return false;

    RefPtr mainFrameView = documentFrame->mainFrame().virtualView();
    if (!mainFrameView)
        return false;

    IntRect mainFrameRectAdjustedForScrollPosition = IntRect(-mainFrameView->documentScrollPositionRelativeToViewOrigin(), mainFrameView->contentsSize());
    IntRect elementRectInMainFrame = element.boundingBoxInRootViewCoordinates();
    auto totalElementArea = elementRectInMainFrame.area<RecordOverflow>();
    if (totalElementArea.hasOverflowed())
        return false;

    elementRectInMainFrame.intersect(mainFrameRectAdjustedForScrollPosition);

    return elementRectInMainFrame.area() > totalElementArea / 2;
}

static bool isElementLargeRelativeToMainFrame(const HTMLMediaElement& element)
{
    static const double minimumPercentageOfMainFrameAreaForMainContent = 0.9;
    auto* renderer = element.renderer();
    if (!renderer)
        return false;

    RefPtr documentFrame = element.document().frame();
    if (!documentFrame)
        return false;

    RefPtr mainFrameView = documentFrame->mainFrame().virtualView();
    if (!mainFrameView)
        return false;

    auto maxVisibleClientWidth = std::min(renderer->clientWidth().toInt(), mainFrameView->visibleWidth());
    auto maxVisibleClientHeight = std::min(renderer->clientHeight().toInt(), mainFrameView->visibleHeight());

    return maxVisibleClientWidth * maxVisibleClientHeight > minimumPercentageOfMainFrameAreaForMainContent * mainFrameView->visibleWidth() * mainFrameView->visibleHeight();
}

static bool isElementLargeEnoughForMainContent(const HTMLMediaElement& element, MediaSessionMainContentPurpose purpose)
{
    static const double elementMainContentAreaMinimum = 400 * 300;
    static const double maximumAspectRatio = purpose == MediaSessionMainContentPurpose::MediaControls ? 3 : 1.8;
    static const double minimumAspectRatio = .5; // Slightly smaller than 9:16.

    // Elements which have not yet been laid out, or which are not yet in the DOM, cannot be main content.
    auto* renderer = element.renderer();
    if (!renderer)
        return false;

    double width = renderer->clientWidth();
    double height = renderer->clientHeight();
    double area = width * height;
    double aspectRatio = width / height;

    if (area < elementMainContentAreaMinimum)
        return false;

    if (aspectRatio >= minimumAspectRatio && aspectRatio <= maximumAspectRatio)
        return true;

    return isElementLargeRelativeToMainFrame(element);
}

static bool isElementLongEnoughForMainContent(const HTMLMediaElement& element)
{
    // Derived from the duration of the "You've got mail!" AOL sound:
    static constexpr MediaTime YouveGotMailDuration = MediaTime(95, 100);

    if (element.readyState() < HTMLMediaElementEnums::ReadyState::HAVE_METADATA)
        return false;

    return element.durationMediaTime() > YouveGotMailDuration;
}

void MediaElementSession::mainContentCheckTimerFired()
{
    if (!hasBehaviorRestriction(OverrideUserGestureRequirementForMainContent))
        return;

    updateIsMainContent();
}

bool MediaElementSession::updateIsMainContent() const
{
    if (m_element.isSuspended())
        return false;

    bool wasMainContent = m_isMainContent;
    m_isMainContent = isElementMainContentForPurposesOfAutoplay(m_element, true);

    if (m_isMainContent != wasMainContent)
        m_element.updateShouldPlay();

    return m_isMainContent;
}

bool MediaElementSession::allowsPlaybackControlsForAutoplayingAudio() const
{
    auto page = m_element.document().page();
    return page && page->allowsPlaybackControlsForAutoplayingAudio();
}

#if ENABLE(MEDIA_SESSION)
#if ENABLE(MEDIA_STREAM)
static bool isDocumentPlayingSeveralMediaStreamsAndCapturing(Document& document)
{
    // We restrict to capturing document for now, until we have a good way to state to the UIProcess application that audio rendering is muted from here.
    auto* page = document.page();
    return document.activeMediaElementsWithMediaStreamCount() > 1 && page && MediaProducer::isCapturing(page->mediaState());
}

static bool processRemoteControlCommandIfPlayingMediaStreams(Document& document, PlatformMediaSession::RemoteControlCommandType commandType)
{
    auto* page = document.page();
    if (!page)
        return false;

    if (!isDocumentPlayingSeveralMediaStreamsAndCapturing(document))
        return false;

    WebCore::MediaProducerMutedStateFlags mutedState;
    mutedState.add(WebCore::MediaProducerMutedState::AudioIsMuted);
    mutedState.add(WebCore::MediaProducer::AudioAndVideoCaptureIsMuted);
    mutedState.add(WebCore::MediaProducerMutedState::ScreenCaptureIsMuted);

    switch (commandType) {
    case PlatformMediaSession::RemoteControlCommandType::PlayCommand:
        page->setMuted({ });
        return true;
    case PlatformMediaSession::RemoteControlCommandType::StopCommand:
    case PlatformMediaSession::RemoteControlCommandType::PauseCommand:
        page->setMuted(mutedState);
        return true;
    case PlatformMediaSession::RemoteControlCommandType::TogglePlayPauseCommand:
        if (page->mutedState().containsAny(mutedState))
            page->setMuted({ });
        else
            page->setMuted(mutedState);
        return true;
    default:
        break;
    }
    return false;
}
#endif

void MediaElementSession::didReceiveRemoteControlCommand(RemoteControlCommandType commandType, const RemoteCommandArgument& argument)
{
    auto* session = mediaSession();
    if (!session || !session->hasActiveActionHandlers()) {
#if ENABLE(MEDIA_STREAM)
        if (processRemoteControlCommandIfPlayingMediaStreams(m_element.document(), commandType))
            return;
#endif
        PlatformMediaSession::didReceiveRemoteControlCommand(commandType, argument);
        return;
    }

    MediaSessionActionDetails actionDetails;
    switch (commandType) {
    case RemoteControlCommandType::NoCommand:
        return;
    case RemoteControlCommandType::PlayCommand:
        actionDetails.action = MediaSessionAction::Play;
        break;
    case RemoteControlCommandType::PauseCommand:
        actionDetails.action = MediaSessionAction::Pause;
        break;
    case RemoteControlCommandType::StopCommand:
        actionDetails.action = MediaSessionAction::Stop;
        break;
    case RemoteControlCommandType::TogglePlayPauseCommand:
        actionDetails.action = m_element.paused() ? MediaSessionAction::Play : MediaSessionAction::Pause;
        break;
    case RemoteControlCommandType::BeginScrubbingCommand:
        m_isScrubbing = true;
        return;
    case RemoteControlCommandType::EndScrubbingCommand:
        m_isScrubbing = false;
        return;
    case RemoteControlCommandType::SeekToPlaybackPositionCommand:
        ASSERT(argument.time);
        if (!argument.time)
            return;
        actionDetails.action = MediaSessionAction::Seekto;
        actionDetails.seekTime = argument.time.value();
        actionDetails.fastSeek = m_isScrubbing;
        break;
    case RemoteControlCommandType::SkipForwardCommand:
        if (argument.time)
            actionDetails.seekOffset = argument.time.value();
        actionDetails.action = MediaSessionAction::Seekforward;
        break;
    case RemoteControlCommandType::SkipBackwardCommand:
        if (argument.time)
            actionDetails.seekOffset = argument.time.value();
        actionDetails.action = MediaSessionAction::Seekbackward;
        break;
    case RemoteControlCommandType::NextTrackCommand:
        actionDetails.action = MediaSessionAction::Nexttrack;
        break;
    case RemoteControlCommandType::PreviousTrackCommand:
        actionDetails.action = MediaSessionAction::Previoustrack;
        break;
    case RemoteControlCommandType::BeginSeekingBackwardCommand:
    case RemoteControlCommandType::EndSeekingBackwardCommand:
    case RemoteControlCommandType::BeginSeekingForwardCommand:
    case RemoteControlCommandType::EndSeekingForwardCommand:
        ASSERT_NOT_REACHED();
        return;
    }

    session->callActionHandler(actionDetails);
}
#endif

std::optional<NowPlayingInfo> MediaElementSession::nowPlayingInfo() const
{
    auto* page = m_element.document().page();

#if ENABLE(MEDIA_SESSION)
    auto* session = mediaSession();
#endif

#if ENABLE(MEDIA_SESSION) && ENABLE(MEDIA_STREAM)
    if (isDocumentPlayingSeveralMediaStreamsAndCapturing(m_element.document()) && (!session || !session->hasActiveActionHandlers()))
        return { };
#endif

    bool allowsNowPlayingControlsVisibility = page && !page->isVisibleAndActive();
    bool isPlaying = state() == PlatformMediaSession::State::Playing;

    bool supportsSeeking = m_element.supportsSeeking();
    double rate = 1.0;
    double duration = supportsSeeking ? m_element.duration() : MediaPlayer::invalidTime();
    double currentTime = m_element.currentTime();
    if (!std::isfinite(currentTime) || !supportsSeeking)
        currentTime = MediaPlayer::invalidTime();
    auto sourceApplicationIdentifier = m_element.sourceApplicationIdentifier();
#if PLATFORM(COCOA)
    // FIXME: Eventually, this should be moved into HTMLMediaElement, so all clients
    // will use the same bundle identifier (the presentingApplication, rather than the
    // sourceApplication).
    if (!presentingApplicationBundleIdentifier().isNull())
        sourceApplicationIdentifier = presentingApplicationBundleIdentifier();
#endif

#if ENABLE(MEDIA_SESSION)
    auto positionState = session ? session->positionState() : std::nullopt;
    auto currentPosition = session ? session->currentPosition() : std::nullopt;
    if (positionState) {
        duration = positionState->duration;
        rate = positionState->playbackRate;
    }
    if (currentPosition)
        currentTime = *currentPosition;
    auto* sessionMetadata = session ? session->metadata() : nullptr;
    if (sessionMetadata) {
        std::optional<NowPlayingInfoArtwork> artwork;
        if (sessionMetadata->artworkImage()) {
            ASSERT(sessionMetadata->artworkImage()->data(), "An image must always have associated data");
            artwork = NowPlayingInfoArtwork { sessionMetadata->artworkSrc(), sessionMetadata->artworkImage()->mimeType(), sessionMetadata->artworkImage() };
        }
        return NowPlayingInfo { sessionMetadata->title(), sessionMetadata->artist(), sessionMetadata->album(), sourceApplicationIdentifier, duration, currentTime, rate, supportsSeeking, m_element.mediaUniqueIdentifier(), isPlaying, allowsNowPlayingControlsVisibility, WTFMove(artwork) };
    }
#endif

    return NowPlayingInfo { m_element.mediaSessionTitle(), emptyString(), emptyString(), sourceApplicationIdentifier, duration, currentTime, rate, supportsSeeking, m_element.mediaUniqueIdentifier(), isPlaying, allowsNowPlayingControlsVisibility, { } };
}

void MediaElementSession::updateMediaUsageIfChanged()
{
    auto& document = m_element.document();
    auto* page = document.page();
    if (!page || page->sessionID().isEphemeral())
        return;

    // Bail out early if the element currently has no source (currentSrc or
    // srcObject) and neither did the previous state, to avoid doing a large
    // amount of unnecessary work below.
    if (!m_element.hasSource() && (!m_mediaUsageInfo || !m_mediaUsageInfo->hasSource))
        return;

    bool isOutsideOfFullscreen = false;
#if ENABLE(FULLSCREEN_API)
    if (CheckedPtr fullscreenManager = document.fullscreenManagerIfExists()) {
        if (auto* fullscreenElement = document.fullscreenManager().currentFullscreenElement())
            isOutsideOfFullscreen = m_element.isDescendantOf(*fullscreenElement);
    }
#endif
    bool isAudio = client().presentationType() == MediaType::Audio;
    bool isVideo = client().presentationType() == MediaType::Video;
    bool processingUserGesture = document.processingUserGestureForMedia();
    bool isPlaying = m_element.isPlaying();

    MediaUsageInfo usage = {
        m_element.currentSrc(),
        m_element.hasSource(),
        state() == PlatformMediaSession::State::Playing,
        canShowControlsManager(PlaybackControlsPurpose::ControlsManager),
        !page->isVisibleAndActive(),
        m_element.isSuspended(),
        m_element.inActiveDocument(),
        m_element.isFullscreen(),
        m_element.muted(),
        document.isMediaDocument() && (document.frame() && document.frame()->isMainFrame()),
        isVideo,
        isAudio,
        m_element.hasVideo(),
        m_element.hasAudio(),
        m_element.hasRenderer(),
        isAudio && hasBehaviorRestriction(RequireUserGestureToControlControlsManager) && !processingUserGesture,
        m_element.hasAudio() && isPlaying && allowsPlaybackControlsForAutoplayingAudio(), // userHasPlayedAudioBefore
        isElementRectMostlyInMainFrame(m_element),
        !!playbackStateChangePermitted(MediaPlaybackState::Playing),
        page->mediaPlaybackIsSuspended(),
        document.isMediaDocument() && !document.ownerElement(),
        pageExplicitlyAllowsElementToAutoplayInline(m_element),
        requiresFullscreenForVideoPlayback() && !fullscreenPermitted(),
        isVideo && hasBehaviorRestriction(RequireUserGestureForVideoRateChange) && !processingUserGesture,
        isAudio && hasBehaviorRestriction(RequireUserGestureForAudioRateChange) && !processingUserGesture && !m_element.muted() && m_element.volume(),
        isVideo && hasBehaviorRestriction(RequireUserGestureForVideoDueToLowPowerMode) && !processingUserGesture,
        !hasBehaviorRestriction(RequireUserGestureToControlControlsManager) || processingUserGesture,
        hasBehaviorRestriction(RequirePlaybackToControlControlsManager) && !isPlaying,
        m_element.hasEverNotifiedAboutPlaying(),
        isOutsideOfFullscreen,
        isLargeEnoughForMainContent(MediaSessionMainContentPurpose::MediaControls),
#if PLATFORM(COCOA) && !HAVE(CGS_FIX_FOR_RADAR_97530095)
        m_element.isVisibleInViewport()
#endif
    };

    if (m_mediaUsageInfo && *m_mediaUsageInfo == usage)
        return;

    m_mediaUsageInfo = WTFMove(usage);

#if ENABLE(MEDIA_USAGE)
    addMediaUsageManagerSessionIfNecessary();
    page->chrome().client().updateMediaUsageManagerSessionState(mediaSessionIdentifier(), *m_mediaUsageInfo);
#endif
}

String convertEnumerationToString(const MediaPlaybackDenialReason enumerationValue)
{
    static const NeverDestroyed<String> values[] = {
        MAKE_STATIC_STRING_IMPL("UserGestureRequired"),
        MAKE_STATIC_STRING_IMPL("FullscreenRequired"),
        MAKE_STATIC_STRING_IMPL("PageConsentRequired"),
        MAKE_STATIC_STRING_IMPL("InvalidState"),
    };
    static_assert(static_cast<size_t>(MediaPlaybackDenialReason::UserGestureRequired) == 0, "MediaPlaybackDenialReason::UserGestureRequired is not 0 as expected");
    static_assert(static_cast<size_t>(MediaPlaybackDenialReason::FullscreenRequired) == 1, "MediaPlaybackDenialReason::FullscreenRequired is not 1 as expected");
    static_assert(static_cast<size_t>(MediaPlaybackDenialReason::PageConsentRequired) == 2, "MediaPlaybackDenialReason::PageConsentRequired is not 2 as expected");
    static_assert(static_cast<size_t>(MediaPlaybackDenialReason::InvalidState) == 3, "MediaPlaybackDenialReason::InvalidState is not 3 as expected");
    ASSERT(static_cast<size_t>(enumerationValue) < std::size(values));
    return values[static_cast<size_t>(enumerationValue)];
}

MediaSession* MediaElementSession::mediaSession() const
{
#if ENABLE(MEDIA_SESSION)
    auto* window = m_element.document().domWindow();
    if (!window)
        return nullptr;
    return &NavigatorMediaSession::mediaSession(window->navigator());
#else
    return nullptr;
#endif
}

void MediaElementSession::ensureIsObservingMediaSession()
{
#if ENABLE(MEDIA_SESSION)
    auto* session = mediaSession();
    if (!session || m_observer)
        return;
    m_observer = makeUnique<MediaSessionObserver>(*this, *session);
#endif
}

void MediaElementSession::metadataChanged(const RefPtr<MediaMetadata>&)
{
    clientCharacteristicsChanged(false);
}

void MediaElementSession::positionStateChanged(const std::optional<MediaPositionState>&)
{
    clientCharacteristicsChanged(false);
}

void MediaElementSession::playbackStateChanged(MediaSessionPlaybackState) { }

void MediaElementSession::actionHandlersChanged() { }

void MediaElementSession::clientCharacteristicsChanged(bool positionChanged)
{
#if ENABLE(MEDIA_SESSION)
    auto* session = mediaSession();
    if (positionChanged && session) {
        auto positionState = session->positionState();
        if (positionState)
            session->setPositionState(MediaPositionState { positionState->duration, positionState->playbackRate, m_element.currentTime() });
    }
#endif
    PlatformMediaSession::clientCharacteristicsChanged(positionChanged);
}

}

#endif // ENABLE(VIDEO)
