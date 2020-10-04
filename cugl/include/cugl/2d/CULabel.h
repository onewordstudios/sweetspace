//
//  CULabel.h
//  Cornell University Game Library (CUGL)
//
//  This module provides a scene graph node that displays a single line of text.
//  We have moved multiline text support to a separate class.
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
//
#ifndef __CU_LABEL_H__
#define __CU_LABEL_H__

#include <string>
#include <cugl/2d/CUNode.h>
#include <cugl/2d/CUFont.h>

namespace cugl {

/**
 * This class is a node the represents a single line of text.
 *
 * This class only represents a single line of text.  To have a node that 
 * represents multiple lines with spacing and justitication, you should use
 * the (as yet unimplemented) MultilineLabel class.
 *
 * By default, the content size is just large enough to render the text given.
 * If you reset the content size to larger than the what the text needs, the
 * the text is placed in the label according to the text bounds.  If you reset
 * it to smaller, the text may be cut off in rendering.
 *
 * If the background color is not clear, then the label will have a colored
 * backing rectangle.  The rectangle will extended from the origin to the
 * content size in Node space.
 *
 * To display the text, you need a {@link Font}.  The rendering style of the
 * font will depend on whether or not it has an atlas.  We highly recommend
 * an atlas if you have a lot of labels using the same font or if the text
 * changes rapidly.  In our experience, generating a simple "Hello World!"
 * label is 1-2 orders of magnitude faster with an atlas.  The only time to
 * use a font without an atlas is if you have relatively few labels and they
 * never change at all.
 */
class Label : public Node {
#pragma mark Values
public:
    /**
     * This enumeration represents the horizontal alignment of text in a {@link Label}.
     *
     * Horizontal alignment can be interpretted in two ways.  First, it can
     * be the relationship between the text and it surrounding bounding box.
     * Alternatively, it can mean justification, which is the relationship of
     * multiple lines of text.  The latter is reserved for the (as yet unimplemented)
     * MultilineLabel class.
     */
    enum class HAlign : int {
        /**
         * Anchors the text on the left side of the label
         *
         * If the initial character has any natural spacing on the left-hand side
         * (e.g the character width is less than the advance), this will be included.
         */
        LEFT = 0,
        /**
         * Anchors the text in the center horiontal position of the label.
         *
         * Centering will include any natural spacing (e.g the character width is
         * less than the advance) around the first and last character.
         */
        CENTER = 1,
        /**
         * Anchors the text on the right side of the label
         *
         * If the final character has any natural spacing on the right-hand side
         * (e.g the character width is less than the advance), this will be included.
         */
        RIGHT = 2,
        /**
         * Anchors the text on the left side of the label
         *
         * Any natural spacing on the left of the first character will be ignored.
         * Hence the character will begin drawing at the very edge of the label.
         */
        HARDLEFT = 3,
        /**
         * Anchors the text in the center horiontal position of the label.
         *
         * Centering will ignore any natural spacing around the first and
         * last character. Hence the text will be measure from the left edge of
         * the first character to the right edge of the second.
         */
        TRUECENTER = 4,
        /**
         * Anchors the text on the right side of the label.
         *
         * Any natural spacing on the right of the last character will be ignored.
         * Hence the character will stop drawing at the very edge of the label.
         */
        HARDRIGHT = 5
    };
    
