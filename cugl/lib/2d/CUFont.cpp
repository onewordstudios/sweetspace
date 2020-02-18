//
//  CUFont.cpp
//  Cornell University Game Library (CUGL)
//
//  This module provides a robust font asset with atlas support.  Unlike other
//  systems we decided to merge fonts and font atlases because it helps with
//  asset management.
//
//  This module makes heavy use of the cross-platform UTF8 utilities by
//  Nemanja Trifunovic ( https://github.com/nemtrif/utfcpp ).
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

#include <cugl/renderer/CUTexture.h>
#include <cugl/util/CUDebug.h>
#include <cugl/2d/CUFont.h>
#include <algorithm>
#include <utf8/utf8.h>
#include <deque>

using namespace cugl;

/** The amount of border to put around a glyph to prevent bleeding. */
#define GLYPH_BORDER    2

#pragma mark -
#pragma mark Constructors
/**
 * Creates a degenerate font with no data.
 *
 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
 * the heap, use one of the static constructors instead.
 */
Font::Font() :
_name(""),
_stylename(""),
_size(0),
_data(nullptr),
_fontHeight(0),
_fontAscent(0),
_fontDescent(0),
_fontLineSkip(0),
_fixedWidth(false),
_useKerning(true),
_style(Style::NORMAL),
_hints(Hinting::NORMAL),
_render(Resolution::BLENDED),
_hasAtlas(false),
_surface(nullptr) { }

/**
 * Deletes the font resources and resets all attributes.
 *
 * You must reinitialize the font to use it.
 */
void Font::dispose() {
    if (_surface != nullptr) { SDL_FreeSurface(_surface); _surface = nullptr;   }
    if (_data != nullptr)    { TTF_CloseFont(_data);      _data = nullptr;      }
    
    _name = "";
    _stylename = "";
    _size = 0;
    _data = nullptr;
    _fontHeight = 0;
    _fontAscent = 0;
    _fontDescent = 0;
    _fontLineSkip = 0;
    _fixedWidth = false;
    _useKerning = true;
    _style  = Style::NORMAL;
    _hints  = Hinting::NORMAL;
    _render = Resolution::BLENDED;
    _hasAtlas = false;
    _texture = nullptr;
    _glyphset.clear();
    _glyphsize.clear();
    _glyphmap.clear();
    _kernmap.clear();
}

/**
 * Initializes a font of the given size from the file.
 *
 * The font size is fixed on initialization.  It cannot be changed without
 * disposing of the entire font.  However, all other attributes may be
 * changed.
 *
 * @param file  The file with the font asset
 * @param size  The font size in points
 *
 * @return true if initialization is successful.
 */
bool Font::init(const std::string& file, int size) {
    if (_data != nullptr) {
        CUAssertLog(false,"Font %s already loaded", _name.c_str());
        return false;
    }
    _data = TTF_OpenFont(file.c_str(), size);
    if (_data == nullptr) {
        CUAssertLog(false, "Font initialization error: %s", TTF_GetError());
        return false;
    }
    _size = size;
    char* strng = TTF_FontFaceFamilyName(_data);
    _name = std::string(strng);

    strng = TTF_FontFaceStyleName(_data);
    _stylename = std::string(strng);
    
    _fontHeight   = TTF_FontHeight(_data);
    _fontAscent   = TTF_FontAscent(_data);
    _fontDescent  = TTF_FontDescent(_data);
    _fontLineSkip = TTF_FontLineSkip(_data);
    _fixedWidth   = TTF_FontFaceIsFixedWidth(_data) != 0;

    return true;
}



#pragma mark -
#pragma mark Attributes
/**
 * Returns true if this font has a glyph for the given (UNICODE) character.
 *
 * The Unicode representation uses the endianness native to the platform.
 * Therefore, this value should not be serialized.  Use UTF8 to represent
 * unicode in a platform-independent manner.
 *
 * If the font has an associated atlas, this will return true only if the
 * character is in the atlas.  You will need to clear the atlas to get the
 * full range of characters.
 *
 * @param thechar   The Unicode character to check.
 *
 * @return true if this font has a glyph for the given (UNICODE) character.
 */
bool Font::hasGlyph(Uint32 a) const {
    if (_hasAtlas) {
        return _glyphmap.find(a) != _glyphmap.end();
    }
    
    return TTF_GlyphIsProvided(_data, (Uint16)a) != 0;
}

/**
 * Returns true if this font can successfuly render the given string.
 *
 * The string may either be in UTF8 or ASCII; the method will handle
 * conversion automatically.
 *
 * If the font has an associated atlas, this will return true only if the
 * string characters are in the atlas.  You will need to clear the atlas
 * to get the full range of characters.
 *
 * @param text  The string to check.
 *
 * @return true if this font can successfuly render the given string.
 */
bool Font::hasString(const std::string& text) const {
    std::string line = text;
    std::string::iterator end_it = utf8::find_invalid(line.begin(), line.end());
    
    CUAssertLog(end_it == line.end(), "String '%s' has an invalid UTF-8 encoding",text.c_str());
    std::vector<Uint32> utf32;
    utf8::utf8to16(line.begin(), line.end(), back_inserter(utf32));
    
    // Now check conversion
    for(auto it = utf32.begin(); it != utf32.end(); ++it) {
        if (!hasGlyph(*it)) { return false; }
    }
    return true;
}

/**
 * Sets whether this font atlas uses kerning when rendering.
 *
 * Without kerning, each character is guaranteed to take up its enitre
 * advance when rendered.  This may make spacing look awkard. This
 * value is true by default.
 *
 * @param kerning   Whether this font atlas uses kerning when rendering.
 */
void Font::setKerning(bool kerning) {
    _useKerning = kerning;
    TTF_SetFontKerning(_data, _useKerning);
}


#pragma mark -
#pragma mark Settings

/**
 * Sets the style for this font.
 *
 * Changing this value will delete any atlas that is present.  The atlas
 * must be regenerated.
 *
 * With the exception of normal style (which is an absent of any style), all
 * of the styles may be combined.  So it is possible to have a bold, italic,
 * underline font with strikethrough. To combine styles, simply treat the
 * Style value as a bitmask, and combine them with bitwise operations.
 *
 * @param style The style for this font.
 */
void Font::setStyle(Style style) {
    clearAtlas(); _style = style;
    TTF_SetFontStyle(_data, (int)style);
}

/**
 * Sets the rasterization hints
 *
 * Changing this value will delete any atlas that is present.  The atlas
 * must be regenerated.
 *
 * Hinting is used to align the font to a rasterized grid. At low screen
 * resolutions, hinting is critical for producing clear, legible text
 * (particularly if you are not supporting antialiasing).
 *
 * @param hinting   The rasterization hints
 */
void Font::setHinting(Hinting hinting) {
    clearAtlas(); _hints = hinting;
    TTF_SetFontHinting(_data, (int)hinting);
}

