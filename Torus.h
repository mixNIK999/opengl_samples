#pragma once

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

class Torus {
private:
    int num_vertices_ = 0;

    double big_radius_, small_radius_;
public:

    Torus(double big_radius, double small_radius);
    [[nodiscard]] virtual glm::vec3 get_norm(double alpha, double beta) const;
    [[nodiscard]] virtual glm::vec3 get_point(double alpha, double beta) const;
    [[nodiscard]] static glm::vec2 get_tex_coord(double alpha, double beta);
    static double normalize_angle(double angle);
};


