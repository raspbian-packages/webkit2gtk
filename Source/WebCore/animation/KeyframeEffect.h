/*
 * Copyright (C) 2018 Apple Inc. All rights reserved.
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

#include "AnimationEffect.h"
#include "AnimationEffectPhase.h"
#include "CSSPropertyBlendingClient.h"
#include "CompositeOperation.h"
#include "CompositeOperationOrAuto.h"
#include "EffectTiming.h"
#include "Element.h"
#include "IterationCompositeOperation.h"
#include "KeyframeEffectOptions.h"
#include "KeyframeList.h"
#include "RenderStyle.h"
#include "StyleProperties.h"
#include "Styleable.h"
#include "WebAnimationTypes.h"
#include <wtf/Ref.h>

namespace WebCore {

class Element;
class FilterOperations;

namespace Style {
struct ResolutionContext;
}

class KeyframeEffect : public AnimationEffect
    , public CSSPropertyBlendingClient {
public:
    static ExceptionOr<Ref<KeyframeEffect>> create(JSC::JSGlobalObject&, Document&, Element*, JSC::Strong<JSC::JSObject>&&, std::optional<std::variant<double, KeyframeEffectOptions>>&&);
    static ExceptionOr<Ref<KeyframeEffect>> create(Ref<KeyframeEffect>&&);
    static Ref<KeyframeEffect> create(const Element&, PseudoId);
    ~KeyframeEffect() { }

    struct BasePropertyIndexedKeyframe {
        std::variant<std::nullptr_t, Vector<std::optional<double>>, double> offset = Vector<std::optional<double>>();
        std::variant<Vector<String>, String> easing = Vector<String>();
        std::variant<Vector<CompositeOperationOrAuto>, CompositeOperationOrAuto> composite = Vector<CompositeOperationOrAuto>();
    };

    struct BaseKeyframe {
        MarkableDouble offset;
        String easing { "linear" };
        CompositeOperationOrAuto composite { CompositeOperationOrAuto::Auto };
    };

    struct PropertyAndValues {
        CSSPropertyID property;
        Vector<String> values;
    };

    struct KeyframeLikeObject {
        BasePropertyIndexedKeyframe baseProperties;
        Vector<PropertyAndValues> propertiesAndValues;
    };

    struct BaseComputedKeyframe : BaseKeyframe {
        double computedOffset;
    };

    struct ComputedKeyframe : BaseComputedKeyframe {
        HashMap<CSSPropertyID, String> styleStrings;
    };

    struct ParsedKeyframe : ComputedKeyframe {
        RefPtr<TimingFunction> timingFunction;
        Ref<MutableStyleProperties> style;

        ParsedKeyframe()
            : style(MutableStyleProperties::create())
        {
        }
    };

    const Vector<ParsedKeyframe>& parsedKeyframes() const { return m_parsedKeyframes; }

    Element* target() const { return m_target.get(); }
    void setTarget(RefPtr<Element>&&);

    bool targetsPseudoElement() const;
    const String pseudoElement() const;
    ExceptionOr<void> setPseudoElement(const String&);

    const std::optional<const Styleable> targetStyleable() const;

    Vector<ComputedKeyframe> getKeyframes(Document&);
    ExceptionOr<void> setBindingsKeyframes(JSC::JSGlobalObject&, Document&, JSC::Strong<JSC::JSObject>&&);
    ExceptionOr<void> setKeyframes(JSC::JSGlobalObject&, Document&, JSC::Strong<JSC::JSObject>&&);

    IterationCompositeOperation iterationComposite() const { return m_iterationCompositeOperation; }
    void setIterationComposite(IterationCompositeOperation iterationCompositeOperation) { m_iterationCompositeOperation = iterationCompositeOperation; }
    CompositeOperation composite() const { return m_compositeOperation; }
    void setComposite(CompositeOperation compositeOperation) { m_compositeOperation = compositeOperation; }
    CompositeOperation bindingsComposite() const;
    void setBindingsComposite(CompositeOperation);

    void getAnimatedStyle(std::unique_ptr<RenderStyle>& animatedStyle);
    void apply(RenderStyle& targetStyle, const Style::ResolutionContext&, std::optional<Seconds> = std::nullopt);
    void invalidate();

    void animationTimingDidChange();
    void transformRelatedPropertyDidChange();
    void propertyAffectingLogicalPropertiesDidChange(RenderStyle&, const Style::ResolutionContext&);
    OptionSet<AcceleratedActionApplicationResult> applyPendingAcceleratedActions();

    void willChangeRenderer();

    RenderElement* renderer() const override;
    const RenderStyle& currentStyle() const override;
    bool triggersStackingContext() const { return m_triggersStackingContext; }
    bool isRunningAccelerated() const { return m_runningAccelerated == RunningAccelerated::Yes; }

    // FIXME: These ignore the fact that some timing functions can prevent acceleration.
    bool isAboutToRunAccelerated() const { return m_acceleratedPropertiesState != AcceleratedProperties::None && m_lastRecordedAcceleratedAction != AcceleratedAction::Stop; }

    bool filterFunctionListsMatch() const override { return m_filterFunctionListsMatch; }
    bool transformFunctionListsMatch() const override { return m_transformFunctionListsMatch; }
#if ENABLE(FILTERS_LEVEL_2)
    bool backdropFilterFunctionListsMatch() const override { return m_backdropFilterFunctionListsMatch; }
#endif
    bool colorFilterFunctionListsMatch() const override { return m_colorFilterFunctionListsMatch; }

    void computeDeclarativeAnimationBlendingKeyframes(const RenderStyle* oldStyle, const RenderStyle& newStyle, const Style::ResolutionContext&);
    const KeyframeList& blendingKeyframes() const { return m_blendingKeyframes; }
    const HashSet<CSSPropertyID>& animatedProperties();
    bool animatesProperty(CSSPropertyID) const;

    bool computeExtentOfTransformAnimation(LayoutRect&) const;
    bool computeTransformedExtentViaTransformList(const FloatRect&, const RenderStyle&, LayoutRect&) const;
    bool computeTransformedExtentViaMatrix(const FloatRect&, const RenderStyle&, LayoutRect&) const;
    bool forceLayoutIfNeeded();

    enum class Accelerated : uint8_t { Yes, No };
    bool isCurrentlyAffectingProperty(CSSPropertyID, Accelerated = Accelerated::No) const;
    bool isRunningAcceleratedAnimationForProperty(CSSPropertyID) const;
    bool isRunningAcceleratedTransformRelatedAnimation() const;

    bool requiresPseudoElement() const;
    bool hasImplicitKeyframes() const;

    void stopAcceleratingTransformRelatedProperties(UseAcceleratedAction);

    void keyframesRuleDidChange();

    bool preventsAcceleration() const;

    static String CSSPropertyIDToIDLAttributeName(CSSPropertyID);

private:
    KeyframeEffect(Element*, PseudoId);

    enum class AcceleratedAction : uint8_t { Play, Pause, UpdateTiming, TransformChange, Stop };
    enum class BlendingKeyframesSource : uint8_t { CSSAnimation, CSSTransition, WebAnimation };
    enum class AcceleratedProperties : uint8_t { None, Some, All };
    enum class RunningAccelerated : uint8_t { NotStarted, Yes, No };

    Document* document() const;
    void updateEffectStackMembership();
    void copyPropertiesFromSource(Ref<KeyframeEffect>&&);
    void didChangeTargetStyleable(const std::optional<const Styleable>&);
    ExceptionOr<void> processKeyframes(JSC::JSGlobalObject&, Document&, JSC::Strong<JSC::JSObject>&&);
    void addPendingAcceleratedAction(AcceleratedAction);
    bool canBeAccelerated() const;
    bool isCompletelyAccelerated() const { return m_acceleratedPropertiesState == AcceleratedProperties::All; }
    void updateAcceleratedActions();
    void setAnimatedPropertiesInStyle(RenderStyle&, double);
    TimingFunction* timingFunctionForKeyframeAtIndex(size_t) const;
    TimingFunction* timingFunctionForBlendingKeyframe(const KeyframeValue&) const;
    Ref<const Animation> backingAnimationForCompositedRenderer() const;
    void computedNeedsForcedLayout();
    void computeStackingContextImpact();
    void computeSomeKeyframesUseStepsTimingFunction();
    void clearBlendingKeyframes();
    void updateBlendingKeyframes(RenderStyle& elementStyle, const Style::ResolutionContext&);
    void computeCSSAnimationBlendingKeyframes(const RenderStyle& unanimatedStyle, const Style::ResolutionContext&);
    void computeCSSTransitionBlendingKeyframes(const RenderStyle* oldStyle, const RenderStyle& newStyle);
    void computeAcceleratedPropertiesState();
    void setBlendingKeyframes(KeyframeList&);
    bool isTargetingTransformRelatedProperty() const;
    void checkForMatchingTransformFunctionLists();
    void checkForMatchingFilterFunctionLists();
    void checkForMatchingColorFilterFunctionLists();
    bool checkForMatchingFilterFunctionLists(CSSPropertyID, const std::function<const FilterOperations& (const RenderStyle&)>&) const;
#if ENABLE(FILTERS_LEVEL_2)
    void checkForMatchingBackdropFilterFunctionLists();
#endif
    void computeHasImplicitKeyframeForAcceleratedProperty();
    void computeHasKeyframeComposingAcceleratedProperty();

    // AnimationEffect
    bool isKeyframeEffect() const final { return true; }
    void animationDidTick() final;
    void animationDidChangeTimingProperties() final;
    void animationWasCanceled() final;
    void animationSuspensionStateDidChange(bool) final;
    void animationTimelineDidChange(AnimationTimeline*) final;
    void setAnimation(WebAnimation*) final;
    Seconds timeToNextTick(BasicEffectTiming) const final;
    bool ticksContinouslyWhileActive() const final;
    std::optional<double> progressUntilNextStep(double) const final;

    KeyframeList m_blendingKeyframes { emptyString() };
    HashSet<CSSPropertyID> m_animatedProperties;
    Vector<ParsedKeyframe> m_parsedKeyframes;
    Vector<AcceleratedAction> m_pendingAcceleratedActions;
    RefPtr<Element> m_target;
    PseudoId m_pseudoId { PseudoId::None };

    AcceleratedAction m_lastRecordedAcceleratedAction { AcceleratedAction::Stop };
    BlendingKeyframesSource m_blendingKeyframesSource { BlendingKeyframesSource::WebAnimation };
    IterationCompositeOperation m_iterationCompositeOperation { IterationCompositeOperation::Replace };
    CompositeOperation m_compositeOperation { CompositeOperation::Replace };
    AcceleratedProperties m_acceleratedPropertiesState { AcceleratedProperties::None };
    AnimationEffectPhase m_phaseAtLastApplication { AnimationEffectPhase::Idle };
    RunningAccelerated m_runningAccelerated;
    bool m_needsForcedLayout { false };
    bool m_triggersStackingContext { false };
    bool m_transformFunctionListsMatch { false };
    bool m_filterFunctionListsMatch { false };
#if ENABLE(FILTERS_LEVEL_2)
    bool m_backdropFilterFunctionListsMatch { false };
#endif
    bool m_colorFilterFunctionListsMatch { false };
    bool m_inTargetEffectStack { false };
    bool m_someKeyframesUseStepsTimingFunction { false };
    bool m_hasImplicitKeyframeForAcceleratedProperty { false };
    bool m_hasKeyframeComposingAcceleratedProperty { false };
};

} // namespace WebCore

SPECIALIZE_TYPE_TRAITS_ANIMATION_EFFECT(KeyframeEffect, isKeyframeEffect());
