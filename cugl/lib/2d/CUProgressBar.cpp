//
//  CUProgressBar.cpp
//  Cornell University Game Library (CUGL)
//
//  This module provides support for a simple progress bar, which is useful
//  for displaying things such as asset loading.
//
//  This class uses our standard shared-pointer architecture.
//
//  1. The constructor does not perform any initialization; it just sets all
//     attributes to their defaults.
//
//  2. All initialization takes place via init methods, which can fail if an
//     object is initialized more than once.
//
//  3. All allocation takes place via static constructors which return a shared
//     pointer.
//
//  CUGL MIT License:
//      This software is provided 'as-is', without any express or implied
//      warranty.  In no event will the authors be held liable for any damages
//      arising from the use of this software.
//
//      Permission is granted to anyone to use this software for any purpose,
//      including commercial applications, and to alter it and redistribute it
//      freely, subject to the following restrictions:
//
//      1. The origin of this software must not be misrepresented; you must not
//      claim that you wrote the original software. If you use this software
//      in a product, an acknowledgment in the product documentation would be
//      appreciated but is not required.
//
//      2. Altered source versions must be plainly marked as such, and must not
//      be misrepresented as being the original software.
//
//      3. This notice may not be removed or altered from any source distribution.
//
//  Author: Walker White
//  Version: 1/8/17
//
#include <cugl/2d/CUProgressBar.h>
#include <cugl/assets/CUSceneLoader.h>
#include <cugl/assets/CUAssetManager.h>

using namespace cugl;

#define UNKNOWN_STR "<unknown>"
#pragma mark -
#pragma mark Constructors

/**
 * Initializes a progress bar with the given textures and size.
 *
 * The progress bar will be the size of the background texture.  The
 * foreground texture and end caps will be scaled so that they are this
 * size when combined together.  None of the textures will be tinted.
 *
 * @param background    The texture for the background
 * @param foreground    The texture for the animated foreground
 * @param beginCap      The left end cap of the foreground
 * @param finalCap      The right end cap of the foreground
 *
 * @return true if the progress bar is initialized properly, false otherwise.
 */
bool ProgressBar::initWithCaps(const std::shared_ptr<Texture>& background,
                               const std::shared_ptr<Texture>& foreground,
                               const std::shared_ptr<Texture>& beginCap,
                               const std::shared_ptr<Texture>& finalCap) {
    CUAssertLog(background != nullptr, "Background texture cannot be null if there is no specified size");
    return initWithCaps(background, foreground, beginCap, finalCap, background->getSize());
}

/**
 * Initializes a progress bar with the given textures and size.
 *
 * The progress bar will scale the background texture to the given size.
 * The foreground texture and end caps will be scaled so that they are this
 * size when combined together.  None of the textures will be tinted.
 *
 * @param background    The texture for the background
 * @param foreground    The texture for the animated foreground
 * @param beginCap      The left end cap of the foreground
 * @param finalCap      The right end cap of the foreground
 * @param size          The progress bar size
 *
 * @return true if the progress bar is initialized properly, false otherwise.
 */
