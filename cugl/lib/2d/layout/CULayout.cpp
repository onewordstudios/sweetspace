//
//  CULayout.cpp
//  Cornell University Game Library (CUGL)
//
//  This module provides an abstract class interface for Java-style Layout
//  managers.  This gives us more flexibility for creating scene graphs
//  on different resolution devices.
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
#include <cugl/2d/layout/CULayout.h>

using namespace cugl;
/**
 * Returns the anchor for the given text values
 *
 * This method is used to get an anchor object from a JSON directory.  The
 * x_anchor should be one of 'left', 'center', 'right', or 'fill'.  The
 * y_anchor should be one of 'bottom', 'middle', 'top', or 'fill'.
 *
 * @param x_anchor  The horizontal anchor setting
 * @param y_anchor  The vertical anchor setting
 *
 * @return the anchor for the given text values
 */
Layout::Anchor Layout::getAnchor(const std::string& x_anchor, const std::string& y_anchor) {
    Anchor anchor = Anchor::CENTER;
    if (x_anchor == "left") {
        if (y_anchor == "top") {
            anchor = Anchor::TOP_LEFT;
        } else if (y_anchor == "bottom") {
            anchor = Anchor::BOTTOM_LEFT;
        } else if (y_anchor == "middle") {
            anchor = Anchor::MIDDLE_LEFT;
        } else {
            anchor = Anchor::LEFT_FILL;
        }
    } else if (x_anchor == "right") {
        if (y_anchor == "top") {
            anchor = Anchor::TOP_RIGHT;
        } else if (y_anchor == "bottom") {
            anchor = Anchor::BOTTOM_RIGHT;
        } else if (y_anchor == "middle") {
            anchor = Anchor::MIDDLE_RIGHT;
        } else {
            anchor = Anchor::RIGHT_FILL;
        }
    } else if (x_anchor == "center")  {
        if (y_anchor == "top") {
            anchor = Anchor::TOP_CENTER;
        } else if (y_anchor == "bottom") {
            anchor = Anchor::BOTTOM_CENTER;
        } else if (y_anchor == "middle") {
            anchor = Anchor::CENTER;
        } else {
            anchor = Anchor::CENTER_FILL;
        }
    } else if (x_anchor == "fill")  {
        if (y_anchor == "top") {
            anchor = Anchor::TOP_FILL;
        } else if (y_anchor == "bottom") {
            anchor = Anchor::BOTTOM_FILL;
        } else if (y_anchor == "middle") {
            anchor = Anchor::MIDDLE_FILL;
        } else {
            anchor = Anchor::TOTAL_FILL;
        }
    } else {
        anchor = Anchor::NONE;
    }
    
    return anchor;
}

/**
 * Repositions the given node according the rules of its anchor.
 *
 * The repositioning is done relative to bounds, not the parent node. This
 * allows us to apply anchors to a subregion, like we do in {@link GridLayout}.
 * The value offset should be in coordinates, and not percentages.
 *
 * @param node      The node to reposition
 * @param anchor    The anchor rule for this node
 * @param bounds    The area to compute the position from
 * @param offset    The offset from the computed anchor position
 */
