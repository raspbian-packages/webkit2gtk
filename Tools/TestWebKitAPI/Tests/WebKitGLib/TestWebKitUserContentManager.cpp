/*
 * Copyright (C) 2013-2014 Igalia S.L.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
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

#include "WebKitTestServer.h"
#include "WebViewTest.h"
#include <WebCore/SoupVersioning.h>
#include <cstdarg>
#include <wtf/glib/GRefPtr.h>
#include <wtf/glib/GUniquePtr.h>

static WebKitTestServer* kServer;

// These are all here so that they can be changed easily, if necessary.
static const char* kTestHTML =
"<html>\n"
"  <head><link rel=\"stylesheet\" type=\"text/css\" href=\"/extra.css\"></head>\n"
"  <body>\n"
"    <div id=\"styledElement\">Sweet stylez!</div>\n"
"    <div id=\"otherElement\">Blocked stylez!</div>\n"
"  </body>\n"
"</html>";

static const char* kTestCSS =
"div#otherElement {\n"
"  font-weight: bold;\n"
"}\n";

static const char* kInjectedStyleSheet = "#styledElement { font-weight: bold; }";
static const char* kStyleSheetTestScript = "getComputedStyle(document.getElementById('styledElement'))['font-weight']";
static const char* kStyleSheetTestScriptResult = "700";
static const char* kInjectedScript = "document.write('<div id=\"item\">Generated by a script</div>')";
static const char* kScriptTestScript = "document.getElementById('item').innerText";
static const char* kScriptTestScriptResult = "Generated by a script";
static const char* kScriptTestCSSBlocked = "getComputedStyle(document.getElementById('otherElement'))['font-weight']";
static const char* kScriptTestCSSBlockedResult = "400";
static const char* kJSONFilter =
"[\n"
"  {\n"
"    \"trigger\": {\n"
"      \"url-filter\": \".*\",\n"
"      \"resource-type\": [\"style-sheet\"]\n"
"    },\n"
"    \"action\": {\n"
"      \"type\": \"block\"\n"
"    }\n"
"  }\n"
"]\n";

static void testWebViewNewWithUserContentManager(Test* test, gconstpointer)
{
    GRefPtr<WebKitUserContentManager> userContentManager1 = adoptGRef(webkit_user_content_manager_new());
    test->assertObjectIsDeletedWhenTestFinishes(G_OBJECT(userContentManager1.get()));
    auto webView1 = Test::adoptView(Test::createWebView(userContentManager1.get()));
    g_assert_true(webkit_web_view_get_user_content_manager(webView1.get()) == userContentManager1.get());

    auto webView2 = Test::adoptView(Test::createWebView());
    g_assert_true(webkit_web_view_get_user_content_manager(webView2.get()) != userContentManager1.get());
}

static bool isStyleSheetInjectedForURLAtPath(WebViewTest* test, const char* path, const char* world = nullptr)
{
    test->loadURI(kServer->getURIForPath(path).data());
    test->waitUntilLoadFinished();

    GUniqueOutPtr<GError> error;
    JSCValue* value = world ? test->runJavaScriptInWorldAndWaitUntilFinished(kStyleSheetTestScript, world, nullptr, &error.outPtr())
        : test->runJavaScriptAndWaitUntilFinished(kStyleSheetTestScript, &error.outPtr());
    g_assert_nonnull(value);
    g_assert_no_error(error.get());

    GUniquePtr<char> resultString(WebViewTest::javascriptResultToCString(value));
    return !g_strcmp0(resultString.get(), kStyleSheetTestScriptResult);
}

static bool isScriptInjectedForURLAtPath(WebViewTest* test, const char* path, const char* world = nullptr)
{
    test->loadURI(kServer->getURIForPath(path).data());
    test->waitUntilLoadFinished();

    GUniqueOutPtr<GError> error;
    JSCValue* value = world ? test->runJavaScriptInWorldAndWaitUntilFinished(kScriptTestScript, world, nullptr, &error.outPtr())
        : test->runJavaScriptAndWaitUntilFinished(kScriptTestScript, &error.outPtr());
    if (value) {
        g_assert_no_error(error.get());

        GUniquePtr<char> resultString(WebViewTest::javascriptResultToCString(value));
        return !g_strcmp0(resultString.get(), kScriptTestScriptResult);
    }
    return false;
}

static void fillURLListFromPaths(char** list, const char* path, ...)
{
    va_list argumentList;
    va_start(argumentList, path);

    int i = 0;
    while (path) {
        // FIXME: We must use a wildcard for the host here until http://wkbug.com/112476 is fixed.
        // Until that time patterns with port numbers in them will not properly match URLs with port numbers.
        list[i++] = g_strdup_printf("http://*/%s*", path);
        path = va_arg(argumentList, const char*);
    }
}

