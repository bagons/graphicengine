#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "gereferences.hpp"
#include "coordinates.h"

class Camera;

/// A base RenderPass method for polymorphism
/// It's like this so that the Engine can hold these objects safely and call change_resolution() update methods based on what happens to the Window.
class RenderPass {
public:
    virtual void change_resolution(const int _width, const int _height){};

    virtual ~RenderPass() = default;
};

/// Renders a basic color over the screen.
class ColorPass : public RenderPass {
public:
    Color color;
    /// Constructor for ColorPass
    explicit ColorPass(Color color);
    /// Fills the screen with the set color
    void render();
};

/// Standard forward opaque renderer
/// Minimizes shader switching and uniform calls
class ForwardOpaque3DPass : public RenderPass {
public:
    /// Holds a reference to the camera from which the 3D scene is rendered. Can be changed before calling render, but usually you don't switch cameras often so, it saves the one you are using
    geRef<Camera> camera;
    /// A bit mask that shows which entities will be rendered by this pass. Useful for view model for FPS gun, or for 3D UI.
    unsigned int render_layer;

    /// Construct the Pass Object, parameters are updatable
    /// @param camera the camera from which the scene is rendered
    /// @param render_layer the render layer mask that defines which things can be rendered by this pass
    ForwardOpaque3DPass(geRef<Camera> camera, unsigned int render_layer = 1);

    /// Main 3D render function. Forward renderer, only opaque unless discard called
    void render();
    /// Changes the Camera matrix based on resolution change.
    /// @note If you switch cameras Camera matrix might not be updated properly, because it was not attached when the resolution changed.
    void change_resolution(int width, int height) override;
};

#endif //RENDERER_HPP
