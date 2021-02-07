#pragma once

#include "obj_model.h"
#include "Torus.h"
#include <GL/glew.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <vector>

class TorusModel : SomeModel{
private:

    int num_vertices_ = 0;
    GLuint vao_ = 0;
    GLuint vbo_ = 0;

    std::vector<glm::vec3> vertices_;
    std::vector<glm::vec3> normals_;
    std::vector<glm::vec2> tex_coords_;

    void init_mash();
    std::shared_ptr<Torus> logic_torus_;
public:
    TorusModel(int number_of_slices, int number_of_points_on_circle, std::shared_ptr<Torus> logic_torus);

    void draw() override;

};