static void removeOldInjectedContentAndResetLists(WebKitUserContentManager* userContentManager, char** allowList, char** blockList)
{
    webkit_user_content_manager_remove_all_style_sheets(userContentManager);
    webkit_user_content_manager_remove_all_scripts(userContentManager);
    webkit_user_content_manager_remove_all_filters(userContentManager);

    while (*allowList) {
        g_free(*allowList);
        *allowList = 0;
        allowList++;
    }

    while (*blockList) {
        g_free(*blockList);
        *blockList = 0;
        blockList++;
    }
}

static void testUserContentManagerInjectedStyleSheet(WebViewTest* test, gconstpointer)
{
    char* allowList[3] = { 0, 0, 0 };
    char* blockList[3] = { 0, 0, 0 };

    removeOldInjectedContentAndResetLists(test->m_userContentManager.get(), allowList, blockList);

    // Without a allowList or a blockList all URLs should have the injected style sheet.
    static const char* randomPath = "somerandompath";
    g_assert_false(isStyleSheetInjectedForURLAtPath(test, randomPath));
    WebKitUserStyleSheet* styleSheet = webkit_user_style_sheet_new(kInjectedStyleSheet, WEBKIT_USER_CONTENT_INJECT_ALL_FRAMES, WEBKIT_USER_STYLE_LEVEL_USER, nullptr, nullptr);
    webkit_user_content_manager_add_style_sheet(test->m_userContentManager.get(), styleSheet);
    webkit_user_style_sheet_unref(styleSheet);
    g_assert_true(isStyleSheetInjectedForURLAtPath(test, randomPath));

    removeOldInjectedContentAndResetLists(test->m_userContentManager.get(), allowList, blockList);

    g_assert_false(isStyleSheetInjectedForURLAtPath(test, randomPath, "WebExtensionTestScriptWorld"));
    styleSheet = webkit_user_style_sheet_new_for_world(kInjectedStyleSheet, WEBKIT_USER_CONTENT_INJECT_ALL_FRAMES, WEBKIT_USER_STYLE_LEVEL_USER, "WebExtensionTestScriptWorld", nullptr, nullptr);
    webkit_user_content_manager_add_style_sheet(test->m_userContentManager.get(), styleSheet);
    webkit_user_style_sheet_unref(styleSheet);
    g_assert_true(isStyleSheetInjectedForURLAtPath(test, randomPath, "WebExtensionTestScriptWorld"));

    removeOldInjectedContentAndResetLists(test->m_userContentManager.get(), allowList, blockList);

    fillURLListFromPaths(blockList, randomPath, 0);
    styleSheet = webkit_user_style_sheet_new(kInjectedStyleSheet, WEBKIT_USER_CONTENT_INJECT_ALL_FRAMES, WEBKIT_USER_STYLE_LEVEL_USER, nullptr, blockList);
    webkit_user_content_manager_add_style_sheet(test->m_userContentManager.get(), styleSheet);
    webkit_user_style_sheet_unref(styleSheet);
    g_assert_false(isStyleSheetInjectedForURLAtPath(test, randomPath));
    g_assert_true(isStyleSheetInjectedForURLAtPath(test, "someotherrandompath"));

    removeOldInjectedContentAndResetLists(test->m_userContentManager.get(), allowList, blockList);

    static const char* inTheAllowList = "intheallowlist";
    static const char* notInAllowList = "notintheallowlist";
    static const char* inTheAllowListAndBlockList = "intheallowlistandblocklist";

    fillURLListFromPaths(allowList, inTheAllowList, inTheAllowListAndBlockList, 0);
    fillURLListFromPaths(blockList, inTheAllowListAndBlockList, 0);
    styleSheet = webkit_user_style_sheet_new(kInjectedStyleSheet, WEBKIT_USER_CONTENT_INJECT_ALL_FRAMES, WEBKIT_USER_STYLE_LEVEL_USER, allowList, blockList);
    webkit_user_content_manager_add_style_sheet(test->m_userContentManager.get(), styleSheet);
    webkit_user_style_sheet_unref(styleSheet);
    g_assert_true(isStyleSheetInjectedForURLAtPath(test, inTheAllowList));
    g_assert_false(isStyleSheetInjectedForURLAtPath(test, inTheAllowListAndBlockList));
    g_assert_false(isStyleSheetInjectedForURLAtPath(test, notInAllowList));

    removeOldInjectedContentAndResetLists(test->m_userContentManager.get(), allowList, blockList);

    g_assert_false(isStyleSheetInjectedForURLAtPath(test, randomPath));
    styleSheet = webkit_user_style_sheet_new(kInjectedStyleSheet, WEBKIT_USER_CONTENT_INJECT_ALL_FRAMES, WEBKIT_USER_STYLE_LEVEL_USER, nullptr, nullptr);
    webkit_user_content_manager_add_style_sheet(test->m_userContentManager.get(), styleSheet);
    g_assert_true(isStyleSheetInjectedForURLAtPath(test, randomPath));
    webkit_user_content_manager_remove_style_sheet(test->m_userContentManager.get(), styleSheet);
    g_assert_false(isStyleSheetInjectedForURLAtPath(test, randomPath));
    webkit_user_style_sheet_unref(styleSheet);

    // It's important to clean up the environment before other tests.
    removeOldInjectedContentAndResetLists(test->m_userContentManager.get(), allowList, blockList);
}

