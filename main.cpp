#pragma optimize("", off)

#include <iostream>
#include <vector>
#include <chrono>

#include <fmt/format.h>

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
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "opengl_shader.h"

static void glfw_error_callback(int error, const char *description)
{
   std::cerr << fmt::format("Glfw Error {}: {}\n", error, description);
}


void step(int step_left, float angle, int l, int r,
          std::vector<glm::vec3>& vertexes, std::vector<unsigned int>& indexes, std::vector<int>& levels) {
   if (step_left <= 0) {
      return;
   }
   auto lv = vertexes[l];
   auto rv = vertexes[r];

   auto edge = rv - lv;
   auto r_edge = glm::vec3(-edge.y, edge.x, 0);

   auto top_lv = lv + r_edge;
   int top_lv_index = vertexes.size();
   vertexes.push_back(top_lv);
   levels.push_back(step_left);
   indexes.push_back(l);
   indexes.push_back(r);
   indexes.push_back(top_lv_index);

   auto top_rv = rv + r_edge;
   int top_rv_index = vertexes.size();
   vertexes.push_back(top_rv);
   levels.push_back(step_left);
   indexes.push_back(r);
   indexes.push_back(top_lv_index);
   indexes.push_back(top_rv_index);

   auto leg = glm::rotate(angle, glm::vec3(0, 0, 1)) * glm::vec4((top_rv - top_lv) * glm::cos(angle), 0);
   auto new_v = glm::vec3(leg) + top_lv;
   int new_v_index = vertexes.size();
   vertexes.push_back(new_v);
   levels.push_back(step_left);
   indexes.push_back(top_lv_index);
   indexes.push_back(top_rv_index);
   indexes.push_back(new_v_index);

   step(step_left - 1, angle, top_lv_index, new_v_index, vertexes, indexes, levels);
   step(step_left - 1, angle, new_v_index, top_rv_index, vertexes, indexes, levels);

}
void fill_buffers(GLuint &vbo, GLuint &vao, GLuint &ebo, std::vector<float> &vertexes, std::vector<unsigned int> &indexes)
{

   glGenVertexArrays(1, &vao);
   glGenBuffers(1, &vbo);
   glGenBuffers(1, &ebo);

   glBindVertexArray(vao);
   glBindBuffer(GL_ARRAY_BUFFER, vbo);
   glBufferData(GL_ARRAY_BUFFER, vertexes.size() * sizeof(float), &vertexes[0], GL_STATIC_DRAW);

   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexes.size() * sizeof(unsigned int), &indexes[0], GL_STATIC_DRAW);

   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
   glEnableVertexAttribArray(0);
   glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
   glEnableVertexAttribArray(1);
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindVertexArray(0);
}

void load_image(GLuint & texture)
{
   int width, height, channels;
   stbi_set_flip_vertically_on_load(true);
   unsigned char *image = stbi_load("lena.jpg",
      &width,
      &height,
      &channels,
      STBI_rgb);

   std::cout << width << height << channels;

   glGenTextures(1, &texture);
   glBindTexture(GL_TEXTURE_2D, texture);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
   glGenerateMipmap(GL_TEXTURE_2D);

   stbi_image_free(image);
}

void load_1d_texture(GLuint & texture)
{
   int width, height, channels;
   stbi_set_flip_vertically_on_load(true);
   unsigned char *image = stbi_load("grad3.png",
                                    &width,
                                    &height,
                                    &channels,
                                    STBI_rgb_alpha);

   std::cout << width << " " <<  height << " " << channels << "\n";

   glGenTextures(1, &texture);
   glBindTexture(GL_TEXTURE_1D, texture);
   glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, width, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
   glGenerateMipmap(GL_TEXTURE_1D);

   stbi_image_free(image);
}

namespace map_ui {
   double scale = 0.5;
   double window_w = 1, window_h = 1;
   glm::vec3 world_offset;
   glm::vec3 prev_pos;

   glm::mat4 calc_mvp() {
      auto model = glm::scale(glm::vec3(map_ui::scale, map_ui::scale, map_ui::scale)) * glm::translate(world_offset);
      auto view = glm::lookAt<float>( glm::vec3(0, 0, -1), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
      auto projection = glm::perspective<float>(glm::radians(90.0), float(window_w) / window_h, 0.1, 100);
      return projection * view * model;
   }

   glm::vec3 pixel_to_coord(double x, double y){
      double nx = x / (window_w  * 0.5f) - 1.0f;
      double ny = y / (window_h  * 0.5f) - 1.0f;
      glm::vec4 screen_pos(nx, -ny, 0, 0);
      return glm::vec3(glm::inverse(calc_mvp()) * screen_pos);
   }

   void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
      if (button == GLFW_MOUSE_BUTTON_LEFT && (action == GLFW_PRESS || action == GLFW_RELEASE)) {
         double xpos, ypos;
         glfwGetCursorPos(window, &xpos, &ypos);
         auto new_pos = pixel_to_coord(xpos, ypos);
//         world_offset = (new_pos - prev_pos);
         prev_pos = new_pos;
      }
   }


