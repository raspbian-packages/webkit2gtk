/*
 * Copyright (C) 2012 Igalia S.L.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2,1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "config.h"
#include "WebKitWebProcessExtension.h"

#include "APIDictionary.h"
#include "APIInjectedBundleBundleClient.h"
#include "APIString.h"
#include "WebKitUserMessagePrivate.h"
#include "WebKitWebPagePrivate.h"
#include "WebKitWebProcessExtensionPrivate.h"
#include "WebProcess.h"
#include "WebProcessProxyMessages.h"
#include <WebCore/GCController.h>
#include <glib/gi18n-lib.h>
#include <wtf/HashMap.h>
#include <wtf/glib/GRefPtr.h>
#include <wtf/glib/WTFGType.h>

using namespace WebKit;

/**
 * WebKitWebProcessExtension:
 *
 * Represents an extension of the web process.
 *
 * WebKitWebProcessExtension is a loadable module for the web process. It allows you to execute code in the
 * web process and being able to use the DOM API, to change any request or to inject custom
 * JavaScript code, for example.
 *
 * To create a WebKitWebProcessExtension you should write a module with an initialization function that could
 * be either webkit_web_process_extension_initialize() with prototype #WebKitWebProcessExtensionInitializeFunction or
 * webkit_web_process_extension_initialize_with_user_data() with prototype #WebKitWebProcessExtensionInitializeWithUserDataFunction.
 * This function has to be public and it has to use the #G_MODULE_EXPORT macro. It is called when the
 * web process is initialized.
 *
 * ```c
 * static void
 * web_page_created_callback (WebKitWebProcessExtension *extension,
 *                            WebKitWebPage             *web_page,
 *                            gpointer                   user_data)
 * {
 *     g_print ("Page %d created for %s\n",
 *              webkit_web_page_get_id (web_page),
 *              webkit_web_page_get_uri (web_page));
 * }
 *
 * G_MODULE_EXPORT void
 * webkit_web_process_extension_initialize (WebKitWebProcessExtension *extension)
 * {
 *     g_signal_connect (extension, "page-created",
 *                       G_CALLBACK (web_page_created_callback),
 *                       NULL);
 * }
 * ```
 *
 * The previous piece of code shows a trivial example of an extension that notifies when
 * a #WebKitWebPage is created.
 *
 * WebKit has to know where it can find the created WebKitWebProcessExtension. To do so you
 * should use the webkit_web_context_set_web_extensions_directory() function. The signal
 * #WebKitWebContext::initialize-web-extensions is the recommended place to call it.
 *
 * To provide the initialization data used by the webkit_web_process_extension_initialize_with_user_data()
 * function, you have to call webkit_web_context_set_web_extensions_initialization_user_data() with
 * the desired data as parameter. You can see an example of this in the following piece of code:
 *
 * ```c
 * #define WEB_EXTENSIONS_DIRECTORY // ...
 *
 * static void
 * initialize_web_extensions (WebKitWebContext *context,
 *                            gpointer          user_data)
 * {
 *   // Web Extensions get a different ID for each Web Process
 *   static guint32 unique_id = 0;
 *
 *   webkit_web_context_set_web_extensions_directory (
 *      context, WEB_EXTENSIONS_DIRECTORY);
 *   webkit_web_context_set_web_extensions_initialization_user_data (
 *      context, g_variant_new_uint32 (unique_id++));
 * }
 *
 * int main (int argc, char **argv)
 * {
 *   g_signal_connect (webkit_web_context_get_default (),
 *                    "initialize-web-extensions",
 *                     G_CALLBACK (initialize_web_extensions),
 *                     NULL);
 *
 *   GtkWidget *view = webkit_web_view_new ();
 *
 *   // ...
 * }
 * ```
 *
 * Since: 2.40
 */

enum {
    PAGE_CREATED,
    USER_MESSAGE_RECEIVED,

    LAST_SIGNAL
};

typedef HashMap<WebPage*, GRefPtr<WebKitWebPage> > WebPageMap;

struct _WebKitWebProcessExtensionPrivate {
    WebPageMap pages;
#if ENABLE(DEVELOPER_MODE)
    bool garbageCollectOnPageDestroy;
#endif
};

static guint signals[LAST_SIGNAL] = { 0, };