static void testUserContentManagerInjectedScript(WebViewTest* test, gconstpointer)
{
    char* allowList[3] = { 0, 0, 0 };
    char* blockList[3] = { 0, 0, 0 };

    removeOldInjectedContentAndResetLists(test->m_userContentManager.get(), allowList, blockList);

    // Without a allowList or a blockList all URLs should have the injected script.
    static const char* randomPath = "somerandompath";
    g_assert_false(isScriptInjectedForURLAtPath(test, randomPath));
    WebKitUserScript* script = webkit_user_script_new(kInjectedScript, WEBKIT_USER_CONTENT_INJECT_ALL_FRAMES, WEBKIT_USER_SCRIPT_INJECT_AT_DOCUMENT_END, nullptr, nullptr);
    webkit_user_content_manager_add_script(test->m_userContentManager.get(), script);
    webkit_user_script_unref(script);
    g_assert_true(isScriptInjectedForURLAtPath(test, randomPath));

    removeOldInjectedContentAndResetLists(test->m_userContentManager.get(), allowList, blockList);

    g_assert_false(isScriptInjectedForURLAtPath(test, randomPath, "WebExtensionTestScriptWorld"));
    script = webkit_user_script_new_for_world(kInjectedScript, WEBKIT_USER_CONTENT_INJECT_ALL_FRAMES, WEBKIT_USER_SCRIPT_INJECT_AT_DOCUMENT_END, "WebExtensionTestScriptWorld", nullptr, nullptr);
    webkit_user_content_manager_add_script(test->m_userContentManager.get(), script);
    webkit_user_script_unref(script);
    g_assert_true(isScriptInjectedForURLAtPath(test, randomPath, "WebExtensionTestScriptWorld"));

    removeOldInjectedContentAndResetLists(test->m_userContentManager.get(), allowList, blockList);

    fillURLListFromPaths(blockList, randomPath, 0);
    script = webkit_user_script_new(kInjectedScript, WEBKIT_USER_CONTENT_INJECT_ALL_FRAMES, WEBKIT_USER_SCRIPT_INJECT_AT_DOCUMENT_END, nullptr, blockList);
    webkit_user_content_manager_add_script(test->m_userContentManager.get(), script);
    webkit_user_script_unref(script);
    g_assert_false(isScriptInjectedForURLAtPath(test, randomPath));
    g_assert_true(isScriptInjectedForURLAtPath(test, "someotherrandompath"));

    removeOldInjectedContentAndResetLists(test->m_userContentManager.get(), allowList, blockList);

    static const char* inTheAllowList = "intheallowlist";
    static const char* notInAllowList = "notintheallowlist";
    static const char* inTheAllowListAndBlockList = "intheallowlistandblockList";

    fillURLListFromPaths(allowList, inTheAllowList, inTheAllowListAndBlockList, 0);
    fillURLListFromPaths(blockList, inTheAllowListAndBlockList, 0);
    script = webkit_user_script_new(kInjectedScript, WEBKIT_USER_CONTENT_INJECT_ALL_FRAMES, WEBKIT_USER_SCRIPT_INJECT_AT_DOCUMENT_END, allowList, blockList);
    webkit_user_content_manager_add_script(test->m_userContentManager.get(), script);
    webkit_user_script_unref(script);
    g_assert_true(isScriptInjectedForURLAtPath(test, inTheAllowList));
    g_assert_false(isScriptInjectedForURLAtPath(test, inTheAllowListAndBlockList));
    g_assert_false(isScriptInjectedForURLAtPath(test, notInAllowList));

    removeOldInjectedContentAndResetLists(test->m_userContentManager.get(), allowList, blockList);

    g_assert_false(isScriptInjectedForURLAtPath(test, randomPath));
    script = webkit_user_script_new(kInjectedScript, WEBKIT_USER_CONTENT_INJECT_ALL_FRAMES, WEBKIT_USER_SCRIPT_INJECT_AT_DOCUMENT_END, nullptr, nullptr);
    webkit_user_content_manager_add_script(test->m_userContentManager.get(), script);
    g_assert_true(isScriptInjectedForURLAtPath(test, randomPath));
    webkit_user_content_manager_remove_script(test->m_userContentManager.get(), script);
    g_assert_false(isScriptInjectedForURLAtPath(test, randomPath));
    webkit_user_script_unref(script);

    // It's important to clean up the environment before other tests.
    removeOldInjectedContentAndResetLists(test->m_userContentManager.get(), allowList, blockList);
}

