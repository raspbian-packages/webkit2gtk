/*
 * Copyright (C) 2004, 2005, 2006, 2008 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2006, 2007 Rob Buis <buis@kde.org>
 * Copyright (C) 2018 Apple Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
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

#pragma once

#include "SVGAnimatedPointList.h"
#include "SVGExternalResourcesRequired.h"
#include "SVGGeometryElement.h"
#include "SVGNames.h"

namespace WebCore {

class SVGPolyElement : public SVGGeometryElement, public SVGExternalResourcesRequired {
    WTF_MAKE_ISO_ALLOCATED(SVGPolyElement);
public:
    Ref<SVGPointList> points();
    Ref<SVGPointList> animatedPoints();

    const SVGPointListValues& pointList() const { return m_points.value(); }

    size_t approximateMemoryCost() const override;

protected:
    SVGPolyElement(const QualifiedName&, Document&);

private:
    using AttributeOwnerProxy = SVGAttributeOwnerProxyImpl<SVGPolyElement, SVGGeometryElement, SVGExternalResourcesRequired>;
    static AttributeOwnerProxy::AttributeRegistry& attributeRegistry() { return AttributeOwnerProxy::attributeRegistry(); }
    static bool isKnownAttribute(const QualifiedName& attributeName) { return AttributeOwnerProxy::isKnownAttribute(attributeName); }
    static void registerAttributes();

    const SVGAttributeOwnerProxy& attributeOwnerProxy() const final { return m_attributeOwnerProxy; }
    void parseAttribute(const QualifiedName&, const AtomicString&) override; 
    void svgAttributeChanged(const QualifiedName&) override;

    bool isValid() const override { return SVGTests::isValid(); }
    bool supportsMarkers() const override { return true; }

    AttributeOwnerProxy m_attributeOwnerProxy { *this };
    SVGAnimatedPointListAttribute m_points;
};

} // namespace WebCore

SPECIALIZE_TYPE_TRAITS_BEGIN(WebCore::SVGPolyElement)
    static bool isType(const WebCore::SVGElement& element) { return element.hasTagName(WebCore::SVGNames::polygonTag) || element.hasTagName(WebCore::SVGNames::polylineTag); }
    static bool isType(const WebCore::Node& node) { return is<WebCore::SVGElement>(node) && isType(downcast<WebCore::SVGElement>(node)); }
SPECIALIZE_TYPE_TRAITS_END()