void Layout::placeNode(Node* node, Anchor anchor, const Rect& bounds, const Vec2& offset) {
    Vec2 spot;
    Vec2 adjust = offset;
    switch (anchor) {
        case Anchor::TOP_LEFT:
            spot.set(0, bounds.size.height);
            break;
        case Anchor::MIDDLE_LEFT:
            spot.set(0, bounds.size.height/2.0f);
            break;
        case Anchor::TOP_CENTER:
            spot.set(bounds.size.width/2.0f, bounds.size.height);
            break;
        case Anchor::CENTER:
            spot.set(bounds.size.width/2.0f, bounds.size.height/2.0f);
            break;
        case Anchor::BOTTOM_CENTER:
            spot.set(bounds.size.width/2.0f, 0);
            break;
        case Anchor::TOP_RIGHT:
            spot.set(bounds.size.width, bounds.size.height);
            break;
        case Anchor::MIDDLE_RIGHT:
            spot.set(bounds.size.width, bounds.size.height/2.0f);
            break;
        case Anchor::BOTTOM_RIGHT:
            spot.set(bounds.size.width, 0);
            break;
        case Anchor::BOTTOM_LEFT:
        case Anchor::NONE:
            break;
        case Anchor::LEFT_FILL:
            spot.set(0,bounds.size.height*node->getAnchor().y);
            node->setContentHeight(bounds.size.height-2*adjust.y);
            adjust.y = 0;
            break;
        case Anchor::CENTER_FILL:
            spot.set(bounds.size.width/2.0f,bounds.size.height*node->getAnchor().y);
            node->setContentHeight(bounds.size.height-2*adjust.y);
            adjust.y = 0;
            break;
        case Anchor::RIGHT_FILL:
            spot.set(bounds.size.width,bounds.size.height*node->getAnchor().y);
            node->setContentHeight(bounds.size.height-2*adjust.y);
            adjust.y = 0;
            break;
        case Anchor::BOTTOM_FILL:
            spot.set(bounds.size.width*node->getAnchor().x,0);
            node->setContentWidth(bounds.size.width-2*adjust.x);
            adjust.x = 0;
            break;
        case Anchor::MIDDLE_FILL:
            spot.set(bounds.size.width*node->getAnchor().x,bounds.size.height/2.0f);
            node->setContentWidth(bounds.size.width-2*adjust.x);
            adjust.x = 0;
            break;
        case Anchor::TOP_FILL:
            spot.set(bounds.size.width*node->getAnchor().x,bounds.size.height);
            node->setContentWidth(bounds.size.width-2*adjust.x);
            adjust.x = 0;
            break;
        case Anchor::TOTAL_FILL:
            spot.set(bounds.size*node->getAnchor());
            node->setContentSize(bounds.size.width-2*adjust.x,bounds.size.height-2*adjust.y);
            adjust = Vec2::ZERO;
            break;
    }
    
    spot += bounds.origin+adjust;
    node->setPosition(spot);
}

/**
 * Resets the node anchor to agree with the layout anchor
 *
 * For some layout managers, the layout anchor (which is an enum) may
 * disagree with the node anchor (which is a percentage vector).  This
 * method allows a layout manager to "disable" the node anchor in favor
 * of the layout anchor.
 *
 * @param node      The node to reanchor
 * @param anchor    The layout anchor to use
 */
void Layout::reanchor(Node* node, Anchor anchor) {
    switch (anchor) {
        case Anchor::TOP_LEFT:
            node->setAnchor(Vec2::ANCHOR_TOP_LEFT);
            break;
        case Anchor::MIDDLE_LEFT:
            node->setAnchor(Vec2::ANCHOR_MIDDLE_LEFT);
            break;
        case Anchor::BOTTOM_LEFT:
            node->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
            break;
        case Anchor::TOP_CENTER:
            node->setAnchor(Vec2::ANCHOR_TOP_CENTER);
            break;
        case Anchor::CENTER:
            node->setAnchor(Vec2::ANCHOR_CENTER);
            break;
        case Anchor::BOTTOM_CENTER:
            node->setAnchor(Vec2::ANCHOR_BOTTOM_CENTER);
            break;
        case Anchor::TOP_RIGHT:
            node->setAnchor(Vec2::ANCHOR_TOP_RIGHT);
            break;
        case Anchor::MIDDLE_RIGHT:
            node->setAnchor(Vec2::ANCHOR_MIDDLE_RIGHT);
            break;
        case Anchor::BOTTOM_RIGHT:
            node->setAnchor(Vec2::ANCHOR_BOTTOM_RIGHT);
            break;
        case Anchor::LEFT_FILL:
            node->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
            break;
        case Anchor::CENTER_FILL:
            node->setAnchor(Vec2::ANCHOR_BOTTOM_CENTER);
            break;
        case Anchor::RIGHT_FILL:
            node->setAnchor(Vec2::ANCHOR_BOTTOM_RIGHT);
            break;
        case Anchor::BOTTOM_FILL:
            node->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
            break;
        case Anchor::MIDDLE_FILL:
            node->setAnchor(Vec2::ANCHOR_MIDDLE_LEFT);
            break;
        case Anchor::TOP_FILL:
            node->setAnchor(Vec2::ANCHOR_TOP_LEFT);
            break;
        case Anchor::TOTAL_FILL:
            node->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
            break;
        case Anchor::NONE:
            break;
    }
    
}

