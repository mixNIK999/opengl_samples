#pragma once

#include <glm/ext/scalar_constants.hpp>
#include <memory>
#include "Torus.h"
#include "RawImage.h"

class TorusWithHeightMap : public Torus {
private:
    const double angle_step_ = glm::pi<double>() / 100;
    const double height_step_ = 1;

    std::shared_ptr<RawImage> height_map_;
public:
    TorusWithHeightMap(double big_radius, double small_radius, std::shared_ptr<RawImage>& height_map);
    [[nodiscard]] glm::vec3 get_norm(double alpha, double beta) const override;
    [[nodiscard]] glm::vec3 get_point(double alpha, double beta) const override;
};

