/*
 * Copyright (C) 2010 Apple Inc. All rights reserved.
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

#if ENABLE(CONTEXT_MENUS)

#include "WebContextMenuItem.h"

#include "APIArray.h"
#include <WebCore/ContextMenuItem.h>
#include <wtf/NeverDestroyed.h>

namespace WebKit {

WebContextMenuItem::WebContextMenuItem(const WebContextMenuItemData& data)
    : m_webContextMenuItemData(data)
{
}

Ref<WebContextMenuItem> WebContextMenuItem::create(const String& title, bool enabled, API::Array* submenuItems)
{
    size_t size = submenuItems->size();

    Vector<WebContextMenuItemData> submenu;
    submenu.reserveInitialCapacity(size);
    for (size_t i = 0; i < size; ++i) {
        if (auto* item = submenuItems->at<WebContextMenuItem>(i))
            submenu.append(item->data());
    }
    submenu.shrinkToFit();

    bool checked = false;
    unsigned indentationLevel = 0;
    return adoptRef(*new WebContextMenuItem(WebContextMenuItemData(WebCore::ContextMenuItemType::Submenu, WebCore::ContextMenuItemTagNoAction, String { title }, enabled, checked, indentationLevel, WTFMove(submenu)))).leakRef();
}

WebContextMenuItem* WebContextMenuItem::separatorItem()
{
    static NeverDestroyed<Ref<WebContextMenuItem>> separatorItem = adoptRef(*new WebContextMenuItem(WebContextMenuItemData(WebCore::ContextMenuItemType::Separator, WebCore::ContextMenuItemTagNoAction, String(), true, false)));
    return separatorItem->ptr();
}

Ref<API::Array> WebContextMenuItem::submenuItemsAsAPIArray() const
{
    if (m_webContextMenuItemData.type() != WebCore::ContextMenuItemType::Submenu)
        return API::Array::create();

    auto submenuItems = m_webContextMenuItemData.submenu().map([](auto& item) -> RefPtr<API::Object> {
        return WebContextMenuItem::create(item);
    });
    return API::Array::create(WTFMove(submenuItems));
}

API::Object* WebContextMenuItem::userData() const
{
    return m_webContextMenuItemData.userData();
}

void WebContextMenuItem::setUserData(API::Object* userData)
{
    m_webContextMenuItemData.setUserData(userData);
}

} // namespace WebKit

#endif // ENABLE(CONTEXT_MENUS)
