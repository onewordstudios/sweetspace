//
//  CUFont.h
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

#ifndef __CU_FONT_H__
#define __CU_FONT_H__

#include <string>
#include <unordered_map>
#include <vector>
#include <cugl/math/CUSize.h>
#include <cugl/math/CURect.h>
#include <cugl/renderer/CUTexture.h>
#include <cugl/renderer/CUVertex.h>
#include <SDL/SDL_ttf.h>

namespace cugl {
    
/**
 * This class represents a true type font at a fixed size.
 *
 * It is possible to change many of the font settings after loading.  However,
 * the size is fixed and cannot be changed.  If you want a different size of
 * the same font, you must load it as a new asset.
 *
 * This font can be used to generate textures and quads for rendering text to
 * the screen. This is typically done via the {@link Label} class, though the
 * methods are available for any rendering pipeline.
 *
 * Rendering ASCII text is easy.  For unicode support however, you need to 
 * encode you text properly. the only unicode encoding that we support is UTF8.  
 * For the reason why, see
 *
 *      http://utf8everywhere.org/#how
 *
 * This font can also support an atlas.  This is a texture with all of the 
 * avaliable characters prerendered.  This single texture is then used to 
 * render the font on screen.  This is a potentially fast way of rendering
 * fonts, because it uses only one call to the graphics card.
 * 
 * However, there are disadvantages to a font atlas, and so you should be aware
 * of how to use them properly.  First of all, you are trading one texture with
 * many vertices, while rendering the font directly only needs four vertices.
 * If you have a static label that never changes, it may be faster to pay the
 * initial rendering cost of a font with no atlas.
 *
 * More importantly, font atlas textures can be huge if all glyphs are included.
 * For example, if we include all the unicode characters in Times New Roman at
 * 48 point font, the resulting atlas texture is 2048x4096, which is too much
 * for mobile devices.
 *
 * For this reason, the method {@link buildAtlas()} and its variants require 
 * that you explicitly specify a character set for the atlas.  Indeed, a 
 * character set is the only way to get unicode support; the basic atlas only 
 * includes ASCII characters.
 */
class Font {
#pragma mark Inner Classes
public:
    /**
     * This class is a simple struct to store glyph metric data
     *
     * A glyph metric stores the bounding box of a glyph, pluse the spacing
     * information around it.  The bouding box is offset from an origin, and
     * the advance the distance to the next glyph origin.  For more information,
     * see http://jcatki.no-ip.org:8080/SDL_ttf/SDL_ttf.html#SEC38
     */
    class Metrics {
    public:
        /** The minimum x-offset of the glyph from the origin (left side) */
        int minx;
        /** The maximum x-offset of the glyph from the origin (right side) */
        int maxx;
        /** The minimum y-offset of the glyph from the origin (bottom edge) */
        int miny;
        /** The maximum y-offset of the glyph from the origin (top edge) */
        int maxy;
        /** The distance from the origin of this glyph to the next */
        int advance;
    };

    /**
     * This enum represents the possible font styles.
     *
     * Generally, these styles would be encoded in the font face, but they
     * are provided to allow the user some flexibility with any font.
     *
     * With the exception of normal style (which is an absent of any style), all
     * of the styles may be combined.  So it is possible to have a bold, italic,
     * underline font with strikethrough. To combine styles, simply treat the
     * Style value as a bitmask, and combine them with bitwise operations.
     */
    enum class Style : int {
        /** The default style provided by this face */
        NORMAL      = TTF_STYLE_NORMAL,
        /** An adhoc created bold style */
        BOLD        = TTF_STYLE_BOLD,
        /** An adhoc created italics style */
        ITALIC      = TTF_STYLE_ITALIC,
        /** An adhoc created underline style */
        UNDERLINE   = TTF_STYLE_UNDERLINE,
        /** An adhoc created strike-through style */
        STRIKE      = TTF_STYLE_STRIKETHROUGH
    };
    
