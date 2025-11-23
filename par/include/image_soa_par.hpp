#ifndef PAR_IMAGE_SOA_HPP
#define PAR_IMAGE_SOA_HPP

#include "color.hpp"
#include <cstdint>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

class ImageSOA {
public:
    ImageSOA(int width, int height) : width_(width), height_(height) {
        if (width > 0 and height > 0) {
            size_t const total_pixels = static_cast<size_t>(width_) * static_cast<size_t>(height_);
            r_channel_.resize(total_pixels);
            g_channel_.resize(total_pixels);
            b_channel_.resize(total_pixels);
        }
    }

    [[nodiscard]] int get_width() const { return width_; }

    [[nodiscard]] int get_height() const { return height_; }

    void set_pixel(int x, int y, render::color const & color, double gamma) {
        if (width_ <= 0 or height_ <= 0) {
            return;
        }

        size_t const index =
            static_cast<size_t>(y) * static_cast<size_t>(width_) + static_cast<size_t>(x);

        if (index >= r_channel_.size()) {
            return; 
        }

        r_channel_[index] = color.to_discrete_r(gamma);
        g_channel_[index] = color.to_discrete_g(gamma);
        b_channel_[index] = color.to_discrete_b(gamma);
    }

    void save_ppm(std::string const & filename) const {
        std::ofstream out(filename);
        if (!out.is_open()) {
            throw std::runtime_error("Error: Cannot open file for writing: " + filename);
        }

        out << "P3\n" << width_ << " " << height_ << "\n255\n";

        size_t const total_pixels = static_cast<size_t>(width_) * static_cast<size_t>(height_);
        for (size_t i = 0; i < total_pixels; ++i) {
            out << static_cast<int>(r_channel_[i]) << " " << static_cast<int>(g_channel_[i]) << " "
                << static_cast<int>(b_channel_[i]) << "\n";
        }
    }

private:
    int width_{0};
    int height_{0};
    std::vector<uint8_t> r_channel_;
    std::vector<uint8_t> g_channel_;
    std::vector<uint8_t> b_channel_;
};

#endif // PAR_IMAGE_SOA_HPP