bool ProgressBar::initWithCaps(const std::shared_ptr<Texture>& background,
                               const std::shared_ptr<Texture>& foreground,
                               const std::shared_ptr<Texture>& beginCap,
                               const std::shared_ptr<Texture>& finalCap,
                               const Size& size) {
    if (!Node::initWithBounds(size)) {
        return false;
    }

    std::shared_ptr<Texture> temp = (background == nullptr ? SpriteBatch::getBlankTexture() : background);
    Vec2 scale = size/temp->getSize();
    _background = PolygonNode::allocWithTexture(temp);
    _background->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
    _background->setPosition(0,0);
    _background->setScale(scale);
    addChild(_background);
    
    _progress = 0;
    _foresize = size;

    if (beginCap != nullptr) {
        scale = size/beginCap->getSize();
        scale.x = 1.0f;
        _begincap = PolygonNode::allocWithTexture(beginCap);
        _begincap->setScale(scale);
        _begincap->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
        _begincap->setPosition(0,0);
        addChild(_begincap);
        _foresize.width -= beginCap->getSize().width;
    }

    if (finalCap != nullptr) {
        scale = size/finalCap->getSize();
        scale.x = 1.0f;
        _finalcap = PolygonNode::allocWithTexture(finalCap);
        _finalcap->setScale(scale);
        _finalcap->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
        if (_begincap == nullptr) {
            _finalcap->setPosition(-_background->getSize().width/2.0f,0);
        } else {
            _finalcap->setPosition(_begincap->getBoundingBox().getMaxX(),0);
        }
        addChild(_finalcap);
        _foresize.width -= finalCap->getSize().width;
    }

    if (foreground != nullptr) {
        scale = size/foreground->getSize();
        scale.x = 1.0f;
        _foreground = PolygonNode::allocWithTexture(foreground);
        _foreground->setScale(scale);
        _foresize.height /= scale.y;
    } else {
        std::shared_ptr<Texture> temp = SpriteBatch::getBlankTexture();
        scale = size/temp->getSize();
        scale.x = 1.0f;
        _foreground = PolygonNode::allocWithTexture(temp);
        _foreground->setColor(Color4::RED);
        _foreground->setScale(scale);
        _foresize.height /= scale.y;
    }

    _foreground->setPolygon(Rect(0,0,0,_foresize.height));
    _foreground->setContentSize(Size(0,_foresize.height));

    _foreground->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
    if (_begincap == nullptr) {
        _foreground->setPosition(-_background->getSize().width/2.0f,0);
    } else {
        _foreground->setPosition(_begincap->getBoundingBox().getMaxX(),0);
    }
    addChild(_foreground);
    return true;
}

/**
 * Initializes a node with the given JSON specificaton.
 *
 * This initializer is designed to receive the "data" object from the
 * JSON passed to {@link SceneLoader}.  This JSON format supports all
 * of the attribute values of its parent class.  In addition, it supports
 * the following additional attributes:
 *
 *      "foreground":   The name of a previously loaded texture asset
 *      "background":   The name of a previously loaded texture asset
 *      "left_cap":     The name of a previously loaded texture asset
 *      "right_cap":    The name of a previously loaded texture asset
 *
 * All attributes are optional.  There are no required attributes.
 *
 * @param loader    The scene loader passing this JSON file
 * @param data      The JSON object specifying the node
 *
 * @return true if initialization was successful.
 */
