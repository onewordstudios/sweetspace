//
//  CUProgressBar.h
//  Cornell University Game Library (CUGL)
//
//  This module provides support for a simple progress bar, which is useful
//  for displaying things such as asset loading.
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
//  Version: 1/8/17
//
#ifndef __CU_PROGRESS_BAR_H__
#define __CU_PROGRESS_BAR_H__
#include <cugl/2d/CUNode.h>
#include <cugl/2d/CUPolygonNode.h>

namespace cugl {

/**
 * This class is a node the represents an (animating) project bar.
 *
 * The progress bar may either be represented via a texture or a simple colored 
 * rectangle. If it is a texture, the foreground texture will be sampled left
 * to right and the maximum horizontal texture coordinate will be the percentage
 * of the progress bar.  So if the progress bar is at 50%, the progress bar will
 * draw the left side of the foreground texture.
 *
 * When using textures it is also possible to specify endcap textures.  This 
 * allows for progress bars that are not completely rectangular.
 */
class ProgressBar: public Node {
protected:
    /** The progress percentage of this progress bar (between 0 and 1) */
    float _progress;
    /** The (maximum) size of the foreground texture or node */
    Size _foresize;

    /** The background image or rectangle (representing total time) */
    std::shared_ptr<PolygonNode> _background;
    /** The foreground image or rectangle (representing progress) */
    std::shared_ptr<PolygonNode> _foreground;
    /** The starting endcap image */
    std::shared_ptr<PolygonNode> _begincap;
    /** The finishing endcap image */
    std::shared_ptr<PolygonNode> _finalcap;
 
public:
#pragma mark -
#pragma mark Constructors
    /**
     * Creates an uninitialized progress bar with no size or texture information.
     *
     * You must initialize this progress bar before use.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a Node on the
     * heap, use one of the static constructors instead.
     */
    ProgressBar() : _progress(1.0f) {}
    
    /**
     * Deletes this progress bar, disposing all resources
     */
    ~ProgressBar() { dispose(); }
    
    /**
     * Disposes all of the resources used by this node.
     *
     * A disposed progress bar can be safely reinitialized. Any children owned 
     * by this node will be released.  They will be deleted if no other object 
     * owns them.
     *
     * It is unsafe to call this on a progress bar that is still currently 
     * inside of a scene graph.
     */
    virtual void dispose() override;
    
    /**
     * Deactivates the default initializer.
     *
     * This initializer may not be used for a progress bar.  A progress bar
     * either needs a texture or a size
     *
     * @return false
     */
    virtual bool init() override {
        CUAssertLog(false,"This node does not support the empty initializer");
        return false;
    }

    /**
     * Initializes a texture-less progress bar of the given size.
     *
     * The background will be a white rectangle, while the foreground (measuring
     * progess) will be a red rectangle. You can change these colors with the
     * {@link setBackgroundColor} and {@link setForegroundColor} methods.
     *
     * @param size  The progress bar size
     *
     * @return true if the progress bar is initialized properly, false otherwise.
     */
    bool init(const Size& size) {
        return initWithCaps(nullptr,nullptr,nullptr,nullptr,size);
    }

    /**
     * Initializes a progress bar with the given texture.
     *
     * The progress bar will be the size of the texture.  It will use the same
     * texture for the background and foreground.  However, the foreground
     * (measuring progress) will be tinted red.  You can change this color with
     * the {@link setForegroundColor} method.
     *
     * @param background    The progress bar texture
     *
     * @return true if the progress bar is initialized properly, false otherwise.
     */
    bool init(const std::shared_ptr<Texture>& background) {
        return initWithCaps(background,nullptr,nullptr,nullptr);
    }
    
    /**
     * Initializes a progress bar with the given texture and size
     *
     * The progress bar texture will scale to the given size.  It will use the
     * same texture for the background and foreground.  However, the foreground
     * (measuring progress) will be tinted red.  You can change this color with
     * the {@link setForegroundColor} method.
     *
     * @param background    The progress bar texture
     * @param size          The progress bar size
     *
     * @return true if the progress bar is initialized properly, false otherwise.
     */
    bool init(const std::shared_ptr<Texture>& background, const Size& size) {
        return initWithCaps(background,nullptr,nullptr,nullptr,size);
    }

