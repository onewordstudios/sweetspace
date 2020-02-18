//
//  CULabel.cpp
//  Cornell University Game Library (CUGL)
//
//  This module provides a scene graph node that displays a single line of text.
//  We have moved multiline text support to a separate class (which has not
//  yet been implemented).
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
//  Version: 7/6/16
#include <cugl/2d/CULabel.h>
#include <cugl/assets/CUSceneLoader.h>
#include <cugl/assets/CUAssetManager.h>

using namespace cugl;


#define UNKNOWN_STR "<unknown>"
#pragma mark Constructors

/**
 * Creates an uninitialized label with no text or font information.
 *
 * You must initialize this Label before use.
 *
 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a Node on the
 * heap, use one of the static constructors instead.
 */
Label::Label() : Node(),
_foreground(Color4::BLACK),
_background(Color4::CLEAR),
_halign(HAlign::LEFT),
_valign(VAlign::BOTTOM),
_rendered(false),
_blendEquation(GL_FUNC_ADD),
_srcFactor(GL_SRC_ALPHA),
_dstFactor(GL_ONE_MINUS_SRC_ALPHA)
{}

/**
 * Disposes all of the resources used by this label.
 *
 * A disposed Label can be safely reinitialized. Any children owned by this
 * node will be released.  They will be deleted if no other object owns them.
 *
 * It is unsafe to call this on a Label that is still currently inside of
 * a scene graph.
 */
void Label::dispose() {
    clearRenderData();
    _text.clear();
    _font = nullptr;
    _foreground = Color4::BLACK;
    _background = Color4::CLEAR;
    _halign = HAlign::LEFT;
    _valign = VAlign::BOTTOM;
    _padding = Vec2::ZERO;
    _rendered = false;
    Node::dispose();
}

/**
 * Initializes a label with the given size and font
 *
 * The text is empty and may be set later with {@link setText(std::string)}.
 *
 * @param size  The size of the label to display
 * @param font  The font for this label
 *
 * @return true if initialization was successful.
 */
bool Label::init(const Size& size, const std::shared_ptr<Font>& font) {
    if (font == nullptr) {
        CUAssertLog(false, "The font is undefined");
    } else if (_font != nullptr || _font != nullptr) {
        CUAssertLog(false, "Label is already initialized");
    } else if (Node::init()) {
        setContentSize(size);
        _font = font;
        return true;
    }
    return false;
}

/**
 * Initializes a label with the given text and font
 *
 * The label will be sized to fit the rendered text exactly. That is, the
 * height will be the maximum height of the font, and the width will be the
 * sum of the advance of the rendered characters.  That means that there
 * may be some natural spacing around the characters.
 *
 * All unprintable characters will be removed from the string.  This
 * includes tabs and newlines. They will be replaced by spaces. If any
 * glyphs are missing from the font atlas, they will not be rendered.
 *
 * @param text  The text to display in the label
 * @param font  The font for this label
 *
 * @return true if initialization was successful.
 */
