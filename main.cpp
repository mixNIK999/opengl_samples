#pragma optimize("", off)

#include <iostream>
#include <vector>
#include <chrono>

#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include <GL/glew.h>

// Imgui + bindings
#include "imgui.h"
#include "bindings/imgui_impl_glfw.h"
#include "bindings/imgui_impl_opengl3.h"

// Include glfw3.h after our OpenGL definitions
#include <GLFW/glfw3.h>

// STB, load images
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


// Math constant and routines for OpenGL interop
#include <glm/gtc/constants.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "opengl_shader.h"

#include "obj_model.h"
#include "RawImage.h"
#include "TorusWithHeightMap.h"
#include "TorusModel.h"

static void glfw_error_callback(int error, const char *description)
{
   throw std::runtime_error(fmt::format("Glfw Error {}: {}\n", error, description));
}

void create_sky_cube(GLuint &vbo, GLuint &vao)
{
   // create the triangle
   float vertices[] = {
           // positions
           -1.0f,  1.0f, -1.0f,
           -1.0f, -1.0f, -1.0f,
           1.0f, -1.0f, -1.0f,
           1.0f, -1.0f, -1.0f,
           1.0f,  1.0f, -1.0f,
           -1.0f,  1.0f, -1.0f,

           -1.0f, -1.0f,  1.0f,
           -1.0f, -1.0f, -1.0f,
           -1.0f,  1.0f, -1.0f,
           -1.0f,  1.0f, -1.0f,
           -1.0f,  1.0f,  1.0f,
           -1.0f, -1.0f,  1.0f,

           1.0f, -1.0f, -1.0f,
           1.0f, -1.0f,  1.0f,
           1.0f,  1.0f,  1.0f,
           1.0f,  1.0f,  1.0f,
           1.0f,  1.0f, -1.0f,
           1.0f, -1.0f, -1.0f,

           -1.0f, -1.0f,  1.0f,
           -1.0f,  1.0f,  1.0f,
           1.0f,  1.0f,  1.0f,
           1.0f,  1.0f,  1.0f,
           1.0f, -1.0f,  1.0f,
           -1.0f, -1.0f,  1.0f,

           -1.0f,  1.0f, -1.0f,
           1.0f,  1.0f, -1.0f,
           1.0f,  1.0f,  1.0f,
           1.0f,  1.0f,  1.0f,
           -1.0f,  1.0f,  1.0f,
           -1.0f,  1.0f, -1.0f,

           -1.0f, -1.0f, -1.0f,
           -1.0f, -1.0f,  1.0f,
           1.0f, -1.0f, -1.0f,
           1.0f, -1.0f, -1.0f,
           -1.0f, -1.0f,  1.0f,
           1.0f, -1.0f,  1.0f
   };

   glGenVertexArrays(1, &vao);
   glGenBuffers(1, &vbo);
   glBindVertexArray(vao);
   glBindBuffer(GL_ARRAY_BUFFER, vbo);
   glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
   glEnableVertexAttribArray(0);
   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)(0));
}

unsigned int loadCubemap(std::vector<std::string> faces)
{
   unsigned int textureID;
   glGenTextures(1, &textureID);
   glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
   stbi_set_flip_vertically_on_load(false);

   int width, height, nrChannels;
   for (unsigned int i = 0; i < faces.size(); i++)
   {
      unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
      if (data)
      {
         glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                      0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
         );
         stbi_image_free(data);
      }
      else
      {
         std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
         stbi_image_free(data);
      }
   }
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

   return textureID;
}

void load_image(GLuint & texture, const std::string& filename)
{
   int width, height, channels;
   stbi_set_flip_vertically_on_load(true);
   unsigned char *image = stbi_load(filename.c_str(),
                                    &width,
                                    &height,
                                    &channels,
                                    STBI_rgb_alpha);

   std::cout << width << height << channels;

   glGenTextures(1, &texture);
   glBindTexture(GL_TEXTURE_2D, texture);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
   glGenerateMipmap(GL_TEXTURE_2D);

   stbi_image_free(image);
}

namespace ui {
    float angle_x = 0;
    float angle_y = 0;
    float zoom = 0.8;

    double alpha = 0;
    double beta = 0;

    bool is_alpha = true;
    bool is_plus = true;

    glm::vec2 prev_pos;

