#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "gereferences.hpp"

class Camera;

class RenderPass {
public:
    virtual void change_resolution(const int _width, const int _height){};

    virtual ~RenderPass() = default;
};


class ForwardOpaque3DPass : public RenderPass {
public:
    geRef<Camera> camera;
    unsigned int render_layer;

    explicit ForwardOpaque3DPass(geRef<Camera> camera, unsigned int _render_layer = 1);

    void render();
    void change_resolution(int width, int height) override;
};

#endif //RENDERER_HPP
