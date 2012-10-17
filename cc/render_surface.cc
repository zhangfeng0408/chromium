// Copyright 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "config.h"

#include "cc/render_surface.h"

#include "CCMathUtil.h"
#include "cc/layer.h"
#include <public/WebTransformationMatrix.h>

using WebKit::WebTransformationMatrix;

namespace cc {

RenderSurfaceChromium::RenderSurfaceChromium(LayerChromium* owningLayer)
    : m_owningLayer(owningLayer)
    , m_drawOpacity(1)
    , m_drawOpacityIsAnimating(false)
    , m_targetSurfaceTransformsAreAnimating(false)
    , m_screenSpaceTransformsAreAnimating(false)
    , m_nearestAncestorThatMovesPixels(0)
{
}

RenderSurfaceChromium::~RenderSurfaceChromium()
{
}

FloatRect RenderSurfaceChromium::drawableContentRect() const
{
    FloatRect drawableContentRect = CCMathUtil::mapClippedRect(m_drawTransform, m_contentRect);
    if (m_owningLayer->hasReplica())
        drawableContentRect.unite(CCMathUtil::mapClippedRect(m_replicaDrawTransform, m_contentRect));
    return drawableContentRect;
}

}
