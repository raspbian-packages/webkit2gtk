/*
 * Copyright (C) 2006, 2007, 2008, 2014 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#include "config.h"
#include "CachedPage.h"

#include "Document.h"
#include "Element.h"
#include "FocusController.h"
#include "FrameView.h"
#include "MainFrame.h"
#include "Node.h"
#include "Page.h"
#include "Settings.h"
#include "VisitedLinkState.h"
#include <wtf/CurrentTime.h>
#include <wtf/RefCountedLeakCounter.h>
#include <wtf/StdLibExtras.h>

#if PLATFORM(IOS)
#include "FrameSelection.h"
#endif

using namespace JSC;

namespace WebCore {

DEFINE_DEBUG_ONLY_GLOBAL(WTF::RefCountedLeakCounter, cachedPageCounter, ("CachedPage"));

CachedPage::CachedPage(Page& page)
    : m_expirationTime(monotonicallyIncreasingTime() + page.settings().backForwardCacheExpirationInterval())
    , m_cachedMainFrame(std::make_unique<CachedFrame>(page.mainFrame()))
{
#ifndef NDEBUG
    cachedPageCounter.increment();
#endif
}

CachedPage::~CachedPage()
{
#ifndef NDEBUG
    cachedPageCounter.decrement();
#endif

    if (m_cachedMainFrame)
        m_cachedMainFrame->destroy();
}

void CachedPage::restore(Page& page)
{
    ASSERT(m_cachedMainFrame);
    ASSERT(m_cachedMainFrame->view()->frame().isMainFrame());
    ASSERT(!page.subframeCount());

    m_cachedMainFrame->open();
    
    // Restore the focus appearance for the focused element.
    // FIXME: Right now we don't support pages w/ frames in the b/f cache.  This may need to be tweaked when we add support for that.
    Document* focusedDocument = page.focusController().focusedOrMainFrame().document();
    if (Element* element = focusedDocument->focusedElement()) {
#if PLATFORM(IOS)
        // We don't want focused nodes changing scroll position when restoring from the cache
        // as it can cause ugly jumps before we manage to restore the cached position.
        page.mainFrame().selection().suppressScrolling();

        bool hadProhibitsScrolling = false;
        FrameView* frameView = page.mainFrame().view();
        if (frameView) {
            hadProhibitsScrolling = frameView->prohibitsScrolling();
            frameView->setProhibitsScrolling(true);
        }
#endif
        element->updateFocusAppearance(SelectionRestorationMode::Restore);
#if PLATFORM(IOS)
        if (frameView)
            frameView->setProhibitsScrolling(hadProhibitsScrolling);
        page.mainFrame().selection().restoreScrolling();
#endif
    }

    if (m_needStyleRecalcForVisitedLinks) {
        for (Frame* frame = &page.mainFrame(); frame; frame = frame->tree().traverseNext())
            frame->document()->visitedLinkState().invalidateStyleForAllLinks();
    }

    if (m_needsDeviceOrPageScaleChanged)
        page.mainFrame().deviceOrPageScaleFactorChanged();

    if (m_needsFullStyleRecalc)
        page.setNeedsRecalcStyleInAllFrames();

#if ENABLE(VIDEO_TRACK)
    if (m_needsCaptionPreferencesChanged)
        page.captionPreferencesChanged();
#endif

    if (m_needsUpdateContentsSize) {
        if (FrameView* frameView = page.mainFrame().view())
            frameView->updateContentsSize();
    }

    clear();
}

void CachedPage::clear()
{
    ASSERT(m_cachedMainFrame);
    m_cachedMainFrame->clear();
    m_cachedMainFrame = nullptr;
    m_needStyleRecalcForVisitedLinks = false;
    m_needsFullStyleRecalc = false;
#if ENABLE(VIDEO_TRACK)
    m_needsCaptionPreferencesChanged = false;
#endif
    m_needsDeviceOrPageScaleChanged = false;
    m_needsUpdateContentsSize = false;
}

bool CachedPage::hasExpired() const
{
    return monotonicallyIncreasingTime() > m_expirationTime;
}

} // namespace WebCore
