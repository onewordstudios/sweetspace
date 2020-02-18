//
//  CUTexturedNode.cpp
//  Cornell University Game Library (CUGL)
//
//  This module provides an abstract class for textured scene graph nodes. It
//  is the core scene graph node in CUGL.
//
//  You should never instantiate an object of this class.  Instead, you should
//  use one of the concrete subclasses: WireNode, PolygonNode, or PathNode.
//  Because it is abstract, it has no allocators.  It only has initializers.
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
//  Version: 6/27/16

#include <algorithm>
#include <cugl/2d/CUTexturedNode.h>
#include <cugl/util/CUStrings.h>
#include <cugl/assets/CUSceneLoader.h>
#include <cugl/assets/CUAssetManager.h>
#include <sstream>

using namespace cugl;

/** For handling JSON issues */
#define UNKNOWN_STR "<unknown>"

#pragma mark Constructors

/**
 * Creates an empty polygon with the degenerate texture.
 *
 * You do not need to set the texture. The polygon, however, will also be
 * empty, and must be set via setPolygon.
 *
 * This constructor should never be called directly, as this is an
 * abstract class.
 */
TexturedNode::TexturedNode() :
Node(),
_classname("TexturedNode"),
_texture(nullptr),
_blendEquation(GL_FUNC_ADD),
_srcFactor(GL_SRC_ALPHA),
_dstFactor(GL_ONE_MINUS_SRC_ALPHA),
_flipHorizontal(false),
_flipVertical(false),
_absolute(false) {
    _name = "TexturedNode";
}

/**
 * Disposes all of the resources used by this node.
 *
 * A disposed Node can be safely reinitialized. Any children owned by this
 * node will be released.  They will be deleted if no other object owns them.
 *
 * It is unsafe to call this on a Node that is still currently inside of
 * a scene graph.
 */
void TexturedNode::dispose() {
    _classname = "TexturedNode";
    _texture = nullptr;
    _blendEquation = GL_FUNC_ADD;
    _srcFactor = GL_SRC_ALPHA;
    _dstFactor = GL_ONE_MINUS_SRC_ALPHA;
    _flipHorizontal = false;
    _flipVertical = false;
    _vertices.clear();
    _polygon.clear();
    Node::dispose();
}

/**
 * Intializes a textured polygon from the image filename.
 *
 * After creation, the polygon will be a rectangle.  The vertices of this
 * polygon will be the corners of the image.
 *
 * @param filename   A path to image file, e.g., "scene1/earthtile.png"
 *
 * @retain  a reference to the newly loaded texture
 * @return  true if the sprite is initialized properly, false otherwise.
 */
bool TexturedNode::initWithFile(const std::string& filename) {
    CUAssertLog(filename.size() > 0, "Invalid filename for sprite");
    
    std::shared_ptr<Texture> texture =  Texture::allocWithFile(filename);
    if (texture != nullptr) {
        Rect bounds = Rect::ZERO;
        bounds.size = texture->getSize();
        return initWithTexture(texture, bounds);
    }
    
    return false;
}

/**
 * Initializes a textured polygon from the image filename and the given polygon.
 *
 * @param filename  A path to image file, e.g., "scene1/earthtile.png"
 * @param poly      The polygon to texture
 *
 * @retain  a reference to the newly loaded texture
 * @return  true if the sprite is initialized properly, false otherwise.
 */
bool TexturedNode::initWithFile(const std::string &filename, const Poly2& poly) {
    CUAssertLog(filename.size() > 0, "Invalid filename for sprite");
    
    std::shared_ptr<Texture> texture =  Texture::allocWithFile(filename);
    if (texture != nullptr) {
        return initWithTexture(texture, poly);
    }
    
    return false;
}

/**
 * Initializes a textured polygon from the image filename and the given rect.
 *
 * The rectangle will be converted into a Poly2.  There is little benefit to
 * using a TexturedNode in this way over a normal Sprite. The option is here
 * only for completion.
 *
 * @param filename  A path to image file, e.g., "scene1/earthtile.png"
 * @param rect      The rectangle to texture
 *
 * @retain  a reference to the newly loaded texture
 * @return  true if the sprite is initialized properly, false otherwise.
 */
