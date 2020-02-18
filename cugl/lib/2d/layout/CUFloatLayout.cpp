//
//  CUFloatLayout.cpp
//  Cornell University Game Library (CUGL)
//
//  This module provides an support for a float layout.  Children in a float
//  layout are arranged in order, according to the layout orientation (horizontal
//  or vertical).  If there is not enough space in the Node for the children
//  to all be in the same row or column (depending on orientation), then the
//  later children wrap around to a new row or column.  This is the same way
//  that float layouts work in Java.
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
//  Author: Walker White and Enze Zhou
//  Version: 1/8/18
//
#include <cugl/2d/layout/CUFloatLayout.h>
#include <algorithm>

using namespace cugl;

#define UNKNOWN_STR "<unknown>"

#pragma mark Constructors
/**
 * Creates a degenerate layout manager with no data.
 *
 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
 * the heap, use one of the static constructors instead.
 */
FloatLayout::FloatLayout() :
_alignment(Alignment::TOP_LEFT),
_horizontal(true) {
}

/**
 * Initializes a new layout manager with the given JSON specificaton.
 *
 * In addition to the 'type' attribute (which must be "float"), the JSON
 * specification supports the following attribute values:
 *
 *      "orientation":  One of 'horizontal' or 'vertical'
 *      "x_alignment":  One of 'left', 'center', or 'right'
 *      "y_alignment":  One of 'bottom', 'middle', or 'top'
 *
 * All attributes other than 'type' are optional.
 *
 * @param data      The JSON object specifying the node
 *
 * @return true if initialization was successful.
 */
bool FloatLayout::initWithData(const std::shared_ptr<JsonValue>& data) {
    std::string orient = data->getString("orientation",UNKNOWN_STR);
    _horizontal = !(orient == "vertical");
    
    std::string horz = data->getString("x_alignment","middle");
    std::string vert = data->getString("y_alignment","middle");
    _alignment = Alignment::TOP_LEFT;
    if (horz == "left") {
        if (vert == "top") {
            _alignment = Alignment::TOP_LEFT;
        } else if (vert == "bottom") {
            _alignment = Alignment::BOTTOM_LEFT;
        } else {
            _alignment = Alignment::MIDDLE_LEFT;
        }
    } else if (horz == "right") {
        if (vert == "top") {
            _alignment = Alignment::TOP_RIGHT;
        } else if (vert == "bottom") {
            _alignment = Alignment::BOTTOM_RIGHT;
        } else {
            _alignment = Alignment::MIDDLE_RIGHT;
        }
    } else if (horz == "center")  {
        if (vert == "top") {
            _alignment = Alignment::TOP_CENTER;
        } else if (vert == "bottom") {
            _alignment = Alignment::BOTTOM_CENTER;
        } else {
            _alignment = Alignment::CENTER;
        }
    }
    return true;
}


#pragma mark -
#pragma mark Layout
/**
 * Assigns layout information for a given key.
 *
 * The JSON object may contain any of the following attribute value:
 *
 *      "priority":     An int indicating placement priority.
 *                      Children with lower priority go first.
 *
 * A child with no priority is put at the end. If there is already a child with
 * the given priority, then this method will fail.
 *
 * To look up the layout information of a scene graph node, we use the name
 * of the node.  This requires all nodes to have unique names. The
 * {@link SceneLoader} prefixes all child names by the parent name, so
 * this is the case in any well-defined JSON file. If the key is already
 * in use, this method will fail.
 *
 * @param key   The key identifying the layout information
 * @param data  A JSON object with the layout information
 *
 * @return true if the layout information was assigned to that key
 */
bool FloatLayout::add(const std::string key, const std::shared_ptr<JsonValue>& data) {
    long priority = data->getLong("priority",0L);
    CUAssertLog(priority >= 0, "'priority' may not be negative");
    return addPriority(key, priority);
}