bool Label::initWithText(const std::string& text, const std::shared_ptr<Font>& fontatlas) {
    if (fontatlas == nullptr) {
        CUAssertLog(false, "The font is undefined");
    } else if (_font != nullptr) {
        CUAssertLog(false, "Label is already initialized");
    } else if (Node::init()) {
        _font = fontatlas;
        setText(text,true);
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
 *      "font":         The name of a previously loaded font asset
 *      "text":         The initial label text
 *      "foreground":   A four-element integer array. Values should be 0..255
 *      "background":   A four-element integer array. Values should be 0..255
 *      "padding":      A two-element float array.
 *      "halign":       One of 'left', 'center', 'right', 'hard left',
 *                      'true center' and 'hard right'.
 *      "valign":       One of 'top', 'middle', 'bottom', 'hard top',
 *                      'true middle' and 'hard bottom'.
 *
 * The attribute 'font' is REQUIRED.  All other attributes are optional.
 *
 * @param loader    The scene loader passing this JSON file
 * @param data      The JSON object specifying the node
 *
 * @return true if initialization was successful.
 */
bool Label::initWithData(const SceneLoader* loader, const std::shared_ptr<JsonValue>& data) {
    if (_font != nullptr) {
        CUAssertLog(false, "Label is already initialized");
        return false;
    } else if (!data) {
        return init();
    } else if (!Node::initWithData(loader, data)) {
        return false;
    }
    
    // All of the code that follows can corrupt the position.
    Vec2 coord = getPosition();
    
    // Set the font
    const AssetManager* assets = loader->getManager();
    auto font = assets->get<Font>(data->getString("font",UNKNOWN_STR));
    if (font == nullptr) {
        CUAssertLog(false, "The font is undefined");
        return false;
    }
    
    _font = font;
    if (data->has("text")) {
        setText(data->getString("text"),!data->has("size"));
    }

    bool update = false;
    if (data->has("foreground")) {
        update = true;
        JsonValue* col = data->get("foreground").get();
        CUAssertLog(col->size() == 4, "'foreground' must be a 4-element array");
        _foreground.r = col->get(0)->asInt(0);
        _foreground.g = col->get(1)->asInt(0);
        _foreground.b = col->get(2)->asInt(0);
        _foreground.a = col->get(3)->asInt(0);
    }
    
    if (data->has("background")) {
        update = true;
        JsonValue* col = data->get("background").get();
        CUAssertLog(col->size() == 4, "'background' must be a 4-element array");
        _background.r = col->get(0)->asInt(0);
        _background.g = col->get(1)->asInt(0);
        _background.b = col->get(2)->asInt(0);
        _background.a = col->get(3)->asInt(0);
    }

    if (update) { updateColor(); }

    if (data->has("padding")) {
        JsonValue* pad = data->get("padding").get();
        CUAssertLog(pad->size() == 2, "'padding' must be a 2-element array");
        float dx = pad->get(0)->asFloat(0.0f);
        float dy = pad->get(1)->asFloat(0.0f);
        setPadding(dx,dy);
    }
    
    if (data->has("halign")) {
        std::string align = data->getString("halign",UNKNOWN_STR);
        HAlign value = HAlign::LEFT;
        if (align == "center") {
            value = HAlign::CENTER;
        } else if (align == "right") {
            value = HAlign::RIGHT;
        } else if (align == "hard left") {
            value = HAlign::HARDLEFT;
        } else if (align == "true center") {
            value = HAlign::TRUECENTER;
        } else if (align == "hard right") {
            value = HAlign::HARDRIGHT;
        }
        setHorizontalAlignment(value);
    }
    
    if (data->has("valign")) {
        std::string align = data->getString("valign",UNKNOWN_STR);
        VAlign value = VAlign::BOTTOM;
        if (align == "middle") {
            value = VAlign::MIDDLE;
        } else if (align == "top") {
            value = VAlign::TOP;
        } else if (align == "hard bottom") {
            value = VAlign::HARDBOTTOM;
        } else if (align == "true middle") {
            value = VAlign::TRUEMIDDLE;
        } else if (align == "hard top") {
            value = VAlign::HARDTOP;
        }
        setVerticalAlignment(value);
    }
    
    // Now redo the position
    setPosition(coord);
    return true;
}


#pragma mark -
#pragma mark Text Attributes
/**
 * Sets the text for this label.
 *
 * All unprintable characters will be removed from the string.  This
 * includes tabs and newlines. They will be replaced by spaces.
 *
 * The string must be in either ASCII or UTF8 format.  No other string
 * encodings are supported.  As all ASCII strings are also UTF8, you can
 * this effectively means that the text must be UTF8.
 *
 * If the font is missing glyphs in this string, the characters in the text
 * may be different than those displayed.
 *
 * Changing this value will regenerate the render data, and is potentially
 * expensive, particularly if the font does not have an atlas.
 *
 * @oaram text      The text for this label.
 * @oaram resize    Whether to resize the label to fit the new text.
 */
void Label::setText(const std::string& text, bool resize) {
    // Let's strip the non-printable characters first
    _text.clear();
    _text.reserve(text.size());
    for(auto it = text.begin(); it != text.end(); ++it) {
        if (((Uint32)*it) > 32 && *it != 127) {
            _text.push_back(*it);
        } else {
            _text.push_back(' ');
        }
    }
    
    computeSize();
    setHorizontalAlignment(_halign);
    setVerticalAlignment(_valign);
    if (resize) {
        setContentSize(_textbounds.size);
    }
    clearRenderData();
}

/**
 * Sets the padding of the rendered text.
 *
 * The application of padding depends on how the text is aligned in the
 * label.  For example, if the text has horizontal alignment LEFT and
 * vertical alignment BOTTOM, the padding will shift the text right and
 * up in the label.  On the other hand, if it has horizontal alignment
 * RIGHT and vertical alignment TOP, then it will shift the text left
 * and down.
 *
 * If the horizontal alignment is CENTER or TRUECENTER, then the x padding
 * will be ignored.  Similarly, if the vertical alignment is MIDDLE or
 * TRUEMIDDLE, then the y padding will be ignored.
 *
 * @param padx      The x-axis padding of the rendered text.
 * @param pady      The y-axis padding of the rendered text.
 */
void Label::setPadding(float padx, float pady) {
    switch (_halign) {
        case HAlign::LEFT:
        case HAlign::HARDLEFT:
            _textbounds.origin.x += (padx-_padding.x);
            break;
        case HAlign::RIGHT:
        case HAlign::HARDRIGHT:
            _textbounds.origin.x -= (padx-_padding.x);
            break;
        default:
            // Do nothing
            ;
    }
    switch (_valign) {
        case VAlign::BOTTOM:
        case VAlign::HARDBOTTOM:
            _textbounds.origin.y += (pady-_padding.y);
            break;
        case VAlign::TOP:
        case VAlign::HARDTOP:
            _textbounds.origin.y -= (pady-_padding.y);
            break;
        default:
            // Do nothing
            ;
    }
    
    _padding.set(padx,pady);
    clearRenderData();
}

/**
 * Sets the horizontal alignment of the text.
 *
 * This value represents the relationship between the text and its
 * label.  It functions as a sort of anchor.
 *
 * @param halign    The horizontal alignment of the text.
 */
void Label::setHorizontalAlignment(HAlign halign) {
    switch (halign) {
        case HAlign::LEFT:
            _textbounds.origin.x = _padding.x;
            break;
        case HAlign::CENTER:
            _textbounds.origin.x = (getContentWidth()-_textbounds.size.width)/2.0f;
            break;
        case HAlign::RIGHT:
            _textbounds.origin.x = getContentWidth()-_textbounds.size.width-_padding.x;
            break;
        case HAlign::HARDLEFT:
            _textbounds.origin.x = -_truebounds.origin.x+_padding.x;
            break;
        case HAlign::TRUECENTER:
            _textbounds.origin.x = (getContentWidth()-_truebounds.size.width)/2.0f;
            _textbounds.origin.x -= _truebounds.origin.x;
            break;
        case HAlign::HARDRIGHT:
            _textbounds.origin.x = getContentWidth()-_truebounds.size.width-_padding.x;
            _textbounds.origin.x -= _truebounds.origin.x;
            break;
    }
    
    
    _halign = halign;
    clearRenderData();
}

/**
 * Sets the horizontal alignment of the text.
 *
 * This value represents the relationship between the text and its
 * label.  It functions as a sort of anchor.
 *
 * @param halign    The horizontal alignment of the text.
 */
void Label::setVerticalAlignment(VAlign valign) {
    switch (valign) {
        case VAlign::BOTTOM:
            _textbounds.origin.y = _padding.y;
            break;
        case VAlign::MIDDLE:
            _textbounds.origin.y = (getContentHeight()-_textbounds.size.height)/2.0f;
            break;
        case VAlign::TOP:
            _textbounds.origin.y = getContentHeight()-_textbounds.size.height-_padding.y;
            break;
        case VAlign::HARDBOTTOM:
            _textbounds.origin.y = -_truebounds.origin.y+_padding.y;
            break;
        case VAlign::TRUEMIDDLE:
            _textbounds.origin.y = (getContentHeight()-_truebounds.size.height)/2.0f;
            _textbounds.origin.y -= _truebounds.origin.y;
            break;
        case VAlign::HARDTOP:
            _textbounds.origin.y = getContentHeight()-_truebounds.size.height-_padding.y;
            _textbounds.origin.y -= _truebounds.origin.y;
            break;
    }

    _valign = valign;
    clearRenderData();
}

/**
 * Returns the position of the baseline with respect to the Node origin
 *
 * The baseline does not necessarily align with the bottom of the text
 * bounds, as letters may overhang.
 *
 * @return the position of the baseline with respect to the Node origin
 */
float Label::getBaseLine() const {
    return _textbounds.origin.y-_font->getDescent();
}

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
 * In addition, if the rendered text is less than the content size, it
 * may be cut off in rendering.
 *
 * @param size  The untransformed size of the node.
 */
void Label::setContentSize(const Size& size) {
    Node::setContentSize(size);

    // This will fix the offsets
    setHorizontalAlignment(_halign);
    setVerticalAlignment(_valign);
}

#pragma mark -
#pragma mark Other Attributes

/**
 * Sets the background color of this label.
 *
 * If this color is not CLEAR (the default color), then the label will have
 * a colored backing rectangle.  The rectangle will extended from the origin
 * to the content size in Node space.
 *
 * @param color The background color of this label.
 */
void Label::setBackground(Color4 color) {
    if (_background == color) {
        return;
    } else if (_background == Color4::CLEAR || color == Color4::CLEAR) {
        clearRenderData();
    }
    _background = color;
    updateColor();
}

#pragma mark -
#pragma mark Rendering
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
 * @param matrix    The global transformation matrix.
 * @param tint      The tint to blend with the Node color.
 */
void Label::draw(const std::shared_ptr<SpriteBatch>& batch, const Mat4& transform, Color4 tint) {
    if (!_rendered) {
        generateRenderData();
    }

    batch->setBlendEquation(_blendEquation);
    batch->setBlendFunc(_srcFactor, _dstFactor);
    if (_background != Color4::CLEAR) {
        batch->setTexture(SpriteBatch::getBlankTexture());
        batch->setColor(tint);
        batch->fill(_vertices.data(),4,0,
                    _indices.data(),6,0,
                    transform);
    }
    batch->setTexture(_texture);
    batch->setColor(tint);
    if (_background != Color4::CLEAR) {
        batch->fill(_vertices.data(),(unsigned int)_vertices.size(),0,
                    _indices.data(),(unsigned int)_indices.size()-6,6,
                    transform);
    } else {
        batch->fill(_vertices.data(),(unsigned int)_vertices.size(),0,
                    _indices.data(),(unsigned int)_indices.size(),0,
                    transform);
    }
}


#pragma mark -
#pragma mark Internal Helpers
/**
 * Computes the default size of this label and stores it in _textbounds
 *
 * The default content size will use the font height and natural advance
 * of all the characters.  This will include natural spacing around the
 * characters.
 *
 * This method does not actually set the content size.  That should be
 * done manually.
 */
void Label::computeSize() {
    _textbounds.size = _font->getSize(_text);
    _truebounds = _font->getInternalBounds(_text);
    
    // This will fix the offsets
    setHorizontalAlignment(_halign);
    setVerticalAlignment(_valign);
}

/**
 * Allocate the render data necessary to render this node.
 */
void Label::generateRenderData() {
    // Make the backdrop
    Rect bounds(Vec2::ZERO,getContentSize());
    unsigned int vsize = 0;
    if (_background != Color4::CLEAR) {
        Vertex2 temp;
    
        // Bottom left corner
        temp.color = _background;
        _vertices.push_back(temp);

        // Bottom right corner
        temp.position.x = bounds.size.width;
        temp.color = _background;
        _vertices.push_back(temp);

        // Top right corner
        temp.position.x = bounds.size.width;
        temp.position.y = bounds.size.height;
        temp.color = _background;
        _vertices.push_back(temp);

        // Top left corner
        temp.position.x = 0;
        temp.position.y = bounds.size.height;
        temp.color = _background;
        _vertices.push_back(temp);

        // And now the indices
        _indices.push_back(0); _indices.push_back(1); _indices.push_back(2);
        _indices.push_back(2); _indices.push_back(3); _indices.push_back(0);
        vsize = 4;
    }
    // Glyphs are defined by _textbounds, regardless of alignment
    _texture = _font->getQuads(_text, _textbounds.origin, bounds, _vertices);
    for(int jj = vsize; jj < _vertices.size(); jj += 4) {
        _vertices[jj  ].color = _foreground;
        _vertices[jj+1].color = _foreground;
        _vertices[jj+2].color = _foreground;
        _vertices[jj+3].color = _foreground;
        _indices.push_back(jj  ); _indices.push_back(jj+1); _indices.push_back(jj+2);
        _indices.push_back(jj+2); _indices.push_back(jj+3); _indices.push_back(jj  );
    }

    _rendered = true;
}

/**
 * Clears the render data, releasing all vertices and indices.
 */
void Label::clearRenderData() {
    _vertices.clear();
    _indices.clear();
    _rendered = false;
}

/**
 * Updates the color value for any other data that needs it.
 *
 * This method is used to synchronize the background color and foreground
 * colors.
 */
void Label::updateColor() {
    if (!_rendered) {
        return;
    }
    
    for(auto it = _vertices.begin(); it != _vertices.begin()+4; ++it) {
        it->color = _background;
    }
    
    for(auto it = _vertices.begin()+4; it != _vertices.end(); ++it) {
        it->color = _foreground;
    }
}
