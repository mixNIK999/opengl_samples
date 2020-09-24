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

void fill_buffers_old(GLuint &vbo, GLuint &vao, GLuint &ebo)
{
   // create the triangle
   float triangle_vertices[] = {
       -1, -1, 0,	// position vertex 1
       0, 1, 0.0f,	 // color vertex 1

       1, -1, 0.0f,  // position vertex 1
       0, 1, 0.0f,	 // color vertex 1

       -1, 1, 0.0f, // position vertex 1
       0, 1, 0,	 // color vertex 1

       1, 1, 0.0f, // position vertex 1
       0, 1, 0,	 // color vertex 1

   };
   unsigned int triangle_indices[] = {
       0, 1, 2, 1, 2, 3 };
   glGenVertexArrays(1, &vao);
   glGenBuffers(1, &vbo);
   glGenBuffers(1, &ebo);
   glBindVertexArray(vao);
   glBindBuffer(GL_ARRAY_BUFFER, vbo);
   glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_vertices), triangle_vertices, GL_DYNAMIC_DRAW );
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(triangle_indices), triangle_indices, GL_DYNAMIC_DRAW );
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

   std::cout << width << " " <<  height << " " << channels;

   glGenTextures(1, &texture);
   glBindTexture(GL_TEXTURE_1D, texture);
   glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, width, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
   glGenerateMipmap(GL_TEXTURE_1D);

   stbi_image_free(image);
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

   auto const start_time = std::chrono::steady_clock::now();

   while (!glfwWindowShouldClose(window))
   {
      glfwPollEvents();

      // Get windows size
      int display_w, display_h;
      glfwGetFramebufferSize(window, &display_w, &display_h);

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
      static float rotation = 0.0;
      ImGui::SliderFloat("rotation", &rotation, 0, 2 * glm::pi<float>());
      static float translation[] = { 0.0, 0.0 };
//      ImGui::SliderFloat2("position", translation, -1.0, 1.0);
//      static float color[4] = { 1.0f,1.0f,1.0f,1.0f };
//      ImGui::ColorEdit3("color", color);
      static float scale = 0.2;
      ImGui::SliderFloat("scale", &scale, 0, 2);
      static int iteration = 5;
      ImGui::SliderInt("iter", &iteration, 1, 20);
      static float angle = glm::pi<float>() / 4;
      ImGui::SliderFloat("angle", &angle, 0.1, glm::pi<float>() / 2 - 0.1);
      ImGui::End();

      //Creating tree
      auto start_l = glm::vec3(-1, -1, 0);
      auto start_r = glm::vec3(1, -1, 0);
      std::vector<glm::vec3> vertexes = {start_l, start_r};
      std::vector<int> levels = {iteration + 1, iteration + 1};
      std::vector<unsigned int> indexes;
      step(iteration, angle, 0, 1, vertexes, indexes, levels);

      // pushing tree to buffer
      std::vector<float> raw_vertex_buffer;
//      printf("Vertex: ");
      for (int i = 0; i < vertexes.size(); i++) {
         auto point = vertexes[i];
//         printf("(%f, %f), ", point.x, point.y);
         // position
         raw_vertex_buffer.push_back(point.x);
         raw_vertex_buffer.push_back(point.y);
         raw_vertex_buffer.push_back(0);
         // color
         raw_vertex_buffer.push_back( 1.0 * levels[i] / (iteration + 1));
         raw_vertex_buffer.push_back(0);
         raw_vertex_buffer.push_back(0);
      }
//      printf(";\n");

//      printf("Indexes(%llu): ", indexes.size());
//      for (auto i : indexes) {
//         printf("%u, ", i);
//      }
//      printf(";\n");
      fill_buffers(vbo, vao, ebo, raw_vertex_buffer,indexes);


      // Pass the parameters to the shader as uniforms
      triangle_shader.set_uniform("u_rotation", rotation);
      triangle_shader.set_uniform("u_translation", translation[0], translation[1]);
      float const time_from_start = (float)(std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start_time).count() / 1000.0);
      triangle_shader.set_uniform("u_time", time_from_start);
//      triangle_shader.set_uniform("u_color", color[0], color[1], color[2]);


      auto model = glm::rotate(glm::mat4(1), glm::radians(rotation * 60), glm::vec3(0, 1, 0)) * glm::scale(glm::vec3(scale, scale, scale)) ;
      auto view = glm::lookAt<float>(glm::vec3(0, 0, -1), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
      auto projection = glm::perspective<float>(90, float(display_w) / display_h, 0.1, 100);
      auto mvp = projection * view * model;
      //glm::mat4 identity(1.0); 
      //mvp = identity;
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
