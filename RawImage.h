#pragma once

#include <memory>
#include <string>

struct RawImage {
    unsigned char *data;
    int width, height;

    RawImage(unsigned char *data, int w, int h);
    ~RawImage();

    [[nodiscard]] unsigned char get_pixel(int w, int h) const;
    [[nodiscard]] unsigned char get_pixel(double w, double h) const;

    static std::shared_ptr<RawImage> load_from_file(const std::string& filename);
};
