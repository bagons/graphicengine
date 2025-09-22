#ifndef MAIN_H
#define MAIN_H
#include <map>
#include <unordered_map>
#include <memory>

#pragma once
#include "glad/glad.h"
#include <vector>

#include "input.hpp"
#include "meshes.hpp"
#include "things.hpp"
#include "shaders.hpp"
#include "textures.hpp"

#include <GLFW/glfw3.h>

typedef std::unordered_map<int, std::unique_ptr<Thing>> things_container;


class Window {
public:
    GLFWwindow *glfwwindow;
    int width{}, height{};
    Window();
    Window(int _width, int _height, const char* title);
    void select() const;
};

template<typename T>
requires std::is_base_of_v<Thing, T>
class geRef {
public:
    int id;
    class Engine* ge;

    geRef() {
        id = -1;
        ge = nullptr;
    }

    geRef(const int _id, Engine* _ge) {
        id = _id;
        ge = _ge;
    }

    T* get() {
        return static_cast<T*>(ge->get_thing(id));
    };
    T* operator->() {
        return get();
    };
};


class Engine {
    int next_thing_id = 0;
    double last_game_time = 0.0;
public:
    unsigned int camera_matrix_ubo;
    float frame_delta = 0.0f;
    things_container things{};
    std::multimap<Material*, int, MaterialSorter> thing_ids_by_shader_program;
    Window window{};
    Input input{};
    Meshes meshes{};
    Textures textures{};

    void render(int camera_index);

    void update() const;

    void load_base_vertex_shader();

    void init_render_pipeline();

    void render(int camera_index) const;

    void send_it_to_window();

    bool is_running() const;

    Thing* get_thing(int id);

    template<typename T, typename... Args>
    geRef<T> add(Args&&... args) {
        geRef<T> ref{next_thing_id, this};

        auto thing = std::make_unique<T>(std::forward<Args>(args)...);

        if constexpr (std::derived_from<T, MeshThing>) {
            thing_ids_by_shader_program.insert({thing.get()->material, next_thing_id});
        }

        things[next_thing_id] = std::move(thing);

        next_thing_id++;
        return ref;
    };
};
Engine* gameengine(const char* game_name);
extern Engine ge;

int noengine();

#endif //MAIN_H