#pragma mark -
#pragma mark Measurements
/**
 * Returns the glyph metrics for the given (ASCII) character.
 *
 * See {@link FontGlyphMetrics} for an explanation of the data provided by
 * this method. This method will fail if the glyph is not in this font.
 *
 * @param thechar   The ASCII character to measure.
 *
 * @return the glyph metrics for the given (ASCII) character.
 */
const Font::Metrics Font::getMetrics(Uint32 thechar) const {
    if (_hasAtlas) {
        CUAssertLog(_glyphmap.find(thechar) != _glyphmap.end(), "Character '%c' is not supported", thechar);
        return _glyphsize.at(thechar);
    }
    
    CUAssertLog(TTF_GlyphIsProvided(_data, (Uint16)thechar), "Character '%c' is not supported", thechar);
    return computeMetrics(thechar);
}

/**
 * Returns the kerning adjustment between the two (Unicode) characters.
 *
 * This value is the amount of overlap (in pixels) between any two adjacent
 * character glyphs rendered by this font.  If the value is 0, there is no
 * kerning for this pair.
 *
 * The Unicode representation uses the endianness native to the platform.
 * Therefore, this value should not be serialized.  Use UTF8 to represent
 * unicode in a platform-independent manner.
 *
 * @param a     The first Unicode character in the pair
 * @param b     The second Unicode character in the pair
 *
 * @return the kerning adjustment between the two (Unicode) characters.
 */
unsigned int Font::getKerning(Uint32 a, Uint32 b) const {
    if (_hasAtlas) {
        CUAssertLog(_glyphmap.find(a) != _glyphmap.end(), "Character '%c' is not supported", a);
        CUAssertLog(_glyphmap.find(b) != _glyphmap.end(), "Character '%c' is not supported", b);
        return _kernmap.at(a).at(b);
    }
    
    CUAssertLog(TTF_GlyphIsProvided(_data, (Uint16)a), "Character '%c' is not supported", a);
    CUAssertLog(TTF_GlyphIsProvided(_data, (Uint16)b), "Character '%c' is not supported", b);

    return computeKerning(a, b);
}

/**
 * Returns the size (in pixels) necessary to render this string.
 *
 * The string may either be in UTF8 or ASCII; the method will handle
 * conversion automatically.  However, by setting the optional value
 * 'utf8' to false, you can speed up the method by skipping the
 * text conversion.
 *
 * This size is a conservative estimate to render the string.  The height
 * is guaranteed to be the maximum height of the font, regardless of the
 * text measured.  In addition, the measurement will include the full
 * advance of the both the first and last characters.  This means that
 * there may be some font-specific padding around these characters.
 *
 * This measure does not actually render the string. This method will
 * not fail if it includes glyphs not present in the font, but it will
 * drop them when measuring the size.
 *
 * @param text  The string to measure.
 * @param utf8  Whether the string is a UTF8 that must be decoded.
 *
 * @return the size (in pixels) necessary to render this string.
 */
Size Font::getSize(const std::string& text, bool utf8) const {
    if (utf8) {
        return getSizeUTF8(text);
    }
    return getSizeASCII(text);
}

/**
 * Returns the pixel offset of the glyphs inside a rendered string.
 *
 * The result of {@link getSize(const std::string&, bool)} is very
 * conservative.  Even if no character uses the maximum height, it
 * provides the full height of the font.  Furthermore, if the last
 * character does not use the full advance, there will be padding after
 * that character.
 *
 * The rectangle returned by this method provide the internal bounds of
 * the rendered text.  The value is in "text space".  If a string is
 * rendered at position (0,0), this is the bounding box for all of the
 * glyphs that are actually rendered.  It is the tightest bounding box
 * that can fit all of the generated glyph.  You can use this rectangle
 * to eliminate any font-specific spacing that may have been placed
 * around the glyphs.
 *
 * For example, suppose the string is "ah".  In many fonts, these two
 * glyphs would not dip below the baseline.  Therefore, the y value of
 * the returned rectangle would be at the font baseline, indicating that
 * it is safe to start rendering there.
 *
 * This measure does not actually render the string. This method will
 * fail if it includes glyphs not present in the font. We cannot get 
 * an accurate measurement when glyphs are missing.
 *
 * @param text  The string to measure.
 * @param utf8  Whether the string is a UTF8 that must be decoded.
 *
 * @return the size of the quad sequence generated for this string.
 */
Rect Font::getInternalBounds(const std::string& text, bool utf8) const {
    if (utf8) {
        return getInternalBoundsUTF8(text);
    }
    return getInternalBoundsASCII(text);
}


#pragma mark -
#pragma mark Atlas Support
/**
 * Deletes the current atlas
 *
 * The font will use direct rendering until a new atlas is created.
 */
void Font::clearAtlas() {
    if (_surface != nullptr) { SDL_FreeSurface(_surface); _surface = nullptr;   }
    _texture = nullptr;
    _glyphmap.clear();
    _glyphset.clear();
    _glyphsize.clear();
    _kernmap.clear();
    _hasAtlas = false;
}

/**
 * Creates an atlas for the ASCII characters in this font.
 *
 * Only the ASCII characters are added to the atlas, even if the font has
 * support for more characters.  You should use a character set method
 * if you want Unicode characters supported.
 *
 * This method does not generate the OpenGL texture, but does all other
 * work in creates the atlas.  In particular it creates the image buffer
 * so that texture creation is just one OpenGL call.  This creation will
 * happen the first time that {@link getAtlas()} is called.
 *
 * As a result, this method is thread safe. It may be called in any
 * thread, including threads other than the main one.
 *
 * @return true if the atlas was successfully created.
 */
bool Font::buildAtlasAsync() {
    int width = prepareAtlas();
    int height = _fontHeight;
    prepareAtlasKerning();
    computeAtlasSize(&width,&height);
    _hasAtlas = generateSurface(width,height);
    return _hasAtlas;
}

/**
 * Creates an for the given character set.
 *
 * The atlas only contains characters in the provided character set, and
 * will omit all other chacters.  This includes ASCII characters that may
 * be missing from the character set. The character set string must either
 * be in ASCII or UTF8 encoding. It will handle both automatically, but no
 * other encoding (e.g. Latin1) is accepted.
 *
 * This method does not generate the OpenGL texture, but does all other
 * work in creates the atlas.  In particular it creates the image buffer
 * so that texture creation is just one OpenGL call.  This creation will
 * happen the first time that {@link getAtlas()} is called.
 *
 * As a result, this method is thread safe. It may be called in any
 * thread, including threads other than the main one.
 *
 * @return true if the atlas was successfully created.
 */
bool Font::buildAtlasAsync(const std::string& charset) {
    int width = prepareAtlas(charset);
    int height = _fontHeight;
    prepareAtlasKerning();
    computeAtlasSize(&width,&height);
    _hasAtlas = generateSurface(width,height);
    return _hasAtlas;
}

/**
 * Returns the OpenGL texture for the associated atlas.
 *
 * When combined with a quad sequence generated by this font, this texture
 * can be used to draw a font in a {@link SpriteBatch}. If there is no
 * atlas, this method returns nullptr.
 *
 * @return the OpenGL texture for the associated atlas.
 */
