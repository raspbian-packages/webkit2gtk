/*
 * Copyright (C) 2020 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"

#if ENABLE(WEBGL)
#include "EXTTextureCompressionRGTC.h"

#include <wtf/IsoMallocInlines.h>

namespace WebCore {

WTF_MAKE_ISO_ALLOCATED_IMPL(EXTTextureCompressionRGTC);

EXTTextureCompressionRGTC::EXTTextureCompressionRGTC(WebGLRenderingContextBase& context)
    : WebGLExtension(context, WebGLExtensionName::EXTTextureCompressionRGTC)
{
    context.protectedGraphicsContextGL()->ensureExtensionEnabled("GL_EXT_texture_compression_rgtc"_s);

    context.addCompressedTextureFormat(GraphicsContextGL::COMPRESSED_RED_RGTC1_EXT);
    context.addCompressedTextureFormat(GraphicsContextGL::COMPRESSED_SIGNED_RED_RGTC1_EXT);
    context.addCompressedTextureFormat(GraphicsContextGL::COMPRESSED_RED_GREEN_RGTC2_EXT);
    context.addCompressedTextureFormat(GraphicsContextGL::COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT);
}

EXTTextureCompressionRGTC::~EXTTextureCompressionRGTC() = default;

bool EXTTextureCompressionRGTC::supported(GraphicsContextGL& context)
{
    return context.supportsExtension("GL_EXT_texture_compression_rgtc"_s);
}

} // namespace WebCore

#endif // ENABLE(WEBGL)