bool TexturedNode::initWithFile(const std::string &filename, const Rect& rect) {
    CUAssertLog(filename.size() > 0, "Invalid filename for sprite");
    
    std::shared_ptr<Texture> texture =  Texture::allocWithFile(filename);
    if (texture != nullptr) {
        return initWithTexture(texture, rect);
    }
    
    return false;
}

/**
 * Initializes a textured polygon from a Texture2D object.
 *
 * After creation, the polygon will be a rectangle. The vertices of this
 * polygon will be the corners of the texture.
 *
 * @param texture   A pointer to a Texture2D object.
 *
 * @retain  a reference to this texture
 * @return  true if the sprite is initialized properly, false otherwise.
 */
bool TexturedNode::initWithTexture(const std::shared_ptr<Texture>& texture) {
    CUAssertLog(texture != nullptr, "Invalid texture for sprite");
    
    Rect bounds = Rect::ZERO;
    bounds.size = texture->getSize();
    return initWithTexture(texture, bounds);
}

/**
 * Initializes a textured polygon from a Texture2D object and the given polygon.
 *
 * @param   texture  A pointer to an existing Texture2D object.
 *                   You can use a Texture2D object for many sprites.
 * @param   poly     The polygon to texture
 *
 * @retain  a reference to this texture
 * @return  true if the sprite is initialized properly, false otherwise.
 */
bool TexturedNode::initWithTexture(const std::shared_ptr<Texture>& texture, const Poly2& poly) {
    if (_texture != nullptr) {
        CUAssertLog(false, "%s is already initialized",_classname.c_str());
        return false;
    }
    
    if (Node::init()) {
        // default transform anchor: center
        setAnchor(Vec2(0.5f, 0.5f));
        
        // Update texture (Sets texture coordinates)
        setTexture(texture);
        setPolygon(poly);
        return true;
    }
    
    return false;
}

/**
 * Initializes a textured polygon from a Texture2D object and the given rect.
 *
 * The rectangle will be converted into a Poly2.  There is little benefit to
 * using a TexturedNode in this way over a normal Sprite. The option is here
 * only for completion.
 *
 * @param   texture  A pointer to an existing Texture2D object.
 *                   You can use a Texture2D object for many sprites.
 * @param   rect     The rectangle to texture
 *
 * @retain  a reference to this texture
 * @return  true if the sprite is initialized properly, false otherwise.
 */
bool TexturedNode::initWithTexture(const std::shared_ptr<Texture>& texture, const Rect& rect) {
    if (_texture != nullptr) {
        CUAssertLog(false, "%s is already initialized",_classname.c_str());
        return false;
    }

    if (Node::init()) {
        // default transform anchor: center
        setAnchor(Vec2(0.5f, 0.5f));
        
        // Update texture (Sets texture coordinates)
        setTexture(texture);
        setPolygon(rect);
        return true;
    }
    
    return false;
}

/**
 * Initializes a node with the given JSON specificaton.
 *
 * This initializer is designed to receive the "data" object from the
 * JSON passed to {@link SceneLoader}.  This JSON format supports all
 * of the attribute values of its parent class.  In addition, it supports
 * the following additional attributes:
 *
 *      "texture":  The name of a previously loaded texture asset
 *      "polygon":  An even array of polygon vertices (numbers)
 *      "indices":  An array of unsigned ints defining triangles from the
 *                  the vertices. The array size should be a multiple of 3.
 *
 * All attributes are optional.  However, it is generally a good idea to
 * specify EITHER the texture or the polygon
 *
 * @param loader    The scene loader passing this JSON file
 * @param data      The JSON object specifying the node
 *
 * @return true if initialization was successful.
 */