const std::shared_ptr<Texture>& Font::getAtlas() {
    if (_surface != nullptr) {
        _texture = Texture::allocWithData(_surface->pixels, _surface->w, _surface->h);
        SDL_FreeSurface(_surface);
        _surface = nullptr;
    }
    return _texture;

}

#pragma mark -
#pragma mark Rendering
/**
 * Creates quads to render this string and stores them in vertices.
 *
 * This method will append the vertices to the given vertex list.  In
 * addition, it will return the texture that should be used with these
 * vertices. If this is an atlas, the texture will be that atlas.
 *
 * The string may either be in UTF8 or ASCII; the method will handle
 * conversion automatically.  However, by setting the optional value
 * 'utf8' to false, you can speed up the method by skipping the
 * text conversion.
 *
 * To use the quad sequence with {@link SpriteBatch}, you will also need
 * a sequence of indices, which this method does not provide.  However,
 * the indices should be obvious, as the vertices are all a sequence of
 * quads. Just create two triangles for each quad.  The quad vertices are
 * in the following order: top left, top right, bottom left, bottom right.
 *
 * The origin value determines the position of the bottom of the glyph,
 * including the descent.  It is not the position of the baseline.
 *
 * If this font has an atlas, it will return the atlas texture.  Otherwise,
 * it is returning a unique texture specifically generated for this string.
 *
 * This method will fail if the string is not supported by this font.
 *
 * @param text      The string to convert to render data.
 * @param origin    The position of the first character
 * @param vertices  The list to append the vertices to.
 * @param utf8      Whether the string is a UTF8 that must be decoded.
 *
 * @return the texture associated with the quads
 */
std::shared_ptr<Texture> Font::getQuads(const std::string& text, const Vec2& origin,
                                            std::vector<Vertex2>& vertices, bool utf8) {
    Rect bounds(origin,getSize(text));
    if (_hasAtlas) {
        getAtlasQuads(text,origin,bounds,vertices,utf8);
        return _texture;
    }
    
    return getRenderedQuads(text,origin,bounds,vertices,utf8);
}

/**
 * Creates quads to render this string and stores them in vertices.
 *
 * This method will append the vertices to the given vertex list.  In
 * addition, it will return the texture that should be used with these
 * vertices. If this is an atlas, the texture will be that atlas.
 *
 * The quad sequence is adjusted so that all of the vertices fit in the
 * provided rectangle.  This may mean that some of the glyphs are truncated
 * or even omitted.
 *
 * The string may either be in UTF8 or ASCII; the method will handle
 * conversion automatically.  However, by setting the optional value
 * 'utf8' to false, you can speed up the method by skipping the
 * text conversion.
 *
 * To use the quad sequence with {@link SpriteBatch}, you will also need
 * a sequence of indices, which this method does not provide.  However,
 * the indices should be obvious, as the vertices are all a sequence of
 * quads. Just create two triangles for each quad.  The quad vertices are
 * in the following order: top left, top right, bottom left, bottom right.
 *
 * The origin value determines the position of the bottom of the glyph,
 * including the descent.  It is not the position of the baseline.
 *
 * If this font has an atlas, it will return the atlas texture.  Otherwise,
 * it is returning a unique texture specifically generated for this string.
 *
 * @param text      The string to convert to render data.
 * @param origin    The position of the first character
 * @param rect      The bounding box for the quads.
 * @param vertices  The list to append the vertices to.
 * @param utf8      Whether the string is a UTF8 that must be decoded.
 *
 * @return the texture associated with the quads
 */
std::shared_ptr<Texture> Font::getQuads(const std::string& text, const Vec2& origin, const Rect& rect,
                                            std::vector<Vertex2>& vertices,  bool utf8) {
    if (_hasAtlas) {
        getAtlasQuads(text,origin,rect,vertices,utf8);
        return _texture;
    }
    
    return getRenderedQuads(text,origin,rect,vertices,utf8);

}

/**
 * Creates a single quad to render this character and stores it in vertices
 *
 * This method will append the vertices to the given vertex list.  In
 * addition, it will return the texture that should be used with these
 * vertices. If this is an atlas, the texture will be that atlas.
 *
 * Once the font is generated, offset will be adjusted to contain the next
 * place to render a character. This method will not generate anything if
 * the character is not supported by this font.
 *
 * @param thechar   The character to convert to render data
 * @param offset    The (unkerned) starting position of the quad
 * @param vertices  The list to append the vertices to
 *
 * If this font has an atlas, it will return the atlas texture.  Otherwise,
 * it is returning a unique texture specifically generated for this string.
 *
 * @return the texture associated with the quad
 */
std::shared_ptr<Texture> Font::getQuad(Uint32 thechar, Vec2& offset, std::vector<Vertex2>& vertices) {
    Rect bounds(offset.x,offset.y, (float)getMetrics(thechar).advance, (float)_fontHeight);
    if (_hasAtlas) {
        getAtlasQuad(thechar,offset,bounds,vertices);
        return _texture;
    }
    
    return getRenderedQuad(thechar,offset,bounds,vertices);
}

/**
 * Creates a single quad to render this character and stores it in vertices
 *
 * This method will append the vertices to the given vertex list.  In
 * addition, it will return the texture that should be used with these
 * vertices. If this is an atlas, the texture will be that atlas.
 *
 * The quad is adjusted so that all of the vertices fit in the provided
 * rectangle.  This may mean that no quad is generated at all.
 *
 * Once the font is generated, offset will be adjusted to contain the next
 * place to render a character. This method will not generate anything if
 * the character is not supported by this font.
 *
 * If this font has an atlas, it will return the atlas texture.  Otherwise,
 * it is returning a unique texture specifically generated for this string.
 *
 * @param thechar   The character to convert to render data
 * @param offset    The (unkerned) starting position of the quad
 * @param rect      The bounding box for the quad
 * @param vertices  The list to append the vertices to
 *
 * @return the texture associated with the quad
 */
std::shared_ptr<Texture> Font::getQuad(Uint32 thechar, Vec2& offset, const Rect& rect,
                                           std::vector<Vertex2>& vertices) {
    if (_hasAtlas) {
        getAtlasQuad(thechar,offset,rect,vertices);
        return _texture;
    }
    
    return getRenderedQuad(thechar,offset,rect,vertices);

}


