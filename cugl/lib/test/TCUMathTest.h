//
//  TCUMathTest.h
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

#ifndef __T_CU_MATH_TEST_H__
#define __T_CU_MATH_TEST_H__

namespace cugl {

/**
 * Unit test for a 2-dimension vector
 */
void testVec2();

/**
 * Unit test for a 3-dimension vector
 */
void testVec3();

/**
 * Unit test for a 4-dimension vector
 *
 * Thic class uses vector acceleration on select platforms.
 */
void testVec4();

/**
 * Unit test for a 4-float color
 *
 * This is the prefered color for math operations.
 */
void testColor4f();

/**
 * Unit test for a 4-byte color
 *
 * This is the preferred color for storage and shaders.
 */
void testColor4();

/**
 * Unit test for a 2-dimensional size.
 */
void testSize();

/**
 * Unit test for a 2-dimensional bounding box.
 */
void testRect();

/**
 * Unit test for a quaternion
 */
void testQuaternion();

/**
 * Unit test for a 4x4 matrix (with homoengenous coordinate support)
 *
 * Thic class uses vector acceleration on select platforms.
 */
void testMat4();

/**
 * Unit test for a 2-dimensional affine transform.
 */
void testAffine2();

/**
 * Unit test for  2-dimensional polygon
 */
void testPoly2();

/**
 * Unit test for a polynomial equation with root solver
 */
void testPolynomial();

/**
 * Unit test for a 3-dimensional ray
 */
void testRay();

/**
 * Unit test for a plane in 3d space
 */
void testPlane();

/**
 * Unit test for a viewing frustrum
 */
void testFrustum();

void testDSP();

void testFilters();

/**
 * Master unit test that invokes all others in this module.
 */
void mathUnitTest();

}


#endif /* __T_CU_MATH_TEST_H__ */