class UserScriptMessageTest : public WebViewTest {
public:
    MAKE_GLIB_TEST_FIXTURE(UserScriptMessageTest);

    bool registerHandler(const char* handlerName, const char* worldName = nullptr, bool async = false)
    {
        if (async)
            return webkit_user_content_manager_register_script_message_handler_with_reply(m_userContentManager.get(), handlerName, worldName);
#if ENABLE(2022_GLIB_API)
        return webkit_user_content_manager_register_script_message_handler(m_userContentManager.get(), handlerName, worldName);
#else
        return worldName ? webkit_user_content_manager_register_script_message_handler_in_world(m_userContentManager.get(), handlerName, worldName)
            : webkit_user_content_manager_register_script_message_handler(m_userContentManager.get(), handlerName);
#endif
    }

    void unregisterHandler(const char* handlerName, const char* worldName = nullptr)
    {
#if !ENABLE(2022_GLIB_API)
        return worldName ? webkit_user_content_manager_unregister_script_message_handler_in_world(m_userContentManager.get(), handlerName, worldName)
            : webkit_user_content_manager_unregister_script_message_handler(m_userContentManager.get(), handlerName);
#else
        return webkit_user_content_manager_unregister_script_message_handler(m_userContentManager.get(), handlerName, worldName);
#endif
    }

#if ENABLE(2022_GLIB_API)
    static void scriptMessageReceived(WebKitUserContentManager* userContentManager, JSCValue* result, UserScriptMessageTest* test)
#else
    static void scriptMessageReceived(WebKitUserContentManager* userContentManager, WebKitJavascriptResult* result, UserScriptMessageTest* test)
#endif
    {
        g_signal_handlers_disconnect_by_func(userContentManager, reinterpret_cast<gpointer>(scriptMessageReceived), test);
        if (!test->m_waitForScriptRun)
            g_main_loop_quit(test->m_mainLoop);

        g_assert_null(test->m_scriptMessage.get());
#if ENABLE(2022_GLIB_API)
        test->m_scriptMessage = result;
#else
        test->m_scriptMessage = webkit_javascript_result_get_js_value(result);
#endif
    }

    JSCValue* waitUntilMessageReceived(const char* handlerName)
    {
        m_scriptMessage = nullptr;

        GUniquePtr<char> signalName(g_strdup_printf("script-message-received::%s", handlerName));
        g_signal_connect(m_userContentManager.get(), signalName.get(), G_CALLBACK(scriptMessageReceived), this);

        g_main_loop_run(m_mainLoop);
        g_assert_false(m_waitForScriptRun);
        g_assert_nonnull(m_scriptMessage.get());
        return m_scriptMessage.get();
    }

    static void runJavaScriptFinished(GObject*, GAsyncResult* result, UserScriptMessageTest* test)
    {
        g_assert_true(test->m_waitForScriptRun);
        test->m_waitForScriptRun = false;
        g_main_loop_quit(test->m_mainLoop);
    }

    JSCValue* postMessageAndWaitUntilReceived(const char* handlerName, const char* javascriptValueAsText, const char* worldName = nullptr)
    {
        GUniquePtr<char> javascriptSnippet(g_strdup_printf("window.webkit.messageHandlers.%s.postMessage(%s);", handlerName, javascriptValueAsText));
        m_waitForScriptRun = true;
        webkit_web_view_evaluate_javascript(m_webView, javascriptSnippet.get(), -1, worldName, nullptr, nullptr, reinterpret_cast<GAsyncReadyCallback>(runJavaScriptFinished), this);
        return waitUntilMessageReceived(handlerName);
    }

    static gboolean asyncScriptMessageReceived(WebKitUserContentManager* userContentManager, JSCValue* jsValue, WebKitScriptMessageReply* scriptMessageReply, UserScriptMessageTest* test)
    {
        g_signal_handlers_disconnect_by_func(userContentManager, reinterpret_cast<gpointer>(asyncScriptMessageReceived), test);
        if (!test->m_waitForScriptRun)
            g_main_loop_quit(test->m_mainLoop);

        g_assert_true(JSC_IS_VALUE(jsValue));
        g_assert_true(jsc_value_is_string(jsValue));
        GUniquePtr<char> message(jsc_value_to_string(jsValue));
        if (!g_strcmp0(message.get(), "DoNothing"))
            return FALSE;

        if (!g_strcmp0(message.get(), "Ping")) {
            GRefPtr<JSCValue> reply = adoptGRef(jsc_value_new_string(jsc_value_get_context(jsValue), "Pong"));
            webkit_script_message_reply_return_value(scriptMessageReply, reply.get());
        } else if (!g_strcmp0(message.get(), "Fail"))
            webkit_script_message_reply_return_error_message(scriptMessageReply, "Failed");
        else
            webkit_script_message_reply_return_value(scriptMessageReply, jsValue);

        return TRUE;
    }

