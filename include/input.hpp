#ifndef INPUT_HPP
#define INPUT_HPP
#pragma once
#include <vector>
#include <GLFW/glfw3.h>
#include "glm/vec2.hpp"

/// An action struct,
struct action {
    int key;
    int state;
};

class Input {
    std::vector<action> actions;
    int all_key_states[GLFW_KEY_LAST]{};
    bool just_updated_key_states[64]{};
    int just_updated_key_states_count = 0;
    glm::dvec2 last_mouse_position{};
public:
    glm::dvec2 mouse_position;
    glm::dvec2 mouse_move_delta;

    bool is_pressed(int action_idx) const;

    bool just_pressed(int action_idx) const;

    bool just_released(int action_idx) const;

    void set_action_list(const std::vector<int> &action_keys);

    void set_mouse_mode(int mode);

    void update();

    void init();

    void connect_callbacks(GLFWwindow* window);
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
};

#endif //INPUT_HPP
