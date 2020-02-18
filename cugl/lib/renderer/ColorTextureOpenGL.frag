//
//  CUColorTextureOpenGL.h
//  Cornell University Game Library (CUGL)
//
//  This module provides a basic SpriteBatch vertex shader in both OpenGL and
//  OpenGL ES. There are slight differences between the two shaders.  Notice
//  the use of #if to separate them.
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

#include <cugl/renderer/CUShader.h>

#if CU_GL_PLATFORM == CU_GL_OPENGL

/**
 * The fragment shader for OpenGL
 */
const char* oglColorTextureFrag = SHADER(
////////// SHADER BEGIN /////////

// The output color
out vec4 frag_color;

// Color result from vertex shader
in vec4 outColor;

// Texture result from vertex shader
in vec2 outTexCoord;

// Texture map
uniform sampler2D uTexture;

void main(void) {
    frag_color = texture(uTexture, outTexCoord)*outColor;
}

/////////// SHADER END //////////
);

#else 

/**
 * The fragment shader for OpenGL ES
 */
const char* oglColorTextureFrag = SHADER(
////////// SHADER BEGIN /////////

// This one line is all the difference
precision mediump float;

// The output color
out vec4 frag_color;
                                         
// Color result from vertex shader
in vec4 outColor;
                                         
// Texture result from vertex shader
in vec2 outTexCoord;
                                         
// Texture map
uniform sampler2D uTexture;

void main(void) {
    frag_color = texture(uTexture, outTexCoord)*outColor;
}

/////////// SHADER END //////////
);

#endif