    void update_angles(glm::vec2 d) {
       angle_x += d.y / 100;
       angle_y += d.x / 100;
    }

    void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
       if (button == GLFW_MOUSE_BUTTON_LEFT && (action == GLFW_PRESS || action == GLFW_RELEASE)) {
          double xpos, ypos;
          glfwGetCursorPos(window, &xpos, &ypos);
          prev_pos = {xpos, ypos};
       }
    }

    void arrow_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
       if (key == GLFW_KEY_UP && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
          alpha += 0.03;
          is_alpha = true;
          is_plus = true;
       }
       if (key == GLFW_KEY_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
          alpha -= 0.03;
          is_alpha = true;
          is_plus = false;
       }
       if (key == GLFW_KEY_RIGHT && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
          beta -= 0.05;
          is_alpha = false;
          is_plus = false;
       }
       if (key == GLFW_KEY_LEFT && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
          beta += 0.05;
          is_alpha = false;
          is_plus = true;
       }
    }

    void mouse_cursor_callback(GLFWwindow *window, double xpos, double ypos) {

       if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
          return;
       }
       glm::vec2 new_pos = {xpos, ypos};
       update_angles(new_pos - prev_pos);
       prev_pos = new_pos;
    }

    void mouse_scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
       zoom -= yoffset / 100;
    }
}


