#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "gereferences.hpp"

class Camera;

class RenderLayer {
public:
    virtual void render() {};
    virtual void change_resolution(int width, int height){};

    virtual ~RenderLayer() = default;
};


class ForwardRenderer3DLayer : public RenderLayer {
public:
    geRef<Camera> cam;
    ForwardRenderer3DLayer(geRef<Camera> camera);

    void render() override;
    void change_resolution(int width, int height) override;
};

#endif //RENDERER_HPP