   void mouse_cursor_callback(GLFWwindow *window, double xpos, double ypos) {

      if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
         return;
      }
      auto new_pos = pixel_to_coord(xpos, ypos);
//      std::cout << "prev:" << prev_pos.x << " " << prev_pos.y << "\n";
//      std::cout << "new:" << new_pos.x << " " << new_pos.y << "\n";
      world_offset += (new_pos - prev_pos);
      prev_pos = new_pos;
   }

   void mouse_scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
      double xpos, ypos;
      glfwGetCursorPos(window, &xpos, &ypos);
      auto prev_world_point = pixel_to_coord(xpos, ypos);
      scale += yoffset / 10;
      auto new_world_point = pixel_to_coord(xpos, ypos);
      world_offset += (new_world_point - prev_world_point);
   }
}

int main(int, char **)
{
   // Use GLFW to create a simple window
   glfwSetErrorCallback(glfw_error_callback);
   if (!glfwInit())
      return 1;


   // GL 3.3 + GLSL 330
   const char *glsl_version = "#version 330";
   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
   //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only

   // Create window with graphics context
   GLFWwindow *window = glfwCreateWindow(1280, 720, "Dear ImGui - Conan", NULL, NULL);
   if (window == NULL)
      return 1;
   glfwMakeContextCurrent(window);
   glfwSwapInterval(1); // Enable vsync
   //set callbacks
   glfwSetMouseButtonCallback(window, map_ui::mouse_button_callback);
   glfwSetCursorPosCallback(window, map_ui::mouse_cursor_callback);
   glfwSetScrollCallback(window, map_ui::mouse_scroll_callback);

   // Initialize GLEW, i.e. fill all possible function pointers for current OpenGL context
   if (glewInit() != GLEW_OK)
   {
      std::cerr << "Failed to initialize OpenGL loader!\n";
      return 1;
   }

   GLuint texture;
//   load_image(texture);
   load_1d_texture(texture);
   // create our geometries
   GLuint vbo, vao, ebo;
//   fill_buffers_old(vbo, vao, ebo);

   // init shader
   shader_t triangle_shader("simple-shader.vs", "simple-shader.fs");

   // Setup GUI context
   IMGUI_CHECKVERSION();
   ImGui::CreateContext();
   ImGuiIO &io = ImGui::GetIO();
   ImGui_ImplGlfw_InitForOpenGL(window, true);
   ImGui_ImplOpenGL3_Init(glsl_version);
   ImGui::StyleColorsDark();

   static int iteration = 7;
   static float angle = 1;
   int prev_iteration = -1;
   float prev_angle = -1;
   std::vector<unsigned int> indexes;

   while (!glfwWindowShouldClose(window))
   {
      glfwPollEvents();

      // Get windows size
      int display_w, display_h;
      glfwGetFramebufferSize(window, &display_w, &display_h);
      glm::vec3 display_size(display_w, display_h, 1);
      map_ui::window_w = display_w;
      map_ui::window_h = display_h;

      // Set viewport to fill the whole window area
      glViewport(0, 0, display_w, display_h);

      // Fill background with solid color
      glClearColor(0.30f, 0.55f, 0.60f, 1.00f);
      glClear(GL_COLOR_BUFFER_BIT);
      //glEnable(GL_CULL_FACE);

      // Gui start new frame
      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();

      // GUI
      ImGui::Begin("Triangle Position/Color");
      ImGui::SliderInt("iter", &iteration, 1, 20);
      ImGui::SliderFloat("angle", &angle, 0.1, glm::pi<float>() / 2 - 0.1);
      ImGui::End();

      //Creating tree
      if (prev_angle != angle || prev_iteration != iteration) {
         indexes.clear();
         auto start_l = glm::vec3(-1, -1, 0);
         auto start_r = glm::vec3(1, -1, 0);
         std::vector<glm::vec3> vertexes = {start_l, start_r};
         std::vector<int> levels = {iteration, iteration};
         step(iteration - 1, angle, 0, 1, vertexes, indexes, levels);

         // pushing tree to buffer
         std::vector<float> raw_vertex_buffer;
         for (int i = 0; i < vertexes.size(); i++) {
            auto point = vertexes[i];
            // position
            raw_vertex_buffer.push_back(point.x);
            raw_vertex_buffer.push_back(point.y);
            raw_vertex_buffer.push_back(0);
            // color
            raw_vertex_buffer.push_back(1.0 * levels[i] / iteration);
            raw_vertex_buffer.push_back(0);
            raw_vertex_buffer.push_back(0);
         }

         fill_buffers(vbo, vao, ebo, raw_vertex_buffer, indexes);
         prev_iteration = iteration;
         prev_angle = angle;
      }


      auto mvp = map_ui::calc_mvp();
      triangle_shader.set_uniform("u_mvp", glm::value_ptr(mvp));
      triangle_shader.set_uniform("u_tex", int(0));


      // Bind triangle shader
      triangle_shader.use();
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_1D, texture);
      glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      // Bind vertex array = buffers + indices
      glBindVertexArray(vao);
      // Execute draw call
      glDrawElements(GL_TRIANGLES, indexes.size(), GL_UNSIGNED_INT, 0);
      glBindTexture(GL_TEXTURE_1D, 0);
      glBindVertexArray(0);

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

   return 0;
}
