#ifndef INPUT_HPP
#define INPUT_HPP
#include <vector>

#include "GLFW/glfw3.h"

struct action {
    int key;
    int state;
};

class Input {
    std::vector<action> actions;
    int all_key_states[GLFW_KEY_LAST];
    bool just_updated_key_states[64];
    int just_updated_key_states_count = 0;
public:
    bool is_pressed(int action_idx);

    bool just_pressed(int action_idx);

    bool just_released(int action_idx);

    void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

    void set_action_list(const std::vector<int> &action_keys);

    void update();

    static void connect_callbacks(GLFWwindow* window);
};

#endif //INPUT_HPP
