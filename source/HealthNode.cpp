#include "HealthNode.h"

#include <cugl/2d/CUAnimationNode.h>

using namespace cugl;

/** The diameter of the ship. Also the x coordinate of the center of the ship */
constexpr unsigned int DIAMETER = 1280;

/** The radius of the ship. */
constexpr unsigned int RADIUS = 550;

const Vec2 pos0 = Vec2(DIAMETER + RADIUS * sin(0),
        DIAMETER / 2.0f - (RADIUS - 345)*cos(0));

const Vec2 pos1 = Vec2(DIAMETER + 150 + RADIUS * sin(0),
        DIAMETER / 2.0f - (RADIUS - 410)*cos(0));

const Vec2 pos2 = Vec2(DIAMETER + 202 + RADIUS * sin(0),
        DIAMETER / 2.0f - (RADIUS - 550)*cos(0));

const Vec2 pos3 = Vec2(DIAMETER + 145 + RADIUS * sin(0),
        DIAMETER / 2.0f + (RADIUS - 410)*cos(0));

const Vec2 pos4 = Vec2(DIAMETER + RADIUS * sin(0),
        DIAMETER / 2.0f + (RADIUS - 345)*cos(0));

const Vec2 pos5 = Vec2(DIAMETER - 145 + RADIUS * sin(0),
        DIAMETER / 2.0f + (RADIUS - 410)*cos(0));

const Vec2 pos6 = Vec2(DIAMETER - 202 + RADIUS * sin(0),
        DIAMETER / 2.0f - (RADIUS - 550)*cos(0));

const Vec2 pos7 = Vec2(DIAMETER - 150 + RADIUS * sin(0),
        DIAMETER / 2.0f - (RADIUS - 410)*cos(0));


void HealthNode::draw(const std::shared_ptr<cugl::SpriteBatch>& batch, const Mat4& transform,
        Color4 tint) {
        if(section == 0) { setPosition(pos0); }
        else if(section == 1) { setPosition(pos1); }
        else if(section == 2) { setPosition(pos2); }
        else if(section == 3) { setPosition(pos3); }
        else if(section == 4) { setPosition(pos4); }
        else if(section == 5) { setPosition(pos5); }
        else if(section == 6) { setPosition(pos6); }
        else { setPosition(pos7); }



    setAngle((float)(M_PI * 45 * section) / 180.0f);
    ship->getHealth() > 11 ? setFrame(11) : setFrame(ship->getHealth());

    AnimationNode::draw(batch, transform, tint);
}
