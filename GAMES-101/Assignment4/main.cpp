#include <chrono>
#include <iostream>
#include <opencv2/opencv.hpp>

std::vector<cv::Point2f> control_points;

void mouse_handler(int event, int x, int y, int flags, void *userdata)
{
    if (event == cv::EVENT_LBUTTONDOWN && control_points.size() < 4)
    {
        std::cout << "Left button of the mouse is clicked - position (" << x << ", "
                  << y << ")" << '\n';
        control_points.emplace_back(x, y);
    }
}

void naive_bezier(const std::vector<cv::Point2f> &points, cv::Mat &window)
{
    auto &p_0 = points[0];
    auto &p_1 = points[1];
    auto &p_2 = points[2];
    auto &p_3 = points[3];

    for (double t = 0.0; t <= 1.0; t += 0.001)
    {
        auto point = std::pow(1 - t, 3) * p_0 + 3 * t * std::pow(1 - t, 2) * p_1 +
                     3 * std::pow(t, 2) * (1 - t) * p_2 + std::pow(t, 3) * p_3;

        window.at<cv::Vec3b>(point.y, point.x)[2] = 255;
    }
}

cv::Point2f recursive_bezier(const std::vector<cv::Point2f> &control_points, float t)
{
    // TODO: Implement de Casteljau's algorithm
    if (control_points.size() < 1)
        throw std::invalid_argument("Invalid control point vector.");

    if (control_points.size() == 1)
    {
        return control_points[0];
    }
    else if (control_points.size() == 2)
    {
        return (1. - t) * control_points[0] + t * control_points[1];
    }
    else
    {
        std::vector<cv::Point2f> new_points;
        new_points.reserve(control_points.size() - 1);
        for (size_t i = 0; i < control_points.size() - 1; ++i)
        {
            new_points.push_back(cv::Point2f((1. - t) * control_points[i] + t * control_points[i + 1]));
        }
        return recursive_bezier(new_points, t);
    }

    return cv::Point2f();
}

void bezier(const std::vector<cv::Point2f> &control_points, cv::Mat &window)
{
    // TODO: Iterate through all t = 0 to t = 1 with small steps, and call de Casteljau's
    // recursive Bezier algorithm.

    for (double t = 0.0; t <= 1.0; t += 0.0001)
    {
        auto point = recursive_bezier(control_points, t);
        window.at<cv::Vec3b>(point.y, point.x)[2] = 255;
    }
}

static bool inline check_dim(cv::Mat &window, float x, float y)
{
    if (x < 0 || y < 0 || window.rows <= (int)y || window.cols <= (int)x)
        return false;
    return true;
}

/**
 * @brief Color the matrix at position (y, x)
 */
void color_mat(cv::Mat &window, float x0, float y0, float x, float y)
{
    if (!check_dim(window, x0, y0))
        return;

    float dist = std::sqrt((y - y0) * (y - y0) + (x - x0) * (x - x0));
    if (dist > 1.)
        return; // radius = 1.
    int val = window.at<cv::Vec3b>(y0, x0)[2] + 255 * (1. - dist) / 1.;
    window.at<cv::Vec3b>(y0, x0)[2] = std::min(val, 255);
}

void bezier_aa(const std::vector<cv::Point2f> &control_points, cv::Mat &window)
{
    for (double t = 0.0; t <= 1.0; t += 0.0001)
    {
        auto point = recursive_bezier(control_points, t);

        // Anti-aliasing
        auto x0 = std::floor(point.x) - 1;
        auto y0 = std::floor(point.y) - 1;
        auto y = y0;
        for (int i = 0; i < 3; ++i)
        {
            auto x = x0;
            for (int j = 0; j < 3; ++j)
            {
                color_mat(window, x, y, point.x, point.y);
                x += 1.;
            }
            y += 1.;
        }
    }
}

int main()
{
    cv::Mat window = cv::Mat(700, 700, CV_8UC3, cv::Scalar(0));
    cv::cvtColor(window, window, cv::COLOR_BGR2RGB);
    cv::namedWindow("Bezier Curve", cv::WINDOW_AUTOSIZE);

    cv::setMouseCallback("Bezier Curve", mouse_handler, nullptr);

    // produce consistent input
    control_points.emplace_back(63, 540);
    control_points.emplace_back(200, 171);
    control_points.emplace_back(522, 177);
    control_points.emplace_back(672, 603);

    int key = -1;
    while (key != 27)
    {
        for (auto &point : control_points)
        {
            cv::circle(window, point, 3, {255, 255, 255}, 3);
        }

        if (control_points.size() == 4)
        {
            // naive_bezier(control_points, window);
            // bezier(control_points, window);
            bezier_aa(control_points, window);

            cv::imshow("Bezier Curve", window);
            cv::imwrite("my_bezier_curve.png", window);
            key = cv::waitKey(0);

            return 0;
        }

        cv::imshow("Bezier Curve", window);
        key = cv::waitKey(20);
    }

    return 0;
}
