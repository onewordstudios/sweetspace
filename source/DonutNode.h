//
// Created by Downian on 3/9/20.
// Copyright (c) 2020 onewordstudios. All rights reserved.
//

#ifndef SWEETSPACE_DONUTNODE_H
#define SWEETSPACE_DONUTNODE_H

#include <cugl/2d/CUPolygonNode.h>
#include "DonutModel.h"

class DonutNode : public cugl::PolygonNode {
#pragma mark Values
protected:
    std::shared_ptr<DonutModel> donutmodel;

public:
#pragma mark -
#pragma mark Constructor
    /**
     * Creates an empty polygon with the degenerate texture.
     *
     * You must initialize this PolygonNode before use.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead.
     */
    DonutNode() : cugl::PolygonNode() { }

    /**
     * Releases all resources allocated with this node.
     *
     * This will release, but not necessarily delete the associated texture.
     * However, the polygon and drawing commands will be deleted and no
     * longer safe to use.
     */
    ~DonutNode() { dispose(); }

#pragma mark -

void setModel(std::shared_ptr<DonutModel> model) { donutmodel = model; }

std::shared_ptr<DonutModel> getModel() { return donutmodel; }

void draw(const std::shared_ptr<SpriteBatch>& cugl::batch, const cugl::Mat4& transform, cugl::Color4 tint) override;
};


#endif //SWEETSPACE_DONUTNODE_H