    JSCValue* waitUntilPromiseResolved(const char* handlerName, GUniquePtr<GError>& error)
    {
        GUniquePtr<char> signalName(g_strdup_printf("script-message-with-reply-received::%s", handlerName));
        g_signal_connect(m_userContentManager.get(), signalName.get(), G_CALLBACK(asyncScriptMessageReceived), this);

        g_main_loop_run(m_mainLoop);
        g_assert_false(m_waitForScriptRun);

        error.reset(m_scriptError.release());
        return m_scriptMessage.get();
    }

    static void runAsyncJavaScriptFinished(GObject* webView, GAsyncResult* result, UserScriptMessageTest* test)
    {
        g_assert_true(test->m_waitForScriptRun);
        test->m_waitForScriptRun = false;
        test->m_scriptMessage = webkit_web_view_call_async_javascript_function_finish(WEBKIT_WEB_VIEW(webView), result, &test->m_scriptError.outPtr());
        g_main_loop_quit(test->m_mainLoop);
    }

    JSCValue* postMessageAndWaitForPromiseResolved(const char* handlerName, const char* javascriptValueAsText, const char* worldName, GUniquePtr<GError>& error)
    {
        GUniquePtr<char> javascriptSnippet(g_strdup_printf("var p = window.webkit.messageHandlers.%s.postMessage(%s); await p; return p;", handlerName, javascriptValueAsText));
        m_waitForScriptRun = true;

        webkit_web_view_call_async_javascript_function(m_webView, javascriptSnippet.get(), -1, nullptr, worldName, nullptr, nullptr, reinterpret_cast<GAsyncReadyCallback>(runAsyncJavaScriptFinished), this);

        return waitUntilPromiseResolved(handlerName, error);
    }

private:
    GRefPtr<JSCValue> m_scriptMessage;
    GUniqueOutPtr<GError> m_scriptError;
    bool m_waitForScriptRun { false };
};

static void testUserContentManagerScriptMessageReceived(UserScriptMessageTest* test, gconstpointer)
{
    g_assert_true(test->registerHandler("msg"));

    // Trying to register the same handler a second time must fail.
    g_assert_false(test->registerHandler("msg"));

    test->loadHtml("<html></html>", nullptr);
    test->waitUntilLoadFinished();

    // Check that the "window.webkit.messageHandlers" namespace exists.
    GUniqueOutPtr<GError> error;
    JSCValue* value = test->runJavaScriptAndWaitUntilFinished("window.webkit.messageHandlers ? 'y' : 'n';", &error.outPtr());
    g_assert_nonnull(value);
    g_assert_no_error(error.get());
    GUniquePtr<char> valueString(WebViewTest::javascriptResultToCString(value));
    g_assert_cmpstr(valueString.get(), ==, "y");

    // Check that the "document.webkit.messageHandlers.msg" namespace exists.
    value = test->runJavaScriptAndWaitUntilFinished("window.webkit.messageHandlers.msg ? 'y' : 'n';", &error.outPtr());
    g_assert_nonnull(value);
    g_assert_no_error(error.get());
    valueString.reset(WebViewTest::javascriptResultToCString(value));
    g_assert_cmpstr(valueString.get(), ==, "y");

    valueString.reset(WebViewTest::javascriptResultToCString(test->postMessageAndWaitUntilReceived("msg", "'user message'")));
    g_assert_cmpstr(valueString.get(), ==, "user message");

    // Messages should arrive despite of other handlers being registered.
    g_assert_true(test->registerHandler("anotherHandler"));

    // Check that the "document.webkit.messageHandlers.msg" namespace still exists.
    value = test->runJavaScriptAndWaitUntilFinished("window.webkit.messageHandlers.msg ? 'y' : 'n';", &error.outPtr());
    g_assert_nonnull(value);
    g_assert_no_error(error.get());
    valueString.reset(WebViewTest::javascriptResultToCString(value));
    g_assert_cmpstr(valueString.get(), ==, "y");

    // Check that the "document.webkit.messageHandlers.anotherHandler" namespace exists.
    value = test->runJavaScriptAndWaitUntilFinished("window.webkit.messageHandlers.anotherHandler ? 'y' : 'n';", &error.outPtr());
    g_assert_nonnull(value);
    g_assert_no_error(error.get());
    valueString.reset(WebViewTest::javascriptResultToCString(value));
    g_assert_cmpstr(valueString.get(), ==, "y");

    valueString.reset(WebViewTest::javascriptResultToCString(test->postMessageAndWaitUntilReceived("msg", "'handler: msg'")));
    g_assert_cmpstr(valueString.get(), ==, "handler: msg");

    valueString.reset(WebViewTest::javascriptResultToCString(test->postMessageAndWaitUntilReceived("anotherHandler", "'handler: anotherHandler'")));
    g_assert_cmpstr(valueString.get(), ==, "handler: anotherHandler");

    // Unregistering a handler and re-registering again under the same name should work.
    test->unregisterHandler("msg");

    value = test->runJavaScriptAndWaitUntilFinished("window.webkit.messageHandlers.msg.postMessage('42');", &error.outPtr());
    g_assert_null(value);
    g_assert_nonnull(error.get());

    // Re-registering a handler that has been unregistered must work
    g_assert_true(test->registerHandler("msg"));
    valueString.reset(WebViewTest::javascriptResultToCString(test->postMessageAndWaitUntilReceived("msg", "'handler: msg'")));
    g_assert_cmpstr(valueString.get(), ==, "handler: msg");

    test->unregisterHandler("anotherHandler");
}