/**
 * Assigns the layout priority for a given key.
 *
 * In a float layout, children with lower priority go first. If there is
 * already a child with the given priority, then this method will fail.
 *
 * To look up the layout information of a scene graph node, we use the name
 * of the node.  This requires all nodes to have unique names. The
 * {@link SceneLoader} prefixes all child names by the parent name, so
 * this is the case in any well-defined JSON file. If the key is already
 * in use, this method will fail.
 *
 * @param key       The key identifying the layout information
 * @param priority  The priority (lower is better) for this key
 *
 * @return true if the priority was assigned to that key
 */
bool FloatLayout::addPriority(const std::string key, size_t priority) {
    while (priority+1 > _priority.size()) {
        _priority.push_back("");
    }
    
    if (_priority[priority] != "") {
        return false;
    }
    
    auto it = _keyset.find(key);
    if (it != _keyset.end()) {
        return false;
    }
    
    _priority[priority] = key;
    _keyset[key] = priority;
    return true;
}

/**
 * Removes the layout information for a given key.
 *
 * To look up the layout information of a scene graph node, we use the name
 * of the node.  This requires all nodes to have unique names. The
 * {@link SceneLoader} prefixes all child names by the parent name, so
 * this is the case in any well-defined JSON file.
 *
 * If the key is not in use, this method will fail.
 *
 * @param key   The key identifying the layout information
 *
 * @return true if the layout information was removed for that key
 */
bool FloatLayout::remove(const std::string key) {
    auto it = _keyset.find(key);
    if (it == _keyset.end()) {
        return false;
    }
    
    _priority[it->second] = "";
    _keyset.erase(it);
    return true;
}

/**
 * Performs a layout on the given node.
 *
 * This layout manager will searches for those children that are registered
 * with it. For those children, it repositions and/or resizes them according
 * to the layout information.
 *
 * This manager attaches a child node to one of nine "anchors" in the parent
 * (corners, sides or middle), together with a percentage (or absolute)
 * offset.  As the parent grows or shinks, the child will move according to
 * its anchor.  For example, nodes in the center will stay centered, while
 * nodes on the left side will move to keep the appropriate distance from
 * the left side. In fact, the stretching behavior is very similar to that
 * of a {@link NinePatch}.
 *
 * To look up the layout information of a scene graph node, this method uses
 * the name of the node.  This requires all nodes to have unique names. The
 * {@link SceneLoader} prefixes all child names by the parent name, so
 * this is the case in any well-defined JSON file.
 *
 * Children not registered with this layout manager are not affected.
 *
 * @param node  The scene graph node to rearrange
 */
void FloatLayout::layout(Node* node) {
    if (_horizontal) {
        layoutHorizontal(node);
    } else {
        layoutVertical(node);
    }
}

#pragma mark -
#pragma mark Internal Helpers
/**
 * Performs a horizontal layout on the given node.
 *
 * This method is identical to {@link layout(Node*)} except that it overrides
 * the orientation settings of the layout manager; it always lays out the
 * children horizontally.
 *
 * @param node  The scene graph node to rearrange
 */