    /**
     * Initializes a progress bar with the given background and foreground.
     *
     * The progress bar will be the size of the background texture.  The 
     * foreground texture will be scaled to this size.  Neither the background
     * nor the foreground texture will be tinted.
     *
     * @param background    The texture for the background
     * @param foreground    The texture for the animated foreground     
     *
     * @return true if the progress bar is initialized properly, false otherwise.
     */
    bool init(const std::shared_ptr<Texture>& background,
              const std::shared_ptr<Texture>& foreground) {
        return initWithCaps(background,foreground,nullptr,nullptr);
    }
    
    /**
     * Initializes a progress bar with the given textures and size.
     *
     * The progress bar will scale both the background and foreground texture. 
     * to the given size. Neither the background nor the foreground texture 
     * will be tinted.
     *
     * @param background    The texture for the background
     * @param foreground    The texture for the animated foreground
     * @param size          The progress bar size
     *
     * @return true if the progress bar is initialized properly, false otherwise.
     */
    bool init(const std::shared_ptr<Texture>& background,
              const std::shared_ptr<Texture>& foreground,
              const Size& size) {
        return initWithCaps(background,foreground,nullptr,nullptr,size);
    }

    /**
     * Initializes a progress bar with the given textures and size.
     *
     * The progress bar will be the size of the background texture.  The
     * foreground texture and end caps will be scaled so that they are this 
     * size when combined together.  None of the textures will be tinted.
     *
     * @param background    The texture for the background
     * @param foreground    The texture for the animated foreground
     * @param beginCap      The left end cap of the foreground
     * @param finalCap      The right end cap of the foreground
     *
     * @return true if the progress bar is initialized properly, false otherwise.
     */
    bool initWithCaps(const std::shared_ptr<Texture>& background,
                      const std::shared_ptr<Texture>& foreground,
                      const std::shared_ptr<Texture>& beginCap,
                      const std::shared_ptr<Texture>& finalCap);

    /**
     * Initializes a progress bar with the given textures and size.
     *
     * The progress bar will scale the background texture to the given size.  
     * The foreground texture and end caps will be scaled so that they are this
     * size when combined together.  None of the textures will be tinted.
     *
     * @param background    The texture for the background
     * @param foreground    The texture for the animated foreground
     * @param beginCap      The left end cap of the foreground
     * @param finalCap      The right end cap of the foreground
     * @param size          The progress bar size
     *
     * @return true if the progress bar is initialized properly, false otherwise.
     */
    bool initWithCaps(const std::shared_ptr<Texture>& background,
                      const std::shared_ptr<Texture>& foreground,
                      const std::shared_ptr<Texture>& beginCap,
                      const std::shared_ptr<Texture>& finalCap,
                      const Size& size);
    
    /**
     * Initializes a node with the given JSON specificaton.
     *
     * This initializer is designed to receive the "data" object from the
     * JSON passed to {@link SceneLoader}.  This JSON format supports all
     * of the attribute values of its parent class.  In addition, it supports
     * the following additional attributes:
     *
     *      "foreground":   The name of a previously loaded texture asset
     *      "background":   The name of a previously loaded texture asset
     *      "left_cap":     The name of a previously loaded texture asset
     *      "right_cap":    The name of a previously loaded texture asset
     *
     * All attributes are optional.  There are no required attributes.
     *
     * @param loader    The scene loader passing this JSON file
     * @param data      The JSON object specifying the node
     *
     * @return true if initialization was successful.
     */
    bool initWithData(const SceneLoader* loader, const std::shared_ptr<JsonValue>& data) override;

#pragma mark -
#pragma mark Static Constructors
    /**
     * Returns a newly allocated texture-less progress bar of the given size.
     *
     * The background will be a white rectangle, while the foreground (measuring
     * progess) will be a red rectangle. You can change these colors with the
     * {@link setBackgroundColor} and {@link setForegroundColor} methods.
     *
     * @param size  The progress bar size
     *
     * @return a newly allocated texture-less progress bar of the given size.
     */
    static std::shared_ptr<ProgressBar> alloc(const Size& size) {
        std::shared_ptr<ProgressBar> node = std::make_shared<ProgressBar>();
        return (node->init(size) ? node : nullptr);
    }

