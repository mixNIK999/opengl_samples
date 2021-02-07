#include <glm/gtc/constants.hpp>
#include <glm/glm.hpp>
#include "TorusWithHeightMap.h"

TorusWithHeightMap::TorusWithHeightMap(double big_radius, double small_radius, std::shared_ptr<RawImage>& height_map)
        : Torus(big_radius, small_radius), height_map_(height_map){

}

glm::vec3 TorusWithHeightMap::get_point(double alpha, double beta) const {
    auto torus_point = Torus::get_point(alpha, beta);
    auto tex = get_tex_coord(alpha, beta);
    float h = (float) height_map_->get_pixel(tex[0], tex[1]) / 255;
    return torus_point + Torus::get_norm(alpha, beta) * h;
}

glm::vec3 TorusWithHeightMap::get_norm(double alpha, double beta) const {

    double step_w = glm::two_pi<double>() / height_map_->width;
    double step_h = glm::two_pi<double>() / height_map_->height;

    auto left = get_point(alpha - step_w, beta);
    auto right = get_point(alpha + step_w, beta);
    auto bot = get_point(alpha, beta - step_h);
    auto top = get_point(alpha, beta + step_h);

    return glm::normalize(glm::cross(right-left, top-bot));
}
