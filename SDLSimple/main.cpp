/**
* Author: [Joseph Lin]
* Assignment: Pong Clone
* Date due: 2024-06-29, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/
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

constexpr int WINDOW_WIDTH = 550 * 2,
              WINDOW_HEIGHT = 400 * 2;

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

//Delta time
constexpr float MILLISECONDS_IN_SECOND = 1000.0;
float g_previous_ticks = 0.0f;

constexpr float ROT_INCREMENT = 1.0f;
//texture global variables
constexpr GLint NUMBER_OF_TEXTURES = 1,
                LEVEL_OF_DETAIL = 0,
                TEXTURE_BORDER = 0;

//pong!
constexpr char RED_SPRITE_FILEPATH[] = "red.png";
constexpr char BLUE_SPRITE_FILEPATH[] = "blue.png";
constexpr char BALLZ_SPRITE_FILEPATH[] = "ballz.png";
constexpr char YOU_WIN_FILEPATH[] = "you_win.png";

//pong!
constexpr glm::vec3 INIT_RED_SCALE = glm::vec3(1.0f, 3.0f, 0.0f),
                    INIT_BLUE_SCALE = glm::vec3(1.0f, 3.0f, 0.0f),
                    INIT_BALLZ_SCALE = glm::vec3(1.0f, 1.0f, 0.0f);

constexpr glm::vec3 INIT_POS_RED = glm::vec3(-4.0f, 0.0f, 0.0f);
constexpr glm::vec3 INIT_POS_BLUE = glm::vec3(4.0f, 0.0f, 0.0f);

bool game_start = false;

bool red_collision_top = false;
bool red_collision_bottom = false;
bool red_collision_top_ai = false;
bool red_collision_bottom_ai = false;

bool blue_collision_top = false;
bool blue_collision_bottom = false;

bool blue_win = false;
bool red_win = false;


bool ballz_collision_top = false;
bool ballz_collision_bottom = false;
bool ballz_collision_right = false;
bool ballz_collision_left = false;



bool ai_mode = false;

constexpr float MIN_COLLISION_DISTANCE = 1.0f;

//keeping track of position using vectors
glm::vec3 g_blue_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_blue_movement = glm::vec3(0.0f, 0.0f, 0.0f);

glm::vec3 g_red_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_red_movement = glm::vec3(0.0f, 0.0f, 0.0f);

glm::vec3 g_ballz_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_ballz_movement = glm::vec3(1.0f, 1.0f, 0.0f);

float ballz_increment_x = 2.0f;
float ballz_increment_y = 2.0f;
float increment = 3.0f;
float direction = 0.0f;
float g_blue_speed = 3.0f;
float g_red_speed = 3.0f;

SDL_Window* g_display_window;
AppStatus g_app_status = RUNNING;
ShaderProgram g_shader_program = ShaderProgram();

//PONG!
glm::mat4 g_view_matrix,
          g_blue_matrix,
          g_red_matrix,
          g_ballz_matrix,
          g_win_matrix,
          g_projection_matrix;

GLuint g_blue_texture_id;
GLuint g_red_texture_id;
GLuint g_ballz_texture_id;
GLuint g_win_texture_id;

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
    g_display_window = SDL_CreateWindow("Assignment 2 :O",
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

    g_red_matrix = glm::mat4(1.0f);
    g_blue_matrix = glm::mat4(1.0f);
    g_ballz_matrix = glm::mat4(1.0f);
    g_view_matrix = glm::mat4(1.0f);
    g_win_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

    g_blue_texture_id = load_texture(BLUE_SPRITE_FILEPATH);
    g_red_texture_id = load_texture(RED_SPRITE_FILEPATH);
    g_ballz_texture_id = load_texture(BALLZ_SPRITE_FILEPATH);
    g_win_texture_id = load_texture(YOU_WIN_FILEPATH);

    //g_kiriko_texture_id = load_texture(KIRIKO_SPRITE_FILEPATH);
    //g_ana_texture_id = load_texture(ANA_SPRITE_FILEPATH);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


void process_input()
{
    
    g_red_movement = glm::vec3(0.0f);
    g_blue_movement = glm::vec3(0.0f);
    g_ballz_movement = glm::vec3(0.0f);


    
    
    
    
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                g_app_status = TERMINATED;
                break;
            
            
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym)
                {
                    case SDLK_UP:
                        g_blue_movement.y = 1.0f;
                        break;

                    case SDLK_DOWN:
                        g_blue_movement.y = -1.0f;
                        break;

                    case SDLK_q:
                        g_app_status = TERMINATED;
                        break;

                    case SDLK_w:
                        g_red_movement.y = 1.0f;
                        break;
                    case SDLK_s:
                        g_red_movement.y = -1.0f;
                        break;
                    case SDLK_t:
                        ai_mode = !(ai_mode);
                        break;
                    case SDLK_SPACE:
                        game_start = true;
                        break;

                    default:
                        break;
                }
                default:
                    break;
        }
    }


    const Uint8* key_state = SDL_GetKeyboardState(NULL);
    if (key_state[SDL_SCANCODE_UP])
    {
        if (blue_collision_top == false)
        {
            g_blue_movement.y = 1.0f;
        }
    }
    else if (key_state[SDL_SCANCODE_DOWN])
    {
        if (blue_collision_bottom == false)
        {
            g_blue_movement.y = -1.0f;
        }
    }
    
    if (key_state[SDL_SCANCODE_W])
    {
        if (ai_mode == false)
        {
            if (red_collision_top == false)
            {
                g_red_movement.y = 1.0f;
            }

        }
        
        
    }
    else if (key_state[SDL_SCANCODE_S])
    {
        if (ai_mode == false)
        {
            if (red_collision_bottom == false)
            {
                g_red_movement.y = -1.0f;
            }

        }
        
    }

    //no cheating!!
    if (glm::length(g_blue_movement) > 1.0f)
    {
        g_blue_movement = glm::normalize(g_blue_movement);
    }
    if (glm::length(g_red_movement) > 1.0f)
    {
        g_red_movement = glm::normalize(g_red_movement);
    }

}


void update()
{

    //DELTA TIME STUFF BRO
    float ticks = (float) SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;
  


    //player input changes
    g_blue_position += g_blue_movement * g_blue_speed * delta_time;
    g_red_position += g_red_movement * g_red_speed * delta_time;

    //PONG RESETS
    g_win_matrix = glm::mat4(1.0f);
    g_red_matrix = glm::mat4(1.0f);
    g_blue_matrix = glm::mat4(1.0f);
    g_ballz_matrix = glm::mat4(1.0f);


    //win text initial position

    if (blue_win == false && red_win == false)
    {
        g_win_matrix = glm::translate(g_win_matrix, glm::vec3(10.0f, 0.0f, 0.0f));
    }
    if (blue_win == true)
    {
        g_win_matrix = glm::translate(g_win_matrix, glm::vec3(3.0f, 0.0f, 0.0f));
    }
    if (red_win == true)
    {
        g_win_matrix = glm::translate(g_win_matrix, glm::vec3(-3.0f, 0.0f, 0.0f));
    }

    g_win_matrix = glm::scale(g_win_matrix, glm::vec3(2.0f, 2.0f, 0.0f));

    //PONG STUFFS BROSSKIE
    
    //red!
    g_red_matrix = glm::translate(g_red_matrix, g_red_position);
    if (ai_mode == true)
    {
        g_red_matrix = glm::mat4(1.0f);
        if (red_collision_top_ai == true || red_collision_bottom_ai == true)
        {
            increment = -increment;
        }
        direction += increment * delta_time;
        g_red_matrix = glm::translate(g_red_matrix, glm::vec3(0.0f, direction, 0.0f));
      
        float y_red_distance_top_ai = (direction + INIT_POS_RED.y + INIT_RED_SCALE.y / 2.0f) - 3.75f;
        float y_red_distance_bottom_ai = (direction + INIT_POS_RED.y - INIT_RED_SCALE.y / 2.0f) + 3.75f;

        if (y_red_distance_top_ai > 0)
        {
            red_collision_top_ai = true;
        }
        else { red_collision_top_ai = false; }
        if (y_red_distance_bottom_ai < 0)
        {
            red_collision_bottom_ai = true;
        }
        else { red_collision_bottom_ai = false; }
    }
    g_red_matrix = glm::translate(g_red_matrix, INIT_POS_RED);
    g_red_matrix = glm::scale(g_red_matrix, INIT_RED_SCALE);
    
    
    //blue! 
    g_blue_matrix = glm::translate(g_blue_matrix, INIT_POS_BLUE);
    g_blue_matrix = glm::translate(g_blue_matrix, g_blue_position);
    g_blue_matrix = glm::scale(g_blue_matrix, INIT_BLUE_SCALE);

 
    

    //COLLISION THINGS
    //float x_red_distance = fabs(g_red_position.x + INIT_POS_RED.x) - ((INIT_RED_SCALE.x + INIT_BLUE_SCALE.x) / 2.0f);

    // Adjust for the top boundary collision
    float y_red_distance_top = (g_red_position.y + INIT_POS_RED.y + INIT_RED_SCALE.y / 2.0f) - 3.75f;
    float y_red_distance_bottom = (g_red_position.y + INIT_POS_RED.y - INIT_RED_SCALE.y / 2.0f) + 3.75f;

    if (y_red_distance_top > 0)
    {
        red_collision_top = true;
    }
    else
    {
        red_collision_top = false;
    }

    if (y_red_distance_bottom < 0)
    {
        red_collision_bottom = true;
    }
    else
    {
        red_collision_bottom = false;
    }


    float y_blue_distance_top = (g_blue_position.y + INIT_POS_BLUE.y + INIT_BLUE_SCALE.y / 2.0f) - 3.75f;
    float y_blue_distance_bottom = (g_blue_position.y + INIT_POS_BLUE.y - INIT_BLUE_SCALE.y / 2.0f) + 3.75f;

    if (y_blue_distance_top > 0)
    {
        blue_collision_top = true;
    }
    else
    {
        blue_collision_top = false;
    }

    if (y_blue_distance_bottom < 0)
    {
        blue_collision_bottom = true;
    }
    else
    {
        blue_collision_bottom = false;
    }


    
    //BALLZ COLLISIONS
    g_ballz_matrix = glm::mat4(1.0f);
    if (ballz_collision_top == true || ballz_collision_bottom == true)
    {
        ballz_increment_y = -ballz_increment_y;
    }
    if (ballz_collision_right == true || ballz_collision_left == true)
    {
        ballz_increment_x = 0;
        ballz_increment_y = 0;
    }
    //if (ballz)
    //g_ballz_movement.y = 3.0f;
    if (game_start == true)
    {
        g_ballz_position.x += ballz_increment_x * delta_time;
        g_ballz_position.y += ballz_increment_y * delta_time;
    }
    
    

    
    //top bottom ball collision
    float y_ballz_distance_top = (g_ballz_position.y + INIT_BALLZ_SCALE.y / 2.0f) - 3.75f;
    float y_ballz_distance_bottom = (g_ballz_position.y - INIT_BALLZ_SCALE.y / 2.0f) + 3.75f;

    //left right ball collision
    //right
    float x_ballz_distance_right = (g_ballz_position.x + INIT_BALLZ_SCALE.x / 2.0f) - 5.0f;
    float x_ballz_distance_left = (g_ballz_position.x - INIT_BALLZ_SCALE.x / 2.0f) + 5.0f;
    
    if (y_ballz_distance_top > 0)
    {
        ballz_collision_top = true;
    }
    else
    {
        ballz_collision_top = false;
    }
    if (y_ballz_distance_bottom < 0)
    {
        ballz_collision_bottom = true;
    }
    else
    {
        ballz_collision_bottom = false;
    }
    //left-right
    if (x_ballz_distance_right > 0)
    {
        ballz_collision_right = true;
        red_win = true;
    }
    else
    {
        ballz_collision_right = false;
    }
    if (x_ballz_distance_left < 0)
    {
        ballz_collision_left = true;
        blue_win = true;
    }
    else
    {
        ballz_collision_left = false;
    }

    

    //MF BALLZ DUDE
    g_ballz_matrix = glm::mat4(1.0f);

    

    float collision_factor = 0.5f;
    //BALLZ COLLISION WITH pADdLleS

    //blue
    float x_ballz_distance_blue = fabs(g_ballz_position.x - INIT_POS_BLUE.x) - 
                   ((INIT_BLUE_SCALE.x * collision_factor + INIT_BALLZ_SCALE.x * collision_factor) / 2.0f);
    float y_ballz_distance_blue = fabs(g_ballz_position.y - (g_blue_position.y + INIT_POS_BLUE.y)) -
        ((INIT_BLUE_SCALE.y * collision_factor + INIT_BALLZ_SCALE.y * collision_factor) / 2.0f);

    //red
    float x_ballz_distance_red = fabs(g_ballz_position.x - INIT_POS_RED.x) -
        ((INIT_RED_SCALE.x * collision_factor + INIT_BALLZ_SCALE.x * collision_factor) / 2.0f);
    float y_ballz_distance_red = fabs(g_ballz_position.y - (g_red_position.y + INIT_POS_RED.y)) -
        ((INIT_RED_SCALE.y * collision_factor + INIT_BALLZ_SCALE.y * collision_factor) / 2.0f);

    if (x_ballz_distance_blue <= 0.0f && y_ballz_distance_blue <= 0.0f)
    {
        ballz_increment_x = -ballz_increment_x;

    }
    if (x_ballz_distance_red <= 0.0f && y_ballz_distance_red <= 0.0f)
    {
        ballz_increment_x = -ballz_increment_x;

    }
    if (game_start == true)
    {
        g_ballz_matrix = glm::translate(g_ballz_matrix, g_ballz_position);
    }
    
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
    draw_object(g_red_matrix, g_red_texture_id);
    draw_object(g_blue_matrix, g_blue_texture_id);
    draw_object(g_ballz_matrix, g_ballz_texture_id);
    draw_object(g_win_matrix, g_win_texture_id);


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