    /**
     * This enum represents the hints for rasterization.
     *
     * Hinting is used to align the font to a rasterized grid. At low screen
     * resolutions, hinting is critical for producing clear, legible text
     * (particularly if you are not supporting antialiasing).
     */
    enum class Hinting : int {
        /**
         * This corresponds to the default hinting algorithm, optimized for
         * standard gray-level rendering
         */
        NORMAL  = TTF_HINTING_NORMAL,
        /**
         * This is a lighter hinting algorithm for non-monochrome modes. Many
         * generated glyphs are more fuzzy but better resemble its original
         * shape. This is a bit like rendering on Mac OS X.
         */
        LIGHT   = TTF_HINTING_LIGHT,
        /**
         * This is a strong hinting algorithm that should only be used for
         * monochrome output. The result is probably unpleasant if the glyph
         * is rendered in non-monochrome modes.
         */
        MONO    = TTF_HINTING_MONO,
        /**
         * In this case, no hinting is usedm so the font may become very blurry
         * or messy at smaller sizes.
         */
        NONE    = TTF_HINTING_NONE
    };
    
    /**
     * This enum represents the font resolution for rendering.
     *
     * The option SOLID is only useful for the case where there is no atlas.
     * The preferred value for atlases and high quality fonts is BLENDED.  
     * However, when you need a font to "pop" out from a background, you may 
     * want SHADED instead.
     */
    enum class Resolution : int {
        /**
         * The text is renderized with no antialiasing.
         *
         * When rendering with this option, the font will have a very hard edge.
         * Rendering is faster than the other options, but this only matters
         * when the font has no atlas.
         */
        SOLID   = 0,
        /**
         * The text is renderized smoothly with a hard edge
         *
         * When rendering with this option, the font will have smooth edges, but
         * there will be a dark border around the font (regardless of the font
         * color).  This acts as a form of outlining and guarantees the font 
         * will always pop out from its background.
         *
         * Rendering fonts this way is much slower than SOLID, but has no
         * serious effects when using an atlas.
         */
        SHADED  = 1,
        /**
         * The text is renderized with a 32-bit RGBA surface.
         *
         * When rendering with this option, the font will have an antialiased 
         * edge that blends in with its background. This is the preferred
         * option for rendering high quality fonts.
         *
         * Rendering fonts this way is much slower than SOLID, but has no
         * serious effects when using an atlas.
         */
        BLENDED  = 2,
    };

#pragma mark -
#pragma mark Values
protected:
    /** The name of this font (typically the family name if known) */
    std::string _name;
    /** The name of this font style */
    std::string _stylename;
    /** The font size in points */
    int _size;
    
    /** The underlying SDL data */
    TTF_Font* _data;

    // Cached settings
    /** The (maximum) height of this font. It is the sum of ascent and descent. */
    unsigned int _fontHeight;
    /** The maximum distance from the baseline to the glyph bottom (always negative) */
    unsigned int _fontDescent;
    /** The maximum distance from the baseline to the glyph top (always positive) */
    unsigned int _fontAscent;
    /** The recommended line skip for this font */
    unsigned int _fontLineSkip;
    /** Whether this is a fixed width font */
    bool _fixedWidth;
    /** Whether to use kerning when rendering */
    bool _useKerning;
    
    // Render settings
    /** The font face style */
    Style _style;
    /** The rasterization hints */
    Hinting _hints;
    /** The rendering resolution (when there is no atlas) */
    Resolution _render;
    
