//
// Created by Downian on 3/9/20.
// Copyright (c) 2020 onewordstudios. All rights reserved.
//

#include "DonutNode.h"
#include <cugl/2d/CUPolygonNode.h>

using namespace cugl;

void draw(const std::shared_ptr<SpriteBatch>& batch, const Mat4& transform, Color4 tint){
    setPosition(donutModel.getSceneGraphPosition());
    PolygonNode::draw(batch, transform, tint);
}