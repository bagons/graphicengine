#include <algorithm>
#include <iostream>
#include <vector>
#include <__msvc_print.hpp>

#include "gameengine.hpp"
#include <glm/ext.hpp>
#include "utils.h"


struct RenderContext {
    // render pipeline
    geRendRef<ForwardRenderer3DLayer> fr;
    //ShaderProgram debug_sp{0};
    MeshThing* debug_thing = nullptr;
    geRef<Camera> main_cam;
};


void update() {
    ge.update();
}

void render(RenderContext& rctx) {
    glClearColor(0.4f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //rctx.debug_sp.use();
    //rctx.debug_thing->render();
    rctx.fr->render();

    ge.send_it_to_window();
}

void run(RenderContext& rctx) {
    while (ge.is_running()) {
        //std::cout << "loop" << std::endl;
        update();
        render(rctx);
    }
}


class Player : public MeshThing {
    float move_speed = 5.0f;
    float mouse_sensitivity = 0.002f;
    geRef<Camera> player_cam;
public:
    Player(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material, const geRef<Camera> cam) : MeshThing(std::move(mesh), std::move(material)) {
        player_cam = cam;
    };

    void update() override {
        //std::cout << 1.0f / ge.frame_delta << std::endl;
        glm::vec3 move_vec{0, 0, 0};


        if (ge.input.is_pressed(0))
            move_vec.z -= 1.0f;
        if (ge.input.is_pressed(1))
            move_vec.z += 1.0f;

        if (ge.input.is_pressed(2))
            move_vec.x -= 1.0f;
        if (ge.input.is_pressed(3))
            move_vec.x += 1.0f;

        //float f = -ge.cameras[0]->transform.rotation.y;
        Rotation::rotate_point(0, -player_cam->transform.rotation.y, 0, move_vec);
        player_cam->transform.position = player_cam->transform.position + (move_vec * ge.frame_delta * move_speed);

        if (abs(ge.input.mouse_move_delta.x) > 0 || abs(ge.input.mouse_move_delta.y) > 0) {
            float mouse_move_x = static_cast<float>(ge.input.mouse_move_delta.x) * mouse_sensitivity;
            float mouse_move_y = static_cast<float>(ge.input.mouse_move_delta.y) * mouse_sensitivity;

            player_cam->transform.rotation.y -= mouse_move_x;

            player_cam->transform.rotation.x -= mouse_move_y;
            player_cam->transform.rotation.x = std::clamp(player_cam->transform.rotation.x, -glm::pi<float>() / 2, glm::pi<float>() / 2);
        }
    };
};

class SpinLight : public MeshThing {
public:
    SpinLight(const std::shared_ptr<Mesh> &mesh, const std::shared_ptr<Material> &material) : MeshThing(mesh, material) {

    }
    void update() override {
        if (ge.input.is_pressed(4))
            transform.position = Position{glm::vec3(sin(glfwGetTime())  * 2, 3, cos(glfwGetTime()) * 2)};
    }
};

// TIP To <b>Run</b> code, press <shortcut actionId="Run"/> or click the <icon src="AllIcons.Actions.Execute"/> icon in the gutter.
int main() {
    if (gameengine("demo project") == nullptr)
        return 1;

    // setup actions
    ge.input.set_action_list(std::vector{
        GLFW_KEY_W,
        GLFW_KEY_S,
        GLFW_KEY_A,
        GLFW_KEY_D,
        GLFW_KEY_E
    });
    ge.input.set_mouse_mode(GLFW_CURSOR_DISABLED);

    // PIPELINE INIT
    RenderContext r_ctx;

    auto camera = ge.add<Camera>(45.0f, 0.1f, 100.0f);
    r_ctx.fr = ge.add_render_layer<ForwardRenderer3DLayer>(camera);

    r_ctx.main_cam = camera;

    auto sp = ShaderGen::phong_shader_program_gen(true);

    //r_ctx.debug_sp = sp;
    auto test_material = std::make_shared<Material>(sp);
    sp.use();
    test_material->shader_program.set_uniform("light_pos", glm::vec3{1.0f, 2.0f, 0.0});
    test_material->shader_program.set_uniform("light_color", glm::vec3{1.0f, 1.0f, 1.0f});
    //test_material->save_uniform_value("object_color", glm::vec3(1.0f, 0.0f, 0.0f));
    //auto test_material = std::make_shared<Material>(ge.base_material->shader_program);
    test_material->save_uniform_value("object_color", glm::vec3(1.0, 0.0, 0.0));

    //std::cout << "POST RED: " << r_ctx.debug_sp.id<< std::endl;


    std::vector VERTEX_DATA = {
        0.5f, 0.0f, -0.5f,
        -0.5f, 0.0f, -0.5f,
        0.0f, 0.5f, 0.0f
    };

    std::vector<unsigned int> INDICES = {
        0, 1, 2
    };

    //auto wierd_cube_mesh = std::make_shared<Mesh>(&VERTEX_DATA, &INDICES, false, false);
    auto wierd_cube_mesh = std::make_shared<Mesh>(&ge.meshes.CUBE_VERTEX_DATA, &ge.meshes.CUBE_INDICES, true, true);
    auto special_cube = std::make_shared<Mesh>("res/mesh.obj");

    ge.add<Player>(special_cube, ge.base_material, camera);

    //MeshThing debg_thing(wierd_cube_mesh, mat);

    //r_ctx.debug_thing = &debg_thing;

    // cam setup
    camera->transform.position = Position{0, 1.8, 0};





    //auto sp = ShaderGen::phong_shader_program_gen(true);

    //auto mat = std::make_shared<Material>(sp);
    //mat->save_uniform_value("object_color", glm::vec3{0, 1, 0});


    //player->transform.scale = glm::vec3(0.1f, 0.1f, 0.1f);
    //player->transform.position.x += 2;

    auto sponza_model = std::make_shared<Model>("res/sponza/sponza.obj");
    auto sponza = ge.add<ModelThing>(sponza_model, std::vector<std::shared_ptr<Material>>{});
    sponza->transform.scale = glm::vec3{0.1f,0.1f,0.1f};

    /*auto sponza_mesh = std::make_shared<Mesh>("./res/sponza/spozna_edit.obj");
    auto sponza = ge.add<MeshThing>(sponza_mesh, test_material);
    std::cout << "sponza id:" << sponza.id << std::endl;
    sponza->transform.scale = glm::vec3(0.25, 0.25, 0.25);*/

    //auto light_indicator = ge.add<SpinLight>(special_cube, mat);
    //light_indicator->transform.scale = glm::vec3(0.1f, 0.1f, 0.1f);

    std::cout << "started running" << std::endl;

    run(r_ctx);

    return noengine();
}

