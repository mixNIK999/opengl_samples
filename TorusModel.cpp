#include <glm/glm.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <utility>
#include <glm/gtc/constants.hpp>
#include "TorusModel.h"


void TorusModel::init_mash() {
    std::vector<float> join;
    for (int i = 0; i < (int)vertices_.size(); i++) {
        join.emplace_back(vertices_[i].x);
        join.emplace_back(vertices_[i].y);
        join.emplace_back(vertices_[i].z);

        join.emplace_back(normals_[i].x);
        join.emplace_back(normals_[i].y);
        join.emplace_back(normals_[i].z);

        join.emplace_back(tex_coords_[i].x);
        join.emplace_back(tex_coords_[i].y);
    }

    num_vertices_ = (int) join.size() / 8;

    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * join.size(), &join[0], GL_STATIC_DRAW);
    int line_size = (3 + 3 + 2) * sizeof(float);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, line_size, nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, line_size, (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, line_size, (void *)(2 * 3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

TorusModel::TorusModel(int number_of_slices, int number_of_points_on_circle, std::shared_ptr<Torus> logic_torus) : logic_torus_(std::move(logic_torus)) {
    for (int j = 0; j < number_of_points_on_circle; j++) {
        for (int i = 0; i <= number_of_slices; i++) {
            for (int shift : {0, 1}) {
                double alpha = (1.0 * i / number_of_slices) * glm::two_pi<double>();
                double beta = (1.0 * (j + shift) / number_of_points_on_circle) * glm::two_pi<double>();
                vertices_.push_back(logic_torus_->get_point(alpha, beta));
                normals_.push_back(logic_torus_->get_norm(alpha, beta));
                tex_coords_.push_back(logic_torus_->get_tex_coord(alpha, beta));
            }
        }
    }
    init_mash();
}

void TorusModel::draw() {
    glBindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, num_vertices_);
    glBindVertexArray(0);
}
