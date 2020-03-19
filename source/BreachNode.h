#ifndef SWEETSPACE_BREACHNODE_H
#define SWEETSPACE_BREACHNODE_H

#include <cugl/2d/CUAnimationNode.h>

#include "BreachModel.h"

class BreachNode : public cugl::AnimationNode {
#pragma mark Values
protected:
    std::shared_ptr<BreachModel> breachModel;

public:
#pragma mark -
#pragma mark Constructor
    /**
     * Creates an empty Breach with the degenerate texture.
     *
     * You must initialize this BreachNode before use.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead.
     */
    BreachNode() : cugl::AnimationNode() {}

    /**
     * Releases all resources allocated with this node.
     *
     * This will release, but not necessarily delete the associated texture.
     * However, the breach and drawing commands will be deleted and no
     * longer safe to use.
     */
    ~BreachNode() { dispose(); }

#pragma mark -
    /**
     * Returns a textured breach from a Texture object.
     *
     * After creation, the breach will be a rectangle. The vertices of this
     * breach will be the corners of the texture.
     *
     * @param texture   A shared pointer to a Texture object.
     *
     * @return a textured breach from a Texture object.
     */
    static std::shared_ptr<BreachNode> allocWithTexture(
            const std::shared_ptr<cugl::Texture> &texture) {
        std::shared_ptr<BreachNode> node = std::make_shared<BreachNode>();
        return (node->initWithTexture(texture) ? node : nullptr);
    }

    void setModel(std::shared_ptr<BreachModel> model) { breachModel = model; }

    std::shared_ptr<BreachModel> getModel() { return breachModel; }

    void draw(const shared_ptr<cugl::SpriteBatch> &batch, const cugl::Mat4 &transform,
            cugl::Color4 tint) override;
};

#endif //SWEETSPACE_BREACHNODE_H
