//
//  TCU2DTest.cpp
//  CUGL
//
//  Created by Walker White on 6/26/16.
//  Copyright © 2016 Game Design Initiative at Cornell. All rights reserved.
//

#include "TCU2DTest.h"
#include <string>
#include "CUDebug.h"
#include "CUStrings.h"
#include "CUNode.h"
#include <chrono>

/** Data type for timestamp support */
typedef std::chrono::high_resolution_clock::time_point timestamp_t;

namespace cugl {

#pragma mark -
#pragma mark Node
    
void testNode() {
    CULog("Running tests for Node.\n");

#pragma mark Basic Constructor Test
    
    Node test1;
    Mat4 mtest;
    CUAssertLog(test1.getPosition() == Vec2::ZERO,          "Constructor failed");
    CUAssertLog(test1.getContentSize() == Size::ZERO,       "Constructor failed");
    CUAssertLog(test1.getAnchor() == Vec2::ANCHOR_CENTER,   "Constructor failed");
    CUAssertLog(test1.getTag() == 0,                        "Constructor failed");
    CUAssertLog(test1.getName() == "",                      "Constructor failed");
    CUAssertLog(test1.getColor() == Color4::WHITE,          "Constructor failed");
    CUAssertLog(test1.hasRelativeColor(),                   "Constructor failed");
    CUAssertLog(test1.isVisible(),                          "Constructor failed");
    CUAssertLog(test1.getScale() == Vec2::ONE,              "Constructor failed");
    CUAssertLog(test1.getAngle() == 0,                      "Constructor failed");
    CUAssertLog(test1.getNodeToParentTransform() == mtest,  "Constructor failed");
    CUAssertLog(test1.getParent() == nullptr,               "Constructor failed");
    CUAssertLog(test1.getChildCount() == 0,                 "Constructor failed");
    CUAssertLog(test1.getZOrder() == 0,                     "Constructor failed");
    CUAssertLog(!test1. withAlternateTransform(),           "Constructor failed");
    CUAssertLog(test1.getAlternateTransform() == Mat4::IDENTITY,
                "Constructor failed");

#pragma mark Basic Initializer Test
    
    Mat4::createTranslation(1, 2, 0, &mtest);
    CUAssertLog(test1.initWithPosition(Vec2(1,2)),          "Method initWithPosition() failed");
    CUAssertLog(test1.getPosition() == Vec2(1,2),           "Method initWithPosition() failed");
    CUAssertLog(test1.getContentSize() == Size::ZERO,       "Method initWithPosition() failed");
    CUAssertLog(test1.getAnchor() == Vec2::ANCHOR_CENTER,   "Method initWithPosition() failed");
    CUAssertLog(test1.getTag() == 0,                        "Method initWithPosition() failed");
    CUAssertLog(test1.getName() == "",                      "Method initWithPosition() failed");
    CUAssertLog(test1.getColor() == Color4::WHITE,          "Method initWithPosition() failed");
    CUAssertLog(test1.hasRelativeColor(),                   "Method initWithPosition() failed");
    CUAssertLog(test1.isVisible(),                          "Method initWithPosition() failed");
    CUAssertLog(test1.getScale() == Vec2::ONE,              "Method initWithPosition() failed");
    CUAssertLog(test1.getAngle() == 0,                      "Method initWithPosition() failed");
    CUAssertLog(test1.getParent() == nullptr,               "Method initWithPosition() failed");
    CUAssertLog(test1.getChildCount() == 0,                 "Method initWithPosition() failed");
    CUAssertLog(test1.getZOrder() == 0,                     "Method initWithPosition() failed");
    CUAssertLog(test1.getNodeToParentTransform() == mtest,  "Method initWithPosition() failed");
    CUAssertLog(test1.getWorldPosition() == Vec2(1,2),      "Method initWithPosition() failed");
    CUAssertLog(test1.getBoundingBox() == Rect(1,2,0,0),    "Method initWithPosition() failed");
    CUAssertLog(!test1. withAlternateTransform(),           "Method initWithPosition() failed");
    CUAssertLog(test1.getAlternateTransform() == Mat4::IDENTITY,
                "Method initWithPosition() failed");

    test1.dispose(); mtest = Mat4::IDENTITY;
    CUAssertLog(test1.getPosition() == Vec2::ZERO,          "Method dispose() failed");
    CUAssertLog(test1.getContentSize() == Size::ZERO,       "Method dispose() failed");
    CUAssertLog(test1.getAnchor() == Vec2::ANCHOR_CENTER,   "Method dispose() failed");
    CUAssertLog(test1.getTag() == 0,                        "Method dispose() failed");
    CUAssertLog(test1.getName() == "",                      "Method dispose() failed");
    CUAssertLog(test1.getColor() == Color4::WHITE,          "Method dispose() failed");
    CUAssertLog(test1.hasRelativeColor(),                   "Method dispose() failed");
    CUAssertLog(test1.isVisible(),                          "Method dispose() failed");
    CUAssertLog(test1.getScale() == Vec2::ONE,              "Method dispose() failed");
    CUAssertLog(test1.getAngle() == 0,                      "Method dispose() failed");
    CUAssertLog(test1.getParent() == nullptr,               "Method dispose() failed");
    CUAssertLog(test1.getChildCount() == 0,                 "Method dispose() failed");
    CUAssertLog(test1.getZOrder() == 0,                     "Method dispose() failed");
    CUAssertLog(test1.getNodeToParentTransform() == mtest,  "Method dispose() failed");
    CUAssertLog(test1.getBoundingBox() == Rect(0,0,0,0),    "Method dispose() failed");
    CUAssertLog(!test1. withAlternateTransform(),           "Method dispose() failed");
    CUAssertLog(test1.getAlternateTransform() == Mat4::IDENTITY,
                "Method dispose() failed");

    CUAssertLog(test1.initWithBounds(Size(1,2)),            "Method initWithBounds() failed");
    CUAssertLog(test1.getPosition() == Vec2(0.5f,1),        "Method initWithBounds() failed");
    CUAssertLog(test1.getContentSize() == Size(1,2),        "Method initWithBounds() failed");
    CUAssertLog(test1.getAnchor() == Vec2::ANCHOR_CENTER,   "Method initWithBounds() failed");
    CUAssertLog(test1.getTag() == 0,                        "Method initWithBounds() failed");
    CUAssertLog(test1.getName() == "",                      "Method initWithBounds() failed");
    CUAssertLog(test1.getColor() == Color4::WHITE,          "Method initWithBounds() failed");
    CUAssertLog(test1.hasRelativeColor(),                   "Method initWithBounds() failed");
    CUAssertLog(test1.isVisible(),                          "Method initWithBounds() failed");
    CUAssertLog(test1.getScale() == Vec2::ONE,              "Method initWithBounds() failed");
    CUAssertLog(test1.getAngle() == 0,                      "Method initWithBounds() failed");
    CUAssertLog(test1.getParent() == nullptr,               "Method initWithBounds() failed");
    CUAssertLog(test1.getChildCount() == 0,                 "Method initWithBounds() failed");
    CUAssertLog(test1.getZOrder() == 0,                     "Method initWithBounds() failed");
    CUAssertLog(test1.getNodeToParentTransform() == mtest,  "Method initWithBounds() failed");
    CUAssertLog(test1.getWorldPosition() == Vec2(0.5f,1),   "Method initWithBounds() failed");
    CUAssertLog(test1.getBoundingBox() == Rect(0,0,1,2),    "Method initWithBounds() failed");
    CUAssertLog(!test1. withAlternateTransform(),           "Method initWithBounds() failed");
    CUAssertLog(test1.getAlternateTransform() == Mat4::IDENTITY,
                "Method initWithBounds() failed");

    test1.dispose();
    Mat4::createTranslation(1, 2, 0, &mtest);
    CUAssertLog(test1.initWithBounds(Rect(1,2,3,4)),        "Method initWithBounds() failed");
    CUAssertLog(test1.getPosition() == Vec2(2.5f, 4),       "Method initWithBounds() failed");
    CUAssertLog(test1.getContentSize() == Size(3,4),        "Method initWithBounds() failed");
    CUAssertLog(test1.getAnchor() == Vec2::ANCHOR_CENTER,   "Method initWithBounds() failed");
    CUAssertLog(test1.getTag() == 0,                        "Method initWithBounds() failed");
    CUAssertLog(test1.getName() == "",                      "Method initWithBounds() failed");
    CUAssertLog(test1.getColor() == Color4::WHITE,          "Method initWithBounds() failed");
    CUAssertLog(test1.hasRelativeColor(),                   "Method initWithBounds() failed");
    CUAssertLog(test1.isVisible(),                          "Method initWithBounds() failed");
    CUAssertLog(test1.getScale() == Vec2::ONE,              "Method initWithBounds() failed");
    CUAssertLog(test1.getAngle() == 0,                      "Method initWithBounds() failed");
    CUAssertLog(test1.getParent() == nullptr,               "Method initWithBounds() failed");
    CUAssertLog(test1.getChildCount() == 0,                 "Method initWithBounds() failed");
    CUAssertLog(test1.getZOrder() == 0,                     "Method initWithBounds() failed");
    CUAssertLog(test1.getNodeToParentTransform() == mtest,  "Method initWithBounds() failed");
    CUAssertLog(test1.getBoundingBox() == Rect(1,2,3,4),    "Method initWithBounds() failed");
    CUAssertLog(!test1. withAlternateTransform(),           "Method initWithBounds() failed");
    CUAssertLog(test1.getAlternateTransform() == Mat4::IDENTITY,
                "Method initWithBounds() failed");

#pragma mark Static Constructor Test

    std::shared_ptr<Node> testptr1 = Node::alloc();
    CUAssertLog(testptr1->getPosition() == Vec2::ZERO,          "Node::create() failed");
    CUAssertLog(testptr1->getContentSize() == Size::ZERO,       "Node::create() failed");
    CUAssertLog(testptr1->getAnchor() == Vec2::ANCHOR_CENTER,   "Node::create() failed");
    CUAssertLog(testptr1->getTag() == 0,                        "Node::create() failed");
    CUAssertLog(testptr1->getName() == "",                      "Node::create() failed");
    CUAssertLog(testptr1->getColor() == Color4::WHITE,          "Node::create() failed");
    CUAssertLog(testptr1->hasRelativeColor(),                   "Node::create() failed");
    CUAssertLog(testptr1->isVisible(),                          "Node::create() failed");
    CUAssertLog(testptr1->getScale() == Vec2::ONE,              "Node::create() failed");
    CUAssertLog(testptr1->getAngle() == 0,                      "Node::create() failed");
    CUAssertLog(testptr1->getParent() == nullptr,               "Node::create() failed");
    CUAssertLog(testptr1->getChildCount() == 0,                 "Node::create() failed");
    CUAssertLog(testptr1->getZOrder() == 0,                     "Node::create() failed");
    CUAssertLog(!testptr1-> withAlternateTransform(),           "Node::create() failed");
    CUAssertLog(testptr1->getAlternateTransform() == Mat4::IDENTITY,
                "Node::create() failed");
    testptr1.reset();
    
    testptr1 = Node::allocWithPosition(Vec2(1,2));
    CUAssertLog(testptr1->getPosition() == Vec2(1,2),           "Node::createWithPosition() failed");
    CUAssertLog(testptr1->getContentSize() == Size::ZERO,       "Node::createWithPosition() failed");
    CUAssertLog(testptr1->getAnchor() == Vec2::ANCHOR_CENTER,   "Node::createWithPosition() failed");
    CUAssertLog(testptr1->getTag() == 0,                        "Node::createWithPosition() failed");
    CUAssertLog(testptr1->getName() == "",                      "Node::createWithPosition() failed");
    CUAssertLog(testptr1->getColor() == Color4::WHITE,          "Node::createWithPosition() failed");
    CUAssertLog(testptr1->hasRelativeColor(),                   "Node::createWithPosition() failed");
    CUAssertLog(testptr1->isVisible(),                          "Node::createWithPosition() failed");
    CUAssertLog(testptr1->getScale() == Vec2::ONE,              "Node::createWithPosition() failed");
    CUAssertLog(testptr1->getAngle() == 0,                      "Node::createWithPosition() failed");
    CUAssertLog(testptr1->getParent() == nullptr,               "Node::createWithPosition() failed");
    CUAssertLog(testptr1->getChildCount() == 0,                 "Node::createWithPosition() failed");
    CUAssertLog(testptr1->getZOrder() == 0,                     "Node::createWithPosition() failed");
    CUAssertLog(!testptr1-> withAlternateTransform(),           "Node::createWithPosition() failed");
    CUAssertLog(testptr1->getAlternateTransform() == Mat4::IDENTITY,
                "Node::createWithPosition() failed");
    testptr1.reset();
    
    testptr1 = Node::allocWithBounds(Size(3,4));
    CUAssertLog(testptr1->getPosition() == Vec2(1.5f,2),        "Node::createWithBounds() failed");
    CUAssertLog(testptr1->getContentSize() == Size(3,4),        "Node::createWithBounds() failed");
    CUAssertLog(testptr1->getAnchor() == Vec2::ANCHOR_CENTER,   "Node::createWithBounds() failed");
    CUAssertLog(testptr1->getTag() == 0,                        "Node::createWithBounds() failed");
    CUAssertLog(testptr1->getName() == "",                      "Node::createWithBounds() failed");
    CUAssertLog(testptr1->getColor() == Color4::WHITE,          "Node::createWithBounds() failed");
    CUAssertLog(testptr1->hasRelativeColor(),                   "Node::createWithBounds() failed");
    CUAssertLog(testptr1->isVisible(),                          "Node::createWithBounds() failed");
    CUAssertLog(testptr1->getScale() == Vec2::ONE,              "Node::createWithBounds() failed");
    CUAssertLog(testptr1->getAngle() == 0,                      "Node::createWithBounds() failed");
    CUAssertLog(testptr1->getParent() == nullptr,               "Node::createWithBounds() failed");
    CUAssertLog(testptr1->getChildCount() == 0,                 "Node::createWithBounds() failed");
    CUAssertLog(testptr1->getZOrder() == 0,                     "Node::createWithBounds() failed");
    CUAssertLog(!testptr1-> withAlternateTransform(),           "Node::createWithBounds() failed");
    CUAssertLog(testptr1->getAlternateTransform() == Mat4::IDENTITY,
                "Node::createWithBounds() failed");
    testptr1.reset();

    testptr1 = Node::allocWithBounds(Rect(1,2,3,4));
    CUAssertLog(testptr1->getPosition() == Vec2(2.5,4),         "Node::createWithBounds() failed");
    CUAssertLog(testptr1->getContentSize() == Size(3,4),        "Node::createWithBounds() failed");
    CUAssertLog(testptr1->getAnchor() == Vec2::ANCHOR_CENTER,   "Node::createWithBounds() failed");
    CUAssertLog(testptr1->getTag() == 0,                        "Node::createWithBounds() failed");
    CUAssertLog(testptr1->getName() == "",                      "Node::createWithBounds() failed");
    CUAssertLog(testptr1->getColor() == Color4::WHITE,          "Node::createWithBounds() failed");
    CUAssertLog(testptr1->hasRelativeColor(),                   "Node::createWithBounds() failed");
    CUAssertLog(testptr1->isVisible(),                          "Node::createWithBounds() failed");
    CUAssertLog(testptr1->getScale() == Vec2::ONE,              "Node::createWithBounds() failed");
    CUAssertLog(testptr1->getAngle() == 0,                      "Node::createWithBounds() failed");
    CUAssertLog(testptr1->getParent() == nullptr,               "Node::createWithBounds() failed");
    CUAssertLog(testptr1->getChildCount() == 0,                 "Node::createWithBounds() failed");
    CUAssertLog(testptr1->getZOrder() == 0,                     "Node::createWithBounds() failed");
    CUAssertLog(!testptr1-> withAlternateTransform(),           "Node::createWithBounds() failed");
    CUAssertLog(testptr1->getAlternateTransform() == Mat4::IDENTITY,
                "Node::createWithBounds() failed");
    testptr1.reset();
    
#pragma mark Identifier Test
    test1.setTag(4);
    CUAssertLog(test1.getTag() == 4,                        "Method getTag() failed");

    test1.setName("fred");
    CUAssertLog(test1.getName() == "fred",                  "Method getTag() failed");

    std::string str1;
    std::string str2 = "(tag:";
    str2 += cugl::to_string(4);
    str2 += ", name:fred, children:0)";
    str1 = test1.toString();
    CUAssertAlwaysLog(str1 == str2,                         "Method toString() failed");
    str1 = test1.toString(true);
    CUAssertAlwaysLog(str1 == "cugl::Node"+str2,            "Method toString() failed");
    str1 = (std::string)test1;
    CUAssertAlwaysLog(str1 == str2,                         "String cast failed");
    
    test1.dispose();
    CUAssertLog(test1.getTag() == 0,                        "Method dispose() failed");
    CUAssertLog(test1.getName() == "",                      "Method dispose() failed");

#pragma mark Position Test
    test1.setPosition(Vec2(3,4));
    CUAssertLog(test1.getPosition() == Vec2(3,4),           "Method setPosition() failed");
    CUAssertLog(test1.getBoundingBox().origin == Vec2(3,4), "Method setPosition() failed");
    CUAssertLog(test1.getPositionX() == 3,                  "Method getPositionX() failed");
    CUAssertLog(test1.getPositionY() == 4,                  "Method getPositionY() failed");
    CUAssertLog(test1.getWorldPosition() == Vec2(3,4),      "Method setPosition() failed");
    
    test1.setPosition(5,6);
    CUAssertLog(test1.getPosition() == Vec2(5,6),           "Method setPosition() failed");
    CUAssertLog(test1.getWorldPosition() == Vec2(5,6),      "Method setPosition() failed");
    
    test1.setPositionX(7);
    CUAssertLog(test1.getPosition() == Vec2(7,6),           "Method setPositionX() failed");
    CUAssertLog(test1.getWorldPosition() == Vec2(7,6),      "Method setPositionX() failed");
   test1.setPositionY(8);
    CUAssertLog(test1.getPosition() == Vec2(7,8),           "Method setPositionY() failed");
    CUAssertLog(test1.getWorldPosition() == Vec2(7,8),      "Method setPositionY() failed");

    test1.setPosition(1,2);
    
#pragma mark Size Test
    test1.setContentSize(Size(3,4));
    CUAssertLog(test1.getContentSize() == Size(3,4),        "Method setContentSize() failed");
    CUAssertLog(test1.getContentWidth() == 3,               "Method setContentSize() failed");
    CUAssertLog(test1.getContentHeight() == 4,              "Method setContentSize() failed");
    CUAssertLog(test1.getWorldPosition() == Vec2(1,2),      "Method setContentSize() failed");

    test1.setContentSize(Size(5,6));
    CUAssertLog(test1.getContentSize() == Size(5,6),        "Method setContentSize() failed");
    CUAssertLog(test1.getWorldPosition() == Vec2(1,2),      "Method setContentSize() failed");

    test1.setContentWidth(7);
    CUAssertLog(test1.getContentSize() == Size(7,6),        "Method setContentWidth() failed");
    CUAssertLog(test1.getWorldPosition() == Vec2(1,2),      "Method setContentWidth() failed");
    test1.setContentHeight(8);
    CUAssertLog(test1.getContentSize() == Size(7,8),        "Method setContentHeight() failed");
    CUAssertLog(test1.getWorldPosition() == Vec2(1,2),      "Method setContentHeight() failed");

    CUAssertLog(test1.getSize() == Size(7,8),               "Method getSize() failed");
    CUAssertLog(test1.getWidth() == 7,                      "Method getWidth() failed");
    CUAssertLog(test1.getHeight() == 8,                     "Method getHeight() failed");
    CUAssertLog(test1.getBoundingBox() == Rect(-2.5,-2,7,8),"Method getBoundingBox() failed");

#pragma mark Anchor Test
    test1.dispose();
    test1.initWithBounds(Rect(0,1,2,4));
    test1.setAnchor(Vec2::ANCHOR_TOP_LEFT);
    CUAssertLog(test1.getAnchor() == Vec2::ANCHOR_TOP_LEFT,     "Method setAnchor() failed");
    CUAssertLog(test1.getPosition() == Vec2(0,5),               "Method setAnchor() failed");
    CUAssertLog(test1.getBoundingBox() == Rect(0,1,2,4),        "Method setAnchor() failed");
    test1.setAnchor(Vec2::ANCHOR_TOP_CENTER);
    CUAssertLog(test1.getAnchor() == Vec2::ANCHOR_TOP_CENTER,   "Method setAnchor() failed");
    CUAssertLog(test1.getPosition() == Vec2(1,5),               "Method setAnchor() failed");
    CUAssertLog(test1.getBoundingBox() == Rect(0,1,2,4),        "Method setAnchor() failed");
    test1.setAnchor(Vec2::ANCHOR_TOP_RIGHT);
    CUAssertLog(test1.getAnchor() == Vec2::ANCHOR_TOP_RIGHT,    "Method setAnchor() failed");
    CUAssertLog(test1.getPosition() == Vec2(2,5),               "Method setAnchor() failed");
    test1.setAnchor(Vec2::ANCHOR_MIDDLE_RIGHT);
    CUAssertLog(test1.getAnchor() == Vec2::ANCHOR_MIDDLE_RIGHT, "Method setAnchor() failed");
    CUAssertLog(test1.getPosition() == Vec2(2,3),               "Method setAnchor() failed");
    CUAssertLog(test1.getBoundingBox() == Rect(0,1,2,4),        "Method setAnchor() failed");
    test1.setAnchor(Vec2::ANCHOR_BOTTOM_RIGHT);
    CUAssertLog(test1.getAnchor() == Vec2::ANCHOR_BOTTOM_RIGHT, "Method setAnchor() failed");
    CUAssertLog(test1.getPosition() == Vec2(2,1),               "Method setAnchor() failed");
    CUAssertLog(test1.getBoundingBox() == Rect(0,1,2,4),        "Method setAnchor() failed");
    test1.setAnchor(Vec2::ANCHOR_BOTTOM_CENTER);
    CUAssertLog(test1.getAnchor() == Vec2::ANCHOR_BOTTOM_CENTER,"Method setAnchor() failed");
    CUAssertLog(test1.getPosition() == Vec2(1,1),               "Method setAnchor() failed");
    CUAssertLog(test1.getBoundingBox() == Rect(0,1,2,4),        "Method setAnchor() failed");
    test1.setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
    CUAssertLog(test1.getAnchor() == Vec2::ANCHOR_BOTTOM_LEFT,  "Method setAnchor() failed");
    CUAssertLog(test1.getPosition() == Vec2(0,1),               "Method setAnchor() failed");
    CUAssertLog(test1.getBoundingBox() == Rect(0,1,2,4),        "Method setAnchor() failed");
    test1.setAnchor(Vec2::ANCHOR_MIDDLE_LEFT);
    CUAssertLog(test1.getAnchor() == Vec2::ANCHOR_MIDDLE_LEFT,  "Method setAnchor() failed");
    CUAssertLog(test1.getPosition() == Vec2(0,3),               "Method setAnchor() failed");
    CUAssertLog(test1.getBoundingBox() == Rect(0,1,2,4),        "Method setAnchor() failed");
    
    test1.setAnchor(Vec2::ANCHOR_TOP_LEFT);
    test1.setContentSize(4,6);
    CUAssertLog(test1.getPosition() == Vec2(0,5),               "Method setContentSize() failed");
    CUAssertLog(test1.getBoundingBox() == Rect(0,-1,4,6),       "Method setContentSize() failed");
    test1.setContentSize(2,4);
    CUAssertLog(test1.getPosition() == Vec2(0,5),               "Method setContentSize() failed");
    CUAssertLog(test1.getBoundingBox() == Rect(0,1,2,4),        "Method setContentSize() failed");

    test1.setAnchor(Vec2::ANCHOR_TOP_CENTER);
    test1.setContentSize(4,6);
    CUAssertLog(test1.getPosition() == Vec2(1,5),               "Method setContentSize() failed");
    CUAssertLog(test1.getBoundingBox() == Rect(-1,-1,4,6),      "Method setContentSize() failed");
    test1.setContentSize(2,4);
    CUAssertLog(test1.getPosition() == Vec2(1,5),               "Method setContentSize() failed");
    CUAssertLog(test1.getBoundingBox() == Rect(0,1,2,4),        "Method setContentSize() failed");

    test1.setAnchor(Vec2::ANCHOR_TOP_RIGHT);
    test1.setContentSize(4,6);
    CUAssertLog(test1.getPosition() == Vec2(2,5),               "Method setContentSize() failed");
    CUAssertLog(test1.getBoundingBox() == Rect(-2,-1,4,6),      "Method setContentSize() failed");
    test1.setContentSize(2,4);
    CUAssertLog(test1.getPosition() == Vec2(2,5),               "Method setContentSize() failed");
    CUAssertLog(test1.getBoundingBox() == Rect(0,1,2,4),        "Method setContentSize() failed");

    test1.setAnchor(Vec2::ANCHOR_MIDDLE_RIGHT);
    test1.setContentSize(4,6);
    CUAssertLog(test1.getPosition() == Vec2(2,3),               "Method setContentSize() failed");
    CUAssertLog(test1.getBoundingBox() == Rect(-2,0,4,6),      "Method setContentSize() failed");
    test1.setContentSize(2,4);
    CUAssertLog(test1.getPosition() == Vec2(2,3),               "Method setContentSize() failed");
    CUAssertLog(test1.getBoundingBox() == Rect(0,1,2,4),        "Method setContentSize() failed");

    test1.setAnchor(Vec2::ANCHOR_BOTTOM_RIGHT);
    test1.setContentSize(4,6);
    CUAssertLog(test1.getPosition() == Vec2(2,1),               "Method setContentSize() failed");
    CUAssertLog(test1.getBoundingBox() == Rect(-2,1,4,6),      "Method setContentSize() failed");
    test1.setContentSize(2,4);
    CUAssertLog(test1.getPosition() == Vec2(2,1),               "Method setContentSize() failed");
    CUAssertLog(test1.getBoundingBox() == Rect(0,1,2,4),        "Method setContentSize() failed");

    test1.setAnchor(Vec2::ANCHOR_BOTTOM_CENTER);
    test1.setContentSize(4,6);
    CUAssertLog(test1.getPosition() == Vec2(1,1),               "Method setContentSize() failed");
    CUAssertLog(test1.getBoundingBox() == Rect(-1,1,4,6),      "Method setContentSize() failed");
    test1.setContentSize(2,4);
    CUAssertLog(test1.getPosition() == Vec2(1,1),               "Method setContentSize() failed");
    CUAssertLog(test1.getBoundingBox() == Rect(0,1,2,4),        "Method setContentSize() failed");

    test1.setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
    test1.setContentSize(4,6);
    CUAssertLog(test1.getPosition() == Vec2(0,1),               "Method setContentSize() failed");
    CUAssertLog(test1.getBoundingBox() == Rect(0,1,4,6),      "Method setContentSize() failed");
    test1.setContentSize(2,4);
    CUAssertLog(test1.getPosition() == Vec2(0,1),               "Method setContentSize() failed");
    CUAssertLog(test1.getBoundingBox() == Rect(0,1,2,4),        "Method setContentSize() failed");

    test1.setAnchor(Vec2::ANCHOR_MIDDLE_LEFT);
    test1.setContentSize(4,6);
    CUAssertLog(test1.getPosition() == Vec2(0,3),               "Method setContentSize() failed");
    CUAssertLog(test1.getBoundingBox() == Rect(0,0,4,6),      "Method setContentSize() failed");
    test1.setContentSize(2,4);
    CUAssertLog(test1.getPosition() == Vec2(0,3),               "Method setContentSize() failed");
    CUAssertLog(test1.getBoundingBox() == Rect(0,1,2,4),        "Method setContentSize() failed");

#pragma mark Visibility Test
    test1.setColor(Color4::RED);
    CUAssertLog(test1.getColor() == Color4::RED,            "Method getColor() failed");
    CUAssertLog(test1.getAbsoluteColor() == Color4::RED,    "Method getAbsoluteColor() failed");
    test1.setVisible(false);
    CUAssertLog(!test1.isVisible(),                         "Method isVisible() failed");
    test1.setRelativeColor(false);
    CUAssertLog(!test1.hasRelativeColor(),                  "Method isVisible() failed");
    
    test1.dispose();
    CUAssertLog(test1.getColor() == Color4::WHITE,          "Method getColor() failed");
    CUAssertLog(test1.isVisible(),                          "Method isVisible() failed");
    CUAssertLog(test1.hasRelativeColor(),                   "Method isVisible() failed");
    
#pragma mark Transform Test
    test1.dispose();
    
    Mat4::createScale(2,2,1,&mtest);
    test1.setScale(2);
    CUAssertLog(test1.getScale() == Vec2(2,2),              "Method setScale() failed");
    CUAssertLog(test1.getNodeToParentTransform() == mtest,  "Method setScale() failed");
    
    test1.setPosition(1,2);
    mtest.translate(1,2,0);
    CUAssertLog(test1.getNodeToParentTransform() == mtest,  "Method setPosition() failed");
    test1.dispose();
    
    Mat4::createScale(3,4,1,&mtest);
    test1.setScale(3,4);
    CUAssertLog(test1.getScale() == Vec2(3,4),              "Method setScale() failed");
    CUAssertLog(test1.getNodeToParentTransform() == mtest,  "Method setScale() failed");

    Mat4::createScale(5,6,1,&mtest);
    test1.setScale(Vec2(5,6));
    CUAssertLog(test1.getScale() == Vec2(5,6),              "Method setScale() failed");
    CUAssertLog(test1.getNodeToParentTransform() == mtest,  "Method setScale() failed");

    mtest.rotateZ(M_PI_4);
    test1.setAngle(M_PI_4);
    CUAssertLog(CU_MATH_APPROX(test1.getAngle(), M_PI_4, CU_MATH_EPSILON),
                                                            "Method setAngle() failed");
    CUAssertLog(test1.getNodeToParentTransform() == mtest,  "Method setAngle() failed");

    test1.setContentSize(2,4);
    Mat4::createTranslation(-1,-2,0,&mtest);
    mtest.scale(5,6,1);
    mtest.rotateZ(M_PI_4);
    CUAssertLog(test1.getNodeToParentTransform() == mtest,  "Method setPosition() failed");

    test1.setPosition(2,3);
    mtest.translate(2,3,0);
    CUAssertLog(test1.getNodeToParentTransform() == mtest,  "Method setPosition() failed");
    
    
    Mat4 mother1, mother2;
    Mat4::createTranslation(10,11,0,&mother1);
    mother1.rotateX(M_PI_4/2.0f);
    
    test1.setAlternateTransform(mother1);
    CUAssertLog(test1.getAlternateTransform() == mother1,   "Method setAlternateTransform() failed");
    CUAssertLog(test1.getNodeToParentTransform() == mtest,  "Method setAlternateTransform() failed");
    
    mother2 = mother1;
    mother2.translate(1,1,0);
    test1.chooseAlternateTransform(true);
    mother1 = test1.getNodeToParentTransform();
    CUAssertLog(test1.getNodeToParentTransform() == mother2,"Method activateAlternateTransform() failed");
    test1.chooseAlternateTransform(false);
    CUAssertLog(test1.getNodeToParentTransform() == mtest,  "Method activateAlternateTransform() failed");

    
    CUAssertLog(test1.getNodeToWorldTransform() == mtest,   "Method getNodeToWorldTransform() failed");

    mtest.invert();
    CUAssertLog(test1.getParentToNodeTransform() == mtest,  "Method getParentToNodeTransform() failed");
    CUAssertLog(test1.getWorldToNodeTransform() == mtest,   "Method getWorldToNodeTransform() failed");

    Vec2 v2test1(5,6);
    Vec2 v2test2;
    v2test2 = mtest.transform(v2test1);
    CUAssertLog(test1.parentToNodeCoords(v2test1).equals(v2test2),
                "Method convertParentToNodeSpace() failed");
    CUAssertLog(test1.worldToNodeCoords(v2test1).equals(v2test2),
                "Method convertWorldToNodeSpace() failed");
    
    mtest.invert();
    v2test2 = mtest.transform(v2test1);
    CUAssertLog(test1.nodeToParentCoords(v2test1).equals(v2test2, 0.00001f),
                "Method convertParentToNodeSpace() failed");
    CUAssertLog(test1.nodeToWorldCoords(v2test1).equals(v2test2,  0.00001f),
                "Method convertWorldToNodeSpace() failed");

#pragma mark Scene Graph Test
    test1.dispose();
    
    testptr1 = Node::alloc();
    test1.addChild(testptr1);
    CUAssertLog(test1.getChildCount() == 1,             "Method addChild() failed");
    CUAssertLog(testptr1->getParent() == &test1,        "Method addChild() failed");
    CUAssertLog(testptr1->getTag() == 0,                "Method addChild() failed");
    CUAssertLog(testptr1->getName() == "",              "Method addChild() failed");

    testptr1 = Node::allocWithPosition(Vec2(1,2));
    test1.addChildWithTag(testptr1,4);
    CUAssertLog(test1.getChildCount() == 2,             "Method addChildWithTag() failed");
    CUAssertLog(testptr1->getParent() == &test1,        "Method addChildWithTag() failed");
    CUAssertLog(testptr1->getTag() == 4,                "Method addChildWithTag() failed");
    CUAssertLog(testptr1->getName() == "",              "Method addChildWithTag() failed");

    testptr1 = Node::allocWithPosition(Vec2(3,4));
    test1.addChildWithName(testptr1,"fred");
    CUAssertLog(test1.getChildCount() == 3,             "Method addChildWithName() failed");
    CUAssertLog(testptr1->getParent() == &test1,        "Method addChildWithName() failed");
    CUAssertLog(testptr1->getTag() == 0,                "Method addChildWithName() failed");
    CUAssertLog(testptr1->getName() == "fred",          "Method addChildWithName() failed");

    testptr1 = Node::allocWithPosition(Vec2(5,6));
    test1.addChild(testptr1);
    testptr1 = Node::allocWithPosition(Vec2(7,8));
    test1.addChildWithTag(testptr1,4);
    testptr1 = Node::allocWithPosition(Vec2(9,10));
    test1.addChildWithName(testptr1,"fred");
    
    CUAssertLog(test1.getChildCount() == 6,                                 "Method addChild() failed");
    CUAssertLog(test1.getChild(0)->getPosition() == Vec2::ZERO,             "Method getChild() failed");
    CUAssertLog(test1.getChild(1)->getPosition() == Vec2(1,2),              "Method getChild() failed");
    CUAssertLog(test1.getChild(2)->getPosition() == Vec2(3,4),              "Method getChild() failed");
    CUAssertLog(test1.getChild(3)->getPosition() == Vec2(5,6),              "Method getChild() failed");
    CUAssertLog(test1.getChild(4)->getPosition() == Vec2(7,8),              "Method getChild() failed");
    CUAssertLog(test1.getChild(5)->getPosition() == Vec2(9,10),             "Method getChild() failed");
    CUAssertLog(test1.getChildByTag(4)->getPosition() == Vec2(1,2),         "Method getChild() failed");
    CUAssertLog(test1.getChildByName("fred")->getPosition() == Vec2(3,4),   "Method getChild() failed");

    std::vector<std::shared_ptr<Node>> kids = test1.getChildren();
    CUAssertLog(kids.size() == 6,                                           "Method getChildren() failed");
    CUAssertLog(kids[0]->getPosition() == Vec2::ZERO,                       "Method getChildren() failed");
    CUAssertLog(kids[1]->getPosition() == Vec2(1,2),                        "Method getChildren() failed");
    CUAssertLog(kids[2]->getPosition() == Vec2(3,4),                        "Method getChildren() failed");
    CUAssertLog(kids[3]->getPosition() == Vec2(5,6),                        "Method getChildren() failed");
    CUAssertLog(kids[4]->getPosition() == Vec2(7,8),                        "Method getChildren() failed");
    CUAssertLog(kids[5]->getPosition() == Vec2(9,10),                       "Method getChildren() failed");
    
    testptr1 = Node::allocWithPosition(Vec2(11,12));
    testptr1->setName("fred");
    std::shared_ptr<Node> testptr2 = kids[2];
    test1.swapChild(testptr2, testptr1);
    
    CUAssertLog(test1.getChildCount() == 6,                                 "Method swapChild() failed");
    CUAssertLog(test1.getChild(0)->getPosition() == Vec2::ZERO,             "Method swapChild() failed");
    CUAssertLog(test1.getChild(1)->getPosition() == Vec2(1,2),              "Method swapChild() failed");
    CUAssertLog(test1.getChild(2)->getPosition() == Vec2(11,12),            "Method swapChild() failed");
    CUAssertLog(test1.getChild(3)->getPosition() == Vec2(5,6),              "Method swapChild() failed");
    CUAssertLog(test1.getChild(4)->getPosition() == Vec2(7,8),              "Method swapChild() failed");
    CUAssertLog(test1.getChild(5)->getPosition() == Vec2(9,10),             "Method swapChild() failed");
    CUAssertLog(test1.getChildByTag(4)->getPosition() == Vec2(1,2),         "Method swapChild() failed");
    CUAssertLog(test1.getChildByName("fred")->getPosition() == Vec2(11,12), "Method swapChild() failed");

    testptr1 = Node::allocWithPosition(Vec2(13,14));
    testptr1->setTag(4);
    testptr2 = kids[1];
    test1.swapChild(testptr2, testptr1);
    
    CUAssertLog(test1.getChildCount() == 6,                                 "Method swapChild() failed");
    CUAssertLog(test1.getChild(0)->getPosition() == Vec2::ZERO,             "Method swapChild() failed");
    CUAssertLog(test1.getChild(1)->getPosition() == Vec2(13,14),            "Method swapChild() failed");
    CUAssertLog(test1.getChild(2)->getPosition() == Vec2(11,12),            "Method swapChild() failed");
    CUAssertLog(test1.getChild(3)->getPosition() == Vec2(5,6),              "Method swapChild() failed");
    CUAssertLog(test1.getChild(4)->getPosition() == Vec2(7,8),              "Method swapChild() failed");
    CUAssertLog(test1.getChild(5)->getPosition() == Vec2(9,10),             "Method swapChild() failed");
    CUAssertLog(test1.getChildByTag(4)->getPosition() == Vec2(13,14),       "Method swapChild() failed");
    CUAssertLog(test1.getChildByName("fred")->getPosition() == Vec2(11,12), "Method swapChild() failed");

    test1.removeChild(3);
    CUAssertLog(test1.getChildCount() == 5,                                 "Method removeChild() failed");
    CUAssertLog(test1.getChild(0)->getPosition() == Vec2::ZERO,             "Method removeChild() failed");
    CUAssertLog(test1.getChild(1)->getPosition() == Vec2(13,14),            "Method removeChild() failed");
    CUAssertLog(test1.getChild(2)->getPosition() == Vec2(11,12),            "Method removeChild() failed");
    CUAssertLog(test1.getChild(3)->getPosition() == Vec2(7,8),              "Method removeChild() failed");
    CUAssertLog(test1.getChild(4)->getPosition() == Vec2(9,10),             "Method removeChild() failed");
    CUAssertLog(test1.getChildByTag(4)->getPosition() == Vec2(13,14),       "Method removeChild() failed");
    CUAssertLog(test1.getChildByName("fred")->getPosition() == Vec2(11,12), "Method removeChild() failed");

    test1.removeChildByTag(4);
    CUAssertLog(test1.getChildCount() == 4,                                 "Method removeChildByTag() failed");
    CUAssertLog(test1.getChild(0)->getPosition() == Vec2::ZERO,             "Method removeChildByTag() failed");
    CUAssertLog(test1.getChild(1)->getPosition() == Vec2(11,12),            "Method removeChildByTag() failed");
    CUAssertLog(test1.getChild(2)->getPosition() == Vec2(7,8),              "Method removeChildByTag() failed");
    CUAssertLog(test1.getChild(3)->getPosition() == Vec2(9,10),             "Method removeChildByTag() failed");
    CUAssertLog(test1.getChildByTag(4)->getPosition() == Vec2(7,8),         "Method removeChildByTag() failed");
    CUAssertLog(test1.getChildByName("fred")->getPosition() == Vec2(11,12), "Method removeChildByTag() failed");

    test1.removeChildByName("fred");
    CUAssertLog(test1.getChildCount() == 3,                                 "Method removeChildByName() failed");
    CUAssertLog(test1.getChild(0)->getPosition() == Vec2::ZERO,             "Method removeChildByName() failed");
    CUAssertLog(test1.getChild(1)->getPosition() == Vec2(7,8),              "Method removeChildByName() failed");
    CUAssertLog(test1.getChild(2)->getPosition() == Vec2(9,10),             "Method removeChildByName() failed");
    CUAssertLog(test1.getChildByTag(4)->getPosition() == Vec2(7,8),         "Method removeChildByName() failed");
    CUAssertLog(test1.getChildByName("fred")->getPosition() == Vec2(9,10),  "Method removeChildByName() failed");

    testptr1 = test1.getChild(1);
    testptr1->removeFromParent();
    CUAssertLog(test1.getChildCount() == 2,                                 "Method removeFromParent() failed");
    CUAssertLog(test1.getChild(0)->getPosition() == Vec2::ZERO,             "Method removeFromParent() failed");
    CUAssertLog(test1.getChild(1)->getPosition() == Vec2(9,10),             "Method removeFromParent() failed");
    CUAssertLog(test1.getChildByTag(4) == nullptr,                          "Method removeFromParent() failed");
    CUAssertLog(test1.getChildByName("fred")->getPosition() == Vec2(9,10),  "Method removeFromParent() failed");

    testptr1 = test1.getChild(1);
    test1.removeChild(testptr1);
    CUAssertLog(test1.getChildCount() == 1,                                 "Method removeChild() failed");
    CUAssertLog(test1.getChild(0)->getPosition() == Vec2::ZERO,             "Method removeChild() failed");
    CUAssertLog(test1.getChildByTag(4) == nullptr,                          "Method removeChild() failed");
    CUAssertLog(test1.getChildByName("fred") == nullptr,                    "Method removeChild() failed");

    test1.addChild(testptr1);
    test1.removeAllChildren();
    CUAssertLog(test1.getChildCount() == 0,                 "Method removeAllChildren() failed");
    CUAssertLog(test1.getChildByTag(4) == nullptr,          "Method removeChild() failed");
    CUAssertLog(test1.getChildByName("fred") == nullptr,    "Method removeChild() failed");

    // HERE WE GO!
    test1.setPosition(1,2);
    test1.setScale(2,3);
    test1.setAngle(M_PI_4);
    test1.setColor(Color4(255,128,255,255));
    
    testptr1 = Node::allocWithPosition(Vec2(3,4));
    testptr1->setScale(5,6);
    testptr1->setAngle(M_PI_4/2.0f);
    testptr1->setColor(Color4(128,255,255,128));

    testptr2 = Node::allocWithPosition(Vec2(7,8));
    testptr2->setScale(9,10);
    testptr2->setAngle(-M_PI_4/2.0f);
    testptr2->setColor(Color4(255,255,128,128));
    
    test1.addChild(testptr1);
    testptr1->addChild(testptr2);
    CUAssertLog(test1.getChildCount() == 1,                     "Method addChild() failed");
    CUAssertLog(testptr1->getChildCount() == 1,                 "Method addChild() failed");
    CUAssertLog(testptr2->getChildCount() == 0,                 "Method addChild() failed");
    
    mtest = test1.getNodeToParentTransform();
    mtest *= testptr1->getNodeToParentTransform();
    mother1 = mtest;
    CUAssertLog(testptr1->getNodeToWorldTransform() == mtest,   "Method getNodeToWorldTransform() failed");
    mother1 *= testptr2->getNodeToParentTransform();
    CUAssertLog(testptr2->getNodeToWorldTransform() == mother1, "Method getNodeToWorldTransform() failed");

    v2test1.set(5,6);
    v2test2 = mtest.transform(v2test1);
    CUAssertLog(testptr1->nodeToWorldCoords(v2test1).equals(v2test2),
                "Method convertNodeToWorldSpace() failed");
    v2test2 = mother1.transform(v2test1);
    CUAssertLog(testptr2->nodeToWorldCoords(v2test1).equals(v2test2),
                "Method convertNodeToWorldSpace() failed");
    
    mtest.invert();
    mother1.invert();
    CUAssertLog(testptr1->getWorldToNodeTransform() == mtest,   "Method getWorldToNodeTransform() failed");
    CUAssertLog(testptr2->getWorldToNodeTransform() == mother1, "Method getWorldToNodeTransform() failed");

    v2test1.set(5,6);
    v2test2 = mtest.transform(v2test1);
    CUAssertLog(testptr1->worldToNodeCoords(v2test1).equals(v2test2),
                "Method convertWorldToNodeSpace() failed");
    v2test2 = mother1.transform(v2test1);
    CUAssertLog(testptr2->worldToNodeCoords(v2test1).equals(v2test2),
                "Method convertWorldToNodeSpace() failed");

    Color4 base = test1.getColor();
    base *= testptr1->getColor();
    CUAssertLog(testptr1->getAbsoluteColor() == base,   "Method getAbsoluteColor() failed");
    base *= testptr2->getColor();
    CUAssertLog(testptr2->getAbsoluteColor() == base,   "Method getAbsoluteColor() failed");
    testptr2->setRelativeColor(false);
    base = test1.getColor();
    base *= testptr1->getColor();
    
    CUAssertLog(testptr1->hasRelativeColor(),           "Method setRelativeColor() failed");
    CUAssertLog(!testptr2->hasRelativeColor(),          "Method setRelativeColor() failed");
    CUAssertLog(testptr1->getAbsoluteColor() == base,   "Method setRelativeColor() failed");
    CUAssertLog(testptr2->getAbsoluteColor() == testptr2->getColor(),
                "Method setRelativeColor() failed");
    
    
#pragma mark z-Order Test
    test1.removeAllChildren();
    
    testptr1 = Node::allocWithPosition(Vec2(4,4));
    test1.addChild(testptr1,4);
    CUAssertLog(!test1.isZDirty(),                                  "Method addChild() failed");
    testptr1 = Node::allocWithPosition(Vec2(2,2));
    test1.addChild(testptr1,2);
    CUAssertLog(test1.isZDirty(),                                   "Method addChild() failed");
    testptr1 = Node::allocWithPosition(Vec2(8,8));
    test1.addChild(testptr1,8);
    testptr1 = Node::allocWithPosition(Vec2(12,12));
    test1.addChild(testptr1,12);
    testptr1 = Node::allocWithPosition(Vec2(6,6));
    test1.addChild(testptr1,6);
    testptr1 = Node::allocWithPosition(Vec2(10,10));
    test1.addChild(testptr1,10);

    CUAssertLog(test1.getChildCount() == 6,                         "Method addChild() failed");
    CUAssertLog(test1.getChild(0)->getPosition() == Vec2(4,4),      "Method addChild() failed");
    CUAssertLog(test1.getChild(1)->getPosition() == Vec2(2,2),      "Method addChild() failed");
    CUAssertLog(test1.getChild(2)->getPosition() == Vec2(8,8),      "Method addChild() failed");
    CUAssertLog(test1.getChild(3)->getPosition() == Vec2(12,12),    "Method addChild() failed");
    CUAssertLog(test1.getChild(4)->getPosition() == Vec2(6,6),      "Method addChild() failed");
    CUAssertLog(test1.getChild(5)->getPosition() == Vec2(10,10),    "Method addChild() failed");

    test1.sortZOrder();
    CUAssertLog(test1.getChildCount() == 6,                         "Method sortZOrder() failed");
    CUAssertLog(!test1.isZDirty(),                                  "Method sortZOrder() failed");
    CUAssertLog(test1.getChild(0)->getPosition() == Vec2(2,2),      "Method sortZOrder() failed");
    CUAssertLog(test1.getChild(1)->getPosition() == Vec2(4,4),      "Method sortZOrder() failed");
    CUAssertLog(test1.getChild(2)->getPosition() == Vec2(6,6),      "Method sortZOrder() failed");
    CUAssertLog(test1.getChild(3)->getPosition() == Vec2(8,8),      "Method sortZOrder() failed");
    CUAssertLog(test1.getChild(4)->getPosition() == Vec2(10,10),    "Method sortZOrder() failed");
    CUAssertLog(test1.getChild(5)->getPosition() == Vec2(12,12),    "Method sortZOrder() failed");

    testptr1 = Node::allocWithPosition(Vec2(14,14));
    test1.addChild(testptr1,14);
    CUAssertLog(test1.getChildCount() == 7,                         "Method addChild() failed");
    CUAssertLog(!test1.isZDirty(),                                  "Method addChild() failed");
    CUAssertLog(test1.getChild(0)->getPosition() == Vec2(2,2),      "Method addChild() failed");
    CUAssertLog(test1.getChild(1)->getPosition() == Vec2(4,4),      "Method addChild() failed");
    CUAssertLog(test1.getChild(2)->getPosition() == Vec2(6,6),      "Method addChild() failed");
    CUAssertLog(test1.getChild(3)->getPosition() == Vec2(8,8),      "Method addChild() failed");
    CUAssertLog(test1.getChild(4)->getPosition() == Vec2(10,10),    "Method addChild() failed");
    CUAssertLog(test1.getChild(5)->getPosition() == Vec2(12,12),    "Method addChild() failed");
    CUAssertLog(test1.getChild(6)->getPosition() == Vec2(14,14),    "Method addChild() failed");

    test1.removeChild(3);
    CUAssertLog(test1.getChildCount() == 6,                         "Method removeChild() failed");
    CUAssertLog(!test1.isZDirty(),                                  "Method removeChild() failed");
    CUAssertLog(test1.getChild(0)->getPosition() == Vec2(2,2),      "Method removeChild() failed");
    CUAssertLog(test1.getChild(1)->getPosition() == Vec2(4,4),      "Method removeChild() failed");
    CUAssertLog(test1.getChild(2)->getPosition() == Vec2(6,6),      "Method removeChild() failed");
    CUAssertLog(test1.getChild(3)->getPosition() == Vec2(10,10),    "Method removeChild() failed");
    CUAssertLog(test1.getChild(4)->getPosition() == Vec2(12,12),    "Method removeChild() failed");
    CUAssertLog(test1.getChild(5)->getPosition() == Vec2(14,14),    "Method removeChild() failed");

    testptr1 = test1.getChild(2);
    testptr1->setZOrder(7);
    CUAssertLog(testptr1->getZOrder() == 7,                         "Method setZOrder() failed");
    CUAssertLog(!test1.isZDirty(),                                  "Method setZOrder() failed");
    testptr1->setZOrder(11);
    CUAssertLog(test1.isZDirty(),                                   "Method setZOrder() failed");
    
    test1.sortZOrder();
    CUAssertLog(test1.getChildCount() == 6,                         "Method sortZOrder() failed");
    CUAssertLog(test1.getChild(0)->getPosition() == Vec2(2,2),      "Method sortZOrder() failed");
    CUAssertLog(test1.getChild(1)->getPosition() == Vec2(4,4),      "Method sortZOrder() failed");
    CUAssertLog(test1.getChild(2)->getPosition() == Vec2(10,10),    "Method sortZOrder() failed");
    CUAssertLog(test1.getChild(3)->getPosition() == Vec2(6,6),      "Method sortZOrder() failed");
    CUAssertLog(test1.getChild(4)->getPosition() == Vec2(12,12),    "Method sortZOrder() failed");
    CUAssertLog(test1.getChild(5)->getPosition() == Vec2(14,14),    "Method sortZOrder() failed");
    
    testptr1->setZOrder(10);
    CUAssertLog(testptr1->getZOrder() == 10,                        "Method setZOrder() failed");
    CUAssertLog(!test1.isZDirty(),                                  "Method setZOrder() failed");
    testptr1->setZOrder(6);
    CUAssertLog(test1.isZDirty(),                                   "Method setZOrder() failed");

    test1.sortZOrder();
    CUAssertLog(test1.getChildCount() == 6,                         "Method sortZOrder() failed");
    CUAssertLog(test1.getChild(0)->getPosition() == Vec2(2,2),      "Method sortZOrder() failed");
    CUAssertLog(test1.getChild(1)->getPosition() == Vec2(4,4),      "Method sortZOrder() failed");
    CUAssertLog(test1.getChild(2)->getPosition() == Vec2(6,6),      "Method sortZOrder() failed");
    CUAssertLog(test1.getChild(3)->getPosition() == Vec2(10,10),    "Method sortZOrder() failed");
    CUAssertLog(test1.getChild(4)->getPosition() == Vec2(12,12),    "Method sortZOrder() failed");
    CUAssertLog(test1.getChild(5)->getPosition() == Vec2(14,14),    "Method sortZOrder() failed");

    
#pragma mark Complete
    CULog("Node tests complete.\n");
    
}
    

#pragma mark -
#pragma mark Main
    
void sceneUnitTest() {
    testNode();
}
    
}