    // Altas support
    /** Whether this font has an active atlas */
    bool _hasAtlas;
    /** The set of (unicode) glyphs supported by this atlas */
    std::vector<Uint32> _glyphset;
    /** The location of each glyph in the atlas texture */
    std::unordered_map<Uint32, Rect> _glyphmap;
    /** The cached metrics for each font glyph */
    std::unordered_map<Uint32, Metrics> _glyphsize;
    /** The kerning for each pair of characters */
    std::unordered_map<Uint32, std::unordered_map<Uint32, Uint32> > _kernmap;
    /** The OpenGL texture representing this atlas */
    std::shared_ptr<Texture> _texture;
    /** A (temporary) SDL surface for computing the atlas texture */
    SDL_Surface* _surface;

    
public:
#pragma mark -
#pragma mark Constructors
    /**
     * Creates a degenerate font with no data.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead.
     */
    Font();
    
    /**
     * Deletes this font, disposing of all resources.
     */
    ~Font() { dispose(); }
    
    /**
     * Deletes the font resources and resets all attributes.
     *
     * This will delete the original font information in addition to any
     * generating atlases.
     *
     * You must reinitialize the font to use it.
     */
    virtual void dispose();
    
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
    bool init(const std::string& file, int size);

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
    bool init(const char* file, int size) {
        return init(std::string(file),size);
    }
    
    
#pragma mark -
#pragma mark Static Constructors
    /**
     * Returns a newly allocated font of the given size from the file.
     *
     * The font size is fixed on creation.  It cannot be changed without
     * creating a new font asset.  However, all other attributes may be
     * changed.
     *
     * @param file  The file with the font asset
     * @param size  The font size in points
     *
     * @return a newly allocated font of the given size from the file.
     */
    static std::shared_ptr<Font> alloc(const std::string& file, int size) {
        std::shared_ptr<Font> result = std::make_shared<Font>();
        return (result->init(file,size) ? result : nullptr);
    }

    /**
     * Returns a newly allocated font of the given size from the file.
     *
     * The font size is fixed on creation.  It cannot be changed without
     * creating a new font asset.  However, all other attributes may be
     * changed.
     *
     * @param file  The file with the font asset
     * @param size  The font size in points
     *
     * @return a newly allocated font of the given size from the file.
     */
    static std::shared_ptr<Font> alloc(const char* file, int size) {
        std::shared_ptr<Font> result = std::make_shared<Font>();
        return (result->init(file,size) ? result : nullptr);
    }

#pragma mark -
#pragma mark Attributes
    /**
     * Returns the family name of this font.
     *
     * This method may return an empty string, meaning the information is not 
     * available.
     *
     * @return the family name of this font.
     */
    const std::string& getName() const { return _name; }

    /**
     * Returns the style name of this font.
     *
     * This method may return an empty string, meaning the information is not
     * available.
     *
     * @return the style name of this font.
     */
    const std::string& getStyleName() const { return _stylename; }
    
    /**
     * Returns the maximum height of this font.
     *
     * This is the sum of the ascent and the negative descent.  Any box that
     * is this many characters high can sucessfully hold a glyph from this 
     * font.
     *
     * @return the maximum height of this font.
     */
    int getHeight() const { return _fontHeight; }
    
    /**
     * Returns the maximum distance from the baseline to the bottom of a glyph.
     *
     * This value will always be negative.  You should add this value to the 
     * y position to shift the baseline down to the rendering origin.
     *
     * @return the maximum distance from the baseline to the bottom of a glyph.
     */
    int getDescent() const { return _fontDescent; }
    
    /**
     * Returns the maximum distance from the baseline to the top of a glyph.
     *
     * This value will always be positive.
     *
     * @return the maximum distance from the baseline to the top of a glyph.
     */
    int getAscent() const { return _fontAscent; }
    
    /**
     * Returns the recommended lineskip of this font.
     *
     * The line skip is the recommended height of a line of text. It is often
     * larger than the font height.
     *
     * @return the recommended lineskip of this font.
     */
    int getLineSkip() const { return _fontLineSkip; }
    
