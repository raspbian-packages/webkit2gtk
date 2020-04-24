/*
 * Copyright (C) 2013, 2015 Apple Inc. All Rights Reserved.
 * Copyright (C) 2011 Google Inc. All rights reserved.
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

#pragma once

#include <JavaScriptCore/InspectorAgentBase.h>
#include <wtf/text/WTFString.h>

namespace WebCore {

class InstrumentingAgents;
class Page;
class WorkerGlobalScope;

// FIXME: move this to Inspector namespace when remaining agents move.
struct WebAgentContext : public Inspector::AgentContext {
    WebAgentContext(AgentContext& context, InstrumentingAgents& instrumentingAgents)
        : AgentContext(context)
        , instrumentingAgents(instrumentingAgents)
    {
    }

    InstrumentingAgents& instrumentingAgents;
};

struct PageAgentContext : public WebAgentContext {
    PageAgentContext(WebAgentContext& context, Page& inspectedPage)
        : WebAgentContext(context)
        , inspectedPage(inspectedPage)
    {
    }

    Page& inspectedPage;
};

struct WorkerAgentContext : public WebAgentContext {
    WorkerAgentContext(WebAgentContext& context, WorkerGlobalScope& workerGlobalScope)
        : WebAgentContext(context)
        , workerGlobalScope(workerGlobalScope)
    {
    }

    WorkerGlobalScope& workerGlobalScope;
};

class InspectorAgentBase : public Inspector::InspectorAgentBase {
protected:
    InspectorAgentBase(const String& name, WebAgentContext& context)
        : Inspector::InspectorAgentBase(name)
        , m_instrumentingAgents(context.instrumentingAgents)
        , m_environment(context.environment)
    {
    }

    InstrumentingAgents& m_instrumentingAgents;
    Inspector::InspectorEnvironment& m_environment;
};
    
} // namespace WebCore