    /**
     * Returns a newly allocated progress bar with the given texture.
     *
     * The progress bar will be the size of the texture.  It will use the same
     * texture for the background and foreground.  However, the foreground
     * (measuring progress) will be tinted red.  You can change this color with
     * the {@link setForegroundColor} method.
     *
     * @param background    The progress bar texture
     *
     * @return a newly allocated progress bar with the given texture.
     */
    static std::shared_ptr<ProgressBar> alloc(const std::shared_ptr<Texture>& background) {
        std::shared_ptr<ProgressBar> node = std::make_shared<ProgressBar>();
        return (node->init(background) ? node : nullptr);
    }

    /**
     * Returns a newly allocated progress bar with the given texture and size
     *
     * The progress bar texture will scale to the given size.  It will use the
     * same texture for the background and foreground.  However, the foreground
     * (measuring progress) will be tinted red.  You can change this color with
     * the {@link setForegroundColor} method.
     *
     * @param background    The progress bar texture
     * @param size          The progress bar size
     *
     * @return a newly allocated progress bar with the given texture and size
     */
    static std::shared_ptr<ProgressBar> alloc(const std::shared_ptr<Texture>& background, const Size& size) {
        std::shared_ptr<ProgressBar> node = std::make_shared<ProgressBar>();
        return (node->init(background,size) ? node : nullptr);
    }

    /**
     * Returns a newly allocated progress bar with the given background and foreground.
     *
     * The progress bar will be the size of the background texture.  The
     * foreground texture will be scaled to this size.  Neither the background
     * nor the foreground texture will be tinted.
     *
     * @param background    The texture for the background
     * @param foreground    The texture for the animated foreground
     *
     * @return a newly allocated progress bar with the given background and foreground.
     */
    static std::shared_ptr<ProgressBar> alloc(const std::shared_ptr<Texture>& background,
                                              const std::shared_ptr<Texture>& foreground) {
        std::shared_ptr<ProgressBar> node = std::make_shared<ProgressBar>();
        return (node->init(background,foreground) ? node : nullptr);
    }
    
    /**
     * Returns a newly allocated progress bar with the given textures and size.
     *
     * The progress bar will scale both the background and foreground texture.
     * to the given size. Neither the background nor the foreground texture
     * will be tinted.
     *
     * @param background    The texture for the background
     * @param foreground    The texture for the animated foreground
     * @param size          The progress bar size
     *
     * @return a newly allocated progress bar with the given textures and size.
     */
    static std::shared_ptr<ProgressBar> alloc(const std::shared_ptr<Texture>& background,
                                              const std::shared_ptr<Texture>& foreground,
                                              const Size& size) {
        std::shared_ptr<ProgressBar> node = std::make_shared<ProgressBar>();
        return (node->init(background,foreground,size) ? node : nullptr);
    }

    /**
     * Returns a newly allocated progress bar with the given textures and size.
     *
     * The progress bar will be the size of the background texture.  The
     * foreground texture and end caps will be scaled so that they are this
     * size when combined together.  None of the textures will be tinted.
     *
     * @param background    The texture for the background
     * @param foreground    The texture for the animated foreground
     * @param beginCap      The left end cap of the foreground
     * @param finalCap      The right end cap of the foreground
     *
     * @return a newly allocated progress bar with the given textures and size.
     */
    static std::shared_ptr<ProgressBar> allocWithCaps(const std::shared_ptr<Texture>& background,
                                                      const std::shared_ptr<Texture>& foreground,
                                                      const std::shared_ptr<Texture>& beginCap,
                                                      const std::shared_ptr<Texture>& finalCap) {
        std::shared_ptr<ProgressBar> node = std::make_shared<ProgressBar>();
        return (node->initWithCaps(background,foreground,beginCap,finalCap) ? node : nullptr);
    }
    
