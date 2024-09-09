/*
 * Copyright (C) 2021-2023 Apple Inc. All rights reserved.
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
#include "GPUDevice.h"

#include "DOMPromiseProxy.h"
#include "EventNames.h"
#include "GPUBindGroup.h"
#include "GPUBindGroupDescriptor.h"
#include "GPUBindGroupLayout.h"
#include "GPUBindGroupLayoutDescriptor.h"
#include "GPUBuffer.h"
#include "GPUBufferDescriptor.h"
#include "GPUCommandEncoder.h"
#include "GPUCommandEncoderDescriptor.h"
#include "GPUComputePipeline.h"
#include "GPUComputePipelineDescriptor.h"
#include "GPUExternalTexture.h"
#include "GPUExternalTextureDescriptor.h"
#include "GPUPipelineError.h"
#include "GPUPipelineLayout.h"
#include "GPUPipelineLayoutDescriptor.h"
#include "GPUPresentationContext.h"
#include "GPUQuerySet.h"
#include "GPUQuerySetDescriptor.h"
#include "GPURenderBundleEncoder.h"
#include "GPURenderBundleEncoderDescriptor.h"
#include "GPURenderPipeline.h"
#include "GPURenderPipelineDescriptor.h"
#include "GPUSampler.h"
#include "GPUSamplerDescriptor.h"
#include "GPUShaderModule.h"
#include "GPUShaderModuleDescriptor.h"
#include "GPUSupportedFeatures.h"
#include "GPUSupportedLimits.h"
#include "GPUTexture.h"
#include "GPUTextureDescriptor.h"
#include "GPUTextureFormat.h"
#include "GPUUncapturedErrorEvent.h"
#include "JSDOMPromiseDeferred.h"
#include "JSGPUComputePipeline.h"
#include "JSGPUDeviceLostInfo.h"
#include "JSGPUInternalError.h"
#include "JSGPUOutOfMemoryError.h"
#include "JSGPUPipelineError.h"
#include "JSGPURenderPipeline.h"
#include "JSGPUUncapturedErrorEvent.h"
#include "JSGPUValidationError.h"
#include "RequestAnimationFrameCallback.h"
#include <wtf/IsoMallocInlines.h>

namespace WebCore {

WTF_MAKE_ISO_ALLOCATED_IMPL(GPUDevice);

GPUDevice::GPUDevice(ScriptExecutionContext* scriptExecutionContext, Ref<WebGPU::Device>&& backing)
    : ActiveDOMObject { scriptExecutionContext }
    , m_lostPromise(makeUniqueRef<LostPromise>())
    , m_backing(WTFMove(backing))
    , m_queue(GPUQueue::create(Ref { m_backing->queue() }))
    , m_autoPipelineLayout(createAutoPipelineLayout())
{
}

GPUDevice::~GPUDevice() = default;

String GPUDevice::label() const
{
    return m_backing->label();
}

void GPUDevice::setLabel(String&& label)
{
    m_backing->setLabel(WTFMove(label));
}

Ref<GPUSupportedFeatures> GPUDevice::features() const
{
    return GPUSupportedFeatures::create(m_backing->features());
}

Ref<GPUSupportedLimits> GPUDevice::limits() const
{
    return GPUSupportedLimits::create(m_backing->limits());
}

Ref<GPUQueue> GPUDevice::queue() const
{
    return m_queue;
}

void GPUDevice::addBufferToUnmap(GPUBuffer& buffer)
{
    m_buffersToUnmap.add(&buffer);
}

void GPUDevice::removeBufferToUnmap(GPUBuffer& buffer)
{
    m_buffersToUnmap.remove(&buffer);
}

void GPUDevice::destroy(ScriptExecutionContext& scriptExecutionContext)
{
    for (auto& buffer : m_buffersToUnmap)
        buffer->destroy(scriptExecutionContext);

    m_buffersToUnmap.clear();

    m_backing->destroy();
}

GPUDevice::LostPromise& GPUDevice::lost()
{
    if (m_waitingForDeviceLostPromise)
        return m_lostPromise;

    m_waitingForDeviceLostPromise = true;
    m_backing->resolveDeviceLostPromise([weakThis = WeakPtr { *this }](WebCore::WebGPU::DeviceLostReason reason) {
        if (!weakThis)
            return;

        auto ref = GPUDeviceLostInfo::create(WebCore::WebGPU::DeviceLostInfo::create(reason, ""_s));
        weakThis->m_lostPromise->resolve(WTFMove(ref));
    });

    return m_lostPromise;
}

ExceptionOr<Ref<GPUBuffer>> GPUDevice::createBuffer(const GPUBufferDescriptor& bufferDescriptor)
{
    auto bufferSize = bufferDescriptor.size;
    if (bufferDescriptor.mappedAtCreation && bufferSize > limits()->maxBufferSize())
        return Exception { ExceptionCode::RangeError };

    auto usage = bufferDescriptor.usage;
    auto mappedAtCreation = bufferDescriptor.mappedAtCreation;
    return GPUBuffer::create(m_backing->createBuffer(bufferDescriptor.convertToBacking()), bufferSize, usage, mappedAtCreation, *this);
}

bool GPUDevice::isSupportedFormat(GPUTextureFormat format) const
{
    const auto& featureContainer = m_backing->features().features();
    switch (format) {
    case GPUTextureFormat::Depth32floatStencil8:
        return featureContainer.contains("depth32float-stencil8"_s);

    // BC compressed formats usable if texture-compression-bc is both
    // supported by the device/user agent and enabled in requestDevice.
    case GPUTextureFormat::Bc1RgbaUnorm:
    case GPUTextureFormat::Bc1RgbaUnormSRGB:
    case GPUTextureFormat::Bc2RgbaUnorm:
    case GPUTextureFormat::Bc2RgbaUnormSRGB:
    case GPUTextureFormat::Bc3RgbaUnorm:
    case GPUTextureFormat::Bc3RgbaUnormSRGB:
    case GPUTextureFormat::Bc4RUnorm:
    case GPUTextureFormat::Bc4RSnorm:
    case GPUTextureFormat::Bc5RgUnorm:
    case GPUTextureFormat::Bc5RgSnorm:
    case GPUTextureFormat::Bc6hRgbUfloat:
    case GPUTextureFormat::Bc6hRgbFloat:
    case GPUTextureFormat::Bc7RgbaUnorm:
    case GPUTextureFormat::Bc7RgbaUnormSRGB:
        return featureContainer.contains("texture-compression-bc"_s);

    // ETC2 compressed formats usable if texture-compression-etc2 is both
    // supported by the device/user agent and enabled in requestDevice.
    case GPUTextureFormat::Etc2Rgb8unorm:
    case GPUTextureFormat::Etc2Rgb8unormSRGB:
    case GPUTextureFormat::Etc2Rgb8a1unorm:
    case GPUTextureFormat::Etc2Rgb8a1unormSRGB:
    case GPUTextureFormat::Etc2Rgba8unorm:
    case GPUTextureFormat::Etc2Rgba8unormSRGB:
    case GPUTextureFormat::EacR11unorm:
    case GPUTextureFormat::EacR11snorm:
    case GPUTextureFormat::EacRg11unorm:
    case GPUTextureFormat::EacRg11snorm:
        return featureContainer.contains("texture-compression-etc2"_s);

    // ASTC compressed formats usable if texture-compression-astc is both
    // supported by the device/user agent and enabled in requestDevice.
    case GPUTextureFormat::Astc4x4Unorm:
    case GPUTextureFormat::Astc4x4UnormSRGB:
    case GPUTextureFormat::Astc5x4Unorm:
    case GPUTextureFormat::Astc5x4UnormSRGB:
    case GPUTextureFormat::Astc5x5Unorm:
    case GPUTextureFormat::Astc5x5UnormSRGB:
    case GPUTextureFormat::Astc6x5Unorm:
    case GPUTextureFormat::Astc6x5UnormSRGB:
    case GPUTextureFormat::Astc6x6Unorm:
    case GPUTextureFormat::Astc6x6UnormSRGB:
    case GPUTextureFormat::Astc8x5Unorm:
    case GPUTextureFormat::Astc8x5UnormSRGB:
    case GPUTextureFormat::Astc8x6Unorm:
    case GPUTextureFormat::Astc8x6UnormSRGB:
    case GPUTextureFormat::Astc8x8Unorm:
    case GPUTextureFormat::Astc8x8UnormSRGB:
    case GPUTextureFormat::Astc10x5Unorm:
    case GPUTextureFormat::Astc10x5UnormSRGB:
    case GPUTextureFormat::Astc10x6Unorm:
    case GPUTextureFormat::Astc10x6UnormSRGB:
    case GPUTextureFormat::Astc10x8Unorm:
    case GPUTextureFormat::Astc10x8UnormSRGB:
    case GPUTextureFormat::Astc10x10Unorm:
    case GPUTextureFormat::Astc10x10UnormSRGB:
    case GPUTextureFormat::Astc12x10Unorm:
    case GPUTextureFormat::Astc12x10UnormSRGB:
    case GPUTextureFormat::Astc12x12Unorm:
    case GPUTextureFormat::Astc12x12UnormSRGB:
        return featureContainer.contains("texture-compression-astc"_s);

    default:
        return true;
    }
}

ExceptionOr<Ref<GPUTexture>> GPUDevice::createTexture(const GPUTextureDescriptor& textureDescriptor)
{
    if (!isSupportedFormat(textureDescriptor.format))
        return Exception { ExceptionCode::TypeError, "GPUDevice.createTexture: Unsupported texture format."_s };

    return GPUTexture::create(m_backing->createTexture(textureDescriptor.convertToBacking()), textureDescriptor, *this);
}

static WebGPU::SamplerDescriptor convertToBacking(const std::optional<GPUSamplerDescriptor>& samplerDescriptor)
{
    if (!samplerDescriptor) {
        return {
            { },
            WebGPU::AddressMode::ClampToEdge,
            WebGPU::AddressMode::ClampToEdge,
            WebGPU::AddressMode::ClampToEdge,
            WebGPU::FilterMode::Nearest,
            WebGPU::FilterMode::Nearest,
            WebGPU::MipmapFilterMode::Nearest,
            0,
            32,
            std::nullopt,
            1
        };
    }

    return samplerDescriptor->convertToBacking();
}

Ref<GPUSampler> GPUDevice::createSampler(const std::optional<GPUSamplerDescriptor>& samplerDescriptor)
{
    return GPUSampler::create(m_backing->createSampler(convertToBacking(samplerDescriptor)));
}

#if ENABLE(VIDEO)
GPUExternalTexture* GPUDevice::externalTextureForDescriptor(const GPUExternalTextureDescriptor& descriptor)
{
    m_videoElementToExternalTextureMap.removeNullReferences();
#if ENABLE(WEB_CODECS)
    if (auto* videoElement = std::get_if<RefPtr<HTMLVideoElement>>(&descriptor.source)) {
#else
    if (auto* videoElement = &descriptor.source) {
#endif
        if (!videoElement->get())
            return nullptr;
        HTMLVideoElement& v = *videoElement->get();
        auto it = m_videoElementToExternalTextureMap.find(v);
        if (it != m_videoElementToExternalTextureMap.end())
            return it->value.get();
    }
    return nullptr;
}

class GPUDeviceVideoFrameRequestCallback final : public VideoFrameRequestCallback {
public:
    CallbackResult<void> handleEvent(double, const VideoFrameMetadata&) override
    {
        auto it = m_weakMap.find(m_videoElement);
        if (it != m_weakMap.end() && m_externalTexture.ptr() == it->value.get())
            m_externalTexture->destroy();

        m_weakMap.remove(m_videoElement);
        return CallbackResult<void>();
    }
    static Ref<GPUDeviceVideoFrameRequestCallback> create(GPUExternalTexture& externalTexture, HTMLVideoElement& videoElement, WeakHashMap<HTMLVideoElement, WeakPtr<GPUExternalTexture>, WeakPtrImplWithEventTargetData>& weakMap, ScriptExecutionContext* scriptExecutionContext)
    {
        return adoptRef(*new GPUDeviceVideoFrameRequestCallback(externalTexture, videoElement, weakMap, scriptExecutionContext));
    }

    ~GPUDeviceVideoFrameRequestCallback() final { }
private:
    GPUDeviceVideoFrameRequestCallback(GPUExternalTexture& externalTexture, HTMLVideoElement& videoElement, WeakHashMap<HTMLVideoElement, WeakPtr<GPUExternalTexture>, WeakPtrImplWithEventTargetData>& weakMap, ScriptExecutionContext* scriptExecutionContext)
        : VideoFrameRequestCallback(scriptExecutionContext)
        , m_externalTexture(externalTexture)
        , m_videoElement(videoElement)
        , m_weakMap(weakMap)
    {
    }

    Ref<GPUExternalTexture> m_externalTexture;
    HTMLVideoElement& m_videoElement;
    WeakHashMap<HTMLVideoElement, WeakPtr<GPUExternalTexture>, WeakPtrImplWithEventTargetData> &m_weakMap;
};
#endif

Ref<GPUExternalTexture> GPUDevice::importExternalTexture(const GPUExternalTextureDescriptor& externalTextureDescriptor)
{
#if ENABLE(VIDEO)
    if (auto* externalTexture = externalTextureForDescriptor(externalTextureDescriptor)) {
        externalTexture->undestroy();
#if ENABLE(WEB_CODECS)
        auto& videoElement = std::get<RefPtr<HTMLVideoElement>>(externalTextureDescriptor.source);
#else
        auto& videoElement = externalTextureDescriptor.source;
#endif
        m_videoElementToExternalTextureMap.remove(*videoElement.get());
        return *externalTexture;
    }
#endif
    auto externalTexture = GPUExternalTexture::create(m_backing->importExternalTexture(externalTextureDescriptor.convertToBacking()));
#if ENABLE(VIDEO)
#if ENABLE(WEB_CODECS)
    if (auto* videoElement = std::get_if<RefPtr<HTMLVideoElement>>(&externalTextureDescriptor.source); videoElement && videoElement->get()) {
#else
    if (auto* videoElement = &externalTextureDescriptor.source; videoElement && videoElement->get()) {
#endif
        HTMLVideoElement* videoElementPtr = videoElement->get();
        m_videoElementToExternalTextureMap.set(*videoElementPtr, externalTexture.get());
        videoElementPtr->requestVideoFrameCallback(GPUDeviceVideoFrameRequestCallback::create(externalTexture.get(), *videoElementPtr, m_videoElementToExternalTextureMap, scriptExecutionContext()));
        queueTaskKeepingObjectAlive(*this, TaskSource::WebGPU, [protecedThis = Ref { *this }, videoElementPtr, externalTextureRef = externalTexture]() {
            auto it = protecedThis->m_videoElementToExternalTextureMap.find(*videoElementPtr);
            if (it == protecedThis->m_videoElementToExternalTextureMap.end() || externalTextureRef.ptr() != it->value.get())
                return;

            externalTextureRef->destroy();
        });
    }
#endif
    return externalTexture;
}

ExceptionOr<Ref<GPUBindGroupLayout>> GPUDevice::createBindGroupLayout(const GPUBindGroupLayoutDescriptor& bindGroupLayoutDescriptor)
{
    for (auto& entry : bindGroupLayoutDescriptor.entries) {
        if (entry.storageTexture) {
            if (!isSupportedFormat(entry.storageTexture->format))
                return Exception { ExceptionCode::TypeError, "GPUDevice.createBindGroupLayout: Unsupported texture format."_s };
        }
    }
    return GPUBindGroupLayout::create(m_backing->createBindGroupLayout(bindGroupLayoutDescriptor.convertToBacking()));
}

Ref<GPUPipelineLayout> GPUDevice::createAutoPipelineLayout()
{
    return GPUPipelineLayout::create(m_backing->createPipelineLayout(WebGPU::PipelineLayoutDescriptor {
        { "autoLayout"_s, },
        std::nullopt
    }));
}

Ref<GPUPipelineLayout> GPUDevice::createPipelineLayout(const GPUPipelineLayoutDescriptor& pipelineLayoutDescriptor)
{
    return GPUPipelineLayout::create(m_backing->createPipelineLayout(pipelineLayoutDescriptor.convertToBacking()));
}

Ref<GPUBindGroup> GPUDevice::createBindGroup(const GPUBindGroupDescriptor& bindGroupDescriptor)
{
    return GPUBindGroup::create(m_backing->createBindGroup(bindGroupDescriptor.convertToBacking()));
}

Ref<GPUShaderModule> GPUDevice::createShaderModule(const GPUShaderModuleDescriptor& shaderModuleDescriptor)
{
    return GPUShaderModule::create(m_backing->createShaderModule(shaderModuleDescriptor.convertToBacking(m_autoPipelineLayout)));
}

Ref<GPUComputePipeline> GPUDevice::createComputePipeline(const GPUComputePipelineDescriptor& computePipelineDescriptor)
{
    return GPUComputePipeline::create(m_backing->createComputePipeline(computePipelineDescriptor.convertToBacking(m_autoPipelineLayout)));
}

ExceptionOr<Ref<GPURenderPipeline>> GPUDevice::createRenderPipeline(const GPURenderPipelineDescriptor& renderPipelineDescriptor)
{
    if (renderPipelineDescriptor.fragment) {
        for (auto& colorState : renderPipelineDescriptor.fragment->targets) {
            if (colorState) {
                if (!isSupportedFormat(colorState->format))
                    return Exception { ExceptionCode::TypeError, "GPUDevice.createRenderPipeline: Unsupported texture format for color target."_s };
            }
        }
    }
    if (renderPipelineDescriptor.depthStencil) {
        if (!isSupportedFormat(renderPipelineDescriptor.depthStencil->format))
            return Exception { ExceptionCode::TypeError, "GPUDevice.createRenderPipeline: Unsupported texture format for depth target."_s };
    }

    return GPURenderPipeline::create(m_backing->createRenderPipeline(renderPipelineDescriptor.convertToBacking(m_autoPipelineLayout)));
}

void GPUDevice::createComputePipelineAsync(const GPUComputePipelineDescriptor& computePipelineDescriptor, CreateComputePipelineAsyncPromise&& promise)
{
    m_backing->createComputePipelineAsync(computePipelineDescriptor.convertToBacking(m_autoPipelineLayout), [promise = WTFMove(promise)](RefPtr<WebGPU::ComputePipeline>&& computePipeline) mutable {
        if (computePipeline.get())
            promise.resolve(GPUComputePipeline::create(computePipeline.releaseNonNull()));
        else
            promise.rejectType<IDLInterface<GPUPipelineError>>(GPUPipelineError::create(""_s, { GPUPipelineErrorReason::Validation }));
    });
}

ExceptionOr<void> GPUDevice::createRenderPipelineAsync(const GPURenderPipelineDescriptor& renderPipelineDescriptor, CreateRenderPipelineAsyncPromise&& promise)
{
    if (renderPipelineDescriptor.fragment) {
        for (auto& colorState : renderPipelineDescriptor.fragment->targets) {
            if (colorState) {
                if (!isSupportedFormat(colorState->format))
                    return Exception { ExceptionCode::TypeError, "GPUDevice.createRenderBundleEncoder: Unsupported texture format for color format."_s };
            }
        }
    }
    if (renderPipelineDescriptor.depthStencil) {
        if (!isSupportedFormat(renderPipelineDescriptor.depthStencil->format))
            return Exception { ExceptionCode::TypeError, "GPUDevice.createRenderBundleEncoder: Unsupported texture format for color format."_s };
    }

    m_backing->createRenderPipelineAsync(renderPipelineDescriptor.convertToBacking(m_autoPipelineLayout), [promise = WTFMove(promise)](RefPtr<WebGPU::RenderPipeline>&& renderPipeline) mutable {
        if (renderPipeline.get())
            promise.resolve(GPURenderPipeline::create(renderPipeline.releaseNonNull()));
        else
            promise.rejectType<IDLInterface<GPUPipelineError>>(GPUPipelineError::create(""_s, { GPUPipelineErrorReason::Validation }));
    });
    return { };
}

static WebGPU::CommandEncoderDescriptor convertToBacking(const std::optional<GPUCommandEncoderDescriptor>& commandEncoderDescriptor)
{
    if (!commandEncoderDescriptor)
        return { };

    return commandEncoderDescriptor->convertToBacking();
}

Ref<GPUCommandEncoder> GPUDevice::createCommandEncoder(const std::optional<GPUCommandEncoderDescriptor>& commandEncoderDescriptor)
{
    return GPUCommandEncoder::create(m_backing->createCommandEncoder(convertToBacking(commandEncoderDescriptor)));
}

ExceptionOr<Ref<GPURenderBundleEncoder>> GPUDevice::createRenderBundleEncoder(const GPURenderBundleEncoderDescriptor& renderBundleEncoderDescriptor)
{
    for (auto& colorFormat : renderBundleEncoderDescriptor.colorFormats) {
        if (colorFormat) {
            if (!isSupportedFormat(*colorFormat))
                return Exception { ExceptionCode::TypeError, "GPUDevice.createRenderBundleEncoder: Unsupported texture format for color format."_s };
        }
    }
    if (renderBundleEncoderDescriptor.depthStencilFormat) {
        if (!isSupportedFormat(*renderBundleEncoderDescriptor.depthStencilFormat))
            return Exception { ExceptionCode::TypeError, "GPUDevice.createRenderBundleEncoder: Unsupported texture format for depth format."_s };
    }

    return GPURenderBundleEncoder::create(m_backing->createRenderBundleEncoder(renderBundleEncoderDescriptor.convertToBacking()));
}

ExceptionOr<Ref<GPUQuerySet>> GPUDevice::createQuerySet(const GPUQuerySetDescriptor& querySetDescriptor)
{
    if (querySetDescriptor.type == GPUQueryType::Timestamp) {
        if (!m_backing->features().features().contains("timestamp-query"_s))
            return Exception { ExceptionCode::TypeError, "Timestamp queries are not supported."_s };
    }

    return GPUQuerySet::create(m_backing->createQuerySet(querySetDescriptor.convertToBacking()), querySetDescriptor);
}

void GPUDevice::pushErrorScope(GPUErrorFilter errorFilter)
{
    m_backing->pushErrorScope(convertToBacking(errorFilter));
}

static GPUError createGPUErrorFromWebGPUError(auto& webGPUError)
{
    return WTF::switchOn(WTFMove(*webGPUError), [](Ref<WebGPU::OutOfMemoryError>&& outOfMemoryError) {
        GPUError error = RefPtr<GPUOutOfMemoryError>(GPUOutOfMemoryError::create(WTFMove(outOfMemoryError)));
        return error;
    }, [](Ref<WebGPU::ValidationError>&& validationError) {
        GPUError error = RefPtr<GPUValidationError>(GPUValidationError::create(WTFMove(validationError)));
        return error;
    }, [](Ref<WebGPU::InternalError>&& internalError) {
        GPUError error = RefPtr<GPUInternalError>(GPUInternalError::create(WTFMove(internalError)));
        return error;
    });
}

void GPUDevice::popErrorScope(ErrorScopePromise&& errorScopePromise)
{
    m_backing->popErrorScope([promise = WTFMove(errorScopePromise)](bool success, std::optional<WebGPU::Error>&& error) mutable {
        if (!error) {
            if (success)
                promise.resolve(std::nullopt);
            else
                promise.reject(Exception { ExceptionCode::OperationError, "popErrorScope failed"_s });
            return;
        }
        promise.resolve(createGPUErrorFromWebGPUError(error));
    });
}

bool GPUDevice::addEventListener(const AtomString& eventType, Ref<EventListener>&& eventListener, const AddEventListenerOptions& options)
{
    auto result = EventTarget::addEventListener(eventType, WTFMove(eventListener), options);
#if PLATFORM(COCOA)
    if (eventType == WebCore::eventNames().uncapturederrorEvent) {
        m_backing->resolveUncapturedErrorEvent([eventType, weakThis = WeakPtr { *this }](bool hasUncapturedError, std::optional<WebGPU::Error>&& error) {
            if (!weakThis || !hasUncapturedError)
                return;

            queueTaskToDispatchEvent(*weakThis.get(), TaskSource::WebGPU, GPUUncapturedErrorEvent::create(WebCore::eventNames().uncapturederrorEvent, GPUUncapturedErrorEventInit { .error = createGPUErrorFromWebGPUError(error) }));
        });
    }
#endif
    return result;
}

}