    /**
     * Returns true if the font is a fixed width font.
     *
     * Fixed width fonts are monospace, meaning every character that exists in
     * the font is the same width. In this case you can assume that a rendered
     * string's width is going to be the result of a simple calculation:
     *
     *      glyph_width * string_length
     *
     * @return true if the font is a fixed width font.
     */
    bool isFixedWidth() const { return _fixedWidth; }
    
    /**
     * Returns true if this font atlas uses kerning when rendering.
     *
     * Without kerning, each character is guaranteed to take up its enitre
     * advance when rendered.  This may make spacing look awkard. This 
     * value is true by default.
     *
     * @return true if this font atlas uses kerning when rendering.
     */
    bool usesKerning() const { return _useKerning; }
    
    /**
     * Sets whether this font atlas uses kerning when rendering.
     *
     * Without kerning, each character is guaranteed to take up its enitre
     * advance when rendered.  This may make spacing look awkard. This
     * value is true by default.
     *
     * @param kerning   Whether this font atlas uses kerning when rendering.
     */
    void setKerning(bool kerning);
    
    /**
     * Returns true if this font has a glyph for the given (ASCII) character.
     *
     * If the font has an associated atlas, this will return true only if the
     * character is in the atlas.  You will need to clear the atlas to get the
     * full range of characters.
     *
     * @param a     The ASCII character to check.
     *
     * @return true if this font has a glyph for the given (ASCII) character.
     */
    bool hasGlyph(char a) const { return hasGlyph((Uint32)a); }
    
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
     * @param a     The Unicode character to check.
     *
     * @return true if this font has a glyph for the given (UNICODE) character.
     */
    bool hasGlyph(Uint32 a) const;
    
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
    bool hasString(const std::string& text) const;
    
    /**
     * Returns true if this font can successfuly render the given string.
     *
     * If the font has an associated atlas, this will return true only if the
     * string characters are in the atlas.  You will need to clear the atlas
     * to get the full range of characters.
     *
     * @param text  The string to check.
     *
     * @return true if this font can successfuly render the given string.
     */
    bool hasString(const char* text) const {
        return hasString(std::string(text));
    }

#pragma mark -
#pragma mark Settings
    /**
     * Returns the style for this font.
     *
     * With the exception of normal style (which is an absent of any style), all
     * of the styles may be combined.  So it is possible to have a bold, italic,
     * underline font with strikethrough. To combine styles, simply treat the
     * Style value as a bitmask, and combine them with bitwise operations.
     *
     * @return the style for this font.
     */
    Style getStyle() const { return _style; }
    
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
    void setStyle(Style style);
    
    /** 
     * Returns the rasterization hints
     *
     * Hinting is used to align the font to a rasterized grid. At low screen
     * resolutions, hinting is critical for producing clear, legible text
     * (particularly if you are not supporting antialiasing).
     *
     * @return the rasterization hints
     */
    Hinting getHinting() const { return _hints; }
    
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
    void setHinting(Hinting hinting);
    
    /**
     * Returns the rendering resolution for this font.
     *
     * The option SOLID is only useful for the case where there is no atlas.
     * The preferred value for atlases and high quality fonts is BLENDED.
     * However, when you need a font to "pop" out from a background, you may
     * want SHADED instead.
     *
     * @return the rendering resolution for this font.
     */
    Resolution getResolution() const { return _render; }
    
    /**
     * Sets the rendering resolution for this font.
     *
     * Changing this value will delete any atlas that is present.  The atlas
     * must be regenerated.
     *
     * The option SOLID is only useful for the case where there is no atlas.
     * The preferred value for atlases and high quality fonts is BLENDED.
     * However, when you need a font to "pop" out from a background, you may
     * want SHADED instead.
     *
     * @param resolution    The rendering resolution for this font.
     */
    void setResolution(Resolution resolution) { clearAtlas(); _render = resolution; }


    
#pragma mark -
#pragma mark Measurements
    /**
     * Returns the glyph metrics for the given (ASCII) character.
     *
     * See {@link Metrics} for an explanation of the data provided by
     * this method. This method will fail if the glyph is not in this font.
     *
     * @param thechar   The ASCII character to measure.
     *
     * @return the glyph metrics for the given (ASCII) character.
     */
    const Metrics getMetrics(char thechar) const {
        return getMetrics((Uint32)thechar);
    }
    