static void testUserContentManagerScriptMessageInWorldReceived(UserScriptMessageTest* test, gconstpointer)
{
    g_assert_true(test->registerHandler("msg"));

    test->loadHtml("<html></html>", nullptr);
    test->waitUntilLoadFinished();

    // Check that the "window.webkit.messageHandlers" namespace doesn't exist in isolated worlds.
    GUniqueOutPtr<GError> error;
    JSCValue* value = test->runJavaScriptInWorldAndWaitUntilFinished("window.webkit.messageHandlers ? 'y' : 'n';", "WebExtensionTestScriptWorld", nullptr, &error.outPtr());
    g_assert_null(value);
    g_assert_error(error.get(), WEBKIT_JAVASCRIPT_ERROR, WEBKIT_JAVASCRIPT_ERROR_SCRIPT_FAILED);
    test->unregisterHandler("msg");

    g_assert_true(test->registerHandler("msg", "WebExtensionTestScriptWorld"));

    // Check that the "window.webkit.messageHandlers" namespace exists in the world.
    value = test->runJavaScriptInWorldAndWaitUntilFinished("window.webkit.messageHandlers ? 'y' : 'n';", "WebExtensionTestScriptWorld", nullptr, &error.outPtr());
    g_assert_nonnull(value);
    g_assert_no_error(error.get());
    GUniquePtr<char> valueString(WebViewTest::javascriptResultToCString(value));
    g_assert_cmpstr(valueString.get(), ==, "y");

    valueString.reset(WebViewTest::javascriptResultToCString(test->postMessageAndWaitUntilReceived("msg", "'user message'", "WebExtensionTestScriptWorld")));
    g_assert_cmpstr(valueString.get(), ==, "user message");

    // Post message in main world should fail.
    value = test->runJavaScriptAndWaitUntilFinished("window.webkit.messageHandlers.msg.postMessage('42');", &error.outPtr());
    g_assert_null(value);
    g_assert_error(error.get(), WEBKIT_JAVASCRIPT_ERROR, WEBKIT_JAVASCRIPT_ERROR_SCRIPT_FAILED);

    test->unregisterHandler("msg", "WebExtensionTestScriptWorld");
}