    /**
     * Returns a newly allocated progress bar with the given textures and size.
     *
     * The progress bar will scale the background texture to the given size.
     * The foreground texture and end caps will be scaled so that they are this
     * size when combined together.  None of the textures will be tinted.
     *
     * @param background    The texture for the background
     * @param foreground    The texture for the animated foreground
     * @param beginCap      The left end cap of the foreground
     * @param finalCap      The right end cap of the foreground
     * @param size          The progress bar size
     *
     * @return a newly allocated progress bar with the given textures and size.
     */
    static std::shared_ptr<ProgressBar> allocWithCaps(const std::shared_ptr<Texture>& background,
                                                      const std::shared_ptr<Texture>& foreground,
                                                      const std::shared_ptr<Texture>& beginCap,
                                                      const std::shared_ptr<Texture>& finalCap,
                                                      const Size& size) {
        std::shared_ptr<ProgressBar> node = std::make_shared<ProgressBar>();
        return (node->initWithCaps(background,foreground,beginCap,finalCap,size) ? node : nullptr);
    }
    
    /**
     * Returns a newly allocated node with the given JSON specificaton.
     *
     * This initializer is designed to receive the "data" object from the
     * JSON passed to {@link SceneLoader}.  This JSON format supports all
     * of the attribute values of its parent class.  In addition, it supports
     * the following additional attributes:
     *
     *      "foreground":   The name of a previously loaded texture asset
     *      "background":   The name of a previously loaded texture asset
     *      "left_cap":     The name of a previously loaded texture asset
     *      "right_cap":    The name of a previously loaded texture asset
     *
     * All attributes are optional.  There are no required attributes.
     *
     * @param loader    The scene loader passing this JSON file
     * @param data      The JSON object specifying the node
     *
     * @return a newly allocated node with the given JSON specificaton.
     */
    static std::shared_ptr<Node> allocWithData(const SceneLoader* loader,
                                               const std::shared_ptr<JsonValue>& data) {
        std::shared_ptr<ProgressBar> result = std::make_shared<ProgressBar>();
        if (!result->initWithData(loader,data)) { result = nullptr; }
        return std::dynamic_pointer_cast<Node>(result);
    }
    
#pragma mark -
#pragma mark Properties
    /**
     * Returns the percentage progress of this progress bar
     *
     * This value is a float between 0 and 1. Changing this value will alter
     * the size of the progress bar foreground.
     *
     * @return the percentage progress of this progress bar
     */
    float getProgress() const { return _progress; }

    /**
     * Sets the percentage progress of this progress bar
     *
     * This value is a float between 0 and 1. Changing this value will alter
     * the size of the progress bar foreground.
     *
     * @param progress  The percentage progress of this progress bar
     */
    void setProgress(float progress);
    
    /**
     * Returns the background color or tint of the progress bar
     *
     * This is the color applied to the background texture if it exists, or
     * the color of the background rectangle. It is white by default.
     *
     * @return the background color or tint of the progress bar
     */
    Color4 getBackgroundColor() const { return _background->getColor(); }
    
    /**
     * Sets the background color or tint of the progress bar
     *
     * This is the color applied to the background texture if it exists, or
     * the color of the background rectangle. It is white by default.
     *
     * @param color The background color or tint of the progress bar
     */
    void setBackgroundColor(Color4 color) { _background->setColor(color); }

    /**
     * Returns the foreground color or tint of the progress bar
     *
     * This is the color applied to the foreground texture (and end caps) if it 
     * exists, or the color of the foreground rectangle. If there is a texture
     * it is white by default.  Otherwise it is red by default.
     *
     * @return the foreground color or tint of the progress bar
     */
    Color4 getForegroundColor() const { return _foreground->getColor(); }
    
    /**
     * Sets the foreground color or tint of the progress bar
     *
     * This is the color applied to the foreground texture (and end caps) if it
     * exists, or the color of the foreground rectangle. If there is a texture
     * it is white by default.  Otherwise it is red by default.
     *
     * @param color The foreground color or tint of the progress bar
     */
    void setForegroundColor(Color4 color);
};

}

#endif /* __CU_PROGRESS_BAR_H__ */