bool TexturedNode::initWithData(const SceneLoader* loader, const std::shared_ptr<JsonValue>& data) {
    if (_texture != nullptr) {
        CUAssertLog(false, "%s is already initialized",_classname.c_str());
        return false;
    } else if (!data) {
        return init();
    } else if (!Node::initWithData(loader, data)) {
        return false;
    }
    
    // All of the code that follows can corrupt the position.
    Vec2 coord = getPosition();
    
    // Set the texture (it might be null)
    const AssetManager* assets = loader->getManager();
    setTexture(assets->get<Texture>(data->getString("texture",UNKNOWN_STR)));

    // Get the geometry
    std::vector<Vec2> vertices;
    std::vector<unsigned short> indices;

    if (data->has("polygon")) {
        JsonValue* poly = data->get("polygon").get();
        CUAssertLog(poly->size() % 2 == 0, "'polygon' should be an even list of numbers");
        for(int ii = 0; ii < poly->size(); ii += 2) {
            Vec2 vert;
            vert.x = poly->get(ii  )->asFloat(0.0f);
            vert.y = poly->get(ii+1)->asFloat(0.0f);
            vertices.push_back(vert);
        }
    }

    if (data->has("indices")) {
        JsonValue* index = data->get("indices").get();
        CUAssertLog(index->size() % 3 == 0, "'indices' should be an list of numbers in multiples of 3");
        for(int ii = 0; ii < index->size(); ii += 3) {
            indices.push_back(index->get(ii  )->asInt(0));
            indices.push_back(index->get(ii+1)->asInt(0));
            indices.push_back(index->get(ii+2)->asInt(0));
        }
    }
    
    // If size was set explicitly, we will need to restore it after polygon
    bool sizefit = data->has("size");
    Size size = getSize();
    
    if (vertices.empty() && indices.empty()) {
        Rect bounds = Rect::ZERO;
        bounds.size = _texture->getSize();
        setPolygon(bounds);
    } else if (indices.empty()) {
        setPolygon(vertices);
    } else {
        setPolygon(Poly2(vertices,indices));
    }

    // Redo the size if necessary
    if (sizefit) {
        setContentSize(size);
    }

    // Now redo the position
    setPosition(coord);
    return true;
}


#pragma mark -
#pragma mark Attributes

/**
 * Sets the node texture to the one specified.
 *
 * This method will have no effect on the polygon vertices.  Unlike Sprite,
 * TexturedNode decouples the geometry from the texture.  That is because
 * we expect the vertices to not match the texture perfectly.
 *
 * @param   texture  A pointer to an existing Texture2D object.
 *                   You can use a Texture2D object for many sprites.
 *
 * @retain  a reference to this texture
 * @release the previous scene texture used by this object
 */
void TexturedNode::setTexture(const std::shared_ptr<Texture>& texture) {
    std::shared_ptr<Texture> temp = (texture == nullptr ? SpriteBatch::getBlankTexture() : texture);
    if (_texture != temp) {
        _texture = temp;
        updateTextureCoords();
    }
}

/**
 * Sets the texture polygon to the given one in image space.
 *
 * @param poly  The polygon to texture
 */
void TexturedNode::setPolygon(const Poly2& poly) {
    if (&_polygon != &poly) {
        _polygon.set(poly);
    }
    
    clearRenderData();
    setContentSize(_polygon.getBounds().size);
}

/**
 * Translates the polygon by the given amount.
 *
 * Remember that translating the polygon has no effect on the shape or position. 
 * Because the polygon is expressed in texture coordinates, all it does is shift 
 * the texture coords of the polygon.  Hence this method can be used for animation
 * and filmstrips.
 *
 * Calling this method is faster than changing the polygon and resetting it.
 *
 * @param   dx  The amount to shift horizontally.
 * @param   dx  The amount to shift horizontally.
 */
void TexturedNode::shiftPolygon(float dx, float dy) {
    _polygon += Vec2(dx,dy);
    float w = (float)_texture->getWidth();
    float h = (float)_texture->getHeight();
    for(auto it = _vertices.begin(); it != _vertices.end(); ++it) {
        it->texcoord.x += dx/w;
        it->texcoord.y -= dy/h;
    }
}