#pragma mark -
#pragma mark Rendering Internals
/**
 * Creates quads to render this string and stores them in vertices.
 *
 * This method will append the vertices to the given vertex list. The quad
 * sequence is adjusted so that all of the vertices fit in the provided
 * rectangle.  This may mean that some of the glyphs are truncated or even
 * omitted.
 *
 * The string may either be in UTF8 or ASCII; the method will handle
 * conversion automatically.  However, by setting the optional value
 * 'utf8' to false, you can speed up the method by skipping the
 * text conversion.
 *
 * To use the quad sequence with {@link SpriteBatch}, you will also need
 * a sequence of indices, which this method does not provide.  However,
 * the indices should be obvious, as the vertices are all a sequence of
 * quads. Just create two triangles for each quad.  The quad vertices are
 * in the following order: top left, top right, bottom left, bottom right.
 *
 * The origin value determines the position of the bottom of the glyph,
 * including the descent.  It is not the position of the baseline.
 *
 * This method does not return anything because the quads use the atlas.
 *
 * @param text      The string to convert to render data.
 * @param origin    The position of the first character
 * @param rect      The bounding box for the quads.
 * @param vertices  The list to append the vertices to.
 * @param utf8      Whether the string is a UTF8 that must be decoded.
 */
void Font::getAtlasQuads(const std::string& text, const Vec2& origin, const Rect& rect,
                             std::vector<Vertex2>& vertices, bool utf8) {
    getAtlas(); // Make sure we have the texture
    std::string line = text;
    Vec2 offset = origin;
    
    if (!utf8) {
        for(int ii = 0; ii < line.size(); ii++) {
            if (ii > 0) {
                offset.x -= _kernmap[line[ii-1]][line[ii]];
            }
            ii = (getQuad(line[ii],offset,rect,vertices) ? ii+1 : (int)line.size());
        }
        return;
    }
    
    std::string::iterator end_it = utf8::find_invalid(line.begin(), line.end());
    CUAssertLog(end_it == line.end(), "String '%s' has an invalid UTF-8 encoding",text.c_str());
    std::vector<Uint32> utf32;
    utf8::utf8to32(line.begin(), line.end(), back_inserter(utf32));
    
    for(int ii = 0; ii < utf32.size();) {
        if (ii > 0) {
            offset.x -= _kernmap[utf32[ii-1]][utf32[ii]];
        }
        ii = (getQuad(utf32[ii],offset,rect,vertices) ? ii+1 : (int)utf32.size());
    }
}

/**
 * Creates quads to render this string and stores them in vertices.
 *
 * This method will append the vertices to the given vertex list.  In
 * addition, it will return the texture that should be used with these
 * vertices. If this is an atlas, the texture will be that atlas.
 *
 * The quad sequence is adjusted so that all of the vertices fit in the
 * provided rectangle.  This may mean that some of the glyphs are truncated
 * or even omitted.
 *
 * The string may either be in UTF8 or ASCII; the method will handle
 * conversion automatically.  However, by setting the optional value
 * 'utf8' to false, you can speed up the method by skipping the
 * text conversion.
 *
 * To use the quad sequence with {@link SpriteBatch}, you will also need
 * a sequence of indices, which this method does not provide.  However,
 * the indices should be obvious, as the vertices are all a sequence of
 * quads. Just create two triangles for each quad.  The quad vertices are
 * in the following order: top left, top right, bottom left, bottom right.
 *
 * The origin value determines the position of the bottom of the glyph,
 * including the descent.  It is not the position of the baseline.
 *
 * @param text      The string to convert to render data.
 * @param origin    The position of the first character
 * @param rect      The bounding box for the quads.
 * @param vertices  The list to append the vertices to.
 * @param utf8      Whether the string is a UTF8 that must be decoded.
 *
 * @return the texture associated with the quads
 */
std::shared_ptr<Texture> Font::getRenderedQuads(const std::string& text, const Vec2& origin, const Rect& rect,
                                                    std::vector<Vertex2>& vertices, bool utf8) {
    SDL_Color color;
    color.r = color.g = color.b = color.a = 255;
    
    // We have to do this because of format differences
	SDL_Surface* surf1 = nullptr;
    SDL_Surface* surf2 = nullptr;
    
    switch (_render) {
        case Resolution::SOLID:
            surf1 = (utf8 ? TTF_RenderUTF8_Solid(_data, text.c_str(), color) :
                     TTF_RenderText_Solid(_data, text.c_str(), color));
            surf2 = allocSurface(surf1->w, surf1->h);
            SDL_SetSurfaceBlendMode(surf1, SDL_BLENDMODE_NONE);
            SDL_BlitSurface(surf1,NULL,surf2,NULL);
            SDL_FreeSurface(surf1);
            surf1 = surf2;
            break;
        case Resolution::SHADED:
            surf1 = (utf8 ? TTF_RenderUTF8_Blended(_data, text.c_str(), color) :
                     TTF_RenderText_Blended(_data, text.c_str(), color));
            surf2 = allocSurface(surf1->w, surf1->h);
            SDL_BlitSurface(surf1,NULL,surf2,NULL);
            SDL_FreeSurface(surf1);
            surf1 = surf2;
            break;
        case Resolution::BLENDED:
            surf1 = (utf8 ? TTF_RenderUTF8_Blended(_data, text.c_str(), color) :
                     TTF_RenderText_Blended(_data, text.c_str(), color));
            break;
    }
    
    if (surf1 == nullptr) { return nullptr; }
    std::shared_ptr<Texture> result = Texture::allocWithData(surf1->pixels,surf1->w,surf1->h);

    Rect quad(origin.x,origin.y, (float)surf1->w, (float)surf1->h);
    quad.intersect(rect);
    
    Vertex2 temp;
    
    // Bottom left
    temp.position = quad.origin;
    temp.color = Color4::WHITE;
    temp.texcoord.x = (quad.getMinX()-origin.x)/surf1->w;
    temp.texcoord.y = 1-(quad.getMinY()-origin.y)/surf1->h;
    vertices.push_back(temp);
    
    // Bottom right
    temp.position = quad.origin;
    temp.position.x += quad.size.width;
    temp.color = Color4::WHITE;
    temp.texcoord.x = (quad.getMaxX()-origin.x)/surf1->w;
    temp.texcoord.y = 1-(quad.getMinY()-origin.y)/surf1->h;
    vertices.push_back(temp);
    
    // Top right
    temp.position = quad.origin + quad.size;
    temp.color = Color4::WHITE;
    temp.texcoord.x = (quad.getMaxX()-origin.x)/surf1->w;
    temp.texcoord.y = 1-(quad.getMaxY()-origin.y)/surf1->h;
    vertices.push_back(temp);
    
    // Top left
    temp.position = quad.origin;
    temp.position.y += quad.size.height;
    temp.color = Color4::WHITE;
    temp.texcoord.x = (quad.getMinX()-origin.x)/surf1->w;
    temp.texcoord.y = 1-(quad.getMaxY()-origin.y)/surf1->h;
    vertices.push_back(temp);
    
    SDL_FreeSurface(surf1);
    return result;
}

/**
 * Creates a single quad to render this character and stores it in vertices
 *
 * This method will append the vertices to the given vertex list. The quad
 * is adjusted so that all of the vertices fit in the provided rectangle.
 * This may mean that no quad is generated at all.
 *
 * This method will return false if the right edge of the glyph is not
 * rendered. This lets us know if a character has exceeded the bounding
 * rectangle.  Without this, kerning may move the next character back
 * into range.
 *
 * @param thechar   The character to convert to render data
 * @param offset    The (unkerned) starting position of the quad
 * @param rect      The bounding box for the quad
 * @param vertices  The list to append the vertices to
 *
 * @return true if the right edge of the glyph was generated
 */
