/**
 * @file main.cpp
 * @author Joseph Lin (jcl9683@nyu.edu)
 * Assignment: Simple 2D Scene
 * Date due: 2024-06-15, 11:59pm
 * I pledge that I have completed this assignment without
 * collaborating with anyone else, in conformance with the
 * NYU School of Engineering Policies and Procedures on
 * Academic Misconduct.
 */
#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define GL_GLEXT_PROTOTYPES 1
#define LOG(argument) std::cout << argument << '\n'

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"

enum AppStatus { RUNNING, TERMINATED };
enum ScaleDirection { GROWING, SHRINKING };

constexpr int WINDOW_WIDTH = 640 * 2,
              WINDOW_HEIGHT = 480 * 2;

constexpr float BG_RED = 0.1922f,
                BG_BLUE = 0.549f,
                BG_GREEN = 0.9059f,
                BG_OPACITY = 1.0f;

constexpr int VIEWPORT_X = 0,
              VIEWPORT_Y = 0,
              VIEWPORT_WIDTH = WINDOW_WIDTH,
              VIEWPORT_HEIGHT = WINDOW_HEIGHT;

constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
                F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

//constexpr int TRIANGLE_RED = 1.0,
//              TRIANGLE_BLUE = 0.4,
//              TRIANGLE_GREEN = 0.4,
//              TRIANGLE_OPACITY = 1.0;

//Delta time
constexpr float MILLISECONDS_IN_SECOND = 1000.0;
float g_previous_ticks = 0.0f;

//transformation tracker
glm::vec3 g_rotation_kiriko = glm::vec3(0.0f, 0.0f, 0.0f);

constexpr float ROT_INCREMENT = 1.0f;
//texture global variables
constexpr GLint NUMBER_OF_TEXTURES = 1,
                LEVEL_OF_DETAIL = 0,
                TEXTURE_BORDER = 0;

constexpr char KIRIKO_SPRITE_FILEPATH[] = "kiriko.png";

constexpr glm::vec3 INIT_SCALE = glm::vec3(2.5f, 5.263f, 0.0f),
                      INIT_POS_KIRIKO = glm::vec3(2.0f, 0.0f, 0.0f);


SDL_Window* g_display_window;
AppStatus g_app_status = RUNNING;
//ScaleDirection g_scale_direction = GROWING;
ShaderProgram g_shader_program = ShaderProgram();

glm::mat4 g_view_matrix,
            g_kiriko_matrix,
            g_projection_matrix;

GLuint g_kiriko_texture_id;

GLuint load_texture(const char* filepath)
{
    // STEP 1: Loading the image file
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }

    // STEP 2: Generating and binding a texture ID to our image
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

    // STEP 3: Setting our texture filter parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // STEP 4: Releasing our file from memory and returning our texture id
    stbi_image_free(image);

    return textureID;
}

void initialise()
{
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("Assignment 1 :O",
                                SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                WINDOW_WIDTH, WINDOW_HEIGHT,
                                SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

    if (g_display_window == nullptr)
    {
        std::cerr << "Error: SDL window could not be created.\n";
        SDL_Quit();
        exit(1);

    }

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_kiriko_matrix = glm::mat4(1.0f);
    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

    g_kiriko_texture_id = load_texture(KIRIKO_SPRITE_FILEPATH);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


void process_input()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE)
        {
            g_app_status = TERMINATED;
        }
    }
}


void update()
{

    //DELTA TIME STUFF BRO
    float ticks = (float) SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;

    //Updating transformation logic
    g_rotation_kiriko.y += ROT_INCREMENT * delta_time;



    //Reset
    g_kiriko_matrix = glm::mat4(1.0f);

    //Transformations
    g_kiriko_matrix = glm::translate(g_kiriko_matrix, INIT_POS_KIRIKO);
    //g_kiriko_matrix = glm::rotate(g_kiriko_matrix, g_rotation_kiriko.y, glm::vec3(0.0f, 1.0f, 0.0f));
    g_kiriko_matrix = glm::scale(g_kiriko_matrix, INIT_SCALE);





    ///** SCALING **/
    //float scale_factor = BASE_SCALE + MAX_AMPLITUDE * glm::cos(g_frame_counter / PULSE_SPEED);
    //glm::vec3 scale_factors = glm::vec3(scale_factor, scale_factor, 0.0f);
    //g_model_matrix = glm::scale(g_model_matrix, scale_factors);

    //    if (g_frame_counter >= MAX_FRAME)
    //    {
    //        g_frame_counter = 0;
    //        g_scale_direction = g_scale_direction == GROWING ?
    //                            g_scale_direction = SHRINKING :
    //                            g_scale_direction = GROWING;
    //    }
    //    
    //    glm::vec3 scale_factors = glm::vec3(
    //                                        // x-direction
    //                                        g_scale_direction == GROWING ?
    //                                        GROWTH_FACTOR : SHRINK_FACTOR,
    //                                        
    //                                        // y-direction
    //                                        g_scale_direction == GROWING ?
    //                                        GROWTH_FACTOR : SHRINK_FACTOR,
    //                                        
    //                                        // z-direction
    //                                        0.0f
    //                                        );
    //    
    //    
    //    g_model_matrix = glm::scale(g_model_matrix, scale_factors);
}

void draw_object(glm::mat4& object_g_model_matrix, GLuint& object_texture_id)
{
    g_shader_program.set_model_matrix(object_g_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}


void render() {
    glClear(GL_COLOR_BUFFER_BIT);


    //vertices
    float vertices[] =
    {
         -0.5f, -0.5f, //bottom left
         0.5f, -0.5f,  //bottom right
         0.5f, 0.5f,
         -0.5f, -0.5f, 
         0.5f, 0.5f, 
         -0.5f, 0.5f
    };



    //textures
    float texture_coordinates[] = {
        0.0f, 1.0f, // bottom left
        1.0f, 1.0f, // bottom right
        1.0f, 0.0f, // top right
        0.0f, 1.0f, // bottom left
        1.0f, 0.0f, // top right
        0.0f, 0.0f,  // top left
    };

    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());

    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
    
    //bind texture
    draw_object(g_kiriko_matrix, g_kiriko_texture_id);


    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    SDL_GL_SwapWindow(g_display_window);
}


void shutdown() { SDL_Quit(); }


int main(int argc, char* args[])
{
    initialise();

    while (g_app_status == RUNNING)
    {
        process_input();
        update();
        render();
    }

    shutdown();
    return 0;
}