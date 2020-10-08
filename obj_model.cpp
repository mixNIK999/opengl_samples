#include "obj_model.h"

#include <glm/glm.hpp>

#include <vector>
#include <filesystem>
namespace fs = std::filesystem;

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include <GL/glew.h>


class obj_model_t : public IObjModel
{
public:
  obj_model_t(char const * filename);
  ~obj_model_t() = default;

  void draw() override;
private:
   int num_vertices_ = 0;
   GLuint vao_ = 0;
   GLuint vbo_ = 0;
};


obj_model_t::obj_model_t(char const * filename)
{
   spdlog::info("Loading model: {}", filename);

   tinyobj::attrib_t attrib;
   std::vector<tinyobj::shape_t> shapes;
   std::vector<tinyobj::material_t> materials;

   auto parent = fs::absolute(fs::path(filename)).parent_path();
   std::string err;
   const bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filename, parent.u8string().c_str(), true);

   if (!ret)
      throw std::runtime_error(fmt::format("Model load error: {}", err));

//   std::vector<glm::vec3> vertices;
//   std::vector<glm::vec3> normals;
//   std::vector<glm::vec2> tex_coords;
   std::vector<float> join;

   for (auto const & shape : shapes)
   {
      for (const auto &idx : shape.mesh.indices)
      {
         const auto v_i = idx.vertex_index;
         const auto vert_x = attrib.vertices[3 * v_i];
         const auto vert_y = attrib.vertices[3 * v_i + 1];
         const auto vert_z = attrib.vertices[3 * v_i + 2];
         join.push_back(vert_x);
         join.push_back(vert_y);
         join.push_back(vert_z);
//         vertices.emplace_back(vert_x, vert_y, vert_z);

         const auto n_i = idx.normal_index;
         const auto n_x = attrib.normals[3 * n_i];
         const auto n_y = attrib.normals[3 * n_i + 1];
         const auto n_z = attrib.normals[3 * n_i + 2];
         join.push_back(n_x);
         join.push_back(n_y);
         join.push_back(n_z);
//         normals.emplace_back(n_x, n_y, n_z);

         const auto t_i = idx.texcoord_index;
         const auto tc_x = attrib.texcoords[2 * t_i];
         const auto tc_y = attrib.texcoords[2 * t_i + 1];
         join.push_back(tc_x);
         join.push_back(tc_y);
//         tex_coords.emplace_back(tc_x, tc_y);
      }
   }

   num_vertices_ = join.size() / 8;

   glGenVertexArrays(1, &vao_);
   glGenBuffers(1, &vbo_);
   glBindVertexArray(vao_);
   glBindBuffer(GL_ARRAY_BUFFER, vbo_);
   glBufferData(GL_ARRAY_BUFFER, sizeof(float) * join.size(), &join[0], GL_STATIC_DRAW);
   int line_size = (3 + 3 + 2) * sizeof(float);
   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, line_size, (void *)0);
   glEnableVertexAttribArray(0);
   glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, line_size, (void *)(3 * sizeof(float)));
   glEnableVertexAttribArray(1);
   glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, line_size, (void *)(2 * 3 * sizeof(float)));
   glEnableVertexAttribArray(2);
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindVertexArray(0);
}

void obj_model_t::draw()
{
   glBindVertexArray(vao_);
   glDrawArrays(GL_TRIANGLES, 0, num_vertices_);
   glBindVertexArray(0);
}

std::shared_ptr<IObjModel> create_model(char const * filename)
{
  return std::static_pointer_cast<IObjModel>(std::make_shared<obj_model_t>(filename));
}