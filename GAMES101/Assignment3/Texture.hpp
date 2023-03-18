//
// Created by LEI XU on 4/27/19.
//

#ifndef RASTERIZER_TEXTURE_H
#define RASTERIZER_TEXTURE_H
#include "global.hpp"
#include <eigen3/Eigen/Eigen>
#include <opencv2/opencv.hpp>

class Texture
{
private:
    cv::Mat image_data;

public:
    Texture(const std::string &name)
    {
        image_data = cv::imread(name);
        cv::cvtColor(image_data, image_data, cv::COLOR_RGB2BGR);
        width = image_data.cols;
        height = image_data.rows;
    }

    int width, height;

    Eigen::Vector3f getColor(float u, float v)
    {
        auto u_img = u * width;
        auto v_img = (1 - v) * height;
        auto color = image_data.at<cv::Vec3b>(v_img, u_img);
        return Eigen::Vector3f(color[0], color[1], color[2]);
    }

    static inline Eigen::Vector3f lerp(Eigen::Vector3f src, Eigen::Vector3f dest, float coeff)
    {
        return src + coeff * (dest - src);
    }

    Eigen::Vector3f getColorBilinear(float u, float v)
    {
        // std::cout << "ColorBilinear: start\n";
        auto u0 = u * width + 0.5f;
        auto v0 = (1 - v) * height + 0.5f;

        // --s3---s4--
        // -----s0----
        // --s1---s2--
        // WARNING: Assuming the (u, v) coordinate is after the (-0.5, -0.5) translation
        // which takes the position of the center of the texel into account.

        auto u1 = std::floor(u0);
        auto v1 = std::floor(v0);

        auto u2 = std::ceil(u0);
        auto v2 = std::floor(v0);

        auto u3 = std::floor(u0);
        auto v3 = std::ceil(v0);

        auto u4 = std::ceil(u0);
        auto v4 = std::ceil(v0);
        // if (u0 < 0 || v0 < 0 || u1 < 0 || v1 < 0 || u2 < 0 || v2 < 0 || u3 < 0 || v3 < 0 || u4 < 0 || v4 < 0) 
        // {
        //     std::cout << "Interpolation: \n";
        //     std::printf("Interpolation: (u0, v0)=(%f, %f)\n", u0, v0);
        //     std::printf("Interpolation: (u1, v1)=(%f, %f)\n", u1, v1);
        //     std::printf("Interpolation: (u2, v2)=(%f, %f)\n", u2, v2);
        //     std::printf("Interpolation: (u3, v3)=(%f, %f)\n", u3, v3);
        //     std::printf("Interpolation: (u4, v4)=(%f, %f)\n", u4, v4);
        // }

        auto raw_color1 = image_data.at<cv::Vec3b>(v1, u1);
        auto raw_color2 = image_data.at<cv::Vec3b>(v2, u2);
        auto raw_color3 = image_data.at<cv::Vec3b>(v3, u3);
        auto raw_color4 = image_data.at<cv::Vec3b>(v4, u4);

        Eigen::Vector3f color1(raw_color1[0], raw_color1[1], raw_color1[2]);
        Eigen::Vector3f color2(raw_color2[0], raw_color2[1], raw_color2[2]);
        Eigen::Vector3f color3(raw_color3[0], raw_color3[1], raw_color3[2]);
        Eigen::Vector3f color4(raw_color4[0], raw_color4[1], raw_color4[2]);

        // WARNING: Possible optimization when either one or both of the coordinates align exactly on the texels.
        auto s = u0 - u1;
        auto t = v0 - v1;
        auto lerp_h1 = lerp(color1, color2, s); // Horizontal lerp
        auto lerp_h2 = lerp(color3, color4, s); // Horizontal lerp
        auto lerp_final = lerp(lerp_h1, lerp_h2, t);
        // std::cout << "ColorBilinear: end\n";

        return Eigen::Vector3f(lerp_final[0], lerp_final[1], lerp_final[2]);
    }
};
#endif // RASTERIZER_TEXTURE_H
