//
//  TCUMathTest.cpp
//  Cornell University Game Library (CUGL)
//
//  This module is a unit test suites for the math classes. It is HIGHLY
//  recommended that you run these tests before trying out OpenGL on a new
//  platform, as this clases are crucial for OpenGL support.
//
//  These test classes only use asserts and have no graphical side-effects.
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
//  Version: 6/20/16

#include "TCUMathTest.h"
#include <string>
#include <ctime>
#include <stdio.h>
#include <stdlib.h>
#include <memory>
#include <cugl/cugl.h>
#include <mach/mach_time.h>
#include <SDL/SDL.h>

using namespace cugl;
using namespace cugl::dsp;


#pragma mark -
#pragma mark Vec2
/**
 * Unit test for a 2-dimension vector
 */
void cugl::testVec2() {
    CULog("Running tests for Vec2.\n");
    
#pragma mark Constructor Test
    // Initial test of constructors
    Vec2 test1;
    CUAssertAlwaysLog(test1.x == 0 && test1.y == 0,     "Trivial constructor failed");

    Vec2 test2(1.5,4);
    CUAssertAlwaysLog(test2.x == 1.5 && test2.y == 4,   "Initialization constructor failed");
    
    float f[2] = {3.5, 6};
    Vec2 test3(f);
    CUAssertAlwaysLog(test3.x == 3.5 && test3.y == 6,   "Array constructor failed");

    Vec2 test4(test2);
    CUAssertAlwaysLog(test4.x == 1.5 && test4.y == 4,   "Copy constructor failed");

    Vec2 test5(test2,test3);
    CUAssertAlwaysLog(test5.x == 2 && test5.y == 2,     "Directional constructor failed");

    
#pragma mark Constants Test
    CUAssertAlwaysLog(Vec2::ZERO.x == 0 && Vec2::ZERO.y == 0,       "Zero vector failed");
    CUAssertAlwaysLog(Vec2::ONE.x == 1 && Vec2::ONE.y == 1,         "Ones vector failed");
    CUAssertAlwaysLog(Vec2::UNIT_X.x == 1 && Vec2::UNIT_X.y == 0,   "X-axis vector failed");
    CUAssertAlwaysLog(Vec2::UNIT_Y.x == 0 && Vec2::UNIT_Y.y == 1,   "Y-axis vector failed");
    CUAssertAlwaysLog(Vec2::ANCHOR_CENTER.x == 0.5 && Vec2::ANCHOR_CENTER.y == 0.5,
                      "Central anchor failed");
    CUAssertAlwaysLog(Vec2::ANCHOR_BOTTOM_LEFT.x == 0.0 && Vec2::ANCHOR_BOTTOM_LEFT.y == 0.0,
                      "Bottom left anchor failed");
    CUAssertAlwaysLog(Vec2::ANCHOR_TOP_LEFT.x == 0.0 && Vec2::ANCHOR_TOP_LEFT.y == 1.0,
                      "Top left anchor failed");
    CUAssertAlwaysLog(Vec2::ANCHOR_BOTTOM_RIGHT.x == 1.0 && Vec2::ANCHOR_BOTTOM_RIGHT.y == 0.0,
                      "Bottom right anchor failed");
    CUAssertAlwaysLog(Vec2::ANCHOR_TOP_RIGHT.x == 1.0 && Vec2::ANCHOR_TOP_RIGHT.y == 1.0,
                      "Top right anchor failed");
    CUAssertAlwaysLog(Vec2::ANCHOR_MIDDLE_RIGHT.x == 1.0 && Vec2::ANCHOR_MIDDLE_RIGHT.y == 0.5,
                      "Middle right anchor failed");
    CUAssertAlwaysLog(Vec2::ANCHOR_MIDDLE_LEFT.x == 0.0 && Vec2::ANCHOR_MIDDLE_LEFT.y == 0.5,
                      "Middle left anchor failed");
    CUAssertAlwaysLog(Vec2::ANCHOR_TOP_CENTER.x == 0.5 && Vec2::ANCHOR_TOP_CENTER.y == 1.0,
                      "Middle top anchor failed");
    CUAssertAlwaysLog(Vec2::ANCHOR_BOTTOM_CENTER.x == 0.5 && Vec2::ANCHOR_BOTTOM_CENTER.y == 0.0,
                      "Middle bottom anchor failed");
    
    
#pragma mark Setter Test
    test1 = test2;
    CUAssertAlwaysLog(test1.x == 1.5 && test1.y == 4, "Basic assignment failed");

    test1 = f;
    CUAssertAlwaysLog(test1.x == 3.5 && test1.y == 6, "Float assignment failed");

    test1.set(-1,1);
    CUAssertAlwaysLog(test1.x == -1 && test1.y == 1, "Parameter assignment failed");

    test1.set(test2);
    CUAssertAlwaysLog(test1.x == 1.5 && test1.y == 4, "Alternate assignment failed");

    test1.set(f);
    CUAssertAlwaysLog(test1.x == 3.5 && test1.y == 6, "Alternate float assignment failed");

    test1.set(test2,test3);
    CUAssertAlwaysLog(test1.x == 2 && test1.y == 2, "Directional assignment failed");

    test1.setZero();
    CUAssertAlwaysLog(test1.x == 0 && test1.y == 0, "Erasing assignment failed");

    
#pragma mark Comparison Test
    test1.set(0,0); test2.set(0,1); test3.set(1,0); test4.set(1,1);
    
    CUAssertAlwaysLog(test1 < test4,    "Less than failed");
    CUAssertAlwaysLog(!(test4 < test1), "Less than failed");
    CUAssertAlwaysLog(test1 < test2,    "Less than failed");
    CUAssertAlwaysLog(test2 < test3,    "Less than failed");
    CUAssertAlwaysLog(!(test1 < test1), "Less than failed");

    CUAssertAlwaysLog(test1 <= test4,   "Less than or equal to failed");
    CUAssertAlwaysLog(!(test4 <= test1),"Less than or equal to failed");
    CUAssertAlwaysLog(test1 <= test2,   "Less than or equal to failed");
    CUAssertAlwaysLog(test2 <= test3,   "Less than or equal to failed");
    CUAssertAlwaysLog(test1 <= test1,   "Less than or equal to failed");

    CUAssertAlwaysLog(test4 > test1,    "Greater than failed");
    CUAssertAlwaysLog(!(test1 > test4), "Greater than failed");
    CUAssertAlwaysLog(test2 > test1,    "Greater than failed");
    CUAssertAlwaysLog(test3 > test2,    "Greater than failed");
    CUAssertAlwaysLog(!(test1 > test1), "Greater than failed");
    
    CUAssertAlwaysLog(test4 >= test1,   "Greater than or equal to failed");
    CUAssertAlwaysLog(!(test1 >= test4),"Greater than or equal to failed");
    CUAssertAlwaysLog(test2 >= test1,   "Greater than or equal to failed");
    CUAssertAlwaysLog(test3 >= test2,   "Greater than or equal to failed");
    CUAssertAlwaysLog(test1 >= test1,   "Greater than or equal to failed");

    CUAssertAlwaysLog(test1 == test1,   "Equals failed");
    CUAssertAlwaysLog(test2 == test2,   "Equals failed");
    CUAssertAlwaysLog(test3 == test3,   "Equals failed");
    CUAssertAlwaysLog(!(test1 == test4),"Equals failed");

    CUAssertAlwaysLog(!(test1 != test1),"Not equals failed");
    CUAssertAlwaysLog(!(test2 != test2),"Not equals failed");
    CUAssertAlwaysLog(!(test3 != test3),"Not equals failed");
    CUAssertAlwaysLog(test1 != test4,   "Not equals failed");

    CUAssertAlwaysLog(test1.under(test4),   "Method under() failed");
    CUAssertAlwaysLog(!test4.under(test1),  "Method under() failed");
    CUAssertAlwaysLog(!test2.under(test3),  "Method under() failed");
    CUAssertAlwaysLog(!test3.under(test2),  "Method under() failed");
    CUAssertAlwaysLog(test1.under(test1),   "Method under() failed");

    CUAssertAlwaysLog(test4.over(test1),    "Method over() failed");
    CUAssertAlwaysLog(!test1.over(test4),   "Method over() failed");
    CUAssertAlwaysLog(!test2.over(test3),   "Method over() failed");
    CUAssertAlwaysLog(!test3.over(test2),   "Method over() failed");
    CUAssertAlwaysLog(test1.over(test1),    "Method over() failed");

    test5.set(0,CU_MATH_EPSILON*0.5);
    CUAssertAlwaysLog(test1.equals(test1), "Approximate equals failed");
    CUAssertAlwaysLog(test1.equals(test5), "Approximate equals failed");
    
    
#pragma mark Static Arithmetic Test
    Vec2* testptr;

    test1 = Vec2::forAngle(0);
    CUAssertAlwaysLog(test1.equals(Vec2::UNIT_X),   "Vec2::forAngle() failed");

    test1 = Vec2::forAngle(M_PI_2);
    CUAssertAlwaysLog(test1.equals(Vec2::UNIT_Y),   "Vec2::forAngle() failed");

    test1 = Vec2::forAngle(M_PI_4);
    test2.set(1/sqrtf(2),1/sqrtf(2));
    CUAssertAlwaysLog(test1.equals(test2),          "Vec2::forAngle() failed");
    
    test1.set(-2,2);
    testptr = Vec2::clamp(test1,Vec2(-3,-3),Vec2(3,3),&test2);
    CUAssertAlwaysLog(test1 == test2,               "Vec2::clamp() failed");
    CUAssertAlwaysLog(testptr == &test2,            "Vec2::clamp() failed");

    Vec2::clamp(test1,Vec2::ZERO,Vec2(3,3),&test2);
    CUAssertAlwaysLog(test1 != test2,               "Vec2::clamp() failed");
    CUAssertAlwaysLog(test2.x == 0 && test2.y == 2, "Vec2::clamp() failed");
    
    Vec2::clamp(test1,Vec2(-3,-3),Vec2::ZERO,&test2);
    CUAssertAlwaysLog(test1 != test2,               "Vec2::clamp() failed");
    CUAssertAlwaysLog(test2.x == -2 && test2.y == 0,"Vec2::clamp() failed");

    Vec2::clamp(test1,Vec2(-1,-1),Vec2(1,1),&test2);
    CUAssertAlwaysLog(test1 != test2,               "Vec2::clamp() failed");
    CUAssertAlwaysLog(test2.x == -1 && test2.y == 1,"Vec2::clamp() failed");

    float angle = Vec2::angle(Vec2::UNIT_X,Vec2::UNIT_Y);
    CUAssertAlwaysLog(CU_MATH_APPROX(angle,M_PI_2,CU_MATH_EPSILON),   "Vec2::angle failed");
    angle = Vec2::angle(Vec2::UNIT_Y,Vec2::UNIT_X);
    CUAssertAlwaysLog(CU_MATH_APPROX(angle,-M_PI_2,CU_MATH_EPSILON),  "Vec2::angle failed");

    testptr = Vec2::add(Vec2::UNIT_X,Vec2::UNIT_Y,&test1);
    CUAssertAlwaysLog(test1 == Vec2::ONE,           "Vec2::add() failed");
    CUAssertAlwaysLog(testptr == &test1,            "Vec2::add() failed");
    
    test1.set(2,2);
    Vec2::add(Vec2::ONE,Vec2::ONE,&test2);
    CUAssertAlwaysLog(test1 == test2,               "Vec2::add() failed");

    test1.set(1,-1);
    testptr = Vec2::subtract(Vec2::UNIT_X,Vec2::UNIT_Y,&test2);
    CUAssertAlwaysLog(test1 == test2,               "Vec2::subtract() failed");
    CUAssertAlwaysLog(testptr == &test2,            "Vec2::subtract() failed");
    
    test1.set(2,2);
    Vec2::subtract(Vec2::ONE,Vec2::ONE,&test1);
    CUAssertAlwaysLog(test1 == Vec2::ZERO,          "Vec2::subtract() failed");
    
    testptr = Vec2::scale(Vec2::ONE,2,&test1);
    CUAssertAlwaysLog(testptr == &test1,            "Vec2::scale() failed");
    CUAssertAlwaysLog(test1 == Vec2(2,2),           "Vec2::scale() failed");
    Vec2::scale(Vec2::UNIT_X,2,&test1);
    CUAssertAlwaysLog(test1 == Vec2(2,0),           "Vec2::scale() failed");
    Vec2::scale(Vec2::UNIT_Y,2,&test1);
    CUAssertAlwaysLog(test1 == Vec2(0,2),           "Vec2::scale() failed");
    
    test2.set(-0.5,0.5);
    testptr = Vec2::scale(Vec2::ONE,test2,&test1);
    CUAssertAlwaysLog(testptr == &test1,            "Vec2::scale() failed");
    CUAssertAlwaysLog(test1 == Vec2(-0.5,0.5),      "Vec2::scale() failed");
    Vec2::scale(Vec2::UNIT_X,test2,&test1);
    CUAssertAlwaysLog(test1 == Vec2(-0.5,0),        "Vec2::scale() failed");
    Vec2::scale(Vec2::UNIT_Y,test2,&test1);
    CUAssertAlwaysLog(test1 == Vec2(0,0.5),         "Vec2::scale() failed");
    
    testptr = Vec2::divide(Vec2::ONE,2,&test1);
    CUAssertAlwaysLog(testptr == &test1,            "Vec2::divide() failed");
    CUAssertAlwaysLog(test1 == Vec2(0.5,0.5),       "Vec2::divide() failed");
    Vec2::divide(Vec2::UNIT_X,2,&test1);
    CUAssertAlwaysLog(test1 == Vec2(0.5,0),         "Vec2::divide() failed");
    Vec2::divide(Vec2::UNIT_Y,2,&test1);
    CUAssertAlwaysLog(test1 == Vec2(0,0.5),         "Vec2::divide() failed");
    
    test2.set(-0.5,0.5);
    testptr = Vec2::divide(Vec2::ONE,test2,&test1);
    CUAssertAlwaysLog(testptr == &test1,            "Vec2::divide() failed");
    CUAssertAlwaysLog(test1 == Vec2(-2,2),          "Vec2::divide() failed");
    Vec2::divide(Vec2::UNIT_X,test2,&test1);
    CUAssertAlwaysLog(test1 == Vec2(-2,0),          "Vec2::divide() failed");
    Vec2::divide(Vec2::UNIT_Y,test2,&test1);
    CUAssertAlwaysLog(test1 == Vec2(0,2),           "Vec2::divide() failed");
    
    testptr = Vec2::negate(Vec3::ONE,&test1);
    CUAssertAlwaysLog(testptr == &test1,            "Vec2::negate() failed");
    CUAssertAlwaysLog(test1 == Vec2(-1,-1),         "Vec2::negate() failed");
    Vec2::negate(Vec2::UNIT_X,&test1);
    CUAssertAlwaysLog(test1 == Vec2(-1, 0),         "Vec2::negate() failed");
    Vec2::negate(Vec2::UNIT_Y,&test1);
    CUAssertAlwaysLog(test1 == Vec2( 0,-1),         "Vec2::negate() failed");
    
    test1.set(2,2);
    testptr = Vec2::reciprocate(test1,&test2);
    CUAssertAlwaysLog(testptr == &test2,            "Vec2::reciprocate() failed");
    CUAssertAlwaysLog(test2 == Vec2(0.5,0.5),       "Vec2::reciprocate() failed");
    Vec2::reciprocate(Vec2::ONE,&test2);
    CUAssertAlwaysLog(test2 == Vec2::ONE,           "Vec2::reciprocate() failed");

#pragma mark Arithmetic Test
    test1.set(-2,2);
    test2.set(-2,2);
    test2.clamp(Vec2(-3,-3),Vec2(3,3));
    CUAssertAlwaysLog(test1 == test2,               "Method clamp() failed");
    
    test2.clamp(Vec2::ZERO,Vec2(3,3));
    CUAssertAlwaysLog(test1 != test2,               "Method clamp() failed");
    CUAssertAlwaysLog(test2.x == 0 && test2.y == 2, "Method clamp() failed");
    
    test2 = test1;
    test2.clamp(Vec2(-3,-3),Vec2::ZERO);
    CUAssertAlwaysLog(test1 != test2,               "Method clamp() failed");
    CUAssertAlwaysLog(test2.x == -2 && test2.y == 0,"Method clamp() failed");
    
    test2 = test1;
    test2.clamp(Vec2(-1,-1),Vec2(1,1));
    CUAssertAlwaysLog(test1 != test2,               "Method clamp() failed");
    CUAssertAlwaysLog(test2.x == -1 && test2.y == 1,"Method clamp() failed");

    test2 = test1;
    test3 = test2.getClamp(Vec2::ZERO,Vec2(3,3));
    CUAssertAlwaysLog(test1 == test2,               "Method getClamp() failed");
    CUAssertAlwaysLog(test1 != test3,               "Method getClamp() failed");
    CUAssertAlwaysLog(test3.x == 0 && test3.y == 2, "Method clamp() failed");
    
    test3 = test2.getClamp(Vec2(-3,-3),Vec2::ZERO);
    CUAssertAlwaysLog(test1 == test2,               "Method getClamp() failed");
    CUAssertAlwaysLog(test1 != test3,               "Method getClamp() failed");
    CUAssertAlwaysLog(test3.x == -2 && test3.y == 0,"Method clamp() failed");
    
    test3 = test2.getClamp(Vec2(-1,-1),Vec2(1,1));
    CUAssertAlwaysLog(test1 == test2,               "Method getClamp() failed");
    CUAssertAlwaysLog(test1 != test3,               "Method getClamp() failed");
    CUAssertAlwaysLog(test3.x == -1 && test3.y == 1,"Method getClamp() failed");
    
    test1 = Vec2::UNIT_X;
    test1.add(Vec2::UNIT_Y);
    CUAssertAlwaysLog(test1 == Vec2::ONE,           "Method add() failed");
    
    test1 = Vec2::ONE;
    test1.add(test1);
    CUAssertAlwaysLog(test1 == Vec2(2,2),           "Method add() failed");

    test1 = Vec2::ONE;
    test1.add(2,3);
    CUAssertAlwaysLog(test1 == Vec2(3,4),           "Method add() failed");

    test1 = Vec2::UNIT_X;
    test1.subtract(Vec2::UNIT_Y);
    CUAssertAlwaysLog(test1 == Vec2(1,-1),          "Method subtract() failed");

    test1 = Vec2::ONE;
    test1.subtract(test1);
    CUAssertAlwaysLog(test1 == Vec2::ZERO,          "Method subtract() failed");

    test1 = Vec2::ONE;
    test1.subtract(2,3);
    CUAssertAlwaysLog(test1 == Vec2(-1,-2),         "Method subtract() failed");

    test1 = Vec2::ONE;
    test2 = Vec2::UNIT_X;
    test3 = Vec2::UNIT_Y;
    test1.scale(2); test2.scale(2); test3.scale(2);
    CUAssertAlwaysLog(test1 == Vec2(2,2),           "Method scale() failed");
    CUAssertAlwaysLog(test2 == Vec2(2,0),           "Method scale() failed");
    CUAssertAlwaysLog(test3 == Vec2(0,2),           "Method scale() failed");

    test1 = Vec2::ONE;
    test2 = Vec2::UNIT_X;
    test3 = Vec2::UNIT_Y;
    test1.scale(2,3); test2.scale(2,3); test3.scale(2,3);
    CUAssertAlwaysLog(test1 == Vec2(2,3),           "Method scale() failed");
    CUAssertAlwaysLog(test2 == Vec2(2,0),           "Method scale() failed");
    CUAssertAlwaysLog(test3 == Vec2(0,3),           "Method scale() failed");

    test1 = Vec2::ONE;
    test2 = Vec2::UNIT_X;
    test3 = Vec2::UNIT_Y;
    test4 = Vec2(-0.5,0.5);
    test1.scale(test4); test2.scale(test4); test3.scale(test4);
    CUAssertAlwaysLog(test1 == Vec2(-0.5,0.5),      "Method scale() failed");
    CUAssertAlwaysLog(test2 == Vec2(-0.5,0),        "Method scale() failed");
    CUAssertAlwaysLog(test3 == Vec2(0,0.5),         "Method scale() failed");

    test1 = Vec2::ONE;
    test2 = Vec2::UNIT_X;
    test3 = Vec2::UNIT_Y;
    test1.divide(2); test2.divide(2); test3.divide(2);
    CUAssertAlwaysLog(test1 == Vec2(0.5,0.5),       "Method divide() failed");
    CUAssertAlwaysLog(test2 == Vec2(0.5,0),         "Method divide() failed");
    CUAssertAlwaysLog(test3 == Vec2(0,0.5),         "Method divide() failed");
    
    test1 = Vec2::ONE;
    test2 = Vec2::UNIT_X;
    test3 = Vec2::UNIT_Y;
    test1.divide(2,4); test2.divide(2,4); test3.divide(2,4);
    CUAssertAlwaysLog(test1 == Vec2(0.5,0.25),      "Method divide() failed");
    CUAssertAlwaysLog(test2 == Vec2(0.5,0),         "Method divide() failed");
    CUAssertAlwaysLog(test3 == Vec2(0,0.25),        "Method divide() failed");
    
    test1 = Vec2::ONE;
    test2 = Vec2::UNIT_X;
    test3 = Vec2::UNIT_Y;
    test4 = Vec2(-0.5,0.5);
    test1.divide(test4); test2.divide(test4); test3.divide(test4);
    CUAssertAlwaysLog(test1 == Vec2(-2,2),          "Method divide() failed");
    CUAssertAlwaysLog(test2 == Vec2(-2,0),          "Method divide() failed");
    CUAssertAlwaysLog(test3 == Vec2(0,2),           "Method divide() failed");
    
    test1 = Vec2::ONE;
    test2 = Vec2::UNIT_X;
    test3 = Vec2::UNIT_Y;
    test1.negate();test2.negate(); test3.negate();
    CUAssertAlwaysLog(test1 == Vec2(-1,-1),         "Method negate() failed");
    CUAssertAlwaysLog(test2 == Vec2(-1, 0),         "Method negate() failed");
    CUAssertAlwaysLog(test3 == Vec2( 0,-1),         "Method negate() failed");
    
    test1 = Vec2::ONE;
    test2 = Vec2::UNIT_X;
    test3 = Vec2::UNIT_Y;
    test5 = test1.getNegation();
    CUAssertAlwaysLog(test5 != test1,               "Method getNegation() failed");
    CUAssertAlwaysLog(test5 == Vec2(-1,-1),         "Method getNegation() failed");
    test5 = test2.getNegation();
    CUAssertAlwaysLog(test5 == Vec2(-1, 0),         "Method getNegation() failed");
    test5 = test3.getNegation();
    CUAssertAlwaysLog(test5 == Vec2( 0,-1),         "Method getNegation() failed");
    
    test1.set(2,2);
    test2 = Vec2::ONE;
    test1.reciprocate(); test2.reciprocate();
    CUAssertAlwaysLog(test1 == Vec2(0.5,0.5),       "Method reciprocate() failed");
    CUAssertAlwaysLog(test2 == Vec3::ONE,           "Method reciprocate() failed");
    
    test1.set(2,2);
    test2 = Vec2::ONE;
    test3 = test1.getReciprocal();
    CUAssertAlwaysLog(test3 != test1,               "Method getReciprocal() failed");
    CUAssertAlwaysLog(test3 == Vec2(0.5,0.5),       "Method getReciprocal() failed");
    test3 = test2.getReciprocal();
    CUAssertAlwaysLog(test3 == Vec2::ONE,           "Method getReciprocal() failed");
    
    test1 = Vec2::ONE;
    test2 = Vec2::UNIT_X;
    test3 = Vec2::UNIT_Y;
    test1.map(asinf); test2.map(asinf); test3.map(asinf);
    CUAssertAlwaysLog(CU_MATH_APPROX(test1.x,M_PI_2,CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test1.y,M_PI_2,CU_MATH_EPSILON),
                      "Method map() failed");
    CUAssertAlwaysLog(CU_MATH_APPROX(test2.x,M_PI_2,CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test2.y,0,CU_MATH_EPSILON),
                      "Method map() failed");
    CUAssertAlwaysLog(CU_MATH_APPROX(test3.x,0,CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test3.y,M_PI_2,CU_MATH_EPSILON),
                      "Method map() failed");

    test1 = Vec2::ONE;
    test2 = Vec2::UNIT_X;
    test3 = Vec2::UNIT_Y;
    test4 = test1.getMap(asinf);
    CUAssertAlwaysLog(test1 != test4,   "Method getMap() failed");
    CUAssertAlwaysLog(CU_MATH_APPROX(test4.x,M_PI_2,CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test4.y,M_PI_2,CU_MATH_EPSILON),
                      "Method getMap() failed");
    test4 = test2.getMap(asinf);
    CUAssertAlwaysLog(test2 != test4,   "Method getMap() failed");
    CUAssertAlwaysLog(CU_MATH_APPROX(test4.x,M_PI_2,CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test4.y,0,CU_MATH_EPSILON),
                      "Method getMap() failed");
    test4 = test3.getMap(asinf);
    CUAssertAlwaysLog(test3 != test4,   "Method getMap() failed");
    CUAssertAlwaysLog(CU_MATH_APPROX(test4.x,0,CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test4.y,M_PI_2,CU_MATH_EPSILON),
                      "Method getMap() failed");

#pragma mark Operator Test
    test1 = Vec2::UNIT_X;
    test1 += Vec2::UNIT_Y;
    CUAssertAlwaysLog(test1 == Vec2::ONE,           "Addition operation failed");
    
    test1 = Vec2::ONE;
    test1 += test1;
    CUAssertAlwaysLog(test1 == Vec2(2,2),           "Addition operation failed");
    CUAssertAlwaysLog(Vec2::UNIT_X+Vec2::UNIT_Y == Vec2::ONE,   "Addition operation failed");
    CUAssertAlwaysLog(Vec2::ONE+Vec2::ONE == Vec2(2,2),         "Addition operation failed");
    
    test1 = Vec2::UNIT_X;
    test1 -= Vec2::UNIT_Y;
    CUAssertAlwaysLog(test1 == Vec2(1,-1),          "Subtraction operation failed");
    
    test1 = Vec2::ONE;
    test1 -= test1;
    CUAssertAlwaysLog(test1 == Vec2::ZERO,          "Subtraction operation failed");
    CUAssertAlwaysLog(Vec2::UNIT_X-Vec2::UNIT_Y == Vec2(1,-1),  "Subtraction operation failed");
    CUAssertAlwaysLog(Vec2::ONE-Vec2::ONE == Vec2::ZERO,        "Subtraction operation failed");
    
    test1 = Vec2::ONE;
    test2 = Vec2::UNIT_X;
    test3 = Vec2::UNIT_Y;
    test1 *= 2; test2 *= 2; test3 *= 2;
    CUAssertAlwaysLog(test1 == Vec2(2,2),           "Scaling operation failed");
    CUAssertAlwaysLog(test2 == Vec2(2,0),           "Scaling operation failed");
    CUAssertAlwaysLog(test3 == Vec2(0,2),           "Scaling operation failed");
    CUAssertAlwaysLog(Vec2::ONE*2 == Vec2(2,2),     "Scaling operation failed");
    CUAssertAlwaysLog(Vec2::UNIT_X*2 == Vec2(2,0),  "Scaling operation failed");
    CUAssertAlwaysLog(Vec2::UNIT_Y*2 == Vec2(0,2),  "Scaling operation failed");
    CUAssertAlwaysLog(2*Vec2::ONE == Vec2(2,2),     "Scaling operation failed");
    CUAssertAlwaysLog(2*Vec2::UNIT_X == Vec2(2,0),  "Scaling operation failed");
    CUAssertAlwaysLog(2*Vec2::UNIT_Y == Vec2(0,2),  "Scaling operation failed");
    

    test1 = Vec2::ONE;
    test2 = Vec2::UNIT_X;
    test3 = Vec2::UNIT_Y;
    test4.set(2,3);
    test1 *= test4; test2 *= test4; test3 *= test4;
    CUAssertAlwaysLog(test1 == Vec2(2,3),           "Scaling operation failed");
    CUAssertAlwaysLog(test2 == Vec2(2,0),           "Scaling operation failed");
    CUAssertAlwaysLog(test3 == Vec2(0,3),           "Scaling operation failed");
    CUAssertAlwaysLog(Vec2::ONE*test4 == Vec2(2,3),     "Scaling operation failed");
    CUAssertAlwaysLog(Vec2::UNIT_X*test4 == Vec2(2,0),  "Scaling operation failed");
    CUAssertAlwaysLog(Vec2::UNIT_Y*test4 == Vec2(0,3),  "Scaling operation failed");

    test1 = Vec2::ONE;
    test2 = Vec2::UNIT_X;
    test3 = Vec2::UNIT_Y;
    test1 /= 0.5; test2 /= 0.5; test3 /= 0.5;
    CUAssertAlwaysLog(test1 == Vec2(2,2),           "Division operation failed");
    CUAssertAlwaysLog(test2 == Vec2(2,0),           "Division operation failed");
    CUAssertAlwaysLog(test3 == Vec2(0,2),           "Division operation failed");
    CUAssertAlwaysLog(Vec2::ONE/0.5 == Vec2(2,2),     "Division operation failed");
    CUAssertAlwaysLog(Vec2::UNIT_X/0.5 == Vec2(2,0),  "Division operation failed");
    CUAssertAlwaysLog(Vec2::UNIT_Y/0.5 == Vec2(0,2),  "Division operation failed");
    
    
    test1 = Vec2::ONE;
    test2 = Vec2::UNIT_X;
    test3 = Vec2::UNIT_Y;
    test4.set(1/2.0f,1/4.0f);
    test1 /= test4; test2 /= test4; test3 /= test4;
    CUAssertAlwaysLog(test1 == Vec2(2,4),           "Division operation failed");
    CUAssertAlwaysLog(test2 == Vec2(2,0),           "Division operation failed");
    CUAssertAlwaysLog(test3 == Vec2(0,4),           "Division operation failed");
    CUAssertAlwaysLog(Vec2::ONE/test4 == Vec2(2,4),     "Division operation failed");
    CUAssertAlwaysLog(Vec2::UNIT_X/test4 == Vec2(2,0),  "Division operation failed");
    CUAssertAlwaysLog(Vec2::UNIT_Y/test4 == Vec2(0,4),  "Division operation failed");

    CUAssertAlwaysLog(-Vec2::ONE == Vec2(-1,-1),    "Negation operation failed");
    CUAssertAlwaysLog(-Vec2::UNIT_X == Vec2(-1, 0), "Negation operation failed");
    CUAssertAlwaysLog(-Vec2::UNIT_Y == Vec2( 0,-1), "Negation operation failed");

#pragma mark Linear Attributes
    test1.set(1/sqrtf(2),1/sqrtf(2));
    CUAssertAlwaysLog(CU_MATH_APPROX(Vec2::UNIT_Y.getAngle(),M_PI_2,CU_MATH_EPSILON),
                      "Method getAngle() failed");
    CUAssertAlwaysLog(CU_MATH_APPROX(Vec2::UNIT_X.getAngle(),0,CU_MATH_EPSILON),
                      "Method getAngle() failed");
    CUAssertAlwaysLog(CU_MATH_APPROX(test1.getAngle(),M_PI_4,CU_MATH_EPSILON),
                      "Method getAngle() failed");
    CUAssertAlwaysLog(CU_MATH_APPROX(Vec2::UNIT_Y.getAngle(Vec2::UNIT_X),-M_PI_2,CU_MATH_EPSILON),
                      "Method getAngle() failed");
    CUAssertAlwaysLog(CU_MATH_APPROX(Vec2::UNIT_X.getAngle(Vec2::UNIT_Y),M_PI_2,CU_MATH_EPSILON),
                      "Method getAngle() failed");
    CUAssertAlwaysLog(CU_MATH_APPROX(test1.getAngle(Vec2::UNIT_X),-M_PI_4,CU_MATH_EPSILON),
                      "Method getAngle() failed");
    CUAssertAlwaysLog(CU_MATH_APPROX(test1.getAngle(Vec2::UNIT_Y),M_PI_4,CU_MATH_EPSILON),
                      "Method getAngle() failed");
    CUAssertAlwaysLog(CU_MATH_APPROX(test1.getAngle(test1),0,CU_MATH_EPSILON),
                      "Method getAngle() failed");
    
    CUAssertAlwaysLog(Vec2::ZERO.isZero(),      "Method isZero() failed");
    CUAssertAlwaysLog(!Vec2::UNIT_X.isZero(),   "Method isZero() failed");
    CUAssertAlwaysLog(!Vec2::UNIT_Y.isZero(),   "Method isZero() failed");
    CUAssertAlwaysLog(!Vec2::ONE.isZero(),      "Method isZero() failed");

    test1.set(0,CU_MATH_EPSILON*0.5);
    CUAssertAlwaysLog(Vec2::ZERO.isNearZero(),      "Method isNearZero() failed");
    CUAssertAlwaysLog(test1.isNearZero(),           "Method isNearZero() failed");
    CUAssertAlwaysLog(!Vec2::UNIT_X.isNearZero(),   "Method isNearZero() failed");
    CUAssertAlwaysLog(!Vec2::UNIT_Y.isNearZero(),   "Method isNearZero() failed");
    CUAssertAlwaysLog(!Vec2::ONE.isNearZero(),      "Method isNearZero() failed");
    
    CUAssertAlwaysLog(!Vec2::ZERO.isOne(),          "Method isOne() failed");
    CUAssertAlwaysLog(!Vec2::UNIT_X.isOne(),        "Method isOne() failed");
    CUAssertAlwaysLog(!Vec2::UNIT_Y.isOne(),        "Method isOne() failed");
    CUAssertAlwaysLog(Vec2::ONE.isOne(),            "Method isOne() failed");

    CUAssertAlwaysLog(!Vec2::ZERO.isInvertible(),   "Method isInvertible() failed");
    CUAssertAlwaysLog(!Vec2::UNIT_X.isInvertible(), "Method isInvertible() failed");
    CUAssertAlwaysLog(!Vec2::UNIT_Y.isInvertible(), "Method isInvertible() failed");
    CUAssertAlwaysLog(Vec2::ONE.isInvertible(),     "Method isInvertible() failed");

    CUAssertAlwaysLog(!Vec2::ZERO.isUnit(),         "Method isUnit() failed");
    CUAssertAlwaysLog(Vec2::UNIT_X.isUnit(),        "Method isUnit() failed");
    CUAssertAlwaysLog(Vec2::UNIT_Y.isUnit(),        "Method isUnit() failed");
    CUAssertAlwaysLog(!Vec2::ONE.isUnit(),          "Method isUnit() failed");
    CUAssertAlwaysLog(Vec2::forAngle(M_PI_4).isUnit(),  "Method isUnit() failed");

    CUAssertAlwaysLog(Vec2::ZERO.distance(Vec2::UNIT_X) == 1,       "Method distance() failed");
    CUAssertAlwaysLog(Vec2::UNIT_X.distance(Vec2::ZERO) == 1,       "Method distance() failed");
    CUAssertAlwaysLog(Vec2::ZERO.distance(Vec2::UNIT_Y) == 1,       "Method distance() failed");
    CUAssertAlwaysLog(Vec2::UNIT_Y.distance(Vec2::ZERO) == 1,       "Method distance() failed");
    CUAssertAlwaysLog(Vec2::ONE.distance(Vec2::UNIT_X) == 1,        "Method distance() failed");
    CUAssertAlwaysLog(Vec2::UNIT_X.distance(Vec2::ONE) == 1,        "Method distance() failed");
    CUAssertAlwaysLog(Vec2::ONE.distance(Vec2::UNIT_Y) == 1,        "Method distance() failed");
    CUAssertAlwaysLog(Vec2::UNIT_Y.distance(Vec2::ONE) == 1,        "Method distance() failed");
    CUAssertAlwaysLog(Vec2::ZERO.distance(Vec2::ONE) == sqrtf(2),   "Method distance() failed");
    CUAssertAlwaysLog(Vec2::ONE.distance(Vec2::ZERO) == sqrtf(2),   "Method distance() failed");
    CUAssertAlwaysLog(Vec2(1,2).distance(Vec2(3,0)) == sqrtf(8),    "Method distance() failed");
    CUAssertAlwaysLog(Vec2(3,0).distance(Vec2(1,2)) == sqrtf(8),    "Method distance() failed");

    CUAssertAlwaysLog(Vec2::ZERO.distanceSquared(Vec2::UNIT_X) == 1,"Method distanceSquared() failed");
    CUAssertAlwaysLog(Vec2::UNIT_X.distanceSquared(Vec2::ZERO) == 1,"Method distanceSquared() failed");
    CUAssertAlwaysLog(Vec2::ZERO.distanceSquared(Vec2::UNIT_Y) == 1,"Method distanceSquared() failed");
    CUAssertAlwaysLog(Vec2::UNIT_Y.distanceSquared(Vec2::ZERO) == 1,"Method distanceSquared() failed");
    CUAssertAlwaysLog(Vec2::ONE.distanceSquared(Vec2::UNIT_X) == 1, "Method distanceSquared() failed");
    CUAssertAlwaysLog(Vec2::UNIT_X.distanceSquared(Vec2::ONE) == 1, "Method distanceSquared() failed");
    CUAssertAlwaysLog(Vec2::ONE.distanceSquared(Vec2::UNIT_Y) == 1, "Method distanceSquared() failed");
    CUAssertAlwaysLog(Vec2::UNIT_Y.distanceSquared(Vec2::ONE) == 1, "Method distanceSquared() failed");
    CUAssertAlwaysLog(Vec2::ZERO.distanceSquared(Vec2::ONE) == 2,   "Method distanceSquared() failed");
    CUAssertAlwaysLog(Vec2::ONE.distanceSquared(Vec2::ZERO) == 2,   "Method distanceSquared() failed");
    CUAssertAlwaysLog(Vec2(1,2).distanceSquared(Vec2(3,0)) == 8,    "Method distanceSquared() failed");
    CUAssertAlwaysLog(Vec2(3,0).distanceSquared(Vec2(1,2)) == 8,    "Method distanceSquared() failed");

    CUAssertAlwaysLog(Vec2::ZERO.length() == 0,             "Method length() failed");
    CUAssertAlwaysLog(Vec2::UNIT_X.length() == 1,           "Method length() failed");
    CUAssertAlwaysLog(Vec2::UNIT_Y.length() == 1,           "Method length() failed");
    CUAssertAlwaysLog(Vec2::ONE.length() == sqrtf(2),       "Method length() failed");
    CUAssertAlwaysLog(Vec2(-3,4).length() == 5,             "Method length() failed");
    
    CUAssertAlwaysLog(Vec2::ZERO.lengthSquared() == 0,      "Method lengthSquared() failed");
    CUAssertAlwaysLog(Vec2::UNIT_X.lengthSquared() == 1,    "Method lengthSquared() failed");
    CUAssertAlwaysLog(Vec2::UNIT_Y.lengthSquared() == 1,    "Method lengthSquared() failed");
    CUAssertAlwaysLog(Vec2::ONE.lengthSquared() == 2,       "Method lengthSquared() failed");
    CUAssertAlwaysLog(Vec2(-3,4).lengthSquared() == 25,     "Method lengthSquared() failed");

#pragma mark Linear Algebra Test
    CUAssertAlwaysLog(Vec2::UNIT_X.dot(Vec2::UNIT_Y) == 0,  "Method dot() failed");
    CUAssertAlwaysLog(Vec2::ONE.dot(Vec2::ZERO) == 0,       "Method dot() failed");
    CUAssertAlwaysLog(Vec2::ONE.dot(Vec2::ONE) == 2,        "Method dot() failed");
    CUAssertAlwaysLog(Vec2::UNIT_X.dot(Vec2::UNIT_X) == 1,  "Method dot() failed");
    
    test1 = Vec2::forAngle(M_PI_4);
    CUAssertAlwaysLog(CU_MATH_APPROX(test1.dot(test1),1,CU_MATH_EPSILON), "Method dot() failed");
    
    CUAssertAlwaysLog(Vec2::UNIT_X.cross(Vec2::UNIT_Y) == 1,"Method cross() failed");
    CUAssertAlwaysLog(CU_MATH_APPROX(Vec2::UNIT_X.cross(test1),1/sqrt(2),CU_MATH_EPSILON),
                      "Method cross() failed");
    CUAssertAlwaysLog(CU_MATH_APPROX(test1.cross(Vec2::UNIT_X),-1/sqrt(2),CU_MATH_EPSILON),
                      "Method cross() failed");
    
    test1 = Vec2::ONE;
    CUAssertAlwaysLog(test1.normalize().equals(Vec2::forAngle(M_PI_4)), "Method normalize() failed.");
    test1 = Vec2::UNIT_X;
    CUAssertAlwaysLog(test1.normalize().equals(Vec2::UNIT_X),           "Method normalize() failed.");
    test1 = Vec2::UNIT_Y;
    CUAssertAlwaysLog(test1.normalize().equals(Vec2::UNIT_Y),           "Method normalize() failed.");

    test1 = Vec2::ONE;
    test2 = test1.getNormalization();
    CUAssertAlwaysLog(test1 != test2,                                   "Method getNormalization() failed.");
    CUAssertAlwaysLog(test2.equals(Vec2::forAngle(M_PI_4)),             "Method getNormalization() failed.");
    test1 = Vec2::UNIT_X;
    CUAssertAlwaysLog(test1.getNormalization().equals(Vec2::UNIT_X),    "Method getNormalization() failed.");
    test1 = Vec2::UNIT_Y;
    CUAssertAlwaysLog(test1.getNormalization().equals(Vec2::UNIT_Y),    "Method getNormalization() failed.");

    test1 = Vec2::UNIT_X;
    test1.rotate(M_PI_4);
    CUAssertAlwaysLog(test1.equals(Vec2::forAngle(M_PI_4)), "Method rotate() failed.");
    test1.rotate(-M_PI_4);
    CUAssertAlwaysLog(test1.equals(Vec2::UNIT_X),           "Method rotate() failed.");

    test1 = Vec2::UNIT_X;
    test1.rotate(M_PI_4, Vec2::ZERO);
    CUAssertAlwaysLog(test1.equals(Vec2::forAngle(M_PI_4)), "Method rotate() failed.");

    test1 = Vec2::UNIT_X;
    test1.rotate(M_PI_2, Vec2::ONE);
    CUAssertAlwaysLog(test1.equals(Vec2(2,1)),              "Method rotate() failed.");
    
    test1 = Vec2::forAngle(M_PI_4);
    test2 = Vec2::forAngle(M_PI_4);
    test1.rotate(test1);
    CUAssertAlwaysLog(test1.equals(Vec2::UNIT_Y),           "Method rotate() failed.");
    
    test1.unrotate(test2);
    CUAssertAlwaysLog(test1.equals(test2),                  "Method unrotate() failed.");
    test1 = Vec2::UNIT_Y;
    test1.unrotate(Vec2::UNIT_Y);
    CUAssertAlwaysLog(test1.equals(Vec2::UNIT_X),           "Method unrotate() failed.");

    
    test1 = Vec2::UNIT_X;
    test2 = test1.getRotation(M_PI_4);
    CUAssertAlwaysLog(test1 != test2,                       "Method getRotation() failed.");
    CUAssertAlwaysLog(test2.equals(Vec2::forAngle(M_PI_4)), "Method getRotation() failed.");
    test2 = test2.getRotation(-M_PI_4);
    CUAssertAlwaysLog(test2.equals(Vec2::UNIT_X),           "Method getRotation() failed.");
    
    test1 = Vec2::UNIT_X;
    test2 = test1.getRotation(M_PI_4, Vec2::ZERO);
    CUAssertAlwaysLog(test2.equals(Vec2::forAngle(M_PI_4)), "Method getRotation() failed.");
    
    test1 = Vec2::UNIT_X;
    test2 = test1.getRotation(M_PI_2, Vec2::ONE);
    CUAssertAlwaysLog(test1 != test2,                       "Method getRotation() failed.");
    CUAssertAlwaysLog(test2.equals(Vec2(2,1)),              "Method getRotation() failed.");
    
    test1 = Vec2::forAngle(M_PI_4);
    test2 = test1.getRotation(test1);
    CUAssertAlwaysLog(test2.equals(Vec2::UNIT_Y),           "Method getRotation() failed.");
    
    test3 = test2.getUnrotation(test1);
    CUAssertAlwaysLog(test2 != test3,                       "Method getUnrotation() failed.");
    CUAssertAlwaysLog(test1.equals(test3),                  "Method getUnrotation() failed.");
    test1 = Vec2::UNIT_Y;
    test3 = test1.getUnrotation(Vec2::UNIT_Y);
    CUAssertAlwaysLog(test3.equals(Vec2::UNIT_X),           "Method getUnrotation() failed.");
    
    test1 = Vec2::ZERO; test1.perp();
    CUAssertAlwaysLog(test1 == Vec2::ZERO,                  "Method perp() failed.");
    test1 = Vec2::UNIT_X; test1.perp();
    CUAssertAlwaysLog(test1 == Vec2::UNIT_Y,                "Method perp() failed.");
    test1 = Vec2::UNIT_Y; test1.perp();
    CUAssertAlwaysLog(test1 == -Vec2::UNIT_X,               "Method perp() failed.");
    test1 = Vec2::forAngle(M_PI_4); test1.perp();
    CUAssertAlwaysLog(test1.equals(Vec2::forAngle(M_PI_2+M_PI_4)), "Method perp() failed.");

    test1 = Vec2::ZERO; test1.rperp();
    CUAssertAlwaysLog(test1 == Vec2::ZERO,                  "Method rperp() failed.");
    test1 = Vec2::UNIT_X; test1.rperp();
    CUAssertAlwaysLog(test1 == -Vec2::UNIT_Y,               "Method rperp() failed.");
    test1 = Vec2::UNIT_Y; test1.rperp();
    CUAssertAlwaysLog(test1 == Vec2::UNIT_X,                "Method rperp() failed.");
    test1 = Vec2::forAngle(M_PI_4); test1.rperp();
    CUAssertAlwaysLog(test1.equals(Vec2::forAngle(-M_PI_4)),"Method rperp() failed.");
    
    test1 = Vec2::ZERO; test2 = test1.getPerp();
    CUAssertAlwaysLog(test2 == Vec2::ZERO,                  "Method getPerp() failed.");
    test1 = Vec2::UNIT_X; test2 = test1.getPerp();
    CUAssertAlwaysLog(test1 != test2,                       "Method getPerp() failed.");
    CUAssertAlwaysLog(test2 == Vec2::UNIT_Y,                "Method getPerp() failed.");
    test1 = Vec2::UNIT_Y; test2 = test1.getPerp();
    CUAssertAlwaysLog(test2 == -Vec2::UNIT_X,               "Method getPerp() failed.");
    test1 = Vec2::forAngle(M_PI_4); test2 = test1.getPerp();
    CUAssertAlwaysLog(test2.equals(Vec2::forAngle(M_PI_2+M_PI_4)), "Method getPerp() failed.");
    
    test1 = Vec2::ZERO; test2 = test1.getRPerp();
    CUAssertAlwaysLog(test2 == Vec2::ZERO,                  "Method getRPerp() failed.");
    test1 = Vec2::UNIT_X; test2 = test1.getRPerp();
    CUAssertAlwaysLog(test1 != test2,                       "Method getRPerp() failed.");
    CUAssertAlwaysLog(test2 == -Vec2::UNIT_Y,               "Method getRPerp() failed.");
    test1 = Vec2::UNIT_Y; test2 = test1.getRPerp();
    CUAssertAlwaysLog(test2 == Vec2::UNIT_X,                "Method getRPerp() failed.");
    test1 = Vec2::forAngle(M_PI_4); test2 = test1.getRPerp();
    CUAssertAlwaysLog(test2.equals(Vec2::forAngle(-M_PI_4)),"Method getRPerp() failed.");

    test1 = Vec2::ZERO;
    test2 = test1.getMidpoint(Vec2::ONE);
    CUAssertAlwaysLog(test1 != test2,                       "Method getMidpoint() failed.");
    CUAssertAlwaysLog(test2 == Vec2(0.5,0.5),               "Method getMidpoint() failed.");

    test1 = Vec2::UNIT_X.getMidpoint(Vec2::UNIT_Y);
    test2 = Vec2::UNIT_Y.getMidpoint(Vec2::UNIT_X);
    CUAssertAlwaysLog(test1 == Vec2(0.5,0.5),               "Method getMidpoint() failed.");
    CUAssertAlwaysLog(test2 == Vec2(0.5,0.5),               "Method getMidpoint() failed.");

    test1.set(2,3);
    test1.project(Vec2::UNIT_X);
    CUAssertAlwaysLog(test1 == Vec2(2,0),                   "Method project() failed.");
    test1.set(2,3);
    test1.project(Vec2::UNIT_Y);
    CUAssertAlwaysLog(test1 == Vec2(0,3),                   "Method project() failed.");
    test1 = Vec2::UNIT_X;
    test1.project(Vec2(1,1));
    CUAssertAlwaysLog(test1 == Vec2(0.5,0.5),               "Method project() failed.");
    
    test1.set(2,3);
    test2 = test1.getProjection(Vec2::UNIT_X);
    CUAssertAlwaysLog(test1 != test2,                       "Method getProjection() failed.");
    CUAssertAlwaysLog(test2 == Vec2(2,0),                   "Method getProjection() failed.");
    test2 = test1.getProjection(Vec2::UNIT_Y);
    CUAssertAlwaysLog(test2 == Vec2(0,3),                   "Method getProjection() failed.");
    test1 = Vec2::UNIT_X;
    test2 = test1.getProjection(Vec2(1,1));
    CUAssertAlwaysLog(test2 == Vec2(0.5,0.5),               "Method getProjection() failed.");

    test1 = Vec2::ONE;
    test2.set(2,3);
    test1.lerp(test2,0);
    CUAssertAlwaysLog(test1 == Vec2::ONE,                   "Method lerp() failed.");
    test1.lerp(test2,1);
    CUAssertAlwaysLog(test1 == test2,                       "Method lerp() failed.");
    test1 = Vec2::ONE;
    test1.lerp(test2,0.5);
    CUAssertAlwaysLog(test1 == Vec2(1.5,2),                 "Method lerp() failed.");
    test1 = Vec2::ONE;
    test1.lerp(test2,-1);
    CUAssertAlwaysLog(test1 == Vec2(0,-1),                  "Method lerp() failed.");
    test1 = Vec2::ONE;
    test1.lerp(test2,2);
    CUAssertAlwaysLog(test1 == Vec2(3,5),                   "Method lerp() failed.");

    test1 = Vec2::ONE;
    test2.set(2,3);
    test3 = test1.getLerp(test2,0);
    CUAssertAlwaysLog(test3 == test1,                       "Method getLerp() failed.");
    test3 = test1.getLerp(test2,1);
    CUAssertAlwaysLog(test1 != test3,                       "Method getLerp() failed.");
    CUAssertAlwaysLog(test3 == test2,                       "Method getLerp() failed.");
    test3 = test1.getLerp(test2,0.5);
    CUAssertAlwaysLog(test3 == Vec2(1.5,2),                 "Method getLerp() failed.");
    test3 = test1.getLerp(test2,-1);
    CUAssertAlwaysLog(test3 == Vec2(0,-1),                  "Method getLerp() failed.");
    test3 = test1.getLerp(test2,2);
    CUAssertAlwaysLog(test3 == Vec2(3,5),                   "Method getLerp() failed.");

#pragma mark Static Linear Algebra Test
    CUAssertAlwaysLog(Vec2::dot(Vec2::UNIT_X,Vec2::UNIT_Y) == 0,  "Vec2::dot() failed");
    CUAssertAlwaysLog(Vec2::dot(Vec2::ONE,Vec2::ZERO) == 0,       "Vec2::dot() failed");
    CUAssertAlwaysLog(Vec2::dot(Vec2::ONE,Vec2::ONE) == 2,        "Vec2::dot() failed");
    CUAssertAlwaysLog(Vec2::dot(Vec2::UNIT_X,Vec2::UNIT_X) == 1,  "Vec2::dot() failed");
    
    test1 = Vec2::forAngle(M_PI_4);
    CUAssertAlwaysLog(CU_MATH_APPROX(Vec2::dot(test1,test1),1,CU_MATH_EPSILON), "Vec2::dot() failed");
    
    CUAssertAlwaysLog(Vec2::cross(Vec2::UNIT_X,Vec2::UNIT_Y) == 1,"Vec2::cross() failed");
    CUAssertAlwaysLog(CU_MATH_APPROX(Vec2::cross(Vec2::UNIT_X,test1),1/sqrt(2),CU_MATH_EPSILON),
                      "Vec2::cross() failed");
    CUAssertAlwaysLog(CU_MATH_APPROX(Vec2::cross(test1,Vec2::UNIT_X),-1/sqrt(2),CU_MATH_EPSILON),
                      "Vec2::cross() failed");

    test1.set(1/sqrtf(2.0),1/sqrtf(2.0));
    testptr = Vec2::normalize(Vec2::ONE,&test2);
    CUAssertAlwaysLog(testptr == &test2,                    "Vec2::normalize() failed");
    CUAssertAlwaysLog(test2.equals(test1),                  "Vec2::normalize() failed.");
    Vec2::normalize(Vec2::UNIT_X,&test2);
    CUAssertAlwaysLog(test2.equals(Vec2::UNIT_X),           "Vec2::normalize() failed.");
    Vec2::normalize(Vec2::UNIT_Y,&test2);
    CUAssertAlwaysLog(test2.equals(Vec2::UNIT_Y),           "Vec2::normalize() failed.");

    test1 = Vec2::ZERO;
    testptr = Vec2::midpoint(test1,Vec2::ONE,&test2);
    CUAssertAlwaysLog(testptr == &test2,                    "Vec2::midpoint() failed");
    CUAssertAlwaysLog(test2 == Vec2(0.5,0.5),               "Vec2::midpoint() failed.");
    
    Vec2::midpoint(Vec2::UNIT_X,Vec2::UNIT_Y, &test1);
    Vec2::midpoint(Vec2::UNIT_Y,Vec2::UNIT_X, &test2);
    CUAssertAlwaysLog(test1 == Vec2(0.5,0.5),               "Vec2::midpoint() failed.");
    CUAssertAlwaysLog(test2 == Vec2(0.5,0.5),               "Vec2::midpoint() failed.");
    
    test1.set(2,3);
    testptr = Vec2::project(test1,Vec2::UNIT_X,&test2);
    CUAssertAlwaysLog(testptr == &test2,                    "Vec2::project() failed");
    CUAssertAlwaysLog(test2 == Vec2(2,0),                   "Vec2::project() failed.");
    Vec2::project(test1, Vec2::UNIT_Y, &test2);
    CUAssertAlwaysLog(test2 == Vec2(0,3),                   "Vec2::project() failed.");
    Vec2::project(Vec2::UNIT_X, Vec2(1,1), &test2);
    CUAssertAlwaysLog(test2 == Vec2(0.5,0.5),               "Vec2::project() failed.");
    
    test1 = Vec2::ONE;
    test2.set(2,3);
    testptr = Vec2::lerp(test1, test2, 0, &test3);
    CUAssertAlwaysLog(testptr == &test3,                    "Vec2::lerp() failed");
    CUAssertAlwaysLog(test3 == test1,                       "Vec2::lerp() failed.");
    Vec2::lerp(test1, test2, 1, &test3);
    CUAssertAlwaysLog(test3 == test2,                       "Vec2::lerp() failed.");
    Vec2::lerp(test1, test2, 0.5, &test3);
    CUAssertAlwaysLog(test3 == Vec2(1.5,2),                 "Vec2::lerp() failed.");
    Vec2::lerp(test1, test2, -1, &test3);
    CUAssertAlwaysLog(test3 == Vec2(0,-1),                  "Vec2::lerp() failed.");
    Vec2::lerp(test1, test2, 2, &test3);
    CUAssertAlwaysLog(test3 == Vec2(3,5),                   "Vec2::lerp() failed.");

    // Do the line segment tests in clusters
    float s, t;
    bool result;
    Vec2 test6;
    
    test1 = Vec2::ZERO;   test2 = test1+2*Vec2::ONE;
    test3 = Vec2::UNIT_Y; test4 = test3+2*Vec2::UNIT_X;
    result = Vec2::doesLineIntersect(test1,test2,test3,test4, &s, &t);
    CUAssertAlwaysLog(result,                               "Method doesLineIntersect() fails");
    test5 = test1+s*(test2-test1);
    test6 = test3+t*(test4-test3);
    CUAssertAlwaysLog(test5.equals(test6),                  "Method doesLineIntersect() fails");
    CUAssertAlwaysLog(test5.equals(Vec2::ONE),              "Method doesLineIntersect() fails");
    test6 = Vec2::getIntersection(test1,test2,test3,test4);
    CUAssertAlwaysLog(test6.equals(test5),                  "Method getIntersection() fails");
    result = Vec2::doesLineOverlap(test1,test2,test3,test4);
    CUAssertAlwaysLog(!result,                              "Method doesLineOverlap() fails");
    result = Vec2::isLineParallel(test1,test2,test3,test4);
    CUAssertAlwaysLog(!result,                              "Method isLineParallel() fails");
    result = Vec2::doesSegmentIntersect(test1,test2,test3,test4);
    CUAssertAlwaysLog(result,                               "Method doesSegmentIntersect() fails");
    result = Vec2::doesSegmentOverlap(test1,test2,test3,test4, &test5, &test6);
    CUAssertAlwaysLog(!result,                              "Method doesSegmentOverlap() fails");

    test1 = Vec2::ZERO;   test2 = test1-2*Vec2::ONE;
    test3 = Vec2::UNIT_Y; test4 = test3+2*Vec2::UNIT_X;
    result = Vec2::doesLineIntersect(test1,test2,test3,test4, &s, &t);
    CUAssertAlwaysLog(result,                               "Method doesLineIntersect() fails");
    test5 = test1+s*(test2-test1);
    test6 = test3+t*(test4-test3);
    CUAssertAlwaysLog(test5.equals(test6),                  "Method doesLineIntersect() fails");
    CUAssertAlwaysLog(test5.equals(Vec2::ONE),              "Method doesLineIntersect() fails");
    test6 = Vec2::getIntersection(test1,test2,test3,test4);
    CUAssertAlwaysLog(test6.equals(test5),                  "Method getIntersection() fails");
    result = Vec2::doesLineOverlap(test1,test2,test3,test4);
    CUAssertAlwaysLog(!result,                              "Method doesLineOverlap() fails");
    result = Vec2::isLineParallel(test1,test2,test3,test4);
    CUAssertAlwaysLog(!result,                              "Method isLineParallel() fails");
    result = Vec2::doesSegmentIntersect(test1,test2,test3,test4);
    CUAssertAlwaysLog(!result,                              "Method doesSegmentIntersect() fails");
    result = Vec2::doesSegmentOverlap(test1,test2,test3,test4, &test5, &test6);
    CUAssertAlwaysLog(!result,                              "Method doesSegmentOverlap() fails");

    test1 = Vec2::ZERO; test2 = test1+2*Vec2::UNIT_X;
    test3 = Vec2::ZERO; test4 = test3+Vec2::UNIT_X;
    result = Vec2::doesLineIntersect(test1,test2,test3,test4, &s, &t);
    CUAssertAlwaysLog(!result,                              "Method doesLineIntersect() fails");
    result = Vec2::doesLineOverlap(test1,test2,test3,test4);
    CUAssertAlwaysLog(result,                               "Method doesLineOverlap() fails");
    result = Vec2::isLineParallel(test1,test2,test3,test4);
    CUAssertAlwaysLog(!result,                              "Method isLineParallel() fails");
    result = Vec2::doesSegmentIntersect(test1,test2,test3,test4);
    CUAssertAlwaysLog(!result,                              "Method doesSegmentIntersect() fails");
    result = Vec2::doesSegmentOverlap(test1,test2,test3,test4, &test5, &test6);
    CUAssertAlwaysLog(result,                               "Method doesSegmentOverlap() fails");
    CUAssertAlwaysLog(test5.equals(Vec2::ZERO),             "Method doesLineIntersect() fails");
    CUAssertAlwaysLog(test6.equals(Vec2::UNIT_X),           "Method doesLineIntersect() fails");


    test1 = Vec2::ZERO;   test2 = test1+Vec2::UNIT_X;
    test3 = Vec2::UNIT_Y; test4 = test3+Vec2::UNIT_X;
    result = Vec2::doesLineIntersect(test1,test2,test3,test4, &s, &t);
    CUAssertAlwaysLog(!result,                              "Method doesLineIntersect() fails");
    result = Vec2::doesLineOverlap(test1,test2,test3,test4);
    CUAssertAlwaysLog(!result,                              "Method doesLineOverlap() fails");
    result = Vec2::isLineParallel(test1,test2,test3,test4);
    CUAssertAlwaysLog(result,                               "Method isLineParallel() fails");
    result = Vec2::doesSegmentIntersect(test1,test2,test3,test4);
    CUAssertAlwaysLog(!result,                              "Method doesSegmentIntersect() fails");
    result = Vec2::doesSegmentOverlap(test1,test2,test3,test4, &test5, &test6);
    CUAssertAlwaysLog(!result,                              "Method doesSegmentOverlap() fails");
    
#pragma mark Conversion Test
    std::string str;
    std::string a, b;
    test1.set(2,3); str = test1.toString();
    a = cugl::to_string(2.0f); b = cugl::to_string(3.0f);
    CUAssertAlwaysLog(str == "("+a+","+b+")",               "Method toString() failed");
    str = test1.toString(true);
    CUAssertAlwaysLog(str == "cugl::Vec2("+a+","+b+")",     "Method toString() failed");
    str = (std::string)test1;
    CUAssertAlwaysLog(str == "("+a+","+b+")",               "String cast failed");
    
    Size size1 = (Size)test1;
    Size size2(1,2);
    CUAssertAlwaysLog(size1.width == 2 && size1.height == 3,"Size cast failed");
    Vec2 test7(size1);
    CUAssertAlwaysLog(test7.x == 2 && test7.y == 3,         "Size constructor failed");
    test6 = size1;
    CUAssertAlwaysLog(test7 == test6,                       "Size assignment failed");
    test6 = size2;
    CUAssertAlwaysLog(test6.x == 1 && test6.y == 2,         "Size assignment failed");
    
    test2 = Vec2::ONE;
    test2 += size1;
    CUAssertAlwaysLog(test2 == Vec2(3,4),                   "Size addition failed");
    test2 = Vec2::ONE;
    test3 = test2+size1;
    CUAssertAlwaysLog(test2 != test3,                       "Size addition failed");
    CUAssertAlwaysLog(test3 == Vec2(3,4),                   "Size addition failed");

    test2 = Vec2::ONE;
    test2 -= size1;
    CUAssertAlwaysLog(test2 == Vec2(-1,-2),                 "Size subtraction failed");
    test2 = Vec2::ONE;
    test3 = test2-size1;
    CUAssertAlwaysLog(test2 != test3,                       "Size subtraction failed");
    CUAssertAlwaysLog(test3 == Vec2(-1,-2),                 "Size subtraction failed");

    test1.set(3,5);
    Vec3 v3test = (Vec3)test1;
    CUAssertAlwaysLog(v3test.x == 3 && v3test.y == 5 && v3test.z == 0, "Vec3 cast failed");
    Vec2 test8(v3test);
    CUAssertAlwaysLog(test8 == test1,                       "Vec3 constructor failed");
    test7 = v3test;
    CUAssertAlwaysLog(test7 == test1,                       "Vec3 assignment failed");

    test1.set(-4,8);
    Vec4 v4test = (Vec4)test1;
    CUAssertAlwaysLog(v4test.x == -4 && v4test.y == 8 && v4test.z == 0 && v4test.w == 1, "Vec4 cast failed");
    Vec2 test9(v4test);
    CUAssertAlwaysLog(test9 == test1,                       "Vec4 constructor failed");
    test7 = v4test;
    CUAssertAlwaysLog(test7 == test1,                       "Vec4 assignment failed");

    
#pragma mark Complete
    CULog("Vec2 tests complete.\n");

}

#pragma mark -
#pragma mark Vec3

/**
 * Unit test for a 3-dimension vector
 */
void cugl::testVec3() {
    CULog("Running tests for Vec3.\n");
        
#pragma mark Constructor Test
    // Initial test of constructors
    Vec3 test1;
    CUAssertAlwaysLog(test1.x == 0 && test1.y == 0 && test1.z == 0,         "Trivial constructor failed");
        
    Vec3 test2(1.5,4,-2.5);
    CUAssertAlwaysLog(test2.x == 1.5 && test2.y == 4 && test2.z == -2.5,    "Initialization constructor failed");
        
    float f[3] = {3.5, 6, 0.5};
    Vec3 test3(f);
    CUAssertAlwaysLog(test3.x == 3.5 && test3.y == 6  && test3.z == 0.5,    "Array constructor failed");
        
    Vec3 test4(test2);
    CUAssertAlwaysLog(test4.x == 1.5 && test4.y == 4 && test4.z == -2.5,    "Copy constructor failed");
    
    Vec3 test5(test2,test3);
    CUAssertAlwaysLog(test5.x == 2 && test5.y == 2 && test5.z == 3,         "Directional constructor failed");
        
        
#pragma mark Constants Test
    CUAssertAlwaysLog(Vec3::ZERO.x == 0 && Vec3::ZERO.y == 0 && Vec3::ZERO.z == 0,          "Zero vector failed");
    CUAssertAlwaysLog(Vec3::ONE.x == 1 && Vec3::ONE.y == 1 && Vec3::ONE.z == 1,             "Ones vector failed");
    CUAssertAlwaysLog(Vec3::UNIT_X.x == 1 && Vec3::UNIT_X.y == 0 && Vec3::UNIT_X.z == 0 ,   "X-axis vector failed");
    CUAssertAlwaysLog(Vec3::UNIT_Y.x == 0 && Vec3::UNIT_Y.y == 1 && Vec3::UNIT_Y.z == 0,    "Y-axis vector failed");
    CUAssertAlwaysLog(Vec3::UNIT_Z.x == 0 && Vec3::UNIT_Z.y == 0 && Vec3::UNIT_Z.z == 1,    "Z-axis vector failed");
    
        
#pragma mark Setter Test
    test1 = test2;
    CUAssertAlwaysLog(test1.x == 1.5 && test1.y == 4 && test1.z == -2.5,    "Basic assignment failed");
    
    test1 = f;
    CUAssertAlwaysLog(test1.x == 3.5 && test1.y == 6  && test1.z == 0.5,    "Float assignment failed");
    
    test1.set(-1,1,5);
    CUAssertAlwaysLog(test1.x == -1 && test1.y == 1 && test1.z == 5,        "Parameter assignment failed");
        
    test1.set(test2);
    CUAssertAlwaysLog(test1.x == 1.5 && test1.y == 4 && test1.z == -2.5,    "Alternate assignment failed");
    
    test1.set(f);
    CUAssertAlwaysLog(test1.x == 3.5 && test1.y == 6  && test1.z == 0.5,    "Alternate float assignment failed");
    
    test1.set(test2,test3);
    CUAssertAlwaysLog(test1.x == 2 && test1.y == 2 && test1.z == 3,         "Directional assignment failed");
    
    test1.setZero();
    CUAssertAlwaysLog(test1.x == 0 && test1.y == 0 && test1.z == 0,         "Erasing assignment failed");
        

#pragma mark Comparison Test
    test1.set(0,0,0); test2.set(0,1,1); test3.set(1,1,0); test4.set(1,1,1);
        
    CUAssertAlwaysLog(test1 < test4,    "Less than failed");
    CUAssertAlwaysLog(!(test4 < test1), "Less than failed");
    CUAssertAlwaysLog(test1 < test2,    "Less than failed");
    CUAssertAlwaysLog(test2 < test3,    "Less than failed");
    CUAssertAlwaysLog(!(test1 < test1), "Less than failed");
        
    CUAssertAlwaysLog(test1 <= test4,   "Less than or equal to failed");
    CUAssertAlwaysLog(!(test4 <= test1),"Less than or equal to failed");
    CUAssertAlwaysLog(test1 <= test2,   "Less than or equal to failed");
    CUAssertAlwaysLog(test2 <= test3,   "Less than or equal to failed");
    CUAssertAlwaysLog(test1 <= test1,   "Less than or equal to failed");
        
    CUAssertAlwaysLog(test4 > test1,    "Greater than failed");
    CUAssertAlwaysLog(!(test1 > test4), "Greater than failed");
    CUAssertAlwaysLog(test2 > test1,    "Greater than failed");
    CUAssertAlwaysLog(test3 > test2,    "Greater than failed");
    CUAssertAlwaysLog(!(test1 > test1), "Greater than failed");
        
    CUAssertAlwaysLog(test4 >= test1,   "Greater than or equal to failed");
    CUAssertAlwaysLog(!(test1 >= test4),"Greater than or equal to failed");
    CUAssertAlwaysLog(test2 >= test1,   "Greater than or equal to failed");
    CUAssertAlwaysLog(test3 >= test2,   "Greater than or equal to failed");
    CUAssertAlwaysLog(test1 >= test1,   "Greater than or equal to failed");
        
    CUAssertAlwaysLog(test1 == test1,   "Equals failed");
    CUAssertAlwaysLog(test2 == test2,   "Equals failed");
    CUAssertAlwaysLog(test3 == test3,   "Equals failed");
    CUAssertAlwaysLog(!(test1 == test4),"Equals failed");
        
    CUAssertAlwaysLog(!(test1 != test1),"Not equals failed");
    CUAssertAlwaysLog(!(test2 != test2),"Not equals failed");
    CUAssertAlwaysLog(!(test3 != test3),"Not equals failed");
    CUAssertAlwaysLog(test1 != test4,   "Not equals failed");
        
    CUAssertAlwaysLog(test1.under(test4),   "Method under() failed");
    CUAssertAlwaysLog(!test4.under(test1),  "Method under() failed");
    CUAssertAlwaysLog(!test2.under(test3),  "Method under() failed");
    CUAssertAlwaysLog(!test3.under(test2),  "Method under() failed");
    CUAssertAlwaysLog(test1.under(test1),   "Method under() failed");
        
    CUAssertAlwaysLog(test4.over(test1),    "Method over() failed");
    CUAssertAlwaysLog(!test1.over(test4),   "Method over() failed");
    CUAssertAlwaysLog(!test2.over(test3),   "Method over() failed");
    CUAssertAlwaysLog(!test3.over(test2),   "Method over() failed");
    CUAssertAlwaysLog(test1.over(test1),    "Method over() failed");
        
    test5.set(0,CU_MATH_EPSILON*0.5,-CU_MATH_EPSILON*0.5);
    CUAssertAlwaysLog(test1.equals(test1), "Approximate equals failed");
    CUAssertAlwaysLog(test1.equals(test5), "Approximate equals failed");
        
        
#pragma mark Static Arithmetic Test
    Vec3* testptr;

    test1.set(-2,2,-3);
    testptr = Vec3::clamp(test1,Vec3(-3,-3,-4),Vec3(3,3,4),&test2);
    CUAssertAlwaysLog(test1 == test2,                                   "Vec3::clamp() failed");
    CUAssertAlwaysLog(testptr == &test2,                                "Vec3::clamp() failed");
        
    Vec3::clamp(test1,Vec3::ZERO,Vec3(3,3,4),&test2);
    CUAssertAlwaysLog(test1 != test2,                                   "Vec3::clamp() failed");
    CUAssertAlwaysLog(test2.x == 0 && test2.y == 2 && test2.z == 0,     "Vec3::clamp() failed");
        
    Vec3::clamp(test1,Vec3(-3,-3,-4),Vec3::ZERO,&test2);
    CUAssertAlwaysLog(test1 != test2,                                   "Vec3::clamp() failed");
    CUAssertAlwaysLog(test2.x == -2 && test2.y == 0 && test2.z == -3,   "Vec3::clamp() failed");
        
    Vec3::clamp(test1,Vec3(-1,-1,-2),Vec3(1,1,2),&test2);
    CUAssertAlwaysLog(test1 != test2,                                   "Vec3::clamp() failed");
    CUAssertAlwaysLog(test2.x == -1 && test2.y == 1 && test2.z == -2,   "Vec3::clamp() failed");
        
    float angle = Vec3::angle(Vec3::UNIT_X,Vec3::UNIT_Z);
    CUAssertAlwaysLog(CU_MATH_APPROX(angle,M_PI_2,CU_MATH_EPSILON),       "Vec3::angle failed");
    angle = Vec3::angle(Vec3::UNIT_Y,Vec3::UNIT_X);
    CUAssertAlwaysLog(CU_MATH_APPROX(angle,-M_PI_2,CU_MATH_EPSILON),      "Vec3::angle failed");
    angle = Vec3::angle(Vec3::UNIT_Y,Vec3::UNIT_X,-Vec3::UNIT_Z);
    CUAssertAlwaysLog(CU_MATH_APPROX(angle,M_PI_2,CU_MATH_EPSILON),       "Vec3::angle failed");
    angle = Vec3::angle(Vec3::ONE,Vec3::UNIT_X);
    CUAssertAlwaysLog(CU_MATH_APPROX(angle,-0.955316603,CU_MATH_EPSILON), "Vec3::angle failed");

    testptr = Vec3::add(Vec3::UNIT_X,Vec3::UNIT_Z,&test1);
    CUAssertAlwaysLog(test1 == Vec3(1,0,1),                 "Vec3::add() failed");
    CUAssertAlwaysLog(testptr == &test1,                    "Vec3::add() failed");
        
    test1.set(2,2,2);
    Vec3::add(Vec3::ONE,Vec3::ONE,&test2);
    CUAssertAlwaysLog(test1 == test2,                       "Vec3::add() failed");
        
    test1.set(1,0,-1);
    testptr = Vec3::subtract(Vec3::UNIT_X,Vec3::UNIT_Z,&test2);
    CUAssertAlwaysLog(test1 == test2,                       "Vec3::subtract() failed");
    CUAssertAlwaysLog(testptr == &test2,                    "Vec3::subtract() failed");
        
    test1.set(2,2,2);
    Vec3::subtract(Vec3::ONE,Vec3::ONE,&test1);
    CUAssertAlwaysLog(test1 == Vec3::ZERO,                  "Vec3::subtract() failed");
    
    testptr = Vec3::scale(Vec3::ONE,2,&test1);
    CUAssertAlwaysLog(testptr == &test1,                    "Vec3::scale() failed");
    CUAssertAlwaysLog(test1 == Vec3(2,2,2),                 "Vec3::scale() failed");
    Vec3::scale(Vec3::UNIT_X,2,&test1);
    CUAssertAlwaysLog(test1 == Vec3(2,0,0),                 "Vec3::scale() failed");
    Vec3::scale(Vec3::UNIT_Y,2,&test1);
    CUAssertAlwaysLog(test1 == Vec3(0,2,0),                 "Vec3::scale() failed");
    Vec3::scale(Vec3::UNIT_Z,2,&test1);
    CUAssertAlwaysLog(test1 == Vec3(0,0,2),                 "Vec3::scale() failed");
    
    test2.set(-0.5,0.5,1.5);
    testptr = Vec3::scale(Vec3::ONE,test2,&test1);
    CUAssertAlwaysLog(testptr == &test1,                    "Vec3::scale() failed");
    CUAssertAlwaysLog(test1 == Vec3(-0.5,0.5,1.5),          "Vec3::scale() failed");
    Vec3::scale(Vec3::UNIT_X,test2,&test1);
    CUAssertAlwaysLog(test1 == Vec3(-0.5,0,0),              "Vec3::scale() failed");
    Vec3::scale(Vec3::UNIT_Y,test2,&test1);
    CUAssertAlwaysLog(test1 == Vec3(0,0.5,0),               "Vec3::scale() failed");
    Vec3::scale(Vec3::UNIT_Z,test2,&test1);
    CUAssertAlwaysLog(test1 == Vec3(0,0,1.5),               "Vec3::scale() failed");
    
    testptr = Vec3::divide(Vec3::ONE,2,&test1);
    CUAssertAlwaysLog(testptr == &test1,                    "Vec3::divide() failed");
    CUAssertAlwaysLog(test1 == Vec3(0.5,0.5,0.5),           "Vec3::divide() failed");
    Vec3::divide(Vec3::UNIT_X,2,&test1);
    CUAssertAlwaysLog(test1 == Vec3(0.5,0,0),               "Vec3::divide() failed");
    Vec3::divide(Vec3::UNIT_Y,2,&test1);
    CUAssertAlwaysLog(test1 == Vec3(0,0.5,0),               "Vec3::divide() failed");
    Vec3::divide(Vec3::UNIT_Z,2,&test1);
    CUAssertAlwaysLog(test1 == Vec3(0,0,0.5),               "Vec3::divide() failed");
    
    test2.set(-0.5,0.5,0.25);
    testptr = Vec3::divide(Vec3::ONE,test2,&test1);
    CUAssertAlwaysLog(testptr == &test1,                    "Vec3::divide() failed");
    CUAssertAlwaysLog(test1 == Vec3(-2,2,4),                "Vec3::divide() failed");
    Vec3::divide(Vec3::UNIT_X,test2,&test1);
    CUAssertAlwaysLog(test1 == Vec3(-2,0,0),                "Vec3::divide() failed");
    Vec3::divide(Vec3::UNIT_Y,test2,&test1);
    CUAssertAlwaysLog(test1 == Vec3(0,2,0),                 "Vec3::divide() failed");
    Vec3::divide(Vec3::UNIT_Z,test2,&test1);
    CUAssertAlwaysLog(test1 == Vec3(0,0,4),                 "Vec3::divide() failed");
    
    testptr = Vec3::negate(Vec3::ONE,&test1);
    CUAssertAlwaysLog(testptr == &test1,                    "Vec3::negate() failed");
    CUAssertAlwaysLog(test1 == Vec3(-1,-1,-1),              "Vec3::negate() failed");
    Vec3::negate(Vec3::UNIT_X,&test1);
    CUAssertAlwaysLog(test1 == Vec3(-1, 0, 0),              "Vec3::negate() failed");
    Vec3::negate(Vec3::UNIT_Y,&test1);
    CUAssertAlwaysLog(test1 == Vec3( 0,-1, 0),              "Vec3::negate() failed");
    Vec3::negate(Vec3::UNIT_Z,&test1);
    CUAssertAlwaysLog(test1 == Vec3( 0, 0,-1),              "Vec3::negate() failed");
    
    test1.set(2,2,2);
    testptr = Vec3::reciprocate(test1,&test2);
    CUAssertAlwaysLog(testptr == &test2,                    "Vec3::reciprocate() failed");
    CUAssertAlwaysLog(test2 == Vec3(0.5,0.5,0.5),           "Vec3::reciprocate() failed");
    Vec3::reciprocate(Vec3::ONE,&test2);
    CUAssertAlwaysLog(test2 == Vec3::ONE,                   "Vec3::reciprocate() failed");
    
#pragma mark Arithmetic Test
    test1.set(-2,2,-3);
    test2.set(-2,2,-3);
    test2.clamp(Vec3(-3,-3,-4),Vec3(3,3,4));
    CUAssertAlwaysLog(test1 == test2,                               "Method clamp() failed");
        
    test2.clamp(Vec3::ZERO,Vec3(3,3,4));
    CUAssertAlwaysLog(test1 != test2,                               "Method clamp() failed");
    CUAssertAlwaysLog(test2.x == 0 && test2.y == 2 && test2.z == 0, "Method clamp() failed");
        
    test2 = test1;
    test2.clamp(Vec3(-3,-3,-4),Vec3::ZERO);
    CUAssertAlwaysLog(test1 != test2,                               "Method clamp() failed");
    CUAssertAlwaysLog(test2.x==-2 && test2.y == 0 && test2.z == -3, "Method clamp() failed");
        
    test2 = test1;
    test2.clamp(Vec3(-1,-1,-2),Vec3(1,1,2));
    CUAssertAlwaysLog(test1 != test2,                               "Method clamp() failed");
    CUAssertAlwaysLog(test2.x==-1 && test2.y == 1 && test2.z == -2, "Method clamp() failed");
    
    test2 = test1;
    test3 = test2.getClamp(Vec3::ZERO,Vec3(3,3,4));
    CUAssertAlwaysLog(test1 == test2,                               "Method getClamp() failed");
    CUAssertAlwaysLog(test1 != test3,                               "Method getClamp() failed");
    CUAssertAlwaysLog(test3.x == 0 && test3.y == 2 && test3.z == 0, "Method clamp() failed");
    
    test3 = test2.getClamp(Vec3(-3,-3,-4),Vec3::ZERO);
    CUAssertAlwaysLog(test1 == test2,                               "Method getClamp() failed");
    CUAssertAlwaysLog(test1 != test3,                               "Method getClamp() failed");
    CUAssertAlwaysLog(test3.x==-2 && test3.y == 0 && test3.z == -3, "Method getClamp() failed");

    test3 = test2.getClamp(Vec3(-1,-1,-2),Vec3(1,1,2));
    CUAssertAlwaysLog(test1 == test2,                               "Method getClamp() failed");
    CUAssertAlwaysLog(test1 != test3,                               "Method getClamp() failed");
    CUAssertAlwaysLog(test3.x==-1 && test3.y == 1 && test3.z == -2, "Method getClamp() failed");
    
    test1 = Vec3::UNIT_X;
    test1.add(Vec3::UNIT_Y);
    test1.add(Vec3::UNIT_Z);
    CUAssertAlwaysLog(test1 == Vec3::ONE,                           "Method add() failed");
        
    test1 = Vec3::ONE;
    test1.add(test1);
    CUAssertAlwaysLog(test1 == Vec3(2,2,2),                         "Method add() failed");
        
    test1 = Vec3::ONE;
    test1.add(2,3,-2);
    CUAssertAlwaysLog(test1 == Vec3(3,4,-1),                        "Method add() failed");
        
    test1 = Vec3::UNIT_X;
    test1.subtract(Vec3::UNIT_Z);
    CUAssertAlwaysLog(test1 == Vec3(1,0,-1),                        "Method subtract() failed");
        
    test1 = Vec3::ONE;
    test1.subtract(test1);
    CUAssertAlwaysLog(test1 == Vec3::ZERO,                          "Method subtract() failed");
        
    test1 = Vec3::ONE;
    test1.subtract(2,3,-1);
    CUAssertAlwaysLog(test1 == Vec3(-1,-2,2),                       "Method subtract() failed");

    test1 = Vec3::ONE;
    test2 = Vec3::UNIT_X;
    test3 = Vec3::UNIT_Y;
    test4 = Vec3::UNIT_Z;
    test1.scale(2); test2.scale(2); test3.scale(2); test4.scale(2);
    CUAssertAlwaysLog(test1 == Vec3(2,2,2),             "Method scale() failed");
    CUAssertAlwaysLog(test2 == Vec3(2,0,0),             "Method scale() failed");
    CUAssertAlwaysLog(test3 == Vec3(0,2,0),             "Method scale() failed");
    CUAssertAlwaysLog(test4 == Vec3(0,0,2),             "Method scale() failed");
    
    test1 = Vec3::ONE;
    test2 = Vec3::UNIT_X;
    test3 = Vec3::UNIT_Y;
    test4 = Vec3::UNIT_Z;
    test1.scale(2,3,-1); test2.scale(2,3,-1); test3.scale(2,3,-1); test4.scale(2,3,-1);
    CUAssertAlwaysLog(test1 == Vec3(2,3,-1),            "Method scale() failed");
    CUAssertAlwaysLog(test2 == Vec3(2,0,0),             "Method scale() failed");
    CUAssertAlwaysLog(test3 == Vec3(0,3,0),             "Method scale() failed");
    CUAssertAlwaysLog(test4 == Vec3(0,0,-1),            "Method scale() failed");
    
    test1 = Vec3::ONE;
    test2 = Vec3::UNIT_X;
    test3 = Vec3::UNIT_Y;
    test4 = Vec3::UNIT_Z;
    test5 = Vec3(-0.5,0.5,1.5);
    test1.scale(test5); test2.scale(test5); test3.scale(test5); test4.scale(test5);
    CUAssertAlwaysLog(test1 == Vec3(-0.5,0.5,1.5),      "Method scale() failed");
    CUAssertAlwaysLog(test2 == Vec3(-0.5,0,0),          "Method scale() failed");
    CUAssertAlwaysLog(test3 == Vec3(0,0.5,0),           "Method scale() failed");
    CUAssertAlwaysLog(test4 == Vec3(0,0,1.5),           "Method scale() failed");
    
    test1 = Vec3::ONE;
    test2 = Vec3::UNIT_X;
    test3 = Vec3::UNIT_Y;
    test4 = Vec3::UNIT_Z;
    test1.divide(2); test2.divide(2); test3.divide(2); test4.divide(2);
    CUAssertAlwaysLog(test1 == Vec3(0.5,0.5,0.5),       "Method divide() failed");
    CUAssertAlwaysLog(test2 == Vec3(0.5,0,0),           "Method divide() failed");
    CUAssertAlwaysLog(test3 == Vec3(0,0.5,0),           "Method divide() failed");
    CUAssertAlwaysLog(test4 == Vec3(0,0,0.5),           "Method divide() failed");
    
    test1 = Vec3::ONE;
    test2 = Vec3::UNIT_X;
    test3 = Vec3::UNIT_Y;
    test4 = Vec3::UNIT_Z;
    test1.divide(2,4,-2); test2.divide(2,4,-2); test3.divide(2,4,-2); test4.divide(2,4,-2);
    CUAssertAlwaysLog(test1 == Vec3(0.5,0.25,-0.5),     "Method divide() failed");
    CUAssertAlwaysLog(test2 == Vec3(0.5,0,0),           "Method divide() failed");
    CUAssertAlwaysLog(test3 == Vec3(0,0.25,0),          "Method divide() failed");
    CUAssertAlwaysLog(test4 == Vec3(0,0,-0.5),          "Method divide() failed");
    
    test1 = Vec3::ONE;
    test2 = Vec3::UNIT_X;
    test3 = Vec3::UNIT_Y;
    test4 = Vec3::UNIT_Z;
    test5 = Vec3(-0.5,0.5,0.25);
    test1.divide(test5); test2.divide(test5); test3.divide(test5); test4.divide(test5);
    CUAssertAlwaysLog(test1 == Vec3(-2,2,4),            "Method divide() failed");
    CUAssertAlwaysLog(test2 == Vec3(-2,0,0),            "Method divide() failed");
    CUAssertAlwaysLog(test3 == Vec3(0,2,0),             "Method divide() failed");
    CUAssertAlwaysLog(test4 == Vec3(0,0,4),             "Method divide() failed");
    
    test1 = Vec3::ONE;
    test2 = Vec3::UNIT_X;
    test3 = Vec3::UNIT_Y;
    test4 = Vec3::UNIT_Z;
    test1.negate();test2.negate(); test3.negate(); test4.negate();
    CUAssertAlwaysLog(test1 == Vec3(-1,-1,-1),          "Method negate() failed");
    CUAssertAlwaysLog(test2 == Vec3(-1, 0, 0),          "Method negate() failed");
    CUAssertAlwaysLog(test3 == Vec3( 0,-1, 0),          "Method negate() failed");
    CUAssertAlwaysLog(test4 == Vec3( 0, 0,-1),          "Method negate() failed");
    
    test1 = Vec3::ONE;
    test2 = Vec3::UNIT_X;
    test3 = Vec3::UNIT_Y;
    test4 = Vec3::UNIT_Z;
    test5 = test1.getNegation();
    CUAssertAlwaysLog(test5 != test1,                   "Method getNegation() failed");
    CUAssertAlwaysLog(test5 == Vec3(-1,-1,-1),          "Method getNegation() failed");
    test5 = test2.getNegation();
    CUAssertAlwaysLog(test5 == Vec3(-1, 0, 0),          "Method getNegation() failed");
    test5 = test3.getNegation();
    CUAssertAlwaysLog(test5 == Vec3( 0,-1, 0),          "Method getNegation() failed");
    test5 = test4.getNegation();
    CUAssertAlwaysLog(test5 == Vec3( 0, 0,-1),          "Method getNegation() failed");
    
    
    test1.set(2,2,2);
    test2 = Vec3::ONE;
    test1.reciprocate(); test2.reciprocate();
    CUAssertAlwaysLog(test1 == Vec3(0.5,0.5,0.5),       "Method reciprocate() failed");
    CUAssertAlwaysLog(test2 == Vec3::ONE,               "Method reciprocate() failed");
    
    test1.set(2,2,2);
    test2 = Vec3::ONE;
    test3 = test1.getReciprocal();
    CUAssertAlwaysLog(test3 != test1,                   "Method getReciprocal() failed");
    CUAssertAlwaysLog(test3 == Vec3(0.5,0.5,0.5),       "Method getReciprocal() failed");
    test3 = test2.getReciprocal();
    CUAssertAlwaysLog(test3 == Vec3::ONE,               "Method getReciprocal() failed");
    
    test1 = Vec3::ONE;
    test2 = Vec3::UNIT_X;
    test3 = Vec3::UNIT_Y;
    test4 = Vec3::UNIT_Z;
    test1.map(asinf); test2.map(asinf); test3.map(asinf); test4.map(asinf);
    CUAssertAlwaysLog(CU_MATH_APPROX(test1.x,M_PI_2,CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test1.y,M_PI_2,CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test1.z,M_PI_2,CU_MATH_EPSILON),
                      "Method map() failed");
    CUAssertAlwaysLog(CU_MATH_APPROX(test2.x,M_PI_2,CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test2.y,0,CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test2.z,0,CU_MATH_EPSILON),
                      "Method map() failed");
    CUAssertAlwaysLog(CU_MATH_APPROX(test3.x,0,CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test3.y,M_PI_2,CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test3.z,0,CU_MATH_EPSILON),
                      "Method map() failed");
    CUAssertAlwaysLog(CU_MATH_APPROX(test4.x,0,CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test4.y,0,CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test4.z,M_PI_2,CU_MATH_EPSILON),
                      "Method map() failed");

    test1 = Vec3::ONE;
    test2 = Vec3::UNIT_X;
    test3 = Vec3::UNIT_Y;
    test4 = Vec3::UNIT_Z;
    test5 = test1.getMap(asinf);
    CUAssertAlwaysLog(test1 != test5,   "Method getMap() failed");
    CUAssertAlwaysLog(CU_MATH_APPROX(test5.x,M_PI_2,CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.y,M_PI_2,CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.z,M_PI_2,CU_MATH_EPSILON),
                      "Method map() failed");
    test5 = test2.getMap(asinf);
    CUAssertAlwaysLog(CU_MATH_APPROX(test5.x,M_PI_2,CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.y,0,CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.z,0,CU_MATH_EPSILON),
                      "Method map() failed");
    test5 = test3.getMap(asinf);
    CUAssertAlwaysLog(CU_MATH_APPROX(test5.x,0,CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.y,M_PI_2,CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.z,0,CU_MATH_EPSILON),
                      "Method map() failed");
    test5 = test4.getMap(asinf);
    CUAssertAlwaysLog(CU_MATH_APPROX(test5.x,0,CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.y,0,CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.z,M_PI_2,CU_MATH_EPSILON),
                      "Method map() failed");

#pragma mark Operator Test
    test1 = Vec3::UNIT_X;
    test1 += Vec3::UNIT_Y;
    test1 += Vec3::UNIT_Z;
    CUAssertAlwaysLog(test1 == Vec3::ONE,               "Addition operation failed");
        
    test1 = Vec3::ONE;
    test1 += test1;
    CUAssertAlwaysLog(test1 == Vec3(2,2,2),             "Addition operation failed");
    CUAssertAlwaysLog(Vec3::UNIT_X+Vec3::UNIT_Y+Vec3::UNIT_Z == Vec3::ONE,  "Addition operation failed");
    CUAssertAlwaysLog(Vec3::ONE+Vec3::ONE == Vec3(2,2,2),                   "Addition operation failed");
        
    test1 = Vec3::UNIT_X;
    test1 -= Vec3::UNIT_Z;
    CUAssertAlwaysLog(test1 == Vec3(1,0,-1),            "Subtraction operation failed");
    
    test1 = Vec3::ONE;
    test1 -= test1;
    CUAssertAlwaysLog(test1 == Vec3::ZERO,              "Subtraction operation failed");
    CUAssertAlwaysLog(Vec3::UNIT_X-Vec3::UNIT_Z == Vec3(1,0,-1),            "Subtraction operation failed");
    CUAssertAlwaysLog(Vec3::ONE-Vec3::ONE == Vec3::ZERO,                    "Subtraction operation failed");
    
    test1 = Vec3::ONE;
    test2 = Vec3::UNIT_X;
    test3 = Vec3::UNIT_Y;
    test4 = Vec3::UNIT_Z;
    test1 *= 2; test2 *= 2; test3 *= 2; test4 *= 2;
    CUAssertAlwaysLog(test1 == Vec3(2,2,2),             "Scaling operation failed");
    CUAssertAlwaysLog(test2 == Vec3(2,0,0),             "Scaling operation failed");
    CUAssertAlwaysLog(test3 == Vec3(0,2,0),             "Scaling operation failed");
    CUAssertAlwaysLog(test4 == Vec3(0,0,2),             "Scaling operation failed");
    CUAssertAlwaysLog(Vec3::ONE*2 == Vec3(2,2,2),       "Scaling operation failed");
    CUAssertAlwaysLog(Vec3::UNIT_X*2 == Vec3(2,0,0),    "Scaling operation failed");
    CUAssertAlwaysLog(Vec3::UNIT_Y*2 == Vec3(0,2,0),    "Scaling operation failed");
    CUAssertAlwaysLog(Vec3::UNIT_Z*2 == Vec3(0,0,2),    "Scaling operation failed");
    CUAssertAlwaysLog(2*Vec3::ONE == Vec3(2,2,2),       "Scaling operation failed");
    CUAssertAlwaysLog(2*Vec3::UNIT_X == Vec3(2,0,0),    "Scaling operation failed");
    CUAssertAlwaysLog(2*Vec3::UNIT_Y == Vec3(0,2,0),    "Scaling operation failed");
    CUAssertAlwaysLog(2*Vec3::UNIT_Z == Vec3(0,0,2),    "Scaling operation failed");

    test1 = Vec3::ONE;
    test2 = Vec3::UNIT_X;
    test3 = Vec3::UNIT_Y;
    test4 = Vec3::UNIT_Z;
    test5 = Vec3(-0.5,0.5,1.5);
    test1 *= test5; test2 *= test5; test3 *= test5; test4 *= test5;
    CUAssertAlwaysLog(test1 == Vec3(-0.5,0.5,1.5),      "Scaling operation failed");
    CUAssertAlwaysLog(test2 == Vec3(-0.5,0,0),          "Scaling operation failed");
    CUAssertAlwaysLog(test3 == Vec3(0,0.5,0),           "Scaling operation failed");
    CUAssertAlwaysLog(test4 == Vec3(0,0,1.5),           "Scaling operation failed");
    CUAssertAlwaysLog(Vec3::ONE*test5 == Vec3(-0.5,0.5,1.5),    "Scaling operation failed");
    CUAssertAlwaysLog(Vec3::UNIT_X*test5 == Vec3(-0.5,0,0),     "Scaling operation failed");
    CUAssertAlwaysLog(Vec3::UNIT_Y*test5 == Vec3(0,0.5,0),      "Scaling operation failed");
    CUAssertAlwaysLog(Vec3::UNIT_Z*test5 == Vec3(0,0,1.5),      "Scaling operation failed");
    
    test1 = Vec3::ONE;
    test2 = Vec3::UNIT_X;
    test3 = Vec3::UNIT_Y;
    test4 = Vec3::UNIT_Z;
    test1 /= 0.5; test2 /= 0.5; test3 /= 0.5; test4 /= 0.5;
    CUAssertAlwaysLog(test1 == Vec3(2,2,2),             "Division operation failed");
    CUAssertAlwaysLog(test2 == Vec3(2,0,0),             "Division operation failed");
    CUAssertAlwaysLog(test3 == Vec3(0,2,0),             "Division operation failed");
    CUAssertAlwaysLog(test4 == Vec3(0,0,2),             "Division operation failed");
    CUAssertAlwaysLog(Vec3::ONE/0.5 == Vec3(2,2,2),     "Division operation failed");
    CUAssertAlwaysLog(Vec3::UNIT_X/0.5 == Vec3(2,0,0),  "Division operation failed");
    CUAssertAlwaysLog(Vec3::UNIT_Y/0.5 == Vec3(0,2,0),  "Division operation failed");
    CUAssertAlwaysLog(Vec3::UNIT_Z/0.5 == Vec3(0,0,2),  "Division operation failed");
    
    test1 = Vec3::ONE;
    test2 = Vec3::UNIT_X;
    test3 = Vec3::UNIT_Y;
    test4 = Vec3::UNIT_Z;
    test5.set(1/2.0f,1/4.0f,-1/2.0f);
    test1 /= test5; test2 /= test5; test3 /= test5; test4 /= test5;
    CUAssertAlwaysLog(test1 == Vec3(2,4,-2),                "Division operation failed");
    CUAssertAlwaysLog(test2 == Vec3(2,0,0),                 "Division operation failed");
    CUAssertAlwaysLog(test3 == Vec3(0,4,0),                 "Division operation failed");
    CUAssertAlwaysLog(Vec3::ONE/test5 == Vec3(2,4,-2),      "Division operation failed");
    CUAssertAlwaysLog(Vec3::UNIT_X/test5 == Vec3(2,0,0),    "Division operation failed");
    CUAssertAlwaysLog(Vec3::UNIT_Y/test5 == Vec3(0,4,0),    "Division operation failed");
    CUAssertAlwaysLog(Vec3::UNIT_Z/test5 == Vec3(0,0,-2),   "Division operation failed");
    
    CUAssertAlwaysLog(-Vec3::ONE == Vec3(-1,-1,-1),         "Negation operation failed");
    CUAssertAlwaysLog(-Vec3::UNIT_X == Vec3(-1, 0, 0),      "Negation operation failed");
    CUAssertAlwaysLog(-Vec3::UNIT_Y == Vec3( 0,-1, 0),      "Negation operation failed");
    CUAssertAlwaysLog(-Vec3::UNIT_Z == Vec3( 0, 0,-1),      "Negation operation failed");
    
#pragma mark Linear Attributes
    angle = Vec3::UNIT_X.getAngle(Vec3::UNIT_Z);
    CUAssertAlwaysLog(CU_MATH_APPROX(angle,M_PI_2,CU_MATH_EPSILON),       "Method getAngle() failed");
    angle = Vec3::UNIT_Y.getAngle(Vec3::UNIT_X);
    CUAssertAlwaysLog(CU_MATH_APPROX(angle,-M_PI_2,CU_MATH_EPSILON),      "Method getAngle() failed");
    angle = Vec3::UNIT_Y.getAngle(Vec3::UNIT_X,-Vec3::UNIT_Z);
    CUAssertAlwaysLog(CU_MATH_APPROX(angle,M_PI_2,CU_MATH_EPSILON),       "Method getAngle() failed");
    angle = Vec3::ONE.getAngle(Vec3::UNIT_X);
    CUAssertAlwaysLog(CU_MATH_APPROX(angle,-0.955316603,CU_MATH_EPSILON), "Method getAngle() failed");
    
    CUAssertAlwaysLog(Vec3::ZERO.isZero(),          "Method isZero() failed");
    CUAssertAlwaysLog(!Vec3::UNIT_X.isZero(),       "Method isZero() failed");
    CUAssertAlwaysLog(!Vec3::UNIT_Y.isZero(),       "Method isZero() failed");
    CUAssertAlwaysLog(!Vec3::UNIT_Z.isZero(),       "Method isZero() failed");
    CUAssertAlwaysLog(!Vec3::ONE.isZero(),          "Method isZero() failed");
        
    test1.set(0,CU_MATH_EPSILON*0.5,-CU_MATH_EPSILON*0.5);
    CUAssertAlwaysLog(Vec3::ZERO.isNearZero(),      "Method isNearZero() failed");
    CUAssertAlwaysLog(test1.isNearZero(),           "Method isNearZero() failed");
    CUAssertAlwaysLog(!Vec3::UNIT_X.isNearZero(),   "Method isNearZero() failed");
    CUAssertAlwaysLog(!Vec3::UNIT_Y.isNearZero(),   "Method isNearZero() failed");
    CUAssertAlwaysLog(!Vec3::UNIT_Z.isNearZero(),   "Method isNearZero() failed");
    CUAssertAlwaysLog(!Vec3::ONE.isNearZero(),      "Method isNearZero() failed");
        
    CUAssertAlwaysLog(!Vec3::ZERO.isOne(),          "Method isOne() failed");
    CUAssertAlwaysLog(!Vec3::UNIT_X.isOne(),        "Method isOne() failed");
    CUAssertAlwaysLog(!Vec3::UNIT_Y.isOne(),        "Method isOne() failed");
    CUAssertAlwaysLog(!Vec3::UNIT_Z.isOne(),        "Method isOne() failed");
    CUAssertAlwaysLog(Vec3::ONE.isOne(),            "Method isOne() failed");
    
    CUAssertAlwaysLog(!Vec3::ZERO.isInvertible(),   "Method isInvertible() failed");
    CUAssertAlwaysLog(!Vec3::UNIT_X.isInvertible(), "Method isInvertible() failed");
    CUAssertAlwaysLog(!Vec3::UNIT_Y.isInvertible(), "Method isInvertible() failed");
    CUAssertAlwaysLog(!Vec3::UNIT_Z.isInvertible(), "Method isInvertible() failed");
    CUAssertAlwaysLog(Vec3::ONE.isInvertible(),     "Method isInvertible() failed");

    CUAssertAlwaysLog(!Vec3::ZERO.isUnit(),         "Method isUnit() failed");
    CUAssertAlwaysLog(Vec3::UNIT_X.isUnit(),        "Method isUnit() failed");
    CUAssertAlwaysLog(Vec3::UNIT_Y.isUnit(),        "Method isUnit() failed");
    CUAssertAlwaysLog(Vec3::UNIT_Z.isUnit(),        "Method isUnit() failed");
    CUAssertAlwaysLog(!Vec3::ONE.isUnit(),          "Method isUnit() failed");
    CUAssertAlwaysLog(Vec3(1/sqrtf(2),0,1/sqrtf(2)).isUnit(),  "Method isUnit() failed");
        
    CUAssertAlwaysLog(Vec3::ZERO.distance(Vec3::UNIT_X) == 1,   "Method distance() failed");
    CUAssertAlwaysLog(Vec3::UNIT_X.distance(Vec3::ZERO) == 1,   "Method distance() failed");
    CUAssertAlwaysLog(Vec3::ZERO.distance(Vec3::UNIT_Y) == 1,   "Method distance() failed");
    CUAssertAlwaysLog(Vec3::UNIT_Y.distance(Vec3::ZERO) == 1,   "Method distance() failed");
    CUAssertAlwaysLog(Vec3::ZERO.distance(Vec3::UNIT_Z) == 1,   "Method distance() failed");
    CUAssertAlwaysLog(Vec3::UNIT_Z.distance(Vec3::ZERO) == 1,   "Method distance() failed");
    CUAssertAlwaysLog(Vec3::ONE.distance(Vec3::UNIT_Z) == sqrtf(2),     "Method distance() failed");
    CUAssertAlwaysLog(Vec3::UNIT_Z.distance(Vec3::ONE) == sqrtf(2),     "Method distance() failed");
    CUAssertAlwaysLog(Vec3::ZERO.distance(Vec3::ONE) == sqrtf(3),       "Method distance() failed");
    CUAssertAlwaysLog(Vec3::ONE.distance(Vec3::ZERO) == sqrtf(3),       "Method distance() failed");
    CUAssertAlwaysLog(Vec3(1,2,-1).distance(Vec3(2,0,1)) == 3,          "Method distance() failed");
    CUAssertAlwaysLog(Vec3(2,0,1).distance(Vec3(1,2,-1)) == 3,          "Method distance() failed");

    CUAssertAlwaysLog(Vec3::ZERO.distanceSquared(Vec3::UNIT_X) == 1,    "Method distanceSquared() failed");
    CUAssertAlwaysLog(Vec3::UNIT_X.distanceSquared(Vec3::ZERO) == 1,    "Method distanceSquared() failed");
    CUAssertAlwaysLog(Vec3::ZERO.distanceSquared(Vec3::UNIT_Y) == 1,    "Method distanceSquared() failed");
    CUAssertAlwaysLog(Vec3::UNIT_Y.distanceSquared(Vec3::ZERO) == 1,    "Method distanceSquared() failed");
    CUAssertAlwaysLog(Vec3::ZERO.distanceSquared(Vec3::UNIT_Z) == 1,    "Method distanceSquared() failed");
    CUAssertAlwaysLog(Vec3::UNIT_Z.distanceSquared(Vec3::ZERO) == 1,    "Method distanceSquared() failed");
    CUAssertAlwaysLog(Vec3::ONE.distanceSquared(Vec3::UNIT_Z) == 2,     "Method distanceSquared() failed");
    CUAssertAlwaysLog(Vec3::UNIT_Z.distanceSquared(Vec3::ONE) == 2,     "Method distanceSquared() failed");
    CUAssertAlwaysLog(Vec3::ZERO.distanceSquared(Vec3::ONE) == 3,       "Method distanceSquared() failed");
    CUAssertAlwaysLog(Vec3::ONE.distanceSquared(Vec3::ZERO) == 3,       "Method distanceSquared() failed");
    CUAssertAlwaysLog(Vec3(1,2,-1).distanceSquared(Vec3(2,0,1)) == 9,   "Method distanceSquared() failed");
    CUAssertAlwaysLog(Vec3(2,0,1).distanceSquared(Vec3(1,2,-1)) == 9,   "Method distanceSquared() failed");
    
    CUAssertAlwaysLog(Vec3::ZERO.length() == 0,             "Method length() failed");
    CUAssertAlwaysLog(Vec3::UNIT_X.length() == 1,           "Method length() failed");
    CUAssertAlwaysLog(Vec3::UNIT_Y.length() == 1,           "Method length() failed");
    CUAssertAlwaysLog(Vec3::UNIT_Z.length() == 1,           "Method length() failed");
    CUAssertAlwaysLog(Vec3::ONE.length() == sqrtf(3),       "Method length() failed");
    CUAssertAlwaysLog(Vec3(-2,1,2).length() == 3,           "Method length() failed");

    CUAssertAlwaysLog(Vec3::ZERO.lengthSquared() == 0,      "Method length() failed");
    CUAssertAlwaysLog(Vec3::UNIT_X.lengthSquared() == 1,    "Method length() failed");
    CUAssertAlwaysLog(Vec3::UNIT_Y.lengthSquared() == 1,    "Method length() failed");
    CUAssertAlwaysLog(Vec3::UNIT_Z.lengthSquared() == 1,    "Method length() failed");
    CUAssertAlwaysLog(Vec3::ONE.lengthSquared() == 3,       "Method length() failed");
    CUAssertAlwaysLog(Vec3(-2,1,2).lengthSquared() == 9,    "Method length() failed");

#pragma mark Linear Algebra Test
    CUAssertAlwaysLog(Vec3::UNIT_X.dot(Vec3::UNIT_Y) == 0,  "Method dot() failed");
    CUAssertAlwaysLog(Vec3::UNIT_X.dot(Vec3::UNIT_Z) == 0,  "Method dot() failed");
    CUAssertAlwaysLog(Vec3::ONE.dot(Vec3::ZERO) == 0,       "Method dot() failed");
    CUAssertAlwaysLog(Vec3::ONE.dot(Vec3::ONE) == 3,        "Method dot() failed");
    CUAssertAlwaysLog(Vec3::UNIT_Z.dot(Vec3::UNIT_Z) == 1,  "Method dot() failed");
        
    test1.set(1/sqrtf(3),1/sqrtf(3),1/sqrtf(3));
    CUAssertAlwaysLog(CU_MATH_APPROX(test1.dot(test1),1,CU_MATH_EPSILON), "Method dot() failed");
    
    test1 = Vec3::UNIT_X;
    test1.cross(Vec3::UNIT_Y);
    CUAssertAlwaysLog(test1 == Vec3::UNIT_Z,                "Method cross() failed");
    test1 = Vec3::UNIT_Y;
    test1.cross(Vec3::UNIT_X);
    CUAssertAlwaysLog(test1 == -Vec3::UNIT_Z,               "Method cross() failed");
    test1 = Vec3::ONE;
    test1.cross(Vec3::UNIT_Z);
    CUAssertAlwaysLog(test1 == Vec3(1,-1,0),                "Method cross() failed");

    test1 = Vec3::UNIT_X;
    test2 = test1.getCross(Vec3::UNIT_Y);
    CUAssertAlwaysLog(test1 != test2,                       "Method getCross() failed");
    CUAssertAlwaysLog(test2 == Vec3::UNIT_Z,                "Method getCross() failed");
    test1 = Vec3::UNIT_Y;
    test2 = test1.getCross(Vec3::UNIT_X);
    CUAssertAlwaysLog(test2 == -Vec3::UNIT_Z,               "Method getCross() failed");
    test1 = Vec3::ONE;
    test2 = test1.getCross(Vec3::UNIT_Z);
    CUAssertAlwaysLog(test2 == Vec3(1,-1,0),                "Method getCross() failed");

    test1 = Vec3::ONE;
    test2.set(1/sqrtf(3),1/sqrtf(3),1/sqrtf(3));
    CUAssertAlwaysLog(test1.normalize().equals(test2),          "Method normalize() failed.");
    test1 = Vec3::UNIT_X;
    CUAssertAlwaysLog(test1.normalize().equals(Vec3::UNIT_X),   "Method normalize() failed.");
    test1 = Vec3::UNIT_Y;
    CUAssertAlwaysLog(test1.normalize().equals(Vec3::UNIT_Y),   "Method normalize() failed.");
    test1 = Vec3::UNIT_Z;
    CUAssertAlwaysLog(test1.normalize().equals(Vec3::UNIT_Z),   "Method normalize() failed.");
    
    test1 = Vec3::ONE;
    test3 = test1.getNormalization();
    CUAssertAlwaysLog(test1 != test3,                                   "Method getNormalization() failed.");
    CUAssertAlwaysLog(test3.equals(test2),                              "Method getNormalization() failed.");
    test1 = Vec3::UNIT_X;
    CUAssertAlwaysLog(test1.getNormalization().equals(Vec3::UNIT_X),    "Method getNormalization() failed.");
    test1 = Vec3::UNIT_Y;
    CUAssertAlwaysLog(test1.getNormalization().equals(Vec3::UNIT_Y),    "Method getNormalization() failed.");
    test1 = Vec3::UNIT_Z;
    CUAssertAlwaysLog(test1.getNormalization().equals(Vec3::UNIT_Z),    "Method getNormalization() failed.");
        
    test1 = Vec3::ZERO;
    test2 = test1.getMidpoint(Vec3::ONE);
    CUAssertAlwaysLog(test1 != test2,                           "Method getMidpoint() failed.");
    CUAssertAlwaysLog(test2 == Vec3(0.5,0.5,0.5),               "Method getMidpoint() failed.");
        
    test1 = Vec3::UNIT_X.getMidpoint(Vec3::UNIT_Y);
    test2 = Vec3::UNIT_Y.getMidpoint(Vec3::UNIT_Z);
    CUAssertAlwaysLog(test1 == Vec3(0.5,0.5,0),                 "Method getMidpoint() failed.");
    CUAssertAlwaysLog(test2 == Vec3(0,0.5,0.5),                 "Method getMidpoint() failed.");

    test1.set(2,3,-1);
    test1.project(Vec3::UNIT_X);
    CUAssertAlwaysLog(test1 == Vec3(2,0,0),                     "Method project() failed.");
    test1.set(2,3,-1);
    test1.project(Vec3::UNIT_Y);
    CUAssertAlwaysLog(test1 == Vec3(0,3,0),                     "Method project() failed.");
    test1.set(2,3,-1);
    test1.project(Vec3::UNIT_Z);
    CUAssertAlwaysLog(test1 == Vec3(0,0,-1),                    "Method project() failed.");
    test1 = 6*Vec3::UNIT_Z;
    test1.project(Vec3(1,1,1));
    CUAssertAlwaysLog(test1 == Vec3(2,2,2),                     "Method project() failed.");

    test1.set(2,3,-1);
    test2 = test1.getProjection(Vec3::UNIT_X);
    CUAssertAlwaysLog(test1 != test2,                           "Method getProjection() failed.");
    CUAssertAlwaysLog(test2 == Vec3(2,0,0),                     "Method getProjection() failed.");
    test2 = test1.getProjection(Vec3::UNIT_Y);
    CUAssertAlwaysLog(test2 == Vec3(0,3,0),                     "Method getProjection() failed.");
    test2 = test1.getProjection(Vec3::UNIT_Z);
    CUAssertAlwaysLog(test2 == Vec3(0,0,-1),                    "Method getProjection() failed.");
    test1 = 6*Vec3::UNIT_Z;
    test2 = test1.getProjection(Vec3(1,1,1));
    CUAssertAlwaysLog(test2 == Vec3(2,2,2),                     "Method getProjection() failed.");

    test1 = Vec3::ONE;
    test2.set(2,3,0);
    test1.lerp(test2,0);
    CUAssertAlwaysLog(test1 == Vec3::ONE,                       "Method lerp() failed.");
    test1.lerp(test2,1);
    CUAssertAlwaysLog(test1 == test2,                           "Method lerp() failed.");
    test1 = Vec3::ONE;
    test1.lerp(test2,0.5);
    CUAssertAlwaysLog(test1 == Vec3(1.5,2,0.5),                 "Method lerp() failed.");
    test1 = Vec3::ONE;
    test1.lerp(test2,-1);
    CUAssertAlwaysLog(test1 == Vec3(0,-1,2),                    "Method lerp() failed.");
    test1 = Vec3::ONE;
    test1.lerp(test2,2);
    CUAssertAlwaysLog(test1 == Vec3(3,5,-1),                    "Method lerp() failed.");

    test1 = Vec3::ONE;
    test2.set(2,3,0);
    test3 = test1.getLerp(test2,0);
    CUAssertAlwaysLog(test3 == Vec3::ONE,                       "Method getLerp() failed.");
    test3 = test1.getLerp(test2,1);
    CUAssertAlwaysLog(test1 != test3,                           "Method getLerp() failed.");
    CUAssertAlwaysLog(test3 == test2,                           "Method getLerp() failed.");
    test3 = test1.getLerp(test2,0.5);
    CUAssertAlwaysLog(test3 == Vec3(1.5,2,0.5),                 "Method getLerp() failed.");
    test3 = test1.getLerp(test2,-1);
    CUAssertAlwaysLog(test3 == Vec3(0,-1,2),                    "Method getLerp() failed.");
    test3 = test1.getLerp(test2,2);
    CUAssertAlwaysLog(test3 == Vec3(3,5,-1),                    "Method getLerp() failed.");

#pragma mark Static Linear Algebra Test
    CUAssertAlwaysLog(Vec3::dot(Vec3::UNIT_X,Vec3::UNIT_Z) == 0,    "Vec3::dot() failed");
    CUAssertAlwaysLog(Vec3::dot(Vec3::ONE,Vec3::ZERO) == 0,         "Vec3::dot() failed");
    CUAssertAlwaysLog(Vec3::dot(Vec3::ONE,Vec3::ONE) == 3,          "Vec3::dot() failed");
    CUAssertAlwaysLog(Vec3::dot(Vec3::UNIT_X,Vec3::UNIT_X) == 1,    "Vec3::dot() failed");
    
    test1.set(2,2,2);
    testptr = Vec3::cross(Vec3::UNIT_X,Vec3::UNIT_Y,&test1);
    CUAssertAlwaysLog(test1 == Vec3::UNIT_Z,                        "Vec3::cross() failed");
    CUAssertAlwaysLog(testptr == &test1,                            "Vec3::cross() failed");
    testptr = Vec3::cross(Vec3::UNIT_X,Vec3::UNIT_Z,&test1);
    CUAssertAlwaysLog(test1 == -Vec3::UNIT_Y,                       "Vec3::cross() failed");
    Vec3::cross(Vec3::UNIT_Z,Vec3::UNIT_X,&test1);
    CUAssertAlwaysLog(test1 == Vec3::UNIT_Y,                        "Vec3::cross() failed");
    Vec3::cross(Vec3::ONE,Vec3::UNIT_X,&test1);
    CUAssertAlwaysLog(test1 == Vec3(0,1,-1),                        "Vec3::cross() failed");
    
    test1.set(1/sqrtf(3.0),1/sqrtf(3.0),1/sqrtf(3.0));
    testptr = Vec3::normalize(Vec3::ONE,&test2);
    CUAssertAlwaysLog(testptr == &test2,                        "Vec3::normalize() failed");
    CUAssertAlwaysLog(test2.equals(test1),                      "Vec3::normalize() failed.");
    Vec3::normalize(Vec3::UNIT_X,&test2);
    CUAssertAlwaysLog(test2.equals(Vec4::UNIT_X),               "Vec3::normalize() failed.");
    Vec3::normalize(Vec3::UNIT_Y,&test2);
    CUAssertAlwaysLog(test2.equals(Vec4::UNIT_Y),               "Vec3::normalize() failed.");
    Vec3::normalize(Vec3::UNIT_Z,&test2);
    CUAssertAlwaysLog(test2.equals(Vec4::UNIT_Z),               "Vec3::normalize() failed.");
    
    test1 = Vec3::ZERO;
    testptr = Vec3::midpoint(test1,Vec3::ONE,&test2);
    CUAssertAlwaysLog(testptr == &test2,                        "Vec3::midpoint() failed");
    CUAssertAlwaysLog(test2 == Vec3(0.5,0.5,0.5),               "Vec3::midpoint() failed.");
        
    Vec3::midpoint(Vec3::UNIT_X,Vec3::UNIT_Y, &test1);
    Vec3::midpoint(Vec3::UNIT_Z,Vec3::UNIT_X, &test2);
    CUAssertAlwaysLog(test1 == Vec3(0.5,0.5,0),                 "Vec3::midpoint() failed.");
    CUAssertAlwaysLog(test2 == Vec3(0.5,0,0.5),                 "Vec3::midpoint() failed.");
        
    test1.set(2,3,-1);
    testptr = Vec3::project(test1,Vec3::UNIT_X,&test2);
    CUAssertAlwaysLog(testptr == &test2,                        "Vec3::project() failed");
    CUAssertAlwaysLog(test2 == Vec3(2,0,0),                     "Vec3::project() failed.");
    Vec3::project(test1, Vec3::UNIT_Y, &test2);
    CUAssertAlwaysLog(test2 == Vec3(0,3,0),                     "Vec3::project() failed.");
    Vec3::project(test1, Vec3::UNIT_Z, &test2);
    CUAssertAlwaysLog(test2 == Vec3(0,0,-1),                    "Vec3::project() failed.");
    Vec3::project(6*Vec3::UNIT_Z, Vec3(1,1,1), &test2);
    CUAssertAlwaysLog(test2 == Vec3(2,2,2),                     "Vec3::project() failed.");
        
    test1 = Vec3::ONE;
    test2.set(2,3,0);
    testptr = Vec3::lerp(test1, test2, 0, &test3);
    CUAssertAlwaysLog(testptr == &test3,                        "Vec3::lerp() failed");
    CUAssertAlwaysLog(test3 == test1,                           "Vec3::lerp() failed.");
    Vec3::lerp(test1, test2, 1, &test3);
    CUAssertAlwaysLog(test3 == test2,                           "Vec3::lerp() failed.");
    Vec3::lerp(test1, test2, 0.5, &test3);
    CUAssertAlwaysLog(test3 == Vec3(1.5,2,0.5),                 "Vec3::lerp() failed.");
    Vec3::lerp(test1, test2, -1, &test3);
    CUAssertAlwaysLog(test3 == Vec3(0,-1,2),                    "Vec3::lerp() failed.");
    Vec3::lerp(test1, test2, 2, &test3);
    CUAssertAlwaysLog(test3 == Vec3(3,5,-1),                    "Vec3::lerp() failed.");
    
#pragma mark Conversion Test
    std::string str;
    std::string a, b, c;
    test1.set(2,3,-1.5); str = test1.toString();
    a = cugl::to_string(2.0f); b = cugl::to_string(3.0f); c = cugl::to_string(-1.5f);
    CUAssertAlwaysLog(str == "("+a+","+b+","+c+")",             "Method toString() failed");
    str = test1.toString(true);
    CUAssertAlwaysLog(str == "cugl::Vec3("+a+","+b+","+c+")",   "Method toString() failed");
    str = (std::string)test1;
    CUAssertAlwaysLog(str == "("+a+","+b+","+c+")",             "String cast failed");

    test1.set(0.25,0.5,0.75);
    Color4 cbtest = (Color4)test1;
    CUAssertAlwaysLog(cbtest.r == 64 && cbtest.g == 128 && cbtest.b == 191 && cbtest.a == 255,
                      "Color4 cast failed");
    Vec3 test6(cbtest);
    CUAssertAlwaysLog(test6.equals(test1,0.01f),            "Color constructor failed");
    test5 = cbtest;
    CUAssertAlwaysLog(test5.equals(test1,0.01f),            "Color assignment failed");

    Color4f cftest = (Color4f)test1;
    CUAssertAlwaysLog(cftest.r == 0.25 && cftest.g == 0.5 && cftest.b == 0.75 && cftest.a == 1,
                      "Color4f cast failed");
    Vec3 test7(cftest);
    CUAssertAlwaysLog(test7 == test1,                       "Colorf constructor failed");
    test6 = cftest;
    CUAssertAlwaysLog(test6 == test1,                       "Colorf assignment failed");

    test1.set(3,5,-1);
    Vec2 v2test = (Vec2)test1;
    CUAssertAlwaysLog(v2test.x == 3 && v2test.y == 5,       "Vec2 cast failed");
    Vec3 test8(v2test);
    CUAssertAlwaysLog(test8 != test1,                       "Vec2 constructor failed");
    CUAssertAlwaysLog(test8-test1 == Vec3::UNIT_Z,          "Vec2 constructor failed");
    test7 = v2test;
    CUAssertAlwaysLog(test7 != test1,                       "Vec2 assignment failed");
    CUAssertAlwaysLog(test7-test1 == Vec3::UNIT_Z,          "Vec2 assignment failed");
    
    test1.set(-4,8,2);
    Vec4 v4test = (Vec4)test1;
    CUAssertAlwaysLog(v4test.x == -4 && v4test.y == 8 && v4test.z == 2 && v4test.w == 1, "Vec4 cast failed");
    Vec3 test9(v4test);
    CUAssertAlwaysLog(test9 == test1,                       "Vec4 constructor failed");
    test8 = v4test;
    CUAssertAlwaysLog(test8 == test1,                       "Vec4 assignment failed");
        
        
#pragma mark Complete
    CULog("Vec3 tests complete.\n");
}
    
#pragma mark -
#pragma mark Vec4

/**
 * Unit test for a 4-dimension vector
 *
 * Thic class uses vector acceleration on select platforms.
 */
void cugl::testVec4() {
    CULog("Running tests for Vec4.\n");
    Timestamp start, end;
#pragma mark Constructor Test
    // Initial test of constructors
    start.mark();
    Vec4 test1;
    CUAssertAlwaysLog(test1.x == 0 && test1.y == 0 && test1.z == 0 && test1.w == 0,
                      "Trivial constructor failed");
    
    Vec4 test2(1.5,4,-2.5,6);
    CUAssertAlwaysLog(test2.x == 1.5 && test2.y == 4 && test2.z == -2.5 && test2.w == 6,
                      "Initialization constructor failed");
    
    float f[4] = {3.5, 6, 0.5, -2};
    Vec4 test3(f);
    CUAssertAlwaysLog(test3.x == 3.5 && test3.y == 6  && test3.z == 0.5 && test3.w == -2,
                      "Array constructor failed");
    
    Vec4 test4(test2);
    CUAssertAlwaysLog(test4.x == 1.5 && test4.y == 4 && test4.z == -2.5 && test4.w == 6,
                      "Copy constructor failed");
    
    Vec4 test5(test2,test3);
    CUAssertAlwaysLog(test5.x == 2 && test5.y == 2 && test5.z == 3 && test5.w == -8,
                      "Directional constructor failed");
    
    
#pragma mark Constants Test
    CUAssertAlwaysLog(Vec4::ZERO.x == 0 && Vec4::ZERO.y == 0 && Vec4::ZERO.z == 0 && Vec4::ZERO.w == 0,
                      "Zero vector failed");
    CUAssertAlwaysLog(Vec4::ONE.x == 1 && Vec4::ONE.y == 1 && Vec4::ONE.z == 1 && Vec4::ONE.w == 1,
                      "Ones vector failed");
    CUAssertAlwaysLog(Vec4::UNIT_X.x == 1 && Vec4::UNIT_X.y == 0 && Vec4::UNIT_X.z == 0 && Vec4::UNIT_X.w == 0,
                      "X-axis vector failed");
    CUAssertAlwaysLog(Vec4::UNIT_Y.x == 0 && Vec4::UNIT_Y.y == 1 && Vec4::UNIT_Y.z == 0 && Vec4::UNIT_Y.w == 0,
                      "Y-axis vector failed");
    CUAssertAlwaysLog(Vec4::UNIT_Z.x == 0 && Vec4::UNIT_Z.y == 0 && Vec4::UNIT_Z.z == 1 && Vec4::UNIT_Y.w == 0,
                      "Z-axis vector failed");
    CUAssertAlwaysLog(Vec4::UNIT_W.x == 0 && Vec4::UNIT_W.y == 0 && Vec4::UNIT_W.z == 0 && Vec4::UNIT_W.w == 1,
                      "W-axis vector failed");
    CUAssertAlwaysLog(Vec4::HOMOG_ORIGIN.x == 0 && Vec4::HOMOG_ORIGIN.y == 0 && Vec4::HOMOG_ORIGIN.z == 0 && Vec4::HOMOG_ORIGIN.w == 1,
                      "Homogenous origin failed");
    CUAssertAlwaysLog(Vec4::HOMOG_X.x == 1 && Vec4::HOMOG_X.y == 0 && Vec4::HOMOG_X.z == 0 && Vec4::HOMOG_X.w == 1,
                      "Homogenous x-axis failed");
    CUAssertAlwaysLog(Vec4::HOMOG_Y.x == 0 && Vec4::HOMOG_Y.y == 1 && Vec4::HOMOG_Y.z == 0 && Vec4::HOMOG_Y.w == 1,
                      "Homogenous y-axis failed");
    CUAssertAlwaysLog(Vec4::HOMOG_Z.x == 0 && Vec4::HOMOG_Z.y == 0 && Vec4::HOMOG_Z.z == 1 && Vec4::HOMOG_Z.w == 1,
                      "Homogenous z-axis failed");
    
    
#pragma mark Setter Test
    test1 = test2;
    CUAssertAlwaysLog(test1.x == 1.5 && test1.y == 4 && test1.z == -2.5 && test1.w == 6,
                      "Basic assignment failed");
    
    test1 = f;
    CUAssertAlwaysLog(test1.x == 3.5 && test1.y == 6  && test1.z == 0.5 && test1.w == -2,
                      "Float assignment failed");
    
    test1.set(-1,1,5,-2);
    CUAssertAlwaysLog(test1.x == -1 && test1.y == 1 && test1.z == 5 && test1.w == -2,
                      "Parameter assignment failed");
    
    test1.set(test2);
    CUAssertAlwaysLog(test1.x == 1.5 && test1.y == 4 && test1.z == -2.5  && test1.w == 6,
                      "Alternate assignment failed");
    
    test1.set(f);
    CUAssertAlwaysLog(test1.x == 3.5 && test1.y == 6  && test1.z == 0.5 && test1.w == -2,
                      "Alternate float assignment failed");
    
    test1.set(test2,test3);
    CUAssertAlwaysLog(test1.x == 2 && test1.y == 2 && test1.z == 3 && test1.w == -8,
                      "Directional assignment failed");
    
    test1.setZero();
    CUAssertAlwaysLog(test1.x == 0 && test1.y == 0 && test1.z == 0  && test1.w == 0,
                      "Erasing assignment failed");

    
#pragma mark Comparison Test
    test1.set(0,0,0,0); test2.set(0,0,1,1); test3.set(1,1,1,0); test4.set(1,1,1,1);
    
    CUAssertAlwaysLog(test1 < test4,    "Less than failed");
    CUAssertAlwaysLog(!(test4 < test1), "Less than failed");
    CUAssertAlwaysLog(test1 < test2,    "Less than failed");
    CUAssertAlwaysLog(test2 < test3,    "Less than failed");
    CUAssertAlwaysLog(!(test1 < test1), "Less than failed");
    
    CUAssertAlwaysLog(test1 <= test4,   "Less than or equal to failed");
    CUAssertAlwaysLog(!(test4 <= test1),"Less than or equal to failed");
    CUAssertAlwaysLog(test1 <= test2,   "Less than or equal to failed");
    CUAssertAlwaysLog(test2 <= test3,   "Less than or equal to failed");
    CUAssertAlwaysLog(test1 <= test1,   "Less than or equal to failed");
    
    CUAssertAlwaysLog(test4 > test1,    "Greater than failed");
    CUAssertAlwaysLog(!(test1 > test4), "Greater than failed");
    CUAssertAlwaysLog(test2 > test1,    "Greater than failed");
    CUAssertAlwaysLog(test3 > test2,    "Greater than failed");
    CUAssertAlwaysLog(!(test1 > test1), "Greater than failed");
    
    CUAssertAlwaysLog(test4 >= test1,   "Greater than or equal to failed");
    CUAssertAlwaysLog(!(test1 >= test4),"Greater than or equal to failed");
    CUAssertAlwaysLog(test2 >= test1,   "Greater than or equal to failed");
    CUAssertAlwaysLog(test3 >= test2,   "Greater than or equal to failed");
    CUAssertAlwaysLog(test1 >= test1,   "Greater than or equal to failed");
    
    CUAssertAlwaysLog(test1 == test1,   "Equals failed");
    CUAssertAlwaysLog(test2 == test2,   "Equals failed");
    CUAssertAlwaysLog(test3 == test3,   "Equals failed");
    CUAssertAlwaysLog(!(test1 == test4),"Equals failed");
    
    CUAssertAlwaysLog(!(test1 != test1),"Not equals failed");
    CUAssertAlwaysLog(!(test2 != test2),"Not equals failed");
    CUAssertAlwaysLog(!(test3 != test3),"Not equals failed");
    CUAssertAlwaysLog(test1 != test4,   "Not equals failed");
    
    CUAssertAlwaysLog(test1.under(test4),   "Method under() failed");
    CUAssertAlwaysLog(!test4.under(test1),  "Method under() failed");
    CUAssertAlwaysLog(!test2.under(test3),  "Method under() failed");
    CUAssertAlwaysLog(!test3.under(test2),  "Method under() failed");
    CUAssertAlwaysLog(test1.under(test1),   "Method under() failed");
    
    CUAssertAlwaysLog(test4.over(test1),    "Method over() failed");
    CUAssertAlwaysLog(!test1.over(test4),   "Method over() failed");
    CUAssertAlwaysLog(!test2.over(test3),   "Method over() failed");
    CUAssertAlwaysLog(!test3.over(test2),   "Method over() failed");
    CUAssertAlwaysLog(test1.over(test1),    "Method over() failed");
    
    test5.set(0,0,CU_MATH_EPSILON*0.5,-CU_MATH_EPSILON*0.5);
    CUAssertAlwaysLog(test1.equals(test1), "Approximate equals failed");
    CUAssertAlwaysLog(test1.equals(test5), "Approximate equals failed");
    
    
#pragma mark Static Arithmetic Test
    Vec4* testptr;
    
    test1.set(-2,2,-3,3);
    testptr = Vec4::clamp(test1,Vec4(-3,-3,-4,-4),Vec4(3,3,4,4),&test2);
    CUAssertAlwaysLog(test1 == test2,                                   "Vec4::clamp() failed");
    CUAssertAlwaysLog(testptr == &test2,                                "Vec4::clamp() failed");
    
    Vec4::clamp(test1,Vec4::ZERO,Vec4(3,3,4,4),&test2);
    CUAssertAlwaysLog(test1 != test2,                                   "Vec4::clamp() failed");
    CUAssertAlwaysLog(test2.x == 0 && test2.y == 2 && test2.z == 0 && test2.w == 3,
                      "Vec4::clamp() failed");
    
    Vec4::clamp(test1,Vec4(-3,-3,-4,-4),Vec4::ZERO,&test2);
    CUAssertAlwaysLog(test1 != test2,                                   "Vec4::clamp() failed");
    CUAssertAlwaysLog(test2.x == -2 && test2.y == 0 && test2.z == -3 && test2.w == 0,
                      "Vec4::clamp() failed");
    
    Vec4::clamp(test1,Vec4(-1,-1,-2,-2),Vec4(1,1,2,2),&test2);
    CUAssertAlwaysLog(test1 != test2,                                   "Vec4::clamp() failed");
    CUAssertAlwaysLog(test2.x == -1 && test2.y == 1 && test2.z == -2 && test2.w == 2,
                      "Vec4::clamp() failed");
    
    float angle = Vec4::angle(Vec4::UNIT_X,Vec4::UNIT_Z);
    CUAssertAlwaysLog(CU_MATH_APPROX(angle,M_PI_2,CU_MATH_EPSILON),       "Vec4::angle failed");
    angle = Vec4::angle(Vec4::UNIT_Y,Vec4::UNIT_X);
    CUAssertAlwaysLog(CU_MATH_APPROX(angle,M_PI_2,CU_MATH_EPSILON),       "Vec4::angle failed");
    angle = Vec4::angle(Vec4::UNIT_Y,Vec4::UNIT_W);
    CUAssertAlwaysLog(CU_MATH_APPROX(angle,M_PI_2,CU_MATH_EPSILON),       "Vec4::angle failed");
    angle = Vec4::angle(Vec4::ONE,Vec4::UNIT_W);
    CUAssertAlwaysLog(CU_MATH_APPROX(angle,1.04719746,CU_MATH_EPSILON),   "Vec4::angle failed");

    testptr = Vec4::add(Vec4::HOMOG_X,Vec4::HOMOG_Z,&test1);
    CUAssertAlwaysLog(test1 == Vec4(1,0,1,2),                       "Vec4::add() failed");
    CUAssertAlwaysLog(testptr == &test1,                            "Vec4::add() failed");
    
    test1.set(2,2,2,2);
    Vec4::add(Vec4::ONE,Vec4::ONE,&test2);
    CUAssertAlwaysLog(test1 == test2,                               "Vec4::add() failed");
    
    test1.set(1,0,-1,0);
    testptr = Vec4::subtract(Vec4::HOMOG_X,Vec4::HOMOG_Z,&test2);
    CUAssertAlwaysLog(test1 == test2,                               "Vec4::subtract() failed");
    CUAssertAlwaysLog(testptr == &test2,                            "Vec4::subtract() failed");
    
    test1.set(2,2,2,2);
    Vec4::subtract(Vec4::ONE,Vec4::ONE,&test1);
    CUAssertAlwaysLog(test1 == Vec4::ZERO,                          "Vec4::subtract() failed");
    
    testptr = Vec4::scale(Vec4::ONE,2,&test1);
    CUAssertAlwaysLog(testptr == &test1,                    "Vec4::scale() failed");
    CUAssertAlwaysLog(test1 == Vec4(2,2,2,2),               "Vec4::scale() failed");
    Vec4::scale(Vec4::UNIT_X,2,&test1);
    CUAssertAlwaysLog(test1 == Vec4(2,0,0,0),               "Vec4::scale() failed");
    Vec4::scale(Vec4::UNIT_Y,2,&test1);
    CUAssertAlwaysLog(test1 == Vec4(0,2,0,0),               "Vec4::scale() failed");
    Vec4::scale(Vec4::UNIT_Z,2,&test1);
    CUAssertAlwaysLog(test1 == Vec4(0,0,2,0),               "Vec4::scale() failed");
    Vec4::scale(Vec4::UNIT_W,2,&test1);
    CUAssertAlwaysLog(test1 == Vec4(0,0,0,2),               "Vec4::scale() failed");
    
    test2.set(-0.5,0.5,1.5,-1.5);
    testptr = Vec4::scale(Vec4::ONE,test2,&test1);
    CUAssertAlwaysLog(testptr == &test1,                    "Vec4::scale() failed");
    CUAssertAlwaysLog(test1 == Vec4(-0.5,0.5,1.5,-1.5),     "Vec4::scale() failed");
    Vec4::scale(Vec4::UNIT_X,test2,&test1);
    CUAssertAlwaysLog(test1 == Vec4(-0.5,0,0,0),            "Vec4::scale() failed");
    Vec4::scale(Vec4::UNIT_Y,test2,&test1);
    CUAssertAlwaysLog(test1 == Vec4(0,0.5,0,0),             "Vec4::scale() failed");
    Vec4::scale(Vec4::UNIT_Z,test2,&test1);
    CUAssertAlwaysLog(test1 == Vec4(0,0,1.5,0),             "Vec4::scale() failed");
    Vec4::scale(Vec4::UNIT_W,test2,&test1);
    CUAssertAlwaysLog(test1 == Vec4(0,0,0,-1.5),            "Vec4::scale() failed");
    
    testptr = Vec4::divide(Vec4::ONE,2,&test1);
    CUAssertAlwaysLog(testptr == &test1,                    "Vec4::divide() failed");
    CUAssertAlwaysLog(test1 == Vec4(0.5,0.5,0.5,0.5),       "Vec4::divide() failed");
    Vec4::divide(Vec4::UNIT_X,2,&test1);
    CUAssertAlwaysLog(test1 == Vec4(0.5,0,0,0),             "Vec4::divide() failed");
    Vec4::divide(Vec4::UNIT_Y,2,&test1);
    CUAssertAlwaysLog(test1 == Vec4(0,0.5,0,0),             "Vec4::divide() failed");
    Vec4::divide(Vec4::UNIT_Z,2,&test1);
    CUAssertAlwaysLog(test1 == Vec4(0,0,0.5,0),             "Vec4::divide() failed");
    Vec4::divide(Vec4::UNIT_W,2,&test1);
    CUAssertAlwaysLog(test1 == Vec4(0,0,0,0.5),             "Vec4::divide() failed");
    
    test2.set(-0.5,0.5,0.25,-0.25);
    testptr = Vec4::divide(Vec4::ONE,test2,&test1);
    CUAssertAlwaysLog(testptr == &test1,                    "Vec4::divide() failed");
    CUAssertAlwaysLog(test1.equals(Vec4(-2,2,4,-4)),        "Vec4::divide() failed");
    Vec4::divide(Vec4::UNIT_X,test2,&test1);
    CUAssertAlwaysLog(test1.equals(Vec4(-2,0,0,0)),         "Vec4::divide() failed");
    Vec4::divide(Vec4::UNIT_Y,test2,&test1);
    CUAssertAlwaysLog(test1.equals(Vec4(0,2,0,0)),          "Vec4::divide() failed");
    Vec4::divide(Vec4::UNIT_Z,test2,&test1);
    CUAssertAlwaysLog(test1.equals(Vec4(0,0,4,0)),          "Vec4::divide() failed");
    Vec4::divide(Vec4::UNIT_W,test2,&test1);
    CUAssertAlwaysLog(test1.equals(Vec4(0,0,0,-4)),         "Vec4::divide() failed");
    
    testptr = Vec4::negate(Vec4::ONE,&test1);
    CUAssertAlwaysLog(testptr == &test1,                    "Vec4::negate() failed");
    CUAssertAlwaysLog(test1 == Vec4(-1,-1,-1,-1),           "Vec4::negate() failed");
    Vec4::negate(Vec4::UNIT_X,&test1);
    CUAssertAlwaysLog(test1 == Vec4(-1, 0, 0, 0),           "Vec4::negate() failed");
    Vec4::negate(Vec4::UNIT_Y,&test1);
    CUAssertAlwaysLog(test1 == Vec4( 0,-1, 0, 0),           "Vec4::negate() failed");
    Vec4::negate(Vec4::UNIT_Z,&test1);
    CUAssertAlwaysLog(test1 == Vec4( 0, 0,-1, 0),           "Vec4::negate() failed");
    Vec4::negate(Vec4::UNIT_W,&test1);
    CUAssertAlwaysLog(test1 == Vec4( 0, 0, 0,-1),           "Vec4::negate() failed");
    
    test1.set(2,2,2,2);
    testptr = Vec4::reciprocate(test1,&test2);
    CUAssertAlwaysLog(testptr == &test2,                    "Vec4::reciprocate() failed");
    CUAssertAlwaysLog(test2.equals(Vec4(0.5,0.5,0.5,0.5)),  "Vec4::reciprocate() failed");
    testptr = Vec4::reciprocate(Vec4::ONE,&test2);
    CUAssertAlwaysLog(test2.equals(Vec4::ONE),              "Vec4::reciprocate() failed");
    
    
#pragma mark Arithmetic Test
    test1.set(-2,2,-3,3);
    test2.set(-2,2,-3,3);
    test2.clamp(Vec4(-3,-3,-4,-4),Vec4(3,3,4,4));
    CUAssertAlwaysLog(test1 == test2,                               "Method clamp() failed");
    
    test2.clamp(Vec4::ZERO,Vec4(3,3,4,4));
    CUAssertAlwaysLog(test1 != test2,                               "Method clamp() failed");
    CUAssertAlwaysLog(test2.x == 0 && test2.y == 2 && test2.z == 0 && test2.w == 3,
                      "Method clamp() failed");
    
    test2 = test1;
    test2.clamp(Vec4(-3,-3,-4,-4),Vec4::ZERO);
    CUAssertAlwaysLog(test1 != test2,                               "Method clamp() failed");
    CUAssertAlwaysLog(test2.x==-2 && test2.y == 0 && test2.z == -3 && test2.w == 0,
                      "Method clamp() failed");
    
    test2 = test1;
    test2.clamp(Vec4(-1,-1,-2,-2),Vec4(1,1,2,2));
    CUAssertAlwaysLog(test1 != test2,                               "Method clamp() failed");
    CUAssertAlwaysLog(test2.x==-1 && test2.y == 1 && test2.z == -2 && test2.w == 2,
                      "Method clamp() failed");
    
    test2 = test1;
    test3 = test2.getClamp(Vec4::ZERO,Vec4(3,3,4,4));
    CUAssertAlwaysLog(test1 == test2,                               "Method getClamp() failed");
    CUAssertAlwaysLog(test1 != test3,                               "Method getClamp() failed");
    CUAssertAlwaysLog(test3.x == 0 && test3.y == 2 && test3.z == 0 && test3.w == 3,
                      "Method getClamp() failed");
    
    test3 = test2.getClamp(Vec4(-3,-3,-4,-4),Vec4::ZERO);
    CUAssertAlwaysLog(test1 == test2,                               "Method getClamp() failed");
    CUAssertAlwaysLog(test1 != test3,                               "Method getClamp() failed");
    CUAssertAlwaysLog(test3.x == -2 && test3.y == 0 && test3.z == -3 && test3.w == 0,
                      "Method getClamp() failed");
    
    test3 = test2.getClamp(Vec4(-1,-1,-2,-2),Vec4(1,1,2,2));
    CUAssertAlwaysLog(test1 == test2,                               "Method getClamp() failed");
    CUAssertAlwaysLog(test1 != test3,                               "Method getClamp() failed");
    CUAssertAlwaysLog(test3.x == -1 && test3.y == 1 && test3.z == -2 && test3.w == 2,
                      "Method getClamp() failed");
    
    test1 = Vec4::HOMOG_X;
    test1.add(Vec4::UNIT_Y);
    test1.add(Vec4::UNIT_Z);
    CUAssertAlwaysLog(test1 == Vec4::ONE,                           "Method add() failed");
    
    test1 = Vec4::ONE;
    test1.add(test1);
    CUAssertAlwaysLog(test1 == Vec4(2,2,2,2),                       "Method add() failed");
    
    test1 = Vec4::ONE;
    test1.add(2,3,-2,1);
    CUAssertAlwaysLog(test1 == Vec4(3,4,-1,2),                      "Method add() failed");
    
    test1 = Vec4::HOMOG_X;
    test1.subtract(Vec4::UNIT_W);
    CUAssertAlwaysLog(test1 == Vec4::UNIT_X,                        "Method subtract() failed");
    
    test1 = Vec4::ONE;
    test1.subtract(test1);
    CUAssertAlwaysLog(test1 == Vec4::ZERO,                          "Method subtract() failed");
    
    test1 = Vec4::ONE;
    test1.subtract(2,3,-1,1);
    CUAssertAlwaysLog(test1 == Vec4(-1,-2,2,0),                     "Method subtract() failed");
    
    test1 = Vec4::ONE;
    test2 = Vec4::UNIT_X;
    test3 = Vec4::UNIT_Y;
    test4 = Vec4::UNIT_Z;
    test5 = Vec4::UNIT_W;
    test1.scale(2); test2.scale(2); test3.scale(2); test4.scale(2); test5.scale(2);
    CUAssertAlwaysLog(test1 == Vec4(2,2,2,2),             "Method scale() failed");
    CUAssertAlwaysLog(test2 == Vec4(2,0,0,0),             "Method scale() failed");
    CUAssertAlwaysLog(test3 == Vec4(0,2,0,0),             "Method scale() failed");
    CUAssertAlwaysLog(test4 == Vec4(0,0,2,0),             "Method scale() failed");
    CUAssertAlwaysLog(test5 == Vec4(0,0,0,2),             "Method scale() failed");
    
    test1 = Vec4::ONE;
    test2 = Vec4::UNIT_X;
    test3 = Vec4::UNIT_Y;
    test4 = Vec4::UNIT_Z;
    test5 = Vec4::UNIT_W;
    test1.scale(2,3,-1,-2); test2.scale(2,3,-1,-2); test3.scale(2,3,-1,-2);
    test4.scale(2,3,-1,-2); test5.scale(2,3,-1,-2);
    CUAssertAlwaysLog(test1 == Vec4(2,3,-1,-2),         "Method scale() failed");
    CUAssertAlwaysLog(test2 == Vec4(2,0,0,0),           "Method scale() failed");
    CUAssertAlwaysLog(test3 == Vec4(0,3,0,0),           "Method scale() failed");
    CUAssertAlwaysLog(test4 == Vec4(0,0,-1,0),          "Method scale() failed");
    CUAssertAlwaysLog(test5 == Vec4(0,0,0,-2),          "Method scale() failed");
    
    test1 = Vec4::ONE;
    test2 = Vec4::UNIT_X;
    test3 = Vec4::UNIT_Y;
    test4 = Vec4::UNIT_Z;
    test5 = Vec4::UNIT_W;
    Vec4 test6 = Vec4(-0.5,0.5,1.5,-1.5);
    test1.scale(test6); test2.scale(test6); test3.scale(test6);
    test4.scale(test6); test5.scale(test6);
    CUAssertAlwaysLog(test1 == Vec4(-0.5,0.5,1.5,-1.5), "Method scale() failed");
    CUAssertAlwaysLog(test2 == Vec4(-0.5,0,0,0),        "Method scale() failed");
    CUAssertAlwaysLog(test3 == Vec4(0,0.5,0,0),         "Method scale() failed");
    CUAssertAlwaysLog(test4 == Vec4(0,0,1.5,0),         "Method scale() failed");
    CUAssertAlwaysLog(test5 == Vec4(0,0,0,-1.5),        "Method scale() failed");
    
    test1 = Vec4::ONE;
    test2 = Vec4::UNIT_X;
    test3 = Vec4::UNIT_Y;
    test4 = Vec4::UNIT_Z;
    test5 = Vec4::UNIT_W;
    test1.divide(2); test2.divide(2); test3.divide(2); test4.divide(2); test5.divide(2);
    CUAssertAlwaysLog(test1.equals(Vec4(0.5,0.5,0.5,0.5)),  "Method divide() failed");
    CUAssertAlwaysLog(test2.equals(Vec4(0.5,0,0,0)),        "Method divide() failed");
    CUAssertAlwaysLog(test3.equals(Vec4(0,0.5,0,0)),        "Method divide() failed");
    CUAssertAlwaysLog(test4.equals(Vec4(0,0,0.5,0)),        "Method divide() failed");
    CUAssertAlwaysLog(test5.equals(Vec4(0,0,0,0.5)),        "Method divide() failed");
    
    test1 = Vec4::ONE;
    test2 = Vec4::UNIT_X;
    test3 = Vec4::UNIT_Y;
    test4 = Vec4::UNIT_Z;
    test5 = Vec4::UNIT_W;
    test1.divide(2,4,-2,-4); test2.divide(2,4,-2,-4); test3.divide(2,4,-2,-4);
    test4.divide(2,4,-2,-4); test5.divide(2,4,-2,-4);
    CUAssertAlwaysLog(test1.equals(Vec4(0.5,0.25,-0.5,-0.25)),  "Method divide() failed");
    CUAssertAlwaysLog(test2.equals(Vec4(0.5,0,0,0)),            "Method divide() failed");
    CUAssertAlwaysLog(test3.equals(Vec4(0,0.25,0,0)),           "Method divide() failed");
    CUAssertAlwaysLog(test4.equals(Vec4(0,0,-0.5,0)),           "Method divide() failed");
    CUAssertAlwaysLog(test5.equals(Vec4(0,0,0,-0.25)),          "Method divide() failed");
    
    test1 = Vec4::ONE;
    test2 = Vec4::UNIT_X;
    test3 = Vec4::UNIT_Y;
    test4 = Vec4::UNIT_Z;
    test5 = Vec4::UNIT_W;
    test6.set(-0.5,0.5,0.25,-0.25);
    test1.divide(test6); test2.divide(test6); test3.divide(test6);
    test4.divide(test6); test5.divide(test6);
    CUAssertAlwaysLog(test1.equals(Vec4(-2,2,4,-4)),         "Method divide() failed");
    CUAssertAlwaysLog(test2.equals(Vec4(-2,0,0,0)),          "Method divide() failed");
    CUAssertAlwaysLog(test3.equals(Vec4(0,2,0,0)),           "Method divide() failed");
    CUAssertAlwaysLog(test4.equals(Vec4(0,0,4,0)),           "Method divide() failed");
    CUAssertAlwaysLog(test5.equals(Vec4(0,0,0,-4)),          "Method divide() failed");
    
    test1 = Vec4::ONE;
    test2 = Vec4::UNIT_X;
    test3 = Vec4::UNIT_Y;
    test4 = Vec4::UNIT_Z;
    test5 = Vec4::UNIT_W;
    test1.negate();test2.negate(); test3.negate(); test4.negate(); test5.negate();
    CUAssertAlwaysLog(test1 == Vec4(-1,-1,-1,-1),       "Method negate() failed");
    CUAssertAlwaysLog(test2 == Vec4(-1, 0, 0, 0),       "Method negate() failed");
    CUAssertAlwaysLog(test3 == Vec4( 0,-1, 0, 0),       "Method negate() failed");
    CUAssertAlwaysLog(test4 == Vec4( 0, 0,-1, 0),       "Method negate() failed");
    CUAssertAlwaysLog(test5 == Vec4( 0, 0, 0,-1),       "Method negate() failed");

    test1 = Vec4::ONE;
    test2 = Vec4::UNIT_X;
    test3 = Vec4::UNIT_Y;
    test4 = Vec4::UNIT_Z;
    test5 = Vec4::UNIT_W;
    test6 = test1.getNegation();
    CUAssertAlwaysLog(test6 != test1,                   "Method getNegation() failed");
    CUAssertAlwaysLog(test6 == Vec4(-1,-1,-1,-1),       "Method getNegation() failed");
    test6 = test2.getNegation();
    CUAssertAlwaysLog(test6 == Vec4(-1, 0, 0, 0),       "Method getNegation() failed");
    test6 = test3.getNegation();
    CUAssertAlwaysLog(test6 == Vec4( 0,-1, 0, 0),       "Method getNegation() failed");
    test6 = test4.getNegation();
    CUAssertAlwaysLog(test6 == Vec4( 0, 0,-1, 0),       "Method getNegation() failed");
    test6 = test5.getNegation();
    CUAssertAlwaysLog(test6 == Vec4( 0, 0, 0,-1),       "Method getNegation() failed");

    
    test1.set(2,2,2,2);
    test2 = Vec4::ONE;
    test1.reciprocate(); test2.reciprocate();
    CUAssertAlwaysLog(test1.equals(Vec4(0.5,0.5,0.5,0.5)),  "Method reciprocate() failed");
    CUAssertAlwaysLog(test2.equals(Vec4::ONE),              "Method reciprocate() failed");

    test1.set(2,2,2,2);
    test2 = Vec4::ONE;
    test3 = test1.getReciprocal();
    CUAssertAlwaysLog(test3 != test1,                       "Method getReciprocal() failed");
    CUAssertAlwaysLog(test3.equals(Vec4(0.5,0.5,0.5,0.5)),  "Method getReciprocal() failed");
    test3 = test2.getReciprocal();
    CUAssertAlwaysLog(test3.equals(Vec4::ONE),              "Method getReciprocal() failed");

    test1 = Vec4::ONE;
    test2 = Vec4::UNIT_X;
    test3 = Vec4::UNIT_Y;
    test4 = Vec4::UNIT_Z;
    test5 = Vec4::UNIT_W;
    test1.map(asinf); test2.map(asinf); test3.map(asinf);
    test4.map(asinf); test5.map(asinf);
    CUAssertAlwaysLog(CU_MATH_APPROX(test1.x,M_PI_2,CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test1.y,M_PI_2,CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test1.z,M_PI_2,CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test1.w,M_PI_2,CU_MATH_EPSILON),
                      "Method map() failed");
    CUAssertAlwaysLog(CU_MATH_APPROX(test2.x,M_PI_2,CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test2.y,0,CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test2.z,0,CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test2.w,0,CU_MATH_EPSILON),
                      "Method map() failed");
    CUAssertAlwaysLog(CU_MATH_APPROX(test3.x,0,CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test3.y,M_PI_2,CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test3.z,0,CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test2.w,0,CU_MATH_EPSILON),
                      "Method map() failed");
    CUAssertAlwaysLog(CU_MATH_APPROX(test4.x,0,CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test4.y,0,CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test4.z,M_PI_2,CU_MATH_EPSILON)&&
                      CU_MATH_APPROX(test2.w,0,CU_MATH_EPSILON),
                      "Method map() failed");
    CUAssertAlwaysLog(CU_MATH_APPROX(test5.x,0,CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.y,0,CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.z,0,CU_MATH_EPSILON)&&
                      CU_MATH_APPROX(test5.w,M_PI_2,CU_MATH_EPSILON),
                      "Method map() failed");
    
    test1 = Vec4::ONE;
    test2 = Vec4::UNIT_X;
    test3 = Vec4::UNIT_Y;
    test4 = Vec4::UNIT_Z;
    test5 = Vec4::UNIT_W;
    test6 = test1.getMap(asinf);
    CUAssertAlwaysLog(test1 != test6,   "Method getMap() failed");
    CUAssertAlwaysLog(CU_MATH_APPROX(test6.x,M_PI_2,CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test6.y,M_PI_2,CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test6.z,M_PI_2,CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test6.w,M_PI_2,CU_MATH_EPSILON),
                      "Method getMap() failed");
    test6 = test2.getMap(asinf);
    CUAssertAlwaysLog(CU_MATH_APPROX(test6.x,M_PI_2,CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test6.y,0,CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test6.z,0,CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test6.w,0,CU_MATH_EPSILON),
                      "Method getMap() failed");
    test6 = test3.getMap(asinf);
    CUAssertAlwaysLog(CU_MATH_APPROX(test6.x,0,CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test6.y,M_PI_2,CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test6.z,0,CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test6.w,0,CU_MATH_EPSILON),
                      "Method getMap() failed");
    test6 = test4.getMap(asinf);
    CUAssertAlwaysLog(CU_MATH_APPROX(test6.x,0,CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test6.y,0,CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test6.z,M_PI_2,CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test6.w,0,CU_MATH_EPSILON),
                      "Method getMap() failed");
    test6 = test5.getMap(asinf);
    CUAssertAlwaysLog(CU_MATH_APPROX(test6.x,0,CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test6.y,0,CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test6.z,0,CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test6.w,M_PI_2,CU_MATH_EPSILON),
                      "Method getMap() failed");

#pragma mark Operator Test
    test1 = Vec4::HOMOG_X;
    test1 += Vec4::UNIT_Y;
    test1 += Vec4::UNIT_Z;
    CUAssertAlwaysLog(test1 == Vec4::ONE,               "Addition operation failed");
    
    test1 = Vec4::ONE;
    test1 += test1;
    CUAssertAlwaysLog(test1 == Vec4(2,2,2,2),           "Addition operation failed");
    CUAssertAlwaysLog(Vec4::HOMOG_X+Vec4::UNIT_Y+Vec4::UNIT_Z == Vec4::ONE, "Addition operation failed");
    CUAssertAlwaysLog(Vec4::ONE+Vec4::ONE == Vec4(2,2,2,2),                 "Addition operation failed");
    
    test1 = Vec4::UNIT_X;
    test1 -= Vec4::UNIT_W;
    CUAssertAlwaysLog(test1 == Vec4(1,0,0,-1),          "Subtraction operation failed");
    
    test1 = Vec4::ONE;
    test1 -= test1;
    CUAssertAlwaysLog(test1 == Vec4::ZERO,              "Subtraction operation failed");
    CUAssertAlwaysLog(Vec4::UNIT_X-Vec4::UNIT_W == Vec4(1,0,0,-1),          "Subtraction operation failed");
    CUAssertAlwaysLog(Vec4::ONE-Vec4::ONE == Vec4::ZERO,                    "Subtraction operation failed");
    
    test1 = Vec4::ONE;
    test2 = Vec4::UNIT_X;
    test3 = Vec4::UNIT_Y;
    test4 = Vec4::UNIT_Z;
    test5 = Vec4::UNIT_W;
    test1 *= 2; test2 *= 2; test3 *= 2; test4 *= 2; test5 *= 2;
    CUAssertAlwaysLog(test1 == Vec4(2,2,2,2),           "Scaling operation failed");
    CUAssertAlwaysLog(test2 == Vec4(2,0,0,0),           "Scaling operation failed");
    CUAssertAlwaysLog(test3 == Vec4(0,2,0,0),           "Scaling operation failed");
    CUAssertAlwaysLog(test4 == Vec4(0,0,2,0),           "Scaling operation failed");
    CUAssertAlwaysLog(test5 == Vec4(0,0,0,2),           "Scaling operation failed");
    CUAssertAlwaysLog(Vec4::ONE*2 == Vec4(2,2,2,2),     "Scaling operation failed");
    CUAssertAlwaysLog(Vec4::UNIT_X*2 == Vec4(2,0,0,0),  "Scaling operation failed");
    CUAssertAlwaysLog(Vec4::UNIT_Y*2 == Vec4(0,2,0,0),  "Scaling operation failed");
    CUAssertAlwaysLog(Vec4::UNIT_Z*2 == Vec4(0,0,2,0),  "Scaling operation failed");
    CUAssertAlwaysLog(Vec4::UNIT_W*2 == Vec4(0,0,0,2),  "Scaling operation failed");
    CUAssertAlwaysLog(2*Vec4::ONE == Vec4(2,2,2,2),     "Scaling operation failed");
    CUAssertAlwaysLog(2*Vec4::UNIT_X == Vec4(2,0,0,0),  "Scaling operation failed");
    CUAssertAlwaysLog(2*Vec4::UNIT_Y == Vec4(0,2,0,0),  "Scaling operation failed");
    CUAssertAlwaysLog(2*Vec4::UNIT_Z == Vec4(0,0,2,0),  "Scaling operation failed");
    CUAssertAlwaysLog(2*Vec4::UNIT_W == Vec4(0,0,0,2),  "Scaling operation failed");
    
    test1 = Vec4::ONE;
    test2 = Vec4::UNIT_X;
    test3 = Vec4::UNIT_Y;
    test4 = Vec4::UNIT_Z;
    test5 = Vec4::UNIT_W;
    test6 = Vec4(-0.5,0.5,1.5,-1.5);
    test1 *= test6; test2 *= test6; test3 *= test6; test4 *= test6; test5 *= test6;
    CUAssertAlwaysLog(test1 == Vec4(-0.5,0.5,1.5,-1.5), "Scaling operation failed");
    CUAssertAlwaysLog(test2 == Vec4(-0.5,0,0,0),        "Scaling operation failed");
    CUAssertAlwaysLog(test3 == Vec4(0,0.5,0,0),         "Scaling operation failed");
    CUAssertAlwaysLog(test4 == Vec4(0,0,1.5,0),         "Scaling operation failed");
    CUAssertAlwaysLog(test5 == Vec4(0,0,0,-1.5),        "Scaling operation failed");
    CUAssertAlwaysLog(Vec3::ONE*test6 == Vec4(-0.5,0.5,1.5,-1.5),   "Scaling operation failed");
    CUAssertAlwaysLog(Vec4::UNIT_X*test6 == Vec4(-0.5,0,0,0),       "Scaling operation failed");
    CUAssertAlwaysLog(Vec4::UNIT_Y*test6 == Vec4(0,0.5,0,0),        "Scaling operation failed");
    CUAssertAlwaysLog(Vec4::UNIT_Z*test6 == Vec4(0,0,1.5,0),        "Scaling operation failed");
    CUAssertAlwaysLog(Vec4::UNIT_W*test6 == Vec4(0,0,0,-1.5),       "Scaling operation failed");
    
    test1 = Vec4::ONE;
    test2 = Vec4::UNIT_X;
    test3 = Vec4::UNIT_Y;
    test4 = Vec4::UNIT_Z;
    test5 = Vec4::UNIT_W;
    test1 /= 0.5; test2 /= 0.5; test3 /= 0.5; test4 /= 0.5; test5 /= 0.5;
    CUAssertAlwaysLog(test1 == Vec4(2,2,2,2),           "Division operation failed");
    CUAssertAlwaysLog(test2 == Vec4(2,0,0,0),           "Division operation failed");
    CUAssertAlwaysLog(test3 == Vec4(0,2,0,0),           "Division operation failed");
    CUAssertAlwaysLog(test4 == Vec4(0,0,2,0),           "Division operation failed");
    CUAssertAlwaysLog(test5 == Vec4(0,0,0,2),           "Division operation failed");
    CUAssertAlwaysLog(Vec4::ONE/0.5 == Vec4(2,2,2,2),   "Division operation failed");
    CUAssertAlwaysLog(Vec4::UNIT_X/0.5 == Vec4(2,0,0,0),"Division operation failed");
    CUAssertAlwaysLog(Vec4::UNIT_Y/0.5 == Vec4(0,2,0,0),"Division operation failed");
    CUAssertAlwaysLog(Vec4::UNIT_Z/0.5 == Vec4(0,0,2,0),"Division operation failed");
    CUAssertAlwaysLog(Vec4::UNIT_W/0.5 == Vec4(0,0,0,2),"Division operation failed");
    
    test1 = Vec4::ONE;
    test2 = Vec4::UNIT_X;
    test3 = Vec4::UNIT_Y;
    test4 = Vec4::UNIT_Z;
    test5 = Vec4::UNIT_W;
    test6.set(1/2.0f,1/4.0f,-1/2.0f,-1/4.0f);
    test1 /= test6; test2 /= test6; test3 /= test6; test4 /= test6; test5 /= test6;
    CUAssertAlwaysLog(test1.equals(Vec4(2,4,-2,-4)),             "Division operation failed");
    CUAssertAlwaysLog(test2.equals(Vec4(2,0,0,0)),               "Division operation failed");
    CUAssertAlwaysLog(test3.equals(Vec4(0,4,0,0)),               "Division operation failed");
    CUAssertAlwaysLog(test4.equals(Vec4(0,0,-2,0)),              "Division operation failed");
    CUAssertAlwaysLog(test5.equals(Vec4(0,0,0,-4)),              "Division operation failed");
    CUAssertAlwaysLog(Vec4(2,4,-2,-4).equals(Vec4::ONE/test6),   "Division operation failed");
    CUAssertAlwaysLog(Vec4(2,0,0,0).equals(Vec4::UNIT_X/test6),  "Division operation failed");
    CUAssertAlwaysLog(Vec4(0,4,0,0).equals(Vec4::UNIT_Y/test6),  "Division operation failed");
    CUAssertAlwaysLog(Vec4(0,0,-2,0).equals(Vec4::UNIT_Z/test6), "Division operation failed");
    CUAssertAlwaysLog(Vec4(0,0,0,-4).equals(Vec4::UNIT_W/test6), "Division operation failed");
    
    CUAssertAlwaysLog(-Vec4::ONE == Vec4(-1,-1,-1,-1),      "Negation operation failed");
    CUAssertAlwaysLog(-Vec4::UNIT_X == Vec4(-1, 0, 0, 0),   "Negation operation failed");
    CUAssertAlwaysLog(-Vec4::UNIT_Y == Vec4( 0,-1, 0, 0),   "Negation operation failed");
    CUAssertAlwaysLog(-Vec4::UNIT_Z == Vec4( 0, 0,-1, 0),   "Negation operation failed");
    CUAssertAlwaysLog(-Vec4::UNIT_W == Vec4( 0, 0, 0,-1),   "Negation operation failed");
    
#pragma mark Linear Attributes
    angle = Vec4::UNIT_X.getAngle(Vec4::UNIT_Z);
    CUAssertAlwaysLog(CU_MATH_APPROX(angle,M_PI_2,CU_MATH_EPSILON),       "Method getAngle() failed");
    angle = Vec4::UNIT_Y.getAngle(Vec4::UNIT_W);
    CUAssertAlwaysLog(CU_MATH_APPROX(angle,M_PI_2,CU_MATH_EPSILON),      "Method getAngle() failed");
    angle = Vec4::UNIT_Y.getAngle(Vec4::UNIT_X);
    CUAssertAlwaysLog(CU_MATH_APPROX(angle,M_PI_2,CU_MATH_EPSILON),       "Method getAngle() failed");
    angle = Vec4::ONE.getAngle(Vec4::UNIT_W);
    CUAssertAlwaysLog(CU_MATH_APPROX(angle,1.04719746,CU_MATH_EPSILON), "Method getAngle() failed");
    
    CUAssertAlwaysLog(Vec4::ZERO.isZero(),          "Method isZero() failed");
    CUAssertAlwaysLog(!Vec4::UNIT_X.isZero(),       "Method isZero() failed");
    CUAssertAlwaysLog(!Vec4::UNIT_Y.isZero(),       "Method isZero() failed");
    CUAssertAlwaysLog(!Vec4::UNIT_Z.isZero(),       "Method isZero() failed");
    CUAssertAlwaysLog(!Vec4::UNIT_W.isZero(),       "Method isZero() failed");
    CUAssertAlwaysLog(!Vec4::ONE.isZero(),          "Method isZero() failed");
    
    test1.set(0,0,CU_MATH_EPSILON*0.5,-CU_MATH_EPSILON*0.5);
    CUAssertAlwaysLog(Vec4::ZERO.isNearZero(),      "Method isNearZero() failed");
    CUAssertAlwaysLog(test1.isNearZero(),           "Method isNearZero() failed");
    CUAssertAlwaysLog(!Vec4::UNIT_X.isNearZero(),   "Method isNearZero() failed");
    CUAssertAlwaysLog(!Vec4::UNIT_Y.isNearZero(),   "Method isNearZero() failed");
    CUAssertAlwaysLog(!Vec4::UNIT_Z.isNearZero(),   "Method isNearZero() failed");
    CUAssertAlwaysLog(!Vec4::UNIT_W.isNearZero(),   "Method isNearZero() failed");
    CUAssertAlwaysLog(!Vec4::ONE.isNearZero(),      "Method isNearZero() failed");
    
    CUAssertAlwaysLog(!Vec4::ZERO.isOne(),          "Method isOne() failed");
    CUAssertAlwaysLog(!Vec4::UNIT_X.isOne(),        "Method isOne() failed");
    CUAssertAlwaysLog(!Vec4::UNIT_Y.isOne(),        "Method isOne() failed");
    CUAssertAlwaysLog(!Vec4::UNIT_Z.isOne(),        "Method isOne() failed");
    CUAssertAlwaysLog(!Vec4::UNIT_W.isOne(),        "Method isOne() failed");
    CUAssertAlwaysLog(Vec4::ONE.isOne(),            "Method isOne() failed");
    
    CUAssertAlwaysLog(!Vec4::ZERO.isInvertible(),   "Method isInvertible() failed");
    CUAssertAlwaysLog(!Vec4::UNIT_X.isInvertible(), "Method isInvertible() failed");
    CUAssertAlwaysLog(!Vec4::UNIT_Y.isInvertible(), "Method isInvertible() failed");
    CUAssertAlwaysLog(!Vec4::UNIT_Z.isInvertible(), "Method isInvertible() failed");
    CUAssertAlwaysLog(!Vec4::UNIT_W.isInvertible(), "Method isInvertible() failed");
    CUAssertAlwaysLog(Vec4::ONE.isInvertible(),     "Method isInvertible() failed");
   
    CUAssertAlwaysLog(!Vec4::ZERO.isUnit(),         "Method isUnit() failed");
    CUAssertAlwaysLog(Vec4::UNIT_X.isUnit(),        "Method isUnit() failed");
    CUAssertAlwaysLog(Vec4::UNIT_Y.isUnit(),        "Method isUnit() failed");
    CUAssertAlwaysLog(Vec4::UNIT_Z.isUnit(),        "Method isUnit() failed");
    CUAssertAlwaysLog(Vec4::UNIT_W.isUnit(),        "Method isUnit() failed");
    CUAssertAlwaysLog(!Vec4::ONE.isUnit(),          "Method isUnit() failed");
    CUAssertAlwaysLog(Vec4(1/sqrtf(2),0,0,1/sqrtf(2)).isUnit(),  "Method isUnit() failed");
    
    CUAssertAlwaysLog(!Vec4::ZERO.isHomogenous(),   "Method isHomogenous() failed");
    CUAssertAlwaysLog(!Vec4::UNIT_X.isHomogenous(), "Method isHomogenous() failed");
    CUAssertAlwaysLog(!Vec4::UNIT_Y.isHomogenous(), "Method isHomogenous() failed");
    CUAssertAlwaysLog(!Vec4::UNIT_Z.isHomogenous(), "Method isHomogenous() failed");
    CUAssertAlwaysLog(Vec4::UNIT_W.isHomogenous(),  "Method isHomogenous() failed");
    CUAssertAlwaysLog(Vec4::ONE.isHomogenous(),     "Method isHomogenous() failed");
    
    CUAssertAlwaysLog(Vec4::ZERO.distance(Vec4::UNIT_X) == 1,   "Method distance() failed");
    CUAssertAlwaysLog(Vec4::UNIT_X.distance(Vec4::ZERO) == 1,   "Method distance() failed");
    CUAssertAlwaysLog(Vec4::ZERO.distance(Vec4::UNIT_Y) == 1,   "Method distance() failed");
    CUAssertAlwaysLog(Vec4::UNIT_Y.distance(Vec4::ZERO) == 1,   "Method distance() failed");
    CUAssertAlwaysLog(Vec4::ZERO.distance(Vec4::UNIT_Z) == 1,   "Method distance() failed");
    CUAssertAlwaysLog(Vec4::UNIT_Z.distance(Vec4::ZERO) == 1,   "Method distance() failed");
    CUAssertAlwaysLog(Vec4::ZERO.distance(Vec4::ONE) == 2,       "Method distance() failed");
    CUAssertAlwaysLog(Vec4::ONE.distance(Vec4::ZERO) == 2,       "Method distance() failed");
    CUAssertAlwaysLog(Vec4::ONE.distance(Vec4::UNIT_Z) == sqrtf(3),     "Method distance() failed");
    CUAssertAlwaysLog(Vec4::UNIT_Z.distance(Vec4::ONE) == sqrtf(3),     "Method distance() failed");
    CUAssertAlwaysLog(Vec4(1,3,2,-1).distance(Vec4(2,-1,0,1)) == 5,     "Method distance() failed");
    CUAssertAlwaysLog(Vec4(2,-1,0,1).distance(Vec4(1,3,2,-1)) == 5,     "Method distance() failed");
    
    CUAssertAlwaysLog(Vec4::ZERO.distanceSquared(Vec4::UNIT_X) == 1,    "Method distanceSquared() failed");
    CUAssertAlwaysLog(Vec4::UNIT_X.distanceSquared(Vec4::ZERO) == 1,    "Method distanceSquared() failed");
    CUAssertAlwaysLog(Vec4::ZERO.distanceSquared(Vec4::UNIT_Y) == 1,    "Method distanceSquared() failed");
    CUAssertAlwaysLog(Vec4::UNIT_Y.distanceSquared(Vec4::ZERO) == 1,    "Method distanceSquared() failed");
    CUAssertAlwaysLog(Vec4::ZERO.distanceSquared(Vec4::UNIT_Z) == 1,    "Method distanceSquared() failed");
    CUAssertAlwaysLog(Vec4::UNIT_Z.distanceSquared(Vec4::ZERO) == 1,    "Method distanceSquared() failed");
    CUAssertAlwaysLog(Vec4::ZERO.distanceSquared(Vec4::ONE) == 4,       "Method distanceSquared() failed");
    CUAssertAlwaysLog(Vec4::ONE.distanceSquared(Vec4::ZERO) == 4,       "Method distanceSquared() failed");
    CUAssertAlwaysLog(Vec4::ONE.distanceSquared(Vec4::UNIT_Z) == 3,     "Method distance() failed");
    CUAssertAlwaysLog(Vec4::UNIT_Z.distanceSquared(Vec4::ONE) == 3,     "Method distance() failed");
    CUAssertAlwaysLog(Vec4(1,3,2,-1).distanceSquared(Vec4(2,-1,0,1)) == 25,     "Method distance() failed");
    CUAssertAlwaysLog(Vec4(2,-1,0,1).distanceSquared(Vec4(1,3,2,-1)) == 25,     "Method distance() failed");
    
    CUAssertAlwaysLog(Vec4::ZERO.length() == 0,             "Method length() failed");
    CUAssertAlwaysLog(Vec4::UNIT_X.length() == 1,           "Method length() failed");
    CUAssertAlwaysLog(Vec4::UNIT_Y.length() == 1,           "Method length() failed");
    CUAssertAlwaysLog(Vec4::UNIT_Z.length() == 1,           "Method length() failed");
    CUAssertAlwaysLog(Vec4::UNIT_Z.length() == 1,           "Method length() failed");
    CUAssertAlwaysLog(Vec4::ONE.length() == 2,              "Method length() failed");
    CUAssertAlwaysLog(Vec4(-2,4,1,2).length() == 5,         "Method length() failed");
    
    CUAssertAlwaysLog(Vec4::ZERO.lengthSquared() == 0,      "Method length() failed");
    CUAssertAlwaysLog(Vec4::UNIT_X.lengthSquared() == 1,    "Method length() failed");
    CUAssertAlwaysLog(Vec4::UNIT_Y.lengthSquared() == 1,    "Method length() failed");
    CUAssertAlwaysLog(Vec4::UNIT_Z.lengthSquared() == 1,    "Method length() failed");
    CUAssertAlwaysLog(Vec4::ONE.lengthSquared() == 4,       "Method length() failed");
    CUAssertAlwaysLog(Vec4(-2,4,1,2).lengthSquared() == 25, "Method length() failed");
    
#pragma mark Linear Algebra Test
    CUAssertAlwaysLog(Vec4::UNIT_X.dot(Vec4::UNIT_Y) == 0,  "Method dot() failed");
    CUAssertAlwaysLog(Vec4::UNIT_X.dot(Vec4::UNIT_W) == 0,  "Method dot() failed");
    CUAssertAlwaysLog(Vec4::ONE.dot(Vec4::ZERO) == 0,       "Method dot() failed");
    CUAssertAlwaysLog(Vec4::ONE.dot(Vec4::ONE) == 4,        "Method dot() failed");
    CUAssertAlwaysLog(Vec4::UNIT_W.dot(Vec4::UNIT_W) == 1,  "Method dot() failed");
    CUAssertAlwaysLog(Vec4::HOMOG_Z.dot(Vec4::HOMOG_Z) == 2,"Method dot() failed");
    
    test1.set(1/2.0,1/2.0,1/2.0,1/2.0);
    CUAssertAlwaysLog(CU_MATH_APPROX(test1.dot(test1),1,CU_MATH_EPSILON), "Method dot() failed");
    
    test2 = test1;
    test1 = Vec4::ONE;
    CUAssertAlwaysLog(test1.normalize().equals(test2),          "Method normalize() failed.");
    test1 = Vec4::UNIT_X;
    CUAssertAlwaysLog(test1.normalize().equals(Vec4::UNIT_X),   "Method normalize() failed.");
    test1 = Vec4::UNIT_Y;
    CUAssertAlwaysLog(test1.normalize().equals(Vec4::UNIT_Y),   "Method normalize() failed.");
    test1 = Vec4::UNIT_Z;
    CUAssertAlwaysLog(test1.normalize().equals(Vec4::UNIT_Z),   "Method normalize() failed.");
    test1 = Vec4::UNIT_W;
    CUAssertAlwaysLog(test1.normalize().equals(Vec4::UNIT_W),   "Method normalize() failed.");
    
    test1 = Vec4::ONE;
    test3 = test1.getNormalization();
    CUAssertAlwaysLog(test1 != test3,                                   "Method getNormalization() failed.");
    CUAssertAlwaysLog(test3.equals(test2),                              "Method getNormalization() failed.");
    test1 = Vec4::UNIT_X;
    CUAssertAlwaysLog(test1.getNormalization().equals(Vec4::UNIT_X),    "Method getNormalization() failed.");
    test1 = Vec4::UNIT_Y;
    CUAssertAlwaysLog(test1.getNormalization().equals(Vec4::UNIT_Y),    "Method getNormalization() failed.");
    test1 = Vec4::UNIT_Z;
    CUAssertAlwaysLog(test1.getNormalization().equals(Vec4::UNIT_Z),    "Method getNormalization() failed.");
    test1 = Vec4::UNIT_W;
    CUAssertAlwaysLog(test1.getNormalization().equals(Vec4::UNIT_W),    "Method getNormalization() failed.");
    
    test1 = Vec4::ZERO;
    test2 = test1.getMidpoint(Vec4::ONE);
    CUAssertAlwaysLog(test1 != test2,                           "Method getMidpoint() failed.");
    CUAssertAlwaysLog(test2 == Vec4(0.5,0.5,0.5,0.5),           "Method getMidpoint() failed.");
    
    test1 = Vec4::UNIT_X.getMidpoint(Vec4::UNIT_Y);
    test2 = Vec4::UNIT_Y.getMidpoint(Vec4::UNIT_W);
    CUAssertAlwaysLog(test1 == Vec4(0.5,0.5,0,0),               "Method getMidpoint() failed.");
    CUAssertAlwaysLog(test2 == Vec4(0,0.5,0,0.5),               "Method getMidpoint() failed.");
    
    test1.set(2,3,-1,4);
    test1.project(Vec4::UNIT_X);
    CUAssertAlwaysLog(test1 == Vec4(2,0,0,0),                   "Method project() failed.");
    test1.set(2,3,-1,4);
    test1.project(Vec4::UNIT_Y);
    CUAssertAlwaysLog(test1 == Vec4(0,3,0,0),                   "Method project() failed.");
    test1.set(2,3,-1,4);
    test1.project(Vec4::UNIT_Z);
    CUAssertAlwaysLog(test1 == Vec4(0,0,-1,0),                  "Method project() failed.");
    test1.set(2,3,-1,4);
    test1.project(Vec4::UNIT_W);
    CUAssertAlwaysLog(test1 == Vec4(0,0,0,4),                   "Method project() failed.");
    test1 = 6*Vec4::UNIT_W;
    test1.project(Vec4::ONE);
    CUAssertAlwaysLog(test1 == Vec4(1.5,1.5,1.5,1.5),           "Method project() failed.");
    
    
    test1.set(2,3,-1,4);
    test2 = test1.getProjection(Vec4::UNIT_X);
    CUAssertAlwaysLog(test1 != test2,                           "Method getProjection() failed.");
    CUAssertAlwaysLog(test2 == Vec4(2,0,0,0),                   "Method getProjection() failed.");
    test2 = test1.getProjection(Vec4::UNIT_Y);
    CUAssertAlwaysLog(test2 == Vec4(0,3,0,0),                   "Method getProjection() failed.");
    test2 = test1.getProjection(Vec4::UNIT_Z);
    CUAssertAlwaysLog(test2 == Vec4(0,0,-1,0),                  "Method getProjection() failed.");
    test2 = test1.getProjection(Vec4::UNIT_W);
    CUAssertAlwaysLog(test2 == Vec4(0,0,0,4),                   "Method getProjection() failed.");
    test1 = 6*Vec4::UNIT_Z;
    test2 = test1.getProjection(Vec4::ONE);
    CUAssertAlwaysLog(test2 == Vec4(1.5,1.5,1.5,1.5),           "Method getProjection() failed.");
    
    test1.set(1,4,-4,2);
    test1.homogenize();
    CUAssertAlwaysLog(test1 == Vec4(0.5,2,-2,1),                "Method homogenize() failed.");
    test1.set(1,4,-4,0);
    test1.homogenize();
    CUAssertAlwaysLog(test1 == Vec4(1,4,-4,1),                  "Method homogenize() failed.");
    test1 = Vec4::ONE;
    test1.homogenize();
    CUAssertAlwaysLog(test1 == Vec4::ONE,                       "Method homogenize() failed.");

    test1.set(1,4,-4,2);
    test2 = test1.getHomogenized();
    CUAssertAlwaysLog(test1 != test2,                           "Method getHomogenized() failed.");
    CUAssertAlwaysLog(test2 == Vec4(0.5,2,-2,1),                "Method getHomogenized() failed.");
    test1.set(1,4,-4,0);
    test2 = test1.getHomogenized();
    CUAssertAlwaysLog(test2 == Vec4(1,4,-4,1),                  "Method getHomogenized() failed.");
    test1 = Vec4::ONE;
    test2 = test1.getHomogenized();
    CUAssertAlwaysLog(test2 == Vec4::ONE,                       "Method getHomogenized() failed.");

    test1 = Vec4::ONE;
    test2.set(2,3,0,-1);
    test1.lerp(test2,0);
    CUAssertAlwaysLog(test1 == Vec4::ONE,                       "Method lerp() failed.");
    test1.lerp(test2,1);
    CUAssertAlwaysLog(test1 == test2,                           "Method lerp() failed.");
    test1 = Vec4::ONE;
    test1.lerp(test2,0.5);
    CUAssertAlwaysLog(test1 == Vec4(1.5,2,0.5,0),               "Method lerp() failed.");
    test1 = Vec4::ONE;
    test1.lerp(test2,-1);
    CUAssertAlwaysLog(test1 == Vec4(0,-1,2,3),                  "Method lerp() failed.");
    test1 = Vec4::ONE;
    test1.lerp(test2,2);
    CUAssertAlwaysLog(test1 == Vec4(3,5,-1,-3),                 "Method lerp() failed.");
    
    test1 = Vec4::ONE;
    test2.set(2,3,0,-1);
    test3 = test1.getLerp(test2,0);
    CUAssertAlwaysLog(test3 == Vec4::ONE,                       "Method getLerp() failed.");
    test3 = test1.getLerp(test2,1);
    CUAssertAlwaysLog(test1 != test3,                           "Method getLerp() failed.");
    CUAssertAlwaysLog(test3 == test2,                           "Method getLerp() failed.");
    test3 = test1.getLerp(test2,0.5);
    CUAssertAlwaysLog(test3 == Vec4(1.5,2,0.5,0),               "Method getLerp() failed.");
    test3 = test1.getLerp(test2,-1);
    CUAssertAlwaysLog(test3 == Vec4(0,-1,2,3),                  "Method getLerp() failed.");
    test3 = test1.getLerp(test2,2);
    CUAssertAlwaysLog(test3 == Vec4(3,5,-1,-3),                 "Method getLerp() failed.");
    
#pragma mark Static Linear Algebra Test
    CUAssertAlwaysLog(Vec4::dot(Vec4::UNIT_X,Vec4::UNIT_W) == 0,    "Vec4::dot() failed");
    CUAssertAlwaysLog(Vec4::dot(Vec4::ONE,Vec4::ZERO) == 0,         "Vec4::dot() failed");
    CUAssertAlwaysLog(Vec4::dot(Vec4::ONE,Vec4::ONE) == 4,          "Vec4::dot() failed");
    CUAssertAlwaysLog(Vec4::dot(Vec4::HOMOG_X,Vec4::HOMOG_X) == 2,  "Vec4::dot() failed");
    
    test1.set(1/2.0,1/2.0,1/2.0,1/2.0);
    testptr = Vec4::normalize(Vec4::ONE,&test2);
    CUAssertAlwaysLog(testptr == &test2,                        "Vec4::normalize() failed");
    CUAssertAlwaysLog(test2.equals(test1),                      "Vec4::normalize() failed.");
    Vec4::normalize(Vec4::UNIT_X,&test2);
    CUAssertAlwaysLog(test2.equals(Vec4::UNIT_X),               "Vec4::normalize() failed.");
    Vec4::normalize(Vec4::UNIT_Y,&test2);
    CUAssertAlwaysLog(test2.equals(Vec4::UNIT_Y),               "Vec4::normalize() failed.");
    Vec4::normalize(Vec4::UNIT_Z,&test2);
    CUAssertAlwaysLog(test2.equals(Vec4::UNIT_Z),               "Vec4::normalize() failed.");
    Vec4::normalize(Vec4::UNIT_W,&test2);
    CUAssertAlwaysLog(test2.equals(Vec4::UNIT_W),               "Vec4::normalize() failed.");
    
    test1.set(1,4,-4,2);
    testptr = Vec4::homogenize(test1,&test2);
    CUAssertAlwaysLog(testptr == &test2,                        "Vec4::homogenize() failed");
    CUAssertAlwaysLog(test2 == Vec4(0.5,2,-2,1),                "Vec4::homogenize() failed.");
    test1.set(1,4,-4,0);
    Vec4::homogenize(test1,&test2);
    CUAssertAlwaysLog(test2 == Vec4(1,4,-4,1),                  "Vec4::homogenize() failed.");
    test1 = Vec4::ONE;
    Vec4::homogenize(test1,&test2);
    CUAssertAlwaysLog(test2 == Vec4::ONE,                       "Vec4::homogenize() failed.");
    
    test1 = Vec4::ZERO;
    testptr = Vec4::midpoint(test1,Vec4::ONE,&test2);
    CUAssertAlwaysLog(testptr == &test2,                        "Vec4::midpoint() failed");
    CUAssertAlwaysLog(test2 == Vec4(0.5,0.5,0.5,0.5),           "Vec4::midpoint() failed.");
    
    Vec4::midpoint(Vec4::UNIT_X,Vec4::UNIT_Y, &test1);
    Vec4::midpoint(Vec4::UNIT_Z,Vec4::UNIT_W, &test2);
    CUAssertAlwaysLog(test1 == Vec4(0.5,0.5,0,0),               "Vec4::midpoint() failed.");
    CUAssertAlwaysLog(test2 == Vec4(0,0,0.5,0.5),               "Vec4::midpoint() failed.");
    
    test1.set(2,3,-1,4);
    testptr = Vec4::project(test1,Vec4::UNIT_X,&test2);
    CUAssertAlwaysLog(testptr == &test2,                        "Vec4::project() failed");
    CUAssertAlwaysLog(test2 == Vec4(2,0,0,0),                   "Vec4::project() failed.");
    Vec4::project(test1, Vec4::UNIT_Y, &test2);
    CUAssertAlwaysLog(test2 == Vec4(0,3,0,0),                   "Vec4::project() failed.");
    Vec4::project(test1, Vec4::UNIT_Z, &test2);
    CUAssertAlwaysLog(test2 == Vec4(0,0,-1,0),                  "Vec4::project() failed.");
    Vec4::project(test1, Vec4::UNIT_Z, &test2);
    CUAssertAlwaysLog(test2 == Vec4(0,0,-1,0),                  "Vec4::project() failed.");
    Vec4::project(6*Vec4::UNIT_Z, Vec4::ONE, &test2);
    CUAssertAlwaysLog(test2 == Vec4(1.5,1.5,1.5,1.5),           "Vec4::project() failed.");
    
    test1 = Vec4::ONE;
    test2.set(2,3,0,-1);
    testptr = Vec4::lerp(test1, test2, 0, &test3);
    CUAssertAlwaysLog(testptr == &test3,                        "Vec4::lerp() failed");
    CUAssertAlwaysLog(test3 == test1,                           "Vec4::lerp() failed.");
    Vec4::lerp(test1, test2, 1, &test3);
    CUAssertAlwaysLog(test3 == test2,                           "Vec4::lerp() failed.");
    Vec4::lerp(test1, test2, 0.5, &test3);
    CUAssertAlwaysLog(test3 == Vec4(1.5,2,0.5,0),               "Vec4::lerp() failed.");
    Vec4::lerp(test1, test2, -1, &test3);
    CUAssertAlwaysLog(test3 == Vec4(0,-1,2,3),                  "Vec4::lerp() failed.");
    Vec4::lerp(test1, test2, 2, &test3);
    CUAssertAlwaysLog(test3 == Vec4(3,5,-1,-3),                 "Vec4::lerp() failed.");
    
#pragma mark Conversion Test
    std::string str;
    std::string a, b, c, d;
    test1.set(2,3,-1.5,0.5); str = test1.toString();
    a = cugl::to_string(2.0f);  b = cugl::to_string(3.0f);
    c = cugl::to_string(-1.5f); d = cugl::to_string(0.5f);
    CUAssertAlwaysLog(str == "("+a+","+b+","+c+","+d+")",           "Method toString() failed");
    str = test1.toString(true);
    CUAssertAlwaysLog(str == "cugl::Vec4("+a+","+b+","+c+","+d+")", "Method toString() failed");
    str = (std::string)test1;
    CUAssertAlwaysLog(str ==  "("+a+","+b+","+c+","+d+")",          "String cast failed");
    
    test1.set(0.25,0.5,0.75,0.125);
    Color4 cbtest = (Color4)test1;
    CUAssertAlwaysLog(cbtest.r == 64 && cbtest.g == 128 && cbtest.b == 191 && cbtest.a == 32,
                      "Color4 cast failed");
    Vec4 test7(cbtest);
    CUAssertAlwaysLog(test7.equals(test1,0.01f),            "Color constructor failed");
    test6 = cbtest;
    CUAssertAlwaysLog(test6.equals(test1,0.01f),            "Color assignment failed");
    
    Color4f cftest = (Color4f)test1;
    CUAssertAlwaysLog(cftest.r == 0.25 && cftest.g == 0.5 && cftest.b == 0.75 && cftest.a == 0.125,
                      "Color4f cast failed");
    Vec4 test8(cftest);
    CUAssertAlwaysLog(test8 == test1,                       "Colorf constructor failed");
    test7 = cftest;
    CUAssertAlwaysLog(test7 == test1,                       "Colorf assignment failed");
    
    test1.set(3,4,-1,2);
    Vec2 v2test = (Vec2)test1;
    CUAssertAlwaysLog(v2test.x == 1.5 && v2test.y == 2,     "Vec2 cast failed");
    Vec4 test9(v2test);
    CUAssertAlwaysLog(test9 != test1,                       "Vec2 constructor failed");
    test3 = test9-test1/2.0;
    CUAssertAlwaysLog(test9-test1/2.0 == 0.5*Vec4::UNIT_Z,  "Vec2 constructor failed");
    test8 = v2test;
    CUAssertAlwaysLog(test8 != test1,                       "Vec2 assignment failed");
    CUAssertAlwaysLog(test8-test1/2.0 == 0.5*Vec4::UNIT_Z,  "Vec2 assignment failed");
    
    test1.set(-2,2,1,0.5);
    Vec3 v3test = (Vec3)test1;
    CUAssertAlwaysLog(v3test.x == -4 && v3test.y == 4 && v3test.z == 2, "Vec3 cast failed");
    Vec4 test10(v3test);
    CUAssertAlwaysLog(test10 == test1.getHomogenized(),     "Vec3 constructor failed");
    test9 = v3test;
    CUAssertAlwaysLog(test9 == test1.getHomogenized(),      "Vec3 assignment failed");
    
    end.mark();
    CULog("Vector test took %llu micros",cugl::Timestamp::ellapsedMicros(start,end));

#pragma mark Complete
    CULog("Vec4 tests complete.\n");

}

#pragma mark -
#pragma mark Color4f

/**
 * Unit test for a 4-float color
 *
 * This is the prefered color for math operations.
 */
void cugl::testColor4f() {
    CULog("Running tests for Color4f.\n");
        
#pragma mark Constructor Test
    // Initial test of constructors
    Color4f test1;
    CUAssertAlwaysLog(test1.r == 0 && test1.g == 0 && test1.b == 0 && test1.a == 0,
                      "Trivial constructor failed");
    
    Color4f test2(0.5f,0.6f,0.25f,0.75f);
    CUAssertAlwaysLog(test2.r == 0.5f && test2.g == 0.6f && test2.b == 0.25f && test2.a == 0.75f,
                      "Initialization constructor failed");
    
    float f[4] = {0.25f, 0.1f, 0.9f, 0.5f};
    Color4f test3(f);
    CUAssertAlwaysLog(test3.r == 0.25f && test3.g == 0.1f && test3.b == 0.9f && test3.a == 0.5f,
                      "Array constructor failed");
    
    Color4f test4(test2);
    CUAssertAlwaysLog(test4.r == 0.5f && test4.g == 0.6f && test4.b == 0.25f && test4.a == 0.75f,
                      "Copy constructor failed");

#if CU_MEMORY_ORDER == CU_ORDER_REVERSED
    Color4f test5(192 << 24 | 64 << 16 | 32 << 8 | 128);
#else
    Color4f test5(128 << 24 | 32 << 16 | 64 << 8 | 192);
#endif
	CUAssertAlwaysLog(CU_MATH_APPROX(test5.r,0.75f,0.005f) && CU_MATH_APPROX(test5.g,0.25f,0.005f) &&
                      CU_MATH_APPROX(test5.b,0.125f,0.005f) && CU_MATH_APPROX(test5.a,0.5f,0.005f),
                      "Packed integer constructor failed");
    
    
#pragma mark Constants Test
    CUAssertAlwaysLog(Color4f::CLEAR.r == 0 && Color4f::CLEAR.g == 0 && Color4f::CLEAR.b == 0 && Color4f::CLEAR.a == 0,
                      "Clear color failed");
    CUAssertAlwaysLog(Color4f::WHITE.r == 1 && Color4f::WHITE.g == 1 && Color4f::WHITE.b == 1 && Color4f::WHITE.a == 1,
                      "White color failed");
    CUAssertAlwaysLog(Color4f::BLACK.r == 0 && Color4f::BLACK.g == 0 && Color4f::BLACK.b == 0 && Color4f::BLACK.a == 1,
                      "Black color failed");
    CUAssertAlwaysLog(Color4f::YELLOW.r == 1 && Color4f::YELLOW.g == 1 && Color4f::YELLOW.b == 0 && Color4f::YELLOW.a == 1,
                      "Yellow color failed");
    CUAssertAlwaysLog(Color4f::BLUE.r == 0 && Color4f::BLUE.g == 0 && Color4f::BLUE.b == 1 && Color4f::BLUE.a == 1,
                      "Blue color failed");
    CUAssertAlwaysLog(Color4f::GREEN.r == 0 && Color4f::GREEN.g == 1 && Color4f::GREEN.b == 0 && Color4f::GREEN.a == 1,
                      "Green color failed");
    CUAssertAlwaysLog(Color4f::RED.r == 1 && Color4f::RED.g == 0 && Color4f::RED.b == 0 && Color4f::RED.a == 1,
                      "Red color failed");
    CUAssertAlwaysLog(Color4f::MAGENTA.r == 1 && Color4f::MAGENTA.g == 0 && Color4f::MAGENTA.b == 1 && Color4f::MAGENTA.a == 1,
                      "Magenta color failed");
    CUAssertAlwaysLog(Color4f::CYAN.r == 0 && Color4f::CYAN.g == 1 && Color4f::CYAN.b == 1 && Color4f::CYAN.a == 1,
                      "Magenta color failed");
    CUAssertAlwaysLog(Color4f::ORANGE.r == 1 && Color4f::ORANGE.g == 0.5 && Color4f::ORANGE.b == 0 && Color4f::ORANGE.a == 1,
                      "Orange color failed");
    CUAssertAlwaysLog(Color4f::GRAY.r == 0.65f && Color4f::GRAY.g == 0.65f && Color4f::GRAY.b == 0.65f && Color4f::GRAY.a == 1,
                      "Gray color failed");
    
    
#pragma mark Setter Test
    test1 = test2;
    CUAssertAlwaysLog(test1.r == 0.5f && test1.g == 0.6f && test1.b == 0.25f && test1.a == 0.75f,
                      "Basic assignment failed");
    
    test1 = f;
    CUAssertAlwaysLog(test1.r == 0.25f && test1.g == 0.1f && test1.b == 0.9f && test1.a == 0.5f,
                      "Float assignment failed");
    
#if CU_MEMORY_ORDER == CU_ORDER_REVERSED
    test1 = (192 << 24 | 64 << 16 | 32 << 8 | 128);
#else
    test1 = (128 << 24 | 32 << 16 | 64 << 8 | 192);
#endif
    CUAssertAlwaysLog(CU_MATH_APPROX(test1.r,0.75f,0.005f)  && CU_MATH_APPROX(test1.g,0.25f,0.005f) &&
                      CU_MATH_APPROX(test1.b,0.125f,0.005f) && CU_MATH_APPROX(test1.a,0.5f,0.005f),
                      "Packed integer assignment failed");

    test1.set(0.2f,0.3f,0.4f,0.5f);
    CUAssertAlwaysLog(test1.r == 0.2f && test1.g == 0.3f && test1.b == 0.4f && test1.a == 0.5f,
                      "Parameter assignment failed");
    
    test1.set(test2);
    CUAssertAlwaysLog(test1.r == 0.5f && test1.g == 0.6f && test1.b == 0.25f && test1.a == 0.75f,
                      "Alternate assignment failed");
    
    test1.set(f);
    CUAssertAlwaysLog(test1.r == 0.25f && test1.g == 0.1f && test1.b == 0.9f && test1.a == 0.5f,
                      "Alternate float assignment failed");
    
#if CU_MEMORY_ORDER == CU_ORDER_REVERSED
    test1.set(192 << 24 | 64 << 16 | 32 << 8 | 128);
#else
    test1.set(128 << 24 | 32 << 16 | 64 << 8 | 192);
#endif
    CUAssertAlwaysLog(CU_MATH_APPROX(test1.r,0.75f,0.005f)  && CU_MATH_APPROX(test1.g,0.25f,0.005f) &&
                      CU_MATH_APPROX(test1.b,0.125f,0.005f) && CU_MATH_APPROX(test1.a,0.5f,0.005f),
                      "Alternate packed integer assignment failed");

    
#pragma mark Comparison Test
    Color4f test6;
    test1.set(0.0f,0.0f,0.0f,0.0f); test2.set(0.0f,0.0f,1.0f,1.0f);
    test3.set(1.0,1.0,0.0f,0.0f);   test4.set(1.0f,1.0f,1.0f,1.0f);
    test5.set(0.0f,0.0f,0.0f,1.0f); test6.set(1.0f,1.0f,1.0f,0.0f);
    
    CUAssertAlwaysLog(test1 < test4,    "Less than failed");
    CUAssertAlwaysLog(!(test4 < test1), "Less than failed");
    CUAssertAlwaysLog(test1 < test2,    "Less than failed");
    CUAssertAlwaysLog(test2 < test3,    "Less than failed");
    CUAssertAlwaysLog(!(test1 < test1), "Less than failed");
    
    CUAssertAlwaysLog(test1 <= test4,   "Less than or equal to failed");
    CUAssertAlwaysLog(!(test4 <= test1),"Less than or equal to failed");
    CUAssertAlwaysLog(test1 <= test2,   "Less than or equal to failed");
    CUAssertAlwaysLog(test2 <= test3,   "Less than or equal to failed");
    CUAssertAlwaysLog(test1 <= test1,   "Less than or equal to failed");
    
    CUAssertAlwaysLog(test4 > test1,    "Greater than failed");
    CUAssertAlwaysLog(!(test1 > test4), "Greater than failed");
    CUAssertAlwaysLog(test2 > test1,    "Greater than failed");
    CUAssertAlwaysLog(test3 > test2,    "Greater than failed");
    CUAssertAlwaysLog(!(test1 > test1), "Greater than failed");
    
    CUAssertAlwaysLog(test4 >= test1,   "Greater than or equal to failed");
    CUAssertAlwaysLog(!(test1 >= test4),"Greater than or equal to failed");
    CUAssertAlwaysLog(test2 >= test1,   "Greater than or equal to failed");
    CUAssertAlwaysLog(test3 >= test2,   "Greater than or equal to failed");
    CUAssertAlwaysLog(test1 >= test1,   "Greater than or equal to failed");
    
    CUAssertAlwaysLog(test1 == test1,   "Equals failed");
    CUAssertAlwaysLog(test2 == test2,   "Equals failed");
    CUAssertAlwaysLog(test3 == test3,   "Equals failed");
    CUAssertAlwaysLog(test4 == test4,   "Equals failed");
    CUAssertAlwaysLog(!(test1 == test4),"Equals failed");
    
    CUAssertAlwaysLog(!(test1 != test1),"Not equals failed");
    CUAssertAlwaysLog(!(test2 != test2),"Not equals failed");
    CUAssertAlwaysLog(!(test3 != test3),"Not equals failed");
    CUAssertAlwaysLog(!(test4 != test4),"Not equals failed");
    CUAssertAlwaysLog(test1 != test4,   "Not equals failed");
    
    CUAssertAlwaysLog(test5.darkerThan(test4), "Method darkerThan() failed");
    CUAssertAlwaysLog(test4.darkerThan(test6), "Method darkerThan() failed");
    CUAssertAlwaysLog(test5.darkerThan(test6), "Method darkerThan() failed");
    CUAssertAlwaysLog(!test1.darkerThan(test4),"Method darkerThan() failed");
    CUAssertAlwaysLog(!test4.darkerThan(test1),"Method darkerThan() failed");
    CUAssertAlwaysLog(!test2.darkerThan(test3),"Method darkerThan() failed");
    CUAssertAlwaysLog(!test3.darkerThan(test2),"Method darkerThan() failed");
    CUAssertAlwaysLog(test1.darkerThan(test1), "Method darkerThan() failed");
    
    CUAssertAlwaysLog(test4.lighterThan(test5), "Method lighterThan() failed");
    CUAssertAlwaysLog(test6.lighterThan(test4), "Method lighterThan() failed");
    CUAssertAlwaysLog(test6.lighterThan(test5), "Method lighterThan() failed");
    CUAssertAlwaysLog(!test4.lighterThan(test1),"Method lighterThan() failed");
    CUAssertAlwaysLog(!test1.lighterThan(test4),"Method lighterThan() failed");
    CUAssertAlwaysLog(!test2.lighterThan(test3),"Method lighterThan() failed");
    CUAssertAlwaysLog(!test3.lighterThan(test2),"Method lighterThan() failed");
    CUAssertAlwaysLog(test1.lighterThan(test1), "Method lighterThan() failed");
    
    test6.set(0,0,CU_MATH_EPSILON*0.5,CU_MATH_EPSILON*0.5);
    CUAssertAlwaysLog(test1.equals(test1), "Approximate equals failed");
    CUAssertAlwaysLog(test1.equals(test6), "Approximate equals failed");
    
#pragma mark Arithmetic Test
    test1.set(0.65f,0.25f,0.75f,0.125f);
    test2.set(0.65f,0.25f,0.75f,0.125f);
    test4.set(0.5f,0.5f,0.5f,0.5f);
    
    test2.clamp(Color4f(0.2f,0.2f,0.1f,0.1f),Color4f(0.8f,0.8f,0.9f,0.9f));
    CUAssertAlwaysLog(test1 == test2,                               "Method clamp() failed");
    
    test2.clamp(test4,Color4f(0.8f,0.8f,0.9f,0.9f));
    CUAssertAlwaysLog(test1 != test2,                               "Method clamp() failed");
    CUAssertAlwaysLog(test2.r == 0.65f && test2.g == 0.5f && test2.b == 0.75f && test2.a == 0.5f,
                      "Method clamp() failed");
    
    test2 = test1;
    test2.clamp(Color4f(0.2f,0.2f,0.1f,0.1f),test4);
    CUAssertAlwaysLog(test1 != test2,                               "Method clamp() failed");
    CUAssertAlwaysLog(test2.r == 0.5f && test2.g == 0.25f && test2.b == 0.5f && test2.a == 0.125f,
                      "Method clamp() failed");
    
    test2 = test1;
    test2.clamp(Color4f(0.4f,0.4f,0.3f,0.3f),Color4f(0.6f,0.6f,0.7f,0.7f));
    CUAssertAlwaysLog(test1 != test2,                               "Method clamp() failed");
    CUAssertAlwaysLog(test2.r == 0.6f && test2.g == 0.4f && test2.b == 0.7f && test2.a ==0.3f,
                      "Method clamp() failed");
    
    test2 = test1;
    test3 = test2.getClamp(test4,Color4f(0.8f,0.8f,0.9f,0.9f));
    CUAssertAlwaysLog(test1 == test2,                               "Method getClamp() failed");
    CUAssertAlwaysLog(test1 != test3,                               "Method getClamp() failed");
    CUAssertAlwaysLog(test3.r == 0.65f && test3.g == 0.5f && test3.b == 0.75f && test3.a == 0.5f,
                      "Method getClamp() failed");
    
    test3 = test2.getClamp(Color4f(0.2f,0.2f,0.1f,0.1f),test4);
    CUAssertAlwaysLog(test1 == test2,                               "Method getClamp() failed");
    CUAssertAlwaysLog(test1 != test3,                               "Method getClamp() failed");
    CUAssertAlwaysLog(test3.r == 0.5f && test3.g == 0.25f && test3.b == 0.5f && test3.a == 0.125f,
                      "Method getClamp() failed");
    
    test3 = test2.getClamp(Color4f(0.4f,0.4f,0.3f,0.3f),Color4f(0.6f,0.6f,0.7f,0.7f));
    CUAssertAlwaysLog(test1 == test2,                               "Method getClamp() failed");
    CUAssertAlwaysLog(test1 != test3,                               "Method getClamp() failed");
    CUAssertAlwaysLog(test3.r == 0.6f && test3.g == 0.4f && test3.b == 0.7f && test3.a ==0.3f,
                      "Method getClamp() failed");
    
    test1 = Color4f::RED;
    test1.add(Color4f::GREEN);
    test1.add(Color4f::BLUE);
    CUAssertAlwaysLog(test1 == Color4f::WHITE,                      "Method add() failed");
    
    test1 = Color4f::WHITE;
    test1.add(test1);
    CUAssertAlwaysLog(test1 == Color4f::WHITE,                      "Method add() failed");

    test1 = Color4f(0.5f,0.5f,0.5f,0.5f);
    test1.add(test1);
    CUAssertAlwaysLog(test1 == Color4f(1.0f,1.0f,1.0f,0.5f),        "Method add() failed");
    
    test1 = Color4f(0.5f,0.5f,0.5f,0.5f);
    test1.add(test1,true);
    CUAssertAlwaysLog(test1 == Color4f::WHITE,                      "Method add() failed");

    test1.set(0.5f,0.5f,0.5f,0.5f);
    test1.add(0.4f,0.125f,0.75f,0.25f);
    CUAssertAlwaysLog(test1 == Color4f(0.9f,0.625f,1.0f,0.75f),     "Method add() failed");

    test1.set(0.5f,0.5f,0.5f,0.5f);
    test1.add(0.4f,0.125f,0.75f);
    CUAssertAlwaysLog(test1 == Color4f(0.9f,0.625f,1.0f,0.5f),      "Method add() failed");

    test1 = Color4f::WHITE;
    test1.subtract(Color4f::RED);
    CUAssertAlwaysLog(test1 == Color4f::CYAN,                       "Method subtract() failed");

    test1 = Color4f::WHITE;
    test1.subtract(Color4f::RED, true);
    CUAssertAlwaysLog(test1 == Color4f(0.0f,1.0f,1.0f,0.0f),        "Method subtract() failed");

    test1 = Color4f::WHITE;
    test1.subtract(test1);
    CUAssertAlwaysLog(test1 == Color4f::BLACK,                      "Method subtract() failed");

    test1 = Color4f::WHITE;
    test1.subtract(test1,true);
    CUAssertAlwaysLog(test1 == Color4f::CLEAR,                      "Method subtract() failed");

    test1 = Color4f::WHITE;
    test1.subtract(0.4f,0.125f,0.75f,0.25f);
    CUAssertAlwaysLog(test1 == Color4f(0.6f,0.875f,0.25f,0.75f),    "Method subtract() failed");

    test1 = Color4f::WHITE;
    test1.subtract(0.4f,0.125f,0.75f);
    CUAssertAlwaysLog(test1 == Color4f(0.6f,0.875f,0.25f,1.0f),    "Method subtract() failed");

    test1 = Color4f::WHITE;
    test2 = Color4f::RED;
    test3 = Color4f::GREEN;
    test4 = Color4f::BLUE;
    test5 = Color4f::BLACK;
    test1.scale(0.5f);
    CUAssertAlwaysLog(test1 == Color4f(0.5f,0.5f,0.5f,1.0f),        "Method scale() failed");

    test1 = Color4f::WHITE;
    test1.scale(0.5f,true); test2.scale(0.5f,true); test3.scale(0.5f,true);
    test4.scale(0.5f,true); test5.scale(0.5f,true);
    CUAssertAlwaysLog(test1 == Color4f(0.5f,0.5f,0.5f,0.5f),        "Method scale() failed");
    CUAssertAlwaysLog(test2 == Color4f(0.5f,0,0,0.5f),              "Method scale() failed");
    CUAssertAlwaysLog(test3 == Color4f(0,0.5f,0,0.5f),              "Method scale() failed");
    CUAssertAlwaysLog(test4 == Color4f(0,0,0.5f,0.5f),              "Method scale() failed");
    CUAssertAlwaysLog(test5 == Color4f(0,0,0,0.5f),                 "Method scale() failed");
    
    test1 = Color4f::WHITE;
    test2 = Color4f::RED;
    test3 = Color4f::GREEN;
    test4 = Color4f::BLUE;
    test5 = Color4f::BLACK;
    test1.scale(0.5f,0.6f,0.4f);
    CUAssertAlwaysLog(test1 == Color4f(0.5f,0.6f,0.4f,1.0f),        "Method scale() failed");
    
    test1 = Color4f::WHITE;
    test1.scale(0.5f,0.6f,0.4f,0.8f); test2.scale(0.5f,0.6f,0.4f,0.8f); test3.scale(0.5f,0.6f,0.4f,0.8f);
    test4.scale(0.5f,0.6f,0.4f,0.8f); test5.scale(0.5f,0.6f,0.4f,0.8f);
    CUAssertAlwaysLog(test1 == Color4f(0.5f,0.6f,0.4f,0.8f),        "Method scale() failed");
    CUAssertAlwaysLog(test2 == Color4f(0.5f,0,0,0.8f),              "Method scale() failed");
    CUAssertAlwaysLog(test3 == Color4f(0,0.6f,0,0.8f),              "Method scale() failed");
    CUAssertAlwaysLog(test4 == Color4f(0,0,0.4f,0.8f),              "Method scale() failed");
    CUAssertAlwaysLog(test5 == Color4f(0,0,0,0.8f),                 "Method scale() failed");
    
    test1 = Color4f::WHITE;
    test2 = Color4f::RED;
    test3 = Color4f::GREEN;
    test4 = Color4f::BLUE;
    test5 = Color4f::BLACK;
    test6.set(0.3f,0.2f,0.8f,0.5f);
    test1.scale(test6);
    CUAssertAlwaysLog(test1 == Color4f(0.3f,0.2f,0.8f,1.0f),        "Method scale() failed");

    test1 = Color4f::WHITE;
    test1.scale(test6,true); test2.scale(test6,true); test3.scale(test6,true);
    test4.scale(test6,true); test5.scale(test6,true);
    CUAssertAlwaysLog(test1 == Color4f(0.3f,0.2f,0.8f,0.5f),        "Method scale() failed");
    CUAssertAlwaysLog(test2 == Color4f(0.3f,0,0,0.5f),              "Method scale() failed");
    CUAssertAlwaysLog(test3 == Color4f(0,0.2f,0,0.5f),              "Method scale() failed");
    CUAssertAlwaysLog(test4 == Color4f(0,0,0.8f,0.5f),              "Method scale() failed");
    CUAssertAlwaysLog(test5 == Color4f(0,0,0,0.5f),                 "Method scale() failed");
    
    // Lambda function
    std::function<float(float)> functor = [](float x) { return 1-x; };
    
    test1 = Color4f::WHITE;
    test2 = Color4f::RED;
    test3 = Color4f::GREEN;
    test4 = Color4f::BLUE;
    test5 = Color4f::BLACK;
    test6 = Color4f::CLEAR;
    test1.map(functor);
    CUAssertAlwaysLog(test1.r == 0.0f && test1.g == 0.0f && test1.b == 0.0f && test1.a == 1.0f,
                      "Method map() failed");
    test1 = Color4f::WHITE;
    test1.map(functor,true); test2.map(functor,true); test3.map(functor,true);
    test4.map(functor,true); test5.map(functor,true); test6.map(functor,true);
    CUAssertAlwaysLog(test1.r == 0.0f && test1.g == 0.0f && test1.b == 0.0f && test1.a == 0.0f,
                      "Method map() failed");
    CUAssertAlwaysLog(test2.r == 0.0f && test2.g == 1.0f && test2.b == 1.0f && test2.a == 0.0f,
                      "Method map() failed");
    CUAssertAlwaysLog(test3.r == 1.0f && test3.g == 0.0f && test3.b == 1.0f && test3.a == 0.0f,
                      "Method map() failed");
    CUAssertAlwaysLog(test4.r == 1.0f && test4.g == 1.0f && test4.b == 0.0f && test4.a == 0.0f,
                      "Method map() failed");
    CUAssertAlwaysLog(test5.r == 1.0f && test5.g == 1.0f && test5.b == 1.0f && test5.a == 0.0f,
                      "Method map() failed");
    CUAssertAlwaysLog(test6.r == 1.0f && test6.g == 1.0f && test6.b == 1.0f && test6.a == 1.0f,
                      "Method map() failed");
    
    test1 = Color4f::WHITE;
    test2 = Color4f::RED;
    test3 = Color4f::GREEN;
    test4 = Color4f::BLUE;
    test5 = Color4f::BLACK;
    test6 = Color4f::CLEAR;
    Color4f test7 = test1.getMap(functor);
    CUAssertAlwaysLog(test1 != test7,   "Method getMap() failed");
    CUAssertAlwaysLog(test7.r == 0.0f && test7.g == 0.0f && test7.b == 0.0f && test7.a == 1.0f,
                      "Method getMap() failed");
    test7 = test1.getMap(functor,true);
    CUAssertAlwaysLog(test7.r == 0.0f && test7.g == 0.0f && test7.b == 0.0f && test7.a == 0.0f,
                      "Method getMap() failed");
    test7 = test2.getMap(functor,true);
    CUAssertAlwaysLog(test7.r == 0.0f && test7.g == 1.0f && test7.b == 1.0f && test7.a == 0.0f,
                      "Method getMap() failed");
    test7 = test3.getMap(functor,true);
    CUAssertAlwaysLog(test7.r == 1.0f && test7.g == 0.0f && test7.b == 1.0f && test7.a == 0.0f,
                      "Method getMap() failed");
    test7 = test4.getMap(functor,true);
    CUAssertAlwaysLog(test7.r == 1.0f && test7.g == 1.0f && test7.b == 0.0f && test7.a == 0.0f,
                      "Method getMap() failed");
    test7 = test5.getMap(functor,true);
    CUAssertAlwaysLog(test7.r == 1.0f && test7.g == 1.0f && test7.b == 1.0f && test7.a == 0.0f,
                      "Method getMap() failed");
    test7 = test6.getMap(functor,true);
    CUAssertAlwaysLog(test7.r == 1.0f && test7.g == 1.0f && test7.b == 1.0f && test7.a == 1.0f,
                      "Method getMap() failed");

#pragma mark Operator Test
    test1 = Color4f::RED;
    test1 += Color4f::GREEN;
    test1 += Color4f::BLUE;
    CUAssertAlwaysLog(test1 == Color4f::WHITE,                      "ddition operation failed");
    
    test1 = Color4f::WHITE;
    test1 += test1;
    CUAssertAlwaysLog(test1 == Color4f::WHITE,                      "Addition operation failed");
    
    test1 = Color4f(0.5f,0.5f,0.5f,0.5f);
    test1 += test1;
    CUAssertAlwaysLog(test1 == Color4f::WHITE,                      "Addition operation failed");
    
    test1 = Color4f(0.5f,0.5f,0.5f,0.5f);
    CUAssertAlwaysLog(Color4f::GREEN+Color4f::BLUE==Color4f::CYAN,  "Addition operation failed");
    CUAssertAlwaysLog(test1+test1 == Color4f::WHITE,                "Addition operation failed");
    
    test1 =  Color4f::MAGENTA;
    test1 -= Color4f::BLUE;
    CUAssertAlwaysLog(test1 == Color4f(1,0,0,0),                    "Subtraction operation failed");
    
    test1 = Color4f::WHITE;
    test1 -= test1;
    CUAssertAlwaysLog(test1 == Color4f::CLEAR,                      "Subtraction operation failed");
    CUAssertAlwaysLog(Color4f::MAGENTA-Color4f::BLUE == Color4f(1,0,0,0),   "Subtraction operation failed");
    CUAssertAlwaysLog(Color4f::WHITE-Color4f::WHITE == Color4f::CLEAR,      "Subtraction operation failed");
    
    test1 = Color4f::WHITE;
    test2 = Color4f::RED;
    test3 = Color4f::GREEN;
    test4 = Color4f::BLUE;
    test5 = Color4f::BLACK;
    test1 *= 0.5f; test2 *= 0.5f; test3 *= 0.5f; test4 *= 0.5f; test5 *= 0.5f;
    CUAssertAlwaysLog(test1 == Color4f(0.5f,0.5f,0.5f,0.5f),        "Scaling operation failed");
    CUAssertAlwaysLog(test2 == Color4f(0.5f,0,0,0.5f),              "Scaling operation failed");
    CUAssertAlwaysLog(test3 == Color4f(0,0.5f,0,0.5f),              "Scaling operation failed");
    CUAssertAlwaysLog(test4 == Color4f(0,0,0.5f,0.5f),              "Scaling operation failed");
    CUAssertAlwaysLog(test5 == Color4f(0,0,0,0.5f),                 "Scaling operation failed");
    CUAssertAlwaysLog(Color4f::WHITE*0.5 == Color4f(0.5f,0.5f,0.5f,0.5f),   "Scaling operation failed");
    CUAssertAlwaysLog(Color4f::RED*0.5   == Color4f(0.5f,0.0f,0.0f,0.5f),   "Scaling operation failed");
    CUAssertAlwaysLog(Color4f::GREEN*0.5 == Color4f(0.0f,0.5f,0.0f,0.5f),   "Scaling operation failed");
    CUAssertAlwaysLog(Color4f::BLUE*0.5  == Color4f(0.0f,0.0f,0.5f,0.5f),   "Scaling operation failed");
    CUAssertAlwaysLog(Color4f::BLACK*0.5 == Color4f(0.0f,0.0f,0.0f,0.5f),   "Scaling operation failed");
    CUAssertAlwaysLog(0.5*Color4f::WHITE == Color4f(0.5f,0.5f,0.5f,0.5f),   "Scaling operation failed");
    CUAssertAlwaysLog(0.5*Color4f::RED   == Color4f(0.5f,0.0f,0.0f,0.5f),   "Scaling operation failed");
    CUAssertAlwaysLog(0.5*Color4f::GREEN == Color4f(0.0f,0.5f,0.0f,0.5f),   "Scaling operation failed");
    CUAssertAlwaysLog(0.5*Color4f::BLUE  == Color4f(0.0f,0.0f,0.5f,0.5f),   "Scaling operation failed");
    CUAssertAlwaysLog(0.5*Color4f::BLACK == Color4f(0.0f,0.0f,0.0f,0.5f),   "Scaling operation failed");

    test1 = Color4f::WHITE;
    test2 = Color4f::RED;
    test3 = Color4f::GREEN;
    test4 = Color4f::BLUE;
    test5 = Color4f::BLACK;
    test6.set(0.3f,0.2f,0.8f,0.5f);
    test1 *= test6; test2 *= test6; test3 *= test6; test4 *= test6; test5 *= test6;
    CUAssertAlwaysLog(test1 == Color4f(0.3f,0.2f,0.8f,0.5f),        "Scaling operation failed");
    CUAssertAlwaysLog(test2 == Color4f(0.3f,0,0,0.5f),              "Scaling operation failed");
    CUAssertAlwaysLog(test3 == Color4f(0,0.2f,0,0.5f),              "Scaling operation failed");
    CUAssertAlwaysLog(test4 == Color4f(0,0,0.8f,0.5f),              "Scaling operation failed");
    CUAssertAlwaysLog(test5 == Color4f(0,0,0,0.5f),                 "Scaling operation failed");
    CUAssertAlwaysLog(Color4f::WHITE*test6 == Color4f(0.3f,0.2f,0.8f,0.5f), "Scaling operation failed");
    CUAssertAlwaysLog(Color4f::RED*test6   == Color4f(0.3f,0,0,0.5f),       "Scaling operation failed");
    CUAssertAlwaysLog(Color4f::GREEN*test6 == Color4f(0,0.2f,0,0.5f),       "Scaling operation failed");
    CUAssertAlwaysLog(Color4f::BLUE*test6  == Color4f(0,0,0.8f,0.5f),       "Scaling operation failed");
    CUAssertAlwaysLog(Color4f::BLACK*test6 == Color4f(0,0,0,0.5f),          "Scaling operation failed");
    CUAssertAlwaysLog(test6*Color4f::WHITE == Color4f(0.3f,0.2f,0.8f,0.5f), "Scaling operation failed");
    CUAssertAlwaysLog(test6*Color4f::RED   == Color4f(0.3f,0,0,0.5f),       "Scaling operation failed");
    CUAssertAlwaysLog(test6*Color4f::GREEN == Color4f(0,0.2f,0,0.5f),       "Scaling operation failed");
    CUAssertAlwaysLog(test6*Color4f::BLUE  == Color4f(0,0,0.8f,0.5f),       "Scaling operation failed");
    CUAssertAlwaysLog(test6*Color4f::BLACK == Color4f(0,0,0,0.5f),          "Scaling operation failed");
    
#pragma mark Color Operations Test
    test1 = Color4f::WHITE;
    test1.complement();
    CUAssertAlwaysLog(test1 == Color4f::BLACK,                  "Method complement() failed");

    test1 = Color4f::RED;
    test1.complement();
    CUAssertAlwaysLog(test1 == Color4f::CYAN,                   "Method complement() failed");

    test1 = Color4f::GRAY;
    test1.complement();
    CUAssertAlwaysLog(test1.equals(Color4f(0.35f,0.35f,0.35f,1.0f)),    "Method complement() failed");

    test1 = Color4f::WHITE;
    test1.complement(true);
    CUAssertAlwaysLog(test1 == Color4f::CLEAR,                  "Method complement() failed");

    test1 = Color4f::WHITE;
    test2 = test1.getComplement();
    CUAssertAlwaysLog(test1 != test2,                           "Method getComplement() failed");
    CUAssertAlwaysLog(test2 == Color4f::BLACK,                  "Method getComplement() failed");
    
    test2 = Color4f::RED.getComplement();
    CUAssertAlwaysLog(test2 == Color4f::CYAN,                   "Method getComplement() failed");

    test2 = Color4f::GRAY.getComplement();
    CUAssertAlwaysLog(test2.equals(Color4f(0.35f,0.35f,0.35f,1.0f)),    "Method getComplement() failed");
    
    test2 = Color4f::WHITE.getComplement(true);
    CUAssertAlwaysLog(test2 == Color4f::CLEAR,                  "Method getComplement() failed");

    test1 = Color4f::WHITE;
    test1.premultiply();
    CUAssertAlwaysLog(test1 == Color4f::WHITE,                  "Method premultiply() failed");

    test1 = Color4f::CLEAR;
    test1.premultiply();
    CUAssertAlwaysLog(test1 == Color4f::CLEAR,                  "Method premultiply() failed");

    test1.set(0.4f,0.5f,0.6f,0.5f);
    test1.premultiply();
    CUAssertAlwaysLog(test1 == Color4f(0.2f,0.25f,0.3f,0.5f),   "Method premultiply() failed");

    test1 = Color4f::WHITE;
    test2 = test1.getPremultiplied();
    CUAssertAlwaysLog(test2 == Color4f::WHITE,                  "Method getPremultiplied() failed");
    
    test1 = Color4f::CLEAR;
    test2 = test1.getPremultiplied();
    CUAssertAlwaysLog(test2 == Color4f::CLEAR,                  "Method getPremultiplied() failed");
    
    test1.set(0.4f,0.5f,0.6f,0.5f);
    test2 = test1.getPremultiplied();
    CUAssertAlwaysLog(test1 != test2,                           "Method getPremultiplied() failed");
    CUAssertAlwaysLog(test2 == Color4f(0.2f,0.25f,0.3f,0.5f),   "Method getPremultiplied() failed");

    test1 = Color4f::WHITE;
    test1.unpremultiply();
    CUAssertAlwaysLog(test1 == Color4f::WHITE,                  "Method unpremultiply() failed");
    
    test1 = Color4f::CLEAR;
    test1.unpremultiply();
    CUAssertAlwaysLog(test1 == Color4f::CLEAR,                  "Method unpremultiply() failed");
    
    test1.set(0.2f,0.25f,0.3f,0.5f);
    test1.unpremultiply();
    CUAssertAlwaysLog(test1 == Color4f(0.4f,0.5f,0.6f,0.5f),    "Method unpremultiply() failed");
    
    test1 = Color4f::WHITE;
    test2 = test1.getUnpremultiplied();
    CUAssertAlwaysLog(test2 == Color4f::WHITE,                  "Method getUnpremultiplied() failed");
    
    test1 = Color4f::CLEAR;
    test2 = test1.getUnpremultiplied();
    CUAssertAlwaysLog(test2 == Color4f::CLEAR,                  "Method getUnpremultiplied() failed");
    
    test1.set(0.2f,0.25f,0.3f,0.5f);
    test2 = test1.getUnpremultiplied();
    CUAssertAlwaysLog(test1 != test2,                           "Method getPremultiplied() failed");
    CUAssertAlwaysLog(test2 == Color4f(0.4f,0.5f,0.6f,0.5f),    "Method getUnpremultiplied() failed");
    
    test1 = Color4f::WHITE;
    test2.set(0.4f,0.5f,0.0f,0.7f);
    test1.lerp(test2,0);
    CUAssertAlwaysLog(test1 == Color4f::WHITE,                  "Method lerp() failed.");
    test1.lerp(test2,1);
    CUAssertAlwaysLog(test1 == test2,                           "Method lerp() failed.");
    test1 = Color4f::WHITE;
    test1.lerp(test2,0.5);
    CUAssertAlwaysLog(test1 == Color4f(0.7f,0.75f,0.5f,0.85f),  "Method lerp() failed.");
    test1 =Color4f::WHITE;
    test1.lerp(test2,-1);
    CUAssertAlwaysLog(test1 == Color4f::WHITE,                  "Method lerp() failed.");
    test1 = Color4f::WHITE;
    test1.lerp(test2,2);
    CUAssertAlwaysLog(test1 == test2,                           "Method lerp() failed.");
    
    test1 = Color4f::WHITE;
    test2.set(0.4f,0.5f,0.0f,0.7f);
    test3 = test1.getLerp(test2,0);
    CUAssertAlwaysLog(test3 == Color4f::WHITE,                  "Method getLerp() failed.");
    test3 = test1.getLerp(test2,1);
    CUAssertAlwaysLog(test1 != test3,                           "Method getLerp() failed.");
    CUAssertAlwaysLog(test3 == test2,                           "Method getLerp() failed.");
    test3 = test1.getLerp(test2,0.5);
    CUAssertAlwaysLog(test3 == Color4f(0.7f,0.75f,0.5f,0.85f),  "Method getLerp() failed.");
    test3 = test1.getLerp(test2,-1);
    CUAssertAlwaysLog(test3 == Color4f::WHITE,                  "Method getLerp() failed.");
    test3 = test1.getLerp(test2,2);
    CUAssertAlwaysLog(test3 == test2,                           "Method getLerp() failed.");
    
    test1 = Color4f::WHITE;
    test2.set(0.4f,0.7f,0.0f,0.5f);
    test1.blend(test2);
    CUAssertAlwaysLog(test1==Color4f(0.7f,0.85f,0.5f,1.0f),     "Method blend() failed.");
    
    test1 = Color4f::WHITE;
    test1.a = 0.6f;
    test1.blend(test2);
    test4.set(0.625f,0.8125f,0.375f,0.8f);
    CUAssertAlwaysLog(test1.equals(test4),                      "Method blend() failed.");

    test1 = Color4f::WHITE;
    test3 = test1.getBlend(test2);
    CUAssertAlwaysLog(test1 != test3,                           "Method getBlend() failed.");
    CUAssertAlwaysLog(test3 != test2,                           "Method getBlend() failed.");
    CUAssertAlwaysLog(test3==Color4f(0.7f,0.85f,0.5f,1.0f),     "Method getBlend() failed.");
    
    test1.a = 0.6f;
    test3 = test1.getBlend(test2);
    CUAssertAlwaysLog(test3.equals(test4),                      "Method getBlend() failed.");

    test1 = Color4f::WHITE;
    test2.set(0.4f,0.7f,0.0f,0.5f);
    test2.premultiply();
    test1.blendPre(test2);
    CUAssertAlwaysLog(test1==Color4f(0.7f,0.85f,0.5f,1.0f),     "Method blendPre() failed.");
    
    test1 = Color4f::WHITE;
    test1.a = 0.6f;
    test1.premultiply();
    test1.blendPre(test2);
    test1.unpremultiply();
    CUAssertAlwaysLog(test1.equals(test4),                      "Method blendPre() failed.");
    
    test1 = Color4f::WHITE;
    test3 = test1.getBlendPre(test2);
    CUAssertAlwaysLog(test1 != test3,                           "Method getBlendPre() failed.");
    CUAssertAlwaysLog(test1 != test2,                           "Method getBlendPre() failed.");
    CUAssertAlwaysLog(test3==Color4f(0.7f,0.85f,0.5f,1.0f),     "Method getBlendPre() failed.");
    
    test1.a = 0.6f;
    test1.premultiply();
    test3 = test1.getBlendPre(test2);
    test3.unpremultiply();
    CUAssertAlwaysLog(test3.equals(test4),                      "Method getBlendPre() failed.");

    CUAssertAlwaysLog(Color4f::WHITE.getRGBA()  == 0xffffffff,  "Method getRGB() failed.");
    CUAssertAlwaysLog(Color4f::RED.getRGBA()    == 0xff0000ff,  "Method getRGB() failed.");
    CUAssertAlwaysLog(Color4f::RED.getRGBA()    == 0xff0000ff,  "Method getRGB() failed.");
    CUAssertAlwaysLog(Color4f::GREEN.getRGBA()  == 0x00ff00ff,  "Method getRGB() failed.");
    CUAssertAlwaysLog(Color4f::BLUE.getRGBA()   == 0x0000ffff,  "Method getRGB() failed.");
    CUAssertAlwaysLog(Color4f::BLACK.getRGBA()  == 0x000000ff,  "Method getRGB() failed.");
    CUAssertAlwaysLog(Color4f::ORANGE.getRGBA() == 0xff8000ff,  "Method getRGB() failed.");
    CUAssertAlwaysLog(Color4f::CLEAR.getRGBA()  == 0x0,         "Method getRGB() failed.");
    
#pragma mark Static Color Operations Test
    Color4f* testptr;
    
    test1 = Color4f::WHITE;
    test2.set(0.4f,0.5f,0.0f,0.7f);
    testptr = Color4f::lerp(test1, test2, 0, &test3);
    CUAssertAlwaysLog(testptr == &test3,                        "Color4f::getLerp() failed");
    CUAssertAlwaysLog(test3 == test1,                           "Color4f::getLerp() failed.");
    Color4f::lerp(test1, test2, 1, &test3);
    CUAssertAlwaysLog(test3 == test2,                           "Color4f::getLerp() failed.");
    Color4f::lerp(test1, test2, 0.5, &test3);
    CUAssertAlwaysLog(test3 == Vec4(0.7f,0.75f,0.5f,0.85f),     "Color4f::getLerp() failed.");
    Color4f::lerp(test1, test2, -1, &test3);
    CUAssertAlwaysLog(test3 == test1,                           "Color4f::getLerp() failed.");
    Color4f::lerp(test1, test2, 2, &test3);
    CUAssertAlwaysLog(test3 == test2,                           "Color4f::getLerp() failed.");

    test1 = Color4f::WHITE;
    test2.set(0.4f,0.7f,0.0f,0.5f);
    testptr = Color4f::blend(test2, test1, &test3);
    CUAssertAlwaysLog(testptr == &test3,                        "Color4f::getBlend() failed");
    CUAssertAlwaysLog(test1 != test3,                           "Color4f::getBlend() failed.");
    CUAssertAlwaysLog(test2 != test3,                           "Color4f::getBlend() failed.");
    CUAssertAlwaysLog(test3==Color4f(0.7f,0.85f,0.5f,1.0f),     "Color4f::getBlend() failed.");
    
    test1.a = 0.6f;
    Color4f::blend(test2, test1, &test3);
    CUAssertAlwaysLog(test3.equals(test4),                      "Color4f::getBlend() failed.");

    test1 = Color4f::WHITE;
    test2.premultiply();
    testptr = Color4f::blendPre(test2, test1, &test3);
    test3 = test1.getBlendPre(test2);
    CUAssertAlwaysLog(testptr == &test3,                        "Color4f::getBlendPre() failed");
    CUAssertAlwaysLog(test1 != test3,                           "Color4f::getBlendPre() failed.");
    CUAssertAlwaysLog(test2 != test3,                           "Color4f::getBlendPre() failed.");
    CUAssertAlwaysLog(test3==Color4f(0.7f,0.85f,0.5f,1.0f),     "Color4f::getBlendPre() failed.");
    
    test1.a = 0.6f;
    test1.premultiply();
    testptr = Color4f::blendPre(test2, test1, &test3);
    test3.unpremultiply();
    CUAssertAlwaysLog(test3.equals(test4),                      "Color4f::getBlendPre() failed.");

    
#pragma mark Conversion Test
    std::string str;
    std::string a, b, c, d;
    test1.set(0.25f,0.5f,1.0f,0.75f); str = test1.toString();
    a = cugl::to_string(0.25f);  b = cugl::to_string(0.5f);
    c = cugl::to_string(1.0f); d = cugl::to_string(0.75f);
    CUAssertAlwaysLog(str == "[r="+a+",g="+b+",b="+c+",a="+d+"]",               "Method toString() failed");
    str = test1.toString(true);
    CUAssertAlwaysLog(str == "cugl::Color4f[r="+a+",g="+b+",b="+c+",a="+d+"]",  "Method toString() failed");
    str = (std::string)test1;
    CUAssertAlwaysLog(str ==  "[r="+a+",g="+b+",b="+c+",a="+d+"]",              "String cast failed");
    
    Color4 cbtest = (Color4)test1;
    CUAssertAlwaysLog(cbtest.r == 64 && cbtest.g == 128 && cbtest.b == 255 && cbtest.a == 191,
                      "Color4 cast failed");
    Color4f test8(cbtest);
    CUAssertAlwaysLog(test8.equals(test1,0.01f),                "Color constructor failed");
    test7 = cbtest;
    CUAssertAlwaysLog(test7.equals(test1,0.01f),                "Color assignment failed");
    
    Vec3 v3test = (Vec3)test1;
    CUAssertAlwaysLog(v3test.x == 0.25f && v3test.y == 0.5f && v3test.z == 1.0f,    "Vec3 cast failed");
    Color4f test9(v3test);
    CUAssertAlwaysLog(test9 == Color4f(0.25f,0.5f,1.0f,1.0f),   "Vec3 constructor failed");
    test8 = v3test;
    CUAssertAlwaysLog(test8 == test9,                           "Vec3 assignment failed");
    
    Vec4 v4test = (Vec4)test1;
    CUAssertAlwaysLog(v4test.x == 0.25f && v4test.y == 0.5f && v4test.z == 1.0f && v4test.w == 0.75f,
                      "Vec4 cast failed");
    Color4f test10(v4test);
    CUAssertAlwaysLog(test10 == Color4f(0.25f,0.5f,1.0f,0.75f), "Vec4 constructor failed");
    test9 = v4test;
    CUAssertAlwaysLog(test9 == test10,                          "Vec4 assignment failed");

    
#pragma mark Complete
    CULog("Color4f tests complete.\n");
}


#pragma mark -
#pragma mark Color4

/**
 * Unit test for a 4-byte color
 *
 * This is the preferred color for storage and shaders.
 */
void cugl::testColor4() {
    CULog("Running tests for Color4.\n");

#pragma mark Constructor Test
    // Initial test of constructors
    Color4 test1;
    CUAssertAlwaysLog(test1.r == 0 && test1.g == 0 && test1.b == 0 && test1.a == 0,
                      "Trivial constructor failed");
    
    Color4 test2(128,64,32,192);
    CUAssertAlwaysLog(test2.r == 128 && test2.g == 64 && test2.b == 32 && test2.a == 192,
                      "Initialization constructor failed");
    
    float f[4] = {0.25f, 0.125f, 0.75f, 0.5f};
    Color4 test3(f);
    CUAssertAlwaysLog(test3.r == 64 && test3.g == 32 && test3.b == 191 && test3.a == 128,
                      "Array constructor failed");
    
    Color4 test4(test2);
    CUAssertAlwaysLog(test4.r == 128 && test4.g == 64 && test4.b == 32 && test4.a == 192,
                      "Copy constructor failed");
    
#if CU_MEMORY_ORDER == CU_ORDER_REVERSED
    Color4 test5(192 << 24 | 64 << 16 | 32 << 8 | 128);
#else
    Color4 test5(128 << 24 | 32 << 16 | 64 << 8 | 192);
#endif
    CUAssertAlwaysLog(test5.r == 192 && test5.g == 64 && test5.b == 32 && test5.a == 128,
                      "Packed integer constructor failed");
    
#pragma mark Constants Test
    CUAssertAlwaysLog(Color4::CLEAR.r == 0 && Color4::CLEAR.g == 0 && Color4::CLEAR.b == 0 && Color4::CLEAR.a == 0,
                      "Clear color failed");
    CUAssertAlwaysLog(Color4::WHITE.r == 255 && Color4::WHITE.g == 255 && Color4::WHITE.b == 255 && Color4::WHITE.a == 255,
                      "White color failed");
    CUAssertAlwaysLog(Color4::BLACK.r == 0 && Color4::BLACK.g == 0 && Color4::BLACK.b == 0 && Color4::BLACK.a == 255,
                      "Black color failed");
    CUAssertAlwaysLog(Color4::YELLOW.r == 255 && Color4::YELLOW.g == 255 && Color4::YELLOW.b == 0 && Color4::YELLOW.a == 255,
                      "Yellow color failed");
    CUAssertAlwaysLog(Color4::BLUE.r == 0 && Color4::BLUE.g == 0 && Color4::BLUE.b == 255 && Color4::BLUE.a == 255,
                      "Blue color failed");
    CUAssertAlwaysLog(Color4::GREEN.r == 0 && Color4::GREEN.g == 255 && Color4::GREEN.b == 0 && Color4::GREEN.a == 255,
                      "Green color failed");
    CUAssertAlwaysLog(Color4::RED.r == 255 && Color4::RED.g == 0 && Color4::RED.b == 0 && Color4::RED.a == 255,
                      "Red color failed");
    CUAssertAlwaysLog(Color4::MAGENTA.r == 255 && Color4::MAGENTA.g == 0 && Color4::MAGENTA.b == 255 && Color4::MAGENTA.a == 255,
                      "Magenta color failed");
    CUAssertAlwaysLog(Color4::CYAN.r == 0 && Color4::CYAN.g == 255 && Color4::CYAN.b == 255 && Color4::CYAN.a == 255,
                      "Magenta color failed");
    CUAssertAlwaysLog(Color4::ORANGE.r == 255 && Color4::ORANGE.g == 128 && Color4::ORANGE.b == 0 && Color4::ORANGE.a == 255,
                      "Orange color failed");
    CUAssertAlwaysLog(Color4::GRAY.r == 166 && Color4::GRAY.g == 166 && Color4::GRAY.b == 166 && Color4::GRAY.a == 255,
                      "Gray color failed");
    
#pragma mark Setter Test
    test1 = test2;
    CUAssertAlwaysLog(test1.r == 128 && test1.g == 64 && test1.b == 32 && test1.a == 192,
                      "Basic assignment failed");
    
    test1 = f;
    CUAssertAlwaysLog(test1.r == 64 && test1.g == 32 && test1.b == 191 && test1.a == 128,
                      "Float assignment failed");
    
    test1 = (192 << 24 | 64 << 16 | 32 << 8 | 180);
    CUAssertAlwaysLog(test1.r == 192 && test1.g == 64 && test1.b == 32 && test1.a == 180,
                      "Packed integer assignment failed");
    
    test1.set(5,200,16,190);
    CUAssertAlwaysLog(test1.r == 5 && test1.g == 200 && test1.b == 16 && test1.a == 190,
                      "Parameter assignment failed");
    
    test1.set(test2);
    CUAssertAlwaysLog(test1.r == 128 && test1.g == 64 && test1.b == 32 && test1.a == 192,
                      "Alternate assignment failed");
    
    test1.set(f);
    CUAssertAlwaysLog(test1.r == 64 && test1.g == 32 && test1.b == 191 && test1.a == 128,
                      "Alternate float assignment failed");
    
    test1.set(192 << 24 | 64 << 16 | 32 << 8 | 180);
    CUAssertAlwaysLog(test1.r == 192 && test1.g == 64 && test1.b == 32 && test1.a == 180,
                      "Alternate packed integer assignment failed");
    
    
#pragma mark Comparison Test
    Color4 test6;
    test1.set(0,0,0,0);         test2.set(0,0,255,255); test3.set(255,255,0,0);
    test4.set(255,255,255,255); test5.set(0,0,0,255);   test6.set(255,255,255,0);
    
    CUAssertAlwaysLog(test1 < test4,    "Less than failed");
    CUAssertAlwaysLog(!(test4 < test1), "Less than failed");
    CUAssertAlwaysLog(test1 < test2,    "Less than failed");
    CUAssertAlwaysLog(test2 < test3,    "Less than failed");
    CUAssertAlwaysLog(!(test1 < test1), "Less than failed");
    
    CUAssertAlwaysLog(test1 <= test4,   "Less than or equal to failed");
    CUAssertAlwaysLog(!(test4 <= test1),"Less than or equal to failed");
    CUAssertAlwaysLog(test1 <= test2,   "Less than or equal to failed");
    CUAssertAlwaysLog(test2 <= test3,   "Less than or equal to failed");
    CUAssertAlwaysLog(test1 <= test1,   "Less than or equal to failed");
    
    CUAssertAlwaysLog(test4 > test1,    "Greater than failed");
    CUAssertAlwaysLog(!(test1 > test4), "Greater than failed");
    CUAssertAlwaysLog(test2 > test1,    "Greater than failed");
    CUAssertAlwaysLog(test3 > test2,    "Greater than failed");
    CUAssertAlwaysLog(!(test1 > test1), "Greater than failed");
    
    CUAssertAlwaysLog(test4 >= test1,   "Greater than or equal to failed");
    CUAssertAlwaysLog(!(test1 >= test4),"Greater than or equal to failed");
    CUAssertAlwaysLog(test2 >= test1,   "Greater than or equal to failed");
    CUAssertAlwaysLog(test3 >= test2,   "Greater than or equal to failed");
    CUAssertAlwaysLog(test1 >= test1,   "Greater than or equal to failed");
    
    CUAssertAlwaysLog(test1 == test1,   "Equals failed");
    CUAssertAlwaysLog(test2 == test2,   "Equals failed");
    CUAssertAlwaysLog(test3 == test3,   "Equals failed");
    CUAssertAlwaysLog(test4 == test4,   "Equals failed");
    CUAssertAlwaysLog(!(test1 == test4),"Equals failed");
    
    CUAssertAlwaysLog(!(test1 != test1),"Not equals failed");
    CUAssertAlwaysLog(!(test2 != test2),"Not equals failed");
    CUAssertAlwaysLog(!(test3 != test3),"Not equals failed");
    CUAssertAlwaysLog(!(test4 != test4),"Not equals failed");
    CUAssertAlwaysLog(test1 != test4,   "Not equals failed");
    
    CUAssertAlwaysLog(test5.darkerThan(test4), "Method darkerThan() failed");
    CUAssertAlwaysLog(test4.darkerThan(test6), "Method darkerThan() failed");
    CUAssertAlwaysLog(test5.darkerThan(test6), "Method darkerThan() failed");
    CUAssertAlwaysLog(!test1.darkerThan(test4),"Method darkerThan() failed");
    CUAssertAlwaysLog(!test4.darkerThan(test1),"Method darkerThan() failed");
    CUAssertAlwaysLog(!test2.darkerThan(test3),"Method darkerThan() failed");
    CUAssertAlwaysLog(!test3.darkerThan(test2),"Method darkerThan() failed");
    CUAssertAlwaysLog(test1.darkerThan(test1), "Method darkerThan() failed");
    
    CUAssertAlwaysLog(test4.lighterThan(test5), "Method lighterThan() failed");
    CUAssertAlwaysLog(test6.lighterThan(test4), "Method lighterThan() failed");
    CUAssertAlwaysLog(test6.lighterThan(test5), "Method lighterThan() failed");
    CUAssertAlwaysLog(!test4.lighterThan(test1),"Method lighterThan() failed");
    CUAssertAlwaysLog(!test1.lighterThan(test4),"Method lighterThan() failed");
    CUAssertAlwaysLog(!test2.lighterThan(test3),"Method lighterThan() failed");
    CUAssertAlwaysLog(!test3.lighterThan(test2),"Method lighterThan() failed");
    CUAssertAlwaysLog(test1.lighterThan(test1), "Method lighterThan() failed");
    

#pragma mark Arithmetic Test
    test1.set(166,64,192,32);
    test2.set(166,64,192,32);
    test4.set(128,128,128,128);
    
    test2.clamp(Color4(60,60,30,30),Color4(195,195,225,225));
    CUAssertAlwaysLog(test1 == test2,                               "Method clamp() failed");
    
    test2.clamp(test4,Color4(195,195,225,225));
    CUAssertAlwaysLog(test1 != test2,                               "Method clamp() failed");
    CUAssertAlwaysLog(test2.r == 166 && test2.g == 128 && test2.b == 192 && test2.a == 128,
                      "Method clamp() failed");
    
    test2 = test1;
    test2.clamp(Color4(60,60,30,30),test4);
    CUAssertAlwaysLog(test1 != test2,                               "Method clamp() failed");
    CUAssertAlwaysLog(test2.r == 128 && test2.g == 64 && test2.b == 128 && test2.a == 32,
                      "Method clamp() failed");
    
    test2 = test1;
    test2.clamp(Color4(80,80,40,40),Color4(150,150,180,180));
    CUAssertAlwaysLog(test1 != test2,                               "Method clamp() failed");
    CUAssertAlwaysLog(test2.r == 150 && test2.g == 80 && test2.b == 180 && test2.a == 40,
                      "Method clamp() failed");
    
    test2 = test1;
    test3 = test2.getClamp(test4,Color4(195,195,225,225));
    CUAssertAlwaysLog(test1 == test2,                               "Method getClamp() failed");
    CUAssertAlwaysLog(test1 != test3,                               "Method getClamp() failed");
    CUAssertAlwaysLog(test3.r ==166 && test3.g == 128 && test3.b == 192 && test3.a == 128,
                      "Method getClamp() failed");
    
    test3 = test2.getClamp(Color4(60,60,30,30),test4);
    CUAssertAlwaysLog(test1 == test2,                               "Method getClamp() failed");
    CUAssertAlwaysLog(test1 != test3,                               "Method getClamp() failed");
    CUAssertAlwaysLog(test3.r == 128 && test3.g == 64 && test3.b == 128 && test3.a == 32,
                      "Method getClamp() failed");
    
    test3 = test2.getClamp(Color4(80,80,40,40),Color4(150,150,180,180));
    CUAssertAlwaysLog(test1 == test2,                               "Method getClamp() failed");
    CUAssertAlwaysLog(test1 != test3,                               "Method getClamp() failed");
    CUAssertAlwaysLog(test3.r == 150 && test3.g == 80 && test3.b == 180 && test3.a == 40,
                      "Method getClamp() failed");
    
    test1 = Color4::RED;
    test1.add(Color4::GREEN);
    test1.add(Color4::BLUE);
    CUAssertAlwaysLog(test1 == Color4f::WHITE,                      "Method add() failed");
    
    test1 = Color4::WHITE;
    test1.add(test1);
    CUAssertAlwaysLog(test1 == Color4f::WHITE,                      "Method add() failed");
    
    test1.set(128,128,128,128);
    test1.add(test1);
    CUAssertAlwaysLog(test1 == Color4(255,255,255,128),             "Method add() failed");
    
    test1.set(128,128,128,128);
    test1.add(test1,true);
    CUAssertAlwaysLog(test1 == Color4::WHITE,                       "Method add() failed");
    
    test1.set(128,128,128,128);
    test1.add(100,30,190,64);
    CUAssertAlwaysLog(test1 == Color4(228,158,255,192),             "Method add() failed");
    
    test1.set(128,128,128,128);
    test1.add(100,30,190);
    CUAssertAlwaysLog(test1 == Color4(228,158,255,128),             "Method add() failed");
    
    test1 = Color4::WHITE;
    test1.subtract(Color4::RED);
    CUAssertAlwaysLog(test1 == Color4::CYAN,                        "Method subtract() failed");
    
    test1 = Color4::WHITE;
    test1.subtract(Color4::RED, true);
    CUAssertAlwaysLog(test1 == Color4(0,255,255,0),                 "Method subtract() failed");
    
    test1 = Color4::WHITE;
    test1.subtract(test1);
    CUAssertAlwaysLog(test1 == Color4::BLACK,                       "Method subtract() failed");
    
    test1 = Color4::WHITE;
    test1.subtract(test1,true);
    CUAssertAlwaysLog(test1 == Color4::CLEAR,                       "Method subtract() failed");
    
    test1 = Color4::WHITE;
    test1.subtract(100,30,190,64);
    CUAssertAlwaysLog(test1 == Color4(155,225,65,191),              "Method subtract() failed");
    
    test1 = Color4::WHITE;
    test1.subtract(100,30,190);
    CUAssertAlwaysLog(test1 == Color4(155,225,65,255),              "Method subtract() failed");

    test1 = Color4::WHITE;
    test2 = Color4::RED;
    test3 = Color4::GREEN;
    test4 = Color4::BLUE;
    test5 = Color4::BLACK;
    test1.scale(0.5f);
    CUAssertAlwaysLog(test1 == Color4(127,127,127,255),            "Method scale() failed");
    
    test1 = Color4::WHITE;
    test1.scale(0.5f,true); test2.scale(0.5f,true); test3.scale(0.5f,true);
    test4.scale(0.5f,true); test5.scale(0.5f,true);
    CUAssertAlwaysLog(test1 == Color4(127,127,127,127),             "Method scale() failed");
    CUAssertAlwaysLog(test2 == Color4(127,0,0,127),                 "Method scale() failed");
    CUAssertAlwaysLog(test3 == Color4(0,127,0,127),                 "Method scale() failed");
    CUAssertAlwaysLog(test4 == Color4(0,0,127,127),                 "Method scale() failed");
    CUAssertAlwaysLog(test5 == Color4(0,0,0,127),                   "Method scale() failed");
    
    test1 = Color4::WHITE;
    test2 = Color4::RED;
    test3 = Color4::GREEN;
    test4 = Color4::BLUE;
    test5 = Color4::BLACK;
    test1.scale(0.5f,0.75f,0.25f);
    CUAssertAlwaysLog(test1 == Color4(127,191,63,255),              "Method scale() failed");
    
    test1 = Color4::WHITE;
    test1.scale(0.5f,0.75f,0.25f,0.125f);   test2.scale(0.5f,0.75f,0.25f,0.125f);
    test3.scale(0.5f,0.75f,0.25f,0.125f);   test4.scale(0.5f,0.75f,0.25f,0.125f);
    test5.scale(0.5f,0.75f,0.25f,0.125f);
    CUAssertAlwaysLog(test1 == Color4(127,191,63,31),               "Method scale() failed");
    CUAssertAlwaysLog(test2 == Color4(127,0,0,31),                  "Method scale() failed");
    CUAssertAlwaysLog(test3 == Color4(0,191,0,31),                  "Method scale() failed");
    CUAssertAlwaysLog(test4 == Color4(0,0,63,31),                   "Method scale() failed");
    CUAssertAlwaysLog(test5 == Color4(0,0,0,31),                    "Method scale() failed");
    
    test1 = Color4::WHITE;
    test2 = Color4::RED;
    test3 = Color4::GREEN;
    test4 = Color4::BLUE;
    test5 = Color4::BLACK;
    test6.set(192,64,32,128);
    test1.scale(test6);
    CUAssertAlwaysLog(test1 == Color4(192,64,32,255),              "Method scale() failed");
    
    test1 = Color4::WHITE;
    test1.scale(test6,true); test2.scale(test6,true); test3.scale(test6,true);
    test4.scale(test6,true); test5.scale(test6,true);
    CUAssertAlwaysLog(test1 == Color4(192,64,32,128),               "Method scale() failed");
    CUAssertAlwaysLog(test2 == Color4(192,0,0,128),                 "Method scale() failed");
    CUAssertAlwaysLog(test3 == Color4(0,64,0,128),                  "Method scale() failed");
    CUAssertAlwaysLog(test4 == Color4(0,0,32,128),                  "Method scale() failed");
    CUAssertAlwaysLog(test5 == Color4(0,0,0,128),                   "Method scale() failed");
    
    // Lambda function
    std::function<GLubyte(GLubyte)> functor = [](GLubyte x) { return 255-x; };
    
    test1 = Color4::WHITE;
    test2 = Color4::RED;
    test3 = Color4::GREEN;
    test4 = Color4::BLUE;
    test5 = Color4::BLACK;
    test6 = Color4::CLEAR;
    test1.map(functor);
    CUAssertAlwaysLog(test1.r == 0 && test1.g == 0 && test1.b == 0 && test1.a == 255,
                      "Method map() failed");
    test1 = Color4::WHITE;
    test1.map(functor,true); test2.map(functor,true); test3.map(functor,true);
    test4.map(functor,true); test5.map(functor,true); test6.map(functor,true);
    CUAssertAlwaysLog(test1.r == 0 && test1.g == 0 && test1.b == 0 && test1.a == 0,
                      "Method map() failed");
    CUAssertAlwaysLog(test2.r == 0 && test2.g == 255 && test2.b == 255 && test2.a == 0,
                      "Method map() failed");
    CUAssertAlwaysLog(test3.r == 255 && test3.g == 0 && test3.b == 255 && test3.a == 0,
                      "Method map() failed");
    CUAssertAlwaysLog(test4.r == 255 && test4.g == 255 && test4.b == 0.0f && test4.a == 0,
                      "Method map() failed");
    CUAssertAlwaysLog(test5.r == 255 && test5.g == 255 && test5.b == 255 && test5.a == 0,
                      "Method map() failed");
    CUAssertAlwaysLog(test6.r == 255 && test6.g == 255 && test6.b == 255 && test6.a == 255,
                      "Method map() failed");
    
    test1 = Color4::WHITE;
    test2 = Color4::RED;
    test3 = Color4::GREEN;
    test4 = Color4::BLUE;
    test5 = Color4::BLACK;
    test6 = Color4::CLEAR;
    Color4 test7 = test1.getMap(functor);
    CUAssertAlwaysLog(test1 != test7,   "Method getMap() failed");
    CUAssertAlwaysLog(test7.r == 0 && test7.g == 0 && test7.b == 0 && test7.a == 255,
                      "Method getMap() failed");
    test7 = test1.getMap(functor,true);
    CUAssertAlwaysLog(test7.r == 0 && test7.g == 0 && test7.b == 0 && test7.a == 0,
                      "Method getMap() failed");
    test7 = test2.getMap(functor,true);
    CUAssertAlwaysLog(test7.r == 0 && test7.g == 255 && test7.b == 255 && test7.a == 0,
                      "Method getMap() failed");
    test7 = test3.getMap(functor,true);
    CUAssertAlwaysLog(test7.r == 255 && test7.g == 0 && test7.b == 255 && test7.a == 0,
                      "Method getMap() failed");
    test7 = test4.getMap(functor,true);
    CUAssertAlwaysLog(test7.r == 255 && test7.g == 255 && test7.b == 0 && test7.a == 0,
                      "Method getMap() failed");
    test7 = test5.getMap(functor,true);
    CUAssertAlwaysLog(test7.r == 255 && test7.g == 255 && test7.b == 255 && test7.a == 0,
                      "Method getMap() failed");
    test7 = test6.getMap(functor,true);
    CUAssertAlwaysLog(test7.r == 255 && test7.g == 255 && test7.b == 255 && test7.a == 255,
                      "Method getMap() failed");
    
#pragma mark Operator Test
    test1 = Color4::RED;
    test1 += Color4::GREEN;
    test1 += Color4::BLUE;
    CUAssertAlwaysLog(test1 == Color4::WHITE,                       "Addition operation failed");
    
    test1 = Color4::WHITE;
    test1 += test1;
    CUAssertAlwaysLog(test1 == Color4::WHITE,                       "Addition operation failed");
    
    test1 = Color4(128,128,128,128);
    test1 += test1;
    CUAssertAlwaysLog(test1 == Color4::WHITE,                       "Addition operation failed");
    
    test1 = Color4(128,128,128,128);
    CUAssertAlwaysLog(Color4::GREEN+Color4::BLUE==Color4::CYAN,     "Addition operation failed");
    CUAssertAlwaysLog(test1+test1 == Color4::WHITE,                 "Addition operation failed");
    
    test1 =  Color4::MAGENTA;
    test1 -= Color4::BLUE;
    CUAssertAlwaysLog(test1 == Color4(255,0,0,0),                   "Subtraction operation failed");
    
    test1 = Color4::WHITE;
    test1 -= test1;
    CUAssertAlwaysLog(test1 == Color4::CLEAR,                       "Subtraction operation failed");
    CUAssertAlwaysLog(Color4::MAGENTA-Color4::BLUE == Color4(255,0,0,0),    "Subtraction operation failed");
    CUAssertAlwaysLog(Color4::WHITE-Color4::WHITE == Color4::CLEAR,         "Subtraction operation failed");
    
    test1 = Color4::WHITE;
    test2 = Color4::RED;
    test3 = Color4f::GREEN;
    test4 = Color4::BLUE;
    test5 = Color4::BLACK;
    test6.set(64,32,192,128);
    test1 *= 0.5f; test2 *= 0.5f; test3 *= 0.5f;
    test4 *= 0.5f; test5 *= 0.5f; test6 *= 0.5f;
    CUAssertAlwaysLog(test1 == Color4(127,127,127,127),             "Scaling operation failed");
    CUAssertAlwaysLog(test2 == Color4(127,0,0,127),                 "Scaling operation failed");
    CUAssertAlwaysLog(test3 == Color4(0,127,0,127),                 "Scaling operation failed");
    CUAssertAlwaysLog(test4 == Color4(0,0,127,127),                 "Scaling operation failed");
    CUAssertAlwaysLog(test5 == Color4(0,0,0,127),                   "Scaling operation failed");
    CUAssertAlwaysLog(test6 == Color4(32,16,96,64),                 "Scaling operation failed");
    
    test6.set(64,32,192,128);
    CUAssertAlwaysLog(Color4::WHITE*0.5 == Color4(127,127,127,127), "Scaling operation failed");
    CUAssertAlwaysLog(Color4::RED*0.5   == Color4(127,0,0,127),     "Scaling operation failed");
    CUAssertAlwaysLog(Color4::GREEN*0.5 == Color4(0,127,0,127),     "Scaling operation failed");
    CUAssertAlwaysLog(Color4::BLUE*0.5  == Color4(0,0,127,127),     "Scaling operation failed");
    CUAssertAlwaysLog(Color4::BLACK*0.5 == Color4(0,0,0,127),       "Scaling operation failed");
    CUAssertAlwaysLog(test6*0.5 == Color4(32,16,96,64),             "Scaling operation failed");
    CUAssertAlwaysLog(0.5*Color4::WHITE == Color4(127,127,127,127), "Scaling operation failed");
    CUAssertAlwaysLog(0.5*Color4::RED   == Color4(127,0,0,127),     "Scaling operation failed");
    CUAssertAlwaysLog(0.5*Color4::GREEN == Color4(0,127,0,127),     "Scaling operation failed");
    CUAssertAlwaysLog(0.5*Color4::BLUE  == Color4(0,0,127,127),     "Scaling operation failed");
    CUAssertAlwaysLog(0.5*Color4::BLACK == Color4(0,0,0,127),       "Scaling operation failed");
    CUAssertAlwaysLog(0.5*test6 == Color4(32,16,96,64),             "Scaling operation failed");
    
    test1 = Color4::WHITE;
    test2 = Color4::RED;
    test3 = Color4::GREEN;
    test4 = Color4::BLUE;
    test5 = Color4::BLACK;
    test6.set(64,32,192,128);
    test1 *= test6; test2 *= test6; test3 *= test6;
    test4 *= test6; test5 *= test6; test6 *= test6;
    CUAssertAlwaysLog(test1 == Color4(64,32,192,128),               "Scaling operation failed");
    CUAssertAlwaysLog(test2 == Color4(64,0,0,128),                  "Scaling operation failed");
    CUAssertAlwaysLog(test3 == Color4(0,32,0,128),                  "Scaling operation failed");
    CUAssertAlwaysLog(test4 == Color4(0,0,192,128),                 "Scaling operation failed");
    CUAssertAlwaysLog(test5 == Color4(0,0,0,128),                   "Scaling operation failed");
    CUAssertAlwaysLog(test6 == Color4(16,4,144,64),                 "Scaling operation failed");
    
    test6.set(64,32,192,128);
    CUAssertAlwaysLog(Color4::WHITE*test6 == Color4(64,32,192,128), "Scaling operation failed");
    CUAssertAlwaysLog(Color4::RED*test6   == Color4(64,0,0,128),    "Scaling operation failed");
    CUAssertAlwaysLog(Color4::GREEN*test6 == Color4(0,32,0,128),    "Scaling operation failed");
    CUAssertAlwaysLog(Color4::BLUE*test6  == Color4(0,0,192,128),   "Scaling operation failed");
    CUAssertAlwaysLog(Color4::BLACK*test6 == Color4(0,0,0,128),     "Scaling operation failed");
    CUAssertAlwaysLog(test6*Color4::WHITE == Color4(64,32,192,128), "Scaling operation failed");
    CUAssertAlwaysLog(test6*Color4::RED   == Color4(64,0,0,128),    "Scaling operation failed");
    CUAssertAlwaysLog(test6*Color4::GREEN == Color4(0,32,0,128),    "Scaling operation failed");
    CUAssertAlwaysLog(test6*Color4::BLUE  == Color4(0,0,192,128),   "Scaling operation failed");
    CUAssertAlwaysLog(test6*Color4::BLACK == Color4(0,0,0,128),     "Scaling operation failed");
    CUAssertAlwaysLog(test6*test6 == Color4(16,4,144,64),           "Scaling operation failed");
    
#pragma mark Color Operations Test
    test1 = Color4::WHITE;
    test1.complement();
    CUAssertAlwaysLog(test1 == Color4::BLACK,                   "Method complement() failed");
    
    test1 = Color4::RED;
    test1.complement();
    CUAssertAlwaysLog(test1 == Color4::CYAN,                    "Method complement() failed");
    
    test1 = Color4::GRAY;
    test1.complement();
    CUAssertAlwaysLog(test1 == Color4(89,89,89,255),            "Method complement() failed");
    
    test1 = Color4::WHITE;
    test1.complement(true);
    CUAssertAlwaysLog(test1 == Color4::CLEAR,                   "Method complement() failed");
    
    test1 = Color4::WHITE;
    test2 = test1.getComplement();
    CUAssertAlwaysLog(test1 != test2,                           "Method getComplement() failed");
    CUAssertAlwaysLog(test2 == Color4::BLACK,                   "Method getComplement() failed");
    
    test2 = Color4::RED.getComplement();
    CUAssertAlwaysLog(test2 == Color4::CYAN,                    "Method getComplement() failed");
    
    test2 = Color4::GRAY.getComplement();
    CUAssertAlwaysLog(test2 == Color4(89,89,89,255),            "Method getComplement() failed");
    
    test2 = Color4::WHITE.getComplement(true);
    CUAssertAlwaysLog(test2 == Color4::CLEAR,                   "Method getComplement() failed");
    
    test1 = Color4::WHITE;
    test1.premultiply();
    CUAssertAlwaysLog(test1 == Color4::WHITE,                   "Method premultiply() failed");
    
    test1 = Color4::CLEAR;
    test1.premultiply();
    CUAssertAlwaysLog(test1 == Color4::CLEAR,                   "Method premultiply() failed");
    
    test1.set(60,128,144,128);
    test1.premultiply();
    CUAssertAlwaysLog(test1 == Color4(30,64,72,128),            "Method premultiply() failed");
    
    test1 = Color4::WHITE;
    test2 = test1.getPremultiplied();
    CUAssertAlwaysLog(test2 == Color4::WHITE,                  "Method getPremultiplied() failed");
    
    test1 = Color4::CLEAR;
    test2 = test1.getPremultiplied();
    CUAssertAlwaysLog(test2 == Color4::CLEAR,                  "Method getPremultiplied() failed");
    
    test1.set(60,128,144,128);
    test2 = test1.getPremultiplied();
    CUAssertAlwaysLog(test1 != test2,                           "Method getPremultiplied() failed");
    CUAssertAlwaysLog(test2 == Color4(30,64,72,128),            "Method getPremultiplied() failed");
    
    test1 = Color4::WHITE;
    test1.unpremultiply();
    CUAssertAlwaysLog(test1 == Color4::WHITE,                   "Method unpremultiply() failed");
    
    test1 = Color4::CLEAR;
    test1.unpremultiply();
    CUAssertAlwaysLog(test1 == Color4::CLEAR,                   "Method unpremultiply() failed");
    
    test1.set(30,64,72,128);
    test1.unpremultiply();
    CUAssertAlwaysLog(test1 == Color4(59,127,143,128),          "Method unpremultiply() failed");
    
    test1 = Color4::WHITE;
    test2 = test1.getUnpremultiplied();
    CUAssertAlwaysLog(test2 == Color4::WHITE,                   "Method getUnpremultiplied() failed");
    
    test1 = Color4::CLEAR;
    test2 = test1.getUnpremultiplied();
    CUAssertAlwaysLog(test2 == Color4::CLEAR,                   "Method getUnpremultiplied() failed");
    
    test1.set(30,64,72,128);
    test2 = test1.getUnpremultiplied();
    CUAssertAlwaysLog(test1 != test2,                           "Method getPremultiplied() failed");
    CUAssertAlwaysLog(test2 == Color4(59,127,143,128),          "Method getUnpremultiplied() failed");
    
    test1 = Color4::WHITE;
    test2.set(64,0,128,192);
    test1.lerp(test2,0);
    CUAssertAlwaysLog(test1 == Color4::WHITE,                   "Method lerp() failed.");
    test1.lerp(test2,1);
    CUAssertAlwaysLog(test1 == test2,                           "Method lerp() failed.");
    test1 = Color4::WHITE;
    test1.lerp(test2,0.5);
    CUAssertAlwaysLog(test1 == Color4(159,127,191,223),         "Method lerp() failed.");
    test1 = Color4::WHITE;
    test1.lerp(test2,-1);
    CUAssertAlwaysLog(test1 == Color4::WHITE,                   "Method lerp() failed.");
    test1 = Color4::WHITE;
    test1.lerp(test2,2);
    CUAssertAlwaysLog(test1 == test2,                           "Method lerp() failed.");

    test1 = Color4::WHITE;
    test2.set(64,0,128,192);
    test3 = test1.getLerp(test2,0);
    CUAssertAlwaysLog(test3 == Color4::WHITE,                   "Method getLerp() failed.");
    test3 = test1.getLerp(test2,1);
    CUAssertAlwaysLog(test1 != test3,                           "Method getLerp() failed.");
    CUAssertAlwaysLog(test3 == test2,                           "Method getLerp() failed.");
    test3 = test1.getLerp(test2,0.5);
    CUAssertAlwaysLog(test3 == Color4(159,127,191,223),         "Method getLerp() failed.");
    test3 = test1.getLerp(test2,-1);
    CUAssertAlwaysLog(test3 == Color4::WHITE,                   "Method getLerp() failed.");
    test3 = test1.getLerp(test2,2);
    CUAssertAlwaysLog(test3 == test2,                           "Method getLerp() failed.");

    test1 = Color4::WHITE;
    test2.set(102,179,0,128);
    test1.blend(test2);
    CUAssertAlwaysLog(test1 == Color4(178,216,126,255),         "Method blend() failed.");

    test1 = Color4::WHITE;
    test1.a = 154;
    test1.blend(test2);
    CUAssertAlwaysLog(test1 == Color4(159,207,95,205),          "Method blend() failed.");
    
    test1 = Color4::WHITE;
    test3 = test1.getBlend(test2);
    CUAssertAlwaysLog(test1 != test3,                           "Method getBlend() failed.");
    CUAssertAlwaysLog(test3 != test2,                           "Method getBlend() failed.");
    CUAssertAlwaysLog(test3==Color4(178,216,126,255),           "Method getBlend() failed.");
    
    test1.a = 154;
    test3 = test1.getBlend(test2);
    CUAssertAlwaysLog(test3 == Color4(159,207,95,205),          "Method getBlend() failed.");
    
    test1 = Color4::WHITE;
    test2.set(102,179,0,128);
    test2.premultiply();
    test1.blendPre(test2);
    CUAssertAlwaysLog(test1 == Color4(178,216,126,255),         "Method blendPre() failed.");
    
    test1 = Color4f::WHITE;
    test1.a = 154;
    test1.premultiply();
    test1.blendPre(test2);
    test1.unpremultiply();
    CUAssertAlwaysLog(test1 == Color4(157,205,94,205),          "Method blendPre() failed."); // SOME ROUND OFF ERROR
    
    test1 = Color4::WHITE;
    test3 = test1.getBlendPre(test2);
    CUAssertAlwaysLog(test1 != test3,                           "Method getBlendPre() failed.");
    CUAssertAlwaysLog(test1 != test2,                           "Method getBlendPre() failed.");
    CUAssertAlwaysLog(test3 == Color4(178,216,126,255),         "Method getBlendPre() failed.");
    
    test1.a = 154;
    test1.premultiply();
    test3 = test1.getBlendPre(test2);
    test3.unpremultiply();
    CUAssertAlwaysLog(test3 == Color4(157,205,94,205),          "Method getBlendPre() failed.");
    
    CUAssertAlwaysLog(Color4::WHITE.getRGBA()  == 0xffffffff,   "Method getRGB() failed.");
    CUAssertAlwaysLog(Color4::RED.getRGBA()    == 0xff0000ff,   "Method getRGB() failed.");
    CUAssertAlwaysLog(Color4::GREEN.getRGBA()  == 0x00ff00ff,   "Method getRGB() failed.");
    CUAssertAlwaysLog(Color4::BLUE.getRGBA()   == 0x0000ffff,   "Method getRGB() failed.");
    CUAssertAlwaysLog(Color4::BLACK.getRGBA()  == 0x000000ff,   "Method getRGB() failed.");
    CUAssertAlwaysLog(Color4::ORANGE.getRGBA() == 0xff8000ff,   "Method getRGB() failed.");
    CUAssertAlwaysLog(Color4::CLEAR.getRGBA()  == 0x0,          "Method getRGB() failed.");
    
#pragma mark Static Color Operations Test
    Color4* testptr;
    
    test1 = Color4::WHITE;
    test2.set(64,0,128,192);
    testptr = Color4::lerp(test1, test2, 0, &test3);
    CUAssertAlwaysLog(testptr == &test3,                        "Color4::getLerp() failed");
    CUAssertAlwaysLog(test3 == test1,                           "Color4::getLerp() failed.");
    Color4::lerp(test1, test2, 1, &test3);
    CUAssertAlwaysLog(test3 == test2,                           "Color4::getLerp() failed.");
    Color4::lerp(test1, test2, 0.5, &test3);
    CUAssertAlwaysLog(test3 == Color4(159,127,191,223),         "Color4::getLerp() failed.");
    Color4::lerp(test1, test2, -1, &test3);
    CUAssertAlwaysLog(test3 == test1,                           "Color4::getLerp() failed.");
    Color4::lerp(test1, test2, 2, &test3);
    CUAssertAlwaysLog(test3 == test2,                           "Color4::getLerp() failed.");
    
    test1 = Color4f::WHITE;
    test2.set(102,179,0,128);
    testptr = Color4::blend(test2, test1, &test3);
    CUAssertAlwaysLog(testptr == &test3,                        "Color4::getBlend() failed");
    CUAssertAlwaysLog(test1 != test3,                           "Color4::getBlend() failed.");
    CUAssertAlwaysLog(test2 != test3,                           "Color4::getBlend() failed.");
    CUAssertAlwaysLog(test3 == Color4(178,216,126,255),         "Color4::getBlend() failed.");
    
    test1.a = 154;
    Color4::blend(test2, test1, &test3);
    CUAssertAlwaysLog(test3 == Color4(159,207,95,205),          "Color4::getBlend() failed.");
    
    test1 = Color4f::WHITE;
    test2.premultiply();
    testptr = Color4::blendPre(test2, test1, &test3);
    test3 = test1.getBlendPre(test2);
    CUAssertAlwaysLog(testptr == &test3,                        "Color4::getBlendPre() failed");
    CUAssertAlwaysLog(test1 != test3,                           "Color4::getBlendPre() failed.");
    CUAssertAlwaysLog(test2 != test3,                           "Color4::getBlendPre() failed.");
    CUAssertAlwaysLog(test3==Color4(178,216,126,255),           "Color4::getBlendPre() failed.");
    
    test1.a = 154;
    test1.premultiply();
    testptr = Color4::blendPre(test2, test1, &test3);
    test3.unpremultiply();
    CUAssertAlwaysLog(test3 == Color4(157,205,94,205),          "Color4::getBlendPre() failed.");
    
#pragma mark Conversion Test
    std::string str;
    std::string a, b, c, d;
    test1.set(64,128,255,192); str = test1.toString();
    a = cugl::to_string(64);    b = cugl::to_string(128);
    c = cugl::to_string(255);   d = cugl::to_string(192);
    CUAssertAlwaysLog(str == "[r="+a+",g="+b+",b="+c+",a="+d+"]",               "Method toString() failed");
    str = test1.toString(true);
    CUAssertAlwaysLog(str == "cugl::Color4[r="+a+",g="+b+",b="+c+",a="+d+"]",   "Method toString() failed");
    str = (std::string)test1;
    CUAssertAlwaysLog(str ==  "[r="+a+",g="+b+",b="+c+",a="+d+"]",              "String cast failed");
    
    Color4f cftest = (Color4f)test1;
    CUAssertAlwaysLog(cftest.equals(Color4f(0.25f,0.5f,1.0f,0.75f),0.005f),     "Color4 cast failed");
    Color4 test8(cftest);
    CUAssertAlwaysLog(test8 == test1,                           "Color constructor failed");
    test7 = cftest;
    CUAssertAlwaysLog(test7 == test8,                           "Color assignment failed");
    
    Vec3 v3test = (Vec3)test1;
    CUAssertAlwaysLog(v3test.equals(Vec3(0.25f,0.5f,1.0f),0.005f),              "Vec3 cast failed");
    Color4 test9(v3test);
    CUAssertAlwaysLog(test9 == Color4(64,128,255,255),          "Vec3 constructor failed");
    test8 = v3test;
    CUAssertAlwaysLog(test8 == test9,                           "Vec3 assignment failed");
    
    Vec4 v4test = (Vec4)test1;
    CUAssertAlwaysLog(v4test.equals(Vec4(0.25f,0.5f,1.0f,0.75f),0.005f),        "Vec4 cast failed");
    Color4 test10(v4test);
    CUAssertAlwaysLog(test10 == test1,                          "Vec4 constructor failed");
    test9 = v4test;
    CUAssertAlwaysLog(test9 == test10,                          "Vec4 assignment failed");
    
#pragma mark Complete
    CULog("Color4f tests complete.\n");
}

#pragma mark -
#pragma mark Size

/**
 * Unit test for a 2-dimensional size.
 */
void cugl::testSize() {
    CULog("Running tests for Size.\n");
    
#pragma mark Constructor Test
    // Initial test of constructors
    Size test1;
    CUAssertAlwaysLog(test1.width == 0 && test1.height == 0,    "Trivial constructor failed");
    
    Size test2(1.5,4);
    CUAssertAlwaysLog(test2.width == 1.5 && test2.height == 4,  "Initialization constructor failed");
    
    float f[2] = {3.5, 6};
    Size test3(f);
    CUAssertAlwaysLog(test3.width == 3.5 && test3.height == 6,  "Array constructor failed");
    
    Size test4(test2);
    CUAssertAlwaysLog(test4.width == 1.5 && test4.height == 4,  "Copy constructor failed");
    
    
#pragma mark Constants Test
    CUAssertAlwaysLog(Size::ZERO.width == 0 && Size::ZERO.height == 0,  "Zero size failed");
    
#pragma mark Setter Test
    test1 = test2;
    CUAssertAlwaysLog(test1.width == 1.5 && test1.height == 4,  "Basic assignment failed");
    
    test1 = f;
    CUAssertAlwaysLog(test1.width == 3.5 && test1.height == 6,  "Float assignment failed");
    
    test1.set(-1,1);
    CUAssertAlwaysLog(test1.width == -1 && test1.height == 1,   "Parameter assignment failed");
    
    test1.set(test2);
    CUAssertAlwaysLog(test1.width == 1.5 && test1.height == 4,  "Alternate assignment failed");
    
    test1.set(f);
    CUAssertAlwaysLog(test1.width == 3.5 && test1.height == 6,  "Alternate float assignment failed");
    
#pragma mark Comparison Test
    test1.set(0,0); test2.set(0,1); test3.set(1,0); test4.set(1,1);
    
    CUAssertAlwaysLog(test1 < test4,    "Less than failed");
    CUAssertAlwaysLog(!(test4 < test1), "Less than failed");
    CUAssertAlwaysLog(test1 < test2,    "Less than failed");
    CUAssertAlwaysLog(test2 < test3,    "Less than failed");
    CUAssertAlwaysLog(!(test1 < test1), "Less than failed");
    
    CUAssertAlwaysLog(test1 <= test4,   "Less than or equal to failed");
    CUAssertAlwaysLog(!(test4 <= test1),"Less than or equal to failed");
    CUAssertAlwaysLog(test1 <= test2,   "Less than or equal to failed");
    CUAssertAlwaysLog(test2 <= test3,   "Less than or equal to failed");
    CUAssertAlwaysLog(test1 <= test1,   "Less than or equal to failed");
    
    CUAssertAlwaysLog(test4 > test1,    "Greater than failed");
    CUAssertAlwaysLog(!(test1 > test4), "Greater than failed");
    CUAssertAlwaysLog(test2 > test1,    "Greater than failed");
    CUAssertAlwaysLog(test3 > test2,    "Greater than failed");
    CUAssertAlwaysLog(!(test1 > test1), "Greater than failed");
    
    CUAssertAlwaysLog(test4 >= test1,   "Greater than or equal to failed");
    CUAssertAlwaysLog(!(test1 >= test4),"Greater than or equal to failed");
    CUAssertAlwaysLog(test2 >= test1,   "Greater than or equal to failed");
    CUAssertAlwaysLog(test3 >= test2,   "Greater than or equal to failed");
    CUAssertAlwaysLog(test1 >= test1,   "Greater than or equal to failed");
    
    CUAssertAlwaysLog(test1 == test1,   "Equals failed");
    CUAssertAlwaysLog(test2 == test2,   "Equals failed");
    CUAssertAlwaysLog(test3 == test3,   "Equals failed");
    CUAssertAlwaysLog(!(test1 == test4),"Equals failed");
    
    CUAssertAlwaysLog(!(test1 != test1),"Not equals failed");
    CUAssertAlwaysLog(!(test2 != test2),"Not equals failed");
    CUAssertAlwaysLog(!(test3 != test3),"Not equals failed");
    CUAssertAlwaysLog(test1 != test4,   "Not equals failed");
    
    CUAssertAlwaysLog(test1.inside(test4),      "Method inside() failed");
    CUAssertAlwaysLog(!test4.inside(test1),     "Method inside() failed");
    CUAssertAlwaysLog(!test2.inside(test3),     "Method inside() failed");
    CUAssertAlwaysLog(!test3.inside(test2),     "Method inside() failed");
    CUAssertAlwaysLog(test1.inside(test1),      "Method inside() failed");
    
    CUAssertAlwaysLog(test4.contains(test1),    "Method over() failed");
    CUAssertAlwaysLog(!test1.contains(test4),   "Method over() failed");
    CUAssertAlwaysLog(!test2.contains(test3),   "Method over() failed");
    CUAssertAlwaysLog(!test3.contains(test2),   "Method over() failed");
    CUAssertAlwaysLog(test1.contains(test1),    "Method over() failed");
    
    Size test5;
    test5.set(0,CU_MATH_EPSILON*0.5);
    CUAssertAlwaysLog(test1.equals(test1),      "Approximate equals failed");
    CUAssertAlwaysLog(test1.equals(test5),      "Approximate equals failed");
    
#pragma mark Operator Test
    test1.set(1,0);
    test2.set(0,1);
    test1 += test2;
    CUAssertAlwaysLog(test1 == Size(1,1),       "Addition operation failed");
    
    test1 += test1;
    CUAssertAlwaysLog(test1 == Size(2,2),       "Addition operation failed");
    CUAssertAlwaysLog(Size(1,0)+Size(0,1) == Size(1,1),     "Addition operation failed");
    CUAssertAlwaysLog(Size(1,1)+Size(1,1) == Size(2,2),     "Addition operation failed");
    
    test1.set(1,0);
    test1 -= test2;
    CUAssertAlwaysLog(test1 == Size(1,-1),      "Subtraction operation failed");
    
    test1.set(1,1);
    test1 -= test1;
    CUAssertAlwaysLog(test1 == Size::ZERO,      "Subtraction operation failed");
    CUAssertAlwaysLog(Size(1,0)-Size(0,1) == Size(1,-1),    "Subtraction operation failed");
    CUAssertAlwaysLog(Size(1,1)-Size(1,1) == Size::ZERO,    "Subtraction operation failed");
    
    test1.set(1,1);
    test2.set(1,0);
    test3.set(0,1);
    test1 *= 2; test2 *= 2; test3 *= 2;
    CUAssertAlwaysLog(test1 == Size(2,2),           "Scaling operation failed");
    CUAssertAlwaysLog(test2 == Size(2,0),           "Scaling operation failed");
    CUAssertAlwaysLog(test3 == Size(0,2),           "Scaling operation failed");
    CUAssertAlwaysLog(Size(1,1)*2 == Size(2,2),     "Scaling operation failed");
    CUAssertAlwaysLog(Size(1,0)*2 == Size(2,0),     "Scaling operation failed");
    CUAssertAlwaysLog(Size(0,1)*2 == Size(0,2),     "Scaling operation failed");
    CUAssertAlwaysLog(2*Size(1,1) == Size(2,2),     "Scaling operation failed");
    CUAssertAlwaysLog(2*Size(1,0) == Size(2,0),     "Scaling operation failed");
    CUAssertAlwaysLog(2*Size(0,1) == Size(0,2),     "Scaling operation failed");
    
    test1.set(1,1);
    test2.set(1,0);
    test3.set(0,1);
    test4.set(2,3);
    test1 *= test4; test2 *= test4; test3 *= test4;
    CUAssertAlwaysLog(test1 == Size(2,3),           "Scaling operation failed");
    CUAssertAlwaysLog(test2 == Size(2,0),           "Scaling operation failed");
    CUAssertAlwaysLog(test3 == Size(0,3),           "Scaling operation failed");
    CUAssertAlwaysLog(Size(1,1)*test4 == Size(2,3), "Scaling operation failed");
    CUAssertAlwaysLog(Size(1,0)*test4 == Size(2,0), "Scaling operation failed");
    CUAssertAlwaysLog(Size(0,1)*test4 == Size(0,3), "Scaling operation failed");
    
    test1.set(1,1);
    test2.set(1,0);
    test3.set(0,1);
    test1 /= 0.5; test2 /= 0.5; test3 /= 0.5;
    CUAssertAlwaysLog(test1 == Size(2,2),           "Division operation failed");
    CUAssertAlwaysLog(test2 == Size(2,0),           "Division operation failed");
    CUAssertAlwaysLog(test3 == Size(0,2),           "Division operation failed");
    CUAssertAlwaysLog(Size(1,1)/0.5 == Size(2,2),   "Division operation failed");
    CUAssertAlwaysLog(Size(1,0)/0.5 == Size(2,0),   "Division operation failed");
    CUAssertAlwaysLog(Size(0,1)/0.5 == Size(0,2),   "Division operation failed");
    
    
    test1.set(1,1);
    test2.set(1,0);
    test3.set(0,1);
    test4.set(1/2.0f,1/4.0f);
    test1 /= test4; test2 /= test4; test3 /= test4;
    CUAssertAlwaysLog(test1 == Size(2,4),           "Division operation failed");
    CUAssertAlwaysLog(test2 == Size(2,0),           "Division operation failed");
    CUAssertAlwaysLog(test3 == Size(0,4),           "Division operation failed");
    CUAssertAlwaysLog(Size(1,1)/test4 == Size(2,4), "Division operation failed");
    CUAssertAlwaysLog(Size(1,0)/test4 == Size(2,0), "Division operation failed");
    CUAssertAlwaysLog(Size(0,1)/test4 == Size(0,4), "Division operation failed");
    
#pragma mark Accessor Test
    
    test1.set(1.0f,2.0f);
    test2.set(1.2f,2.3f);
    test3.set(1.6f,2.7f);
    test4.set(-2.3f,-1.2f);
    test5.set(-2.7f,-1.6f);
    CUAssertAlwaysLog(test1.getIWidth() == 1,       "Method getIWidth failed");
    CUAssertAlwaysLog(test2.getIWidth() == 2,       "Method getIWidth failed");
    CUAssertAlwaysLog(test3.getIWidth() == 2,       "Method getIWidth failed");
    CUAssertAlwaysLog(test4.getIWidth() == -2,      "Method getIWidth failed");
    CUAssertAlwaysLog(test5.getIWidth() == -2,      "Method getIWidth failed");

    CUAssertAlwaysLog(test1.getIHeight() == 2,      "Method getIHeight failed");
    CUAssertAlwaysLog(test2.getIHeight() == 3,      "Method getIHeight failed");
    CUAssertAlwaysLog(test3.getIHeight() == 3,      "Method getIHeight failed");
    CUAssertAlwaysLog(test4.getIHeight() == -1,     "Method getIHeight failed");
    CUAssertAlwaysLog(test5.getIHeight() == -1,     "Method getIHeight failed");

#pragma mark Conversion Test
    std::string str;
    std::string a, b;
    test1.set(2,3); str = test1.toString();
    a = cugl::to_string(2.0f); b = cugl::to_string(3.0f);
    CUAssertAlwaysLog(str == "(w="+a+",h="+b+")",           "Method toString() failed");
    str = test1.toString(true);
    CUAssertAlwaysLog(str == "cugl::Size(w="+a+",h="+b+")", "Method toString() failed");
    str = (std::string)test1;
    CUAssertAlwaysLog(str == "(w="+a+",h="+b+")",           "String cast failed");
    
    Vec2 v2test = (Vec2)test1;
    CUAssertAlwaysLog(v2test.x == 2 && v2test.y == 3,       "Vec2 cast failed");
    Size test6(v2test);
    CUAssertAlwaysLog(test6 == test1,                       "Vec2 constructor failed");
    test5 = v2test;
    CUAssertAlwaysLog(test5 == test1,                       "Vec2 assignment failed");
    test4.set(v2test);
    CUAssertAlwaysLog(test4 == test1,                       "Alternate Vec2 assignment failed");
    
    Vec2 other(4,1);
    Size test7(v2test,other);
    CUAssertAlwaysLog(test7 == Size(2,2),                   "Envelope constructor failed");
    test6.set(v2test,other);
    CUAssertAlwaysLog(test6 == Size(2,2),                   "Envelope assignment failed");
    
#pragma mark Complete
    CULog("Size tests complete.\n");
}

#pragma mark -
#pragma mark Rect
/**
 * Unit test for a 2-dimensional bounding box.
 */
void cugl::testRect() {
    CULog("Running tests for Rect.\n");
    
#pragma mark Constructor Test
    // Initial test of constructors
    Rect test1;
    CUAssertAlwaysLog(test1.origin == Vec2::ZERO && test1.size == Size::ZERO,   "Trivial constructor failed");
    
    Rect test2(1,2,3,4);
    CUAssertAlwaysLog(test2.origin == Vec2(1,2) && test2.size == Size(3,4),     "Initialization constructor failed");
    
    Vec2 v2test(-2, -5);
    Size sztest(0.5, 1);
    Rect test3(v2test,sztest);
    CUAssertAlwaysLog(test3.origin == v2test && test3.size == sztest,           "Alternate initialization constructor failed");

    float f[4] = {-1, 3.5, 6, 2.5};
    Rect test4(f);
    CUAssertAlwaysLog(test4.origin == Vec2(-1,3.5) && test4.size == Size(6,2.5),"Array constructor failed");

    Rect test5(test2);
    CUAssertAlwaysLog(test2.origin == Vec2(1,2) && test2.size == Size(3,4),     "Copy constructor failed");
    
#pragma mark Constants Test
    CUAssertAlwaysLog(Rect::ZERO.origin == Vec2::ZERO && Rect::ZERO.size == Size::ZERO, "Zero rect failed");
    CUAssertAlwaysLog(Rect::UNIT.origin == Vec2::ZERO && Rect::UNIT.size == Size(1,1),  "Unit rect failed");
    
#pragma mark Setter Test
    test1 = test2;
    CUAssertAlwaysLog(test1.origin==test2.origin && test1.size==test2.size,     "Basic assignment failed");
    
    test1 = f;
    CUAssertAlwaysLog(test1.origin == Vec2(-1,3.5) && test1.size == Size(6,2.5),"Float assignment failed");
    
    test1.set(1,2,3,4);
    CUAssertAlwaysLog(test1.origin == Vec2(1,2) && test1.size == Size(3,4),     "Parameter assignment failed");

    test1.set(v2test,sztest);
    CUAssertAlwaysLog(test1.origin == v2test && test1.size == sztest,           "Alternate arameter assignment failed");

    test1.set(test2);
    CUAssertAlwaysLog(test1.origin==test2.origin && test1.size==test2.size,     "Alternate assignment failed");
    
    test1.set(f);
    CUAssertAlwaysLog(test1.origin == Vec2(-1,3.5) && test1.size == Size(6,2.5),"Alternate float assignment failed");

#pragma mark Comparison Test
    test1.set(1,1,1,1); test2.set(0,0,1.5,1.5); test3.set(2,2,0.5,0.5); test4.set(0,0,3,3);
    
    CUAssertAlwaysLog(test1.inside(test4),      "Method inside() failed");
    CUAssertAlwaysLog(!test4.inside(test1),     "Method inside() failed");
    CUAssertAlwaysLog(!test2.inside(test3),     "Method inside() failed");
    CUAssertAlwaysLog(!test3.inside(test2),     "Method inside() failed");
    CUAssertAlwaysLog(test1.inside(test1),      "Method inside() failed");
    CUAssertAlwaysLog(test2.inside(test4),      "Method inside() failed");
    CUAssertAlwaysLog(test3.inside(test4),      "Method inside() failed");
    
    CUAssertAlwaysLog(test4.contains(test1),    "Method contains() failed");
    CUAssertAlwaysLog(!test1.contains(test4),   "Method contains() failed");
    CUAssertAlwaysLog(!test2.contains(test3),   "Method contains() failed");
    CUAssertAlwaysLog(!test3.contains(test2),   "Method contains() failed");
    CUAssertAlwaysLog(test1.contains(test1),    "Method contains() failed");
    CUAssertAlwaysLog(test4.contains(test2),    "Method contains() failed");
    CUAssertAlwaysLog(test4.contains(test3),    "Method contains() failed");

    CUAssertAlwaysLog(!test4.contains(Vec2::ZERO,1),"Method contains() failed");
    CUAssertAlwaysLog(test4.contains(Vec2::ONE,1),  "Method contains() failed");
    CUAssertAlwaysLog(!test4.contains(Vec2::ONE,2), "Method contains() failed");

    CUAssertAlwaysLog(test4.contains(Vec2::ZERO),   "Method touches() failed");
    CUAssertAlwaysLog(test1.contains(Vec2::ONE),    "Method touches() failed");
    CUAssertAlwaysLog(test4.touches(Vec2(1,3)),     "Method touches() failed");
    CUAssertAlwaysLog(!test4.contains(Vec2(-1,3)),  "Method touches() failed");
    CUAssertAlwaysLog(test4.contains(Vec2::ONE),    "Method touches() failed");

    CUAssertAlwaysLog(test4.touches(Vec2::ZERO),    "Method touches() failed");
    CUAssertAlwaysLog(test1.touches(Vec2::ONE),     "Method touches() failed");
    CUAssertAlwaysLog(test4.touches(Vec2(1,3)),     "Method touches() failed");
    CUAssertAlwaysLog(!test4.touches(Vec2(-1,3)),   "Method touches() failed");
    CUAssertAlwaysLog(!test4.touches(Vec2::ONE),    "Method touches() failed");

    CUAssertAlwaysLog(test1 < test4,            "Less than failed");
    CUAssertAlwaysLog(!(test4 < test1),         "Less than failed");
    CUAssertAlwaysLog(!(test2 < test4),         "Less than failed");
    CUAssertAlwaysLog(test3 < test4,            "Less than failed");
    CUAssertAlwaysLog(!(test1 < test1),         "Less than failed");
    CUAssertAlwaysLog(!(test2 < test3),         "Less than failed");
    CUAssertAlwaysLog(!(test3 < test2),         "Less than failed");

    CUAssertAlwaysLog(test1 <= test4,           "Less than or equal to failed");
    CUAssertAlwaysLog(!(test4 <= test1),        "Less than or equal to failed");
    CUAssertAlwaysLog(test2 <= test4,           "Less than or equal to failed");
    CUAssertAlwaysLog(test3 <= test4,           "Less than or equal to failed");
    CUAssertAlwaysLog(test1 <= test1,           "Less than or equal to failed");
    CUAssertAlwaysLog(!(test2 <= test3),        "Less than or equal to failed");
    CUAssertAlwaysLog(!(test3 <= test2),        "Less than or equal to failed");

    CUAssertAlwaysLog(test4 > test1,            "Greater than failed");
    CUAssertAlwaysLog(!(test1 > test4),         "Greater than failed");
    CUAssertAlwaysLog(!(test4 > test2),         "Greater than failed");
    CUAssertAlwaysLog(test4 > test3,            "Greater than failed");
    CUAssertAlwaysLog(!(test1 > test1),         "Greater than failed");
    CUAssertAlwaysLog(!(test2 > test3),         "Greater than failed");
    CUAssertAlwaysLog(!(test3 > test2),         "Greater than failed");
    
    CUAssertAlwaysLog(test4 >= test1,           "Greater than or equal to failed");
    CUAssertAlwaysLog(!(test1 >= test4),        "Greater than or equal to failed");
    CUAssertAlwaysLog(test4 >= test2,           "Greater than or equal to failed");
    CUAssertAlwaysLog(test4 >= test3,           "Greater than or equal to failed");
    CUAssertAlwaysLog(test1 >= test1,           "Greater than or equal to failed");
    CUAssertAlwaysLog(!(test2 >= test3),        "Greater than or equal to failed");
    CUAssertAlwaysLog(!(test3 >= test2),        "Greater than or equal to failed");

    
    CUAssertAlwaysLog(test1 == test1,           "Equals failed");
    CUAssertAlwaysLog(test2 == test2,           "Equals failed");
    CUAssertAlwaysLog(test3 == test3,           "Equals failed");
    CUAssertAlwaysLog(!(test1 == test4),        "Equals failed");
    
    CUAssertAlwaysLog(!(test1 != test1),        "Not equals failed");
    CUAssertAlwaysLog(!(test2 != test2),        "Not equals failed");
    CUAssertAlwaysLog(!(test3 != test3),        "Not equals failed");
    CUAssertAlwaysLog(test1 != test4,           "Not equals failed");
    
    test5.set(Vec2(0,-CU_MATH_EPSILON*0.5f),Size(1,1+CU_MATH_EPSILON*0.5f));
    CUAssertAlwaysLog(test1.equals(test1),      "Approximate equals failed");
    CUAssertAlwaysLog(test5.equals(Rect::UNIT), "Approximate equals failed");

    CUAssertAlwaysLog(test1.doesIntersect(test4),   "Method doesIntersect() failed");
    CUAssertAlwaysLog(test4.doesIntersect(test1),   "Method doesIntersect() failed");
    CUAssertAlwaysLog(test1.doesIntersect(test2),   "Method doesIntersect() failed");
    CUAssertAlwaysLog(!test2.doesIntersect(test3),  "Method doesIntersect() failed");
    CUAssertAlwaysLog(!test3.doesIntersect(test2),  "Method doesIntersect() failed");
    CUAssertAlwaysLog(test1.doesIntersect(test1),   "Method doesIntersect() failed");
    CUAssertAlwaysLog(test2.doesIntersect(test4),   "Method doesIntersect() failed");
    CUAssertAlwaysLog(test3.doesIntersect(test4),   "Method doesIntersect() failed");
    
    CUAssertAlwaysLog(!test1.doesIntersect(Vec2::ZERO,0.5), "Method doesIntersect() failed");
    CUAssertAlwaysLog(test1.doesIntersect(Vec2(0,1),1),     "Method doesIntersect() failed");
    CUAssertAlwaysLog(test4.doesIntersect(Vec2::ONE,1),     "Method doesIntersect() failed");
    CUAssertAlwaysLog(test4.doesIntersect(Vec2::ONE,2),     "Method doesIntersect() failed");

#pragma mark Arithmetic Test
    test1.set(0,1,1,1); test2.set(0,0,2,1); test3.set(0.5,-0.5,2,3); test4.set(0,0,3,4);

    test5 = test2;
    test5.merge(test3);
    CUAssertAlwaysLog(test5 == Rect(0,-0.5,2.5,3),  "Method merge() failed");
    test5 = test3;
    test5.merge(test2);
    CUAssertAlwaysLog(test5 == Rect(0,-0.5,2.5,3),  "Method merge() failed");
    test5 = test1;
    test5.merge(test2);
    CUAssertAlwaysLog(test5 == Rect(0,0,2,2),       "Method merge() failed");
    test5 = test1;
    test5.merge(test4);
    CUAssertAlwaysLog(test5 == test4,               "Method merge() failed");

    test5 = test2.getMerge(test3);
    CUAssertAlwaysLog(test5 != test2,               "Method getMerge() failed");
    CUAssertAlwaysLog(test5 == Rect(0,-0.5,2.5,3),  "Method getMerge() failed");
    test5 = test3.getMerge(test2);
    CUAssertAlwaysLog(test5 == Rect(0,-0.5,2.5,3),  "Method getMerge() failed");
    test5 = test1.getMerge(test2);
    CUAssertAlwaysLog(test5 == Rect(0,0,2,2),       "Method getMerge() failed");
    test5 = test1.getMerge(test4);
    CUAssertAlwaysLog(test5 == test4,               "Method getMerge() failed");

    test5 = test2;
    test5.intersect(test3);
    CUAssertAlwaysLog(test5 == Rect(0.5,0,1.5,1),   "Method intersect() failed");
    test5 = test3;
    test5.intersect(test2);
    CUAssertAlwaysLog(test5 == Rect(0.5,0,1.5,1),   "Method intersect() failed");
    test5 = test1;
    test5.intersect(test2);
    CUAssertAlwaysLog(test5 == Rect(0,1,1,0),       "Method intersect() failed");
    test5 = test1;
    test5.intersect(test4);
    CUAssertAlwaysLog(test5 == test1,               "Method intersect() failed");
    
    test5 = test2.getIntersection(test3);
    CUAssertAlwaysLog(test5 != test2,               "Method getIntersection() failed");
    CUAssertAlwaysLog(test5 == Rect(0.5,0,1.5,1),   "Method getIntersection() failed");
    test5 = test3.getIntersection(test2);
    CUAssertAlwaysLog(test5 == Rect(0.5,0,1.5,1),   "Method getIntersection() failed");
    test5 = test1.getIntersection(test2);
    CUAssertAlwaysLog(test5 == Rect(0,1,1,0),       "Method getIntersection() failed");
    test5 = test1.getIntersection(test4);
    CUAssertAlwaysLog(test5 == test1,               "Method getIntersection() failed");

    test5 = test1;
    test5.expand(1);
    CUAssertAlwaysLog(test5 == Rect(-1,0,2,2),     "Method expand() failed");
    test5 = test4;
    test5.expand(-1);
    CUAssertAlwaysLog(test5 == Rect(1,1,2,3),       "Method expand() failed");

    test5 = test2.getExpansion(1);
    CUAssertAlwaysLog(test5 != test2,               "Method getExpansion() failed");
    CUAssertAlwaysLog(test5 == Rect(-1,-1,3,2),     "Method getExpansion() failed");
    test5 = test4.getExpansion(-1);
    CUAssertAlwaysLog(test5 == Rect(1,1,2,3),       "Method getExpansion() failed");

    test5 = test1;
    test5.expand(Vec2::ZERO);
    CUAssertAlwaysLog(test5 == Rect(0,0,1,2),       "Method expand() failed");
    test5 = test4;
    test5.expand(Vec2(4,4));
    CUAssertAlwaysLog(test5 == Rect(0,0,4,4),       "Method expand() failed");
    test5 = test4;
    test5.expand(Vec2::ONE);
    CUAssertAlwaysLog(test5 == test4,               "Method expand() failed");
    
    test5 = test1.getExpansion(Vec2::ZERO);
    CUAssertAlwaysLog(test5 != test1,               "Method getExpansion() failed");
    CUAssertAlwaysLog(test5 == Rect(0,0,1,2),       "Method getExpansion() failed");
    test5 = test4.getExpansion(Vec2(4,4));
    CUAssertAlwaysLog(test5 == Rect(0,0,4,4),       "Method getExpansion() failed");
    test5 = test4.getExpansion(Vec2::ONE);
    CUAssertAlwaysLog(test5 == test4,               "Method getExpansion() failed");

#pragma mark Attribute Test
    test1.set(1,2,3,4); test2.set(1,2,0,0); test3.set(1,2,-2,-4);
    
    CUAssertAlwaysLog(test1.getMinX() == 1,         "Method getMinX() failed");
    CUAssertAlwaysLog(test2.getMinX() == 1,         "Method getMinX() failed");
    CUAssertAlwaysLog(test3.getMinX() == -1,        "Method getMinX() failed");

    CUAssertAlwaysLog(test1.getMidX() == 2.5,       "Method getMidX() failed");
    CUAssertAlwaysLog(test2.getMidX() == 1,         "Method getMidX() failed");
    CUAssertAlwaysLog(test3.getMidX() == 0,         "Method getMidX() failed");

    CUAssertAlwaysLog(test1.getMaxX() == 4,         "Method getMaxX() failed");
    CUAssertAlwaysLog(test2.getMaxX() == 1,         "Method getMaxX() failed");
    CUAssertAlwaysLog(test3.getMaxX() == 1,         "Method getMaxX() failed");

    CUAssertAlwaysLog(test1.getMinY() == 2,         "Method getMinY() failed");
    CUAssertAlwaysLog(test2.getMinY() == 2,         "Method getMinY() failed");
    CUAssertAlwaysLog(test3.getMinY() == -2,        "Method getMinY() failed");
    
    CUAssertAlwaysLog(test1.getMidY() == 4,         "Method getMidY() failed");
    CUAssertAlwaysLog(test2.getMidY() == 2,         "Method getMidY() failed");
    CUAssertAlwaysLog(test3.getMidY() == 0,         "Method getMidY() failed");
    
    CUAssertAlwaysLog(test1.getMaxY() == 6,         "Method getMaxY() failed");
    CUAssertAlwaysLog(test2.getMaxY() == 2,         "Method getMaxY() failed");
    CUAssertAlwaysLog(test3.getMaxY() == 2,         "Method getMaxY() failed");

    test4.set(1,2,2,-4); test5.set(1,2,-2,4);
    CUAssertAlwaysLog(!test1.isDegenerate(),        "Method isDegenerate() failed");
    CUAssertAlwaysLog(test2.isDegenerate(),         "Method isDegenerate() failed");
    CUAssertAlwaysLog(test3.isDegenerate(),         "Method isDegenerate() failed");
    CUAssertAlwaysLog(test4.isDegenerate(),         "Method isDegenerate() failed");
    CUAssertAlwaysLog(test5.isDegenerate(),         "Method isDegenerate() failed");

    
#pragma mark Complete
    CULog("Rect tests complete.\n");
}

#pragma mark -
#pragma mark Quaternion
/**
 * Unit test for a quaternion
 */
void cugl::testQuaternion() {
    CULog("Running tests for Quaternion.\n");
    Timestamp start, end;
    
#pragma mark Constructor Test
    start.mark();
    // Initial test of constructors
    Quaternion test1;
    CUAssertAlwaysLog(test1.x == 0 && test1.y == 0 && test1.z == 0 && test1.w == 0,
                      "Trivial constructor failed");
    
    Quaternion test2(1.5,4,-2.5,6);
    CUAssertAlwaysLog(test2.x == 1.5 && test2.y == 4 && test2.z == -2.5 && test2.w == 6,
                      "Initialization constructor failed");
    
    float f[4] = {3.5, 6, 0.5, -2};
    Quaternion test3(f);
    CUAssertAlwaysLog(test3.x == 3.5 && test3.y == 6  && test3.z == 0.5 && test3.w == -2,
                      "Array constructor failed");
    
    Quaternion test4(test2);
    CUAssertAlwaysLog(test4.x == 1.5 && test4.y == 4 && test4.z == -2.5 && test4.w == 6,
                      "Copy constructor failed");
    
    Vec3 v3test(1,2,1);
    
    Quaternion test5(Vec3::UNIT_Z,M_PI_2);
    CUAssertAlwaysLog(CU_MATH_APPROX(test5.x, 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.y, 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.z, 1.0f/sqrt(2.0f), CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.w, 1.0f/sqrt(2.0f), CU_MATH_EPSILON),
                      "Rotational constructor failed");

    Quaternion test6(v3test,M_PI_4);
    CUAssertAlwaysLog(CU_MATH_APPROX(test6.x, 0.156229854, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test6.y, 0.312459707, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test6.z, 0.156229854, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test6.w, 0.923879504, CU_MATH_EPSILON),
                      "Rotational constructor failed");
    
    Quaternion* testptr;
    testptr = Quaternion::createFromAxisAngle(v3test,M_PI_4,&test5);
    CUAssertAlwaysLog(testptr == &test5,
                      "Static rotational constructor failed");
    CUAssertAlwaysLog(CU_MATH_APPROX(test5.x, 0.156229854, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.y, 0.312459707, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.z, 0.156229854, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.w, 0.923879504, CU_MATH_EPSILON),
                      "Static rotational constructor failed");
    
#pragma mark Constants Test
    CUAssertAlwaysLog(Quaternion::ZERO.x == 0 && Quaternion::ZERO.y == 0 &&
                      Quaternion::ZERO.z == 0 && Quaternion::ZERO.w == 0,
                      "Zero quaternion failed");
    CUAssertAlwaysLog(Quaternion::IDENTITY.x == 0 && Quaternion::IDENTITY.y == 0 &&
                      Quaternion::IDENTITY.z == 0 && Quaternion::IDENTITY.w == 1,
                      "Identity quaternion failed");
    
    
#pragma mark Setter Test
    test1 = test2;
    CUAssertAlwaysLog(test1.x == 1.5 && test1.y == 4 && test1.z == -2.5 && test1.w == 6,
                      "Basic assignment failed");
    
    test1 = f;
    CUAssertAlwaysLog(test1.x == 3.5 && test1.y == 6  && test1.z == 0.5 && test1.w == -2,
                      "Float assignment failed");
    
    test1.set(-1,1,5,-2);
    CUAssertAlwaysLog(test1.x == -1 && test1.y == 1 && test1.z == 5 && test1.w == -2,
                      "Parameter assignment failed");
    
    test1.set(test2);
    CUAssertAlwaysLog(test1.x == 1.5 && test1.y == 4 && test1.z == -2.5  && test1.w == 6,
                      "Alternate assignment failed");
    
    test1.set(f);
    CUAssertAlwaysLog(test1.x == 3.5 && test1.y == 6  && test1.z == 0.5 && test1.w == -2,
                      "Alternate float assignment failed");
    
    test1.set(Vec3::UNIT_Z,M_PI_2);
    CUAssertAlwaysLog(CU_MATH_APPROX(test1.x, 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test1.y, 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test1.z, 1.0f/sqrtf(2.0f), CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test1.w, 1.0f/sqrtf(2.0f), CU_MATH_EPSILON),
                      "Rotational assignment failed");

    test1.set(v3test,M_PI_4);
    CUAssertAlwaysLog(CU_MATH_APPROX(test1.x, 0.156229854, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test1.y, 0.312459707, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test1.z, 0.156229854, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test1.w, 0.923879504, CU_MATH_EPSILON),
                      "Rotational assignment failed");
    
    test1.setZero();
    CUAssertAlwaysLog(test1.x == 0 && test1.y == 0 && test1.z == 0  && test1.w == 0,
                      "Erasing assignment failed");

    test1.setIdentity();
    CUAssertAlwaysLog(test1.x == 0 && test1.y == 0 && test1.z == 0  && test1.w == 1,
                      "Identity assignment failed");
    
#pragma mark Comparison Test
    test1.set(0,0,0,0); test2.set(0,0,1,1); test3.set(1,1,1,0); test4.set(1,1,1,1);
    
    CUAssertAlwaysLog(test1 == test1,   "Equals failed");
    CUAssertAlwaysLog(test2 == test2,   "Equals failed");
    CUAssertAlwaysLog(test3 == test3,   "Equals failed");
    CUAssertAlwaysLog(!(test1 == test4),"Equals failed");
    
    CUAssertAlwaysLog(!(test1 != test1),"Not equals failed");
    CUAssertAlwaysLog(!(test2 != test2),"Not equals failed");
    CUAssertAlwaysLog(!(test3 != test3),"Not equals failed");
    CUAssertAlwaysLog(test1 != test4,   "Not equals failed");
    
    
    test5.set(0,0,CU_MATH_EPSILON*0.5f,-CU_MATH_EPSILON*0.5f);
    CUAssertAlwaysLog(test1.equals(test1), "Approximate equals failed");
    CUAssertAlwaysLog(test1.equals(test5), "Approximate equals failed");

    test5.set(1,1,1+CU_MATH_EPSILON*0.5f,1-CU_MATH_EPSILON*0.5f);
    CUAssertAlwaysLog(test4.equals(test4), "Approximate equals failed");
    CUAssertAlwaysLog(test4.equals(test5), "Approximate equals failed");

    
#pragma mark Static Arithmetic Test
    test1.set(2,2,2,2);
    test2.set(1,1,1,1);
    testptr = Quaternion::add(test2,test2,&test4);
    CUAssertAlwaysLog(testptr == &test4,                            "Quaternion::add() failed");
    CUAssertAlwaysLog(test1 == test4,                               "Quaternion::add() failed");

    test1.set(3,0,2,-1);
    test3.set(2,-1,1,-2);
    Quaternion::add(test2,test3,&test4);
    CUAssertAlwaysLog(test1 == test4,                               "Quaternion::add() failed");

    Quaternion::subtract(test2,test2,&test4);
    CUAssertAlwaysLog(testptr == &test4,                            "Quaternion::subtract() failed");
    CUAssertAlwaysLog(test4 == Quaternion::ZERO,                    "Quaternion::subtract() failed");

    test1.set(-1,2,0,3);
    testptr = Quaternion::subtract(test2,test3,&test4);
    CUAssertAlwaysLog(test1 == test4,                               "Quaternion::subtract() failed");
  
    testptr = Quaternion::multiply(test3,Quaternion::IDENTITY,&test4);
    CUAssertAlwaysLog(testptr == &test4,                            "Quaternion::multiply() failed");
    CUAssertAlwaysLog(test3 == test4,                               "Quaternion::multiply() failed");

    test1.set(-1,2,0,3);
    testptr = Quaternion::multiply(test1,test3,&test4);
    CUAssertAlwaysLog(test4 == Quaternion(10,-6,0,-2),              "Quaternion::multiply() failed");

    testptr = Quaternion::divide(test3,Quaternion::IDENTITY,&test4);
    CUAssertAlwaysLog(testptr == &test4,                            "Quaternion::divide() failed");
    CUAssertAlwaysLog(test3 == test4,                               "Quaternion::divide() failed");
    
    test1.set(10,-6,0,-2);
    testptr = Quaternion::divide(test1,test3,&test4);
    CUAssertAlwaysLog(test4.equals(Quaternion(-1,2,0,3)),           "Quaternion::divide() failed");

    test1.set(4,-2,2,-4);
    testptr = Quaternion::scale(test3,2,&test4);
    CUAssertAlwaysLog(testptr == &test4,                            "Quaternion::scale() failed");
    CUAssertAlwaysLog(test1 == test4,                               "Quaternion::scale() failed");

    testptr = Quaternion::scale(test3,0,&test4);
    CUAssertAlwaysLog(test4 == Quaternion::ZERO,                    "Quaternion::scale() failed");

    Quaternion::scale(test3,-1,&test1);
    testptr = Quaternion::negate(test3,&test4);
    CUAssertAlwaysLog(testptr == &test4,                            "Quaternion::negate() failed");
    CUAssertAlwaysLog(test4 == test1,                               "Quaternion::negate() failed");

    test1.w = -test1.w;
    testptr = Quaternion::conjugate(test3,&test4);
    CUAssertAlwaysLog(testptr == &test4,                            "Quaternion::conjugate() failed");
    CUAssertAlwaysLog(test4 == test1,                               "Quaternion::conjugate() failed");

    float value = 0.5;
    test1.set(test2);
    testptr = Quaternion::normalize(test1,&test4);
    CUAssertAlwaysLog(testptr == &test4,                            "Quaternion::normalize() failed");
    CUAssertAlwaysLog(CU_MATH_APPROX(test4.x, value, CU_MATH_EPSILON) && CU_MATH_APPROX(test4.y, value, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test4.z, value, CU_MATH_EPSILON) && CU_MATH_APPROX(test4.w, value, CU_MATH_EPSILON),
                      "Quaternion::normalize() failed");
    
    value = 1.0f/sqrtf(10);
    testptr = Quaternion::normalize(test3,&test4);
    CUAssertAlwaysLog(CU_MATH_APPROX(test4.x, 2*value, CU_MATH_EPSILON) && CU_MATH_APPROX(test4.y, -value, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test4.z, value, CU_MATH_EPSILON) && CU_MATH_APPROX(test4.w, -2*value, CU_MATH_EPSILON),
                      "Quaternion::normalize() failed");
    
    testptr = Quaternion::invert(Quaternion::IDENTITY,&test4);
    CUAssertAlwaysLog(testptr == &test4,                            "Quaternion::invert() failed");
    CUAssertAlwaysLog(test4 == Quaternion::IDENTITY,                "Quaternion::invert() failed");
    
    testptr = Quaternion::invert(test1,&test4);
    CUAssertAlwaysLog(test4.x == -0.25f && test4.y == -0.25f && test4.z == -0.25f && test4.w == 0.25f,
                      "Quaternion::invert() failed");
    
    testptr = Quaternion::invert(test3,&test4);
    CUAssertAlwaysLog(test4.x == -0.2f && test4.y == 0.1f && test4.z == -0.1f && test4.w == -0.2f,
                      "Quaternion::invert() failed");
    
#pragma mark Arithmetic Test
    test1.set(2,2,2,2);
    test2.set(1,1,1,1);
    test4.set(test2);
    test4.add(test2);
    CUAssertAlwaysLog(test1 == test4,                               "Method add() failed");
    
    test1.set(3,0,2,-1);
    test3.set(2,-1,1,-2);
    test4.set(test2);
    test4.add(test3);
    CUAssertAlwaysLog(test1 == test4,                               "Method add() failed");
    
    test4.set(test2);
    test4.subtract(test2);
    CUAssertAlwaysLog(test4 == Quaternion::ZERO,                    "Method subtract() failed");
    
    test1.set(-1,2,0,3);
    test4.set(test2);
    test4.subtract(test3);
    CUAssertAlwaysLog(test1 == test4,                               "Method subtract() failed");
    
    test4.set(test3);
    test4.multiply(Quaternion::IDENTITY);
    CUAssertAlwaysLog(test3 == test4,                               "Method multiply() failed");
    
    test1.set(-1,2,0,3);
    test4.set(test1);
    test4.multiply(test3);
    CUAssertAlwaysLog(test4 == Quaternion(10,-6,0,-2),              "Method multiply() failed");

    test4.set(test3);
    test4.divide(Quaternion::IDENTITY);
    CUAssertAlwaysLog(test3 == test4,                               "Method divide() failed");
    
    test1.set(10,-6,0,-2);
    test4.set(test1);
    test4.divide(test3);
    CUAssertAlwaysLog(test4.equals(Quaternion(-1,2,0,3)),           "Method divide() failed");

    test1.set(4,-2,2,-4);
    test4.set(test3);
    test4.scale(2);
    CUAssertAlwaysLog(test1 == test4,                               "Method scale() failed");
    
    test4.scale(0);
    CUAssertAlwaysLog(test4 == Quaternion::ZERO,                    "Method scale() failed");
    
    Quaternion::negate(test3,&test1);
    test4.set(test3);
    test4.negate();
    CUAssertAlwaysLog(test4 == test1,                               "Method negate() failed");

    test4.set(test3);
    test5 = test4.getNegation();
    CUAssertAlwaysLog(test4 == test3,                               "Method getNegation() failed");
    CUAssertAlwaysLog(test4 != test5,                               "Method getNegation() failed");
    CUAssertAlwaysLog(test5 == test1,                               "Method getNegation() failed");

    test1.w = -test1.w;
    test4.set(test3);
    test4.conjugate();
    CUAssertAlwaysLog(test4 == test1,                               "Method conjugate() failed");
    
    test4.set(test3);
    test5 = test4.getConjugate();
    CUAssertAlwaysLog(test4 == test3,                               "Method getConjugate() failed");
    CUAssertAlwaysLog(test4 != test5,                               "Method getConjugate() failed");
    CUAssertAlwaysLog(test5 == test1,                               "Method getConjugate() failed");

    value = 0.5f;
    test1.set(value,value,value,value);
    test4.set(test2);
    test4.normalize();
    CUAssertAlwaysLog(test1.equals(test4),                          "Method normalize() failed");

    value = 1.0f/sqrtf(10);
    test1.set(2*value,-value,value,-2*value);
    test4.set(test3);
    test4.normalize();
    CUAssertAlwaysLog(test1.equals(test4),                          "Method normalize() failed");

    test4.normalize();
    CUAssertAlwaysLog(test1.equals(test4),                          "Method normalize() failed");

    value = 0.5f;
    test1.set(value,value,value,value);
    test4.set(test2);
    test5 = test4.getNormalization();
    CUAssertAlwaysLog(test4.equals(test2),                          "Method getNormalization() failed");
    CUAssertAlwaysLog(!test4.equals(test5),                         "Method getNormalization() failed");
    CUAssertAlwaysLog(test5.equals(test1),                          "Method getNormalization() failed");

    value = 1.0f/sqrtf(10);
    test1.set(2*value,-value,value,-2*value);
    test4.set(test3);
    test5 = test4.getNormalization();
    CUAssertAlwaysLog(test5.equals(test1),                          "Method getNormalization() failed");
    test5 = test5.getNormalization();
    CUAssertAlwaysLog(test5.equals(test1),                          "Method getNormalization() failed");

    test4.set(Quaternion::IDENTITY);
    CUAssertAlwaysLog(test4 == Quaternion::IDENTITY,                "Method invert() failed");
    
    test1.set(-0.25f,-0.25f,-0.25f,0.25f);
    test4.set(test2);
    test4.invert();
    CUAssertAlwaysLog(test1.equals(test4),                          "Method invert() failed");

    test1.set(-0.2f,0.1f,-0.1f,-0.2f);
    test4.set(test3);
    test4.invert();
    CUAssertAlwaysLog(test1.equals(test4),                          "Method invert() failed");

    test4.invert();
    CUAssertAlwaysLog(test3.equals(test4,CU_MATH_EPSILON),          "Method invert() failed");

    test1.set(-0.25f,-0.25f,-0.25f,0.25f);
    test4.set(test2);
    test5 = test4.getInverse();
    CUAssertAlwaysLog(test4.equals(test2),                          "Method getInverse() failed");
    CUAssertAlwaysLog(!test4.equals(test5),                         "Method getInverse() failed");
    CUAssertAlwaysLog(test5.equals(test1),                          "Method getInverse() failed");
    
    test1.set(-0.2f,0.1f,-0.1f,-0.2f);
    test4.set(test3);
    test5 = test4.getInverse();
    CUAssertAlwaysLog(test5.equals(test1),                          "Method getInverse() failed");
    test5 = test5.getInverse();
    CUAssertAlwaysLog(test5.equals(test3,CU_MATH_EPSILON),          "Method getInverse() failed");

    
#pragma mark Operator Test
    test1.set(2,2,2,2);
    test4 = test2;
    test4 += test2;
    CUAssertAlwaysLog(test4 == test1,                               "Addition operation failed");
    CUAssertAlwaysLog(test2 + test2 == test1,                       "Addition operation failed");
    
    test1.set(3,0,2,-1);
    test4 = test2;
    test4 += test3;
    CUAssertAlwaysLog(test4 == test1,                               "Addition operation failed");
    CUAssertAlwaysLog(test3 + test2 == test1,                       "Addition operation failed");
    
    test4 = test2;
    test4 -= test2;
    CUAssertAlwaysLog(test4 == Quaternion::ZERO,                    "Subtraction operation failed");
    CUAssertAlwaysLog(test2-test2 == Quaternion::ZERO,              "Subtraction operation failed");
    
    test1.set(-1,2,0,3);
    test4 = test2;
    test4 -= test3;
    CUAssertAlwaysLog(test4 == test1,                               "Subtraction operation failed");
    CUAssertAlwaysLog(test2-test3 == test1,                         "Subtraction operation failed");
    
    test4 = test3;
    test4 *= Quaternion::IDENTITY;
    CUAssertAlwaysLog(test3 == test4,                               "Multiplication operation failed");
    CUAssertAlwaysLog(test3*Quaternion::IDENTITY == test4,          "Multiplication operation failed");
    
    test1.set(10,-6,0,-2);
    test5.set(-1,2,0,3);
    test4 = test5;
    test4 *= test3;
    CUAssertAlwaysLog(test4 == test1,                               "Multiplication operation failed");
    CUAssertAlwaysLog(test5*test3 == test1,                         "Multiplication operation failed");

    test4 = test3;
    test4 /= Quaternion::IDENTITY;
    CUAssertAlwaysLog(test3 == test4,                               "Division operation failed");
    CUAssertAlwaysLog(test3*Quaternion::IDENTITY == test4,          "Division operation failed");
    
    test1.set(-1,2,0,3);
    test5.set(10,-6,0,-2);
    test4 = test5;
    test4 /= test3;
    CUAssertAlwaysLog(test4.equals(test1),                          "Division operation failed");
    CUAssertAlwaysLog(test1.equals(test5/test3),                    "Division operation failed");

    test1.set(4,-2,2,-4);
    test4 = test3;
    test4 *= 2;
    CUAssertAlwaysLog(test4 == test1,                               "Scaling operation failed");
    CUAssertAlwaysLog(test3*2 == test1,                             "Scaling operation failed");
    CUAssertAlwaysLog(2*test3 == test1,                             "Scaling operation failed");
    
    test4 *= 0;
    CUAssertAlwaysLog(test4 == Quaternion::ZERO,                    "Scaling operation failed");
    CUAssertAlwaysLog(test3*0 == Quaternion::ZERO,                  "Scaling operation failed");
    CUAssertAlwaysLog(0*test3 == Quaternion::ZERO,                  "Scaling operation failed");

    test1.set(1,-0.5f,0.5f,-1);
    test4 = test3;
    test4 /= 2;
    CUAssertAlwaysLog(test4 == test1,                               "Scaling operation failed");
    CUAssertAlwaysLog(test3/2 == test1,                             "Scaling operation failed");

    test1 = test3.getNegation();
    CUAssertAlwaysLog(-test3 == test1,                              "Negation operation failed");
    CUAssertAlwaysLog(-Quaternion::ZERO == Quaternion::ZERO,        "Negation operation failed");
    
    
#pragma mark Linear Attributes
    CUAssertAlwaysLog(Quaternion::ZERO.norm() == 0,             "Method norm() failed");
    CUAssertAlwaysLog(Quaternion::IDENTITY.norm() == 1,         "Method norm() failed");
    CUAssertAlwaysLog(Quaternion(-2,4,1,2).norm() == 5,         "Method norm() failed");
    
    CUAssertAlwaysLog(Quaternion::ZERO.normSquared() == 0,      "Method normSquared() failed");
    CUAssertAlwaysLog(Quaternion::IDENTITY.normSquared() == 1,  "Method normSquared() failed");
    CUAssertAlwaysLog(Quaternion(-2,4,1,2).normSquared() == 25, "Method normSquared() failed");

    
    CUAssertAlwaysLog(Quaternion::ZERO.isZero(),                "Method isZero() failed");
    CUAssertAlwaysLog(!Quaternion::IDENTITY.isZero(),           "Method isZero() failed");
    
    test1.set(0,0,CU_MATH_EPSILON*0.5,-CU_MATH_EPSILON*0.5);
    CUAssertAlwaysLog(Quaternion::ZERO.isNearZero(),            "Method isNearZero() failed");
    CUAssertAlwaysLog(test1.isNearZero(),                       "Method isNearZero() failed");
    CUAssertAlwaysLog(!Quaternion::IDENTITY.isNearZero(),       "Method isNearZero() failed");
    
    CUAssertAlwaysLog(!Quaternion::ZERO.isIdentity(),           "Method isIdentity() failed");
    CUAssertAlwaysLog(Quaternion::IDENTITY.isIdentity(),        "Method isIdentity() failed");

    test1.set(0,0,CU_MATH_EPSILON*0.5f,1-CU_MATH_EPSILON*0.5f);
    CUAssertAlwaysLog(!Quaternion::ZERO.isNearIdentity(),       "Method isNearIdentity() failed");
    CUAssertAlwaysLog(test1.isNearIdentity(),                   "Method isNearIdentity() failed");
    CUAssertAlwaysLog(Quaternion::IDENTITY.isNearIdentity(),    "Method isNearIdentity() failed");

    test1.set(1.0f/sqrtf(2.0f),0,0,1.0f/sqrtf(2.0f));
    test2.set(0.5f,0.5f,0.5f,0.5f);
    CUAssertAlwaysLog(!Quaternion::ZERO.isUnit(),               "Method isUnit() failed");
    CUAssertAlwaysLog(Quaternion::IDENTITY.isUnit(),            "Method isUnit() failed");
    CUAssertAlwaysLog(test1.isUnit(),                           "Method isUnit() failed");
    CUAssertAlwaysLog(!(2*test1).isUnit(),                      "Method isUnit() failed");
    CUAssertAlwaysLog(test2.isUnit(),                           "Method isUnit() failed");
    CUAssertAlwaysLog(!(2*test2).isUnit(),                      "Method isUnit() failed");
    
    test1.set(v3test,M_PI_4);
    CUAssertAlwaysLog(test1.isUnit(),                           "Method isUnit() failed");
    
    Vec3 v3other;
    test4.set(Vec3::UNIT_Z,(float)M_PI);
    value = test4.toAxisAngle(&v3other);
    CUAssertAlwaysLog(CU_MATH_APPROX(value, M_PI, CU_MATH_EPSILON),   "Method toAxisAngle() failed");
    CUAssertAlwaysLog(Vec3::UNIT_Z.equals(v3other),                     "Method toAxisAngle() failed");
    
    test4.set(v3test,M_PI_4);
    value = test4.toAxisAngle(&v3other);
    CUAssertAlwaysLog(CU_MATH_APPROX(value, M_PI_4, CU_MATH_EPSILON),    "Method toAxisAngle() failed");
    CUAssertAlwaysLog(v3other.equals(v3test.getNormalization()),        "Method toAxisAngle() failed");
    
    
#pragma mark Static Interpolation
    test1.set(1,1,1,1);
    test2.set(2,3,0,-1);
    testptr = Quaternion::lerp(test1, test2, 0, &test3);
    CUAssertAlwaysLog(testptr == &test3,                        "Quaternion::lerp() failed");
    CUAssertAlwaysLog(test3 == test1,                           "Quaternion::lerp() failed.");
    Quaternion::lerp(test1, test2, 1, &test3);
    CUAssertAlwaysLog(test3 == test2,                           "Quaternion::lerp() failed.");
    Quaternion::lerp(test1, test2, 0.5f, &test3);
    CUAssertAlwaysLog(test3 == Quaternion(1.5,2,0.5,0),         "Quaternion::lerp() failed.");
    Quaternion::lerp(test1, test2, 0.25f, &test3);
    CUAssertAlwaysLog(test3 == Quaternion(1.25,1.5,0.75,0.5),   "Quaternion::lerp() failed.");

    test1.set(Vec3::UNIT_Z,0);
    test2.set(Vec3::UNIT_Z,M_PI_2);
    test3.set(Vec3::UNIT_Z,M_PI_4);
    test4.set(Vec3::UNIT_Z,M_PI_4/2.0f);
    testptr = Quaternion::slerp(test1, test2, 0, &test5);
    CUAssertAlwaysLog(testptr == &test5,                        "Quaternion::slerp() failed");
    CUAssertAlwaysLog(test5.equals(test1),                      "Quaternion::slerp() failed.");
    Quaternion::slerp(test1, test2, 1, &test5);
    CUAssertAlwaysLog(test5.equals(test2),                      "Quaternion::slerp() failed.");
    Quaternion::slerp(test1, test2, 0.5f, &test5);
    CUAssertAlwaysLog(test5.equals(test3, CU_MATH_EPSILON),		"Quaternion::slerp() failed.");
    Quaternion::slerp(test1, test2, 0.25f, &test5);
    CUAssertAlwaysLog(test5.equals(test4, CU_MATH_EPSILON),		"Quaternion::slerp() failed.");

    test1.set(v3test,0);
    test2.set(v3test,M_PI_2);
    test3.set(v3test,M_PI_4);
    test4.set(v3test,M_PI_4/2.0f);
    testptr = Quaternion::slerp(test1, test2, 0, &test5);
    CUAssertAlwaysLog(test5.equals(test1),                      "Quaternion::slerp() failed.");
    Quaternion::slerp(test1, test2, 1, &test5);
    CUAssertAlwaysLog(test5.equals(test2),                      "Quaternion::slerp() failed.");
    Quaternion::slerp(test1, test2, 0.5f, &test5);
    CUAssertAlwaysLog(test5.equals(test3, CU_MATH_EPSILON),		"Quaternion::slerp() failed.");
    Quaternion::slerp(test1, test2, 0.25f, &test5);
    CUAssertAlwaysLog(test5.equals(test4, CU_MATH_EPSILON),		"Quaternion::slerp() failed.");

    test1.set(Vec3::UNIT_Z,0);
    test2.set(Vec3::UNIT_Z,M_PI_2);
    test3.set(Vec3::UNIT_Z,M_PI_4);
    test4.set(Vec3::UNIT_Z,M_PI_4/2.0f);
    testptr = Quaternion::nlerp(test1, test2, 0, &test5);
    CUAssertAlwaysLog(testptr == &test5,                        "Quaternion::nlerp() failed");
    CUAssertAlwaysLog(test5 == test1,                           "Quaternion::nlerp() failed.");
    Quaternion::nlerp(test1, test2, 1, &test5);
    CUAssertAlwaysLog(test5.equals(test2),                      "Quaternion::nlerp() failed.");
    Quaternion::nlerp(test1, test2, 0.5f, &test5);
    CUAssertAlwaysLog(test5.equals(test3, CU_MATH_EPSILON),		"Quaternion::nlerp() failed.");
    Quaternion::nlerp(test1, test2, 0.25f, &test5);
    CUAssertAlwaysLog(test5.equals(test4, 0.01f),				"Quaternion::nlerp() failed.");
    
    test1.set(v3test,0);
    test2.set(v3test,M_PI_2);
    test3.set(v3test,M_PI_4);
    test4.set(v3test,M_PI_4/2.0f);
    testptr = Quaternion::nlerp(test1, test2, 0, &test5);
    CUAssertAlwaysLog(test5 == test1,                           "Quaternion::nlerp() failed.");
    Quaternion::nlerp(test1, test2, 1, &test5);
    CUAssertAlwaysLog(test5.equals(test2),                      "Quaternion::nlerp() failed.");
    Quaternion::nlerp(test1, test2, 0.5f, &test5);
    CUAssertAlwaysLog(test5.equals(test3,CU_MATH_EPSILON),		"Quaternion::nlerp() failed.");
    Quaternion::nlerp(test1, test2, 0.25f, &test5);
    CUAssertAlwaysLog(test5.equals(test4,0.01f),                "Quaternion::nlerp() failed.");

    test1.set(Vec3::UNIT_Z,M_PI_2);
    v3test = Vec3::UNIT_X;
    Vec3* v3ptr = Quaternion::rotate(v3test, test1, &v3other);
    CUAssertAlwaysLog(v3ptr == &v3other,                            "Quaternion::rotate() failed.");
    CUAssertAlwaysLog(v3other.equals(Vec3::UNIT_Y,CU_MATH_EPSILON), "Quaternion::rotate() failed.");

    test1.set(Vec3::UNIT_X,M_PI_4);
    v3test = Vec3::ONE;
    Quaternion::rotate(v3test, test1, &v3other);
    CUAssertAlwaysLog(v3other.equals(Vec3(1,0,sqrtf(2.0f))),    "Quaternion::rotate() failed.");
    
    
#pragma mark Interpolation
    test1.set(1,1,1,1);
    test2.set(2,3,0,-1);
    test3 = test1;
    test3.lerp(test2, 0);
    CUAssertAlwaysLog(test3 == test1,                           "Method lerp() failed.");
    
    test3 = test1;
    test3.lerp(test2, 1);
    CUAssertAlwaysLog(test3 == test2,                           "Method lerp() failed.");

    test3 = test1;
    test3.lerp(test2, 0.5f);
    CUAssertAlwaysLog(test3 == Quaternion(1.5,2,0.5,0),         "Method lerp() failed.");
    
    test3 = test1;
    test3.lerp(test2, 0.25f);
    CUAssertAlwaysLog(test3 == Quaternion(1.25,1.5,0.75,0.5),   "Method lerp() failed.");
    
    test3 = test1.getLerp(test2,0);
    CUAssertAlwaysLog(test3 == test1,                           "Method getLerp() failed.");
    test3 = test1.getLerp(test2,1);
    CUAssertAlwaysLog(test3 == test2,                           "Method getLerp() failed.");
    CUAssertAlwaysLog(test3 != test1,                           "Method getLerp() failed.");
    test3 = test1.getLerp(test2, 0.5f);
    CUAssertAlwaysLog(test3 == Quaternion(1.5,2,0.5,0),         "Method getLerp() failed.");
    test3 = test1.getLerp(test2, 0.25f);
    CUAssertAlwaysLog(test3 == Quaternion(1.25,1.5,0.75,0.5),   "Method getLerp() failed.");

    v3test.set(1,2,1);
    test1.set(v3test,0);
    test2.set(v3test,M_PI_2);
    test3.set(v3test,M_PI_4);
    test4.set(v3test,M_PI_4/2.0f);
    
    test5 = test1;
    test5.slerp(test2,0);
    CUAssertAlwaysLog(test5.equals(test1),                      "Method slerp() failed.");

    test5 = test1;
    test5.slerp(test2,1);
    CUAssertAlwaysLog(test5.equals(test2),                      "Method slerp() failed.");
    
    test5 = test1;
    test5.slerp(test2,0.5f);
    CUAssertAlwaysLog(test5.equals(test3, CU_MATH_EPSILON),		"Method slerp() failed.");
    
    test5 = test1;
    test5.slerp(test2,0.25f);
    CUAssertAlwaysLog(test5.equals(test4, CU_MATH_EPSILON),		"Method slerp() failed.");
    
    test5 = test1.getSlerp(test2,0);
    CUAssertAlwaysLog(test5.equals(test1),                      "Method getSlerp() failed.");
    test5 = test1.getSlerp(test2,1);
    CUAssertAlwaysLog(test5.equals(test2),                      "Method getSlerp() failed.");
    CUAssertAlwaysLog(test5 != test1,                           "Method getSlerp() failed.");
    test5 = test1.getSlerp(test2, 0.5f);
    CUAssertAlwaysLog(test5.equals(test3, CU_MATH_EPSILON),		"Method getSlerp() failed.");
    test5 = test1.getSlerp(test2, 0.25f);
    CUAssertAlwaysLog(test5.equals(test4, CU_MATH_EPSILON),		"Method getSlerp() failed.");

    
    test5 = test1;
    test5.nlerp(test2,0);
    CUAssertAlwaysLog(test5.equals(test1),                      "Method nlerp() failed.");
    
    test5 = test1;
    test5.nlerp(test2,1);
    CUAssertAlwaysLog(test5.equals(test2),                      "Method nlerp() failed.");
    
    test5 = test1;
    test5.nlerp(test2,0.5f);
    CUAssertAlwaysLog(test5.equals(test3, CU_MATH_EPSILON),		"Method nlerp() failed.");
    
    test5 = test1;
    test5.nlerp(test2,0.25f);
    CUAssertAlwaysLog(test5.equals(test4,0.01f),                "Method nlerp() failed.");
    
    test5 = test1.getNlerp(test2,0);
    CUAssertAlwaysLog(test5.equals(test1),                      "Method getNlerp() failed.");
    test5 = test1.getNlerp(test2,1);
    CUAssertAlwaysLog(test5.equals(test2),                      "Method getNlerp() failed.");
    CUAssertAlwaysLog(test5 != test1,                           "Method getNlerp() failed.");
    test5 = test1.getNlerp(test2, 0.5f);
    CUAssertAlwaysLog(test5.equals(test3, CU_MATH_EPSILON),		"Method getNlerp() failed.");
    test5 = test1.getNlerp(test2, 0.25f);
    CUAssertAlwaysLog(test5.equals(test4,0.01f),                "Method getNlerp() failed.");
    
    test1.set(Vec3::UNIT_Z,M_PI_2);
    v3test = Vec3::UNIT_X;
    v3other = test1.getRotation(v3test);
    CUAssertAlwaysLog(v3other.equals(Vec3::UNIT_Y,CU_MATH_EPSILON),             "Method getRotation() failed.");
    
    v3other = v3test;
    v3other *= test1;
    CUAssertAlwaysLog(v3other.equals(Vec3::UNIT_Y,CU_MATH_EPSILON),             "Rotation operator failed.");
    CUAssertAlwaysLog((v3test * test1).equals(Vec3::UNIT_Y,CU_MATH_EPSILON),    "Rotation operator failed.");
    
    test1.set(Vec3::UNIT_X,M_PI_4);
    v3test = Vec3::ONE;
    Vec3 v3cmp(1,0,sqrtf(2.0f));
    v3other = test1.getRotation(v3test);
    CUAssertAlwaysLog(v3other.equals(v3cmp),                    "Method getRotation() failed.");

    v3other = v3test;
    v3other *= test1;
    CUAssertAlwaysLog(v3other.equals(v3cmp),                    "Rotation operator failed.");
    CUAssertAlwaysLog((v3test * test1).equals(v3cmp),           "Rotation operator failed.");

#pragma mark Conversion Test
    std::string str;
    std::string a, b, c, d;
    test1.set(2,3,-1.5,0.5); str = test1.toString();
    a = cugl::to_string(2.0f);  b = cugl::to_string(3.0f);
    c = cugl::to_string(-1.5f); d = cugl::to_string(0.5f);
    CUAssertAlwaysLog(str == ""+d+"+"+a+"i+"+b+"j+"+c+"k",      "Method toString() failed");
    str = test1.toString(true);
    CUAssertAlwaysLog(str == "cugl::Quaternion["+d+"+"+a+"i+"+b+"j+"+c+"k]",
                      "Method toString() failed");
    str = (std::string)test1;
    CUAssertAlwaysLog(str == ""+d+"+"+a+"i+"+b+"j+"+c+"k",      "String cast failed");
    
    test1.set(2,1,-1,-2);
    Vec4 v4test = (Vec4)test1;
    CUAssertAlwaysLog(v4test.x == 2 && v4test.y == 1 && v4test.z == -1 && v4test.w == -2,
                      "Vec4 cast failed");
    Vec4 test7(v4test);
    CUAssertAlwaysLog(test7 == test1,                           "Vec4 constructor failed");
    test6 = v4test;
    CUAssertAlwaysLog(test6 == test1,                           "Vec4 assignment failed");
    test5.set(v4test);
    CUAssertAlwaysLog(test5 == test1,                           "Vec4 assignment failed");
    
    // Delay matrix test to testMat4
    end.mark();
    CULog("Quaternion test took %llu micros",cugl::Timestamp::ellapsedMicros(start,end));

#pragma mark Complete
    CULog("Quaternion tests complete.\n");
}
    
    
#pragma mark -
#pragma mark Mat4
/**
 * Unit test for a 4x4 matrix (with homoengenous coordinate support)
 *
 * Thic class uses vector acceleration on select platforms.
 */
void cugl::testMat4() {
    CULog("Running tests for Mat4.\n");
    Timestamp start, end, globl;

#pragma mark Constructor Test
    // Includes the quaternion tests
    start.mark();
    globl.mark();
    Mat4 test1;
    CUAssertAlwaysLog(test1.m[0] == 1  && test1.m[1] == 0  && test1.m[2] == 0  && test1.m[3] == 0 &&
                      test1.m[4] == 0  && test1.m[5] == 1  && test1.m[6] == 0  && test1.m[7] == 0 &&
                      test1.m[8] == 0  && test1.m[9] == 0  && test1.m[10] == 1 && test1.m[11] == 0 &&
                      test1.m[12] == 0 && test1.m[13] == 0 && test1.m[14] == 0 && test1.m[15] == 1,
                      "Trivial constructor failed");
    
    Mat4 test2(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16);
    CUAssertAlwaysLog(test2.m[0] == 1  && test2.m[1] == 5  && test2.m[2] == 9  && test2.m[3] == 13 &&
                      test2.m[4] == 2  && test2.m[5] == 6  && test2.m[6] == 10 && test2.m[7] == 14 &&
                      test2.m[8] == 3  && test2.m[9] == 7  && test2.m[10]== 11 && test2.m[11]== 15 &&
                      test2.m[12]== 4  && test2.m[13]== 8  && test2.m[14]== 12 && test2.m[15]== 16,
                      "Initialization constructor failed");
    
    float f[16] = {16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1};
    Mat4 test3(f);
    CUAssertAlwaysLog(test3.m[0] == 16 && test3.m[1] == 15 && test3.m[2] == 14 && test3.m[3] == 13 &&
                      test3.m[4] == 12 && test3.m[5] == 11 && test3.m[6] == 10 && test3.m[7] == 9  &&
                      test3.m[8] == 8  && test3.m[9] == 7  && test3.m[10] == 6 && test3.m[11] == 5 &&
                      test3.m[12] == 4 && test3.m[13] == 3 && test3.m[14] == 2 && test3.m[15] == 1,
                      "Array constructor failed");
    
    Mat4 test4(test2);
    CUAssertAlwaysLog(test4.m[0] == test2.m[0] && test4.m[1] == test2.m[1] && test4.m[2] == test2.m[2] && test4.m[3] == test2.m[3] &&
                      test4.m[4] == test2.m[4] && test4.m[5] == test2.m[5] && test4.m[6] == test2.m[6] && test4.m[7] == test2.m[7] &&
                      test4.m[8] == test2.m[8] && test4.m[9] == test2.m[9] && test4.m[10]==test2.m[10] && test4.m[11]==test2.m[11] &&
                      test4.m[12]==test2.m[12] && test4.m[13]==test2.m[13] && test4.m[14]==test2.m[14] && test4.m[15]==test2.m[15],
                      "Copy constructor failed");
    
    Quaternion qtest(Vec3::UNIT_Z,M_PI_2);
    Mat4 test5(qtest);
    CUAssertAlwaysLog(CU_MATH_APPROX(test5.m[0],  0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.m[1],  1.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.m[2],  0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.m[3],  0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.m[4], -1.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.m[5],  0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.m[6],  0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.m[7],  0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.m[8],  0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.m[9],  0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.m[10], 1.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.m[11], 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.m[12], 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.m[13], 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.m[14], 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.m[15], 1.0f, CU_MATH_EPSILON),
                      "Rotational constructor failed");
    
    qtest.set(Vec3::UNIT_X,M_PI_4);
    Mat4 test6(qtest);
    CUAssertAlwaysLog(CU_MATH_APPROX(test6.m[0],  1.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test6.m[1],  0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test6.m[2],  0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test6.m[3],  0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test6.m[4],  0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test6.m[5],  1.0f/sqrtf(2.0f), CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test6.m[6],  1.0f/sqrtf(2.0f), CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test6.m[7],  0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test6.m[8],  0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test6.m[9],  -1.0f/sqrtf(2.0f), CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test6.m[10], 1.0f/sqrtf(2.0f),  CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test6.m[11], 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test6.m[12], 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test6.m[13], 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test6.m[14], 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test6.m[15], 1.0f, CU_MATH_EPSILON),
                      "Rotational constructor failed");
    end.mark();
    CULog("Constructor test took %llu micros",cugl::Timestamp::ellapsedMicros(start,end));

#pragma mark Static Constructor Test
    start.mark();
    Mat4 test7;
    Mat4* testptr;
    testptr = Mat4::createLookAt(Vec3::ZERO,Vec3::ONE,Vec3::UNIT_Z,&test7);
    CUAssertAlwaysLog(testptr == &test7,    "Look-at constructor failed");
    CUAssertAlwaysLog(CU_MATH_APPROX(test7.m[0],  1.0f/sqrt(2.0f),     CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[1], -1.0f/sqrtf(6.0f),    CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[2], -1.0f/sqrtf(3.0f),    CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[3],  0.0f,                CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[4], -1.0f/sqrt(2.0f),     CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[5], -1.0f/sqrtf(6.0f),    CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[6], -1.0f/sqrtf(3.0f),    CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[7],  0.0f,                CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[8],  0.0f,                CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[9],  2.0f/sqrtf(6.0f),    CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[10],-1.0f/sqrtf(3.0f),    CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[11], 0.0f,                CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[12], 0.0f,                CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[13], 0.0f,                CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[14], 0.0f,                CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[15], 1.0f,                CU_MATH_EPSILON),
                      "Look-at constructor failed");

    testptr = Mat4::createLookAt(1,1,1,0,0,0,0,1,0,&test7);
    CUAssertAlwaysLog(testptr == &test7,    "Look-at constructor failed");
    CUAssertAlwaysLog(CU_MATH_APPROX(test7.m[0],  1.0f/sqrt(2.0f),     CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[1], -1.0f/sqrtf(6.0f),    CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[2], 1.0f/sqrtf(3.0f),     CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[3],  0.0f,                CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[4],  0.0f,                CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[5],  2.0f/sqrtf(6.0f),    CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[6],  1.0f/sqrtf(3.0f),    CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[7],  0.0f,                CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[8], -1.0f/sqrtf(2.0f),    CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[9], -1.0f/sqrtf(6.0f),    CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[10], 1.0f/sqrtf(3.0f),    CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[11], 0.0f,                CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[12], 0.0f,                CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[13], 0.0f,                CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[14], -sqrt(3.0f),         CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[15], 1.0f,                CU_MATH_EPSILON),
                      "Look-at constructor failed");

    testptr = Mat4::createPerspective(90.0f,0.5f,10,-10,&test7);
    CUAssertAlwaysLog(testptr == &test7,    "Perspective constructor failed");
    CUAssertAlwaysLog(test7.m[0] == 2 && test7.m[1] == 0 && test7.m[2] == 0  && test7.m[3] == 0 &&
                      test7.m[4] == 0 && test7.m[5] == 1 && test7.m[6] == 0  && test7.m[7] == 0 &&
                      test7.m[8] == 0 && test7.m[9] == 0 && test7.m[10]== 0  && test7.m[11]==-1 &&
                      test7.m[12]== 0 && test7.m[13]== 0 && test7.m[14]==-10 && test7.m[15]== 0,
                      "Perspective constructor failed");

    testptr = Mat4::createPerspective(45,1,1,-1,&test7);
    CUAssertAlwaysLog(testptr == &test7,    "Look-at constructor failed");
    CUAssertAlwaysLog(CU_MATH_APPROX(test7.m[0], 1+sqrtf(2.0f),CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[1], 0.0f,         CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[2], 0.0f,         CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[3], 0.0f,         CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[4], 0.0f,         CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[5], 1+sqrtf(2.0f),CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[6], 0.0f,         CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[7], 0.0f,         CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[8], 0.0f,         CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[9], 0.0f,         CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[10], 0.0f,        CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[11], -1.0f,       CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[12], 0.0f,        CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[13], 0.0f,        CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[14], -1.0f,       CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[15], 0.0f,        CU_MATH_EPSILON),
                      "Perspective constructor failed");
    
    testptr = Mat4::createOrthographic(100.0f,200.0f,10,-10,&test7);
    CUAssertAlwaysLog(testptr == &test7,    "Orthographic constructor failed");
    CUAssertAlwaysLog(CU_MATH_APPROX(test7.m[0], 0.02f,        CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[1], 0.0f,         CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[2], 0.0f,         CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[3], 0.0f,         CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[4], 0.0f,         CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[5], 0.01f,        CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[6], 0.0f,         CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[7], 0.0f,         CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[8], 0.0f,         CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[9], 0.0f,         CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[10], 0.1f,        CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[11], 0.0f,        CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[12], 0.0f,        CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[13], 0.0f,        CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[14], 0.0f,        CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[15], 1.0f,        CU_MATH_EPSILON),
                      "Perspective constructor failed");

    testptr = Mat4::createOrthographicOffCenter(50.0f,150.0f,100.0f,300.0f,10,-10,&test7);
    CUAssertAlwaysLog(testptr == &test7,    "Orthographic constructor failed");
    CUAssertAlwaysLog(CU_MATH_APPROX(test7.m[0], 0.02f,        CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[1], 0.0f,         CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[2], 0.0f,         CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[3], 0.0f,         CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[4], 0.0f,         CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[5], 0.01f,        CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[6], 0.0f,         CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[7], 0.0f,         CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[8], 0.0f,         CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[9], 0.0f,         CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[10], 0.1f,        CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[11], 0.0f,        CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[12],-2.0f,        CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[13],-2.0f,        CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[14], 0.0f,        CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[15], 1.0f,        CU_MATH_EPSILON),
                      "Perspective constructor failed");
    
    testptr = Mat4::createScale(2.0f,&test7);
    CUAssertAlwaysLog(testptr == &test7,    "Scale constructor failed");
    CUAssertAlwaysLog(test7.m[0] == 2 && test7.m[1] == 0 && test7.m[2] == 0 && test7.m[3] == 0 &&
                      test7.m[4] == 0 && test7.m[5] == 2 && test7.m[6] == 0 && test7.m[7] == 0 &&
                      test7.m[8] == 0 && test7.m[9] == 0 && test7.m[10]== 2 && test7.m[11]== 0 &&
                      test7.m[12]== 0 && test7.m[13]== 0 && test7.m[14]== 0 && test7.m[15]== 1,
                      "Scale constructor failed");

    Mat4::createLookAt(Vec3::ZERO,Vec3::ONE,Vec3::UNIT_Z,&test7); // To scramble data
    testptr = Mat4::createScale(3.0f,4.0f,5.0f,&test7);
    CUAssertAlwaysLog(testptr == &test7,    "Scale constructor failed");
    CUAssertAlwaysLog(test7.m[0] == 3 && test7.m[1] == 0 && test7.m[2] == 0 && test7.m[3] == 0 &&
                      test7.m[4] == 0 && test7.m[5] == 4 && test7.m[6] == 0 && test7.m[7] == 0 &&
                      test7.m[8] == 0 && test7.m[9] == 0 && test7.m[10]== 5 && test7.m[11]== 0 &&
                      test7.m[12]== 0 && test7.m[13]== 0 && test7.m[14]== 0 && test7.m[15]== 1,
                      "Scale constructor failed");

    Mat4::createLookAt(Vec3::ZERO,Vec3::ONE,Vec3::UNIT_Z,&test7); // To scramble data
    testptr = Mat4::createScale(Vec3(6.0f,7.0f,8.0f),&test7);
    CUAssertAlwaysLog(testptr == &test7,    "Scale constructor failed");
    CUAssertAlwaysLog(test7.m[0] == 6 && test7.m[1] == 0 && test7.m[2] == 0 && test7.m[3] == 0 &&
                      test7.m[4] == 0 && test7.m[5] == 7 && test7.m[6] == 0 && test7.m[7] == 0 &&
                      test7.m[8] == 0 && test7.m[9] == 0 && test7.m[10]== 8 && test7.m[11]== 0 &&
                      test7.m[12]== 0 && test7.m[13]== 0 && test7.m[14]== 0 && test7.m[15]== 1,
                      "Scale constructor failed");

    Mat4::createLookAt(Vec3::ZERO,Vec3::ONE,Vec3::UNIT_Z,&test7); // To scramble data
    testptr = Mat4::createRotation(qtest,&test7);
    CUAssertAlwaysLog(testptr == &test7,    "Rotation constructor failed");
    CUAssertAlwaysLog(CU_MATH_APPROX(test7.m[0],  1.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[1],  0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[2],  0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[3],  0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[4],  0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[5],  1.0f/sqrtf(2.0f), CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[6],  1.0f/sqrtf(2.0f), CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[7],  0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[8],  0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[9],  -1.0f/sqrtf(2.0f), CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[10], 1.0f/sqrtf(2.0f),  CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[11], 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[12], 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[13], 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[14], 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[15], 1.0f, CU_MATH_EPSILON),
                      "Rotation constructor failed");
    
    Mat4::createLookAt(Vec3::ZERO,Vec3::ONE,Vec3::UNIT_Z,&test7); // To scramble data
    testptr = Mat4::createRotation(Vec3::UNIT_X,M_PI_4,&test7);
    CUAssertAlwaysLog(testptr == &test7,    "Rotation constructor failed");
    CUAssertAlwaysLog(CU_MATH_APPROX(test7.m[0],  1.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[1],  0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[2],  0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[3],  0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[4],  0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[5],  1.0f/sqrtf(2.0f), CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[6],  1.0f/sqrtf(2.0f), CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[7],  0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[8],  0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[9],  -1.0f/sqrtf(2.0f), CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[10], 1.0f/sqrtf(2.0f),  CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[11], 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[12], 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[13], 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[14], 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[15], 1.0f, CU_MATH_EPSILON),
                      "Rotation constructor failed");

    Mat4::createLookAt(Vec3::ZERO,Vec3::ONE,Vec3::UNIT_Z,&test7); // To scramble data
    testptr = Mat4::createRotationX(M_PI_4,&test7);
    CUAssertAlwaysLog(testptr == &test7,    "Rotation constructor failed");
    CUAssertAlwaysLog(CU_MATH_APPROX(test7.m[0],  1.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[1],  0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[2],  0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[3],  0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[4],  0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[5],  1.0f/sqrtf(2.0f), CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[6],  1.0f/sqrtf(2.0f), CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[7],  0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[8],  0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[9],  -1.0f/sqrtf(2.0f), CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[10], 1.0f/sqrtf(2.0f),  CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[11], 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[12], 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[13], 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[14], 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[15], 1.0f, CU_MATH_EPSILON),
                      "RotationX constructor failed");

    Mat4::createLookAt(Vec3::ZERO,Vec3::ONE,Vec3::UNIT_Z,&test7); // To scramble data
    testptr = Mat4::createRotationY(M_PI_4,&test7);
    CUAssertAlwaysLog(testptr == &test7,    "Rotation constructor failed");
    CUAssertAlwaysLog(CU_MATH_APPROX(test7.m[0],  1.0f/sqrtf(2.0f), CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[1],  0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[2], -1.0f/sqrtf(2.0f), CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[3],  0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[4],  0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[5],  1.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[6],  0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[7],  0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[8],  1.0f/sqrtf(2.0f),  CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[9],  0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[10], 1.0f/sqrtf(2.0f),  CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[11], 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[12], 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[13], 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[14], 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[15], 1.0f, CU_MATH_EPSILON),
                      "RotationY constructor failed");

    Mat4::createLookAt(Vec3::ZERO,Vec3::ONE,Vec3::UNIT_Z,&test7); // To scramble data
    testptr = Mat4::createRotationZ(M_PI_4,&test7);
    CUAssertAlwaysLog(testptr == &test7,    "Rotation constructor failed");
    CUAssertAlwaysLog(CU_MATH_APPROX(test7.m[0],  1.0f/sqrtf(2.0f), CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[1],  1.0f/sqrtf(2.0f), CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[2],  0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[3],  0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[4], -1.0f/sqrtf(2.0f), CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[5],  1.0f/sqrtf(2.0f), CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[6],  0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[7],  0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[8],  0.0f,  CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[9],  0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[10], 1.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[11], 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[12], 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[13], 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[14], 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test7.m[15], 1.0f, CU_MATH_EPSILON),
                      "RotationZ constructor failed");

    Mat4::createLookAt(Vec3::ZERO,Vec3::ONE,Vec3::UNIT_Z,&test7); // To scramble data
    testptr = Mat4::createTranslation(3.0f,4.0f,5.0f,&test7);
    CUAssertAlwaysLog(testptr == &test7,    "Translation constructor failed");
    CUAssertAlwaysLog(test7.m[0] == 1 && test7.m[1] == 0 && test7.m[2] == 0 && test7.m[3] == 0 &&
                      test7.m[4] == 0 && test7.m[5] == 1 && test7.m[6] == 0 && test7.m[7] == 0 &&
                      test7.m[8] == 0 && test7.m[9] == 0 && test7.m[10]== 1 && test7.m[11]== 0 &&
                      test7.m[12]== 3 && test7.m[13]== 4 && test7.m[14]== 5 && test7.m[15]== 1,
                      "Translation constructor failed");
    
    Mat4::createLookAt(Vec3::ZERO,Vec3::ONE,Vec3::UNIT_Z,&test7); // To scramble data
    testptr = Mat4::createTranslation(Vec3(6.0f,7.0f,8.0f),&test7);
    CUAssertAlwaysLog(testptr == &test7,    "Translation constructor failed");
    CUAssertAlwaysLog(test7.m[0] == 1 && test7.m[1] == 0 && test7.m[2] == 0 && test7.m[3] == 0 &&
                      test7.m[4] == 0 && test7.m[5] == 1 && test7.m[6] == 0 && test7.m[7] == 0 &&
                      test7.m[8] == 0 && test7.m[9] == 0 && test7.m[10]== 1 && test7.m[11]== 0 &&
                      test7.m[12]== 6 && test7.m[13]== 7 && test7.m[14]== 8 && test7.m[15]== 1,
                      "Translation constructor failed");
    end.mark();
    CULog("Static constructor test took %llu micros",cugl::Timestamp::ellapsedMicros(start,end));

#pragma mark Constants Test
    start.mark();
    CUAssertAlwaysLog(Mat4::IDENTITY.m[0] == 1  && Mat4::IDENTITY.m[1] == 0  && Mat4::IDENTITY.m[2] == 0  && Mat4::IDENTITY.m[3] == 0 &&
                      Mat4::IDENTITY.m[4] == 0  && Mat4::IDENTITY.m[5] == 1  && Mat4::IDENTITY.m[6] == 0  && Mat4::IDENTITY.m[7] == 0 &&
                      Mat4::IDENTITY.m[8] == 0  && Mat4::IDENTITY.m[9] == 0  && Mat4::IDENTITY.m[10]== 1  && Mat4::IDENTITY.m[11]== 0 &&
                      Mat4::IDENTITY.m[12]== 0  && Mat4::IDENTITY.m[13]== 0  && Mat4::IDENTITY.m[14]== 0  && Mat4::IDENTITY.m[15]== 1,
                      "Identity matrix failed");
    
    CUAssertAlwaysLog(Mat4::ZERO.m[0] == 0  && Mat4::ZERO.m[1] == 0  && Mat4::ZERO.m[2] == 0  && Mat4::ZERO.m[3] == 0 &&
                      Mat4::ZERO.m[4] == 0  && Mat4::ZERO.m[5] == 0  && Mat4::ZERO.m[6] == 0  && Mat4::ZERO.m[7] == 0 &&
                      Mat4::ZERO.m[8] == 0  && Mat4::ZERO.m[9] == 0  && Mat4::ZERO.m[10]== 0  && Mat4::ZERO.m[11]== 0 &&
                      Mat4::ZERO.m[12]== 0  && Mat4::ZERO.m[13]== 0  && Mat4::ZERO.m[14]== 0  && Mat4::ZERO.m[15]== 0,
                      "Zero matrix failed");
    
    CUAssertAlwaysLog(Mat4::ONE.m[0] == 1  && Mat4::ONE.m[1] == 1  && Mat4::ONE.m[2] == 1  && Mat4::ONE.m[3] == 1 &&
                      Mat4::ONE.m[4] == 1  && Mat4::ONE.m[5] == 1  && Mat4::ONE.m[6] == 1  && Mat4::ONE.m[7] == 1 &&
                      Mat4::ONE.m[8] == 1  && Mat4::ONE.m[9] == 1  && Mat4::ONE.m[10]== 1  && Mat4::ONE.m[11]== 1 &&
                      Mat4::ONE.m[12]== 1  && Mat4::ONE.m[13]== 1  && Mat4::ONE.m[14]== 1  && Mat4::ONE.m[15]== 1,
                      "Ones matrix failed");
    end.mark();
    CULog("Constants test took %llu micros",cugl::Timestamp::ellapsedMicros(start,end));

#pragma mark Setter Test
    start.mark();
    test1 = test2;
    CUAssertAlwaysLog(test1.m[0] == 1  && test1.m[1] == 5  && test1.m[2] == 9  && test1.m[3] == 13 &&
                      test1.m[4] == 2  && test1.m[5] == 6  && test1.m[6] == 10 && test1.m[7] == 14 &&
                      test1.m[8] == 3  && test1.m[9] == 7  && test1.m[10]== 11 && test1.m[11]== 15 &&
                      test1.m[12]== 4  && test1.m[13]== 8  && test1.m[14]== 12 && test1.m[15]== 16,
                      "Basic assignment failed");
    
    test1 = f;
    CUAssertAlwaysLog(test1.m[0] == 16 && test1.m[1] == 15 && test1.m[2] == 14 && test1.m[3] == 13 &&
                      test1.m[4] == 12 && test1.m[5] == 11 && test1.m[6] == 10 && test1.m[7] == 9  &&
                      test1.m[8] == 8  && test1.m[9] == 7  && test1.m[10] == 6 && test1.m[11] == 5 &&
                      test1.m[12] == 4 && test1.m[13] == 3 && test1.m[14] == 2 && test1.m[15] == 1,
                      "Float assignment failed");
    
    test1 = qtest;
    CUAssertAlwaysLog(CU_MATH_APPROX(test1.m[0],  1.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test1.m[1],  0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test1.m[2],  0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test1.m[3],  0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test1.m[4],  0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test1.m[5],  1.0f/sqrtf(2.0f), CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test1.m[6],  1.0f/sqrtf(2.0f), CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test1.m[7],  0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test1.m[8],  0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test1.m[9],  -1.0f/sqrtf(2.0f), CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test1.m[10], 1.0f/sqrtf(2.0f),  CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test1.m[11], 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test1.m[12], 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test1.m[13], 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test1.m[14], 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test1.m[15], 1.0f, CU_MATH_EPSILON),
                      "Quaternion assignment failed");
    
    test1.set(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16);
    CUAssertAlwaysLog(test1.m[0] == 1  && test1.m[1] == 5  && test1.m[2] == 9  && test1.m[3] == 13 &&
                      test1.m[4] == 2  && test1.m[5] == 6  && test1.m[6] == 10 && test1.m[7] == 14 &&
                      test1.m[8] == 3  && test1.m[9] == 7  && test1.m[10]== 11 && test1.m[11]== 15 &&
                      test1.m[12]== 4  && test1.m[13]== 8  && test1.m[14]== 12 && test1.m[15]== 16,
                      "Parameter assignment failed");
    
    test1.set(f);
    CUAssertAlwaysLog(test1.m[0] == 16 && test1.m[1] == 15 && test1.m[2] == 14 && test1.m[3] == 13 &&
                      test1.m[4] == 12 && test1.m[5] == 11 && test1.m[6] == 10 && test1.m[7] == 9  &&
                      test1.m[8] == 8  && test1.m[9] == 7  && test1.m[10] == 6 && test1.m[11] == 5 &&
                      test1.m[12] == 4 && test1.m[13] == 3 && test1.m[14] == 2 && test1.m[15] == 1,
                      "Alternate float assignment failed");

    test1.set(test2);
    CUAssertAlwaysLog(test1.m[0] == 1  && test1.m[1] == 5  && test1.m[2] == 9  && test1.m[3] == 13 &&
                      test1.m[4] == 2  && test1.m[5] == 6  && test1.m[6] == 10 && test1.m[7] == 14 &&
                      test1.m[8] == 3  && test1.m[9] == 7  && test1.m[10]== 11 && test1.m[11]== 15 &&
                      test1.m[12]== 4  && test1.m[13]== 8  && test1.m[14]== 12 && test1.m[15]== 16,
                      "Alternate assignment failed");

    test1.set(qtest);
    CUAssertAlwaysLog(CU_MATH_APPROX(test1.m[0],  1.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test1.m[1],  0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test1.m[2],  0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test1.m[3],  0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test1.m[4],  0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test1.m[5],  1.0f/sqrtf(2.0f), CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test1.m[6],  1.0f/sqrtf(2.0f), CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test1.m[7],  0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test1.m[8],  0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test1.m[9],  -1.0f/sqrtf(2.0f), CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test1.m[10], 1.0f/sqrtf(2.0f),  CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test1.m[11], 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test1.m[12], 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test1.m[13], 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test1.m[14], 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test1.m[15], 1.0f, CU_MATH_EPSILON),
                      "Alternative quaternion assignment failed");

    test1.setZero();
    CUAssertAlwaysLog(test1.m[0] == 0  && test1.m[1] == 0  && test1.m[2] == 0  && test1.m[3] == 0 &&
                      test1.m[4] == 0  && test1.m[5] == 0  && test1.m[6] == 0  && test1.m[7] == 0 &&
                      test1.m[8] == 0  && test1.m[9] == 0  && test1.m[10]== 0  && test1.m[11]== 0 &&
                      test1.m[12]== 0  && test1.m[13]== 0  && test1.m[14]== 0  && test1.m[15]== 0,
                      "Erasing assignment  failed");

    test1 = test2;
    test1.setIdentity();
    CUAssertAlwaysLog(test1.m[0] == 1  && test1.m[1] == 0  && test1.m[2] == 0  && test1.m[3] == 0 &&
                      test1.m[4] == 0  && test1.m[5] == 1  && test1.m[6] == 0  && test1.m[7] == 0 &&
                      test1.m[8] == 0  && test1.m[9] == 0  && test1.m[10]== 1  && test1.m[11]== 0 &&
                      test1.m[12]== 0  && test1.m[13]== 0  && test1.m[14]== 0  && test1.m[15]== 1,
                      "Identity assignment  failed");
    end.mark();
    CULog("Setter test took %llu micros",cugl::Timestamp::ellapsedMicros(start,end));

#pragma mark Quaternion Test
    start.mark();
    Mat4::createRotation(qtest,&test6);
    Mat4::createRotationX(M_PI_4,&test7);
    
    Quaternion qother(test7);
    CUAssertAlwaysLog(qtest.equals(qother),     "Quaternion matrix constructor failed");

    qother.setZero();
    Quaternion* qptr = Quaternion::createFromRotationMatrix(test7, &qother);
    CUAssertAlwaysLog(qptr == &qother,          "Quaternion matrix constructor failed");
    CUAssertAlwaysLog(qtest.equals(qother),     "Quaternion matrix constructor failed");

    qother.setZero();
    qother = test7;
    CUAssertAlwaysLog(qtest.equals(qother),     "Quaternion matrix assignment failed");

    qother.setZero();
    qother.set(test7);
    CUAssertAlwaysLog(qtest.equals(qother),     "Alternate quaternion matrix assignment failed");
    
    test7 = (Mat4)qtest;
    CUAssertAlwaysLog(test6.equals(test7),      "Quaternion cast to matrix failed");
    
    end.mark();
    CULog("Quaternion test took %llu micros",cugl::Timestamp::ellapsedMicros(start,end));

#pragma mark Comparison Test
    start.mark();
    Mat4::createRotationX(M_PI_4,&test7);
    CUAssertAlwaysLog(test2.isExactly(test2),   "Method isExactly() failed");
    CUAssertAlwaysLog(test2.isExactly(test4),   "Method isExactly() failed");
    CUAssertAlwaysLog(!test2.isExactly(test3),  "Method isExactly() failed");
    CUAssertAlwaysLog(!test6.isExactly(test7),  "Method isExactly() failed");

    CUAssertAlwaysLog(test2.equals(test2),      "Method equals() failed");
    CUAssertAlwaysLog(test2.equals(test4),      "Method equals() failed");
    CUAssertAlwaysLog(!test2.equals(test3),     "Method equals() failed");
    CUAssertAlwaysLog(test6.equals(test7),      "Method equals() failed");

    CUAssertAlwaysLog(test2 == test2,           "Equals failed");
    CUAssertAlwaysLog(test2 == test4,           "Equals failed");
    CUAssertAlwaysLog(!(test2 == test3),        "Equals failed");
    CUAssertAlwaysLog(!(test6 == test7),        "Equals failed");

    CUAssertAlwaysLog(!(test2 != test2),        "Not equals failed");
    CUAssertAlwaysLog(!(test2 != test4),        "Not equals failed");
    CUAssertAlwaysLog(test2 != test3,           "Not equals failed");
    CUAssertAlwaysLog(test6 != test7,           "Not equals failed");
    end.mark();
    CULog("Comparison test took %llu micros",cugl::Timestamp::ellapsedMicros(start,end));

#pragma mark Static Arithmetic Test
    start.mark();
    Mat4::createScale(2,3,4,&test1);
    Mat4::createTranslation(5,6,7,&test2);
    Mat4::createRotation(qtest,&test3);
    test4.set(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16);
    test6.set(1,0,0,1,1,1,0,0,0,0,1,0,0,1,1,0);

    testptr = Mat4::add(Mat4::ONE,2,&test5);
    CUAssertAlwaysLog(testptr == &test5,        "Mat4::add() failed");
    CUAssertAlwaysLog(test5.m[0] == 3.0f  && test5.m[1] == 3.0f  && test5.m[2] == 3.0f && test5.m[3] == 3.0f &&
                      test5.m[4] == 3.0f  && test5.m[5] == 3.0f  && test5.m[6] == 3.0f && test5.m[7] == 3.0f &&
                      test5.m[8] == 3.0f  && test5.m[9] == 3.0f  && test5.m[10]== 3.0f && test5.m[11]== 3.0f &&
                      test5.m[12]== 3.0f  && test5.m[13]== 3.0f  && test5.m[14]== 3.0f && test5.m[15]== 3.0f,
                      "Mat4::add() failed");

    testptr = Mat4::add(Mat4::ONE,test4,&test5);
    CUAssertAlwaysLog(testptr == &test5,        "Mat4::add() failed");
    CUAssertAlwaysLog(test5.m[0] == 2.0f  && test5.m[1] == 6.0f  && test5.m[2] == 10.0f && test5.m[3] == 14.0f &&
                      test5.m[4] == 3.0f  && test5.m[5] == 7.0f  && test5.m[6] == 11.0f && test5.m[7] == 15.0f &&
                      test5.m[8] == 4.0f  && test5.m[9] == 8.0f  && test5.m[10]== 12.0f && test5.m[11]== 16.0f &&
                      test5.m[12]== 5.0f  && test5.m[13]== 9.0f  && test5.m[14]== 13.0f && test5.m[15]== 17.0f,
                      "Mat4::add() failed");
    Mat4::add(test4,Mat4::ONE,&test5);
    CUAssertAlwaysLog(test5.m[0] == 2.0f  && test5.m[1] == 6.0f  && test5.m[2] == 10.0f && test5.m[3] == 14.0f &&
                      test5.m[4] == 3.0f  && test5.m[5] == 7.0f  && test5.m[6] == 11.0f && test5.m[7] == 15.0f &&
                      test5.m[8] == 4.0f  && test5.m[9] == 8.0f  && test5.m[10]== 12.0f && test5.m[11]== 16.0f &&
                      test5.m[12]== 5.0f  && test5.m[13]== 9.0f  && test5.m[14]== 13.0f && test5.m[15]== 17.0f,
                      "Mat4::add() failed");
    Mat4::add(test4,test4,&test5);
    CUAssertAlwaysLog(test5.m[0] == 2.0f  && test5.m[1] == 10.0f && test5.m[2] == 18.0f && test5.m[3] == 26.0f &&
                      test5.m[4] == 4.0f  && test5.m[5] == 12.0f && test5.m[6] == 20.0f && test5.m[7] == 28.0f &&
                      test5.m[8] == 6.0f  && test5.m[9] == 14.0f && test5.m[10]== 22.0f && test5.m[11]== 30.0f &&
                      test5.m[12]== 8.0f  && test5.m[13]== 16.0f && test5.m[14]== 24.0f && test5.m[15]== 32.0f,
                      "Mat4::add() failed");
    
    testptr = Mat4::subtract(Mat4::ONE,2,&test5);
    CUAssertAlwaysLog(testptr == &test5,        "Mat4::subtract() failed");
    CUAssertAlwaysLog(test5.m[0] == -1.0f && test5.m[1] == -1.0f && test5.m[2] == -1.0f && test5.m[3] == -1.0f &&
                      test5.m[4] == -1.0f && test5.m[5] == -1.0f && test5.m[6] == -1.0f && test5.m[7] == -1.0f &&
                      test5.m[8] == -1.0f && test5.m[9] == -1.0f && test5.m[10]== -1.0f && test5.m[11]== -1.0f &&
                      test5.m[12]== -1.0f && test5.m[13]== -1.0f && test5.m[14]== -1.0f && test5.m[15]== -1.0f,
                      "Mat4::add() failed");
    
    testptr = Mat4::subtract(Mat4::ONE,test4,&test5);
    CUAssertAlwaysLog(testptr == &test5,        "Mat4::subtract() failed");
    CUAssertAlwaysLog(test5.m[0] == 0.0f  && test5.m[1] == -4.0f && test5.m[2] == -8.0f && test5.m[3] ==-12.0f &&
                      test5.m[4] == -1.0f && test5.m[5] == -5.0f && test5.m[6] == -9.0f && test5.m[7] ==-13.0f &&
                      test5.m[8] == -2.0f && test5.m[9] == -6.0f && test5.m[10]==-10.0f && test5.m[11]==-14.0f &&
                      test5.m[12]== -3.0f && test5.m[13]== -7.0f && test5.m[14]==-11.0f && test5.m[15]==-15.0f,
                      "Mat4::subtract() failed");
    Mat4::subtract(test4,Mat4::ONE,&test5);
    CUAssertAlwaysLog(test5.m[0] == 0.0f  && test5.m[1] == 4.0f && test5.m[2] == 8.0f  && test5.m[3] == 12.0f &&
                      test5.m[4] == 1.0f  && test5.m[5] == 5.0f && test5.m[6] == 9.0f  && test5.m[7] == 13.0f &&
                      test5.m[8] == 2.0f  && test5.m[9] == 6.0f && test5.m[10]== 10.0f && test5.m[11]== 14.0f &&
                      test5.m[12]== 3.0f  && test5.m[13]== 7.0f && test5.m[14]== 11.0f && test5.m[15]== 15.0f,
                      "Mat4::subtract() failed");
    Mat4::subtract(test4,test4,&test5);
    CUAssertAlwaysLog(test5 == Mat4::ZERO,      "Mat4::subtract() failed");

    testptr = Mat4::multiply(test4,2,&test5);
    CUAssertAlwaysLog(testptr == &test5,        "Mat4::multiply() failed");
    CUAssertAlwaysLog(test5.m[0] == 2.0f  && test5.m[1] == 10.0f && test5.m[2] == 18.0f && test5.m[3] == 26.0f &&
                      test5.m[4] == 4.0f  && test5.m[5] == 12.0f && test5.m[6] == 20.0f && test5.m[7] == 28.0f &&
                      test5.m[8] == 6.0f  && test5.m[9] == 14.0f && test5.m[10]== 22.0f && test5.m[11]== 30.0f &&
                      test5.m[12]== 8.0f  && test5.m[13]== 16.0f && test5.m[14]== 24.0f && test5.m[15]== 32.0f,
                      "Mat4::multiply() failed");
    
    testptr = Mat4::multiply(test1,test2,&test5);
	CUAssertAlwaysLog(testptr == &test5,        "Mat4::multiply() failed");
    CUAssertAlwaysLog(test5.m[0] == 2.0f && test5.m[1] == 0.0f && test5.m[2] == 0.0f && test5.m[3] == 0.0f &&
                      test5.m[4] == 0.0f && test5.m[5] == 3.0f && test5.m[6] == 0.0f && test5.m[7] == 0.0f &&
                      test5.m[8] == 0.0f && test5.m[9] == 0.0f && test5.m[10]== 4.0f && test5.m[11]== 0.0f &&
                      test5.m[12]== 5.0f && test5.m[13]== 6.0f && test5.m[14]== 7.0f && test5.m[15]== 1.0f,
                      "Mat4::multiply() failed");
    Mat4::multiply(test2,test1,&test5);
    CUAssertAlwaysLog(test5.m[0] == 2.0f && test5.m[1] == 0.0f && test5.m[2] == 0.0f && test5.m[3] == 0.0f &&
                      test5.m[4] == 0.0f && test5.m[5] == 3.0f && test5.m[6] == 0.0f && test5.m[7] == 0.0f &&
                      test5.m[8] == 0.0f && test5.m[9] == 0.0f && test5.m[10]== 4.0f && test5.m[11]== 0.0f &&
                      test5.m[12]==10.0f && test5.m[13]==18.0f && test5.m[14]==28.0f && test5.m[15]== 1.0f,
                      "Mat4::multiply() failed");
    Mat4::multiply(test4,Mat4::IDENTITY,&test5);
    CUAssertAlwaysLog(test5 == test4,           "Mat4::multiply() failed");
    Mat4::multiply(Mat4::IDENTITY,test4,&test5);
    CUAssertAlwaysLog(test5 == test4,           "Mat4::multiply() failed");

    testptr = Mat4::negate(test4,&test5);
    CUAssertAlwaysLog(testptr == &test5,        "Mat4::negate() failed");
    CUAssertAlwaysLog(test5.m[0] == -1.0f && test5.m[1] == -5.0f && test5.m[2] == -9.0f && test5.m[3] ==-13.0f &&
                      test5.m[4] == -2.0f && test5.m[5] == -6.0f && test5.m[6] ==-10.0f && test5.m[7] ==-14.0f &&
                      test5.m[8] == -3.0f && test5.m[9] == -7.0f && test5.m[10]==-11.0f && test5.m[11]==-15.0f &&
                      test5.m[12]== -4.0f && test5.m[13]== -8.0f && test5.m[14]==-12.0f && test5.m[15]==-16.0f,
                      "Mat4::subtract() failed");
    testptr = Mat4::negate(Mat4::ZERO,&test5);
    CUAssertAlwaysLog(test5.equals(Mat4::ZERO), "Mat4::negate() failed");
    
    testptr = Mat4::transpose(test4,&test5);
    CUAssertAlwaysLog(testptr == &test5,        "Mat4::transpose() failed");
    CUAssertAlwaysLog(test5.m[0] == 1.0f && test5.m[1] == 2.0f && test5.m[2] == 3.0f && test5.m[3] == 4.0f &&
                      test5.m[4] == 5.0f && test5.m[5] == 6.0f && test5.m[6] == 7.0f && test5.m[7] == 8.0f &&
                      test5.m[8] == 9.0f && test5.m[9] ==10.0f && test5.m[10]==11.0f && test5.m[11]==12.0f &&
                      test5.m[12]==13.0f && test5.m[13]==14.0f && test5.m[14]==15.0f && test5.m[15]==16.0f,
                      "Mat4::transpose() failed");
    Mat4::transpose(test5,&test5);
    CUAssertAlwaysLog(test5 == test4,           "Mat4::transpose() failed");
    Mat4::transpose(Mat4::IDENTITY,&test5);
    CUAssertAlwaysLog(test5 == Mat4::IDENTITY,  "Mat4::transpose() failed");

    testptr = Mat4::invert(test1,&test5);
    CUAssertAlwaysLog(testptr == &test5,        "Mat4::invert() failed");
    CUAssertAlwaysLog(CU_MATH_APPROX(test5.m[0], 1.0f/2.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.m[1], 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.m[2], 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.m[3], 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.m[4], 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.m[5], 1.0f/3.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.m[6], 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.m[7], 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.m[8], 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.m[9], 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.m[10], 1.0f/4.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.m[11], 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.m[12], 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.m[13], 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.m[14], 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.m[15], 1.0f, CU_MATH_EPSILON),
                      "Mat4::invert() failed");
    Mat4::invert(test5,&test5);
    CUAssertAlwaysLog(test5.equals(test1),      "Mat4::invert() failed");
    Mat4::invert(test2,&test5);
    CUAssertAlwaysLog(CU_MATH_APPROX(test5.m[0], 1.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.m[1], 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.m[2], 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.m[3], 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.m[4], 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.m[5], 1.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.m[6], 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.m[7], 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.m[8], 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.m[9], 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.m[10], 1.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.m[11], 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.m[12],-5.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.m[13],-6.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.m[14],-7.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.m[15], 1.0f, CU_MATH_EPSILON),
                      "Mat4::invert() failed");
    Mat4::invert(test5,&test5);
    CUAssertAlwaysLog(test5.equals(test2),          "Mat4::invert() failed");
    Mat4::invert(test6,&test5);
    CUAssertAlwaysLog(CU_MATH_APPROX(test5.m[0], 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.m[1], 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.m[2], 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.m[3], 1.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.m[4], 1.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.m[5], 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.m[6], 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.m[7],-1.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.m[8], 1.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.m[9],-1.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.m[10], 1.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.m[11],-1.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.m[12],-1.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.m[13], 1.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.m[14], 0.0f, CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.m[15], 1.0f, CU_MATH_EPSILON),
                      "Mat4::invert() failed");
    Mat4::invert(test5,&test5);
    CUAssertAlwaysLog(test5.equals(test6),          "Mat4::invert() failed");
    Mat4::invert(Mat4::IDENTITY,&test5);
    CUAssertAlwaysLog(test5.equals(Mat4::IDENTITY), "Mat4::invert() failed");
    Mat4::invert(Mat4::ONE,&test5);
    CUAssertAlwaysLog(test5 == Mat4::ZERO,          "Mat4::invert() failed");

    Mat4::invert(test3,&test5);
    Mat4::multiply(test3,test5,&test5);
    CUAssertAlwaysLog(test5.equals(Mat4::IDENTITY), "Mat4::invert() failed");
    
    Vec3 v3test1, v3test2;
    Mat4::decompose(test1,&v3test1,nullptr,nullptr);
    CUAssertAlwaysLog(v3test1 == Vec3(2,3,4),           "Mat4::decompose failed");
    Mat4::decompose(test2,&v3test1,nullptr,nullptr);
    CUAssertAlwaysLog(v3test1 == Vec3::ONE,             "Mat4::decompose  failed");
    Mat4::decompose(test3,&v3test1,nullptr,nullptr);
    CUAssertAlwaysLog(v3test1 == Vec3::ONE,             "Mat4::decompose  failed");

    Mat4::decompose(test1,nullptr,nullptr,&v3test2);
    CUAssertAlwaysLog(v3test2 == Vec3::ZERO,            "Mat4::decompose failed");
    Mat4::decompose(test2,nullptr,nullptr,&v3test2);
    CUAssertAlwaysLog(v3test2 == Vec3(5,6,7),           "Mat4::decompose  failed");
    Mat4::decompose(test3,nullptr,nullptr,&v3test2);
    CUAssertAlwaysLog(v3test2 == Vec3::ZERO,            "Mat4::decompose  failed");

    Mat4::decompose(test1,nullptr,&qother,nullptr);
    CUAssertAlwaysLog(qother == Quaternion::IDENTITY,   "Mat4::decompose failed");
    Mat4::decompose(test2,nullptr,&qother,nullptr);
    CUAssertAlwaysLog(qother == Quaternion::IDENTITY,   "Mat4::decompose failed");
    Mat4::decompose(test3,nullptr,&qother,nullptr);
    CUAssertAlwaysLog(qother == qtest,                  "Mat4::decompose failed");
    
    Mat4::multiply(test1,test3,&test5);
    Mat4::multiply(test5,test2,&test5);
    Mat4::decompose(test5,&v3test1,&qother,&v3test2);
    CUAssertAlwaysLog(v3test1.equals(Vec3(2,3,4)),      "Mat4::decompose failed");
    CUAssertAlwaysLog(qother.equals(qtest),             "Mat4::decompose failed");
    CUAssertAlwaysLog(v3test2.equals(Vec3(5,6,7)),      "Mat4::decompose failed");

    // Only rotation is guaranteed to be correct in this order
    Mat4::multiply(test3,test2,&test5);
    Mat4::multiply(test5,test1,&test5);
    Mat4::decompose(test5,nullptr,&qother,nullptr);
    CUAssertAlwaysLog(qother.equals(qtest,0.01f),       "Mat4::decompose failed");

    Mat4::multiply(test1,test2,&test5);
    Mat4::multiply(test5,test3,&test5);
    Mat4::decompose(test5,nullptr,&qother,nullptr);
    CUAssertAlwaysLog(qother.equals(qtest,0.01f),       "Mat4::decompose failed");

    end.mark();
    CULog("Static arithmetic test took %llu micros",cugl::Timestamp::ellapsedMicros(start,end));

#pragma mark Arithmetic Test
    start.mark();
    test6 = Mat4::ONE;
    test6.add(2);
    Mat4::add(Mat4::ONE,2,&test5);
    CUAssertAlwaysLog(test6 == test5,       "Method add() failed");
    
    test6 = Mat4::ONE;
    test6.add(test4);
    Mat4::add(Mat4::ONE,test4,&test5);
    CUAssertAlwaysLog(test6 == test5,       "Method add() failed");

    test6 = test4;
    test6.add(Mat4::ONE);
    CUAssertAlwaysLog(test6 == test5,       "Method add() failed");
    
    test6 = test4;
    test6.add(test4);
    Mat4::add(test4,test4,&test5);
    CUAssertAlwaysLog(test6 == test5,       "Method add() failed");

    test6 = Mat4::ONE;
    test6.subtract(2);
    Mat4::subtract(Mat4::ONE,2,&test5);
    CUAssertAlwaysLog(test6 == test5,       "Method subtract() failed");
    
    test6 = Mat4::ONE;
    test6.subtract(test4);
    Mat4::subtract(Mat4::ONE,test4,&test5);
    CUAssertAlwaysLog(test6 == test5,       "Method subtract() failed");
    
    test6 = test4;
    test6.subtract(Mat4::ONE);
    Mat4::subtract(test4,Mat4::ONE,&test5);
    CUAssertAlwaysLog(test6 == test5,       "Method subtract() failed");
    
    test6 = test4;
    test6.subtract(test4);
    CUAssertAlwaysLog(test6 == Mat4::ZERO,  "Method subtract() failed");

    test6 = test4;
    test6.multiply(2);
    Mat4::multiply(test4,2,&test5);
    CUAssertAlwaysLog(test6 == test5,       "Method multiply() failed");
    
    test6 = test1;
    test6.multiply(test2);
    Mat4::multiply(test1,test2,&test5);
    CUAssertAlwaysLog(test6 == test5,       "Method multiply() failed");
    
    test6 = test2;
    test6.multiply(test1);
    Mat4::multiply(test2,test1,&test5);
    CUAssertAlwaysLog(test6 == test5,       "Method multiply() failed");

    test6 = test4;
    test6.multiply(Mat4::IDENTITY);
    CUAssertAlwaysLog(test6 == test4,       "Method multiply() failed");

    test6 = Mat4::IDENTITY;
    test6.multiply(test4);
    CUAssertAlwaysLog(test6 == test4,       "Method multiply() failed");
    
    test6 = test4;
    test6.negate();
    Mat4::negate(test4,&test5);
    CUAssertAlwaysLog(test6 == test5,           "Method negate() failed");
    
    test6 = Mat4::ZERO;
    test6.negate();
    CUAssertAlwaysLog(test6.equals(Mat4::ZERO), "Method negate() failed");
    
    test6 = test4;
    test7 = test6.getNegation();
    CUAssertAlwaysLog(test7 != test6,           "Method getNegation() failed");
    CUAssertAlwaysLog(test7 == test5,           "Method getNegation() failed");

    test7 = Mat4::ZERO.getNegation();
    CUAssertAlwaysLog(test7.equals(Mat4::ZERO), "Method getNegation() failed");

    test6 = test4;
    test6.transpose();
    Mat4::transpose(test4,&test5);
    CUAssertAlwaysLog(test6 == test5,           "Method transpose() failed");
    test6.transpose();
    CUAssertAlwaysLog(test6 == test4,           "Method transpose() failed");
    test6 = Mat4::IDENTITY;
    test6.transpose();
    CUAssertAlwaysLog(test6 == Mat4::IDENTITY,  "Method transpose() failed");

    test6 = test4;
    test7 = test6.getTranspose();
    CUAssertAlwaysLog(test7 != test6,           "Method getTranspose() failed");
    CUAssertAlwaysLog(test7 == test5,           "Method getTranspose() failed");
    test7 = test7.getTranspose();
    CUAssertAlwaysLog(test7 == test4,           "Method getTranspose() failed");
    test7 = Mat4::IDENTITY.getTranspose();
    CUAssertAlwaysLog(test7 == Mat4::IDENTITY,  "Method getTranspose() failed");

    test6 = test1;
    test6.invert();
    Mat4::invert(test1,&test5);
    CUAssertAlwaysLog(test6 == test5,               "Method invert() failed");
    test6.invert();
    CUAssertAlwaysLog(test6.equals(test1),          "Method invert() failed");

    test6 = test2;
    test6.invert();
    Mat4::invert(test2,&test5);
    CUAssertAlwaysLog(test6 == test5,               "Method invert() failed");
    test6.invert();
    CUAssertAlwaysLog(test6.equals(test2),          "Method invert() failed");

    test6 = Mat4::IDENTITY;
    test6.invert();
    CUAssertAlwaysLog(test6.equals(Mat4::IDENTITY), "Method invert() failed");
    test6 = Mat4::ONE;
    test6.invert();
    CUAssertAlwaysLog(test6 == Mat4::ZERO,          "Method invert() failed");

    test6 = test3;
    test6.invert();
    test6 *= test3;
    CUAssertAlwaysLog(test6.equals(Mat4::IDENTITY), "Method invert() failed");

    test6 = test1;
    test7 = test6.getInverse();
    Mat4::invert(test1,&test5);
    CUAssertAlwaysLog(test7 != test6,               "Method getInverse() failed");
    CUAssertAlwaysLog(test7.equals(test5),          "Method getInverse() failed");
    test7 = test7.getInverse();
    CUAssertAlwaysLog(test7.equals(test1),          "Method getInverse() failed");
    
    test7 = test2.getInverse();
    Mat4::invert(test2,&test5);
    CUAssertAlwaysLog(test7.equals(test5),          "Method getInverse() failed");
    test7 = Mat4::IDENTITY.getInverse();
    CUAssertAlwaysLog(test7.equals(Mat4::IDENTITY), "Method getInverse() failed");
    test7 = Mat4::ONE.getInverse();
    CUAssertAlwaysLog(test7 == Mat4::ZERO,          "Method getInverse() failed");
    
    test7 = test3.getInverse()*test3;
    CUAssertAlwaysLog(test7.equals(Mat4::IDENTITY), "Method invert() failed");
    test7 = test3*test3.getInverse();
    CUAssertAlwaysLog(test7.equals(Mat4::IDENTITY), "Method invert() failed");

    end.mark();
    CULog("Arithmetic test took %llu micros",cugl::Timestamp::ellapsedMicros(start,end));

#pragma mark Operator Test
    start.mark();
    test6 = Mat4::ONE;
    test6 += test4;
    Mat4::add(Mat4::ONE,test4,&test5);
    CUAssertAlwaysLog(test6 == test5,                   "Addition operation failed");
    CUAssertAlwaysLog(Mat4::ONE + test4 == test5,       "Addition operation failed");
    CUAssertAlwaysLog(test4 + Mat4::ONE == test5,       "Addition operation failed");

    test6 = test4;
    test6 += test4;
    Mat4::add(test4,test4,&test5);
    CUAssertAlwaysLog(test6 == test5,                   "Addition operation failed");
    CUAssertAlwaysLog(test4 + test4 == test5,           "Addition operation failed");

    test6 = Mat4::ONE;
    test6 -= test4;
    Mat4::subtract(Mat4::ONE,test4,&test5);
    CUAssertAlwaysLog(test6 == test5,                   "Subtraction operation failed");
    CUAssertAlwaysLog(Mat4::ONE - test4 == test5,       "Subtraction operation failed");
    
    test6 = test4;
    test6 -= Mat4::ONE;
    Mat4::subtract(test4,Mat4::ONE,&test5);
    CUAssertAlwaysLog(test6 == test5,                   "Subtraction operation failed");
    CUAssertAlwaysLog(test4 - Mat4::ONE == test5,       "Subtraction operation failed");
    
    test6 = test4;
    test6 -= test4;
    CUAssertAlwaysLog(test6 == Mat4::ZERO,              "Subtraction operation failed");
    CUAssertAlwaysLog(test4-test4 == Mat4::ZERO,        "Subtraction operation failed");
    
    test6 = test4;
    test6 *= 2;
    Mat4::multiply(test4,2,&test5);
    CUAssertAlwaysLog(test6 == test5,                   "Scaling operation failed");
    CUAssertAlwaysLog(test4*2 == test5,                 "Scaling operation failed");
    CUAssertAlwaysLog(2*test4 == test5,                 "Scaling operation failed");
    
    test6 = test1;
    test6 *= test2;
    Mat4::multiply(test1,test2,&test5);
    CUAssertAlwaysLog(test6 == test5,                   "Multiplication operation failed");
    CUAssertAlwaysLog(test1 * test2 == test5,           "Multiplication operation failed");

    test6 = test2;
    test6 *= test1;
    Mat4::multiply(test2,test1,&test5);
    CUAssertAlwaysLog(test6 == test5,                   "Multiplication operation failed");
    CUAssertAlwaysLog(test2 * test1 == test5,           "Multiplication operation failed");
    
    test6 = test4;
    test6 *= Mat4::IDENTITY;
    CUAssertAlwaysLog(test6 == test4,                   "Multiplication operation failed");
    CUAssertAlwaysLog(test4 * Mat4::IDENTITY == test4,  "Multiplication operation failed");
    CUAssertAlwaysLog(Mat4::IDENTITY * test4 == test4,  "Multiplication operation failed");
    
    test6 = test4;
    Mat4::negate(test4,&test5);
    CUAssertAlwaysLog(-test6 == test5,                  "Negation operation failed");
    CUAssertAlwaysLog(-(-test6) == test6,               "Negation operation failed");
    CUAssertAlwaysLog((-Mat4::ZERO).equals(Mat4::ZERO), "Negation operation failed");

    end.mark();
    CULog("Operator test took %llu micros",cugl::Timestamp::ellapsedMicros(start,end));

#pragma mark Attribute Test
    start.mark();
    Mat4::createScale(1.0f, &test5);
    CUAssertAlwaysLog(!test1.isIdentity(),              "Method isIdentity() failed");
    CUAssertAlwaysLog(!test2.isIdentity(),              "Method isIdentity() failed");
    CUAssertAlwaysLog(test5.isIdentity(),               "Method isIdentity() failed");
    CUAssertAlwaysLog(Mat4::IDENTITY.isIdentity(),      "Method isIdentity() failed");
    CUAssertAlwaysLog(!Mat4::ONE.isIdentity(),          "Method isIdentity() failed");

    CUAssertAlwaysLog(CU_MATH_APPROX(test1.getDeterminant(), 24, CU_MATH_EPSILON),
                      "Method getDeterminant() failed");
    CUAssertAlwaysLog(CU_MATH_APPROX(test2.getDeterminant(), 1, CU_MATH_EPSILON),
                      "Method getDeterminant() failed");
    CUAssertAlwaysLog(CU_MATH_APPROX(test3.getDeterminant(), 1, CU_MATH_EPSILON),
                      "Method getDeterminant() failed");
    CUAssertAlwaysLog(CU_MATH_APPROX(Mat4::IDENTITY.getDeterminant(), 1, CU_MATH_EPSILON),
                      "Method getDeterminant() failed");
    CUAssertAlwaysLog(CU_MATH_APPROX(Mat4::ONE.getDeterminant(), 0, CU_MATH_EPSILON),
                      "Method getDeterminant() failed");

    CUAssertAlwaysLog(test1.isInvertible(),             "Method isInvertible() failed");
    CUAssertAlwaysLog(test2.isInvertible(),             "Method isInvertible() failed");
    CUAssertAlwaysLog(test3.isInvertible(),             "Method isInvertible() failed");
    CUAssertAlwaysLog(Mat4::IDENTITY.isInvertible(),    "Method isInvertible() failed");
    CUAssertAlwaysLog(!Mat4::ONE.isInvertible(),        "Method isInvertible() failed");
    CUAssertAlwaysLog(!Mat4::ZERO.isInvertible(),       "Method isInvertible() failed");

    CUAssertAlwaysLog(!test1.isOrthogonal(),            "Method isOrthogonal() failed");
    CUAssertAlwaysLog(!test2.isOrthogonal(),            "Method isOrthogonal() failed");
    CUAssertAlwaysLog(test3.isOrthogonal(),             "Method isOrthogonal() failed");
    CUAssertAlwaysLog(Mat4::IDENTITY.isOrthogonal(),    "Method isOrthogonal() failed");
    CUAssertAlwaysLog(!Mat4::ONE.isOrthogonal(),        "Method isOrthogonal() failed");

    CUAssertAlwaysLog(test1.getScale() == Vec3(2,3,4),  "Method getScale() failed");
    CUAssertAlwaysLog(test2.getScale() == Vec3::ONE,    "Method getScale() failed");
    CUAssertAlwaysLog(test3.getScale() == Vec3::ONE,    "Method getScale() failed");

    CUAssertAlwaysLog(test1.getTranslation() == Vec3::ZERO,         "Method getTranslation() failed");
    CUAssertAlwaysLog(test2.getTranslation() == Vec3(5,6,7),        "Method getTranslation() failed");
    CUAssertAlwaysLog(test3.getTranslation() == Vec3::ZERO,         "Method getTranslation() failed");
    
    CUAssertAlwaysLog(test1.getRotation() == Quaternion::IDENTITY,  "Method getRotation() failed");
    CUAssertAlwaysLog(test2.getRotation() == Quaternion::IDENTITY,  "Method getRotation() failed");
    CUAssertAlwaysLog(test3.getRotation() == qtest,                 "Method getRotation() failed");
    
    test5 = test1 * test3 * test2;
    CUAssertAlwaysLog(test5.getScale() == Vec3(2,3,4),              "Method getScale() failed");
    CUAssertAlwaysLog(test5.getTranslation() == Vec3(5,6,7),        "Method getTranslation() failed");
    CUAssertAlwaysLog(test5.getRotation() == qtest,                 "Method getRotation() failed");
    
    // Only rotation is guaranteed to be correct in this order
    test5 = test3 * test2 * test1;
    CUAssertAlwaysLog(test5.getRotation().equals(qtest,0.01f),      "Method getRotation() failed");
    test5 = test1 * test2 * test3;
    CUAssertAlwaysLog(test5.getRotation().equals(qtest,0.01f),      "Method getRotation() failed");

    Vec3 v3test3, v3test4;
    v3test1.set(1,1,1);
    v3test2.set(1,2,3);
    v3test3.set(2,1,1);
    Mat4::createLookAt(v3test1, v3test2, v3test3, &test5);
    Mat4::invert(test5, &test5);

    v3test4 = (v3test2-v3test1).getNormalization();
    CUAssertAlwaysLog(test5.getForwardVector().equals(v3test4),     "Method getForwardVector() failed");
    CUAssertAlwaysLog(test5.getBackVector().equals(-v3test4),       "Method getBackVector() failed");

    Vec3::cross(v3test3,v3test4,&v3test4); v3test4.normalize();
    CUAssertAlwaysLog(test5.getRightVector().equals(v3test4),       "Method getRightVector() failed");
    CUAssertAlwaysLog(test5.getLeftVector().equals(-v3test4),       "Method getLeftVector() failed");

    v3test2 -= v3test1; v3test2.normalize();
    Vec3::cross(v3test2,v3test4,&v3test4); v3test4.normalize();
    CUAssertAlwaysLog(test5.getUpVector().equals(v3test4),          "Method getUpVector() failed");
    CUAssertAlwaysLog(test5.getDownVector().equals(-v3test4),       "Method getDownVector() failed");
    
    end.mark();
    CULog("Attribute test took %llu micros",cugl::Timestamp::ellapsedMicros(start,end));

#pragma mark Static Transform Test
    start.mark();
    Mat4::createRotation(qtest,&test5);
    Mat4::rotate(Mat4::IDENTITY,qtest,&test6);
    CUAssertAlwaysLog(test6.equals(test5),                  "Mat4::rotate() failed");
    Mat4::rotate(test6,qtest.getConjugate(),&test6);
    CUAssertAlwaysLog(test6.equals(Mat4::IDENTITY),         "Mat4::rotate() failed");
    
    test5 = test1*test5;
    Mat4::rotate(test1,qtest,&test6);
    CUAssertAlwaysLog(test6.equals(test5),                  "Mat4::rotate() failed");
    test5 = test2*Mat4(qtest);
    Mat4::rotate(test2,qtest,&test6);
    CUAssertAlwaysLog(test6.equals(test5),                  "Mat4::rotate() failed");
    
    Mat4::createRotation(qtest,&test5);
    Mat4::rotate(Mat4::IDENTITY,Vec3::UNIT_X,M_PI_4,&test6);
    CUAssertAlwaysLog(test6.equals(test5),                  "Mat4::rotate() failed");
    Mat4::rotate(test6,Vec3::UNIT_X,-M_PI_4,&test6);
    CUAssertAlwaysLog(test6.equals(Mat4::IDENTITY),         "Mat4::rotate() failed");

    test5 = test1*test5;
    Mat4::rotate(test1,Vec3::UNIT_X,M_PI_4,&test6);
    CUAssertAlwaysLog(test6.equals(test5,CU_MATH_EPSILON),  "Mat4::rotate() failed");
    test5 = test2*Mat4(qtest);
    Mat4::rotate(test2,Vec3::UNIT_X,M_PI_4,&test6);
    CUAssertAlwaysLog(test6.equals(test5,CU_MATH_EPSILON),  "Mat4::rotate() failed");

    Mat4::createRotationX(M_PI_4/2.0f,&test5);
    Mat4::rotateX(Mat4::IDENTITY,M_PI_4/2.0f,&test6);
    CUAssertAlwaysLog(test6.equals(test5),                  "Mat4::rotateX() failed");
    Mat4::rotateX(test6,-M_PI_4/2.0f,&test6);
    CUAssertAlwaysLog(test6.equals(Mat4::IDENTITY),         "Mat4::rotateX() failed");

    test5 = test1*test5;
    Mat4::rotateX(test1,M_PI_4/2.0f,&test6);
    CUAssertAlwaysLog(test6.equals(test5),                  "Mat4::rotateX() failed");
    Mat4::createRotationX(M_PI_4/2.0f,&test5);
    test5 = test2*test5;
    Mat4::rotateX(test2,M_PI_4/2.0f,&test6);
    CUAssertAlwaysLog(test6.equals(test5),                  "Mat4::rotateX() failed");

    Mat4::createRotationY(M_PI_4/2.0f,&test5);
    Mat4::rotateY(Mat4::IDENTITY,M_PI_4/2.0f,&test6);
    CUAssertAlwaysLog(test6.equals(test5),                  "Mat4::rotateY() failed");
    Mat4::rotateY(test6,-M_PI_4/2.0f,&test6);
    CUAssertAlwaysLog(test6.equals(Mat4::IDENTITY),         "Mat4::rotateY() failed");
    
    test5 = test1*test5;
    Mat4::rotateY(test1,M_PI_4/2.0f,&test6);
    CUAssertAlwaysLog(test6.equals(test5),                  "Mat4::rotateY() failed");
    Mat4::createRotationY(M_PI_4/2.0f,&test5);
    test5 = test2*test5;
    Mat4::rotateY(test2,M_PI_4/2.0f,&test6);
    CUAssertAlwaysLog(test6.equals(test5),                  "Mat4::rotateY() failed");
    
    Mat4::createRotationZ(M_PI_4/2.0f,&test5);
    Mat4::rotateZ(Mat4::IDENTITY,M_PI_4/2.0f,&test6);
    CUAssertAlwaysLog(test6.equals(test5),                  "Mat4::rotateZ() failed");
    Mat4::rotateZ(test6,-M_PI_4/2.0f,&test6);
    CUAssertAlwaysLog(test6.equals(Mat4::IDENTITY),         "Mat4::rotateZ() failed");
    
    test5 = test1*test5;
    Mat4::rotateZ(test1,M_PI_4/2.0f,&test6);
    CUAssertAlwaysLog(test6.equals(test5),                  "Mat4::rotateZ() failed");
    Mat4::createRotationZ(M_PI_4/2.0f,&test5);
    test5 = test2*test5;
    Mat4::rotateZ(test2,M_PI_4/2.0f,&test6);
    CUAssertAlwaysLog(test6.equals(test5),                  "Mat4::rotateZ() failed");

    Mat4::createScale(2.0f,&test5);
    Mat4::scale(Mat4::IDENTITY,2.0f,&test6);
    CUAssertAlwaysLog(test6.equals(test5),                  "Mat4::scale() failed");
    Mat4::scale(test6,0.5f,&test6);
    CUAssertAlwaysLog(test6.equals(Mat4::IDENTITY),         "Mat4::scale() failed");
    
    test5 = test1*test5;
    Mat4::scale(test1,2.0f,&test6);
    CUAssertAlwaysLog(test6.equals(test5),                  "Mat4::scale() failed");
    Mat4::createScale(2.0f,&test5);
    test5 = test2*test5;
    Mat4::scale(test2,2.0f,&test6);
    CUAssertAlwaysLog(test6.equals(test5),                  "Mat4::scale() failed");
    
    Mat4::createScale(2.0f,4.0f,8.0f,&test5);
    Mat4::scale(Mat4::IDENTITY,2.0f,4.0f,8.0f,&test6);
    CUAssertAlwaysLog(test6.equals(test5),                  "Mat4::scale() failed");
    Mat4::scale(test6,0.5f,0.25f,0.125f,&test6);
    CUAssertAlwaysLog(test6.equals(Mat4::IDENTITY),         "Mat4::scale() failed");
    
    test5 = test1*test5;
    Mat4::scale(test1,2.0f,4.0f,8.0f,&test6);
    CUAssertAlwaysLog(test6.equals(test5),                  "Mat4::scale() failed");
    Mat4::createScale(2.0f,4.0f,8.0f,&test5);
    test5 = test2*test5;
    Mat4::scale(test2,2.0f,4.0f,8.0f,&test6);
    CUAssertAlwaysLog(test6.equals(test5),                  "Mat4::scale() failed");
    
    Mat4::createScale(2.0f,4.0f,8.0f,&test5);
    Mat4::scale(Mat4::IDENTITY,Vec3(2.0f,4.0f,8.0f),&test6);
    CUAssertAlwaysLog(test6.equals(test5),                  "Mat4::scale() failed");
    Mat4::scale(test6,Vec3(0.5f,0.25f,0.125f),&test6);
    CUAssertAlwaysLog(test6.equals(Mat4::IDENTITY),         "Mat4::scale() failed");
    
    test5 = test1*test5;
    Mat4::scale(test1,Vec3(2.0f,4.0f,8.0f),&test6);
    CUAssertAlwaysLog(test6.equals(test5),                  "Mat4::scale() failed");
    Mat4::createScale(2.0f,4.0f,8.0f,&test5);
    test5 = test2*test5;
    Mat4::scale(test2,Vec3(2.0f,4.0f,8.0f),&test6);
    CUAssertAlwaysLog(test6.equals(test5),                  "Mat4::scale() failed");
    
    Mat4::createTranslation(2.0f,4.0f,8.0f,&test5);
    Mat4::translate(Mat4::IDENTITY,2.0f,4.0f,8.0f,&test6);
    CUAssertAlwaysLog(test6.equals(test5),                  "Mat4::scale() failed");
    Mat4::translate(test6,-2.0f,-4.0f,-8.0f,&test6);
    CUAssertAlwaysLog(test6.equals(Mat4::IDENTITY),         "Mat4::scale() failed");
    
    test5 = test1*test5;
    Mat4::translate(test1,2.0f,4.0f,8.0f,&test6);
    CUAssertAlwaysLog(test6.equals(test5),                  "Mat4::scale() failed");
    Mat4::createTranslation(2.0f,4.0f,8.0f,&test5);
    test5 = test2*test5;
    Mat4::translate(test2,2.0f,4.0f,8.0f,&test6);
    CUAssertAlwaysLog(test6.equals(test5),                  "Mat4::scale() failed");
    
    Mat4::createTranslation(2.0f,4.0f,8.0f,&test5);
    Mat4::translate(Mat4::IDENTITY,Vec3(2.0f,4.0f,8.0f),&test6);
    CUAssertAlwaysLog(test6.equals(test5),                  "Mat4::scale() failed");
    Mat4::translate(test6,Vec3(-2.0f,-4.0f,-8.0f),&test6);
    CUAssertAlwaysLog(test6.equals(Mat4::IDENTITY),         "Mat4::scale() failed");
    
    test5 = test1*test5;
    Mat4::translate(test1,Vec3(2.0f,4.0f,8.0f),&test6);
    CUAssertAlwaysLog(test6.equals(test5),                  "Mat4::scale() failed");
    Mat4::createTranslation(2.0f,4.0f,8.0f,&test5);
    test5 = test2*test5;
    Mat4::translate(test2,Vec3(2.0f,4.0f,8.0f),&test6);
    CUAssertAlwaysLog(test6.equals(test5),                  "Mat4::scale() failed");

    end.mark();
    CULog("Static transform test took %llu micros",cugl::Timestamp::ellapsedMicros(start,end));

#pragma mark Transform Test
    start.mark();
    Mat4::createRotation(qtest,&test5);
    test6 = Mat4::IDENTITY;
    test6.rotate(qtest);
    CUAssertAlwaysLog(test6.equals(test5),                  "Method rotate() failed");
    test6.rotate(qtest.getConjugate());
    CUAssertAlwaysLog(test6.equals(Mat4::IDENTITY),         "Method rotate() failed");
    
    test5 = test1*test5;
    test6 = test1; test6.rotate(qtest);
    CUAssertAlwaysLog(test6.equals(test5),                  "Method rotate() failed");
    test5 = test2*Mat4(qtest);
    test6 = test2; test6.rotate(qtest);
    CUAssertAlwaysLog(test6.equals(test5),                  "Method rotate() failed");
    
    Mat4::createRotation(qtest,&test5);
    test6 = Mat4::IDENTITY;
    test6.rotate(Vec3::UNIT_X,M_PI_4);
    CUAssertAlwaysLog(test6.equals(test5),                  "Method rotate() failed");
    test6.rotate(Vec3::UNIT_X,-M_PI_4);
    CUAssertAlwaysLog(test6.equals(Mat4::IDENTITY),         "Method rotate() failed");
    
    test5 = test1*test5;
    test6 = test1; test6.rotate(Vec3::UNIT_X,M_PI_4);
    CUAssertAlwaysLog(test6.equals(test5,CU_MATH_EPSILON),  "Method rotate() failed");
    test5 = test2*Mat4(qtest);
    test6 = test2; test6.rotate(Vec3::UNIT_X,M_PI_4);
    CUAssertAlwaysLog(test6.equals(test5,CU_MATH_EPSILON),  "Method rotate() failed");
    
    Mat4::createRotationX(M_PI_4/2.0f,&test5);
    test6 = Mat4::IDENTITY;
    test6.rotateX(M_PI_4/2.0f);
    CUAssertAlwaysLog(test6.equals(test5),                  "Method rotateX() failed");
    test6.rotateX(-M_PI_4/2.0f);
    CUAssertAlwaysLog(test6.equals(Mat4::IDENTITY),         "Method rotateX() failed");
    
    test5 = test1*test5;
    test6 = test1; test6.rotateX(M_PI_4/2.0f);
    CUAssertAlwaysLog(test6.equals(test5),                  "Method rotateX() failed");
    Mat4::createRotationX(M_PI_4/2.0f,&test5);
    test5 = test2*test5;
    test6 = test2; test6.rotateX(M_PI_4/2.0f);
    CUAssertAlwaysLog(test6.equals(test5),                  "Method rotateX() failed");
    
    Mat4::createRotationY(M_PI_4/2.0f,&test5);
    test6 = Mat4::IDENTITY;
    test6.rotateY(M_PI_4/2.0f);
    CUAssertAlwaysLog(test6.equals(test5),                  "Method rotateY() failed");
    test6.rotateY(-M_PI_4/2.0f);
    CUAssertAlwaysLog(test6.equals(Mat4::IDENTITY),         "Method rotateY() failed");
    
    test5 = test1*test5;
    test6 = test1; test6.rotateY(M_PI_4/2.0f);
    CUAssertAlwaysLog(test6.equals(test5),                  "Method rotateY() failed");
    Mat4::createRotationY(M_PI_4/2.0f,&test5);
    test5 = test2*test5;
    test6 = test2; test6.rotateY(M_PI_4/2.0f);
    CUAssertAlwaysLog(test6.equals(test5),                  "Method rotateY() failed");
    
    Mat4::createRotationZ(M_PI_4/2.0f,&test5);
    test6 = Mat4::IDENTITY;
    test6.rotateZ(M_PI_4/2.0f);
    CUAssertAlwaysLog(test6.equals(test5),                  "Method rotateZ() failed");
    test6.rotateZ(-M_PI_4/2.0f);
    CUAssertAlwaysLog(test6.equals(Mat4::IDENTITY),         "Method rotateZ() failed");
    
    test5 = test1*test5;
    test6 = test1; test6.rotateZ(M_PI_4/2.0f);
    CUAssertAlwaysLog(test6.equals(test5),                  "Method rotateZ() failed");
    Mat4::createRotationZ(M_PI_4/2.0f,&test5);
    test5 = test2*test5;
    test6 = test2; test6.rotateZ(M_PI_4/2.0f);
    CUAssertAlwaysLog(test6.equals(test5),                  "Method rotateZ() failed");
    
    Mat4::createScale(2.0f,&test5);
    test6 = Mat4::IDENTITY;
    test6.scale(2.0f);
    CUAssertAlwaysLog(test6.equals(test5),                  "Method scale() failed");
    test6.scale(0.5f);
    CUAssertAlwaysLog(test6.equals(Mat4::IDENTITY),         "Method scale() failed");
    
    test5 = test1*test5;
    test6 = test1; test6.scale(2.0f);
    CUAssertAlwaysLog(test6.equals(test5),                  "Method scale() failed");
    Mat4::createScale(2.0f,&test5);
    test5 = test2*test5;
    test6 = test2; test6.scale(2.0f);
    CUAssertAlwaysLog(test6.equals(test5),                  "Method scale() failed");
    
    Mat4::createScale(2.0f,4.0f,8.0f,&test5);
    test6 = Mat4::IDENTITY;
    test6.scale(2.0f,4.0f,8.0f);
    CUAssertAlwaysLog(test6.equals(test5),                  "Method scale() failed");
    test6.scale(0.5f,0.25f,0.125f);
    CUAssertAlwaysLog(test6.equals(Mat4::IDENTITY),         "Method scale() failed");
    
    test5 = test1*test5;
    test6 = test1; test6.scale(2.0f,4.0f,8.0f);
    CUAssertAlwaysLog(test6.equals(test5),                  "Method scale() failed");
    Mat4::createScale(2.0f,4.0f,8.0f,&test5);
    test5 = test2*test5;
    test6 = test2; test6.scale(2.0f,4.0f,8.0f);
    CUAssertAlwaysLog(test6.equals(test5),                  "Method scale() failed");
    
    Mat4::createScale(2.0f,4.0f,8.0f,&test5);
    test6 = Mat4::IDENTITY;
    test6.scale(Vec3(2.0f,4.0f,8.0f));
    Mat4::scale(Mat4::IDENTITY,Vec3(2.0f,4.0f,8.0f),&test6);
    CUAssertAlwaysLog(test6.equals(test5),                  "Method scale() failed");
    test6.scale(Vec3(0.5f,0.25f,0.125f));
    CUAssertAlwaysLog(test6.equals(Mat4::IDENTITY),         "Method scale() failed");
    
    test5 = test1*test5;
    test6 = test1; test6.scale(Vec3(2.0f,4.0f,8.0f));
    CUAssertAlwaysLog(test6.equals(test5),                  "Method scale() failed");
    Mat4::createScale(2.0f,4.0f,8.0f,&test5);
    test5 = test2*test5;
    test6 = test2; test6.scale(Vec3(2.0f,4.0f,8.0f));
    CUAssertAlwaysLog(test6.equals(test5),                  "Method scale() failed");
    
    Mat4::createTranslation(2.0f,4.0f,8.0f,&test5);
    test6 = Mat4::IDENTITY;
    test6.translate(2.0f,4.0f,8.0f);
    CUAssertAlwaysLog(test6.equals(test5),                  "Method scale() failed");
    test6.translate(-2.0f,-4.0f,-8.0f);
    CUAssertAlwaysLog(test6.equals(Mat4::IDENTITY),         "Method scale() failed");
    
    test5 = test1*test5;
    test6 = test1; test6.translate(2.0f,4.0f,8.0f);
    CUAssertAlwaysLog(test6.equals(test5),                  "Method scale() failed");
    Mat4::createTranslation(2.0f,4.0f,8.0f,&test5);
    test5 = test2*test5;
    test6 = test2; test6.translate(2.0f,4.0f,8.0f);
    CUAssertAlwaysLog(test6.equals(test5),                  "Method scale() failed");
    
    Mat4::createTranslation(2.0f,4.0f,8.0f,&test5);
    test6 = Mat4::IDENTITY;
    test6.translate(Vec3(2.0f,4.0f,8.0f));
    CUAssertAlwaysLog(test6.equals(test5),                  "Method scale() failed");
    test6.translate(Vec3(-2.0f,-4.0f,-8.0f));
    CUAssertAlwaysLog(test6.equals(Mat4::IDENTITY),         "Method scale() failed");
    
    test5 = test1*test5;
    test6 = test1; test6.translate(Vec3(2.0f,4.0f,8.0f));
    CUAssertAlwaysLog(test6.equals(test5),                  "Method scale() failed");
    Mat4::createTranslation(2.0f,4.0f,8.0f,&test5);
    test5 = test2*test5;
    test6 = test2; test6.translate(Vec3(2.0f,4.0f,8.0f));
    CUAssertAlwaysLog(test6.equals(test5),                  "Method scale() failed");

    end.mark();
    CULog("Transform test took %llu micros",cugl::Timestamp::ellapsedMicros(start,end));

#pragma mark Static Vector Test
    start.mark();
    Vec2 v2test;
    Vec4 v4test;
    const float O_SQRT2 = 1.0f/sqrtf(2.0f);
    
    Mat4::transform(test1,Vec2::ONE,&v2test);
    CUAssertAlwaysLog(v2test.equals(Vec2(2,3)),                 "Mat4::transform() failed");
    Mat4::transform(test2,Vec2::ONE,&v2test);
    CUAssertAlwaysLog(v2test.equals(Vec2(6,7)),                 "Mat4::transform() failed");
    Mat4::transform(test3,Vec2::UNIT_X,&v2test);
    CUAssertAlwaysLog(v2test.equals(Vec2::UNIT_X),              "Mat4::transform() failed");
    Mat4::transform(test3,Vec2::UNIT_Y,&v2test);
    CUAssertAlwaysLog(v2test.equals(Vec2(0,O_SQRT2)),           "Mat4::transform() failed");

    Mat4::transformVector(test1,Vec2::ONE,&v2test);
    CUAssertAlwaysLog(v2test.equals(Vec2(2,3)),                 "Mat4::transformVector() failed");
    Mat4::transformVector(test2,Vec2::ONE,&v2test);
    CUAssertAlwaysLog(v2test.equals(Vec2::ONE),                 "Mat4::transformVector() failed");
    Mat4::transformVector(test3,Vec2::UNIT_X,&v2test);
    CUAssertAlwaysLog(v2test.equals(Vec2::UNIT_X),              "Mat4::transformVector() failed");
    Mat4::transformVector(test3,Vec2::UNIT_Y,&v2test);
    CUAssertAlwaysLog(v2test.equals(Vec2(0,O_SQRT2)),           "Mat4::transformVector() failed");

    Mat4::transform(test1,Vec3::ONE,&v3test1);
    CUAssertAlwaysLog(v3test1.equals(Vec3(2,3,4)),              "Mat4::transform() failed");
    Mat4::transform(test2,Vec3::ONE,&v3test1);
    CUAssertAlwaysLog(v3test1.equals(Vec3(6,7,8)),              "Mat4::transform() failed");
    Mat4::transform(test3,Vec3::UNIT_X,&v3test1);
    CUAssertAlwaysLog(v3test1.equals(Vec3::UNIT_X),             "Mat4::transform() failed");
    Mat4::transform(test3,Vec3::UNIT_Y,&v3test1);
    CUAssertAlwaysLog(v3test1.equals(Vec3(0,O_SQRT2,O_SQRT2)),  "Mat4::transform() failed");
    Mat4::transform(test3,Vec3::UNIT_Z,&v3test1);
    CUAssertAlwaysLog(v3test1.equals(Vec3(0,-O_SQRT2,O_SQRT2)), "Mat4::transform() failed");

    Mat4::transformVector(test1,Vec3::ONE,&v3test1);
    CUAssertAlwaysLog(v3test1.equals(Vec3(2,3,4)),              "Mat4::transformVector() failed");
    Mat4::transformVector(test2,Vec3::ONE,&v3test1);
    CUAssertAlwaysLog(v3test1.equals(Vec3::ONE),                "Mat4::transformVector() failed");
    Mat4::transformVector(test3,Vec3::UNIT_X,&v3test1);
    CUAssertAlwaysLog(v3test1.equals(Vec3::UNIT_X),             "Mat4::transformVector() failed");
    Mat4::transformVector(test3,Vec3::UNIT_Y,&v3test1);
    CUAssertAlwaysLog(v3test1.equals(Vec3(0,O_SQRT2,O_SQRT2)),  "Mat4::transformVector() failed");
    Mat4::transformVector(test3,Vec3::UNIT_Z,&v3test1);
    CUAssertAlwaysLog(v3test1.equals(Vec3(0,-O_SQRT2,O_SQRT2)), "Mat4::transformVector() failed");

    Mat4::transform(test1,Vec4::ONE,&v4test);
    CUAssertAlwaysLog(v4test.equals(Vec4(2,3,4,1)),             "Mat4::transform() failed");
    Mat4::transform(test2,Vec3::ONE,&v4test);
    CUAssertAlwaysLog(v4test.equals(Vec4(6,7,8,1)),             "Mat4::transform() failed");
    Mat4::transform(test3,Vec4::HOMOG_X,&v4test);
    CUAssertAlwaysLog(v4test.equals(Vec4::HOMOG_X),             "Mat4::transform() failed");
    Mat4::transform(test3,Vec4::HOMOG_Y,&v4test);
    CUAssertAlwaysLog(v4test.equals(Vec4(0,O_SQRT2,O_SQRT2,1)), "Mat4::transform() failed");
    Mat4::transform(test3,Vec4::HOMOG_Z,&v4test);
    CUAssertAlwaysLog(v4test.equals(Vec4(0,-O_SQRT2,O_SQRT2,1)),"Mat4::transform() failed");

    Rect rect1, rect2;
    Mat4::createRotationZ(M_PI_2, &test5);
    Mat4::createRotationZ(M_PI_4, &test6);
    
    rect1.set(-1,-2,2,4);
    Mat4::transform(test1,rect1,&rect2);
    CUAssertAlwaysLog(rect2.equals(Rect(-2,-6,4,12)),           "Affine2::transform() failed");
    Mat4::transform(test2,rect1,&rect2);
    CUAssertAlwaysLog(rect2.equals(Rect(4,4,2,4)),              "Affine2::transform() failed");
    Mat4::transform(test5,rect1,&rect2);
    CUAssertAlwaysLog(rect2.equals(Rect(-2,-1,4,2),CU_MATH_EPSILON),    "Affine2::transform() failed");
    Mat4::transform(test6,rect1,&rect2);
    CUAssertAlwaysLog(rect2.equals(Rect(-3*O_SQRT2,-3*O_SQRT2,6*O_SQRT2,6*O_SQRT2)),
                      "Affine2::transform() failed");

    end.mark();
    CULog("Static vector test took %llu micros",cugl::Timestamp::ellapsedMicros(start,end));

#pragma mark Vector Test
    start.mark();
    v2test = test1.transform(Vec2::ONE);
    CUAssertAlwaysLog(v2test.equals(Vec2(2,3)),                 "Method transform() failed");
    v2test = test2.transform(Vec2::ONE);
    CUAssertAlwaysLog(v2test.equals(Vec2(6,7)),                 "Method transform() failed");
    v2test = test3.transform(Vec2::UNIT_X);
    CUAssertAlwaysLog(v2test.equals(Vec2::UNIT_X),              "Method transform() failed");
    v2test = test3.transform(Vec2::UNIT_Y);
    CUAssertAlwaysLog(v2test.equals(Vec2(0,O_SQRT2)),           "Method transform() failed");
    
    v2test = test1.transformVector(Vec2::ONE);
    CUAssertAlwaysLog(v2test.equals(Vec2(2,3)),                 "Method transformVector() failed");
    v2test = test2.transformVector(Vec2::ONE);
    CUAssertAlwaysLog(v2test.equals(Vec2::ONE),                 "Method transformVector() failed");
    v2test = test3.transformVector(Vec2::UNIT_X);
    CUAssertAlwaysLog(v2test.equals(Vec2::UNIT_X),              "Method transformVector() failed");
    v2test = test3.transformVector(Vec2::UNIT_Y);
    CUAssertAlwaysLog(v2test.equals(Vec2(0,O_SQRT2)),           "Method transformVector() failed");
    
    v2test = Vec2::ONE; v2test *= test1;
    CUAssertAlwaysLog(v2test.equals(Vec2(2,3)),                 "Transform operation failed");
    CUAssertAlwaysLog((Vec2::ONE*test1).equals(Vec2(2,3)),      "Transform operation failed");
    v2test = Vec2::ONE; v2test *= test2;
    CUAssertAlwaysLog(v2test.equals(Vec2(6,7)),                 "Transform operation failed");
    CUAssertAlwaysLog((Vec2::ONE*test2).equals(Vec2(6,7)),      "Transform operation failed");
    v2test = Vec2::ONE; v2test *= test3;
    CUAssertAlwaysLog(v2test.equals(Vec2(1,O_SQRT2)),           "Transform operation failed");
    CUAssertAlwaysLog((Vec2::ONE*test3).equals(Vec2(1,O_SQRT2)),"Transform operation failed");
    
    v3test1 = test1.transform(Vec3::ONE);
    CUAssertAlwaysLog(v3test1.equals(Vec3(2,3,4)),              "Method transform() failed");
    v3test1 = test2.transform(Vec3::ONE);
    CUAssertAlwaysLog(v3test1.equals(Vec3(6,7,8)),              "Method transform() failed");
    v3test1 = test3.transform(Vec3::UNIT_X);
    CUAssertAlwaysLog(v3test1.equals(Vec3::UNIT_X),             "Method transform() failed");
    v3test1 = test3.transform(Vec3::UNIT_Y);
    CUAssertAlwaysLog(v3test1.equals(Vec3(0,O_SQRT2,O_SQRT2)),  "Method transform() failed");
    v3test1 = test3.transform(Vec3::UNIT_Z);
    CUAssertAlwaysLog(v3test1.equals(Vec3(0,-O_SQRT2,O_SQRT2)), "Method transform() failed");
    
    v3test1 = test1.transformVector(Vec3::ONE);
    CUAssertAlwaysLog(v3test1.equals(Vec3(2,3,4)),              "Method transformVector() failed");
    v3test1 = test2.transformVector(Vec3::ONE);
    CUAssertAlwaysLog(v3test1.equals(Vec3::ONE),                "Method transformVector() failed");
    v3test1 = test3.transformVector(Vec3::UNIT_X);
    CUAssertAlwaysLog(v3test1.equals(Vec3::UNIT_X),             "Method transformVector() failed");
    v3test1 = test3.transformVector(Vec3::UNIT_Y);
    CUAssertAlwaysLog(v3test1.equals(Vec3(0,O_SQRT2,O_SQRT2)),  "Method transformVector() failed");
    v3test1 = test3.transformVector(Vec3::UNIT_Z);
    CUAssertAlwaysLog(v3test1.equals(Vec3(0,-O_SQRT2,O_SQRT2)), "Method transformVector() failed");
    
    v3test1 = Vec3::ONE; v3test1 *= test1;
    CUAssertAlwaysLog(v3test1.equals(Vec3(2,3,4)),              "Transform operation failed");
    CUAssertAlwaysLog((Vec3::ONE*test1).equals(Vec3(2,3,4)),    "Transform operation failed");
    v3test1 = Vec3::ONE; v3test1 *= test2;
    CUAssertAlwaysLog(v3test1.equals(Vec3(6,7,8)),              "Transform operation failed");
    CUAssertAlwaysLog((Vec3::ONE*test2).equals(Vec3(6,7,8)),    "Transform operation failed");
    v3test1 = Vec3::ONE; v3test1 *= test3;
    CUAssertAlwaysLog(v3test1.equals(Vec3(1,0,sqrtf(2.0f)),CU_MATH_EPSILON),
                      "Transform operation failed");
    CUAssertAlwaysLog((Vec3::ONE*test3).equals(Vec3(1,0,sqrtf(2.0f)),CU_MATH_EPSILON),
                      "Transform operation failed");

    v4test = test1.transform(Vec4::ONE);
    CUAssertAlwaysLog(v4test.equals(Vec4(2,3,4,1)),             "Method transform() failed");
    v4test = test2.transform(Vec4::ONE);
    CUAssertAlwaysLog(v4test.equals(Vec4(6,7,8,1)),             "Method transform() failed");
    v4test = test3.transform(Vec4::HOMOG_X);
    CUAssertAlwaysLog(v4test.equals(Vec4::HOMOG_X),             "Method transform() failed");
    v4test = test3.transform(Vec4::HOMOG_Y);
    CUAssertAlwaysLog(v4test.equals(Vec4(0,O_SQRT2,O_SQRT2,1)), "Method transform() failed");
    v4test = test3.transform(Vec4::HOMOG_Z);
    CUAssertAlwaysLog(v4test.equals(Vec4(0,-O_SQRT2,O_SQRT2,1)),"Method transform() failed");

    v4test = Vec4::ONE; v4test *= test1;
    CUAssertAlwaysLog(v4test.equals(Vec4(2,3,4,1)),             "Transform operation failed");
    CUAssertAlwaysLog((Vec4::ONE*test1).equals(Vec4(2,3,4,1)),  "Transform operation failed");
    v4test = Vec4::ONE; v4test *= test2;
    CUAssertAlwaysLog(v4test.equals(Vec4(6,7,8,1)),             "Transform operation failed");
    CUAssertAlwaysLog((Vec4::ONE*test2).equals(Vec4(6,7,8,1)),  "Transform operation failed");
    v4test = Vec4::ONE; v4test *= test3;
    CUAssertAlwaysLog(v4test.equals(Vec4(1,0,sqrtf(2.0f),1),CU_MATH_EPSILON),
                      "Transform operation failed");
    CUAssertAlwaysLog((Vec4::ONE*test3).equals(Vec4(1,0,sqrtf(2.0f),1),CU_MATH_EPSILON),
                      "Transform operation failed");
    
    rect2 = test1.transform(rect1);
    CUAssertAlwaysLog(rect2.equals(Rect(-2,-6,4,12)),           "Method transform() failed");
    rect2 = test2.transform(rect1);
    CUAssertAlwaysLog(rect2.equals(Rect(4,4,2,4)),              "Method transform() failed");
    rect2 = test5.transform(rect1);
    CUAssertAlwaysLog(rect2.equals(Rect(-2,-1,4,2),CU_MATH_EPSILON),    "Method transform() failed");
    rect2 = test6.transform(rect1);
    CUAssertAlwaysLog(rect2.equals(Rect(-3*O_SQRT2,-3*O_SQRT2,6*O_SQRT2,6*O_SQRT2)),
                      "Method transform() failed");

    end.mark();
    CULog("Vector test took %llu micros",cugl::Timestamp::ellapsedMicros(start,end));

#pragma mark Conversion Test
    start.mark();
    std::string str1;
    test5.set(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16);
    
    std::string str2 = "";
    for(int ii = 0; ii < 4; ii++) {
        str2 += "\n";
        str2 += "|  ";
        str2 +=  cugl::to_string(test5.m[ii   ]).substr(0,8);
        str2 += ", ";
        str2 +=  cugl::to_string(test5.m[ii+4 ]).substr(0,8);
        str2 += ", ";
        str2 +=  cugl::to_string(test5.m[ii+8 ]).substr(0,8);
        str2 += ", ";
        str2 +=  cugl::to_string(test5.m[ii+12]).substr(0,8);
        str2 += "  |";
    }
    
    str1 = test5.toString();
    CUAssertAlwaysLog(str1 == str2,                 "Method toString() failed");
    str1 = test5.toString(true);
    CUAssertAlwaysLog(str1 == "cugl::Mat4"+str2,    "Method toString() failed");
    str1 = (std::string)test5;
    CUAssertAlwaysLog(str1 == str2,                 "String cast failed");

    Mat4::createScale(2,3,1,&test1);
    Mat4::createRotationZ(M_PI_4,&test2);
    Mat4::createTranslation(5,6,0,&test3);
    
    Affine2 atest1 = (Affine2)Mat4::IDENTITY;
    CUAssertAlwaysLog(atest1 == Affine2::IDENTITY,  "Affine2 cast failed");

    Affine2 atest2;
    Affine2::createScale(2,3,&atest2);
    atest1 = (Affine2)test1;
    CUAssertAlwaysLog(atest1 == atest2,             "Affine2 cast failed");
    
    Affine2::createRotation(M_PI_4,&atest2);
    atest1 = (Affine2)test2;
    CUAssertAlwaysLog(atest1 == atest2,             "Affine2 cast failed");
    
    Affine2::createTranslation(5,6,&atest2);
    atest1 = (Affine2)test3;
    CUAssertAlwaysLog(atest1 == atest2,             "Affine2 cast failed");
    
    Affine2::createScale(2,3,&atest1);
    atest1.rotate(M_PI_4);
    atest1.translate(5,6);
    
    Affine2 test8(atest1);
    test5 = test1*test2*test3;
    CUAssertAlwaysLog(test8.equals(test5),          "Affine2 constructor failed");
    test7 = atest1;
    CUAssertAlwaysLog(test7.equals(test5),          "Affine2 assignment failed");
    test6.set(atest1);
    CUAssertAlwaysLog(test6.equals(test5),          "Alternate Affine2 assignment failed");

    end.mark();
    CULog("Conversion test took %llu micros",cugl::Timestamp::ellapsedMicros(start,end));

    // And now a performance test
    start.mark();
    Mat4::createRotationZ(M_PI_4,&test2);
    test1 = test2;
    for(size_t ii = 0; ii < 100000; ii++) {
        Mat4::multiply(test1, test2, &test1);
    }
    end.mark();
    CULog("Performance test took %llu micros",cugl::Timestamp::ellapsedMicros(start,end));
    CULog("Matrix test took %llu micros",cugl::Timestamp::ellapsedMicros(globl,end));
#pragma mark Complete
    CULog("Mat4 tests complete.\n");
    

}

#pragma mark -
#pragma mark Affine2
/**
 * Unit test for a 2-dimensional affine transform.
 */
void cugl::testAffine2() {
    CULog("Running tests for Affine2.\n");
        
#pragma mark Constructor Test
    Affine2 test1;
    CUAssertAlwaysLog(test1.m[0] == 1.0f && test1.m[2] == 0.0f &&
                      test1.m[1] == 0.0f && test1.m[3] == 1.0f &&
                      test1.offset == Vec2::ZERO,
                      "Trivial constructor failed");
    
    Affine2 test2(1,2,3,4,5,6);
    CUAssertAlwaysLog(test2.m[0] == 1 && test2.m[1] == 3 &&
                      test2.m[2] == 2 && test2.m[3] == 4 &&
                      test2.offset == Vec2(5,6),
                      "Initialization constructor failed");
    
    float f[6] = {6,5,4,3,2,1};
    Affine2 test3(f);
    CUAssertAlwaysLog(test3.m[0] == 6 && test3.m[2] == 4 &&
                      test3.m[1] == 5 && test3.m[3] == 3 &&
                      test3.offset == Vec2(2,1),
                      "Array constructor failed");
    
    Affine2 test4(test2);
    CUAssertAlwaysLog(test4.m[0] == test2.m[0] && test4.m[1] == test2.m[1] &&
                      test4.m[2] == test2.m[2] && test4.m[3] == test2.m[3] &&
                      test4.offset == test2.offset,
                      "Copy constructor failed");
    
#pragma mark Static Constructor Test
    Affine2 test5;
    Affine2* testptr;
    testptr = Affine2::createScale(2.0f,&test5);
    CUAssertAlwaysLog(testptr == &test5,    "Scale constructor failed");
    CUAssertAlwaysLog(test5.m[0] == 2 && test5.m[2] == 0 &&
                      test5.m[1] == 0 && test5.m[3] == 2 &&
                      test5.offset == Vec2::ZERO,
                      "Scale constructor failed");
    
    test5 = Affine2::ONE;   // To scramble data
    testptr = Affine2::createScale(3.0f,4.0f,&test5);
    CUAssertAlwaysLog(testptr == &test5,    "Scale constructor failed");
    CUAssertAlwaysLog(test5.m[0] == 3 && test5.m[2] == 0 &&
                      test5.m[1] == 0 && test5.m[3] == 4 &&
                      test5.offset == Vec2::ZERO,
                      "Scale constructor failed");
    
    test5 = Affine2::ONE;   // To scramble data
    testptr = Affine2::createScale(Vec2(5.0f,6.0f),&test5);
    CUAssertAlwaysLog(testptr == &test5,    "Scale constructor failed");
    CUAssertAlwaysLog(test5.m[0] == 5 && test5.m[2] == 0 &&
                      test5.m[1] == 0 && test5.m[3] == 6 &&
                      test5.offset == Vec2::ZERO,
                      "Scale constructor failed");
    
    test5 = Affine2::ONE;   // To scramble data
    testptr = Affine2::createRotation(M_PI_4,&test5);
    CUAssertAlwaysLog(testptr == &test5,    "Rotation constructor failed");
    CUAssertAlwaysLog(CU_MATH_APPROX(test5.m[0],  1.0f/sqrtf(2.0f), CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.m[1],  1.0f/sqrtf(2.0f), CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.m[2], -1.0f/sqrtf(2.0f), CU_MATH_EPSILON) &&
                      CU_MATH_APPROX(test5.m[3],  1.0f/sqrtf(2.0f), CU_MATH_EPSILON) &&
                      test5.offset == Vec2::ZERO,
                      "Rotation constructor failed");
    
    test5 = Affine2::ONE;   // To scramble data
    testptr = Affine2::createTranslation(3.0f,4.0f,&test5);
    CUAssertAlwaysLog(testptr == &test5,    "Translation constructor failed");
    CUAssertAlwaysLog(test5.m[0] == 1 && test5.m[2] == 0 &&
                      test5.m[1] == 0 && test5.m[3] == 1 &&
                      test5.offset == Vec2(3,4),
                      "Translation constructor failed");
    
    test5 = Affine2::ONE;   // To scramble data
    testptr = Affine2::createTranslation(Vec2(5.0f,6.0f),&test5);
    CUAssertAlwaysLog(testptr == &test5,    "Translation constructor failed");
    CUAssertAlwaysLog(test5.m[0] == 1 && test5.m[2] == 0 &&
                      test5.m[1] == 0 && test5.m[3] == 1 &&
                      test5.offset == Vec2(5,6),
                      "Translation constructor failed");
    
#pragma mark Constants Test
    CUAssertAlwaysLog(Affine2::IDENTITY.m[0] == 1  && Affine2::IDENTITY.m[2] == 0  &&
                      Affine2::IDENTITY.m[1] == 0  && Affine2::IDENTITY.m[3] == 1  &&
                      Affine2::IDENTITY.offset == Vec2::ZERO,
                      "Identity transform failed");
    
    CUAssertAlwaysLog(Affine2::ZERO.m[0] == 0  && Affine2::ZERO.m[2] == 0  &&
                      Affine2::ZERO.m[1] == 0  && Affine2::ZERO.m[3] == 0  &&
                      Affine2::ZERO.offset == Vec2::ZERO,
                      "Zero transform failed");
    
    CUAssertAlwaysLog(Affine2::ONE.m[0] == 1  && Affine2::ONE.m[2] == 1  &&
                      Affine2::ONE.m[1] == 1  && Affine2::ONE.m[3] == 1  &&
                      Affine2::ONE.offset == Vec2::ONE,
                      "Ones transform failed");
#pragma mark Setter Test
    test1 = test2;
    CUAssertAlwaysLog(test1.m[0] == 1 && test1.m[1] == 3 &&
                      test1.m[2] == 2 && test1.m[3] == 4 &&
                      test1.offset == Vec2(5,6),
                      "Basic assignment failed");
    
    test1 = f;
    CUAssertAlwaysLog(test1.m[0] == 6 && test1.m[2] == 4 &&
                      test1.m[1] == 5 && test1.m[3] == 3 &&
                      test1.offset == Vec2(2,1),
                      "Float assignment failed");
    
    test1.set(1,2,3,4,5,6);
    CUAssertAlwaysLog(test1.m[0] == 1 && test1.m[1] == 3 &&
                      test1.m[2] == 2 && test1.m[3] == 4 &&
                      test1.offset == Vec2(5,6),
                      "Parameter assignment failed");
    
    test1.set(f);
    CUAssertAlwaysLog(test1.m[0] == 6 && test1.m[2] == 4 &&
                      test1.m[1] == 5 && test1.m[3] == 3 &&
                      test1.offset == Vec2(2,1),
                      "Alternate float assignment failed");
    
    test1.set(test2);
    CUAssertAlwaysLog(test1.m[0] == 1 && test1.m[1] == 3 &&
                      test1.m[2] == 2 && test1.m[3] == 4 &&
                      test1.offset == Vec2(5,6),
                      "Alternate assignment failed");
    test1.setZero();
    CUAssertAlwaysLog(test1.m[0] == 0  && test1.m[2] == 0 &&
                      test1.m[1] == 0  && test1.m[3] == 0 &&
                      test1.offset == Vec2::ZERO,
                      "Erasing assignment  failed");
    
    test1 = test2;
    test1.setIdentity();
    CUAssertAlwaysLog(test1.m[0] == 1  && test1.m[2] == 0 &&
                      test1.m[1] == 0  && test1.m[3] == 1 &&
                      test1.offset == Vec2::ZERO,
                      "Identity assignment  failed");
    
#pragma mark Comparison Test
    Affine2 test6;
    Affine2::createRotation(M_PI_4,&test5);
    Affine2::createRotation(M_PI_4,&test6);
    test6.offset += Vec2(CU_MATH_EPSILON/2.0f,-CU_MATH_EPSILON/2.0f);
    test6.m[0] += CU_MATH_EPSILON/2.0f;
    CUAssertAlwaysLog(test2.isExactly(test2),   "Method isExactly() failed");
    CUAssertAlwaysLog(test2.isExactly(test4),   "Method isExactly() failed");
    CUAssertAlwaysLog(!test2.isExactly(test3),  "Method isExactly() failed");
    CUAssertAlwaysLog(!test6.isExactly(test5),  "Method isExactly() failed");
    
    CUAssertAlwaysLog(test2.equals(test2),      "Method equals() failed");
    CUAssertAlwaysLog(test2.equals(test4),      "Method equals() failed");
    CUAssertAlwaysLog(!test2.equals(test3),     "Method equals() failed");
    CUAssertAlwaysLog(test6.equals(test5),      "Method equals() failed");
    
    CUAssertAlwaysLog(test2 == test2,           "Equals failed");
    CUAssertAlwaysLog(test2 == test4,           "Equals failed");
    CUAssertAlwaysLog(!(test2 == test3),        "Equals failed");
    CUAssertAlwaysLog(!(test6 == test5),        "Equals failed");
    
    CUAssertAlwaysLog(!(test2 != test2),        "Not equals failed");
    CUAssertAlwaysLog(!(test2 != test4),        "Not equals failed");
    CUAssertAlwaysLog(test2 != test3,           "Not equals failed");
    CUAssertAlwaysLog(test6 != test5,           "Not equals failed");
    
#pragma mark Static Arithmetic Test
    Affine2::createScale(2,3,&test1);
    Affine2::createTranslation(5,6,&test2);
    Affine2::createRotation(M_PI_4,&test3);
    test4.set(1,2,3,4,5,6);
    
    testptr = Affine2::add(Affine2::ONE,Vec2(1,2),&test5);
    CUAssertAlwaysLog(testptr == &test5,        "Affine2::add() failed");
    CUAssertAlwaysLog(memcmp(test5.m,Affine2::ONE.m,4*sizeof(float)) == 0 && test5.offset == Vec2(2,3),
                      "Affine2::add() failed");
    Affine2::add(Affine2::ONE,-Vec2::ONE,&test5);
    CUAssertAlwaysLog(memcmp(test5.m,Affine2::ONE.m,4*sizeof(float)) == 0 && test5.offset == Vec2::ZERO,
                      "Affine2::add() failed");

    testptr = Affine2::subtract(Affine2::ONE,Vec2(1,2),&test5);
    CUAssertAlwaysLog(testptr == &test5,        "Affine2::subtract() failed");
    CUAssertAlwaysLog(memcmp(test5.m,Affine2::ONE.m,4*sizeof(float)) == 0 && test5.offset == Vec2(0,-1),
                      "Affine2::subtract() failed");
    Affine2::subtract(Affine2::ONE,-Vec2::ONE,&test5);
    CUAssertAlwaysLog(memcmp(test5.m,Affine2::ONE.m,4*sizeof(float)) == 0 && test5.offset == Vec2(2,2),
                      "Affine2::subtract() failed");
    
    testptr = Affine2::multiply(test4,2,&test5);
    CUAssertAlwaysLog(testptr == &test5,        "Affine2::multiply() failed");
    CUAssertAlwaysLog(test5.m[0] == 2  && test5.m[2] == 4 &&
                      test5.m[1] == 6  && test5.m[3] == 8 &&
                      test5.offset == Vec2(10,12),
                      "Affine2::multiply() failed");
    
    testptr = Affine2::multiply(test1,test2,&test5);
    CUAssertAlwaysLog(testptr == &test5,        "Mat4::multiply() failed");
    CUAssertAlwaysLog(test5.m[0] == 2  && test5.m[2] == 0 &&
                      test5.m[1] == 0  && test5.m[3] == 3 &&
                      test5.offset == Vec2(5,6),
                      "Affine2::multiply() failed");
    Affine2::multiply(test2,test1,&test5);
    CUAssertAlwaysLog(test5.m[0] == 2  && test5.m[2] == 0 &&
                      test5.m[1] == 0  && test5.m[3] == 3 &&
                      test5.offset == Vec2(10,18),
                      "Affine2::multiply() failed");
    Affine2::multiply(test4,Affine2::IDENTITY,&test5);
    CUAssertAlwaysLog(test5 == test4,           "Affine2::multiply() failed");
    Affine2::multiply(Affine2::IDENTITY,test4,&test5);
    CUAssertAlwaysLog(test5 == test4,           "Affine2::multiply() failed");
    
    testptr = Affine2::invert(test1,&test5);
    CUAssertAlwaysLog(testptr == &test5,        "Affine2::invert() failed");
    CUAssertAlwaysLog(test5.m[0] == 1.0f/2.0f && test5.m[2] == 0         &&
                      test5.m[1] == 0         && test5.m[3] == 1.0f/3.0f &&
                      test5.offset == -test1.offset,
                      "Affine2::invert() failed");
    Affine2::invert(test5,&test5);
    CUAssertAlwaysLog(test5.equals(test1),      "Affine2::invert() failed");
    Affine2::invert(test2,&test5);
    CUAssertAlwaysLog(test5.m[0] == 1 && test5.m[2] == 0 &&
                      test5.m[1] == 0 && test5.m[3] == 1 &&
                      test5.offset == -test2.offset,
                      "Mat4::invert() failed");
    Affine2::invert(test5,&test5);
    CUAssertAlwaysLog(test5.equals(test2),              "Affine2::invert() failed");
    Affine2::invert(Affine2::IDENTITY,&test5);
    CUAssertAlwaysLog(test5.equals(Affine2::IDENTITY),  "Affine2::invert() failed");
    Affine2::invert(Affine2::ONE,&test5);
    CUAssertAlwaysLog(test5 == Affine2::ZERO,           "Affine2::invert() failed");
    
    Affine2::invert(test3,&test5);
    Affine2::multiply(test3,test5,&test5);
    CUAssertAlwaysLog(test5.equals(Affine2::IDENTITY),  "Affine2::invert() failed");
    
    Vec2 v2test1, v2test2;
    float value;
    Affine2::decompose(test1,&v2test1,nullptr,nullptr);
    CUAssertAlwaysLog(v2test1.equals(Vec2(2,3)),        "Affine2::decompose failed");
    Affine2::decompose(test2,&v2test1,nullptr,nullptr);
    CUAssertAlwaysLog(v2test1 == Vec2::ONE,             "Affine2::decompose  failed");
    Affine2::decompose(test3,&v2test1,nullptr,nullptr);
    CUAssertAlwaysLog(v2test1.equals(Vec2::ONE),        "Affine2::decompose  failed");
    
    Affine2::decompose(test1,nullptr,nullptr,&v2test2);
    CUAssertAlwaysLog(v2test2 == Vec2::ZERO,            "Affine2::decompose failed");
    Affine2::decompose(test2,nullptr,nullptr,&v2test2);
    CUAssertAlwaysLog(v2test2 == Vec2(5,6),             "Affine2::decompose  failed");
    Affine2::decompose(test3,nullptr,nullptr,&v2test2);
    CUAssertAlwaysLog(v2test2 == Vec2::ZERO,            "Affine2::decompose  failed");
    
    Affine2::decompose(test1,nullptr,&value,nullptr);
    CUAssertAlwaysLog(value == 0.0f,                    "Affine2::decompose failed");
    Affine2::decompose(test2,nullptr,&value,nullptr);
    CUAssertAlwaysLog(value == 0.0f,                    "Affine2::decompose failed");
    Affine2::decompose(test3,nullptr,&value,nullptr);
    CUAssertAlwaysLog(CU_MATH_APPROX(value,M_PI_4,CU_MATH_EPSILON),   "Affine2::decompose failed");
    
    Affine2::multiply(test1,test3,&test5);
    Affine2::multiply(test5,test2,&test5);
    Affine2::decompose(test5,&v2test1,&value,&v2test2);
    CUAssertAlwaysLog(v2test1.equals(Vec2(2,3),CU_MATH_EPSILON),      "Affine2::decompose failed");
    CUAssertAlwaysLog(CU_MATH_APPROX(value, M_PI_4, CU_MATH_EPSILON), "Affine2::decompose failed");
    CUAssertAlwaysLog(v2test2.equals(Vec2(5,6)),                "Affine2::decompose failed");
    
    // Only rotation is guaranteed to be correct in this order
    Affine2::multiply(test3,test2,&test5);
    Affine2::multiply(test5,test1,&test5);
    Affine2::decompose(test5,&v2test1,&value,nullptr);
    CUAssertAlwaysLog(CU_MATH_APPROX(value, M_PI_4, CU_MATH_EPSILON), "Mat4::decompose failed");
    
    Affine2::multiply(test1,test2,&test5);
    Affine2::multiply(test5,test3,&test5);
    Affine2::decompose(test5,nullptr,&value,nullptr);
    CUAssertAlwaysLog(CU_MATH_APPROX(value, M_PI_4, CU_MATH_EPSILON), "Mat4::decompose failed");

#pragma mark Arithmetic Test
    test6 = Affine2::ONE;
    test6.add(Vec2(1,2));
    Affine2::add(Affine2::ONE,Vec2(1,2),&test5);
    CUAssertAlwaysLog(test6 == test5,       "Method add() failed");
    
    test6 = Affine2::ONE;
    test6.add(-Vec2::ONE);
    Affine2::add(Affine2::ONE,-Vec2::ONE,&test5);
    CUAssertAlwaysLog(test6 == test5,       "Method add() failed");
    
    test6 = Affine2::ONE;
    test6.subtract(Vec2(1,2));
    Affine2::subtract(Affine2::ONE,Vec2(1,2),&test5);
    CUAssertAlwaysLog(test6 == test5,       "Method subtract() failed");
    
    test6 = Affine2::ONE;
    test6.subtract(-Vec2::ONE);
    Affine2::subtract(Affine2::ONE,-Vec2::ONE,&test5);
    CUAssertAlwaysLog(test6 == test5,       "Method subtract() failed");
    
    test6 = test4;
    test6.multiply(2);
    Affine2::multiply(test4,2,&test5);
    CUAssertAlwaysLog(test6 == test5,       "Method multiply() failed");
    
    test6 = test1;
    test6.multiply(test2);
    Affine2::multiply(test1,test2,&test5);
    CUAssertAlwaysLog(test6 == test5,       "Method multiply() failed");
    
    test6 = test2;
    test6.multiply(test1);
    Affine2::multiply(test2,test1,&test5);
    CUAssertAlwaysLog(test6 == test5,       "Method multiply() failed");
    
    test6 = test4;
    test6.multiply(Affine2::IDENTITY);
    CUAssertAlwaysLog(test6 == test4,       "Method multiply() failed");
    
    test6 = Affine2::IDENTITY;
    test6.multiply(test4);
    CUAssertAlwaysLog(test6 == test4,       "Method multiply() failed");

    test6 = test1;
    test6.invert();
    Affine2::invert(test1,&test5);
    CUAssertAlwaysLog(test6 == test5,                   "Method invert() failed");
    test6.invert();
    CUAssertAlwaysLog(test6.equals(test1),              "Method invert() failed");
    
    test6 = test2;
    test6.invert();
    Affine2::invert(test2,&test5);
    CUAssertAlwaysLog(test6 == test5,                   "Method invert() failed");
    test6.invert();
    CUAssertAlwaysLog(test6.equals(test2),              "Method invert() failed");
    
    test6 = Affine2::IDENTITY;
    test6.invert();
    CUAssertAlwaysLog(test6.equals(Affine2::IDENTITY),  "Method invert() failed");
    test6 = Affine2::ONE;
    test6.invert();
    CUAssertAlwaysLog(test6.equals(Affine2::ZERO),      "Method invert() failed");
    
    test6 = test3;
    test6.invert();
    test6 *= test3;
    CUAssertAlwaysLog(test6.equals(Affine2::IDENTITY),  "Method invert() failed");
    
    test6 = test1;
    Affine2 test7 = test6.getInverse();
    Affine2::invert(test1,&test5);
    CUAssertAlwaysLog(test7 != test6,                   "Method getInverse() failed");
    CUAssertAlwaysLog(test7 == test5,                   "Method getInverse() failed");
    test7 = test7.getInverse();
    CUAssertAlwaysLog(test7.equals(test1),              "Method getInverse() failed");
    
    test7 = test2.getInverse();
    Affine2::invert(test2,&test5);
    CUAssertAlwaysLog(test7 == test5,                   "Method getInverse() failed");
    test7 = Affine2::IDENTITY.getInverse();
    CUAssertAlwaysLog(test7.equals(Affine2::IDENTITY),  "Method getInverse() failed");
    test7 = Affine2::ONE.getInverse();
    CUAssertAlwaysLog(test7.equals(Affine2::ZERO),      "Method getInverse() failed");
    
    test7 = test3.getInverse()*test3;
    CUAssertAlwaysLog(test7.equals(Affine2::IDENTITY),  "Method invert() failed");
    test7 = test3*test3.getInverse();
    CUAssertAlwaysLog(test7.equals(Affine2::IDENTITY),  "Method invert() failed");
    
#pragma mark Operator Test
    test6 = Affine2::ONE;
    test6 += Vec2(1,2);
    Affine2::add(Affine2::ONE,Vec2(1,2),&test5);
    CUAssertAlwaysLog(test6 == test5,                       "Addition operation failed");
    CUAssertAlwaysLog(Affine2::ONE + Vec2(1,2) == test5,    "Addition operation failed");
    
    test6 = test4;
    test6 += -Vec2::ONE;
    Affine2::add(test4,-Vec2::ONE,&test5);
    CUAssertAlwaysLog(test6 == test5,                       "Addition operation failed");
    CUAssertAlwaysLog(test4 + -Vec2::ONE == test5,          "Addition operation failed");
    
    test6 = Affine2::ONE;
    test6 -= Vec2(1,2);
    Affine2::subtract(Affine2::ONE,Vec2(1,2),&test5);
    CUAssertAlwaysLog(test6 == test5,                       "Subtraction operation failed");
    CUAssertAlwaysLog(Affine2::ONE - Vec2(1,2) == test5,    "Subtraction operation failed");
    
    test6 = test4;
    test6 -= -Vec2::ONE;
    Affine2::subtract(test4,-Vec2::ONE,&test5);
    CUAssertAlwaysLog(test6 == test5,                       "Subtraction operation failed");
    CUAssertAlwaysLog(test4 - -Vec2::ONE == test5,          "Subtraction operation failed");
    
    test6 = test4;
    test6 *= 2;
    Affine2::multiply(test4,2,&test5);
    CUAssertAlwaysLog(test6 == test5,                       "Scaling operation failed");
    CUAssertAlwaysLog(test4*2 == test5,                     "Scaling operation failed");
    CUAssertAlwaysLog(2*test4 == test5,                     "Scaling operation failed");
    
    test6 = test1;
    test6 *= test2;
    Affine2::multiply(test1,test2,&test5);
    CUAssertAlwaysLog(test6 == test5,                       "Multiplication operation failed");
    CUAssertAlwaysLog(test1 * test2 == test5,               "Multiplication operation failed");
    
    test6 = test2;
    test6 *= test1;
    Affine2::multiply(test2,test1,&test5);
    CUAssertAlwaysLog(test6 == test5,                       "Multiplication operation failed");
    CUAssertAlwaysLog(test2 * test1 == test5,               "Multiplication operation failed");
    
    test6 = test4;
    test6 *= Affine2::IDENTITY;
    CUAssertAlwaysLog(test6 == test4,                       "Multiplication operation failed");
    CUAssertAlwaysLog(test4 * Affine2::IDENTITY == test4,   "Multiplication operation failed");
    CUAssertAlwaysLog(Affine2::IDENTITY * test4 == test4,   "Multiplication operation failed");
    
#pragma mark Attribute Test
    Affine2::createScale(1.0f, &test5);
    CUAssertAlwaysLog(!test1.isIdentity(),              "Method isIdentity() failed");
    CUAssertAlwaysLog(!test2.isIdentity(),              "Method isIdentity() failed");
    CUAssertAlwaysLog(test5.isIdentity(),               "Method isIdentity() failed");
    CUAssertAlwaysLog(Affine2::IDENTITY.isIdentity(),   "Method isIdentity() failed");
    CUAssertAlwaysLog(!Affine2::ONE.isIdentity(),       "Method isIdentity() failed");
    
    CUAssertAlwaysLog(CU_MATH_APPROX(test1.getDeterminant(), 6, CU_MATH_EPSILON),
                      "Method getDeterminant() failed");
    CUAssertAlwaysLog(CU_MATH_APPROX(test2.getDeterminant(), 1, CU_MATH_EPSILON),
                      "Method getDeterminant() failed");
    CUAssertAlwaysLog(CU_MATH_APPROX(test3.getDeterminant(), 1, CU_MATH_EPSILON),
                      "Method getDeterminant() failed");
    CUAssertAlwaysLog(CU_MATH_APPROX(Affine2::IDENTITY.getDeterminant(), 1, CU_MATH_EPSILON),
                      "Method getDeterminant() failed");
    CUAssertAlwaysLog(CU_MATH_APPROX(Affine2::ONE.getDeterminant(), 0, CU_MATH_EPSILON),
                      "Method getDeterminant() failed");
    
    CUAssertAlwaysLog(test1.isInvertible(),             "Method isInvertible() failed");
    CUAssertAlwaysLog(test2.isInvertible(),             "Method isInvertible() failed");
    CUAssertAlwaysLog(test3.isInvertible(),             "Method isInvertible() failed");
    CUAssertAlwaysLog(Affine2::IDENTITY.isInvertible(), "Method isInvertible() failed");
    CUAssertAlwaysLog(!Affine2::ONE.isInvertible(),     "Method isInvertible() failed");
    CUAssertAlwaysLog(!Affine2::ZERO.isInvertible(),    "Method isInvertible() failed");
    
    CUAssertAlwaysLog(test1.getScale() == Vec2(2,3),        "Method getScale() failed");
    CUAssertAlwaysLog(test2.getScale() == Vec2::ONE,        "Method getScale() failed");
    CUAssertAlwaysLog(test3.getScale().equals(Vec2::ONE),   "Method getScale() failed");
    
    CUAssertAlwaysLog(test1.getTranslation() == Vec2::ZERO, "Method getTranslation() failed");
    CUAssertAlwaysLog(test2.getTranslation() == Vec2(5,6),  "Method getTranslation() failed");
    CUAssertAlwaysLog(test3.getTranslation() == Vec2::ZERO, "Method getTranslation() failed");
    
    CUAssertAlwaysLog(CU_MATH_APPROX(test1.getRotation(),0,CU_MATH_EPSILON),      "Method getRotation() failed");
    CUAssertAlwaysLog(CU_MATH_APPROX(test2.getRotation(),0,CU_MATH_EPSILON),      "Method getRotation() failed");
    CUAssertAlwaysLog(CU_MATH_APPROX(test3.getRotation(),M_PI_4,CU_MATH_EPSILON), "Method getRotation() failed");
    
    test5 = test1 * test3 * test2;
    CUAssertAlwaysLog(test5.getScale().equals(Vec2(2,3),CU_MATH_EPSILON),   "Method getScale() failed");
    CUAssertAlwaysLog(test5.getTranslation().equals(Vec2(5,6)),             "Method getTranslation() failed");
    CUAssertAlwaysLog(CU_MATH_APPROX(test5.getRotation(),M_PI_4,CU_MATH_EPSILON), "Method getRotation() failed");
    
    // Only rotation is guaranteed to be correct in this order
    test5 = test3 * test2 * test1;
    CUAssertAlwaysLog(CU_MATH_APPROX(test5.getRotation(),M_PI_4,CU_MATH_EPSILON), "Method getRotation() failed");
    test5 = test1 * test2 * test3;
    CUAssertAlwaysLog(CU_MATH_APPROX(test5.getRotation(),M_PI_4,CU_MATH_EPSILON), "Method getRotation() failed");
    
#pragma mark Static Transform Test
    Affine2::createRotation(M_PI_4/2.0f,&test5);
    Affine2::rotate(Affine2::IDENTITY,M_PI_4/2.0f,&test6);
    CUAssertAlwaysLog(test6.equals(test5),                      "Affine2::rotate() failed");
    Affine2::rotate(test6,-M_PI_4/2.0f,&test6);
    CUAssertAlwaysLog(test6.equals(Affine2::IDENTITY),          "Affine2::rotate() failed");
    
    test5 = test1*test5;
    Affine2::rotate(test1,M_PI_4/2.0f,&test6);
    CUAssertAlwaysLog(test6.equals(test5),                      "Affine2::rotate() failed");
    Affine2::createRotation(M_PI_4/2.0f,&test5);
    test5 = test2*test5;
    Affine2::rotate(test2,M_PI_4/2.0f,&test6);
    CUAssertAlwaysLog(test6.equals(test5),                      "Affine2::rotate() failed");
    
    Affine2::createScale(2.0f,&test5);
    Affine2::scale(Affine2::IDENTITY,2.0f,&test6);
    CUAssertAlwaysLog(test6.equals(test5),                      "Affine2::scale() failed");
    Affine2::scale(test6,0.5f,&test6);
    CUAssertAlwaysLog(test6.equals(Affine2::IDENTITY),          "Affine2::scale() failed");
    
    test5 = test1*test5;
    Affine2::scale(test1,2.0f,&test6);
    CUAssertAlwaysLog(test6.equals(test5),                      "Affine2::scale() failed");
    Affine2::createScale(2.0f,&test5);
    test5 = test2*test5;
    Affine2::scale(test2,2.0f,&test6);
    CUAssertAlwaysLog(test6.equals(test5),                      "Affine2::scale() failed");
    
    Affine2::createScale(2.0f,4.0f,&test5);
    Affine2::scale(Affine2::IDENTITY,2.0f,4.0f,&test6);
    CUAssertAlwaysLog(test6.equals(test5),                      "Affine2::scale() failed");
    Affine2::scale(test6,0.5f,0.25f,&test6);
    CUAssertAlwaysLog(test6.equals(Affine2::IDENTITY),          "Affine2::scale() failed");
    
    test5 = test1*test5;
    Affine2::scale(test1,2.0f,4.0f,&test6);
    CUAssertAlwaysLog(test6.equals(test5),                      "Affine2::scale() failed");
    Affine2::createScale(2.0f,4.0f,&test5);
    test5 = test2*test5;
    Affine2::scale(test2,2.0f,4.0f,&test6);
    CUAssertAlwaysLog(test6.equals(test5),                      "Affine2::scale() failed");
    
    Affine2::createScale(2.0f,4.0f,&test5);
    Affine2::scale(Affine2::IDENTITY,Vec2(2.0f,4.0f),&test6);
    CUAssertAlwaysLog(test6.equals(test5),                      "Affine2::scale() failed");
    Affine2::scale(test6,Vec2(0.5f,0.25f),&test6);
    CUAssertAlwaysLog(test6.equals(Affine2::IDENTITY),          "Affine2::scale() failed");
    
    test5 = test1*test5;
    Affine2::scale(test1,Vec2(2.0f,4.0f),&test6);
    CUAssertAlwaysLog(test6.equals(test5),                      "Affine2::scale() failed");
    Affine2::createScale(2.0f,4.0f,&test5);
    test5 = test2*test5;
    Affine2::scale(test2,Vec2(2.0f,4.0f),&test6);
    CUAssertAlwaysLog(test6.equals(test5),                      "Affine2::scale() failed");
    
    Affine2::createTranslation(2.0f,4.0f,&test5);
    Affine2::translate(Affine2::IDENTITY,2.0f,4.0f,&test6);
    CUAssertAlwaysLog(test6.equals(test5),                      "Affine2::translate() failed");
    Affine2::translate(test6,-2.0f,-4.0f,&test6);
    CUAssertAlwaysLog(test6.equals(Affine2::IDENTITY),          "Affine2::translate() failed");
    
    test5 = test1*test5;
    Affine2::translate(test1,2.0f,4.0f,&test6);
    CUAssertAlwaysLog(test6.equals(test5),                      "Affine2::translate() failed");
    Affine2::createTranslation(2.0f,4.0f,&test5);
    test5 = test2*test5;
    Affine2::translate(test2,2.0f,4.0f,&test6);
    CUAssertAlwaysLog(test6.equals(test5),                      "Affine2::translate() failed");
    
    Affine2::createTranslation(2.0f,4.0f,&test5);
    Affine2::translate(Affine2::IDENTITY,Vec2(2.0f,4.0f),&test6);
    CUAssertAlwaysLog(test6.equals(test5),                      "Affine2::translate() failed");
    Affine2::translate(test6,Vec2(-2.0f,-4.0f),&test6);
    CUAssertAlwaysLog(test6.equals(Affine2::IDENTITY),          "Affine2::translate() failed");
    
    test5 = test1*test5;
    Affine2::translate(test1,Vec2(2.0f,4.0f),&test6);
    CUAssertAlwaysLog(test6.equals(test5),                      "Affine2::translate() failed");
    Affine2::createTranslation(2.0f,4.0f,&test5);
    test5 = test2*test5;
    Affine2::translate(test2,Vec2(2.0f,4.0f),&test6);
    CUAssertAlwaysLog(test6.equals(test5),                      "Affine2::translate() failed");
    
#pragma mark Transform Test
    Affine2::createRotation(M_PI_4/2.0f,&test5);
    test6 = Affine2::IDENTITY;
    test6.rotate(M_PI_4/2.0f);
    CUAssertAlwaysLog(test6.equals(test5),                      "Method rotate() failed");
    test6.rotate(-M_PI_4/2.0f);
    CUAssertAlwaysLog(test6.equals(Affine2::IDENTITY),          "Method rotate() failed");
    
    test5 = test1*test5;
    test6 = test1; test6.rotate(M_PI_4/2.0f);
    CUAssertAlwaysLog(test6.equals(test5),                      "Method rotate() failed");
    Affine2::createRotation(M_PI_4/2.0f,&test5);
    test5 = test2*test5;
    test6 = test2; test6.rotate(M_PI_4/2.0f);
    CUAssertAlwaysLog(test6.equals(test5),                      "Method rotate() failed");
    
    Affine2::createScale(2.0f,&test5);
    test6 = Affine2::IDENTITY;
    test6.scale(2.0f);
    CUAssertAlwaysLog(test6.equals(test5),                      "Method scale() failed");
    test6.scale(0.5f);
    CUAssertAlwaysLog(test6.equals(Affine2::IDENTITY),          "Method scale() failed");
    
    test5 = test1*test5;
    test6 = test1; test6.scale(2.0f);
    CUAssertAlwaysLog(test6.equals(test5),                      "Method scale() failed");
    Affine2::createScale(2.0f,&test5);
    test5 = test2*test5;
    test6 = test2; test6.scale(2.0f);
    CUAssertAlwaysLog(test6.equals(test5),                      "Method scale() failed");
    
    Affine2::createScale(2.0f,4.0f,&test5);
    test6 = Affine2::IDENTITY;
    test6.scale(2.0f,4.0f);
    CUAssertAlwaysLog(test6.equals(test5),                      "Method scale() failed");
    test6.scale(0.5f,0.25f);
    CUAssertAlwaysLog(test6.equals(Affine2::IDENTITY),          "Method scale() failed");
    
    test5 = test1*test5;
    test6 = test1; test6.scale(2.0f,4.0f);
    CUAssertAlwaysLog(test6.equals(test5),                      "Method scale() failed");
    Affine2::createScale(2.0f,4.0f,&test5);
    test5 = test2*test5;
    test6 = test2; test6.scale(2.0f,4.0f);
    CUAssertAlwaysLog(test6.equals(test5),                      "Method scale() failed");
    
    Affine2::createScale(2.0f,4.0f,&test5);
    test6 = Affine2::IDENTITY;
    test6.scale(Vec2(2.0f,4.0f));
    Affine2::scale(Affine2::IDENTITY,Vec2(2.0f,4.0f),&test6);
    CUAssertAlwaysLog(test6.equals(test5),                      "Method scale() failed");
    test6.scale(Vec2(0.5f,0.25f));
    CUAssertAlwaysLog(test6.equals(Affine2::IDENTITY),          "Method scale() failed");
    
    test5 = test1*test5;
    test6 = test1; test6.scale(Vec2(2.0f,4.0f));
    CUAssertAlwaysLog(test6.equals(test5),                      "Method scale() failed");
    Affine2::createScale(2.0f,4.0f,&test5);
    test5 = test2*test5;
    test6 = test2; test6.scale(Vec2(2.0f,4.0f));
    CUAssertAlwaysLog(test6.equals(test5),                      "Method scale() failed");
    
    Affine2::createTranslation(2.0f,4.0f,&test5);
    test6 = Affine2::IDENTITY;
    test6.translate(2.0f,4.0f);
    CUAssertAlwaysLog(test6.equals(test5),                      "Method translate() failed");
    test6.translate(-2.0f,-4.0f);
    CUAssertAlwaysLog(test6.equals(Affine2::IDENTITY),          "Method translate() failed");
    
    test5 = test1*test5;
    test6 = test1; test6.translate(2.0f,4.0f);
    CUAssertAlwaysLog(test6.equals(test5),                      "Method translate() failed");
    Affine2::createTranslation(2.0f,4.0f,&test5);
    test5 = test2*test5;
    test6 = test2; test6.translate(2.0f,4.0f);
    CUAssertAlwaysLog(test6.equals(test5),                      "Method translate() failed");
    
    Affine2::createTranslation(2.0f,4.0f,&test5);
    test6 = Affine2::IDENTITY;
    test6.translate(Vec2(2.0f,4.0f));
    CUAssertAlwaysLog(test6.equals(test5),                      "Method translate() failed");
    test6.translate(Vec2(-2.0f,-4.0f));
    CUAssertAlwaysLog(test6.equals(Affine2::IDENTITY),          "Method translate() failed");
    
    test5 = test1*test5;
    test6 = test1; test6.translate(Vec2(2.0f,4.0f));
    CUAssertAlwaysLog(test6.equals(test5),                      "Method translate() failed");
    Affine2::createTranslation(2.0f,4.0f,&test5);
    test5 = test2*test5;
    test6 = test2; test6.translate(Vec2(2.0f,4.0f));
    CUAssertAlwaysLog(test6.equals(test5),                      "Method translate() failed");
    
#pragma mark Vector Test
    const float O_SQRT2 = 1.0f/sqrtf(2.0f);
    Affine2::transform(test1,Vec2::ONE,&v2test1);
    CUAssertAlwaysLog(v2test1.equals(Vec2(2,3)),                "Affine2::transform() failed");
    Affine2::transform(test2,Vec2::ONE,&v2test1);
    CUAssertAlwaysLog(v2test1.equals(Vec2(6,7)),                "Affine2::transform() failed");
    Affine2::transform(test3,Vec2::UNIT_X,&v2test1);
    CUAssertAlwaysLog(v2test1.equals(Vec2(O_SQRT2,-O_SQRT2)),   "Affine2::transform() failed");
    Affine2::transform(test3,Vec2::UNIT_Y,&v2test1);
    CUAssertAlwaysLog(v2test1.equals(Vec2(O_SQRT2,O_SQRT2)),    "Affine2::transform() failed");
    
    v2test1 = test1.transform(Vec2::ONE);
    CUAssertAlwaysLog(v2test1.equals(Vec2(2,3)),                "Method transform() failed");
    v2test1 = test2.transform(Vec2::ONE);
    CUAssertAlwaysLog(v2test1.equals(Vec2(6,7)),                "Method transform() failed");
    v2test1 = test3.transform(Vec2::UNIT_X);
    CUAssertAlwaysLog(v2test1.equals(Vec2(O_SQRT2,-O_SQRT2)),   "Method transform() failed");
    v2test1 = test3.transform(Vec2::UNIT_Y);
    CUAssertAlwaysLog(v2test1.equals(Vec2(O_SQRT2,O_SQRT2)),    "Method transform() failed");
    
    v2test1 = Vec2::ONE; v2test1 *= test1;
    CUAssertAlwaysLog(v2test1.equals(Vec2(2,3)),                "Transform operation failed");
    CUAssertAlwaysLog((Vec2::ONE*test1).equals(Vec2(2,3)),      "Transform operation failed");
    v2test1 = Vec2::ONE; v2test1 *= test2;
    CUAssertAlwaysLog(v2test1.equals(Vec2(6,7)),                "Transform operation failed");
    CUAssertAlwaysLog((Vec2::ONE*test2).equals(Vec2(6,7)),      "Transform operation failed");
    v2test1 = Vec2::ONE; v2test1 *= test3;
    CUAssertAlwaysLog(v2test1.equals(Vec2(sqrtf(2),0)),          "Transform operation failed");
    CUAssertAlwaysLog((Vec2::ONE*test3).equals(Vec2(sqrtf(2),0)),"Transform operation failed");
    
    Rect rect1, rect2;
    Affine2::createRotation(M_PI_2, &test5);

    rect1.set(-1,-2,2,4);
    Affine2::transform(test1,rect1,&rect2);
    CUAssertAlwaysLog(rect2.equals(Rect(-2,-6,4,12)),           "Affine2::transform() failed");
    Affine2::transform(test2,rect1,&rect2);
    CUAssertAlwaysLog(rect2.equals(Rect(4,4,2,4)),              "Affine2::transform() failed");
    Affine2::transform(test5,rect1,&rect2);
    CUAssertAlwaysLog(rect2.equals(Rect(-2,-1,4,2),CU_MATH_EPSILON),    "Affine2::transform() failed");
    Affine2::transform(test3,rect1,&rect2);
    CUAssertAlwaysLog(rect2.equals(Rect(-3*O_SQRT2,-3*O_SQRT2,6*O_SQRT2,6*O_SQRT2)),
                      "Affine2::transform() failed");

    rect2 = test1.transform(rect1);
    CUAssertAlwaysLog(rect2.equals(Rect(-2,-6,4,12)),           "Method transform() failed");
    rect2 = test2.transform(rect1);
    CUAssertAlwaysLog(rect2.equals(Rect(4,4,2,4)),              "Method transform() failed");
    rect2 = test5.transform(rect1);
    CUAssertAlwaysLog(rect2.equals(Rect(-2,-1,4,2),CU_MATH_EPSILON),    "Method transform() failed");
    rect2 = test3.transform(rect1);
    CUAssertAlwaysLog(rect2.equals(Rect(-3*O_SQRT2,-3*O_SQRT2,6*O_SQRT2,6*O_SQRT2)),
                      "Method transform() failed");
    
    
#pragma mark Conversion Test
    std::string str1;
    test5.set(1,2,3,4,5,6);
    
    std::string str2 = "";
    for(int ii = 0; ii < 2; ii++) {
        str2 += "\n";
        str2 += "|  ";
        str2 +=  cugl::to_string(test5.m[ii   ]).substr(0,8);
        str2 += ", ";
        str2 +=  cugl::to_string(test5.m[ii+2 ]).substr(0,8);
        str2 += "  | ";
    }
    str2 += "+ "+test5.offset.toString(false);
    
    str1 = test5.toString();
    CUAssertAlwaysLog(str1 == str2,                 "Method toString() failed");
    str1 = test5.toString(true);
    CUAssertAlwaysLog(str1 == "cugl::Affine2"+str2, "Method toString() failed");
    str1 = (std::string)test5;
    CUAssertAlwaysLog(str1 == str2,                 "String cast failed");

    Mat4 mtest1 = (Mat4)Affine2::IDENTITY;
    CUAssertAlwaysLog(mtest1 == Mat4::IDENTITY,     "Mat4 cast failed");

    Mat4 mtest2;
    Mat4::createScale(2,3,1,&mtest2);
    mtest1 = (Mat4)test1;
    CUAssertAlwaysLog(mtest1 == mtest2,             "Mat4 cast failed");

    Mat4::createTranslation(5, 6, 0, &mtest2);
    mtest1 = (Mat4)test2;
    CUAssertAlwaysLog(mtest1 == mtest2,             "Mat4 cast failed");

    Mat4::createRotationZ(M_PI_4,&mtest2);
    mtest1 = (Mat4)test3;
    CUAssertAlwaysLog(mtest1 == mtest2,             "Mat4 cast failed");

    Mat4::createScale(2,3,1,&mtest1);
    mtest1.rotateZ(M_PI_4);
    mtest1.translate(5,6,0);
    
    Affine2 test8(mtest1);
    test5 = test1*test3*test2;
    CUAssertAlwaysLog(test8.equals(test5),          "Mat4 constructor failed");
    test7 = mtest1;
    CUAssertAlwaysLog(test7.equals(test5),          "Mat4 assignment failed");
    test6.set(mtest1);
    CUAssertAlwaysLog(test6.equals(test5),          "Alternate Mat4 assignment failed");
    
    
#pragma mark Complete
    CULog("Affine2 tests complete.\n");
    
}


#pragma mark -
#pragma mark Poly2
/**
 * Unit test for  2-dimensional polygon
 */
void cugl::testPoly2() {
    CULog("Running tests for Poly2.\n");

#pragma mark Constructor Test
    // Set up some initializers.
    float vertices[6] = {1,2,3,4,5,6};
    std::vector<Vec2>  vvec;
    std::vector<float> fvec;
    for(int ii = 0; ii < 3; ii++) {
        vvec.push_back(Vec2(vertices[2*ii],vertices[2*ii+1]));
        fvec.push_back(vertices[2*ii  ]);
        fvec.push_back(vertices[2*ii+1]);
    }
    
    unsigned short parr[6] = {0,1,1,2,2,0};
    std::vector<unsigned short> pindx;
    pindx.push_back(0);
    pindx.push_back(1);
    pindx.push_back(1);
    pindx.push_back(2);
    pindx.push_back(2);
    pindx.push_back(0);
    
    unsigned short sarr[3] = {0,1,2};
    std::vector<unsigned short> sindx;
    sindx.push_back(0);
    sindx.push_back(1);
    sindx.push_back(2);
    
    Rect bounds(1,2,4,4);
    
    Poly2 test1;
    CUAssertAlwaysLog(test1.getVertices().size() == 0 && test1.getIndices().size() == 0 &&
                      test1.getType() == Poly2::Type::UNDEFINED && test1.getBounds() == Rect::ZERO,
                      "Trivial constructor failed");
    
    Poly2 test2(vvec);
    CUAssertAlwaysLog(test2.getVertices() == vvec && test2.getIndices().size() == 0 &&
                      test2.getType() == Poly2::Type::UNDEFINED && test2.getBounds() == bounds,
                      "Vec2 vector constructor failed");

    Poly2 test3(vvec,pindx);
    CUAssertAlwaysLog(test3.getVertices() == vvec && test3.getIndices() == pindx &&
                      test3.getType() == Poly2::Type::PATH  && test3.getBounds() == bounds,
                      "Indexed Vec2 vector constructor failed");

    Poly2 test4(fvec);
    CUAssertAlwaysLog(test4.getVertices() == vvec && test4.getIndices().size() == 0 &&
                      test4.getType() == Poly2::Type::UNDEFINED  && test4.getBounds() == bounds,
                      "Float vector constructor failed");
    
    Poly2 test5(fvec,sindx);
    CUAssertAlwaysLog(test5.getVertices() == vvec && test5.getIndices() == sindx &&
                      test5.getType() == Poly2::Type::SOLID  && test5.getBounds() == bounds,
                      "Indexed float vector constructor failed");

    Poly2 test6((Vec2*)vertices, 3);
    CUAssertAlwaysLog(test6.getVertices() == vvec && test6.getIndices().size() == 0 &&
                      test6.getType() == Poly2::Type::UNDEFINED  && test6.getBounds() == bounds,
                      "Vec2 array constructor failed");

    Poly2 test7(vertices, 6);
    CUAssertAlwaysLog(test7.getVertices() == vvec && test7.getIndices().size() == 0 &&
                      test7.getType() == Poly2::Type::UNDEFINED  && test7.getBounds() == bounds,
                      "Float array constructor failed");

    Poly2 test8((Vec2*)vertices, 3, sarr, 3);
    CUAssertAlwaysLog(test8.getVertices() == vvec && test8.getIndices() == sindx &&
                      test8.getType() == Poly2::Type::SOLID && test8.getBounds() == bounds,
                      "Indexed Vec2 array constructor failed");

    Poly2 test9(vertices, 6, parr, 6);
    CUAssertAlwaysLog(test9.getVertices() == vvec && test9.getIndices() == pindx &&
                      test9.getType() == Poly2::Type::PATH  && test9.getBounds() == bounds,
                      "Indexed float array constructor failed");

    Poly2 test10(test5);
    CUAssertAlwaysLog(test10.getVertices() == vvec && test10.getIndices() == sindx &&
                      test10.getType() == Poly2::Type::SOLID  && test10.getBounds() == bounds,
                      "Copy constructor failed");

    Rect rect(0,0,10,10);
    Poly2 test11(rect);
    CUAssertAlwaysLog(test11.getVertices().size() == 4 && test11.getIndices().size() == 6 &&
                      test11.getType() == Poly2::Type::SOLID && test11.getBounds() == rect,
                      "Rect constructor failed");
    
#pragma mark Setter Test
    test3.clear();
    CUAssertAlwaysLog(test3.getVertices().size() == 0 && test3.getIndices().size() == 0 &&
                      test3.getType() == Poly2::Type::UNDEFINED && test3.getBounds() == Rect::ZERO,
                      "Erasing assignment failed");

    test1 = test11;
    CUAssertAlwaysLog(test1.getVertices() == test11.getVertices() &&
                      test1.getIndices()  == test11.getIndices() &&
                      test1.getType()     == test11.getType() &&
                      test1.getBounds()   == test11.getBounds(),
                      "Copy assignment failed");
    
    test1.clear();
    test1 = rect;
    CUAssertAlwaysLog(test1.getVertices() == test11.getVertices() &&
                      test1.getIndices()  == test11.getIndices() &&
                      test1.getType()     == test11.getType() &&
                      test1.getBounds()   == test11.getBounds(),
                      "Rect assignment failed");
    
    test1.clear();
    test1.set(vvec);
    CUAssertAlwaysLog(test2.getVertices() == vvec && test2.getIndices().size() == 0 &&
                      test2.getType() == Poly2::Type::UNDEFINED && test2.getBounds() == bounds,
                      "Vec2 vector assignment failed");
    
    test1.clear();
    test1.set(vvec,pindx);
    CUAssertAlwaysLog(test1.getVertices() == vvec && test1.getIndices() == pindx &&
                      test1.getType() == Poly2::Type::PATH  && test1.getBounds() == bounds,
                      "Indexed Vec2 vector assignment failed");
    
    test1.clear();
    test1.set(fvec);
    CUAssertAlwaysLog(test1.getVertices() == vvec && test1.getIndices().size() == 0 &&
                      test1.getType() == Poly2::Type::UNDEFINED  && test1.getBounds() == bounds,
                      "Float vector assignment failed");
    
    test1.clear();
    test1.set(fvec,sindx);
    CUAssertAlwaysLog(test1.getVertices() == vvec && test1.getIndices() == sindx &&
                      test1.getType() == Poly2::Type::SOLID  && test1.getBounds() == bounds,
                      "Indexed float vector assignment failed");
    
    test1.clear();
    test1.set((Vec2*)vertices, 3);
    CUAssertAlwaysLog(test1.getVertices() == vvec && test1.getIndices().size() == 0 &&
                      test1.getType() == Poly2::Type::UNDEFINED  && test1.getBounds() == bounds,
                      "Vec2 array assignment failed");
    
    test1.clear();
    test1.set(vertices, 6);
    CUAssertAlwaysLog(test1.getVertices() == vvec && test1.getIndices().size() == 0 &&
                      test1.getType() == Poly2::Type::UNDEFINED  && test1.getBounds() == bounds,
                      "Float array assignment failed");
    
    test1.clear();
    test1.set((Vec2*)vertices, 3, sarr, 3);
    CUAssertAlwaysLog(test1.getVertices() == vvec && test1.getIndices() == sindx &&
                      test1.getType() == Poly2::Type::SOLID && test1.getBounds() == bounds,
                      "Indexed Vec2 array assignment failed");
    
    test1.clear();
    test1.set(vertices, 6, parr, 6);
    CUAssertAlwaysLog(test1.getVertices() == vvec && test1.getIndices() == pindx &&
                      test1.getType() == Poly2::Type::PATH  && test1.getBounds() == bounds,
                      "Indexed float array assignment failed");

    test1.clear();
    test1.set(test11);
    CUAssertAlwaysLog(test1.getVertices() == test11.getVertices() &&
                      test1.getIndices()  == test11.getIndices() &&
                      test1.getType()     == test11.getType() &&
                      test1.getBounds()   == test11.getBounds(),
                      "Alternate copy assignment failed");
    
    test1.clear();
    test1.set(rect);
    CUAssertAlwaysLog(test1.getVertices() == test11.getVertices() &&
                      test1.getIndices()  == test11.getIndices() &&
                      test1.getType()     == test11.getType() &&
                      test1.getBounds()   == test11.getBounds(),
                      "Alternate rect assignment failed");

#pragma mark Static Constructor Test
    Poly2* testptr;
    
    test1 = Poly2::createLine(Vec2(1,2), Vec2(3,4));
    CUAssertAlwaysLog(test1.getVertices().size() == 2 && test1.getIndices().size() == 2 &&
                      test1.getType() == Poly2::Type::PATH && test1.getBounds() == Rect(1,2,2,2),
                      "Create line constructor failed");
    
    test1.clear();
    testptr = Poly2::createLine(Vec2(1,2), Vec2(3,4), &test1);
    CUAssertAlwaysLog(testptr == &test1,        "Create line constructor failed");
    CUAssertAlwaysLog(test1.getVertices().size() == 2 && test1.getIndices().size() == 2 &&
                      test1.getType() == Poly2::Type::PATH && test1.getBounds() == Rect(1,2,2,2),
                      "Create line constructor failed");

    test1 = Poly2::createTriangle(Vec2(1,2), Vec2(3,4), Vec2(5,0));
    CUAssertAlwaysLog(test1.getVertices().size() == 3 && test1.getIndices().size() == 3 &&
                      test1.getType() == Poly2::Type::SOLID && test1.getBounds() == Rect(1,0,4,4),
                      "Create triangle constructor failed");
    test1 = Poly2::createTriangle(Vec2(1,2), Vec2(3,4), Vec2(5,0), false);
    CUAssertAlwaysLog(test1.getVertices().size() == 3 && test1.getIndices().size() == 6 &&
                      test1.getType() == Poly2::Type::PATH && test1.getBounds() == Rect(1,0,4,4),
                      "Create triangle constructor failed");

    test1.clear(); testptr = nullptr;
    testptr = Poly2::createTriangle(Vec2(1,2), Vec2(3,4), Vec2(5,0), &test1);
    CUAssertAlwaysLog(testptr == &test1,        "Create triangle constructor failed");
    CUAssertAlwaysLog(test1.getVertices().size() == 3 && test1.getIndices().size() == 3 &&
                      test1.getType() == Poly2::Type::SOLID && test1.getBounds() == Rect(1,0,4,4),
                      "Create triangle constructor failed");
    Poly2::createTriangle(Vec2(1,2), Vec2(3,4), Vec2(5,0), &test1, false);
    CUAssertAlwaysLog(test1.getVertices().size() == 3 && test1.getIndices().size() == 6 &&
                      test1.getType() == Poly2::Type::PATH && test1.getBounds() == Rect(1,0,4,4),
                      "Create triangle constructor failed");
    
    test1 = Poly2::createEllipse(Vec2(1,2), Size(4,6), 8);
    CUAssertAlwaysLog(test1.getVertices().size() == 9 && test1.getIndices().size() == 24 &&
                      test1.getType() == Poly2::Type::SOLID && test1.getBounds() == Rect(-1,-1,4,6),
                      "Create ellipse constructor failed");
    test1 = Poly2::createEllipse(Vec2(1,2), Size(4,6), 8, false);
    CUAssertAlwaysLog(test1.getVertices().size() == 8 && test1.getIndices().size() == 16 &&
                      test1.getType() == Poly2::Type::PATH && test1.getBounds() == Rect(-1,-1,4,6),
                      "Create ellipse constructor failed");

    test1.clear(); testptr = nullptr;
    testptr = Poly2::createEllipse(Vec2(1,2), Size(4,6), 8, &test1);
    CUAssertAlwaysLog(test1.getVertices().size() == 9 && test1.getIndices().size() == 24 &&
                      test1.getType() == Poly2::Type::SOLID && test1.getBounds() == Rect(-1,-1,4,6),
                      "Create ellipse constructor failed");
    Poly2::createEllipse(Vec2(1,2), Size(4,6), 8, &test1, false);
    CUAssertAlwaysLog(test1.getVertices().size() == 8 && test1.getIndices().size() == 16 &&
                      test1.getType() == Poly2::Type::PATH && test1.getBounds() == Rect(-1,-1,4,6),
                      "Create ellipse constructor failed");

#pragma mark Index Test
    test1.set(vvec);
    test1.setIndices(pindx);
    CUAssertAlwaysLog(test1.getVertices() == vvec && test1.getIndices() == pindx &&
                      test1.getType() == Poly2::Type::PATH  && test1.getBounds() == bounds,
                      "Vector-based setIndex failed");
    test1.setIndices(sindx);
    CUAssertAlwaysLog(test1.getVertices() == vvec && test1.getIndices() == sindx &&
                      test1.getType() == Poly2::Type::SOLID  && test1.getBounds() == bounds,
                      "Vector-based setIndex failed");
    test1.setIndices(parr,6);
    CUAssertAlwaysLog(test1.getVertices() == vvec && test1.getIndices() == pindx &&
                      test1.getType() == Poly2::Type::PATH  && test1.getBounds() == bounds,
                      "Array-based setIndex failed");
    test1.setIndices(sarr,3);
    CUAssertAlwaysLog(test1.getVertices() == vvec && test1.getIndices() == sindx &&
                      test1.getType() == Poly2::Type::SOLID  && test1.getBounds() == bounds,
                      "Array-based setIndex failed");
    
    unsigned short arr1[3] = {10,11,12};
    unsigned short arr2[5] = {0,1,2,1,0};
    test1.clear();
    test2.set(vvec,sindx);
    test3.set(vvec,pindx);
    test4.set(vvec,sindx); test4.setType(Poly2::Type::PATH);
    test5.set(vvec,pindx); test5.setType(Poly2::Type::UNDEFINED);
    test6.set(vertices, 6, arr1, 3);
    test7.set(vertices, 6, arr2, 5);
    
    CUAssertAlwaysLog(test1.isStandardized(),   "Method isStandardized() failed");
    CUAssertAlwaysLog(test2.isStandardized(),   "Method isStandardized() failed");
    CUAssertAlwaysLog(test3.isStandardized(),   "Method isStandardized() failed");
    CUAssertAlwaysLog(!test4.isStandardized(),  "Method isStandardized() failed");
    CUAssertAlwaysLog(!test5.isStandardized(),  "Method isStandardized() failed");
    CUAssertAlwaysLog(test6.isStandardized(),   "Method isStandardized() failed");
    CUAssertAlwaysLog(!test7.isStandardized(),  "Method isStandardized() failed");

    CUAssertAlwaysLog(test1.isValid(),          "Method isValid() failed");
    CUAssertAlwaysLog(test2.isValid(),          "Method isValid() failed");
    CUAssertAlwaysLog(test3.isValid(),          "Method isValid() failed");
    CUAssertAlwaysLog(!test4.isValid(),         "Method isValid() failed");
    CUAssertAlwaysLog(!test5.isValid(),         "Method isValid() failed");
    CUAssertAlwaysLog(!test6.isValid(),         "Method isValid() failed");
    CUAssertAlwaysLog(!test7.isValid(),         "Method isValid() failed");

#pragma mark Operator Test
    test1.set(vvec);
    test4.set(vvec);
    test1 *= 2;
    test2 = 2*test4;
    test3 = test4*2;
    CUAssertAlwaysLog(test1.getVertices()[0] == Vec2(2,4),  "Scaling operation failed");
    CUAssertAlwaysLog(test1.getVertices()[1] == Vec2(6,8),  "Scaling operation failed");
    CUAssertAlwaysLog(test1.getVertices()[2] == Vec2(10,12),"Scaling operation failed");
    CUAssertAlwaysLog(test1.getVertices() == test2.getVertices(),   "Scaling operation failed");
    CUAssertAlwaysLog(test1.getVertices() == test3.getVertices(),   "Scaling operation failed");

    test1.set(vvec);
    test4.set(vvec);
    test1 /= 0.5f;
    test2 = test4/0.5f;
    CUAssertAlwaysLog(test1.getVertices()[0] == Vec2(2,4),  "Scaling operation failed");
    CUAssertAlwaysLog(test1.getVertices()[1] == Vec2(6,8),  "Scaling operation failed");
    CUAssertAlwaysLog(test1.getVertices()[2] == Vec2(10,12),"Scaling operation failed");
    CUAssertAlwaysLog(test1.getVertices() == test2.getVertices(),   "Scaling operation failed");

    test1.set(vvec);
    test1 *= Vec2(2,3);
    test2 = Vec2(2,3)*test4;
    test3 = test4*Vec2(2,3);
    CUAssertAlwaysLog(test1.getVertices()[0] == Vec2(2,6),  "Scaling operation failed");
    CUAssertAlwaysLog(test1.getVertices()[1] == Vec2(6,12), "Scaling operation failed");
    CUAssertAlwaysLog(test1.getVertices()[2] == Vec2(10,18),"Scaling operation failed");
    CUAssertAlwaysLog(test1.getVertices() == test2.getVertices(),   "Scaling operation failed");
    CUAssertAlwaysLog(test1.getVertices() == test3.getVertices(),   "Scaling operation failed");

    test1.set(vvec);
    test1 /= Vec2(0.5f,0.25f);
    test2 = test4/Vec2(0.5f,0.25f);
    CUAssertAlwaysLog(test1.getVertices()[0] == Vec2(2,8),  "Scaling operation failed");
    CUAssertAlwaysLog(test1.getVertices()[1] == Vec2(6,16), "Scaling operation failed");
    CUAssertAlwaysLog(test1.getVertices()[2] == Vec2(10,24),"Scaling operation failed");
    CUAssertAlwaysLog(test1.getVertices() == test2.getVertices(),   "Scaling operation failed");

    Affine2 atest;
    Affine2::createTranslation(5, 6, &atest);
    test1.set(vvec);
    test1 *= atest;
    test2 = test4*atest;
    CUAssertAlwaysLog(test1.getVertices()[0] == Vec2(6,8),  "Transform operation failed");
    CUAssertAlwaysLog(test1.getVertices()[1] == Vec2(8,10), "Transform operation failed");
    CUAssertAlwaysLog(test1.getVertices()[2] == Vec2(10,12),"Transform operation failed");
    CUAssertAlwaysLog(test1.getVertices() == test2.getVertices(),   "Transform operation failed");

    Mat4 mtest;
    Mat4::createTranslation(5, 6, 0, &mtest);
    test1.set(vvec);
    test1 *= mtest;
    test2 = test4*mtest;
    CUAssertAlwaysLog(test1.getVertices()[0] == Vec2(6,8),  "Transform operation failed");
    CUAssertAlwaysLog(test1.getVertices()[1] == Vec2(8,10), "Transform operation failed");
    CUAssertAlwaysLog(test1.getVertices()[2] == Vec2(10,12),"Transform operation failed");
    CUAssertAlwaysLog(test1.getVertices() == test2.getVertices(),   "Transform operation failed");

    test1.set(vvec);
    test1 += 6;
    test2 = test4+6;
    CUAssertAlwaysLog(test1.getVertices()[0] == Vec2(7,8),  "Translation operation failed");
    CUAssertAlwaysLog(test1.getVertices()[1] == Vec2(9,10), "Translation operation failed");
    CUAssertAlwaysLog(test1.getVertices()[2] == Vec2(11,12),"Translation operation failed");
    CUAssertAlwaysLog(test1.getVertices() == test2.getVertices(),   "Translation operation failed");

    test1.set(vvec);
    test1 += Vec2(5,6);
    test2 = test4+Vec2(5,6);
    CUAssertAlwaysLog(test1.getVertices()[0] == Vec2(6,8),  "Translation operation failed");
    CUAssertAlwaysLog(test1.getVertices()[1] == Vec2(8,10), "Translation operation failed");
    CUAssertAlwaysLog(test1.getVertices()[2] == Vec2(10,12),"Translation operation failed");
    CUAssertAlwaysLog(test1.getVertices() == test2.getVertices(),   "Translation operation failed");

    test1.set(vvec);
    test1 -= 1;
    test2 = test4-1;
    CUAssertAlwaysLog(test1.getVertices()[0] == Vec2(0,1),  "Translation operation failed");
    CUAssertAlwaysLog(test1.getVertices()[1] == Vec2(2,3),  "Translation operation failed");
    CUAssertAlwaysLog(test1.getVertices()[2] == Vec2(4,5),  "Translation operation failed");
    CUAssertAlwaysLog(test1.getVertices() == test2.getVertices(),   "Translation operation failed");
    
    test1.set(vvec);
    test1 -= Vec2(1,2);
    test2 = test4-Vec2(1,2);
    CUAssertAlwaysLog(test1.getVertices()[0] == Vec2(0,0),  "Translation operation failed");
    CUAssertAlwaysLog(test1.getVertices()[1] == Vec2(2,2),  "Translation operation failed");
    CUAssertAlwaysLog(test1.getVertices()[2] == Vec2(4,4),  "Translation operation failed");
    CUAssertAlwaysLog(test1.getVertices() == test2.getVertices(),   "Translation operation failed");

#pragma mark Geometry Test
    Poly2::createEllipse(Vec2::ZERO, Size(1,1), 8, &test4);
    Poly2::createEllipse(Vec2::ZERO, Size(1,1), 8, &test5, false);
    
    test1.set(rect);
    std::vector<Vec2> hull = test1.convexHull();
    CUAssertAlwaysLog(test1.getVertices() == hull,                  "Method convexHull() failed");
    hull = test4.convexHull();
    CUAssertAlwaysLog(test4.getVertices().size() == hull.size()+1,  "Method convexHull() failed");
    hull = test5.convexHull();
    CUAssertAlwaysLog(test5.getVertices().size() == hull.size(),    "Method convexHull() failed");
    
    CUAssertAlwaysLog(test1.contains(Vec2::ZERO),           "Method contains() failed");
    CUAssertAlwaysLog(test1.contains(Vec2(5,5)),            "Method contains() failed");
    CUAssertAlwaysLog(test1.contains(Vec2(10,10)),          "Method contains() failed");
    CUAssertAlwaysLog(!test1.contains(-Vec2::ONE),          "Method contains() failed");
    CUAssertAlwaysLog(test4.contains(Vec2::ZERO),           "Method contains() failed");
    CUAssertAlwaysLog(!test5.contains(Vec2::ZERO),          "Method contains() failed");
    CUAssertAlwaysLog(test4.contains(Vec2(0.5,0)),          "Method contains() failed");
    CUAssertAlwaysLog(!test4.contains(Vec2(0.5,0.5)),       "Method contains() failed");

    CUAssertAlwaysLog(test1.incident(Vec2::ZERO),           "Method incident() failed");
    CUAssertAlwaysLog(!test1.incident(Vec2(5,5)),           "Method incident() failed");
    CUAssertAlwaysLog(test1.incident(Vec2(10,10)),          "Method incident() failed");
    CUAssertAlwaysLog(!test1.incident(-Vec2::ONE),          "Method incident() failed");
    CUAssertAlwaysLog(!test4.incident(Vec2::ZERO),          "Method incident() failed");
    CUAssertAlwaysLog(!test5.incident(Vec2::ZERO),          "Method incident() failed");
    CUAssertAlwaysLog(test4.incident(Vec2(0.5,0)),          "Method incident() failed");
    CUAssertAlwaysLog(test5.incident(Vec2(0.5,0)),          "Method incident() failed");

#pragma mark Complete
    CULog("Poly2 tests complete.\n");
    
}

#pragma mark -
#pragma mark Polynomial
/**
 * Unit test for a polynomial equation with root solver
 */
void cugl::testPolynomial() {
    CULog("Running tests for Polynomial.\n");
    
#pragma mark Constructor Test
    float values[4] = {2.0f,2.0f,-2.0f,4.0f};
    Polynomial test1;
    CUAssertLog(test1.size() == 1 && test1[0] == 0, "Trivial constructor failed");
    Polynomial test2(2);
    CUAssertLog(test2.size() == 3 && test2[0] == 1, "Mononomial constructor failed");
    Polynomial test3(0,3.0f);
    CUAssertLog(test3.size() == 1 && test3[0] == 3, "Single element constructor failed");
    Polynomial test4(values,4);
    CUAssertLog(test4.size() == 4 && test4[0] == 2, "Array constructor failed");
    Polynomial test5(test4);
    CUAssertLog(test5 == test4,                     "Copy constructor failed");
    Polynomial test6(test4.begin(),test4.end());
    CUAssertLog(test6 == test4,                     "Iterator constructor failed");
    
#pragma mark Constant Test
    CUAssertLog(Polynomial::ZERO.size() == 1 && Polynomial::ZERO[0] == 0,   "Zero constant failed");
    CUAssertLog(Polynomial::ONE.size() == 1 && Polynomial::ONE[0] == 1,     "One constant failed");
    
#pragma mark Setter Test
    test5 = 3;
    CUAssertLog(test5.size() == 1 && test5[0] == 3, "Float assignment failed");
    
    test5 = test4; // To scramble the data.
    test5.set(3);
    CUAssertLog(test5.size() == 1 && test5[0] == 3, "Alternate float assignment failed");
    
    float longest[7] = {1.0f,0.0f,0.0f,0.0f,0.0f,0.0f,1.0f};
    test5.set(longest,7);
    CUAssertLog(test5.size() == 7 && test5[0] == 1, "Float array assignment failed");
    
#pragma mark Attribute Test
    CUAssertLog(test1.degree()+1 == test1.size(),   "Method degree() failed");
    CUAssertLog(test2.degree()+1 == test2.size(),   "Method degree() failed");
    CUAssertLog(test3.degree()+1 == test3.size(),   "Method degree() failed");
    CUAssertLog(test4.degree()+1 == test4.size(),   "Method degree() failed");
    CUAssertLog(test5.degree()+1 == test5.size(),   "Method degree() failed");
    
    CUAssertLog(test1.isZero(),                     "Method isZero() failed");
    CUAssertLog(!test2.isZero(),                    "Method isZero() failed");
    CUAssertLog(!test3.isZero(),                    "Method isZero() failed");
    CUAssertLog(!test4.isZero(),                    "Method isZero() failed");
    
    CUAssertLog(test1.isConstant(),                 "Method isConstant() failed");
    CUAssertLog(!test2.isConstant(),                "Method isConstant() failed");
    CUAssertLog(test3.isConstant(),                 "Method isConstant() failed");
    CUAssertLog(!test4.isConstant(),                "Method isConstant() failed");
    
    test5[0] = 0;
    CUAssertLog(test1.isValid(),                    "Method isValid() failed");
    CUAssertLog(test2.isValid(),                    "Method isValid() failed");
    CUAssertLog(test3.isValid(),                    "Method isValid() failed");
    CUAssertLog(test4.isValid(),                    "Method isValid() failed");
    CUAssertLog(!test5.isValid(),                   "Method isValid() failed");
    
#pragma mark Comparison Test
    CUAssertLog(test1 == test1,                     "Equals failed");
    CUAssertLog(test4 == test4,                     "Equals failed");
    CUAssertLog(!(test1 == test2),                  "Equals failed");
    CUAssertLog(!(test4 == test2),                  "Equals failed");
    CUAssertLog(!(test1 == test3),                  "Equals failed");
    CUAssertLog(!(test1 == 3),                      "Equals failed");
    CUAssertLog(test3 == 3,                         "Equals failed");
    CUAssertLog(!(test4 == 3),                      "Equals failed");
    
    
    CUAssertLog(!(test1 != test1),                  "Not Equals failed");
    CUAssertLog(!(test4 != test4),                  "Not Equals failed");
    CUAssertLog(test1 != test2,                     "Not Equals failed");
    CUAssertLog(test4 != test2,                     "Not Equals failed");
    CUAssertLog(test1 != test3,                     "Not Equals failed");
    CUAssertLog(test1 != 3,                         "Not Equals failed");
    CUAssertLog(!(test3 != 3),                      "Not Equals failed");
    CUAssertLog(test4 != 3,                         "Not Equals failed");
    
    CUAssertLog(!(test1 < test1),                   "Less than failed");
    CUAssertLog(!(test4 < test4),                   "Less than failed");
    CUAssertLog(!(test4 < test1),                   "Less than failed");
    CUAssertLog(test1 < test4,                      "Less than failed");
    CUAssertLog(test1 < test3,                      "Less than failed");
    CUAssertLog(test2 < test4,                      "Less than failed");
    CUAssertLog(test1 < 3,                          "Less than failed");
    CUAssertLog(!(test3 < 3),                       "Less than failed");
    CUAssertLog(!(test4 < 3),                       "Less than failed");
    CUAssertLog(!(3 < test3),                       "Less than failed");
    CUAssertLog(3 < test4,                          "Less than failed");
    
    CUAssertLog(test1 <= test1,                     "Less than or equal failed");
    CUAssertLog(test4 <= test4,                     "Less than or equal failed");
    CUAssertLog(!(test4 <= test1),                  "Less than or equal failed");
    CUAssertLog(test1 <= test4,                     "Less than or equal failed");
    CUAssertLog(test1 <= test3,                     "Less than or equal failed");
    CUAssertLog(test2 <= test4,                     "Less than or equal failed");
    CUAssertLog(test1 <= 3,                         "Less than or equal failed");
    CUAssertLog(test3 <= 3,                         "Less than or equal failed");
    CUAssertLog(!(test4 <= 3),                      "Less than or equal failed");
    CUAssertLog(3 <= test3,                         "Less than or equal failed");
    CUAssertLog(3 <= test4,                         "Less than or equal failed");
    
    CUAssertLog(!(test1 > test1),                   "Greater than failed");
    CUAssertLog(!(test4 > test4),                   "Greater than failed");
    CUAssertLog(!(test1 > test4),                   "Greater than failed");
    CUAssertLog(test4 > test1,                      "Greater than failed");
    CUAssertLog(test3 > test1,                      "Greater than failed");
    CUAssertLog(test4 > test2,                      "Greater than failed");
    CUAssertLog(!(test1 > 3),                       "Greater than failed");
    CUAssertLog(!(test3 > 3),                       "Greater than failed");
    CUAssertLog(test4 > 3,                          "Greater than failed");
    CUAssertLog(3 > test1,                          "Greater than failed");
    CUAssertLog(!(3 > test3),                       "Greater than failed");
    CUAssertLog(!(3 > test4),                       "Greater than failed");
    
    CUAssertLog(test1 >= test1,                     "Greater than or equal failed");
    CUAssertLog(test4 >= test4,                     "Greater than or equal failed");
    CUAssertLog(!(test1 >= test4),                  "Greater than or equal failed");
    CUAssertLog(test4 >= test1,                     "Greater than or equal failed");
    CUAssertLog(test3 >= test1,                     "Greater than or equal failed");
    CUAssertLog(test4 >= test2,                     "Greater than or equal failed");
    CUAssertLog(!(test1 >= 3),                      "Greater than or equal failed");
    CUAssertLog(test3 >= 3,                         "Greater than or equal failed");
    CUAssertLog(test4 >= 3,                         "Greater than or equal failed");
    CUAssertLog(3 >= test1,                         "Greater than or equal failed");
    CUAssertLog(3 >= test3,                         "Greater than or equal failed");
    CUAssertLog(!(3 >= test4),                      "Greater than or equal failed");
    
#pragma mark Operator Test
    test6 += test4;
    CUAssertLog(test6.degree() == 3 && test6[0] == 4,   "Addition operator failed");
    CUAssertLog(test4 + test4 == test6,                 "Addition operator failed");
    
    test6 = test2;
    test6 += test3;
    CUAssertLog(test6.degree() == 2 && test6[2] == 3,   "Addition operator failed");
    CUAssertLog(test2 + test3 == test6,                 "Addition operator failed");
    CUAssertLog(test3 + test2 == test6,                 "Addition operator failed");
    
    test6 = test2;
    test6 += 4;
    CUAssertLog(test6.degree() == 2 && test6[2] == 4,   "Addition operator failed");
    CUAssertLog(test2 + 4 == test6,                     "Addition operator failed");
    CUAssertLog(4 + test2 == test6,                     "Addition operator failed");
    
    test6 = test4;
    test6 -= test4;
    CUAssertLog(test6 == Polynomial::ZERO,              "Subtraction operator failed");
    CUAssertLog(test4 - test4 == Polynomial::ZERO,      "Subtraction operator failed");
    
    test6 = test2;
    test6 -= test3;
    CUAssertLog(test6.degree() == 2 && test6[2] == -3,  "Subtraction operator failed");
    CUAssertLog(test2 - test3 == test6,                 "Subtraction operator failed");
    CUAssertLog(test3 - test2 != test6,                 "Subtraction operator failed");
    
    test6 = test2;
    test6 -= 4;
    CUAssertLog(test6.degree() == 2 && test6[2] == -4,  "Subtraction operator failed");
    CUAssertLog(test2 - 4 == test6,                     "Subtraction operator failed");
    CUAssertLog(4 - test2 != test6,                     "Subtraction operator failed");
    
    test6 = -test4;
    CUAssertLog(test6.degree() == 3 && test6[0] == -2,  "Negation operator failed");
    CUAssertLog(-test6 ==test4,                         "Negation operator failed");
    
    test6 = test4;
    test6 *= test2;
    CUAssertLog(test6.degree() == test4.degree()+2 &&
                test6[0] == 2,                          "Multiplication operator failed");
    CUAssertLog(test4 * test2 == test6,                 "Multiplication operator failed");
    CUAssertLog(test2 * test4 == test6,                 "Multiplication operator failed");
    
    test6 = test2;
    test6 *= test2;
    CUAssertLog(test6.degree() == 4 && test6[0] == 1,   "Multiplication operator failed");
    CUAssertLog(test2 * test2 == test6,                 "Multiplication operator failed");
    
    test6 = test4;
    test6 *= 4;
    CUAssertLog(test6.degree() == test4.degree() &&
                test6[0] == 8,                          "Multiplication operator failed");
    CUAssertLog(test4 * 4 == test6,                     "Multiplication operator failed");
    CUAssertLog(4 * test4 == test6,                     "Multiplication operator failed");
    
    test5.set(longest,7);
    test6 = test4*test2;
    test6 /= test2;
    CUAssertLog(test6 == test4,                         "Division operator failed");
    CUAssertLog((test4 * test2)/test2 == test4,         "Division operator failed");
    test6 /= test5;
    CUAssertLog(test6 == Polynomial::ZERO,              "Division operator failed");
    CUAssertLog(test4/test5 == Polynomial::ZERO,        "Division operator failed");
    
    test6 = test4;
    test6 /= 2;
    CUAssertLog(test6.size() == test4.size() && test6[0] == 1,  "Division operator failed");
    CUAssertLog(test4/2 == test6,                       "Division operator failed");
    CUAssertLog(2/test4 == Polynomial::ZERO,            "Division operator failed");
    
    test6 = test4*test2;
    test6 %= test2;
    CUAssertLog(test6 == Polynomial::ZERO,              "Mod operator failed");
    test6 = test4*test2;
    CUAssertLog(test6 % test2 == Polynomial::ZERO,      "Mod operator failed");
    
    test6 = test4;
    test6 %= test5;
    CUAssertLog(test6 == test4,                         "Mod operator failed");
    CUAssertLog(test4 % test5 == test4,                 "Mod operator failed");
    
    test6 = test4;
    test6 %= 2;
    CUAssertLog(test6 == Polynomial::ZERO,              "Mod operator failed");
    CUAssertLog(test4 % 2 == Polynomial::ZERO,          "Mod operator failed");
    test6 = 2 % test4;
    CUAssertLog(2 % test4 == 2,                         "Mod operator failed");
    
#pragma mark Calculation Test
    test6 = test4.derivative();
    CUAssertLog(test6.degree() == test4.degree()-1 && test6[0] == 6,"Method derivative() failed");
    test6 = test3.derivative();
    CUAssertLog(test6 == Polynomial::ZERO,                          "Method derivative() failed");
    CUAssertLog(Polynomial::ZERO.derivative() == Polynomial::ZERO,  "Method derivative() failed");
    
    test5.set(1);
    test5.push_back(3);
    test6.set(1);
    test6.push_back(-3);
    CUAssertLog(test3.evaluate(2) == 3,             "Method evaluate() failed");
    CUAssertLog(test5.evaluate(2) == 5,             "Method evaluate() failed");
    CUAssertLog(test6.evaluate(2) == -1,            "Method evaluate() failed");
    test6 *= test5;
    CUAssertLog(test6.evaluate(2) == -5,            "Method evaluate() failed");
    
    std::vector<float> roots;
    CUAssertLog(test6.roots(roots),                 "Method roots() failed");
    CUAssertLog(roots.size() == 2,                  "Method roots() failed");
    CUAssertLog(roots[0] == 3 || roots[1] == 3,     "Method roots() failed");
    
    test6[2] = -test6[2];
    roots.clear();
    CUAssertLog(test6.roots(roots),                 "Method roots() failed");
    CUAssertLog(roots.size() == 2,                  "Method roots() failed");
    
    test6[0] = 0;
    test6.validate();
    CUAssertLog(test6 == 9,                         "Method validate() failed");
    
    CUAssertLog(test4.normalize() == 2,             "Method normalize() failed");
    CUAssertLog(test4.degree() == 3 && test4[0] == 1,   "Method normalize() failed");
    
    
#pragma mark Complete
    CULog("Polynomial tests complete.\n");
    
}

#pragma mark -
#pragma mark Ray
/**
 * Unit test for a 3-dimensional ray
 */
void cugl::testRay() {
    CULog("Running tests for Ray.\n");
    
#pragma mark Constructor Test
    Ray test1;
    CUAssertAlwaysLog(test1.origin == Vec3::ZERO && test1.direction == Vec3::UNIT_X,
                      "Trivial constructor failed");

    Ray test2(Vec3(0,2,0));
    CUAssertAlwaysLog(test2.origin == Vec3::ZERO && test2.direction == Vec3::UNIT_Y,
                      "Direction constructor failed");

    Ray test3(Vec3::ONE, Vec3(0,0,2));
    CUAssertAlwaysLog(test3.origin == Vec3::ONE && test3.direction == Vec3::UNIT_Z,
                      "Initialization constructor failed");

    Ray test4(test3);
    CUAssertAlwaysLog(test4.origin == Vec3::ONE && test4.direction == Vec3::UNIT_Z,
                      "Initialization constructor failed");

#pragma mark Constants Test
    CUAssertAlwaysLog(Ray::X_AXIS.origin == Vec3::ZERO && Ray::X_AXIS.direction == Vec3::UNIT_X,
                      "Ray for x-axis failed");
    CUAssertAlwaysLog(Ray::Y_AXIS.origin == Vec3::ZERO && Ray::Y_AXIS.direction == Vec3::UNIT_Y,
                      "Ray for y-axis failed");
    CUAssertAlwaysLog(Ray::Z_AXIS.origin == Vec3::ZERO && Ray::Z_AXIS.direction == Vec3::UNIT_Z,
                      "Ray for z-axis failed");
    
#pragma mark Setter Test
    test1 = test4;
    CUAssertAlwaysLog(test1.origin==test4.origin && test1.direction==test4.direction,
                      "Basic assignment failed");
    test1 = 2*Vec3::UNIT_X;
    CUAssertAlwaysLog(test1.origin==Vec3::ZERO && test1.direction==Vec3::UNIT_X,
                      "Directional assignment failed");
    test1.set(Vec3::ONE, Vec3(0,0,2));
    CUAssertAlwaysLog(test1.origin == Vec3::ONE && test1.direction == Vec3::UNIT_Z,
                      "Parameter assignment failed");
    test1.set(test4);
    CUAssertAlwaysLog(test1.origin==test4.origin && test1.direction==test4.direction,
                      "Alternate assignment failed");
    test1.set(2*Vec3::UNIT_X);
    CUAssertAlwaysLog(test1.origin==Vec3::ZERO && test1.direction==Vec3::UNIT_X,
                      "Alternate directional assignment failed");

#pragma mark Static Arithmetic Test
    
    Vec3 v3test;
    Vec3* v3testptr = Ray::endpoint(test4,3,&v3test);
    CUAssertAlwaysLog(v3testptr == &v3test,         "Ray::endpoint() failed");
    CUAssertAlwaysLog(v3test == Vec3(1,1,4),        "Ray::endpoint() failed");
    
    Mat4 mtest = Mat4::createRotationY(M_PI_4);
    Ray* testptr =  Ray::multiply(test4,mtest,&test1);
    CUAssertAlwaysLog(testptr == &test1,            "Ray::multiply() failed");
    
    v3test = Vec3(Vec4(test4.direction,0)*mtest);
    CUAssertAlwaysLog(test1.origin==test4.origin*mtest && test1.direction.equals(v3test),
                      "Ray::multiply() failed");

#pragma mark Arithmetic Test
    test1.set(test4);
    v3test = test1.getEndpoint(3);
    CUAssertAlwaysLog(v3test == Vec3(1,1,4),        "Method getEndpoint() failed");

    test1.multiply(mtest);
    v3test = Vec3(Vec4(test4.direction,0)*mtest);
    CUAssertAlwaysLog(test1.origin==test4.origin*mtest && test1.direction.equals(v3test),
                      "Method multiply() failed");

#pragma mark Operator Test
    
    test1.set(test4);
    test1 *= mtest;
    CUAssertAlwaysLog(test1.origin==test4.origin*mtest && test1.direction.equals(v3test),
                      "Multiply opertion failed");
    CUAssertAlwaysLog((test4*mtest).origin==test4.origin*mtest &&
                      (test4*mtest).direction.equals(v3test),
                      "Multiply opertion failed");

    v3test = test4*3;
    CUAssertAlwaysLog(v3test == Vec3(1,1,4),
                      "Multiply opertion failed");

#pragma mark Comparison Test
    test1.set(Vec3::ZERO,Vec3::UNIT_X);
    test2.set(Vec3::ONE,Vec3::UNIT_X);
    test3.set(Vec3::ONE,Vec3::UNIT_Y);
    test4.set(Vec3::ZERO,Vec3::UNIT_X);
    
    Ray test5(test4);
    test5.origin.x += CU_MATH_EPSILON/2.0f;
    test5.direction.y += CU_MATH_EPSILON/2.0f;
    
    CUAssertAlwaysLog(test1 == test1,       "Equals failed");
    CUAssertAlwaysLog(!(test1 == test2),    "Equals failed");
    CUAssertAlwaysLog(!(test1 == test3),    "Equals failed");
    CUAssertAlwaysLog(test1 == test4,       "Equals failed");

    CUAssertAlwaysLog(!(test1 != test1),    "Not equals failed");
    CUAssertAlwaysLog(test1 != test2,       "Not equals failed");
    CUAssertAlwaysLog(test1 != test3,       "Not equals failed");
    CUAssertAlwaysLog(!(test1 != test4),    "Not equals failed");
    
    CUAssertAlwaysLog(test1.equals(test1),  "Approximate equals failed");
    CUAssertAlwaysLog(test1.equals(test5),  "Approximate equals failed");
    CUAssertAlwaysLog(!test1.equals(test3), "Approximate equals failed");

#pragma mark Conversion Test
    std::string str1;
    
    std::string str2 = "(origin:";
    str2 +=  test4.origin.toString();
    str2 += ",direction:";
    str2 +=  test4.direction.toString();
    str2 += ")";
    
    str1 = test4.toString();
    CUAssertAlwaysLog(str1 == str2,                 "Method toString() failed");
    str1 = test4.toString(true);
    CUAssertAlwaysLog(str1 == "cugl::Ray"+str2,     "Method toString() failed");
    str1 = (std::string)test4;
    CUAssertAlwaysLog(str1 == str2,                 "String cast failed");
    
#pragma mark Complete
    CULog("Ray tests complete.\n");
}

#pragma mark -
#pragma mark Plane
/**
 * Unit test for a plane in 3d space
 */
void cugl::testPlane() {
    CULog("Running tests for Plane.\n");

#pragma mark Constructor Test
    Plane test1;
    CUAssertAlwaysLog(test1.normal == Vec3::UNIT_Z && test1.offset == 0,
                      "Trivial constructor failed");
    
    Plane test2(Vec3::ONE, 2);
    CUAssertAlwaysLog(test2.normal == Vec3::ONE.getNormalization() && test2.offset == 2,
                      "Initialization constructor failed");

    Plane test3(Vec3::ONE, Vec3::ONE);
    CUAssertAlwaysLog(test3.normal == Vec3::ONE.getNormalization() && test3.offset == -sqrtf(3),
                      "Point in plane constructor failed");

    Plane test4(Vec3::UNIT_X, Vec3::UNIT_Y, Vec3::UNIT_Z);
    CUAssertAlwaysLog(test4.normal == Vec3::ONE.getNormalization() && test4.offset == -1/sqrtf(3),
                      "Three point constructor failed");

    Plane test5(2,2,2,sqrtf(12));
    CUAssertAlwaysLog(test5.normal == Vec3::ONE.getNormalization() && test5.offset == 1,
                      "Equational constructor failed");

    Plane test6(test5);
    CUAssertAlwaysLog(test6.normal == test5.normal && test6.offset == test5.offset,
                      "Copy constructor failed");

#pragma mark Constant Test
    CUAssertAlwaysLog(Plane::XY.normal == Vec3::UNIT_Z && Plane::XY.offset == 0,
                      "XY plane failed");
    CUAssertAlwaysLog(Plane::YZ.normal == Vec3::UNIT_X && Plane::YZ.offset == 0,
                      "XY plane failed");
    CUAssertAlwaysLog(Plane::XZ.normal == Vec3::UNIT_Y && Plane::XZ.offset == 0,
                      "XY plane failed");

#pragma mark Setter Test
    test1 = test5;
    CUAssertAlwaysLog(test1.normal == test5.normal && test1.offset == test5.offset,
                      "Basic assignment failed");

    test1 = Vec3::ONE;
    CUAssertAlwaysLog(test1.normal == test2.normal && test1.offset == 0,
                      "Normal vector assignment failed");

    test1.set(Vec3::UNIT_X,2);
    CUAssertAlwaysLog(test1.normal == Vec3::UNIT_X && test1.offset == 2,
                      "Parameter assignment failed");

    test1.set(Vec3::ONE,Vec3::ONE);
    CUAssertAlwaysLog(test1.normal == test3.normal && test1.offset == test3.offset,
                      "Point in place assignment failed");

    test1.set(Vec3::UNIT_X, Vec3::UNIT_Y, Vec3::UNIT_Z);
    CUAssertAlwaysLog(test1.normal == test4.normal && test1.offset == test4.offset,
                      "Three point assignment failed");

    test1.set(2,2,2,sqrtf(12));
    CUAssertAlwaysLog(test1.normal == test5.normal && test1.offset == test5.offset,
                      "Equational assignment failed");

    test1.set(test4);
    CUAssertAlwaysLog(test1.normal == test4.normal && test1.offset == test4.offset,
                      "Alternate assignment failed");
    
#pragma mark Static Arithmetic Test
    Mat4 mtest = Mat4::createRotationY(-M_PI_2);
    Plane* testptr = Plane::multiply(Plane::XY,mtest,&test1);
    CUAssertAlwaysLog(testptr == &test1,            "Plane::multiply() failed");
    CUAssertAlwaysLog(test1.equals(Plane::YZ),      "Plane::multiply() failed");
    
    test1.set(Vec3::UNIT_Z,3);
    Ray rtest(Vec3::ONE, -4*Vec3::UNIT_Z);
    float value = Plane::intersection(test1,rtest);
    CUAssertAlwaysLog(CU_MATH_APPROX(value, 4, CU_MATH_EPSILON),
                      "Plane::intersection() failed");
    CUAssertAlwaysLog(rtest.getEndpoint(value).equals(Vec3(1,1,-3)),
                      "Plane::intersection() failed");
 
#pragma mark Comparison Test
    test1.set(Vec3::UNIT_X,2);
    test2.set(Vec3::UNIT_X,3);
    test3.set(Vec3::UNIT_Y,2);
    test4.set(Vec3::UNIT_X,2);
    
    test5 = test4;
    test5.normal.x += CU_MATH_EPSILON/2.0f;
    test5.offset += CU_MATH_EPSILON/2.0f;
    
    CUAssertAlwaysLog(test1 == test1,       "Equals failed");
    CUAssertAlwaysLog(!(test1 == test2),    "Equals failed");
    CUAssertAlwaysLog(!(test1 == test3),    "Equals failed");
    CUAssertAlwaysLog(test1 == test4,       "Equals failed");
    
    CUAssertAlwaysLog(!(test1 != test1),    "Not equals failed");
    CUAssertAlwaysLog(test1 != test2,       "Not equals failed");
    CUAssertAlwaysLog(test1 != test3,       "Not equals failed");
    CUAssertAlwaysLog(!(test1 != test4),    "Not equals failed");
    
    CUAssertAlwaysLog(test1.equals(test1),  "Approximate equals failed");
    CUAssertAlwaysLog(test1.equals(test5),  "Approximate equals failed");
    CUAssertAlwaysLog(!test1.equals(test3), "Approximate equals failed");

#pragma mark Arithmetic Test
    test1 = Plane::XY;
    test1.multiply(mtest);
    CUAssertAlwaysLog(test1.equals(Plane::YZ),              "Method multiply() failed");
    test1 = Plane::XY;
    test1 *= mtest;
    CUAssertAlwaysLog(test1.equals(Plane::YZ),              "Mutliplication operation failed");
    CUAssertAlwaysLog((Plane::XY*mtest).equals(Plane::YZ),  "Mutliplication operation failed");

#pragma mark Method Test
    value = Plane::XY.distance(Vec3::ONE);
    CUAssertAlwaysLog( value == 1,                          "Method distance() failed");

    test1.set(Vec3::UNIT_Z,3);
    value = test1.getIntersection(rtest);
    CUAssertAlwaysLog(CU_MATH_APPROX(value, 4, CU_MATH_EPSILON),
                      "Method getIntersection() failed");
    CUAssertAlwaysLog(rtest.getEndpoint(value).equals(Vec3(1,1,-3)),
                      "Method getIntersection() failed");

    CUAssertAlwaysLog(Plane::XY.contains(Vec3::UNIT_X),     "Method contains() failed");
    CUAssertAlwaysLog(Plane::XY.contains(Vec3::UNIT_Y),     "Method contains() failed");
    CUAssertAlwaysLog(Plane::XY.contains(Vec3(1,1,0)),      "Method contains() failed");
    CUAssertAlwaysLog(!Plane::XY.contains(Vec3::ONE),       "Method contains() failed");
    CUAssertAlwaysLog(!Plane::XY.contains(Vec3::UNIT_Z),     "Method contains() failed");

    CUAssertAlwaysLog(Plane::XY.sideOf(Vec3::UNIT_X) == Plane::Side::INCIDENT, "Method side() failed");
    CUAssertAlwaysLog(Plane::XY.sideOf(Vec3::ONE) == Plane::Side::FRONT,       "Method side() failed");
    CUAssertAlwaysLog(Plane::XY.sideOf(-Vec3::ONE) == Plane::Side::BACK,       "Method side() failed");

    CUAssertAlwaysLog(Plane::XY.isFrontFacing(Vec3::UNIT_X),    "Method isFrontFacing() failed");
    CUAssertAlwaysLog(!Plane::XY.isFrontFacing(Vec3::ONE),      "Method isFrontFacing() failed");
    CUAssertAlwaysLog(Plane::XY.isFrontFacing(-Vec3::ONE),      "Method .isFrontFacing() failed");

#pragma mark Conversion Test
    test1.set(Vec3::ONE,1);
    std::string str1;
    
    Vec3 norm = Vec3::ONE.getNormalization();
    std::string str2 = "[";
    str2 += cugl::to_string(norm.x);
    str2 +=  "x+";
    str2 += cugl::to_string(norm.y);
    str2 +=  "y+";
    str2 += cugl::to_string(norm.z);
    str2 +=  "z = ";
    str2 += cugl::to_string(1.0f);
    str2 += "]";
    
    str1 = test1.toString();
    CUAssertAlwaysLog(str1 == str2,                     "Method toString() failed");
    str1 = test1.toString(true);
    CUAssertAlwaysLog(str1 == "cugl::Plane"+str2,       "Method toString() failed");
    str1 = (std::string)test1;
    CUAssertAlwaysLog(str1 == str2,                     "String cast failed");

#pragma mark Complete
    CULog("Plane tests complete.\n");
}

#pragma mark -
#pragma mark Plane
/**
 * Unit test for a viewing frustrum
 */
void cugl::testFrustum() {
    CULog("Running tests for Frustrum.\n");
    
// These are just some token tests
#pragma mark Constructor Test
    Frustum test1;
    Plane ptest = test1.getPlane(Frustum::Side::CLOSE);
    CUAssertAlwaysLog(test1.getPlane(Frustum::Side::CLOSE)  == Plane(Vec3::UNIT_Z, 1),
                      "Trivial constructor failed");
    CUAssertAlwaysLog(test1.getPlane(Frustum::Side::AWAY)   == Plane(-Vec3::UNIT_Z, 1),
                      "Trivial constructor failed");
    CUAssertAlwaysLog(test1.getPlane(Frustum::Side::LEFT)   == Plane(Vec3::UNIT_X, 1),
                      "Trivial constructor failed");
    CUAssertAlwaysLog(test1.getPlane(Frustum::Side::RIGHT)  == Plane(-Vec3::UNIT_X, 1),
                      "Trivial constructor failed");
    CUAssertAlwaysLog(test1.getPlane(Frustum::Side::BOTTOM) == Plane(Vec3::UNIT_Y, 1),
                      "Trivial constructor failed");
    CUAssertAlwaysLog(test1.getPlane(Frustum::Side::TOP)    == Plane(-Vec3::UNIT_Y, 1),
                      "Trivial constructor failed");
    
    
#pragma mark Containment Test
    Mat4 mtest = Mat4::createOrthographic(100,100,10,-10);
    mtest.invert();
    
    test1.set(mtest);
    CUAssertAlwaysLog(test1.find(Vec3::ZERO)        == Frustum::Region::INSIDE,
                      "Method find() failed");
    CUAssertAlwaysLog(test1.find(Vec3(49,49,5))     == Frustum::Region::INSIDE,
                      "Method find() failed");
    CUAssertAlwaysLog(test1.find(Vec3(200,200,0))   == Frustum::Region::OUTSIDE,
                      "Method contains() failed");
    CUAssertAlwaysLog(test1.find(Vec3(0,0,20))      == Frustum::Region::OUTSIDE,
                      "Method find() failed");

    CUAssertAlwaysLog(test1.findSphere(Vec3::ZERO,5)    == Frustum::Region::INSIDE,
                      "Method findSphere() failed");
    CUAssertAlwaysLog(test1.findSphere(Vec3(300,0,0),5) == Frustum::Region::OUTSIDE,
                      "Method findSphere() failed");
    CUAssertAlwaysLog(test1.findSphere(Vec3::ZERO,20)   == Frustum::Region::INTERSECT,
                      "Method findSphere() failed");
    CUAssertAlwaysLog(test1.findSphere(Vec3::ZERO,300)  == Frustum::Region::INTERSECT,
                      "Method findSphere() failed");
    CUAssertAlwaysLog(test1.findSphere(Vec3(50,0,0),5)  == Frustum::Region::INTERSECT,
                      "Method findSphere() failed");

    CUAssertAlwaysLog(test1.findSphereWithoutNearFar(Vec3::ZERO,5)      == Frustum::Region::INSIDE,
                      "Method findSphereWithoutNearFar() failed");
    CUAssertAlwaysLog(test1.findSphereWithoutNearFar(Vec3(300,0,0),5)   == Frustum::Region::OUTSIDE,
                      "Method findSphereWithoutNearFar() failed");
    CUAssertAlwaysLog(test1.findSphereWithoutNearFar(Vec3::ZERO,20)     == Frustum::Region::INSIDE,
                      "Method findSphereWithoutNearFar() failed");
    CUAssertAlwaysLog(test1.findSphereWithoutNearFar(Vec3::ZERO,300)    == Frustum::Region::INTERSECT,
                      "Method findSphereWithoutNearFar() failed");
    CUAssertAlwaysLog(test1.findSphereWithoutNearFar(Vec3(50,0,0),5)    == Frustum::Region::INTERSECT,
                      "Method findSphereWithoutNearFar() failed");

    CUAssertAlwaysLog(test1.findBox(Vec3::ZERO,Vec3::ONE)       == Frustum::Region::INSIDE,
                      "Method findBox() failed");
    CUAssertAlwaysLog(test1.findBox(Vec3::ONE,Vec3::ONE)        == Frustum::Region::INSIDE,
                      "Method findBox() failed");
    CUAssertAlwaysLog(test1.findBox(Vec3(300,0,0),Vec3::ONE)    == Frustum::Region::OUTSIDE,
                      "Method findBox() failed");
    CUAssertAlwaysLog(test1.findBox(Vec3::ZERO,Vec3(30,30,30))  == Frustum::Region::INTERSECT,
                      "Method findBox() failed");
    CUAssertAlwaysLog(test1.findBox(Vec3(50,0,0),Vec3::ONE)     == Frustum::Region::INTERSECT,
                      "Method findBox() failed");

#pragma mark Complete
    CULog("Frustrum tests complete.\n");
}

#pragma mark -
#pragma mark DSP

#define ARRAY_SIZE 1024
#define LOOP_SIZE 1000

void cugl::testDSP() {
    CULog("Running tests for DSP math.\n");
    
    float* input1 = new float[ARRAY_SIZE];
    float* input2 = new float[ARRAY_SIZE];
    float* output1 = new float[ARRAY_SIZE];
    float* output2 = new float[ARRAY_SIZE];
    
    for(int ii = 0; ii < ARRAY_SIZE; ii++) {
        input1[ii] = sinf(ii * M_PI / 10.0f);
        input2[ii] = cosf(ii * M_PI / 10.0f);
        output1[ii] = 0;
        output2[ii] = 0;
    }

    cugl::Timestamp start, midl, end;
#pragma mark DSP Add
    int same = -1;

    start.mark();
    DSPMath::VECTORIZE = true;
    for(int ii = 0; ii < LOOP_SIZE; ii++) {
        DSPMath::add(input1,input2,output1,ARRAY_SIZE);
    }
    midl.mark();
    DSPMath::VECTORIZE = false;
    for(int ii = 0; ii < LOOP_SIZE; ii++) {
        DSPMath::add(input1,input2,output2,ARRAY_SIZE);
    }
    end.mark();

    same = -1;
    for(int ii = 0; same == -1 && ii < ARRAY_SIZE; ii++) {
        if (fabsf(output1[ii] - output2[ii]) >= CU_MATH_EPSILON) {
            same = ii;
        }
    }
    CUAssertAlwaysLog(same == -1, "%s failed at position %d [%f vs %f]",
                      "add",same,output1[same],output2[same]);

    CULog("%s time: %llu vs %llu micros","add",
          cugl::Timestamp::ellapsedMicros(start,midl),
          cugl::Timestamp::ellapsedMicros(midl,end));
    
    start.mark();
    DSPMath::VECTORIZE = true;
    for(int ii = 0; ii < LOOP_SIZE; ii++) {
        DSPMath::multiply(input1,input2,output1,ARRAY_SIZE);
    }
    midl.mark();
    DSPMath::VECTORIZE = false;
    for(int ii = 0; ii < LOOP_SIZE; ii++) {
        DSPMath::multiply(input1,input2,output2,ARRAY_SIZE);
    }
    end.mark();
    
    same = -1;
    for(int ii = 0; same == -1 && ii < ARRAY_SIZE; ii++) {
        if (fabsf(output1[ii] - output2[ii]) >= CU_MATH_EPSILON) {
            same = ii;
        }
    }
    CUAssertAlwaysLog(same == -1, "%s failed at position %d [%f vs %f]",
                      "mult",same,output1[same],output2[same]);
    
    CULog("%s time: %llu vs %llu micros","mult",
          cugl::Timestamp::ellapsedMicros(start,midl),
          cugl::Timestamp::ellapsedMicros(midl,end));

    start.mark();
    DSPMath::VECTORIZE = true;
    for(int ii = 0; ii < LOOP_SIZE; ii++) {
        DSPMath::scale(input1,2.0f,output1,ARRAY_SIZE);
    }
    midl.mark();
    DSPMath::VECTORIZE = false;
    for(int ii = 0; ii < LOOP_SIZE; ii++) {
        DSPMath::scale(input1,2.0f,output2,ARRAY_SIZE);
    }
    end.mark();
    
    same = -1;
    for(int ii = 0; same == -1 && ii < ARRAY_SIZE; ii++) {
        if (fabsf(output1[ii] - output2[ii]) >= CU_MATH_EPSILON) {
            same = ii;
        }
    }
    CUAssertAlwaysLog(same == -1, "%s failed at position %d [%f vs %f]",
                      "mult",same,output1[same],output2[same]);
    
    CULog("%s time: %llu vs %llu micros","scale",
          cugl::Timestamp::ellapsedMicros(start,midl),
          cugl::Timestamp::ellapsedMicros(midl,end));

    start.mark();
    DSPMath::VECTORIZE = true;
    for(int ii = 0; ii < LOOP_SIZE; ii++) {
        DSPMath::scale_add(input1,input2,2.0f,output1,ARRAY_SIZE);
    }
    midl.mark();
    DSPMath::VECTORIZE = false;
    for(int ii = 0; ii < LOOP_SIZE; ii++) {
        DSPMath::scale_add(input1,input2,2.0f,output2,ARRAY_SIZE);
    }
    end.mark();
    
    same = -1;
    for(int ii = 0; same == -1 && ii < ARRAY_SIZE; ii++) {
        if (fabsf(output1[ii] - output2[ii]) >= CU_MATH_EPSILON) {
            same = ii;
        }
    }
    CUAssertAlwaysLog(same == -1, "%s failed at position %d [%f vs %f]",
                      "scale_add",same,output1[same],output2[same]);

    CULog("%s time: %llu vs %llu micros","scale_add",
          cugl::Timestamp::ellapsedMicros(start,midl),
          cugl::Timestamp::ellapsedMicros(midl,end));
    
    start.mark();
    DSPMath::VECTORIZE = true;
    for(int ii = 0; ii < LOOP_SIZE; ii++) {
        DSPMath::slide(input1,0.0f,1.0f,output1,ARRAY_SIZE);
    }
    midl.mark();
    DSPMath::VECTORIZE = false;
    for(int ii = 0; ii < LOOP_SIZE; ii++) {
        DSPMath::slide(input1,0.0f,1.0f,output2,ARRAY_SIZE);
    }
    end.mark();
    
    same = -1;
    for(int ii = 0; same == -1 && ii < ARRAY_SIZE; ii++) {
        if (fabsf(output1[ii] - output2[ii]) >= CU_MATH_EPSILON) {
            same = ii;
        }
    }
    CUAssertAlwaysLog(same == -1, "%s failed at position %d [%f vs %f]",
                      "slide",same,output1[same],output2[same]);
    
    CULog("%s time: %llu vs %llu micros","slide",
          cugl::Timestamp::ellapsedMicros(start,midl),
          cugl::Timestamp::ellapsedMicros(midl,end));

    start.mark();
    DSPMath::VECTORIZE = true;
    for(int ii = 0; ii < LOOP_SIZE; ii++) {
        DSPMath::slide_add(input1,input2,0.0f,1.0f,output1,ARRAY_SIZE);
    }
    midl.mark();
    DSPMath::VECTORIZE = false;
    for(int ii = 0; ii < LOOP_SIZE; ii++) {
        DSPMath::slide_add(input1,input2,0.0f,1.0f,output2,ARRAY_SIZE);
    }
    end.mark();
    
    same = -1;
    for(int ii = 0; same == -1 && ii < ARRAY_SIZE; ii++) {
        if (fabsf(output1[ii] - output2[ii]) >= CU_MATH_EPSILON) {
            same = ii;
        }
    }
    CUAssertAlwaysLog(same == -1, "%s failed at position %d [%f vs %f]",
                      "slide_add",same,output1[same],output2[same]);
    
    CULog("%s time: %llu vs %llu micros","slide_add",
          cugl::Timestamp::ellapsedMicros(start,midl),
          cugl::Timestamp::ellapsedMicros(midl,end));
    
    std::memcpy(output1,input1,ARRAY_SIZE*sizeof(float));
    std::memcpy(output2,input1,ARRAY_SIZE*sizeof(float));
    start.mark();
    DSPMath::VECTORIZE = true;
    for(int ii = 0; ii < LOOP_SIZE; ii++) {
        DSPMath::clamp(output1,-0.25,0.5,ARRAY_SIZE);
    }
    midl.mark();
    DSPMath::VECTORIZE = false;
    for(int ii = 0; ii < LOOP_SIZE; ii++) {
        DSPMath::clamp(output2,-0.25,0.5,ARRAY_SIZE);
    }
    end.mark();
    
    same = -1;
    for(int ii = 0; same == -1 && ii < ARRAY_SIZE; ii++) {
        if (fabsf(output1[ii] - output2[ii]) >= CU_MATH_EPSILON) {
            same = ii;
        }
    }
    CUAssertAlwaysLog(same == -1, "%s failed at position %d [%f vs %f]",
                      "clamp",same,output1[same],output2[same]);
    
    CULog("%s time: %llu vs %llu micros","clamp",
          cugl::Timestamp::ellapsedMicros(start,midl),
          cugl::Timestamp::ellapsedMicros(midl,end));
    
    std::memcpy(output1,input1,ARRAY_SIZE*sizeof(float));
    std::memcpy(output2,input1,ARRAY_SIZE*sizeof(float));
    start.mark();
    DSPMath::VECTORIZE = true;
    for(int ii = 0; ii < LOOP_SIZE; ii++) {
        DSPMath::ease(output1,1.0,0.75,ARRAY_SIZE);
    }
    midl.mark();
    DSPMath::VECTORIZE = false;
    for(int ii = 0; ii < LOOP_SIZE; ii++) {
        DSPMath::ease(output2,1.0,0.75,ARRAY_SIZE);
    }
    end.mark();
    
    same = -1;
    for(int ii = 0; same == -1 && ii < ARRAY_SIZE; ii++) {
        if (fabsf(output1[ii] - output2[ii]) >= CU_MATH_EPSILON) {
            same = ii;
        }
    }
    CUAssertAlwaysLog(same == -1, "%s failed at position %d [%f vs %f]",
                      "ease",same,output1[same],output2[same]);
    
    CULog("%s time: %llu vs %llu micros","ease",
          cugl::Timestamp::ellapsedMicros(start,midl),
          cugl::Timestamp::ellapsedMicros(midl,end));
    
#pragma mark Complete
    delete[] input1;
    delete[] input2;
    delete[] output1;
    delete[] output2;
    CULog("DSP tests complete.\n");
}
    
struct dsprun {
    size_t size;
    size_t count;
    size_t stride;
    float gain;
    float* input;
    float* output;
    float* compare;
};

template<class T> static void dspStep(T& filter, dsprun& data, const char* ident, bool speed) {
    cugl::Timestamp start;
    start.mark();
    for(int ii = 0; ii < data.count; ii++) {
        for(int jj = 0; jj < data.size; jj++) {
            filter.step(data.gain, data.input+jj*data.stride, data.output+jj*data.stride);
        }
    }
    cugl::Timestamp end;
    
    filter.clear();
    if (data.compare != nullptr) {
        int same = -1;
        for(int ii = 0; same == -1 && ii < data.size*data.stride; ii++) {
            if (fabsf(data.output[ii] - data.compare[ii]) >= CU_MATH_EPSILON) {
                same = ii;
            }
        }
        CUAssertAlwaysLog(same == -1, "%s failed at position %d [%f vs %f for %f]",ident,same,data.output[same],data.compare[same],data.input[same]);
    }

    if (speed) {
        CULog("%s time: %llu micros",ident,cugl::Timestamp::ellapsedMicros(start,end));
    }
}

template<class T> static void dspBulk(T& filter, dsprun& data, const char* ident, bool speed) {
    filter.calculate(data.gain, data.input, data.output, data.size);
    filter.clear();
    cugl::Timestamp start;
    start.mark();
    for(int ii = 0; ii < data.count; ii++) {
        filter.calculate(data.gain, data.input, data.output, data.size);
    }
    cugl::Timestamp end;
    
    filter.clear();
    if (data.compare != nullptr) {
        int same = -1;
        for(int ii = 0; same == -1 && ii < data.size*data.stride; ii++) {
            if (fabsf(data.output[ii] - data.compare[ii]) >= CU_MATH_EPSILON) {
                same = ii;
            }
        }
        CUAssertAlwaysLog(same == -1, "%s failed at position %d",ident,same);
    }
    
    if (speed) {
        CULog("%s time: %llu micros",ident,cugl::Timestamp::ellapsedMicros(start,end));
    }
}

template<class E,class F,class G>
static void dspCompose(E& filter1, F& filter2, G& filter3, dsprun& data, const char* msg) {
    filter1.setChannels((unsigned)data.stride);
    filter2.setChannels((unsigned)data.stride);
    filter3.setChannels((unsigned)data.stride);
    filter1.clear();
    filter1.calculate(data.gain, data.input, data.output, data.size);
    filter2.clear();
    filter2.calculate(1.0f, data.output, data.compare, data.size);
    filter3.clear();
    filter3.calculate(data.gain, data.input, data.output, data.size);

    int same = -1;
    for(int ii = 0; same == -1 && ii < data.size*data.stride; ii++) {
        if (fabsf(data.output[ii] - data.compare[ii]) >= CU_MATH_EPSILON) {
            same = ii;
        }
    }
    CUAssertAlwaysLog(same == -1, "%s composition failed at position %d [%f vs %f]",msg,same,data.output[same],data.compare[same]);
}

    
template<class T> static void dspRegression(IIRFilter& base, T& targ, dsprun& data, const char* ident, bool speed) {
    char buff[100];

    size_t size = data.size;
    float* output1 = data.output;
    float* output2 = data.compare;
    
    data.stride  = 1;
    data.output  = output2;
    data.compare = nullptr;

    base.setChannels(1);
    snprintf(buff, sizeof(buff), "IIR %s channel (1)", ident);
    dspBulk<IIRFilter>(base,data,buff,speed);
    
    targ.setChannels(1);
    data.output  = output1;
    data.compare = output2;
    
    snprintf(buff, sizeof(buff), "%s step base (1)", ident);
    dspStep<T>(targ,data,buff,speed);

    snprintf(buff, sizeof(buff), "%s channel (1)", ident);
    dspBulk<T>(targ,data,buff,speed);

    data.stride  = 2;
    data.size    = size/2;
    base.setChannels((unsigned)data.stride);
    targ.setChannels((unsigned)data.stride);
    
    data.output  = output2;
    data.compare = nullptr;

    snprintf(buff, sizeof(buff), "IIR %s channel (2)", ident);
    dspBulk<IIRFilter>(base,data,buff,speed);
    
    data.output  = output1;
    data.compare = output2;

    snprintf(buff, sizeof(buff), "%s step base (2)", ident);
    dspStep<T>(targ,data,buff,speed);
    
    snprintf(buff, sizeof(buff), "%s channel (2)", ident);
    dspBulk<T>(targ,data,buff,speed);
    
    data.stride  = 3;
    data.size    = size/3-((size/3) % 4);
    base.setChannels((unsigned)data.stride);
    targ.setChannels((unsigned)data.stride);
    
    data.output  = output2;
    data.compare = nullptr;
    snprintf(buff, sizeof(buff), "IIR %s channel (3)", ident);
    dspBulk<IIRFilter>(base,data,buff,speed);
    
    data.output  = output1;
    data.compare = output2;
    
    snprintf(buff, sizeof(buff), "%s step base (3)", ident);
    dspStep<T>(targ,data,buff,speed);
    
    snprintf(buff, sizeof(buff), "%s channel (3)", ident);
    dspBulk<T>(targ,data,buff,speed);
    
    data.stride  = 4;
    data.size    = size/4;
    base.setChannels((unsigned)data.stride);
    targ.setChannels((unsigned)data.stride);
    
    data.output  = output2;
    data.compare = nullptr;
    snprintf(buff, sizeof(buff), "IIR %s channel (4)", ident);
    dspBulk<IIRFilter>(base,data,buff,speed);
    
    data.output  = output1;
    data.compare = output2;
    
    snprintf(buff, sizeof(buff), "%s step base (4)", ident);
    dspStep<T>(targ,data,buff,speed);
    
    snprintf(buff, sizeof(buff), "%s channel (4)", ident);
    dspBulk<T>(targ,data,buff,speed);
    
    data.stride  = 8;
    data.size    = size/8;
    base.setChannels((unsigned)data.stride);
    targ.setChannels((unsigned)data.stride);
    
    data.output  = output2;
    data.compare = nullptr;
    snprintf(buff, sizeof(buff), "IIR %s channel (8)", ident);
    dspBulk<IIRFilter>(base,data,buff,speed);
    
    data.output  = output1;
    data.compare = output2;
    
    snprintf(buff, sizeof(buff), "%s step base (8)", ident);
    dspStep<T>(targ,data,buff,speed);
    
    snprintf(buff, sizeof(buff), "%s channel (8)", ident);
    dspBulk<T>(targ,data,buff,speed);
    
    data.stride = 1;
    data.size = size;
    data.output  = output1;
    data.compare = output2;
}

void cugl::testFilters() {
    CULog("Running tests for DSP filters.\n");

#pragma mark Coefficient Bootstrap
    std::vector<float> bs;
    std::vector<float> as;
    std::vector<float> cs;

    // Performance cost of two chained Biquads
    bs.push_back(0.9f);
    bs.push_back(0.3f);
    bs.push_back(0.1f);
    bs.push_back(0.1f);
    bs.push_back(0.1f);
    
    as.push_back(1.0f);
    as.push_back(0.3f);
    as.push_back(0.1f);
    as.push_back(0.1f);
    as.push_back(0.2f);
    
    cs.push_back(1.0f);

    float input[ARRAY_SIZE];
    float output1[ARRAY_SIZE];
    float output2[ARRAY_SIZE];

    for(int ii = 0; ii < ARRAY_SIZE; ii++) {
        input[ii] = sinf(ii * M_PI / 10.0f);
        output1[ii] = 0;
        output2[ii] = 0;
    }
    
    dsprun data;
    data.stride = 1;
    data.gain = 0.5f;
    //data.gain = 1.0f;
    data.size = ARRAY_SIZE;
    data.count = LOOP_SIZE;
    data.input = input;
    data.output  = output1;
    data.compare = output2;
    
    bool timer = true;

#pragma mark IIR Test
    IIRFilter filter1(1);
    filter1.setCoeff(bs,as);
    
    dspRegression<IIRFilter>(filter1,filter1,data,"a-IIR",timer);

#pragma mark FIR Test
    FIRFilter filter2(1);
    
    filter1.setChannels(1);
    filter1.setCoeff(bs,cs);
    filter2.setCoeff(bs,cs);

    dspRegression<FIRFilter>(filter1,filter2,data,"b-FIR",timer);

#pragma mark Two Pole Test
    TwoPoleIIR filter3(1);

    as.clear();
    as.push_back(1.0f);
    as.push_back(0.3f);
    as.push_back(0.1f);
    
    filter1.setChannels(1);
    filter1.setCoeff(cs,as);
    filter3.setCoeff(cs,as);

    dspRegression<TwoPoleIIR>(filter1,filter3,data,"2 pole",timer);
    
#pragma mark Two Zero Test
    TwoZeroFIR filter4(1);

    bs.clear();
    bs.push_back(0.9f);
    bs.push_back(0.3f);
    bs.push_back(0.1f);
    
    filter1.setChannels(1);
    filter1.setCoeff(bs,cs);
    filter4.setCoeff(bs,cs);

    dspRegression<TwoZeroFIR>(filter1,filter4,data,"2 zero",timer);
    
#pragma mark Biquad Test
    BiquadIIR filter5(1);

    filter1.setChannels(1);
    filter1.setCoeff(bs,as);
    filter5.setCoeff(bs,as);
    
    dspRegression<BiquadIIR>(filter1,filter5,data,"biquad",timer);

#pragma mark One Pole Test
    OnePoleIIR filter6(1);
    
    as.clear();
    as.push_back(1.0f);
    as.push_back(0.3f);
    
    filter6.setChannels(1);
    filter1.setCoeff(cs,as);
    filter6.setCoeff(cs,as);
    
    dspRegression<OnePoleIIR>(filter1,filter6,data,"1 pole",timer);

#pragma mark One Zero Test
    OneZeroFIR filter7(1);
    
    bs.clear();
    bs.push_back(0.9f);
    bs.push_back(0.3f);

    filter1.setChannels(1);
    filter1.setCoeff(bs,cs);
    filter7.setCoeff(bs,cs);
    
    dspRegression<OneZeroFIR>(filter1,filter7,data,"1 zero",timer);

#pragma mark Pole Zero Test
    PoleZeroFIR filter8(1);
    
    filter1.setChannels(1);
    filter1.setCoeff(bs,as);
    filter8.setCoeff(bs,as);
    
    dspRegression<PoleZeroFIR>(filter1,filter8,data,"pole 0",timer);

#pragma mark Polynomial Test
    Polynomial p;
    Polynomial q(1);
    p[0] = 1.0f;
    q[0] = 0.2;
    q[1] = 1.0;
    
    filter6.setTransfer(p, q);
    filter3.setTransfer(p, q*q);
    CULog("Poly is %s",(q*q).toString().c_str());
    dspCompose<OnePoleIIR,OnePoleIIR,TwoPoleIIR>(filter6,filter6,filter3,data,"1 pole");


#pragma mark Complete
    CULog("Filter tests complete.\n");
}

#pragma mark -
#pragma mark Main

/**
 * Master unit test that invokes all others in this module.
 */
void cugl::mathUnitTest() {
    testVec2();
    testVec3();
    testVec4();
    testColor4f();
	testColor4();
    testSize();
    testRect();
    testQuaternion();
    testMat4();
    testAffine2();
    testPolynomial();
    testPoly2();
    testRay();
    testPlane();
    testFrustum();
    testDSP();
    testFilters();

    int i, count = SDL_GetNumAudioDevices(0);
    for (i = 0; i < count; ++i) {
        printf("Audio device %d: %s\n", i, SDL_GetAudioDeviceName(i, 0));
    }
    for (i = 0; i < SDL_GetNumAudioDrivers(); ++i) {
        printf("Audio driver %d: %s\n", i, SDL_GetAudioDriver(i));
    }
}