int main(int, char **)
{
   try
   {
      // Use GLFW to create a simple window
      glfwSetErrorCallback(glfw_error_callback);
      if (!glfwInit())
         throw std::runtime_error("glfwInit error");

      // GL 3.3 + GLSL 330
      const char *glsl_version = "#version 330";
      glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
      glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
      glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
      //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only

      // Create window with graphics context
      GLFWwindow *window = glfwCreateWindow(1280, 720, "Dear ImGui - Conan", NULL, NULL);
      if (window == NULL)
         throw std::runtime_error("Can't create glfw window");

      glfwMakeContextCurrent(window);
      glfwSwapInterval(1); // Enable vsync

      if (glewInit() != GLEW_OK)
         throw std::runtime_error("Failed to initialize glew");

      //set callbacks
      glfwSetMouseButtonCallback(window, ui::mouse_button_callback);
      glfwSetCursorPosCallback(window, ui::mouse_cursor_callback);
      glfwSetScrollCallback(window, ui::mouse_scroll_callback);
      glfwSetKeyCallback(window, ui::arrow_key_callback);

      //load textures
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

      std::vector<std::string> faces =
      {
         "posx.jpg",
         "negx.jpg",
         "posy.jpg",
         "negy.jpg",
         "posz.jpg",
         "negz.jpg"
      };

      unsigned int cubemapTexture = loadCubemap(faces);
      GLuint height_map_texture;
//      load_image(height_map_texture, "wellington-1m-dem.png");
      load_image(height_map_texture, "Stadium2-256x256.png");
      auto hm = RawImage::load_from_file("Stadium2-256x256.png");

      GLuint texture_car;
      load_image(texture_car, "Car_Orange_D.jpg");

//      auto torus = create_model("torus.obj");
      auto logic_torus = std::make_shared<TorusWithHeightMap>(3, 1, hm);
//      auto logic_torus = std::make_shared<Torus>(3, 1);
      auto torus = TorusModel(100, 50, logic_torus);
      auto car = create_model("Toy_Car.obj");

      GLuint sky_vbo, sky_vao;
      create_sky_cube(sky_vbo, sky_vao);

      // init shader
      shader_t skybox_shader("simple-shader.vs", "simple-shader.fs");
      shader_t model_texture_shader("model.vs", "model.fs");

      // Setup GUI context
      IMGUI_CHECKVERSION();
      ImGui::CreateContext();
      ImGuiIO &io = ImGui::GetIO();
      ImGui_ImplGlfw_InitForOpenGL(window, true);
      ImGui_ImplOpenGL3_Init(glsl_version);
      ImGui::StyleColorsDark();


      if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
         std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

      glBindFramebuffer(GL_FRAMEBUFFER, 0);

      while (!glfwWindowShouldClose(window))
      {
         glfwPollEvents();

         // Get windows size
         int display_w, display_h;
         glfwGetFramebufferSize(window, &display_w, &display_h);

         // Gui start new frame
         ImGui_ImplOpenGL3_NewFrame();
         ImGui_ImplGlfw_NewFrame();
         ImGui::NewFrame();

         // GUI
         ImGui::Begin("Triangle Position/Color");

         static float model_scale = 0.2;
         ImGui::SliderFloat("model scale ", &model_scale, 0, 1);

         static bool free_camera = false;
         ImGui::Checkbox("free camera ", &free_camera);

         ImGui::End();

         glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
         glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

         auto car_center = logic_torus->get_point(ui::alpha, ui::beta);

         auto norm = logic_torus->get_norm(ui::alpha, ui::beta);
         auto car_norm = glm::vec3(0, 1, 0);
//         auto new_z = glm::normalize(glm::cross(norm, car_norm));
         auto nex_point = logic_torus->get_point(ui::alpha + ui::is_alpha * ((ui::is_plus)? 1 : -1) * 0.1, ui::beta + (!ui::is_alpha) * ((ui::is_plus)? 1 : -1) * 0.1);
         auto new_z = glm::normalize(nex_point - logic_torus->get_point(ui::alpha, ui::beta));
         auto new_x = glm::normalize(glm::cross(norm, new_z));

         auto transform = glm::mat4(glm::mat3(new_x, norm, new_z));

         auto torus_model = glm::scale(glm::vec3(model_scale, model_scale, model_scale));

         auto car_model = torus_model * glm::translate(car_center) * glm::scale(glm::vec3(0.001, 0.001, 0.001)) * transform;

         auto world_car_center = glm::vec3(car_model * glm::vec4(car_center, 1));

         glm::mat4 view;

         if (!free_camera) {
            view = glm::lookAt<float>(world_car_center + norm * glm::vec3(0.1, 0.1, 0.1), new_z - norm, norm);
         } else {
            view = glm::lookAt<float>(glm::vec3(0, 0, ui::zoom), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0))
                 * glm::rotate(ui::angle_x, glm::vec3(1, 0, 0))
                 * glm::rotate(ui::angle_y, glm::vec3(0, 1, 0));
         }
         auto camera_pos = glm::vec3(inverse(view)[3]);
//               std::cout << glm::to_string(camera_pos) << "\n";

         auto projection = glm::perspective<float>(glm::radians(90.0), 1.0 * display_w / display_h, 0.01, 100);
         // Render sky
         {
            auto mvp = projection * glm::mat4(glm::mat3(view));

            glViewport(0, 0, display_w, display_h);


            glDepthMask(GL_FALSE);
            skybox_shader.use();
            skybox_shader.set_uniform("u_mvp", glm::value_ptr(mvp));
            skybox_shader.set_uniform("u_tex", int(0));

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
            glBindVertexArray(sky_vao);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glBindVertexArray(0);
            glDepthMask(GL_TRUE);
         }

         // Render object
         {

//            std::cout << logic_torus->get_point(alpha, beta).x << "\n";

            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);

            model_texture_shader.use();
            model_texture_shader.set_uniform("model", glm::value_ptr(torus_model));
            model_texture_shader.set_uniform("view", glm::value_ptr(view));
            model_texture_shader.set_uniform("projection", glm::value_ptr(projection));
            model_texture_shader.set_uniform("u_tex", int(0));

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, height_map_texture);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

//       draw objects

            torus.draw();

            model_texture_shader.use();
            model_texture_shader.set_uniform("model", glm::value_ptr(car_model));
            model_texture_shader.set_uniform("view", glm::value_ptr(view));
            model_texture_shader.set_uniform("projection", glm::value_ptr(projection));
            model_texture_shader.set_uniform("u_tex", int(0));

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture_car);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            car->draw();

            glDisable(GL_DEPTH_TEST);
         }

         // Generate gui render commands
         ImGui::Render();

         // Execute gui render commands using OpenGL backend
         ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

         // Swap the backbuffer with the frontbuffer that is used for screen display
         glfwSwapBuffers(window);
      }



      // Cleanup
      ImGui_ImplOpenGL3_Shutdown();
      ImGui_ImplGlfw_Shutdown();
      ImGui::DestroyContext();

      glfwDestroyWindow(window);
      glfwTerminate();
   }
   catch (std::exception const & e)
   {
      spdlog::critical("{}", e.what());
      return 1;
   }

   return 0;
}