bool Font::getAtlasQuad(Uint32 thechar, Vec2& offset, const Rect& rect,
                            std::vector<Vertex2>& vertices) {
    // Technically, this answer is correct
    if (!hasGlyph(thechar)) { return true; }
    
    Rect bounds = _glyphmap[thechar];
    Rect quad(offset,bounds.size);
    
    // Skip over glyph, but recognize we may have later glyphs
    if (!rect.doesIntersect(quad)) {
        offset.x += bounds.size.width;
        return quad.getMaxX() <= rect.getMaxX();
    }
    
    // Compute intersection and adjust cookie cutter
    quad.intersect(rect);
    bool result = quad.getMaxX() <= rect.getMaxX();
    
    // REMEMBER! Bounds and rect have different y-orientations.
    bounds.origin.x += quad.origin.x-offset.x;
    bounds.origin.y -= quad.origin.y+quad.size.height-offset.y-bounds.size.height;
    
    offset.x += bounds.size.width;
    bounds.size = quad.size;
    
    int width  = _texture->getWidth();
    int height = _texture->getHeight();
    Vertex2 temp;
    
    // Bottom left
    temp.position = quad.origin;
    temp.color = Color4::WHITE;
    temp.texcoord.x = bounds.origin.x/(float)width;
    temp.texcoord.y = (bounds.origin.y+bounds.size.height)/(float)height;
    vertices.push_back(temp);
    
    // Bottom right
    temp.position = quad.origin;
    temp.position.x += bounds.size.width;
    temp.color = Color4::WHITE;
    temp.texcoord.x = (bounds.origin.x+bounds.size.width)/(float)width;
    temp.texcoord.y = (bounds.origin.y+bounds.size.height)/(float)height;
    vertices.push_back(temp);
    
    // Top right
    temp.position = quad.origin+bounds.size;
    temp.color = Color4::WHITE;
    temp.texcoord.x = (bounds.origin.x+bounds.size.width)/(float)width;
    temp.texcoord.y = bounds.origin.y/(float)height;
    vertices.push_back(temp);
    
    // Top left
    temp.position = quad.origin;
    temp.position.y += bounds.size.height;
    temp.color = Color4::WHITE;
    temp.texcoord.x = bounds.origin.x/(float)width;
    temp.texcoord.y = bounds.origin.y/(float)height;
    vertices.push_back(temp);
    
    return result;
}

/**
 * Creates a single quad to render this character and stores it in vertices
 *
 * This method will append the vertices to the given vertex list.  In
 * addition, it will return the texture that should be used with these
 * vertices. If this is an atlas, the texture will be that atlas.
 *
 * The quad is adjusted so that all of the vertices fit in the provided
 * rectangle.  This may mean that no quad is generated at all.
 *
 * Once the font is generated, offset will be adjusted to contain the next
 * place to render a character. This method will not generate anything if
 * the character is not supported by this font.
 *
 * @param thechar   The character to convert to render data
 * @param offset    The (unkerned) starting position of the quad
 * @param rect      The bounding box for the quad
 * @param vertices  The list to append the vertices to
 *
 * @return the texture associated with the quads
 */
std::shared_ptr<Texture> Font::getRenderedQuad(Uint32 thechar, Vec2& offset, const Rect& rect,
                                                   std::vector<Vertex2>& vertices) {
    SDL_Color color;
    color.r = color.g = color.b = color.a = 255;
    
    // We have to do this because of format differences
	SDL_Surface* surf1 = nullptr;
    SDL_Surface* surf2 = nullptr;
    
    switch (_render) {
        case Resolution::SOLID:
            surf1 = TTF_RenderGlyph_Solid(_data, thechar, color);
            surf2 = allocSurface(surf1->w, surf1->h);
            SDL_SetSurfaceBlendMode(surf1, SDL_BLENDMODE_NONE);
            SDL_BlitSurface(surf1,NULL,surf2,NULL);
            SDL_FreeSurface(surf1);
            surf1 = surf2;
            break;
        case Resolution::SHADED:
            surf1 = TTF_RenderGlyph_Blended(_data, thechar, color);
            surf2 = allocSurface(surf1->w, surf1->h);
            SDL_BlitSurface(surf1,NULL,surf2,NULL);
            SDL_FreeSurface(surf1);
            surf1 = surf2;
            break;
        case Resolution::BLENDED:
            surf1 = TTF_RenderGlyph_Blended(_data, thechar, color);
            break;
    }
    
    if (surf1 == nullptr) { return nullptr; }
    std::shared_ptr<Texture> result = Texture::allocWithData(surf1->pixels,surf1->w,surf1->h);
    
    Rect quad(offset.x,offset.y, (float)surf1->w, (float)surf1->h);
    quad.intersect(rect);

    Vertex2 temp;
    
    // Bottom left
    temp.position = quad.origin;
    temp.color = Color4::WHITE;
    temp.texcoord.x = (quad.getMinX()-offset.x)/surf1->w;
    temp.texcoord.y = 1-(quad.getMinY()-offset.y)/surf1->h;
    vertices.push_back(temp);
    
    // Bottom right
    temp.position = quad.origin;
    temp.position.x += quad.size.width;
    temp.color = Color4::WHITE;
    temp.texcoord.x = (quad.getMaxX()-offset.x)/surf1->w;
    temp.texcoord.y = 1-(quad.getMinY()-offset.y)/surf1->h;
    vertices.push_back(temp);
    
    // Top right
    temp.position = quad.origin+quad.size;
    temp.color = Color4::WHITE;
    temp.texcoord.x = (quad.getMaxX()-offset.x)/surf1->w;
    temp.texcoord.y = 1-(quad.getMaxY()-offset.y)/surf1->h;
    vertices.push_back(temp);
    
    // Top left
    temp.position = quad.origin;
    temp.position.y += quad.size.height;
    temp.color = Color4::WHITE;
    temp.texcoord.x = (quad.getMinX()-offset.x)/surf1->w;
    temp.texcoord.y = 1-(quad.getMaxY()-offset.y)/surf1->h;
    vertices.push_back(temp);
    
    offset.x = quad.getMaxX();
    SDL_FreeSurface(surf1);
    return result;
}

/**
 * Returns the size (in pixels) necessary to render this string.
 *
 * This size is a conservative estimate to render the string.  The height
 * is guaranteed to be the maximum height of the font, regardless of the
 * text measured.  In addition, the measurement will include the full
 * advance of the both the first and last characters.  This means that
 * there may be some font-specific padding around these characters.
 *
 * This measure does not actually render the string. This method will
 * not fail if it includes glyphs not present in the font, but it will
 * drop them when measuring the size.
 *
 * @param text  The ASCII string to measure.
 *
 * @return the size (in pixels) necessary to render this string.
 */
