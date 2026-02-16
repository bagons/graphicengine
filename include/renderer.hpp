#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "gereferences.hpp"

class Camera;

class RenderPass {
public:
    virtual void change_resolution(int width, int height){};

    virtual ~RenderPass() = default;
};


class ForwardOpaque3DPass : public RenderPass {
public:
    geRef<Camera> camera;
    explicit ForwardOpaque3DPass(geRef<Camera> camera);

    void render();
    void change_resolution(int width, int height) override;
};

#endif //RENDERER_HPP
