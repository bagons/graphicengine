#include <glad/glad.h>
#include "graphicengine.hpp"
#include "input.hpp"


void Input::set_action_list(const std::vector<int> &action_keys) {
    for (const auto key : action_keys) {
        actions.push_back(action{key, 0});
    }
}

bool Input::is_pressed(int action_idx) {
    // pressed state == 1 or + 2 (value: 3) if just happened
    return all_key_states[actions[action_idx].key] % 2 == 1;
}

bool Input::just_pressed(int action_idx) {
    // pressed state == 1 (+ 2 if just happened)
    return all_key_states[actions[action_idx].key] == 3;
}


bool Input::just_released(int action_idx) {
    // released state == 0 (+ 2 if just happened)
    return all_key_states[actions[action_idx].key] == 2;
}


void Input::update() {
    // change states of keys that have been just pressed
    for (size_t i = 0; i < just_updated_key_states_count; i++) {
        all_key_states[just_updated_key_states[i]] -= 2;
    }
    just_updated_key_states_count = 0;

    // update mouse position
    glfwGetCursorPos(ge.window.glfwwindow, &mouse_position.x, &mouse_position.y);

    // calculate mouse relative position
    mouse_move_delta = last_mouse_position - mouse_position;

    last_mouse_position = mouse_position;
}

void Input::init() {
    glfwGetCursorPos(ge.window.glfwwindow, &mouse_position.x, &mouse_position.y);
    last_mouse_position = mouse_position;
}


void Input::key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    Input& input_man = static_cast<Engine*>(glfwGetWindowUserPointer(window))->input;
    if (action == GLFW_PRESS || action == GLFW_RELEASE) {
        input_man.all_key_states[key] = action + 2;

        // update in order a 64 int long buffer that tells wich keys were just pressed (next part processed in update method)
        if (input_man.just_updated_key_states_count < 64) {
            input_man.just_updated_key_states[input_man.just_updated_key_states_count] = key;
            input_man.just_updated_key_states_count++;
        }
    }
}

void Input::connect_callbacks(GLFWwindow *window) {
    glfwSetKeyCallback(window, key_callback);
}


void Input::set_mouse_mode(int mode) {
    glfwSetInputMode(ge.window.glfwwindow, GLFW_CURSOR, mode);
}