WEBKIT_DEFINE_FINAL_TYPE(WebKitWebProcessExtension, webkit_web_process_extension, G_TYPE_OBJECT, GObject)

static void webkit_web_process_extension_class_init(WebKitWebProcessExtensionClass* klass)
{
    /**
     * WebKitWebProcessExtension::page-created:
     * @extension: the #WebKitWebProcessExtension on which the signal is emitted
     * @web_page: the #WebKitWebPage created
     *
     * This signal is emitted when a new #WebKitWebPage is created in
     * the Web Process.
     *
     * Since: 2.40
     */
    signals[PAGE_CREATED] = g_signal_new(
        "page-created",
        G_TYPE_FROM_CLASS(klass),
        G_SIGNAL_RUN_LAST,
        0, 0, 0,
        g_cclosure_marshal_VOID__OBJECT,
        G_TYPE_NONE, 1,
        WEBKIT_TYPE_WEB_PAGE);

    /**
     * WebKitWebProcessExtension::user-message-received:
     * @extension: the #WebKitWebProcessExtension on which the signal is emitted
     * @message: the #WebKitUserMessage received
     *
     * This signal is emitted when a #WebKitUserMessage is received from the
     * #WebKitWebContext corresponding to @extension. Messages sent by #WebKitWebContext
     * are always broadcasted to all web extensions and they can't be
     * replied to. Calling webkit_user_message_send_reply() will do nothing.
     *
     * Since: 2.40
     */
    signals[USER_MESSAGE_RECEIVED] = g_signal_new(
        "user-message-received",
        G_TYPE_FROM_CLASS(klass),
        G_SIGNAL_RUN_LAST,
        0,
        nullptr, nullptr,
        g_cclosure_marshal_generic,
        G_TYPE_NONE, 1,
        WEBKIT_TYPE_USER_MESSAGE);
}

class WebProcessExtensionInjectedBundleClient final : public API::InjectedBundle::Client {
public:
    explicit WebProcessExtensionInjectedBundleClient(WebKitWebProcessExtension* extension)
        : m_extension(extension)
    {
    }

private:
    void didCreatePage(InjectedBundle&, WebPage& page) override
    {
        GRefPtr<WebKitWebPage> webPage = adoptGRef(webkitWebPageCreate(&page));
        m_extension->priv->pages.add(&page, webPage);
        g_signal_emit(m_extension, signals[PAGE_CREATED], 0, webPage.get());
    }

    void willDestroyPage(InjectedBundle&, WebPage& page) override
    {
        m_extension->priv->pages.remove(&page);
#if ENABLE(DEVELOPER_MODE)
        if (m_extension->priv->garbageCollectOnPageDestroy)
            WebCore::GCController::singleton().garbageCollectNow();
#endif
    }

    WebKitWebProcessExtension* m_extension;
};

WebKitWebProcessExtension* webkitWebProcessExtensionCreate(InjectedBundle* bundle)
{
    WebKitWebProcessExtension* extension = WEBKIT_WEB_PROCESS_EXTENSION(g_object_new(WEBKIT_TYPE_WEB_PROCESS_EXTENSION, NULL));
    bundle->setClient(makeUnique<WebProcessExtensionInjectedBundleClient>(extension));
    return extension;
}

void webkitWebProcessExtensionDidReceiveUserMessage(WebKitWebProcessExtension* extension, UserMessage&& message)
{
    // Sink the floating ref.
    GRefPtr<WebKitUserMessage> userMessage = webkitUserMessageCreate(WTFMove(message), [](UserMessage&&) { });
    g_signal_emit(extension, signals[USER_MESSAGE_RECEIVED], 0, userMessage.get());
}

void webkitWebProcessExtensionSetGarbageCollectOnPageDestroy(WebKitWebProcessExtension* extension)
{
#if ENABLE(DEVELOPER_MODE)
    extension->priv->garbageCollectOnPageDestroy = true;
#endif
}

/**
 * webkit_web_process_extension_get_page:
 * @extension: a #WebKitWebProcessExtension
 * @page_id: the identifier of the #WebKitWebPage to get
 *
 * Get the web page of the given @page_id.
 *
 * Returns: (transfer none): the #WebKitWebPage for the given @page_id, or %NULL if the
 *    identifier doesn't correspond to an existing web page.
 *
 * Since: 2.40
 */