void FloatLayout::layoutHorizontal(Node* node) {
    Size size = node->getContentSize();
    Vec2 pos;
    Rect bounds;
    
    // Running calculations of size
    std::vector<float> height;
    std::vector<float> width;
    std::vector<int> count;
    width.push_back(0.0f);
    height.push_back(0.0f);
    count.push_back(0);
    
    // Get the bounding box for the contents (ignoring alignment for now)
    bool stop = false;

    for(auto it = _priority.begin(); !stop && it != _priority.end(); ++it) {
        std::shared_ptr<Node> child;
        if (*it != "") {
            child = node->getChildByName(*it);
        }
        if (child) {
            Size extra = child->getSize();
            if (extra.width > size.width) {
                stop = true;
            } else if (width.back()+extra.width > size.width) {
                if (height.back()+extra.height > size.height) {
                    stop = true;
                } else {
                    bounds.size.width = std::max(bounds.size.width,width.back());
                    bounds.size.height += height.back();
                    width.push_back(extra.width);
                    height.push_back(extra.height);
                    count.push_back(1);
                }
            } else {
                width.back() += extra.width;
                height.back() = std::max(extra.height,height.back());
                count.back()++;
            }
        }
    }
    
    // Record the last
    bounds.size.width = std::max(bounds.size.width,width.back());
    bounds.size.height += height.back();
    
    // Now do layout.
    switch(_alignment) {
        case Alignment::BOTTOM_LEFT:
        bounds.origin = Vec2::ZERO;
        break;
        case Alignment::BOTTOM_CENTER:
        bounds.origin = Vec2((size.width-bounds.size.width)/2,0);
        break;
        case Alignment::BOTTOM_RIGHT:
        bounds.origin = Vec2(size.width-bounds.size.width/2,0);
        break;
        case Alignment::MIDDLE_LEFT:
        bounds.origin = Vec2(0,(size.height-bounds.size.height)/2);
        break;
        case Alignment::CENTER:
        bounds.origin = Vec2((size.width-bounds.size.width)/2,(size.height-bounds.size.height)/2);
        break;
        case Alignment::MIDDLE_RIGHT:
        bounds.origin = Vec2(size.width-bounds.size.width,(size.height-bounds.size.height)/2);
        break;
        case Alignment::TOP_LEFT:
        bounds.origin = Vec2(0,size.height-bounds.size.height);
        break;
        case Alignment::TOP_CENTER:
        bounds.origin = Vec2((size.width-bounds.size.width)/2,size.height-bounds.size.height);
        break;
        case Alignment::TOP_RIGHT:
        bounds.origin = Vec2(size.width-bounds.size.width,size.height-bounds.size.height);
        break;
    }
    
    float ypos;
    switch(_alignment) {
    case Alignment::BOTTOM_LEFT:
    case Alignment::BOTTOM_CENTER:
    case Alignment::BOTTOM_RIGHT:
        ypos = bounds.size.height;
        break;
    case Alignment::MIDDLE_LEFT:
    case Alignment::CENTER:
    case Alignment::MIDDLE_RIGHT:
        ypos = (size.height+bounds.size.height)/2;
        break;
    case Alignment::TOP_LEFT:
    case Alignment::TOP_CENTER:
    case Alignment::TOP_RIGHT:
        ypos = size.height;
        break;
    }
    
    auto jt = _priority.begin();
    for(size_t row = 0; row < count.size(); row++) {
        Vec2 pos; // Top LEFT corner of float.
        float xpos;
        switch(_alignment) {
        case Alignment::BOTTOM_LEFT:
        case Alignment::MIDDLE_LEFT:
        case Alignment::TOP_LEFT:
            xpos = 0;
            break;
        case Alignment::BOTTOM_CENTER:
        case Alignment::CENTER:
        case Alignment::TOP_CENTER:
            xpos = (size.width-width[row])/2;
            break;
        case Alignment::BOTTOM_RIGHT:
        case Alignment::MIDDLE_RIGHT:
        case Alignment::TOP_RIGHT:
            xpos = size.width-width[row];
            break;
        }
        for(size_t col = 0; col < count[row]; col++) {
            std::shared_ptr<Node> child;
            if (*jt != "") {
                child = node->getChildByName(*jt);
            }
            if (child) {
                Size tmp = child->getSize();
                switch(_alignment) {
                case Alignment::BOTTOM_LEFT:
                    child->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
                    child->setPosition(xpos, ypos-height[row]);
                    break;
                case Alignment::BOTTOM_CENTER:
                    child->setAnchor(Vec2::ANCHOR_BOTTOM_CENTER);
                    child->setPosition(xpos+tmp.width/2, ypos-height[row]);
                    break;
                case Alignment::BOTTOM_RIGHT:
                    child->setAnchor(Vec2::ANCHOR_BOTTOM_RIGHT);
                    child->setPosition(xpos+tmp.width, ypos-height[row]);
                    break;
                case Alignment::MIDDLE_LEFT:
                    child->setAnchor(Vec2::ANCHOR_MIDDLE_LEFT);
                    child->setPosition(xpos, ypos-height[row]/2);
                    break;
                case Alignment::CENTER:
                    child->setAnchor(Vec2::ANCHOR_CENTER);
                    child->setPosition(xpos+tmp.width/2, ypos-height[row]/2);
                    break;
                case Alignment::MIDDLE_RIGHT:
                    child->setAnchor(Vec2::ANCHOR_MIDDLE_RIGHT);
                    child->setPosition(xpos+tmp.width, ypos-height[row]/2);
                    break;
                case Alignment::TOP_LEFT:
                    child->setAnchor(Vec2::ANCHOR_TOP_LEFT);
                    child->setPosition(xpos, ypos);
                    break;
                case Alignment::TOP_CENTER:
                    child->setAnchor(Vec2::ANCHOR_TOP_CENTER);
                    child->setPosition(xpos+tmp.width/2, ypos);
                    break;
                case Alignment::TOP_RIGHT:
                    child->setAnchor(Vec2::ANCHOR_TOP_RIGHT);
                    child->setPosition(xpos+tmp.width, ypos);
                    break;
                }
                xpos += tmp.width;
            }
            ++jt;
        }
        ypos -= height[row];
    }
}

