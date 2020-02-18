//
//  CUVertex2.h
//  Cornell University Game Library (CUGL)
//
//  This module provides the basic structs for the default rendering pipeline.
//  These structs are meant to be passed by value, so we have no methods for
//  shared pointers.
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
//  Version: 6/23/16

#ifndef __CU_VERTEX2_H__
#define __CU_VERTEX2_H__

#include <cugl/math/CUVec2.h>
#include <cugl/math/CUVec3.h>
#include <cugl/math/CUColor4.h>

namespace cugl {

/**
 * This class/struct represents the rendering information for a 2d vertex.
 *
 * The class is intended to be used as a struct.  The static methods are to
 * compute the offset for VBO access.
 */
class Vertex2 {
public:
    /** The vertex position */
    cugl::Vec2    position;
    /** The vertex color */
    cugl::Color4  color;
    /** The vertex texture coordinate */
    cugl::Vec2    texcoord;
    
    /** The memory offset of the vertex position */
    static const GLvoid* positionOffset()   { return (GLvoid*)offsetof(Vertex2, position);  }
    /** The memory offset of the vertex color */
    static const GLvoid* colorOffset()      { return (GLvoid*)offsetof(Vertex2, color);     }
    /** The memory offset of the vertex texture coordinate */
    static const GLvoid* texcoordOffset()   { return (GLvoid*)offsetof(Vertex2, texcoord);  }
};

/**
 * This class/struct represents the rendering information for a 2d vertex.
 *
 * The class is intended to be used as a struct.  The static methods are to
 * compute the offset for VBO access.
 */
class Vertex3 {
public:
    /** The vertex position */
    cugl::Vec3    position;
    /** The vertex color */
    cugl::Color4  color;
    /** The vertex texture coordinate */
    cugl::Vec2    texcoord;
    
    /** The memory offset of the vertex position */
    static const GLvoid* positionOffset()   { return (GLvoid*)offsetof(Vertex3, position);  }
    /** The memory offset of the vertex color */
    static const GLvoid* colorOffset()      { return (GLvoid*)offsetof(Vertex3, color);     }
    /** The memory offset of the vertex texture coordinate */
    static const GLvoid* texcoordOffset()   { return (GLvoid*)offsetof(Vertex3, texcoord);  }
};

}

#endif /* __CU_VERTEX2_H__ */
