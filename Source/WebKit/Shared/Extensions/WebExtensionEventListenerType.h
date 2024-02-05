/*
 * Copyright (C) 2022 Apple Inc. All rights reserved.
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

#if ENABLE(WK_WEB_EXTENSIONS)

namespace WebKit {

// If you are adding a new event, you will also need to increase 'currentBackgroundPageListenerStateVersion'
// so that your new event gets fired to non-persistent background pages.
enum class WebExtensionEventListenerType : uint8_t {
    Unknown = 0,
    ActionOnClicked,
    AlarmsOnAlarm,
    CommandsOnCommand,
    ContextMenusOnClicked,
    CookiesOnChanged,
    DevToolsElementsPanelOnSelectionChanged,
    DevToolsExtensionPanelOnShown,
    DevToolsExtensionPanelOnHidden,
    DevToolsExtensionSidebarPaneOnShown,
    DevToolsExtensionSidebarPaneOnHidden,
    DevToolsExtensionPanelOnSearch,
    DevToolsNetworkOnNavigated,
    DevToolsNetworkOnRequestFinished,
    DevToolsPanelsOnThemeChanged,
    NotificationsOnButtonClicked,
    NotificationsOnClicked,
    PermissionsOnAdded,
    PermissionsOnRemoved,
    PortOnDisconnect,
    PortOnMessage,
    RuntimeOnConnect,
    RuntimeOnConnectExternal,
    RuntimeOnInstalled,
    RuntimeOnMessage,
    RuntimeOnMessageExternal,
    RuntimeOnStartup,
    StorageOnChanged,
    TabsOnActivated,
    TabsOnAttached,
    TabsOnCreated,
    TabsOnDetached,
    TabsOnHighlighted,
    TabsOnMoved,
    TabsOnRemoved,
    TabsOnReplaced,
    TabsOnUpdated,
    WebNavigationOnBeforeNavigate,
    WebNavigationOnCommitted,
    WebNavigationOnCompleted,
    WebNavigationOnDOMContentLoaded,
    WebNavigationOnErrorOccurred,
    WebRequestOnAuthRequired,
    WebRequestOnBeforeRedirect,
    WebRequestOnBeforeRequest,
    WebRequestOnBeforeSendHeaders,
    WebRequestOnCompleted,
    WebRequestOnErrorOccurred,
    WebRequestOnHeadersReceived,
    WebRequestOnResponseStarted,
    WebRequestOnSendHeaders,
    WindowsOnCreated,
    WindowsOnFocusChanged,
    WindowsOnRemoved,
    DevToolsInspectedWindowOnResourceAdded,
    DownloadsOnCreated,
    DownloadsOnChanged,
};

} // namespace WebKit

#endif // ENABLE(WK_WEB_EXTENSIONS)