/**
 * Performs a vertical layout on the given node.
 *
 * This method is identical to {@link layout(Node*)} except that it overrides
 * the orientation settings of the layout manager; it always lays out the
 * children vertically.
 *
 * @param node  The scene graph node to rearrange
 */
void FloatLayout::layoutVertical(Node* node) {
    Size size = node->getContentSize();
    Vec2 pos;
    Rect bounds;
    
    // Running calculations of size
    std::vector<float> height;
    std::vector<float> width;
    std::vector<int> count;
    width.push_back(0.0f);
    height.push_back(0.0f);
    count.push_back(0);
    
    // Get the bounding box for the contents (ignoring alignment for now)
    bool stop = false;
    for(auto it = _priority.begin(); !stop && it != _priority.end(); ++it) {
        std::shared_ptr<Node> child;
        if (*it != "") {
            child = node->getChildByName(*it);
        }
        if (child) {
            Size extra = child->getSize();
            if (extra.height > size.height) {
                stop = true;
            } else if (height.back()+extra.height > size.height) {
                if (width.back()+extra.width > size.width) {
                    stop = true;
                } else {
                    bounds.size.height = std::max(bounds.size.height,height.back());
                    bounds.size.width += width.back();
                    width.push_back(extra.width);
                    height.push_back(extra.height);
                    count.push_back(1);
                }
            } else {
                height.back() += extra.height;
                width.back() = std::max(extra.width,width.back());
                count.back()++;
            }
        }
    }
    
    // Record the last
    bounds.size.height = std::max(bounds.size.height,height.back());
    bounds.size.width += width.back();
    
    // Now do layout.
    switch(_alignment) {
        case Alignment::BOTTOM_LEFT:
        bounds.origin = Vec2::ZERO;
        break;
        case Alignment::BOTTOM_CENTER:
        bounds.origin = Vec2((size.width-bounds.size.width)/2,0);
        break;
        case Alignment::BOTTOM_RIGHT:
        bounds.origin = Vec2(size.width-bounds.size.width/2,0);
        break;
        case Alignment::MIDDLE_LEFT:
        bounds.origin = Vec2(0,(size.height-bounds.size.height)/2);
        break;
        case Alignment::CENTER:
        bounds.origin = Vec2((size.width-bounds.size.width)/2,(size.height-bounds.size.height)/2);
        break;
        case Alignment::MIDDLE_RIGHT:
        bounds.origin = Vec2(size.width-bounds.size.width,(size.height-bounds.size.height)/2);
        break;
        case Alignment::TOP_LEFT:
        bounds.origin = Vec2(0,size.height-bounds.size.height);
        break;
        case Alignment::TOP_CENTER:
        bounds.origin = Vec2((size.width-bounds.size.width)/2,size.height-bounds.size.height);
        break;
        case Alignment::TOP_RIGHT:
        bounds.origin = Vec2(size.width-bounds.size.width,size.height-bounds.size.height);
        break;
    }
    
    float xpos;
    switch(_alignment) {
        case Alignment::BOTTOM_LEFT:
        case Alignment::MIDDLE_LEFT:
        case Alignment::TOP_LEFT:
        xpos = 0;
        break;
        case Alignment::BOTTOM_CENTER:
        case Alignment::CENTER:
        case Alignment::TOP_CENTER:
        xpos = (size.width-bounds.size.width)/2;
        break;
        case Alignment::BOTTOM_RIGHT:
        case Alignment::MIDDLE_RIGHT:
        case Alignment::TOP_RIGHT:
        xpos = size.width-bounds.size.width;
        break;
    }
    
    auto jt = _priority.begin();
    for(size_t col = 0; col < count.size(); col++) {
        Vec2 pos; // Top LEFT corner of float.
        float ypos;
        switch(_alignment) {
            case Alignment::BOTTOM_LEFT:
            case Alignment::BOTTOM_CENTER:
            case Alignment::BOTTOM_RIGHT:
            ypos = height[col];
            break;
            case Alignment::MIDDLE_LEFT:
            case Alignment::CENTER:
            case Alignment::MIDDLE_RIGHT:
            ypos = (size.height+height[col])/2;
            break;
            case Alignment::TOP_LEFT:
            case Alignment::TOP_CENTER:
            case Alignment::TOP_RIGHT:
            ypos = size.height;
            break;
        }
        for(size_t row = 0; row < count[col]; row++) {
            std::shared_ptr<Node> child;
            if (*jt != "") {
                child = node->getChildByName(*jt);
            }
            if (child) {
                Size tmp = child->getSize();
                switch(_alignment) {
                    case Alignment::BOTTOM_LEFT:
                    child->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
                    child->setPosition(xpos, ypos-tmp.height);
                    break;
                    case Alignment::BOTTOM_CENTER:
                    child->setAnchor(Vec2::ANCHOR_BOTTOM_CENTER);
                    child->setPosition(xpos+tmp.width/2, ypos-tmp.height);
                    break;
                    case Alignment::BOTTOM_RIGHT:
                    child->setAnchor(Vec2::ANCHOR_BOTTOM_RIGHT);
                    child->setPosition(xpos+tmp.width, ypos-tmp.height);
                    break;
                    case Alignment::MIDDLE_LEFT:
                    child->setAnchor(Vec2::ANCHOR_MIDDLE_LEFT);
                    child->setPosition(xpos, ypos-tmp.height/2);
                    break;
                    case Alignment::CENTER:
                    child->setAnchor(Vec2::ANCHOR_CENTER);
                    child->setPosition(xpos+tmp.width/2, ypos-tmp.height/2);
                    break;
                    case Alignment::MIDDLE_RIGHT:
                    child->setAnchor(Vec2::ANCHOR_MIDDLE_RIGHT);
                    child->setPosition(xpos+tmp.width, ypos-tmp.height/2);
                    break;
                    case Alignment::TOP_LEFT:
                    child->setAnchor(Vec2::ANCHOR_TOP_LEFT);
                    child->setPosition(xpos, ypos);
                    break;
                    case Alignment::TOP_CENTER:
                    child->setAnchor(Vec2::ANCHOR_TOP_CENTER);
                    child->setPosition(xpos+tmp.width/2, ypos);
                    break;
                    case Alignment::TOP_RIGHT:
                    child->setAnchor(Vec2::ANCHOR_TOP_RIGHT);
                    child->setPosition(xpos+tmp.width, ypos);
                    break;
                }
                ypos -= tmp.height;
            }
            ++jt;
        }
        xpos += width[col];
    }
}
