#ifndef MAIN_H
#define MAIN_H
#include <map>
#include <unordered_map>
#include <memory>
#include <deque>

#include "glad/glad.h"
#include "gereferences.hpp"
#include "input.hpp"
#include "meshes.hpp"
#include "shaders.hpp"
#include "things.hpp"
#include "renderer.hpp"
#include "lights.hpp"

#include <GLFW/glfw3.h>

typedef std::unordered_map<unsigned int, std::unique_ptr<Thing>> things_container;
typedef std::unordered_map<int, std::unique_ptr<RenderPass>> render_layer_container;


/// Wrapper for a GLFWwindow. Only one window allowed.
class Window {
public:
    /// pointer to a glfwwindow;
    GLFWwindow *glfwwindow;
    /// Window dimensions
    int width, height;
    Window(const char* title, int _width, int _height);
    /// Makes this window context the one which is render on.
    void select() const;
    /// Sets VSync on this window
    void set_vsync(bool _vsync) const;
    ~Window();
};

/// Engine class, it's initialization starts the engine. Holds all managers. Is ment to be a global variable instanced only once, all engine managing is accessible through that object.
class Engine {
    /// Auto increment ID counter for render layers
    int next_render_layer_id = 0;
    double last_game_time = 0.0;
    bool bindless_texture_supported = false;

    /// Inits render pipeline using OpenGL functions, called in the constructor
    void init_render_pipeline();
    /// Auto increment ID counter for entities
    unsigned int next_thing_id = 0;
    /// The last geRef ID that was used
    unsigned int last_used_thing_id = -1;
    /// Deleted geRef IDs, which can be used
    std::deque<unsigned int> deleted_geRef_ids{};
public:
    /// The main window in which the engine draws images (the only window)
    Window window;
    /// Input manager, through here you interact with all the input system features.
    Input input{};
    /// Mesh manager
    Meshes meshes{};
    /// Shader manager, hold base materials, has shader gen features, more on Shaders page
    Shaders shaders{};
    /// Light system manager
    Lights lights;

    /// Get the lowest unused ID for a geRef
    /// @note By getting it, the id is considered to be in use. This method is mainly intended for the Engine.
    [[nodiscard]] unsigned int get_next_geRef_id();
    [[nodiscard]] unsigned int get_last_used_geRef_id() const;

    /// Camera Matrices Uniform Buffer Object ID.
    unsigned int camera_matrix_ubo = -1;
    /// Time elapsed between the last 2 frames. Used as a normalizer so that movement can occur approximately the same speed regardless of the frame rate.
    float frame_delta = 0.0f;
    /// Container that hold all the std::unique_ptr of all spawned entities. You can receive a pointer through the entity ID.
    things_container things{};
    /// Data structure that holds entity ids sorted by Materials, so that entities can be rendered in an optimized order.
    std::multimap<std::shared_ptr<Material>, unsigned int, MaterialSorter> thing_ids_by_shader_program;
    /// Container holding all the render layers, at this point in time usually only one, but serves as a scalable infrastructure
    render_layer_container render_layers{};

    /// Starts the entire Engine, instances all managers.
    /// @param display_name name of the window
    /// @param window_width default window width
    /// @param window_height default window height
    /// @param MAX_NR_POINT_LIGHTS the maximum amount of rendered point lights (default = 16)
    /// @param MAX_NR_DIRECTIONAL_LIGHTS the maximum amount of rendered directional lights (default = 3)
    /// @param light_overflow_action what does the engine do if max amount of rendered lights is excited, more at Lights page
    Engine(const char *display_name, int window_width, int window_height, unsigned int MAX_NR_POINT_LIGHTS = 16, unsigned int MAX_NR_DIRECTIONAL_LIGHTS = 3,  Lights::LightOverflowAction light_overflow_action = Lights::SORT_BY_PROXIMITY);
    /// deletes camera_matrix_ubo from the GPU
    ~Engine();

    /// Calls update on all spawned updatable entities, also calls Input.update(); Ment to be called every frame in the games update function. More in getting started guide.
    void update();

    /// Does the buffer swap and displays image in window, pools for new window events. Ment to be called every frame, after all render functions were called.
    void send_it_to_window();

    /// Returns true unless the window should close (user or OS wants that). Used in while loops to that run until game stops.
    [[nodiscard]] bool is_running() const;

    /// Things container getter, the propper way of getting a Thing* if you are not using a geRef
    Thing* get_thing(unsigned int id);

    /// Render layer container getter, the propper way of getting a RenderPass* if you are not using a geRendRef
    RenderPass* get_render_layer(int id);

    /// Central Engine Message Method
    static void debug_message(const std::string &message);
    /// Central Engine Error Method
    static void debug_error(const std::string &message);
    /// Central Engine Warning Method
    static void debug_warning(const std::string &message);

    /// Spawns an entity in the engine.
    /// @tparam T any class base of Thing, because it's saved in the things_container
    /// @param args a list of arguments passed to the constructor of templated class
    /// @return a geRef<T> object, by which you can reference the entity
    template<typename T, typename... Args>
    requires std::is_base_of_v<Thing, T>
    geRef<T> add(Args&&... args) {
        geRef<T> ref{get_next_geRef_id(), this};

        auto thing = std::make_unique<T>(std::forward<Args>(args)...);

        if constexpr (std::is_base_of_v<MeshThing, T>) {
            thing_ids_by_shader_program.insert({thing.get()->get_material(), ref.id});
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

    /// Spawns a render layer
    /// @tparam T any class base of RenderPass
    /// @param args a list of arguments passed to the constructor of templated class
    /// @return a geRendRef<T> object, by which you can reference the render layer
    template<typename T, typename... Args>
    requires std::is_base_of_v<RenderPass, T>
    geRendRef<T> add_render_layer(Args&&... args) {
        geRendRef<T> ref(next_render_layer_id, this);

        auto thing = std::make_unique<T>(std::forward<Args>(args)...);

        render_layers[next_render_layer_id] = std::move(thing);

        next_render_layer_id++;
        return ref;
    };

    /// Removes a spawned entity
    /// @param id the ID in the geRef.
    void remove_thing(unsigned int id);

    /// Bindless Texture Support Setter
    /// @warning Do not touch this
    void set_bindless_texture_support(bool support_bindless_textures);
    /// Bindless Texture Support Getter
    [[nodiscard]] bool are_bindless_textures_supported() const;
};

///  ment to work with a global instance of the Engine class in the main.cpp of the game
extern Engine ge;

/// @defgroup Resources
/// Classes that represent in engine GPU resources. Except for ShaderProgram and Shader all resources work on a std::shared_ptr basis.
/// @warning ! Multithreaded allocation not supported !
/// @warning Resources interact with the GPU through OpenGL on construction and deconstruction. Meaning if you create any of these resources on different thread then the OpenGL context is open on OpenGL will automatically end the program.
/// @defgroup Things
/// All entities in this graphics engine are called Things. Based on a OOP model. Created using Engine.add method. Accessed through geRef which the engine gives.
/// @defgroup Coordinates
/// A set of classes that serve as data types for Postion, Scale, Color and Rotation etc. with some useful functions. Connected with types from the GLM library.
#endif //MAIN_H
