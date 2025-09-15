#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "gameengine.hpp"
#include "things.hpp"

class RenderLayer {

};

class ForwardRenderer3DLayer : public RenderLayer {
public:
    geRef<Camera> cam;
    explicit ForwardRenderer3DLayer(geRef<Camera> camera);
    void render();
};

class DifferedRender3DLayer : public RenderLayer {
public:
    void render();
};

#endif //RENDERER_HPP