    /**
     * Returns the glyph metrics for the given (Unicode) character.
     *
     * See {@link Metrics} for an explanation of the data provided by
     * this method. This method will fail if the glyph is not in this font.
     *
     * The Unicode representation uses the endianness native to the platform.
     * Therefore, this value should not be serialized.  Use UTF8 to represent
     * unicode in a platform-independent manner.
     *
     * @param thechar   The Unicode character to measure.
     *
     * @return the glyph metrics for the given (Unicode) character.
     */
    const Metrics getMetrics(Uint32 thechar) const;
    
    /**
     * Returns the kerning adjustment between the two (ASCII) characters.
     *
     * This value is the amount of overlap (in pixels) between any two adjacent
     * character glyphs rendered by this font.  If the value is 0, there is no
     * kerning for this pair.
     *
     * @param a     The first ASCII character in the pair
     * @param b     The second ASCII character in the pair
     *
     * @return the kerning adjustment between the two (ASCII) characters.
     */
    unsigned int getKerning(char a, char b) const {
        return getKerning((Uint32)a, (Uint32)b);
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
    unsigned int getKerning(Uint32 a, Uint32 b) const;
    
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
    Size getSize(const std::string& text, bool utf8=true) const;
    
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
    Size getSize(const char* text, bool utf8=true) const {
        return getSize(std::string(text), utf8);
    }
    
    /**
     * Returns the pixel offset of the glyphs inside a rendered string.
     *
     * The result of {@link getSize()} is very conservative.  Even if no 
     * character uses the maximum height, it provides the full height of 
     * the font.  Furthermore, if the last character does not use the full 
     * advance, there will be padding after that character.
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
     * The string may either be in UTF8 or ASCII; the method will handle
     * conversion automatically.  However, by setting the optional value
     * 'utf8' to false, you can speed up the method by skipping the
     * text conversion.
     *
     * This measure does not actually render the string. This method will
     * not fail if it includes glyphs not present in the font, but it will
     * drop them when measuring the size.
     *
     * @param text  The string to measure.
     * @param utf8  Whether the string is a UTF8 that must be decoded.
     *
     * @return the size of the quad sequence generated for this string.
     */
    Rect getInternalBounds(const std::string& text, bool utf8=true) const;
    
    /**
     * Returns the pixel offset of the glyphs inside a rendered string.
     *
     * The result of {@link getSize()} is very conservative.  Even if no 
     * character uses the maximum height, it provides the full height of 
     * the font.  Furthermore, if the last character does not use the full 
     * advance, there will be padding after that character.
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
     * The string may either be in UTF8 or ASCII; the method will handle
     * conversion automatically.  However, by setting the optional value
     * 'utf8' to false, you can speed up the method by skipping the
     * text conversion.
     *
     * This measure does not actually render the string. This method will
     * not fail if it includes glyphs not present in the font, but it will
     * drop them when measuring the size.
     *
     * @param text  The string to measure.
     * @param utf8  Whether the string is a UTF8 that must be decoded.
     *
     * @return the size of the quad sequence generated for this string.
     */
    Rect getInternalBounds(const char* text, bool utf8=true) const {
        return getInternalBounds(std::string(text), utf8);
    }

#pragma mark -
#pragma mark Atlas Support
    /**
     * Deletes the current atlas
     * 
     * The font will use direct rendering until a new atlas is created.
     */
    void clearAtlas();

    /**
     * Creates an atlas for the ASCII characters in this font.
     *
     * Only the ASCII characters are added to the atlas, even if the font has
     * support for more characters.  You should use a character set method
     * if you want Unicode characters supported.
     *
     * The character atlas texture is generated immediately, so the method
     * {@link getAtlas()} may be called with no delay.
     *
     * WARNING: This initializer is not thread safe.  It generates an OpenGL
     * texture, which means that it may only be called in the main thread.
     *
     * @return true if the atlas was successfully created.
     */
    bool buildAtlas() {
        bool result = buildAtlasAsync();
        return result && (getAtlas() != nullptr);
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
     * The character atlas texture is generated immediately, so the method
     * {@link getAtlas()} may be called with no delay.
     *
     * WARNING: This initializer is not thread safe.  It generates an OpenGL
     * texture, which means that it may only be called in the main thread.
     *
     * @param charset   The set of characters in the atlas
     *
     * @return true if the atlas was successfully created.
     */
    bool buildAtlas(const std::string& charset) {
        bool result = buildAtlasAsync(charset);
        return result && (getAtlas() != nullptr);
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
     * The character atlas texture is generated immediately, so the method
     * {@link getAtlas()} may be called with no delay.
     *
     * WARNING: This initializer is not thread safe.  It generates an OpenGL
     * texture, which means that it may only be called in the main thread.
     *
     * @param charset   The set of characters in the atlas
     *
     * @return true if the atlas was successfully created.
     */
    bool buildAtlas(const char* charset) {
        bool result = buildAtlasAsync(charset);
        return result && (getAtlas() != nullptr);
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
    bool buildAtlasAsync();
    
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
    bool buildAtlasAsync(const std::string& charset);

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
    bool buildAtlasAsync(const char* charset) {
        return buildAtlasAsync(std::string(charset));
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
    const std::shared_ptr<Texture>& getAtlas();

    /**
     * Returns true if this font has an active atlas.
     * 
     * If this method is true, then {@link getAtlas()} can be used to draw a
     * font in a {@link SpriteBatch}. 
     *
     * @return true if this font has an active atlas.
     */
    bool hasAtlas() const { return _hasAtlas; }
    
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
    std::shared_ptr<Texture> getQuads(const std::string& text, const Vec2& origin,
                                      std::vector<Vertex2>& vertices, bool utf8=true);
    
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
     * @param text      The string to convert to render data.
     * @param origin    The position of the first character
     * @param vertices  The list to append the vertices to.
     * @param utf8      Whether the string is a UTF8 that must be decoded.
     *
     * @return the texture associated with the quads
     */
    std::shared_ptr<Texture> getQuads(const char* text, const Vec2& origin,
                                      std::vector<Vertex2>& vertices, bool utf8=true)  {
        return getQuads(std::string(text),origin,vertices,utf8);
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
    std::shared_ptr<Texture> getQuads(const std::string& text, const Vec2& origin, const Rect& rect,
                                      std::vector<Vertex2>& vertices,  bool utf8=true);
    
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
    std::shared_ptr<Texture> getQuads(const char* text, const Vec2& origin, const Rect& rect,
                                      std::vector<Vertex2>& vertices, bool utf8=true) {
        return getQuads(std::string(text),origin,rect,vertices,utf8);
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
    std::shared_ptr<Texture> getQuad(Uint32 thechar, Vec2& offset, std::vector<Vertex2>& vertices);

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
    std::shared_ptr<Texture> getQuad(Uint32 thechar, Vec2& offset, const Rect& rect,
                                     std::vector<Vertex2>& vertices);
    
    
#pragma mark -
#pragma mark Rendering Internals
protected:
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
    void getAtlasQuads(const std::string& text, const Vec2& origin, const Rect& rect,
                       std::vector<Vertex2>& vertices, bool utf8);
    
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
    std::shared_ptr<Texture> getRenderedQuads(const std::string& text, const Vec2& origin, const Rect& rect,
                                              std::vector<Vertex2>& vertices, bool utf8);
    
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
    bool getAtlasQuad(Uint32 thechar, Vec2& offset, const Rect& rect, std::vector<Vertex2>& vertices);
    
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
    std::shared_ptr<Texture> getRenderedQuad(Uint32 thechar, Vec2& offset, const Rect& rect,
                                             std::vector<Vertex2>& vertices);
    
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
    Size getSizeASCII(const std::string& text) const;

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
    Size getSizeUTF8(const std::string& text) const;

    /**
     * Returns the pixel offset of the glyphs inside a rendered string.
     *
     * The result of {@link getSize()} is very conservative.  Even if no 
     * character uses the maximum height, it provides the full height of 
     * the font.  Furthermore, if the last character does not use the full 
     * advance, there will be padding after that character.
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
    Rect getInternalBoundsASCII(const std::string& text) const;

    /**
     * Returns the pixel offset of the glyphs inside a rendered string.
     *
     * The result of {@link getSize()} is very conservative.  Even if no 
     * character uses the maximum height, it provides the full height of 
     * the font.  Furthermore, if the last character does not use the full 
     * advance, there will be padding after that character.
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
    Rect getInternalBoundsUTF8(const std::string& text) const;

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
    int prepareAtlas();
    
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
    int prepareAtlas(std::string charset);
    
    /**
     * Gathers the kerning information for the atlas.
     */
    void prepareAtlasKerning();

    /**
     * Returns the metrics for the given character if available.
     * 
     * This method returns a metric with all zeroes if no data is fount.
     *
     * @return the metrics for the given character if available.
     */
    Metrics computeMetrics(Uint32 thechar) const;

    /**
     * Returns the kerning between the two characters if available.
     *
     * The method will return -1 if there either of the two characters are
     * not supported by this font.
     *
     * @return the kerning between the two characters if available.
     */
    int computeKerning(Uint32 a, Uint32 b) const;

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
    void computeAtlasSize(int* width, int* height);
    
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
    std::vector< std::vector<Uint32> > planAtlas(int width, int height);
    
    /**
     * Takes the rectangular plan and arranges the glyphs in the SDL surface.
     *
     * @param rectangle A plan for the atlas as a rectangular array of characters.
     */
    void layoutAtlas(const std::vector< std::vector<Uint32> >& rectangle);
    
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
    bool generateSurface(int width, int height);
    
    /**
     * Allocates a blank surface of the given size.
     *
     * This method is necessary because SDL surface allocation is quite
     * involved when you want proper alpha support.
     *
     * @return a blank surface of the given size.
     */
    SDL_Surface* allocSurface(int width, int height);
};
    
#pragma mark -
#pragma mark Style Bit-Wise Operators
    
/**
 * Returns the int equivalent of a font style.
 *
 * @return the int equivalent of a font style.
 */
inline int operator*(Font::Style value) {
    return static_cast<int>(value);
}

/**
 * Returns the bitwise or of two font styles.
 *
 * @return the bitwise or of two font styles.
 */
inline Font::Style operator|(Font::Style lhs, Font::Style rhs) {
    return static_cast<Font::Style>((*lhs) | (*rhs));
}
    
/**
 * Returns the bitwise and of two font styles.
 *
 * @return the bitwise and of two font styles.
 */
inline Font::Style operator&(Font::Style lhs, Font::Style rhs) {
    return static_cast<Font::Style>((*lhs) & (*rhs));
}

/**
 * Returns the bitwise exclusive or of two font styles.
 *
 * @return the bitwise exclusive or of two font styles.
 */

inline Font::Style operator^(Font::Style lhs, Font::Style rhs) {
    return static_cast<Font::Style>((*lhs) ^ (*rhs));
}

/**
 * Returns the bitwise complement of a font style.
 *
 * @return the bitwise complement of a font style.
 */
inline Font::Style operator~(Font::Style lhs) {
    return static_cast<Font::Style>(~(*lhs));
}


}

#endif /* __CU_FONT_H__ */
