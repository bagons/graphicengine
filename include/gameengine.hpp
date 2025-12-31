#ifndef MAIN_H
#define MAIN_H
#include <map>
#include <unordered_map>
#include <memory>
#include <iostream>

#pragma once
#include "glad/glad.h"

#include "input.hpp"
#include "meshes.hpp"
#include "things.hpp"
#include "shaders.hpp"
#include "textures.hpp"
#include "renderer.hpp"
#include "gereferences.hpp"

#include <GLFW/glfw3.h>

typedef std::unordered_map<int, std::unique_ptr<Thing>> things_container;
typedef std::unordered_map<int, std::unique_ptr<class RenderLayer>> render_layer_container;


class Window {
public:
    GLFWwindow *glfwwindow;
    int width{}, height{};
    Window();
    Window(int _width, int _height, const char* title);
    void select() const;
};


class Engine {
    int next_render_layer_id = 0;
    double last_game_time = 0.0;
public:
    Window window{};
    Input input{};
    Meshes meshes{};
    Textures textures{};
    Shaders shaders{};

    int next_thing_id = 0;
    unsigned int camera_matrix_ubo;
    float frame_delta = 0.0f;
    things_container things{};
    render_layer_container render_layers{};
    std::multimap<std::shared_ptr<Material>, int, MaterialSorter> thing_ids_by_shader_program;

    void update() const;

    void init_render_pipeline();

    void send_it_to_window();

    bool is_running() const;

    Thing* get_thing(int id);

    RenderLayer* get_render_layer(int id);

    template<typename T, typename... Args>
    requires std::is_base_of_v<Thing, T>
    geRef<T> add(Args&&... args) {
        geRef<T> ref{next_thing_id, this};
        next_thing_id++;

        auto thing = std::make_unique<T>(std::forward<Args>(args)...);

        if constexpr (std::is_base_of_v<MeshThing, T>) {
            thing_ids_by_shader_program.insert({thing.get()->material, ref.id});
        }

        things[ref.id] = std::move(thing);

        return ref;
    };

    template<typename T, typename... Args>
    requires std::is_base_of_v<RenderLayer, T>
    geRendRef<T> add_render_layer(Args&&... args) {
        geRendRef<T> ref(next_render_layer_id, this);

        auto thing = std::make_unique<T>(std::forward<Args>(args)...);

        render_layers[next_render_layer_id] = std::move(thing);

        next_render_layer_id++;
        return ref;
    };
};
Engine* gameengine(const char* game_name);
extern Engine ge;

int noengine();

#endif //MAIN_H