    /**
     * This enumeration represents the vertical alignment of text in a {@link Label}.
     *
     * Vertical alignment is always interprettes as the relationship between the
     * text and it surrounding bounding box.
     */
    enum class VAlign {
        /**
         * Anchors the text at the bottom of the label
         *
         * This option uses the font descent in placing the characters.  Hence
         * if no character is below the baseline, this descent will be included
         * as spacing at the bottom.
         */
        BOTTOM = 0,
        /**
         * Anchors the text at the middle vertical position of the label
         *
         * This option uses the font height in placing the characters.  Hence if
         * no character is below the baseline, this descent will be included as
         * spacing at the bottom. In addition, if no character is reaches the
         * maximum above baseline, the ascent will be included as spacing at the top.
         */
        MIDDLE = 1,
        /**
         * Anchors the text at the top of the label
         *
         * This option uses the font ascent in placing the characters.  Hence
         * if no character is reaches the maximum above baseline, this ascent will
         * be included as spacing at the top.
         */
        TOP = 2,
        /**
         * Anchors the text at the bottom of the label
         *
         * This option ignores the font descent in placing the characters.  It
         * places the characters so that the one with the greatest extent below
         * the base line is at the very bottom edge of the label.
         */
        HARDBOTTOM= 3,
        /**
         * Anchors the text at the middle vertical position of the label
         *
         * This option ignores the font height in placing the characters. It
         * measure the characters from the top of the highest character to the
         * bottom of the character with greatest extend below the baseline.
         */
        TRUEMIDDLE= 4,
        /**
         * Anchors the text at the top of the label
         *
         * This option ignores the font ascent in placing the characters.    It
         * places the characters so that the one with the greatest extent above
         * the baseline is at the very top edge of the label.
         */
        HARDTOP = 5,
    };
protected:
    /** The font (with or without an atlas) */
    std::shared_ptr<Font> _font;
    
    /** The label text */
    std::string  _text;
    /** The bounds of this rendered text */
    RectCugl _textbounds;
    /** The true bounds of this rendered text, ignoring any natural spacing */
    RectCugl _truebounds;
    
    /** The padding offset */
    Vec2 _padding;
    /** The horizontal alignment of the text in this label */
    HAlign _halign;
    /** The vertical alignment of the text in this label */
    VAlign _valign;
    
    /** The color of the text (default is BLACK) */
    Color4 _foreground;
    /** The color of the background panel (default is CLEAR) */
    Color4 _background;
    
    /** The blending equation for this texture */
    GLenum _blendEquation;
    /** The source factor for the blend function */
    GLenum _srcFactor;
    /** The destination factor for the blend function */
    GLenum _dstFactor;

    /** Whether or not the glyphs have been rendered */
    bool _rendered;
    /** The glyph vertices */
    std::vector<Vertex2> _vertices;
    /** The quad indices for the vertices */
    std::vector<unsigned short> _indices;
    /** The underlying atlas texture */
    std::shared_ptr<Texture> _texture;

public:
#pragma mark -
#pragma mark Constructors
    /**
     * Creates an uninitialized label with no text or font information.
     *
     * You must initialize this Label before use.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a Node on the
     * heap, use one of the static constructors instead.
     */
    Label();
    
    /**
     * Deletes this label, disposing all resources
     */
    ~Label() { dispose(); }
    
    /**
     * Disposes all of the resources used by this label.
     *
     * A disposed Label can be safely reinitialized. Any children owned by this
     * node will be released.  They will be deleted if no other object owns them.
     *
     * It is unsafe to call this on a Label that is still currently inside of
     * a scene graph.
     */
    virtual void dispose() override;
    
    /**
     * Deactivates the default initializer.
     *
     * This initializer may not be used for a label. A label must have a font.
     *
     * @return false
     */
    virtual bool init() override {
        CUAssertLog(false,"This node does not support the empty initializer");
        return false;
    }
    
