/*
 * Copyright (C) 2005, 2006, 2008 Apple Inc. All rights reserved.
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

#include "EditCommand.h"

namespace WebCore {

class InsertNodeBeforeCommand : public SimpleEditCommand {
public:
    static Ref<InsertNodeBeforeCommand> create(Ref<Node>&& childToInsert, Node& childToInsertBefore,
        ShouldAssumeContentIsAlwaysEditable shouldAssumeContentIsAlwaysEditable, EditAction editingAction = EditAction::Insert)
    {
        return adoptRef(*new InsertNodeBeforeCommand(WTFMove(childToInsert), childToInsertBefore, shouldAssumeContentIsAlwaysEditable, editingAction));
    }

protected:
    InsertNodeBeforeCommand(Ref<Node>&& childToInsert, Node& childToInsertBefore, ShouldAssumeContentIsAlwaysEditable, EditAction);

private:
    void doApply() override;
    void doUnapply() override;

#ifndef NDEBUG
    void getNodesInCommand(HashSet<Node*>&) override;
#endif

    Ref<Node> m_insertChild;
    Ref<Node> m_refChild;
    ShouldAssumeContentIsAlwaysEditable m_shouldAssumeContentIsAlwaysEditable;
};

} // namespace WebCore
