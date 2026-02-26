#ifndef INPUT_HPP
#define INPUT_HPP
#pragma once
#include <vector>
#include <array>
#include "coordinates.h"
#include <GLFW/glfw3.h>


/// Input manager class
class Input {
public:
    enum KeyState {
        RELEASED,
        PRESSED,
        JUST_PRESSED,
        JUST_RELEASED,
    };
private:
    /// a list of KeyCodes, that map to "Actions", so one can say is_pressed(0) except for is pressed Space
    std::vector<unsigned int> actions;
    /// A list of states of all keys + mouse buttons
    std::array<KeyState, GLFW_KEY_LAST> all_key_states{};
    /// Keys that just changed state (limit 64)
    std::array<unsigned int, 64> just_updated_keys{};
    /// How many keys just changed state this frame
    size_t just_updated_key_count = 0;

    Vector2 last_mouse_position;
    /// Performs in engine mouse tracking
    void update_mouse_positions();
public:
    /// Mouse mode
    /// Normal - Normal mouse cursor
    /// Hidden - Cursor invisible in window, can move freely
    /// Disabled - Hides cursor, provides unlimited cursor movement, for FP controllers
    enum MouseMode {
        NORMAL = GLFW_CURSOR_NORMAL,
        HIDDEN = GLFW_CURSOR_HIDDEN,
        DISABLED = GLFW_CURSOR_DISABLED
    };



    /// Last mouse position
    Vector2 mouse_position;
    /// How the mouse moved in between frames
    Vector2 mouse_move_delta;

    /// If action is pressed down
    /// @param action_idx 0, 1, 2... the index of the action. Good idea to have the indexes stored in an enum
    [[nodiscard]] bool is_pressed(unsigned int action_idx) const;

    /// If action was just pressed down
    /// @param action_idx 0, 1, 2... the index of the action. Good idea to have the indexes stored in an enum
    [[nodiscard]] bool just_pressed(unsigned int action_idx) const;

    /// If action was just released
    /// @param action_idx 0, 1, 2... the index of the action. Good idea to have the indexes stored in an enum
    [[nodiscard]] bool just_released(unsigned int action_idx) const;

    /// Rebinds the key for an action
    /// @param action_idx the index of the action
    /// @param new_key the new key (from GLFW library)
    void rebind_action(unsigned int action_idx, int new_key);

    /// Defines a list of actions. Which keys correspond to which action indexes.
    /// Just provide a list like this {GLFW_KEY_W, GLFW_KEY_S, GLFW_KEY_A, GLFW_KEY_D} => actions 0, 1, 2, 3 for W, S, A, D
    /// @note Action keys are changeable using rebind_action- That's the whole point
    void set_action_list(const std::vector<int>&& action_keys);

    /// Sets a mouse mode (Default mode NORMAL)
    void set_mouse_mode(MouseMode);

    /// Makes Input manager tick. Ment to be called every frame and is called by Engine.update() function.
    /// @note This function is not ment for the user.
    void update();

    /// Initializes Input manager, called by the Engine constructor at the right time.
    /// @note This function is not ment for the user.
    void init();

    /// Connects callback event to window. In engine function
    void connect_callbacks(GLFWwindow* window);

    /// Static (necessary) key_callback method. Responsible for just pressed and released features.
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
};

#endif //INPUT_HPP
