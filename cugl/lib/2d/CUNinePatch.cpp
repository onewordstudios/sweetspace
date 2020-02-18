//
//  CUNinePatch.cpp
//  Cornell University Game Library (CUGL)
//
//  This module implements a ninepatch for expandable UI elements.  A ninepatch
//  breaks up an image into nine parts.  It expands the middle elements while
//  preserving the corners.  This allows us to arbitrarily stretch an image
//  like a beveled button without distorting it. Ninepatches are used heavily
//  in mobile UI development.
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
//  Version: 11/8/17
//
#include <cugl/2d/CUNinePatch.h>
#include <cugl/util/CUStrings.h>
#include <cugl/assets/CUSceneLoader.h>
#include <cugl/assets/CUAssetManager.h>
#include <algorithm>
#include <sstream>

using namespace cugl;

#define UNKNOWN_TEXTURE "<unknown>"

#pragma mark Constructors
/**
 * Creates a NinePatch with the degenerate texture.
 *
 * You must initialize this NinePatch before use.
 *
 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
 * the heap, use one of the static constructors instead.
 */
NinePatch::NinePatch() : Node(),
_texture(nullptr),
_rendered(false),
_blendEquation(GL_FUNC_ADD),
_srcFactor(GL_SRC_ALPHA),
_dstFactor(GL_ONE_MINUS_SRC_ALPHA) {
    _name = "NinePatch";
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
void NinePatch::dispose() {
    _texture = nullptr;
    _interior = Rect::ZERO;
    _blendEquation = GL_FUNC_ADD;
    _srcFactor = GL_SRC_ALPHA;
    _dstFactor = GL_ONE_MINUS_SRC_ALPHA;
    _vertices.clear();
    Node::dispose();
}

/**
 * Intializes a degenerate NinePatch from the image filename.
 *
 * After creation, this will be a degenerate NinePatch.  It will identify
 * the center pixel of the image as the interior.  All other pixels will
 * either be a corner or side.
 *
 * @param filename  A path to image file, e.g., "scene1/earthtile.png"
 *
 * @return  true if the node is initialized properly, false otherwise.
 */
bool NinePatch::initWithFile(const std::string& filename) {
    CUAssertLog(filename.size() > 0, "Invalid filename for sprite");
    
    std::shared_ptr<Texture> texture =  Texture::allocWithFile(filename);
    if (texture != nullptr) {
        Size size = texture->getSize();
        Rect bounds((float)(((int)size.width)/2),(float)(((int)size.height)/2),1,1);
        return initWithTexture(texture, bounds);
    }
    
    return false;
}

/**
 * Initializes a NinePatch with the given interior from the image filename.
 *
 * The interior rectangle is specified in pixel coordinates. As with
 * {@link PolygonNode}, we assume that the pixel origin is the bottom-left
 * corner of the image.
 *
 * The interior rectangle fully defines the NinePatch. For example, suppose
 * the rectangle has origin (2,3) and size (4,2).  Then (1,1) is in the
 * bottom left corner, while (3,1) is in the bottom middle, and so on.
 *
 * @param filename  A path to image file, e.g., "scene1/earthtile.png"
 * @param interior  The rectangle (in pixel coordinates) defining the interior.
 *
 * @return  true if the node is initialized properly, false otherwise.
 */
bool NinePatch::initWithFile(const std::string &filename, const Rect& interior) {
    CUAssertLog(filename.size() > 0, "Invalid filename for sprite");
    
    std::shared_ptr<Texture> texture =  Texture::allocWithFile(filename);
    if (texture != nullptr) {
        return initWithTexture(texture, interior);
    }
    
    return false;
}

/**
 * Initializes a degenerate NinePatch from a Texture object.
 *
 * After creation, this will be a degenerate NinePatch.  It will identify
 * the center pixel of the image as the interior.  All other pixels will
 * either be a corner or side.
 *     *
 * @param texture   A shared pointer to a Texture object.
 *
 * @return  true if the node is initialized properly, false otherwise.
 */
bool NinePatch::initWithTexture(const std::shared_ptr<Texture>& texture) {
    CUAssertLog(texture != nullptr, "Invalid texture for sprite");
    Size size = texture->getSize();
    Rect bounds((float)(((int)size.width)/2),(float)(((int)size.height)/2),1,1);
    return initWithTexture(texture, bounds);
}

/**
 * Initializes a NinePatch with the given interior from a Texture object.
 *
 * The interior rectangle is specified in pixel coordinates. As with
 * {@link PolygonNode}, we assume that the pixel origin is the bottom-left
 * corner of the image.
 *
 * The interior rectangle fully defines the NinePatch. For example, suppose
 * the rectangle has origin (2,3) and size (4,2).  Then (1,1) is in the
 * bottom left corner, while (3,1) is in the bottom middle, and so on.
 *
 * @param texture    A shared pointer to a Texture object.
 * @param vertices   The vertices to texture (expressed in image space)
 *
 * @return  true if the node is initialized properly, false otherwise.
 */
bool NinePatch::initWithTexture(const std::shared_ptr<Texture>& texture, const Rect& interior) {
    if (_texture != nullptr) {
        CUAssertLog(false, "NinePatch is already initialized");
        return false;
    }
    
    if (Node::init()) {
        // default transform anchor: center
        setAnchor(Vec2::ANCHOR_CENTER);
        
        // Update texture (Sets texture coordinates)
        setTexture(texture);
        setInterior(interior);
        setContentSize(texture->getSize());
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
 *      "interior": An four-element array of numbers (x,y,width,height)
 *
 * All attributes are optional.  However, it is generally a good idea to
 * specify both to take full advantage of NinePatch features.
 *
 * @param loader    The scene loader passing this JSON file
 * @param data      The JSON object specifying the node
 *
 * @return true if initialization was successful.
 */
bool NinePatch::initWithData(const SceneLoader* loader, const std::shared_ptr<JsonValue>& data) {
    if (_texture != nullptr) {
        CUAssertLog(false, "NinePatch is already initialized");
        return false;
    } else if (!data) {
        return init();
    } else if (!Node::initWithData(loader, data)) {
        return false;
    }
    
    // Set the texture
    const AssetManager* assets = loader->getManager();
    std::shared_ptr<Loader<Texture>> load = assets->access<Texture>();
    std::string key = data->getString("texture",UNKNOWN_TEXTURE);
    setTexture(assets->get<Texture>(key));
    
    Rect interior;
    if (data->has("interior")) {
        JsonValue* rect = data->get("interior").get();
        CUAssertLog(rect->size() == 4, "'interior' must be a 4-element list of numbers");
        interior.origin.x = rect->get(0)->asFloat(1.0f);
        interior.origin.y = rect->get(1)->asFloat(1.0f);
        interior.size.width  = rect->get(2)->asFloat(1.0f);
        interior.size.height = rect->get(3)->asFloat(1.0f);
    } else {
        Size size = _texture->getSize();
        interior.origin.x = (float)(((int)size.width)/2);
        interior.origin.y = (float)(((int)size.height)/2);
        interior.size.width  = 1.0f;
        interior.size.height = 1.0f;
    }
    setInterior(interior);
    
    if (!data->has("size")) {
        setContentSize(_texture->getSize());
    }
    
    return true;
}


#pragma mark -
#pragma mark Attributes
/**
 * Sets the untransformed size of the node.
 *
 * The content size remains the same no matter how the node is scaled or
 * rotated. All nodes must have a size, though it may be degenerate (0,0).
 *
 * Changing the size of a rectangle will not change the position of the
 * node.  However, if the anchor is not the bottom-left corner, it will
 * change the origin.  The Node will grow out from an anchor on an edge,
 * and equidistant from an anchor in the center.
 *
 * @param size  The untransformed size of the node.
 */
void NinePatch::setContentSize(const Size& size) {
    Size actual = size;
    actual.width  = std::max(size.width, _texture->getWidth()-_interior.size.width);
    actual.height = std::max(size.height,_texture->getHeight()-_interior.size.height);
    Node::setContentSize(actual);
    clearRenderData();
}

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
void NinePatch::setTexture(const std::shared_ptr<Texture>& texture) {
    std::shared_ptr<Texture> temp = (texture == nullptr ? SpriteBatch::getBlankTexture() : texture);
    if (_texture != temp) {
        _texture = temp;
        clearRenderData();
    }
}

/**
 * Sets interior rectangle defining the NinePatch.
 *
 * The interior rectangle is specified in pixel coordinates. As with
 * {@link PolygonNode}, we assume that the pixel origin is the bottom-left
 * corner of the image.
 *
 * The interior rectangle fully defines the NinePatch. For example, suppose
 * the rectangle has origin (2,3) and size (4,2).  Then (1,1) is in the
 * bottom left corner, while (3,1) is in the bottom middle, and so on.
 *
 * @param intertior The NinePatch interior
 */
void NinePatch::setInterior(const Rect& interior) {
    _interior = interior;
    clearRenderData();
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
std::string NinePatch::toString(bool verbose) const {
    std::stringstream ss;
    if (verbose) {
        ss << "cugl::NinePatch";
    }
    int texid = (_texture == nullptr ? -1 : _texture->getBuffer());
    ss << "(tag:";
    ss <<  cugl::to_string(_tag);
    ss << ", name:" << _name;
    ss << ", texture:";
    ss <<  cugl::to_string(texid);
    ss << ")";
    if (verbose) {
        ss << "\n";
        for(auto it = _children.begin(); it != _children.end(); ++it) {
            ss << "  " << (*it)->toString(verbose);
        }
    }
    
    return ss.str();
}

#pragma mark -
#pragma mark Internal Helpers
/**
 * Clears the render data, releasing all vertices and indices.
 */
void NinePatch::clearRenderData() {
    _vertices.clear();
    _indices.clear();
    _rendered = false;
}

/**
 * Allocate the render data necessary to render this node.
 */
void NinePatch::generateRenderData() {
    CUAssertLog(!_rendered, "Render data is already present");
    if (_texture == nullptr) {
        return;
    }
    
    Vertex2 temp;
    temp.color = Color4::WHITE;
    unsigned short index = 0;
    
    // Find the opposite corner of the NinePatch
    Vec2 corner;
    corner.x = _contentSize.width-(_texture->getWidth()-_interior.size.width-_interior.origin.x);
    corner.y = _contentSize.height-(_texture->getHeight()-_interior.size.height-_interior.origin.y);

    Rect dst, src;
    
    // Bottom left corner
    dst.origin.x = 0;
    dst.origin.y = 0;
    dst.size.width  = _interior.origin.x;
    dst.size.height = _interior.origin.y;
    src = dst;
    index = generatePatch(src, dst, index);

    // Middle left
    dst.origin.x = 0;
    dst.origin.y = _interior.origin.y;
    dst.size.width  = _interior.origin.x;
    dst.size.height = corner.y-_interior.origin.y;
    src.origin = dst.origin;
    src.size.width  = _interior.origin.x;
    src.size.height = _interior.size.height;
    index = generatePatch(src, dst, index);

    // Top left corner
    dst.origin.x = 0;
    dst.origin.y = corner.y;
    dst.size.width  = _interior.origin.x;
    dst.size.height = _contentSize.height-dst.origin.y;
    src.origin.x = 0;
    src.origin.y = _interior.origin.y + _interior.size.height;
    src.size.width  = _interior.origin.x;
    src.size.height = _texture->getHeight()-src.origin.y;
    index = generatePatch(src, dst, index);
    
    // Middle bottom
    dst.origin.x = _interior.origin.x;
    dst.origin.y = 0;
    dst.size.width  = corner.x-_interior.origin.x;
    dst.size.height = _interior.origin.y;
    src.origin = dst.origin;
    src.size.width = _interior.size.width;
    src.size.height = _interior.origin.y;
    index = generatePatch(src, dst, index);

    // Middle
    dst.origin.x = _interior.origin.x;
    dst.origin.y = _interior.origin.y;
    dst.size.width  = corner.x-_interior.origin.x;
    dst.size.height = corner.y-_interior.origin.y;
    src.origin = dst.origin;
    src.size   = _interior.size;
    index = generatePatch(src, dst, index);

    // Middle top
    dst.origin.x = _interior.origin.x;
    dst.origin.y = corner.y;
    dst.size.width  = corner.x-_interior.origin.x;
    dst.size.height = _contentSize.height-dst.origin.y;
    src.origin.x = _interior.origin.x;
    src.origin.y = _interior.origin.y + _interior.size.height;
    src.size.width  = _interior.size.width;
    src.size.height = _texture->getHeight()-src.origin.y;
    index = generatePatch(src, dst, index);

    // Bottom right corner
    dst.origin.x = corner.x;
    dst.origin.y = 0;
    dst.size.width  = _contentSize.width-dst.origin.x;
    dst.size.height = _interior.origin.y;
    src.origin.x = _interior.origin.x + _interior.size.width;
    src.origin.y = 0;
    src.size.width  = _texture->getWidth()-src.origin.x;
    src.size.height = _interior.origin.y;
    index = generatePatch(src, dst, index);

    // Middle right
    dst.origin.x = corner.x;
    dst.origin.y = _interior.origin.y;
    dst.size.width  = _contentSize.width-dst.origin.x;
    dst.size.height = corner.y-_interior.origin.y;
    src.origin.x = _interior.origin.x + _interior.size.width;
    src.origin.y = _interior.origin.y;
    src.size.width  = _texture->getWidth()-src.origin.x;
    src.size.height = _interior.size.height;
    index = generatePatch(src, dst, index);
    
    // Top right corner
    dst.origin = corner;
    dst.size   = _contentSize-dst.origin;
    src.origin = _interior.origin + _interior.size;
    src.size   = _texture->getSize()-src.origin;
    index = generatePatch(src, dst, index);

    _rendered = true;
}

/**
 * Generates the textured quad for one of the nine patches.
 *
 * This function generates a quad for the rectangle dst, using the
 * subtexture identified by src.  If dst is larger than src, the image
 * is stretched to fit.  The vertices are added to _vertices and the
 * indices are added to _indices.
 *
 * The value offset specifies the initial index to use for _indices.
 * The indices for all created vertices are start from this value.
 * The value returned is the next index available.
 *
 * @param src       The subtexture region to use
 * @param dst       The rectangle to texture and to add to _vertices
 * @param offset    The first available vertex index
 *
 * @return the next available vertex index
 */

unsigned short NinePatch::generatePatch(const Rect& src, const Rect& dst, unsigned short offset) {
    Size tsize = _texture->getSize();
    Vertex2 temp;
    temp.color = Color4::WHITE;

    temp.position = dst.origin;
    temp.texcoord.x = src.origin.x/tsize.width;
    temp.texcoord.y = 1-src.origin.y/tsize.height;
    _vertices.push_back(temp);
    _indices.push_back(offset);
    
    temp.position.x = dst.origin.x;
    temp.position.y = dst.origin.y+dst.size.height;
    temp.texcoord.x = src.origin.x/tsize.width;
    temp.texcoord.y = 1-(src.origin.y+src.size.height)/tsize.height;
    _vertices.push_back(temp);
    _indices.push_back(offset+1);
    
    temp.position.x = dst.origin.x+dst.size.width;
    temp.position.y = dst.origin.y+dst.size.height;
    temp.texcoord.x = (src.origin.x+src.size.width)/tsize.width;
    temp.texcoord.y = 1-(src.origin.y+src.size.height)/tsize.height;
    _vertices.push_back(temp);
    _indices.push_back(offset+2);
    _indices.push_back(offset);
    _indices.push_back(offset+2);
    
    temp.position.x = dst.origin.x+dst.size.width;
    temp.position.y = dst.origin.y;
    temp.texcoord.x = (src.origin.x+src.size.width)/tsize.width;
    temp.texcoord.y = 1-src.origin.y/tsize.height;
    _vertices.push_back(temp);
    _indices.push_back(offset+3);

    return offset+4;
}

/**
 * Draws this Node via the given SpriteBatch.
 *
 * This method only worries about drawing the current node.  It does not
 * attempt to render the children.
 *
 * This is the method that you should override to implement your custom
 * drawing code.  You are welcome to use any OpenGL commands that you wish.
 * You can even skip use of the SpriteBatch.  However, if you do so, you
 * must flush the SpriteBatch by calling end() at the start of the method.
 * in addition, you should remember to call begin() at the start of the
 * method.
 *
 * This method provides the correct transformation matrix and tint color.
 * You do not need to worry about whether the node uses relative color.
 * This method is called by render() and these values are guaranteed to be
 * correct.  In addition, this method does not need to check for visibility,
 * as it is guaranteed to only be called when the node is visible.
 *
 * @param batch     The SpriteBatch to draw with.
 * @param transform The global transformation matrix.
 * @param tint      The tint to blend with the Node color.
 */
void NinePatch::draw(const std::shared_ptr<SpriteBatch>& batch, const Mat4& transform, Color4 tint) {
    if (!_rendered) {
        generateRenderData();
    }
        
    batch->setColor(tint);
    batch->setTexture(_texture);
    batch->setBlendEquation(_blendEquation);
    batch->setBlendFunc(_srcFactor, _dstFactor);
    batch->fill(_vertices.data(),(unsigned int)_vertices.size(),0,
                _indices.data(), (unsigned int)_indices.size(), 0,
                transform);
}