/**
 * Returns a string representation of this node for debugging purposes.
 *
 * If verbose is true, the string will include class information.  This
 * allows us to unambiguously identify the class.
 *
 * @param verbose Whether to include class information
 *
 * @return a string representation of this node for debuggging purposes.
 */
std::string TexturedNode::toString(bool verbose) const {
    std::stringstream ss;
    if (verbose) {
        ss << "cugl::";
        ss << _classname;
    }
    int texid = (_texture == nullptr ? -1 : _texture->getBuffer());
    ss << "(tag:";
    ss <<  cugl::to_string(_tag);
    ss << ", name:" << _name;
    ss << ", texture:";
    ss <<  cugl::to_string(texid);
    ss << ")";
    return ss.str();
}

/**
 * Sets the untransformed size of the node.
 *
 * The content size remains the same no matter how the node is scaled or
 * rotated. All nodes must have a size, though it may be degenerate (0,0).
 *
 * By default, the content size of a TexturedNode is the size of the
 * bounding box of the defining polygon.  Resizing a texture node will
 * stretch the image to fill in the new size.
 *
 * @param size  The untransformed size of the node.
 */
void TexturedNode::setContentSize(const Size& size) {
    Node::setContentSize(size);
    clearRenderData();
}


#pragma mark -
#pragma mark Internal Helpers

/**
 * Allocate the render data necessary to render this node.
 */
void TexturedNode::generateRenderData() {
    CUAssertLog(!_rendered, "Render data is already present");
    if (_texture == nullptr) {
        return;
    }
    
    Vertex2 temp;
    temp.color = Color4::WHITE;

    Size nsize = getContentSize();
    Size bsize = _polygon.getBounds().size;
    Size tsize = _texture->getSize();
    Vec2 scale = Vec2::ONE;
    if (nsize != bsize) {
        scale.x = (bsize.width > 0 ? nsize.width/bsize.width : 0);
        scale.y = (bsize.height > 0 ? nsize.height/bsize.height : 0);
    }

    Vec2 offset = _polygon.getBounds().origin;
    for(auto it = _polygon.getVertices().begin(); it != _polygon.getVertices().end(); ++it) {
        temp.position = *it*scale;
        if (!_absolute) {
            temp.position -= offset*scale;
        }
        
        float s = (it->x)/tsize.width;
        float t = (it->y)/tsize.height;
        if (_flipHorizontal) { s = 1-s; }
        if (!_flipVertical)  { t = 1-t; }
        
        temp.texcoord.x = s*_texture->getMaxS()+(1-s)*_texture->getMinS();
        temp.texcoord.y = t*_texture->getMaxT()+(1-t)*_texture->getMinT();
        _vertices.push_back(temp);
    }
    
    _rendered = true;
}

/**
 * Clears the render data, releasing all vertices and indices.
 */
void TexturedNode::clearRenderData() {
    _vertices.clear();
    _rendered = false;
    
}

/**
 * Updates the texture coordinates for this polygon
 *
 * The texture coordinates are computed assuming that the polygon is
 * defined in image space, with the origin in the bottom left corner
 * of the texture.
 */
void TexturedNode::updateTextureCoords() {
    if (!_rendered) {
        return;
    }
    
    float w = (float)_texture->getWidth();
    float h = (float)_texture->getHeight();
    Vec2 offset = _polygon.getBounds().origin;
    for(auto it = _vertices.begin(); it != _vertices.end(); ++it) {
        it->texcoord.x = (it->position.x+offset.x)/w;
        it->texcoord.y = (it->position.y+offset.y)/h;
        if (_flipHorizontal) {
            it->texcoord.x = 1-it->texcoord.x;
        }
        if (!_flipVertical) {
            it->texcoord.y = 1-it->texcoord.y;
        }
    }
}