Size Font::getSizeASCII(const std::string& text) const {
    if (!_hasAtlas) {
        int w, h;
        TTF_SizeText(_data,text.c_str(), &w, &h);
        return Size((float)w, (float)h);
    }
    
    // Atlas computation
    Size result(0, (float)_fontHeight);
    for(int ii = 0; ii < text.size(); ii++) {
        if (hasGlyph(text[ii])) {
            if (ii > 0) {
                result.width -= _kernmap.at((Uint32)text[ii-1]).at((Uint32)text[ii]);
            }
            result.width += _glyphsize.at((Uint32)text[ii]).advance;
        }
    }
    return result;
}

/**
 * Returns the size (in pixels) necessary to render this string.
 *
 * This size is a conservative estimate to render the string.  The height
 * is guaranteed to be the maximum height of the font, regardless of the
 * text measured.  In addition, the measurement will include the full
 * advance of the both the first and last characters.  This means that
 * there may be some font-specific padding around these characters.
 *
 * This measure does not actually render the string. This method will
 * not fail if it includes glyphs not present in the font, but it will
 * drop them when measuring the size.
 *
 * @param text  The UTF8 string to measure.
 *
 * @return the size (in pixels) necessary to render this string.
 */
Size Font::getSizeUTF8(const std::string& text) const {
    if (!_hasAtlas) {
        int w, h;
        TTF_SizeUTF8(_data,text.c_str(),&w,&h);
        return Size((float)w, (float)h);
    }
    
    // Do the conversion
    std::string line = text;
    std::string::iterator end_it = utf8::find_invalid(line.begin(), line.end());
    CUAssertLog(end_it == line.end(), "String '%s' has an invalid UTF-8 encoding",text.c_str());
    std::vector<Uint32> utf32;
    utf8::utf8to16(line.begin(), line.end(), back_inserter(utf32));

    
    // Atlas computation
    Size result(0, (float)_fontHeight);
    for(int ii = 0; ii < utf32.size(); ii++) {
        if (hasGlyph(utf32[ii])) {
            if (ii > 0 && hasGlyph(utf32[ii-1])) {
                result.width -= _kernmap.at(utf32[ii-1]).at(utf32[ii]);
            }
            result.width += _glyphsize.at(utf32[ii]).advance;
        }
    }
    return result;
}


/**
 * Returns the pixel offset of the glyphs inside a rendered string.
 *
 * The result of {@link getSize(const std::string&, bool)} is very
 * conservative.  Even if no character uses the maximum height, it
 * provides the full height of the font.  Furthermore, if the last
 * character does not use the full advance, there will be padding after
 * that character.
 *
 * The rectangle returned by this method provide the internal bounds of
 * the rendered text.  The value is in "text space".  If a string is
 * rendered at position (0,0), this is the bounding box for all of the
 * glyphs that are actually rendered.  It is the tightest bounding box
 * that can fit all of the generated glyph.  You can use this rectangle
 * to eliminate any font-specific spacing that may have been placed
 * around the glyphs.
 *
 * For example, suppose the string is "ah".  In many fonts, these two
 * glyphs would not dip below the baseline.  Therefore, the y value of
 * the returned rectangle would be at the font baseline, indicating that
 * it is safe to start rendering there.
 *
 * This measure does not actually render the string. This method will
 * not fail if it includes glyphs not present in the font, but it will
 * drop them when measuring the size.
 *
 * @param text  The ASCII string to measure.
 *
 * @return the size of the quad sequence generated for this string.
 */
Rect Font::getInternalBoundsASCII(const std::string& text) const {
    Rect result;
    Metrics metrics;
    
    // These values allow us to skip over characters
    Uint32 first = 0;
    Uint32 last  = 0;
    
    // To track the height
    int maxy = 0;
    int miny = 0;
    
    // First character
    for(int ii = 0; first == 0 && ii < text.size(); ii++) {
        Uint32 ch = (Uint32)text[ii];
        if (hasGlyph(ch)) {
            metrics = (_hasAtlas ? _glyphsize.at(ch) : computeMetrics(ch));
            result.origin.x = (float)metrics.minx;
            result.size.width = (float)metrics.advance-metrics.minx;
            maxy = (metrics.maxy > maxy ? metrics.maxy : maxy);
            miny = (metrics.miny < miny ? metrics.miny : miny);
            first = ch;
        }
    }
    
    if (first == 0) {
        return result;
    }
    
    // Later characters
    last = first;
    for(int ii = first+1; ii < text.size(); ii++) {
        Uint32 ch = (Uint32)text[ii];
        if (hasGlyph(ch)) {
            result.size.width -= _kernmap.at(last).at(ch);
            metrics = (_hasAtlas ? _glyphsize.at(ch) : computeMetrics(ch));
            result.size.width += metrics.advance;
            maxy = (metrics.maxy > maxy ? metrics.maxy : maxy);
            miny = (metrics.miny < miny ? metrics.miny : miny);
            last = ch;
        }
    }
    
    // Adjust for last character
    if (last != 32) {
        result.size.width -= metrics.advance-metrics.maxx;
        result.origin.y = (float)(-getDescent()+miny);
        result.size.height = (float)(maxy-miny);
    }
    return result;
}

/**
 * Returns the pixel offset of the glyphs inside a rendered string.
 *
 * The result of {@link getSize(const std::string&, bool)} is very
 * conservative.  Even if no character uses the maximum height, it
 * provides the full height of the font.  Furthermore, if the last
 * character does not use the full advance, there will be padding after
 * that character.
 *
 * The rectangle returned by this method provide the internal bounds of
 * the rendered text.  The value is in "text space".  If a string is
 * rendered at position (0,0), this is the bounding box for all of the
 * glyphs that are actually rendered.  It is the tightest bounding box
 * that can fit all of the generated glyph.  You can use this rectangle
 * to eliminate any font-specific spacing that may have been placed
 * around the glyphs.
 *
 * For example, suppose the string is "ah".  In many fonts, these two
 * glyphs would not dip below the baseline.  Therefore, the y value of
 * the returned rectangle would be at the font baseline, indicating that
 * it is safe to start rendering there.
 *
 * This measure does not actually render the string. This method will
 * not fail if it includes glyphs not present in the font, but it will
 * drop them when measuring the size.
 *
 * @param text  The UTF8 string to measure.
 *
 * @return the size of the quad sequence generated for this string.
 */
