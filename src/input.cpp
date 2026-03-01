#include <glad/glad.h>
#include "graphicengine.hpp"
#include "input.hpp"


void Input::set_action_list(const std::vector<int>&& action_keys) {
    actions.clear();
    for (const auto key : action_keys) {
        actions.push_back(key);
    }
}

void Input::rebind_action(unsigned int action_idx, int new_key) {
    if (action_idx >= actions.size()) {
        Engine::debug_warning("Action '" + std::to_string(action_idx) + "' does not exist. Define it in set action list.");
        return;
    }
    actions[action_idx] = new_key;
}


bool Input::is_pressed(const unsigned int action_idx) const {
    if (action_idx >= actions.size()) {
        Engine::debug_warning("Action '" + std::to_string(action_idx) + "' does not exist. Define it in set action list.");
        return false;
    }
    KeyState ks = all_key_states[actions[action_idx]];
    return ks == PRESSED or ks == JUST_PRESSED;
}

bool Input::just_pressed(const unsigned int action_idx) const {
    if (action_idx >= actions.size()) {
        Engine::debug_warning("Action '" + std::to_string(action_idx) + "' does not exist. Define it in set action list.");
        return false;
    }
    return all_key_states[actions[action_idx]] == JUST_PRESSED;
}


bool Input::just_released(const unsigned int action_idx) const {
    if (action_idx >= actions.size()) {
        Engine::debug_warning("Action '" + std::to_string(action_idx) + "' does not exist. Define it in set action list.");
        return false;
    }
    return all_key_states[actions[action_idx]] == JUST_RELEASED;
}


void Input::update() {
    // change states of keys that have been just pressed
    for (size_t i = 0; i < just_updated_key_count; i++) {
        if (all_key_states[just_updated_keys[i]] == JUST_PRESSED) {
            all_key_states[just_updated_keys[i]] = PRESSED;
        } else if (all_key_states[just_updated_keys[i]] == JUST_RELEASED) {
            all_key_states[just_updated_keys[i]] = RELEASED;
        }
    }
    just_updated_key_count = 0;

    update_mouse_positions();
}

void Input::update_mouse_positions() {
    double mouse_x, mouse_y;
    glfwGetCursorPos(ge.window.glfwwindow, &mouse_x, &mouse_y);
    mouse_position.x = static_cast<float>(mouse_x);
    mouse_position.y = static_cast<float>(mouse_y);

    // calculate mouse relative position
    mouse_move_delta = last_mouse_position - mouse_position;
    last_mouse_position = mouse_position;
}

void Input::init() {
    update_mouse_positions();
    set_mouse_mode(NORMAL);
}


void Input::key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    // due to the need for a static callback
    Input& im = static_cast<Engine*>(glfwGetWindowUserPointer(window))->input;

    if (action == GLFW_PRESS and im.all_key_states[key] != PRESSED) {
        im.all_key_states[key] = JUST_PRESSED;
        if (im.just_updated_key_count < im.just_updated_keys.size()) {
            im.just_updated_keys[im.just_updated_key_count] = key;
            im.just_updated_key_count++;
        }
    } else if (action == GLFW_RELEASE) {
        im.all_key_states[key] = JUST_RELEASED;
        if (im.just_updated_key_count < im.just_updated_keys.size()) {
            im.just_updated_keys[im.just_updated_key_count] = key;
            im.just_updated_key_count++;
        }
    }
}

void Input::mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
    Input& im = static_cast<Engine*>(glfwGetWindowUserPointer(window))->input;

    if (action == GLFW_PRESS and im.all_key_states[button] != PRESSED) {
        im.all_key_states[button] = JUST_PRESSED;
        if (im.just_updated_key_count < im.just_updated_keys.size()) {
            im.just_updated_keys[im.just_updated_key_count] = button;
            im.just_updated_key_count++;
        }
    } else if (action == GLFW_RELEASE) {
        im.all_key_states[button] = JUST_RELEASED;
        if (im.just_updated_key_count < im.just_updated_keys.size()) {
            im.just_updated_keys[im.just_updated_key_count] = button;
            im.just_updated_key_count++;
        }
    }
}


void Input::connect_callbacks(GLFWwindow *window) {
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
}


void Input::set_mouse_mode(const MouseMode mode) {
    glfwSetInputMode(ge.window.glfwwindow, GLFW_CURSOR, mode);
    update_mouse_positions();
    mouse_move_delta = Vector2();
}