bool ProgressBar::initWithData(const SceneLoader* loader, const std::shared_ptr<JsonValue>& data) {
    if (!data) {
        return init();
    } else if (!Node::initWithData(loader,data)) {
        return false;
    }
    
    // All of the code that follows can corrupt the position.
    Vec2 coord = getPosition();
    
    std::string temp = data->getString("background",UNKNOWN_STR);
    const AssetManager* assets = loader->getManager();
    
    std::shared_ptr<Texture> background = assets->get<Texture>(temp);
    if (background == nullptr) {
        background = SpriteBatch::getBlankTexture();
    }
    
    Size size;
    if (data->has("size")) {
        JsonValue* sdata = data->get("size").get();
        size.width  = sdata->get(0)->asFloat(0.0f);
        size.height = sdata->get(1)->asFloat(0.0f);
    } else {
        size = background->getSize();
    }
    setContentSize(size);
    
    Vec2 scale = size/background->getSize();
    _background = PolygonNode::allocWithTexture(background);
    _background->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
    _background->setPosition(0,0);
    _background->setScale(scale);
    addChild(_background);
    
    _progress = 0;
    _foresize = size;
    
    temp = data->getString("left_cap",UNKNOWN_STR);
    std::shared_ptr<Texture> beginCap = assets->get<Texture>(temp);
    if (beginCap != nullptr) {
        scale = size/beginCap->getSize();
        scale.x = 1.0f;
        _begincap = PolygonNode::allocWithTexture(beginCap);
        _begincap->setScale(scale);
        _begincap->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
        _begincap->setPosition(0,0);
        addChild(_begincap);
        _foresize.width -= beginCap->getSize().width;
    }
    
    temp = data->getString("right_cap",UNKNOWN_STR);
    std::shared_ptr<Texture> finalCap = assets->get<Texture>(temp);
    if (finalCap != nullptr) {
        scale = size/finalCap->getSize();
        scale.x = 1.0f;
        _finalcap = PolygonNode::allocWithTexture(finalCap);
        _finalcap->setScale(scale);
        _finalcap->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
        if (_begincap == nullptr) {
            _finalcap->setPosition(-_background->getSize().width/2.0f,0);
        } else {
            _finalcap->setPosition(_begincap->getBoundingBox().getMaxX(),0);
        }
        addChild(_finalcap);
        _foresize.width -= finalCap->getSize().width;
    }
    
    temp = data->getString("foreground",UNKNOWN_STR);
    std::shared_ptr<Texture> foreground = assets->get<Texture>(temp);
    if (foreground != nullptr) {
        scale = size/foreground->getSize();
        scale.x = 1.0f;
        _foreground = PolygonNode::allocWithTexture(foreground);
        _foreground->setScale(scale);
        _foresize.height /= scale.y;
    } else {
        std::shared_ptr<Texture> temp = SpriteBatch::getBlankTexture();
        scale = size/temp->getSize();
        scale.x = 1.0f;
        _foreground = PolygonNode::allocWithTexture(temp);
        _foreground->setColor(Color4::RED);
        _foreground->setScale(scale);
        _foresize.height /= scale.y;
    }
    
    _foreground->setPolygon(Rect(0,0,0,_foresize.height));
    _foreground->setContentSize(Size(0,_foresize.height));
    
    _foreground->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
    if (_begincap == nullptr) {
        _foreground->setPosition(-_background->getSize().width/2.0f,0);
    } else {
        _foreground->setPosition(_begincap->getBoundingBox().getMaxX(),0);
    }
    addChild(_foreground);
    
    // Now redo the position
    setPosition(coord);
    return true;
}


/**
 * Disposes all of the resources used by this node.
 *
 * A disposed progress bar can be safely reinitialized. Any children owned
 * by this node will be released.  They will be deleted if no other object
 * owns them.
 *
 * It is unsafe to call this on a progress bar that is still currently
 * inside of a scene graph.
 */
void ProgressBar::dispose() {
    _background = nullptr;
    _foreground = nullptr;
    _begincap = nullptr;
    _finalcap = nullptr;
    Node::dispose();
}

#pragma mark -
#pragma mark Properties
/**
 * Sets the percentage progress of this progress bar
 *
 * This value is a float between 0 and 1. Changing this value will alter
 * the size of the progress bar foreground.
 *
 * @param progress  The percentage progress of this progress bar
 */
void ProgressBar::setProgress(float progress) {
    _progress = progress;
    
    _foreground->setPolygon(Rect(0,0,progress*_foresize.width,_foresize.height));
    _foreground->setContentSize(Size(progress*_foresize.width,_foresize.height));
    if (_finalcap != nullptr) {
        _finalcap->setPosition(_foreground->getBoundingBox().getMaxX(),0);
    }
}

/**
 * Sets the foreground color or tint of the progress bar
 *
 * This is the color applied to the foreground texture (and end caps) if it
 * exists, or the color of the foreground rectangle. If there is a texture
 * it is white by default.  Otherwise it is red by default.
 *
 * @param color The foreground color or tint of the progress bar
 */
void ProgressBar::setForegroundColor(Color4 color) {
    _foreground->setColor(color);
    if (_begincap != nullptr) {
        _begincap->setColor(color);
    }
    if (_finalcap != nullptr) {
        _finalcap->setColor(color);
    }
}
