#ifndef MAIN_H
#define MAIN_H

#include "input.hpp"


class Window {
public:
    GLFWwindow *glfwwindow;
    int width{}, height{};
    Window();
    Window(int width, int height, const char* title);
    void select() const;
};

class Thing;
class Camera;

class Engine {
double last_game_time = 0.0;
public:
    unsigned int base_vertex_shader;
    float frame_delta = 0.0f;
    std::vector<Camera *> cameras{};
    std::vector<Thing *> things{};
    Window window{};
    Input input{};

    void render(int camera_index);

    void update() const;

    void load_base_vertex_shader();

    void init_render_pipeline();

    void render(int camera_index) const;

    void send_it_to_window();

    bool is_running() const;

    ~Engine();
};
Engine* gameengine(const char* game_name);
extern Engine ge;

int noengine();

#endif //MAIN_H