Rect Font::getInternalBoundsUTF8(const std::string& text) const {
    Rect result;
    Metrics metrics;
    
    // These values allow us to skip over characters
    int first = -1;
    Uint32 last  = 0;
    
    // To track the height
    int maxy = 0;
    int miny = 0;
    
    // Do the conversion
    std::string line = text;
    std::string::iterator end_it = utf8::find_invalid(line.begin(), line.end());
    CUAssertLog(end_it == line.end(), "String '%s' has an invalid UTF-8 encoding",text.c_str());
    std::vector<Uint32> utf32;
    utf8::utf8to16(line.begin(), line.end(), back_inserter(utf32));

    // First character
    for(int ii = 0; first == -1 && ii < utf32.size(); ii++) {
        Uint32 ch = utf32[ii];
        if (hasGlyph(ch)) {
            metrics = (_hasAtlas ? _glyphsize.at(ch) : computeMetrics(ch));
            result.origin.x = (float)metrics.minx;
            result.size.width = (float)(metrics.advance-metrics.minx);
            maxy = (metrics.maxy > maxy ? metrics.maxy : maxy);
            miny = (metrics.miny < miny ? metrics.miny : miny);
            first = ii;
        }
    }

    if (first == -1) {
        return result;
    }
    
    // Later characters
    last = utf32[first];
    for(int ii = first+1; ii < utf32.size(); ii++) {
        Uint32 ch = utf32[ii];
        if (hasGlyph(ch)) {
            result.size.width -= (_hasAtlas ? _kernmap.at(last).at(ch) : computeKerning(last, ch));
            metrics = (_hasAtlas ? _glyphsize.at(ch) : computeMetrics(ch));
            result.size.width += metrics.advance;
            maxy = (metrics.maxy > maxy ? metrics.maxy : maxy);
            miny = (metrics.miny < miny ? metrics.miny : miny);
            last = ch;
        }
    }
    
    // Adjust for last character
    if (last != 32) {
        result.size.width -= metrics.advance-metrics.maxx;
        result.origin.y = (float)(-getDescent()+miny);
        result.size.height = (float)(maxy-miny);
    }
    return result;
}


#pragma mark -
#pragma mark Atlas Preparation
/**
 * Prepares an atlas of all of the ASCII glyphs in this font
 *
 * This method records what glyphs are available in the atlas. It also
 * stores the metrics and initializes the bounding rectangles.
 *
 * @return the maximum width of this font atlas
 */
int Font::prepareAtlas() {
    // Check all the glyphs
    int maxwidth = 0;
    
    for(unsigned int ii = 32; ii < 127; ii++) {
        if (TTF_GlyphIsProvided(_data, (Uint16)ii)) {
            Metrics metrics = computeMetrics(ii);
            _glyphsize.emplace(ii,metrics);
            _glyphmap.emplace(ii,Rect(0,0, (float)(metrics.advance+GLYPH_BORDER), (float)(_fontHeight+GLYPH_BORDER)));
            _glyphset.push_back(ii);
            if (metrics.advance > maxwidth) {
                maxwidth = metrics.advance;
            }
        }
    }
    
    // Sort them by width
    std::sort(_glyphset.begin(),_glyphset.end(),[&](Uint32 a, Uint32 b) {
        int aad = _glyphsize[a].advance; int bad = _glyphsize[b].advance;
        return (aad > bad || (aad == bad && a > b));
    });
    
    return maxwidth;}

/**
 * Prepares an atlas of the font glyphs from the given charset.
 *
 * This method records what glyphs are available in the atlas. It also
 * stores the metrics and initializes the bounding rectangles.
 *
 * @param charset   The set of characters to support
 *
 * @return the maximum width of this font atlas
 */
int Font::prepareAtlas(std::string charset) {
    // Check all the glyphs
    int maxwidth = 0;
    
    std::string::iterator end_it = utf8::find_invalid(charset.begin(), charset.end());
    CUAssertLog(end_it == charset.end(), "String '%s' has an invalid UTF-8 encoding",charset.c_str());
    std::vector<Uint32> utf32;
    utf8::utf8to16(charset.begin(), charset.end(), back_inserter(utf32));
    
    for(auto it = utf32.begin(); it != utf32.end(); ++it) {
        CUAssertLog(*it <= USHRT_MAX, "SDL_TTF does not currently support UCS4");
        Uint16 thechar = (Uint16)*it;
        if (_glyphmap.find(thechar) == _glyphmap.end() && TTF_GlyphIsProvided(_data, (Uint16)thechar)) {
            Metrics metrics = computeMetrics(thechar);
            _glyphsize.emplace(thechar,metrics);
            _glyphmap.emplace(thechar,Rect(0,0, (float)(metrics.advance+GLYPH_BORDER), (float)(_fontHeight+GLYPH_BORDER)));
            _glyphset.push_back(thechar);
            if (metrics.advance > maxwidth) {
                maxwidth = metrics.advance;
            }
        }
    }
    
    // Sort them by width
    std::sort(_glyphset.begin(),_glyphset.end(),[&](Uint32 a, Uint32 b) {
        int aad = _glyphsize[a].advance; int bad = _glyphsize[b].advance;
        return (aad > bad || (aad == bad && a > b));
    });
    
    return maxwidth;
}

/**
 * Gathers the kerning information for the atlas.
 */
void Font::prepareAtlasKerning() {
    for(auto it = _glyphset.begin(); it != _glyphset.end(); ++it) {
        _kernmap.emplace(*it, std::unordered_map<Uint32, Uint32>());
        for(auto jt = _glyphset.begin(); jt != _glyphset.end(); ++jt) {
            _kernmap[*it].emplace(*jt, computeKerning(*it, *jt));
        }
    }
}

/**
 * Returns the metrics for the given character if available.
 *
 * This method returns a metric with all zeroes if no data is fount.
 *
 * @return the metrics for the given character if available.
 */
Font::Metrics Font::computeMetrics(Uint32 thechar) const {
    Metrics metrics;
    int success = TTF_GlyphMetrics(_data, thechar, &metrics.minx, &metrics.maxx,
                                   &metrics.miny,  &metrics.maxy, &metrics.advance);
    
    // Only store if we have metrics
    if (success != -1) {
        // Fix because there is a render difference.
        Uint16 str[2];
        str[0] = thechar; str[1] = 0;
        
        int w = 0;
        int h = 0;
        TTF_SizeUNICODE(_data, str, &w, &h);
        if (w != metrics.advance) {
            int diff = w-metrics.advance;
            metrics.minx += diff/2; metrics.maxx += diff/2;
            metrics.advance += diff;
        }
    }
    
    return metrics;
}

/**
 * Returns the kerning between the two characters if available.
 *
 * The method will return -1 if there either of the two characters are
 * not supported by this font.
 *
 * @return the kerning between the two characters if available.
 */
int Font::computeKerning(Uint32 a, Uint32 b) const {
    Uint16 str[3];
    str[0] = (Uint16)a;
    str[1] = (Uint16)b;
    str[2] = 0;

    int w1, w2;
    TTF_SizeUNICODE(_data, str, &w1, &w2);
    w2 =  (!_glyphsize.empty() ? _glyphsize.at(a).advance : computeMetrics(a).advance);
    w2 += (!_glyphsize.empty() ? _glyphsize.at(b).advance : computeMetrics(b).advance);
    return w2-w1;
}

