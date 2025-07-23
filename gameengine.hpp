#ifndef MAIN_H
#define MAIN_H

class Thing;
class Camera;

class Engine {
public:
    unsigned int base_vertex_shader;
    std::vector<Camera *> cameras;
    std::vector<Thing *> things;
    void render(int camera_index);
    void send_it_to_window();
    bool is_running();
};
Engine* gameengine(const char* game_name);
extern Engine ge;

int noengine();

#endif //MAIN_H
