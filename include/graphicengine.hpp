#ifndef MAIN_H
#define MAIN_H
#include <map>
#include <unordered_map>
#include <memory>

#pragma once
#include <glad/glad.h>

#include "gereferences.hpp"
#include "input.hpp"
#include "meshes.hpp"
#include "textures.hpp"
#include "shaders.hpp"
#include "things.hpp"
#include "renderer.hpp"
#include "lights.hpp"

#include <GLFW/glfw3.h>

typedef std::unordered_map<int, std::unique_ptr<Thing>> things_container;
typedef std::unordered_map<int, std::unique_ptr<RenderLayer>> render_layer_container;


class Window {
public:
    GLFWwindow *glfwwindow;
    int width, height;
    Window(const char* title, int _width, int _height);
    void select() const;
    void set_vsync(bool _vsync) const;
    ~Window();
};


class Engine {
    int next_render_layer_id = 0;
    double last_game_time = 0.0;
    bool bindless_texture_supported = false;
public:
    Window window;
    Input input{};
    Meshes meshes{};
    Shaders shaders{};
    Lights lights;

    int next_thing_id = 0;
    unsigned int camera_matrix_ubo = -1;
    float frame_delta = 0.0f;
    things_container things{};
    render_layer_container render_layers{};
    std::multimap<std::shared_ptr<Material>, int, MaterialSorter> thing_ids_by_shader_program;

    Engine(const char *display_name, int window_width, int window_height, unsigned int MAX_NR_POINT_LIGHTS = 16, unsigned int MAX_NR_DIRECTIONAL_LIGHTS = 3,  Lights::LightOverflowAction light_overflow_action = Lights::SORT_BY_PROXIMITY);
    ~Engine();

    void update();

    void init_render_pipeline();

    void send_it_to_window();

    [[nodiscard]] bool is_running() const;

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
        } else if constexpr (std::is_base_of_v<PointLight, T>) {
            if (!lights.add_point_light(ref.id)) {
                ref.id = -1;
                ref.ge = nullptr;
                return ref;
            }
        } else if constexpr (std::is_base_of_v<DirectionalLight, T>) {
            if (!lights.add_directional_light(ref.id)) {
                ref.id = -1;
                ref.ge = nullptr;
                return ref;
            }
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

    void set_bindless_texture_support(bool support_bindless_textures);
    [[nodiscard]] bool are_bindless_textures_supported() const;
};

extern Engine ge;

#endif //MAIN_H
