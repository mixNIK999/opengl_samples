#include <stb_image.h>
#include <iostream>
#include "RawImage.h"

RawImage::~RawImage() {
    stbi_image_free(data);
}

std::shared_ptr<RawImage> RawImage::load_from_file(const std::string &filename) {
    int width, height, channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *image = stbi_load(filename.c_str(),
                                     &width,
                                     &height,
                                     &channels,
                                     STBI_grey);

    std::cout << width << " " << height << " " << channels << std::endl;

    return std::make_shared<RawImage>(image, width, height);
}

RawImage::RawImage(unsigned char *data, int w, int h): data(data), width(w), height(h) {}

unsigned char RawImage::get_pixel(int w, int h) const {
    w = (w % width + width) % width;
    h = (h % height + height) % height;
    return data[h * width + w];
}

unsigned char RawImage::get_pixel(double w, double h) const {
    return get_pixel((int)(w * width), (int)(h * height));
}
