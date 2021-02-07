#include <glm/ext/scalar_constants.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include "Torus.h"

glm::vec3 Torus::get_norm(double alpha, double beta) const {
    double x = glm::cos(alpha) * glm::cos(beta);
    double y = glm::sin(alpha) * glm::cos(beta);
    double z = glm::sin(beta);
    return {x, y, z};
}

glm::vec3 Torus::get_point(double alpha, double beta) const {
    double x = (big_radius_ + small_radius_ * glm::cos(beta)) * glm::cos(alpha);
    double y = (big_radius_ + small_radius_ * glm::cos(beta)) * glm::sin(alpha);
    double z = small_radius_ * glm::sin(beta);
    return {x, y, z};
}

glm::vec2 Torus::get_tex_coord(double alpha, double beta) {
    alpha = normalize_angle(alpha);
    beta = normalize_angle(beta);

    return {alpha / glm::two_pi<double>(), beta / glm::two_pi<double>()};
}

Torus::Torus(double big_radius, double small_radius) : big_radius_(big_radius), small_radius_(small_radius){}

double Torus::normalize_angle(double angle) {
    while (angle < 0) {
        angle += glm::two_pi<double>();
    }
    while (angle > glm::two_pi<double>()) {
        angle -= glm::two_pi<double>();
    }
    return angle;
}