static void testUserContentManagerScriptMessageWithReplyReceived(UserScriptMessageTest* test, gconstpointer)
{
    g_assert_true(test->registerHandler("msg", nullptr, true));

    // Trying to register the same handler a second time must fail.
    g_assert_false(test->registerHandler("msg"));

    test->loadHtml("<html></html>", nullptr);
    test->waitUntilLoadFinished();

    // Check that the "window.webkit.messageHandlers" namespace exists.
    GUniqueOutPtr<GError> error;
    JSCValue* value = test->runJavaScriptAndWaitUntilFinished("window.webkit.messageHandlers ? 'y' : 'n';", &error.outPtr());
    g_assert_nonnull(value);
    g_assert_no_error(error.get());
    GUniquePtr<char> valueString(WebViewTest::javascriptResultToCString(value));
    g_assert_cmpstr(valueString.get(), ==, "y");

    // Check that the "document.webkit.messageHandlers.msg" namespace exists.
    value = test->runJavaScriptAndWaitUntilFinished("window.webkit.messageHandlers.msg ? 'y' : 'n';", &error.outPtr());
    g_assert_nonnull(value);
    g_assert_no_error(error.get());
    valueString.reset(WebViewTest::javascriptResultToCString(value));
    g_assert_cmpstr(valueString.get(), ==, "y");

    GUniquePtr<GError> scriptError;
    value = test->postMessageAndWaitForPromiseResolved("msg", "'Ping'", nullptr, scriptError);
    g_assert_nonnull(value);
    g_assert_no_error(scriptError.get());
    valueString.reset(WebViewTest::javascriptResultToCString(value));
    g_assert_cmpstr(valueString.get(), ==, "Pong");

    value = test->postMessageAndWaitForPromiseResolved("msg", "'Fail'", nullptr, scriptError);
    g_assert_null(value);
    g_assert_error(scriptError.get(), WEBKIT_JAVASCRIPT_ERROR, WEBKIT_JAVASCRIPT_ERROR_SCRIPT_FAILED);
    g_assert_cmpstr(scriptError->message, ==, "Error: Failed");

    value = test->postMessageAndWaitForPromiseResolved("msg", "'DoNothing'", nullptr, scriptError);
    g_assert_nonnull(value);
    g_assert_no_error(scriptError.get());
    g_assert_true(WebViewTest::javascriptResultIsUndefined(value));

    // Check that the "window.webkit.messageHandlers" namespace doesn't exist in isolated worlds.
    value = test->runJavaScriptInWorldAndWaitUntilFinished("window.webkit.messageHandlers ? 'y' : 'n';", "WebExtensionTestScriptWorld", nullptr, &error.outPtr());
    g_assert_null(value);
    g_assert_error(error.get(), WEBKIT_JAVASCRIPT_ERROR, WEBKIT_JAVASCRIPT_ERROR_SCRIPT_FAILED);
    test->unregisterHandler("msg", nullptr);

    g_assert_true(test->registerHandler("msg", "WebExtensionTestScriptWorld", true));

    // Check that the "window.webkit.messageHandlers" namespace exists in the world.
    value = test->runJavaScriptInWorldAndWaitUntilFinished("window.webkit.messageHandlers ? 'y' : 'n';", "WebExtensionTestScriptWorld", nullptr, &error.outPtr());
    g_assert_nonnull(value);
    g_assert_no_error(error.get());
    valueString.reset(WebViewTest::javascriptResultToCString(value));
    g_assert_cmpstr(valueString.get(), ==, "y");

    value = test->postMessageAndWaitForPromiseResolved("msg", "'Ping'", "WebExtensionTestScriptWorld", scriptError);
    g_assert_nonnull(value);
    g_assert_no_error(scriptError.get());
    valueString.reset(WebViewTest::javascriptResultToCString(value));
    g_assert_cmpstr(valueString.get(), ==, "Pong");

    value = test->postMessageAndWaitForPromiseResolved("msg", "'Fail'", "WebExtensionTestScriptWorld", scriptError);
    g_assert_null(value);
    g_assert_error(scriptError.get(), WEBKIT_JAVASCRIPT_ERROR, WEBKIT_JAVASCRIPT_ERROR_SCRIPT_FAILED);
    g_assert_cmpstr(scriptError->message, ==, "Error: Failed");

    value = test->postMessageAndWaitForPromiseResolved("msg", "'DoNothing'", "WebExtensionTestScriptWorld", scriptError);
    g_assert_nonnull(value);
    g_assert_no_error(scriptError.get());
    g_assert_true(WebViewTest::javascriptResultIsUndefined(value));

    // Post message in main world should fail.
    value = test->runJavaScriptAndWaitUntilFinished("window.webkit.messageHandlers.msg.postMessage('42');", &error.outPtr());
    g_assert_null(value);
    g_assert_error(error.get(), WEBKIT_JAVASCRIPT_ERROR, WEBKIT_JAVASCRIPT_ERROR_SCRIPT_FAILED);

    test->unregisterHandler("msg", "WebExtensionTestScriptWorld");
}

#if PLATFORM(GTK) && !USE(GTK4)
static void testUserContentManagerScriptMessageFromDOMBindings(UserScriptMessageTest* test, gconstpointer)
{
    g_assert_true(test->registerHandler("dom"));

    test->loadHtml("<html>1</html>", nullptr);
    JSCValue* value = test->waitUntilMessageReceived("dom");
    g_assert_nonnull(value);
    GUniquePtr<char> valueString(WebViewTest::javascriptResultToCString(value));
    g_assert_cmpstr(valueString.get(), ==, "DocumentLoaded");

    test->unregisterHandler("dom");
}
#endif

static bool isCSSBlockedForURLAtPath(WebViewTest* test, const char* path)
{
    test->loadURI(kServer->getURIForPath(path).data());
    test->waitUntilLoadFinished();

    GUniqueOutPtr<GError> error;
    JSCValue* value = test->runJavaScriptAndWaitUntilFinished(kScriptTestCSSBlocked, &error.outPtr());
    g_assert_nonnull(value);
    g_assert_no_error(error.get());

    GUniquePtr<char> valueString(WebViewTest::javascriptResultToCString(value));
    return !g_strcmp0(valueString.get(), kScriptTestCSSBlockedResult);
}