WebKitWebPage* webkit_web_process_extension_get_page(WebKitWebProcessExtension* extension, guint64 pageID)
{
    g_return_val_if_fail(WEBKIT_IS_WEB_PROCESS_EXTENSION(extension), 0);

    WebKitWebProcessExtensionPrivate* priv = extension->priv;
    WebPageMap::const_iterator end = priv->pages.end();
    for (WebPageMap::const_iterator it = priv->pages.begin(); it != end; ++it) {
        if (it->key->identifier().toUInt64() == pageID)
            return it->value.get();
    }

    return 0;
}

/**
 * webkit_web_process_extension_send_message_to_context:
 * @extension: a #WebKitWebProcessExtension
 * @message: a #WebKitUserMessage
 * @cancellable: (nullable): a #GCancellable or %NULL to ignore
 * @callback: (scope async) (nullable): A #GAsyncReadyCallback to call when the request is satisfied or %NULL
 * @user_data: the data to pass to callback function
 *
 * Send @message to the #WebKitWebContext corresponding to @extension. If @message is floating, it's consumed.
 *
 * If you don't expect any reply, or you simply want to ignore it, you can pass %NULL as @calback.
 * When the operation is finished, @callback will be called. You can then call
 * webkit_web_process_extension_send_message_to_context_finish() to get the message reply.
 *
 * Since: 2.40
 */
void webkit_web_process_extension_send_message_to_context(WebKitWebProcessExtension* extension, WebKitUserMessage* message, GCancellable* cancellable, GAsyncReadyCallback callback, gpointer userData)
{
    g_return_if_fail(WEBKIT_IS_WEB_PROCESS_EXTENSION(extension));
    g_return_if_fail(WEBKIT_IS_USER_MESSAGE(message));

    // We sink the reference in case of being floating.
    GRefPtr<WebKitUserMessage> adoptedMessage = message;
    if (!callback) {
        WebProcess::singleton().parentProcessConnection()->send(Messages::WebProcessProxy::SendMessageToWebContext(webkitUserMessageGetMessage(message)), 0);
        return;
    }

    GRefPtr<GTask> task = adoptGRef(g_task_new(extension, cancellable, callback, userData));
    CompletionHandler<void(UserMessage&&)> completionHandler = [task = WTFMove(task)](UserMessage&& replyMessage) {
        switch (replyMessage.type) {
        case UserMessage::Type::Null:
            g_task_return_new_error(task.get(), G_IO_ERROR, G_IO_ERROR_CANCELLED, _("Operation was cancelled"));
            break;
        case UserMessage::Type::Message:
            g_task_return_pointer(task.get(), g_object_ref_sink(webkitUserMessageCreate(WTFMove(replyMessage))), static_cast<GDestroyNotify>(g_object_unref));
            break;
        case UserMessage::Type::Error:
            g_task_return_new_error(task.get(), WEBKIT_USER_MESSAGE_ERROR, replyMessage.errorCode, _("Message %s was not handled"), replyMessage.name.data());
            break;
        }
    };
    WebProcess::singleton().parentProcessConnection()->sendWithAsyncReply(Messages::WebProcessProxy::SendMessageToWebContextWithReply(webkitUserMessageGetMessage(message)), WTFMove(completionHandler));
}

/**
 * webkit_web_process_extension_send_message_to_context_finish:
 * @extension: a #WebKitWebProcessExtension
 * @result: a #GAsyncResult
 * @error: return location for error or %NULL to ignor
 *
 * Finish an asynchronous operation started with webkit_web_process_extension_send_message_to_context().
 *
 * Returns: (transfer full): a #WebKitUserMessage with the reply or %NULL in case of error.
 *
 * Since: 2.40
 */
WebKitUserMessage* webkit_web_process_extension_send_message_to_context_finish(WebKitWebProcessExtension* extension, GAsyncResult* result, GError** error)
{
    g_return_val_if_fail(WEBKIT_IS_WEB_PROCESS_EXTENSION(extension), nullptr);
    g_return_val_if_fail(g_task_is_valid(result, extension), nullptr);

    return WEBKIT_USER_MESSAGE(g_task_propagate_pointer(G_TASK(result), error));
}