    /**
     * Initializes a label with the given size and font
     *
     * The text is empty and may be set later with {@link setText()}.
     *
     * @param size  The size of the label to display
     * @param font  The font for this label
     *
     * @return true if initialization was successful.
     */
    bool init(const Size& size, const std::shared_ptr<Font>& font);
    
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
    bool initWithText(const char* text, const std::shared_ptr<Font>& font) {
        return initWithText(std::string(text), font);
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
    bool initWithText(const std::string& text, const std::shared_ptr<Font>& font);

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
    virtual bool initWithData(const SceneLoader* loader, const std::shared_ptr<JsonValue>& data) override;

#pragma mark -
#pragma mark Static Constructors
    /**
     * Returns a newly allocated label with the given size and font
     *
     * The text is empty and may be set later with {@link setText()}.
     *
     * @param size  The size of the label to display
     * @param font  The font for this label
     *
     * @return a newly allocated label with the given size and font atlas
     */
    static std::shared_ptr<Label> alloc(const Size& size, const std::shared_ptr<Font>& font) {
        std::shared_ptr<Label> result = std::make_shared<Label>();
        return (result->init(size,font) ? result : nullptr);
    }

    /**
     * Returns a newly allocated label with the given text and font
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
     * @return a newly allocated label with the given text and font atlas
     */
    static std::shared_ptr<Label> alloc(const std::string& text, const std::shared_ptr<Font>& font) {
        std::shared_ptr<Label> result = std::make_shared<Label>();
        return (result->initWithText(text,font) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated label with the given text and font
     *
     * The text string will be purged of all non-printable characters.  If any
     * glyphs are missing from the font atlas, they will not be rendered.
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
     * @return a newly allocated label with the given text and font atlas
     */
    static std::shared_ptr<Label> alloc(const char* text, const std::shared_ptr<Font>& font) {
        std::shared_ptr<Label> result = std::make_shared<Label>();
        return (result->initWithText(text,font) ? result : nullptr);
    }

    /**
     * Returns a newly allocated node with the given JSON specificaton.
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
     * @return a newly allocated node with the given JSON specificaton.
     */
    static std::shared_ptr<Node> allocWithData(const SceneLoader* loader,
                                               const std::shared_ptr<JsonValue>& data) {
        std::shared_ptr<Label> result = std::make_shared<Label>();
        if (!result->initWithData(loader,data)) { result = nullptr; }
        return std::dynamic_pointer_cast<Node>(result);
    }
    
#pragma mark -
#pragma mark Text Attributes
    /**
     * Returns the text for this label.
     *
     * The string will be in either ASCII or UTF8 format.  As all ASCII strings
     * are also UTF8, you can assume that the text is UTF8.
     *
     * If the font is missing glyphs in this string, the characters in the text 
     * may be different than those displayed.
     *
     * @return the text for this label.
     */
    const std::string& getText() const { return _text; }
    
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
     * @param text      The text for this label.
     * @param resize    Whether to resize the label to fit the new text.
     */
    virtual void setText(const std::string& text, bool resize=false);
    
    /**
     * Returns the padding of the rendered text.
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
     * @return the padding of the rendered text.
     */
    const Vec2& getPadding() const { return _padding; }

    /**
     * Returns the x-axis padding of the rendered text.
     *
     * The application of padding depends on how the text is aligned in the
     * label.  For example, if the text has alignment LEFT and, the padding 
     * will shift the text right in the label.  On the other hand, if it has 
     * alignment RIGHT, then it will shift the text left.
     *
     * If the alignment is CENTER or TRUECENTER, then the padding will be 
     * ignored.  
     *
     * @return the x-axis padding of the rendered text.
     */
    float getPaddingX() const { return _padding.x; }
    
    /**
     * Returns the y-axis padding of the rendered text.
     *
     * The application of padding depends on how the text is aligned in the
     * label.  For example, if the text has BOTTOM, the padding will shift the 
     * text up in the label.  On the other hand, if it has alignment TOP,
     * then it will shift the text down.
     *
     * If the alignment is MIDDLE or TRUEMIDDLE, then the padding will be
     * ignored.
     *
     * @return the y-axis padding of the rendered text.
     */
    float getPaddingY() const { return _padding.y; }
    
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
     * @param padding   The padding of the rendered text.
     */
    void setPadding(const Vec2& padding) {
        setPadding(padding.x,padding.y);
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
    void setPadding(float padx, float pady);
    
    /**
     * Sets the x-axis padding of the rendered text.
     *
     * The application of padding depends on how the text is aligned in the
     * label.  For example, if the text has alignment LEFT and, the padding
     * will shift the text right in the label.  On the other hand, if it has
     * alignment RIGHT, then it will shift the text left.
     *
     * If the alignment is CENTER or TRUECENTER, then the padding will be
     * ignored.
     *
     * @param padx      The x-axis padding of the rendered text.
     */
    void setPaddingX(float padx) {
        setPadding(padx,_padding.y);
    }

    /**
     * Sets the y-axis padding of the rendered text.
     *
     * The application of padding depends on how the text is aligned in the
     * label.  For example, if the text has BOTTOM, the padding will shift the
     * text up in the label.  On the other hand, if it has alignment TOP,
     * then it will shift the text down.
     *
     * If the alignment is MIDDLE or TRUEMIDDLE, then the padding will be
     * ignored.
     *
     * @param pady      The y-axis padding of the rendered text.
     */
    void setPaddingY(float pady) {
        setPadding(_padding.x,pady);
    }
    
    /**
     * Returns the horizontal alignment of the text.
     * 
     * This value represents the relationship between the text and its
     * label.  It functions as a sort of anchor.
     *
     * @return the horizontal alignment of the text.
     */
    HAlign getHorizontalAlignment() const { return _halign; }

    /**
     * Sets the horizontal alignment of the text.
     *
     * This value represents the relationship between the text and its
     * label.  It functions as a sort of anchor.
     *
     * @param halign    The horizontal alignment of the text.
     */
    void setHorizontalAlignment(HAlign halign);

    /**
     * Returns the horizontal alignment of the text.
     *
     * This value represents the relationship between the text and its
     * label.  It functions as a sort of anchor.
     *
     * @return the horizontal alignment of the text.
     */
    VAlign getVerticalAlignment() const { return _valign; }
    
    /**
     * Sets the horizontal alignment of the text.
     *
     * This value represents the relationship between the text and its
     * label.  It functions as a sort of anchor.
     *
     * @param valign    The horizontal alignment of the text.
     */
    void setVerticalAlignment(VAlign valign);

    /**
     * Returns the bounds of the rendered text.
     * 
     * This is the bounds of the rendered text with respect to the Node space.
     * The size of the bounding box is the minimum size to render the text.
     * That is, the height will be the maximum height of the font, and the width 
     * will be the sum of the advance of the rendered characters.  That means 
     * that there may be some natural spacing around the characters.
     *
     * The origin of the bounds is determined by the padding and the alignment.
     *
     * If this rectangle extends outside the bounding box of the label (e.g. 
     * the rectangle with origin (0,0) and the content size), then only the 
     * part of the rectangle inside the bounding box will be rendered.
     *
     * @return the bounds of the rendered text.
     */
    const RectCugl& getTextBounds() const { return _textbounds; }

    /**
     * Returns the tightest bounds of the rendered text.
     *
     * This is the bounds of the rendered text, with respect to the Node space.
     * The size of the bounding box ignores any nautural spacing around the 
     * characters.  Hence the height is ascent of the tallest character plus
     * the descent of the character with the lowest overhang.  The width is
     * that of the characters, ignoring the advance.
     *
     * The origin of the bounds is determined by the padding and the alignment.
     *
     * If this rectangle extends outside the bounding box of the label (e.g.
     * the rectangle with origin (0,0) and the content size), then only the
     * part of the rectangle inside the bounding box will be rendered.
     *
     * @return the bounds of the rendered text.
     */
    const RectCugl getTrueBounds() const  {
        return RectCugl(_textbounds.origin+_truebounds.origin,_truebounds.size);
    }
    
    /**
     * Returns the position of the baseline with respect to the Node origin
     *
     * The baseline does not necessarily align with the bottom of the text
     * bounds, as letters may overhang.
     *
     * @return the position of the baseline with respect to the Node origin
     */
    float getBaseLine() const;

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
    virtual void setContentSize(const Size& size) override;
    
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
     * @param width     The untransformed width of the node.
     * @param height    The untransformed height of the node.
     */
    virtual void setContentSize(float width, float height) override {
        setContentSize(Size(width, height));
    }
    
#pragma mark -
#pragma mark Other Attributes
    /**
     * Returns the foreground color of this label.
     * 
     * This color will be applied to the characters themselves. This color is
     * BLACK by default.
     *
     * @return the foreground color of this label.
     */
    Color4 getForeground() const { return _foreground; }

    /**
     * Sets the foreground color of this label.
     *
     * This color will be applied to the characters themselves. This color is
     * BLACK by default.
     *
     * @param color The foreground color of this label.
     */
    void setForeground(Color4 color) { _foreground = color; updateColor(); }

    /**
     * Returns the background color of this label.
     *
     * If this color is not CLEAR (the default color), then the label will have 
     * a colored backing rectangle.  The rectangle will extended from the origin 
     * to the content size in Node space.
     *
     * @return the background color of this label.
     */
    Color4 getBackground() const { return _background; }
    
    /**
     * Sets the background color of this label.
     *
     * If this color is not CLEAR (the default color), then the label will have
     * a colored backing rectangle.  The rectangle will extended from the origin
     * to the content size in Node space.
     *
     * @param color The background color of this label.
     */
    void setBackground(Color4 color);
    
    /**
     * Returns the font to use for this label
     *
     * @return the font to use for this label
     */
    std::shared_ptr<Font> getFont() const { return _font; }
    
    /**
     * Sets the font to use this label
     *
     * Changing this value will regenerate the render data, and is potentially
     * expensive, particularly if the font does not have an atlas.
     *
     * @param font  	The font to use for this label
     * @param resize	Whether to resize this label to fit the new font
     */
    void setFont(const std::shared_ptr<Font>& font, bool resize=false);
    
#pragma mark -
#pragma mark Rendering
    /**
     * Sets the blending function for this texture node.
     *
     * The enums are the standard ones supported by OpenGL.  See
     *
     *      https://www.opengl.org/sdk/docs/man/html/glBlendFunc.xhtml
     *
     * However, this setter does not do any error checking to verify that
     * the enums are valid.  By default, srcFactor is GL_SRC_ALPHA while
     * dstFactor is GL_ONE_MINUS_SRC_ALPHA. This corresponds to non-premultiplied
     * alpha blending.
     *
     * This blending factor only affects the texture of the current node.  It
     * does not affect any children of the node.
     *
     * @param srcFactor Specifies how the source blending factors are computed
     * @param dstFactor Specifies how the destination blending factors are computed.
     */
    void setBlendFunc(GLenum srcFactor, GLenum dstFactor) { _srcFactor = srcFactor; _dstFactor = dstFactor; }
    
    /**
     * Returns the source blending factor
     *
     * By default this value is GL_SRC_ALPHA. For other options, see
     *
     *      https://www.opengl.org/sdk/docs/man/html/glBlendFunc.xhtml
     *
     * This blending factor only affects the texture of the current node.  It
     * does not affect any children of the node.
     *
     * @return the source blending factor
     */
    GLenum getSourceBlendFactor() const { return _srcFactor; }
    
    /**
     * Returns the destination blending factor
     *
     * By default this value is GL_ONE_MINUS_SRC_ALPHA. For other options, see
     *
     *      https://www.opengl.org/sdk/docs/man/html/glBlendFunc.xhtml
     *
     * This blending factor only affects the texture of the current node.  It
     * does not affect any children of the node.
     *
     * @return the destination blending factor
     */
    GLenum getDestinationBlendFactor() const { return _srcFactor; }
    
    /**
     * Sets the blending equation for this textured node
     *
     * The enum must be a standard ones supported by OpenGL.  See
     *
     *      https://www.opengl.org/sdk/docs/man/html/glBlendEquation.xhtml
     *
     * However, this setter does not do any error checking to verify that
     * the input is valid.  By default, the equation is GL_FUNC_ADD.
     *
     * This blending equation only affects the texture of the current node.  It
     * does not affect any children of the node.
     *
     * @param equation  Specifies how source and destination colors are combined
     */
    void setBlendEquation(GLenum equation) { _blendEquation = equation; }
    
    /**
     * Returns the blending equation for this textured node
     *
     * By default this value is GL_FUNC_ADD. For other options, see
     *
     *      https://www.opengl.org/sdk/docs/man/html/glBlendEquation.xhtml
     *
     * This blending equation only affects the texture of the current node.  It
     * does not affect any children of the node.
     *
     * @return the blending equation for this sprite batch
     */
    GLenum getBlendEquation() const { return _blendEquation; }

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
    virtual void draw(const std::shared_ptr<SpriteBatch>& batch, const Mat4& transform, Color4 tint) override;
    
private:
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
    void computeSize();

    /**
     * Allocate the render data necessary to render this node.
     */
    void generateRenderData();
    
    /**
     * Clears the render data, releasing all vertices and indices.
     */
    void clearRenderData();
    
    /**
     * Updates the color value for any other data that needs it.
     *
     * This method is used to synchronize the background color and foreground
     * colors.
     */
    void updateColor();
};
    
}
#endif /* __CU_LABEL_H__ */