static WebKitUserContentFilter* getUserContentFilter(WebViewTest* test)
{
    GUniquePtr<char> filtersPath(g_build_filename(test->dataDirectory(), "filters", nullptr));
    WebKitUserContentFilterStore* store = webkit_user_content_filter_store_new(filtersPath.get());
    test->assertObjectIsDeletedWhenTestFinishes(G_OBJECT(store));

    struct Data {
        GMainLoop* mainLoop;
        WebKitUserContentFilter* filter;
    };
    Data data { test->m_mainLoop, nullptr, };

    GRefPtr<GBytes> source = adoptGRef(g_bytes_new_static(kJSONFilter, strlen(kJSONFilter)));
    webkit_user_content_filter_store_save(store, "TestFilter", source.get(), nullptr, [](GObject* sourceObject, GAsyncResult* result, void* userData) {
        auto* data = static_cast<Data*>(userData);
        GUniqueOutPtr<GError> error;
        data->filter = webkit_user_content_filter_store_save_finish(WEBKIT_USER_CONTENT_FILTER_STORE(sourceObject), result, &error.outPtr());
        g_assert_nonnull(data->filter);
        g_assert_no_error(error.get());
        g_main_loop_quit(data->mainLoop);
    }, &data);
    g_main_loop_run(data.mainLoop);

    g_object_unref(store);

    g_assert_nonnull(data.filter);
    return data.filter;
}

static void testUserContentManagerContentFilter(WebViewTest* test, gconstpointer)
{
    char* allowList[] = { nullptr, nullptr, nullptr };
    char* blockList[] = { nullptr, nullptr, nullptr };

    removeOldInjectedContentAndResetLists(test->m_userContentManager.get(), allowList, blockList);

    static const char* somePath = "somepath";
    g_assert_false(isCSSBlockedForURLAtPath(test, somePath));

    WebKitUserContentFilter* filter = getUserContentFilter(test);
    webkit_user_content_manager_add_filter(test->m_userContentManager.get(), filter);
    g_assert_true(isCSSBlockedForURLAtPath(test, somePath));

    webkit_user_content_manager_remove_filter(test->m_userContentManager.get(), filter);
    g_assert_false(isCSSBlockedForURLAtPath(test, somePath));

    webkit_user_content_manager_add_filter(test->m_userContentManager.get(), filter);
    g_assert_true(isCSSBlockedForURLAtPath(test, somePath));

    webkit_user_content_manager_remove_filter_by_id(test->m_userContentManager.get(), webkit_user_content_filter_get_identifier(filter));
    g_assert_false(isCSSBlockedForURLAtPath(test, somePath));

    webkit_user_content_filter_unref(filter);
}

#if USE(SOUP2)
static void serverCallback(SoupServer* server, SoupMessage* message, const char* path, GHashTable*, SoupClientContext*, gpointer)
#else
static void serverCallback(SoupServer* server, SoupServerMessage* message, const char* path, GHashTable*, gpointer)
#endif
{
    soup_server_message_set_status(message, SOUP_STATUS_OK, nullptr);
    auto* responseBody = soup_server_message_get_response_body(message);
    if (!g_strcmp0(path, "/extra.css"))
        soup_message_body_append(responseBody, SOUP_MEMORY_STATIC, kTestCSS, strlen(kTestCSS));
    else
        soup_message_body_append(responseBody, SOUP_MEMORY_STATIC, kTestHTML, strlen(kTestHTML));
    soup_message_body_complete(responseBody);
}

void beforeAll()
{
    kServer = new WebKitTestServer();
    kServer->run(serverCallback);

    Test::shouldInitializeWebProcessExtensions = true;

    Test::add("WebKitWebView", "new-with-user-content-manager", testWebViewNewWithUserContentManager);
    WebViewTest::add("WebKitUserContentManager", "injected-style-sheet", testUserContentManagerInjectedStyleSheet);
    WebViewTest::add("WebKitUserContentManager", "injected-script", testUserContentManagerInjectedScript);
    UserScriptMessageTest::add("WebKitUserContentManager", "script-message-received", testUserContentManagerScriptMessageReceived);
    UserScriptMessageTest::add("WebKitUserContentManager", "script-message-in-world-received", testUserContentManagerScriptMessageInWorldReceived);
    UserScriptMessageTest::add("WebKitUserContentManager", "script-message-with-reply-received", testUserContentManagerScriptMessageWithReplyReceived);
#if PLATFORM(GTK) && !USE(GTK4)
    UserScriptMessageTest::add("WebKitUserContentManager", "script-message-from-dom-bindings", testUserContentManagerScriptMessageFromDOMBindings);
#endif
    WebViewTest::add("WebKitUserContentManager", "content-filter", testUserContentManagerContentFilter);
}

void afterAll()
{
    delete kServer;
}