/**
 * Computes the size of the atlas texture
 *
 * This method computes the minimum bounding box that will contain all
 * of the glyphs in this atlas.  The bounding box is guaranteed to have
 * dimensions that are a power of 2, so that the texture is compatible
 * with all graphics cards.
 *
 * The dimensions are store in the provided pointers.  The width should
 * be a value > 1.  For best results, the width should be the size of the
 * maximum character width.
 *
 * @param width     Integer to store the width in
 * @param height    Integer to store the height in
 */
void Font::computeAtlasSize(int* width, int* height) {
    // Make enough room for largest glyph
    *width  = nextPOT(*width+GLYPH_BORDER);
    *height = nextPOT(_fontHeight+GLYPH_BORDER);
    
    // Copy the glyphs to make a visited set
    int nrows  = 1;
    std::deque< wchar_t > copied;
    std::copy(_glyphset.begin(), _glyphset.end(), std::back_inserter(copied));
    
    std::vector<float> used;
    used.push_back(2); // Give us a spot for a 2-patch
    int line = 0;
    while (!copied.empty()) {
        
        // We have finished the line
        if (used[line] >= *width) {
            // There is no more room
            if (line+1 >= nrows) {
                if (*width < *height) {
                    *width *= 2; line = 0;
                } else {
                    int orows = nrows;
                    *height *= 2; nrows *=2;
                    used.reserve(nrows);
                    for(int ii = 0; ii < orows; ii++) {
                        used.push_back(0);
                    }
                    line++;
                }
            } else {
                line++;
            }
        }
        
        // Fit the largest glyph possible on this line
        bool found = false;
		auto pos = copied.begin();
        for(auto it = copied.begin(); !found && it != copied.end(); ++it) {
            if (_glyphsize[*it].advance < *width-used[line]) {
                used[line] += _glyphsize[*it].advance+GLYPH_BORDER;
				pos = it;
                found = true;
            }
        }
        
        if (!found) {
            used[line] = (float)*width;
		} else {
			copied.erase(pos);
		}
    }
}

/**
 * Returns a plan for the atlas as a rectangular array of characters.
 *
 * These characters are organized to fit in the size predicted by
 * computeSize.
 *
 * @param width     The width of the bounding box
 * @param height    The height of the bounding box
 *
 * @return a plan for the atlas as a rectangular array of characters.
 */
std::vector< std::vector<Uint32> > Font::planAtlas(int width, int height) {
    // Time to order the glyphs
    int line = 0;
    int left = width-2; // Give us a spot for a 2-patch
    
    // Create a (inverse) visited set
    std::deque< wchar_t > copied;
    std::copy(_glyphset.begin(), _glyphset.end(), std::back_inserter(copied));
    
    // Create the return value
    std::vector< std::vector<Uint32> > result;
    result.push_back( std::vector<Uint32>() );
    
    while (!copied.empty()) {
        // Go to the next line.
        if (left == 0) {
            left = width; line++;
            result.push_back( std::vector<Uint32>() );
        }
        
        // Find the largest glyph that will fit on line.
        bool found = false;
        int fheight = _fontHeight+GLYPH_BORDER;
		auto value = copied.begin();
        for(auto it = copied.begin(); !found && it != copied.end(); ++it) {
            wchar_t thechar = (wchar_t)(*it);
            int glwidth = _glyphsize[*it].advance+GLYPH_BORDER;
            if (glwidth < left) {
                result[line].push_back(thechar);
                _glyphmap[thechar].origin.x = (float)(width-left);
                _glyphmap[thechar].origin.y = (float)(line*fheight);
                left -= glwidth;
				value = it;
                found = true;
            }
        }
        
        // Queue next line if not found.
        if (!found) { 
			left = 0; 
		} else {
			copied.erase(value);
		}
    }
    
    return result;
}

/**
 * Takes the rectangular plan and arranges the glyphs in the SDL surface.
 *
 * @param rectangle A plan for the atlas as a rectangular array of characters.
 */
void Font::layoutAtlas(const std::vector< std::vector<Uint32> >& rectangle) {
    SDL_Rect srcrect, dstrect;
    
    SDL_Color color;
    color.r = color.g = color.b = color.a = 255;
    SDL_Color bkgrd;
    bkgrd.r = bkgrd.g = bkgrd.b = bkgrd.a = 0;
    
    // Add a 2 patch at the beginning
    srcrect.x = srcrect.y = 0;
    srcrect.w = srcrect.h = 2;
    SDL_FillRect(_surface,&srcrect,SDL_MapRGBA(_surface->format, 255, 255, 255, 255));
    
    for(auto it = _glyphset.begin(); it != _glyphset.end(); ++it) {
		SDL_Surface* temp = nullptr;
        switch (_render) {
            case Resolution::SOLID:
                temp = TTF_RenderGlyph_Solid(_data, *it, color);
                break;
            case Resolution::SHADED:
            case Resolution::BLENDED:
                temp = TTF_RenderGlyph_Blended(_data, *it, color);
                break;
        }
        
        // Resize the boundary now that spacing is safe.
        _glyphmap[*it].origin.x += GLYPH_BORDER/2;
        _glyphmap[*it].origin.y += GLYPH_BORDER/2;
        _glyphmap[*it].size.width  -= GLYPH_BORDER;
        _glyphmap[*it].size.height -= GLYPH_BORDER;
        
        // Convert to SDL rects
        dstrect.x = (int)_glyphmap[*it].origin.x;
        dstrect.y = (int)_glyphmap[*it].origin.y;
        srcrect.x = srcrect.y = 0;
        dstrect.w = srcrect.w = (int)_glyphmap[*it].size.width;
        dstrect.h = srcrect.h = (int)_glyphmap[*it].size.height;
        
        // Blit on to atlas
        if (_render != Resolution::SHADED) {
            SDL_SetSurfaceBlendMode(temp, SDL_BLENDMODE_NONE);
        }
        SDL_BlitSurface(temp,&srcrect,_surface,&dstrect);
        SDL_FreeSurface(temp);
    }
}

/**
 * Generates an SDL surface for the font atlas.
 *
 * The data from this surface can be used to generate the OpenGL texture.
 *
 * @param width     The width of the bounding box
 * @param height    The height of the bounding box
 *
 * @return true if the surface was successfully generated.
 */
bool Font::generateSurface(int width, int height) {
    _surface = allocSurface(width, height);
    layoutAtlas(planAtlas(width,height));
    return _surface != nullptr;
}

/**
 * Allocates a blank surface of the given size.
 *
 * This method is necessary because SDL surface allocation is quite
 * involved when you want proper alpha support.
 *
 * @return a blank surface of the given size.
 */
SDL_Surface* Font::allocSurface(int width, int height) {
    SDL_Surface* result;
    
    // Masks appear to be necessary for alpha support
    Uint32 rmask, gmask, bmask, amask;
    
    // Unfortunately, masks are endian
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif
    
    result = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 32, rmask, gmask, bmask, amask);
    SDL_SetSurfaceBlendMode(result, SDL_BLENDMODE_BLEND);
    SDL_FillRect(result, NULL, SDL_MapRGBA(result->format, 0, 0, 0, 0));
    return result;
}


