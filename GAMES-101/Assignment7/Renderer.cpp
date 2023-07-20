//
// Created by goksu on 2/25/20.
//

#include <fstream>
#include "Scene.hpp"
#include "Renderer.hpp"

inline float deg2rad(const float &deg) { return deg * M_PI / 180.0; }

const float EPSILON = 1e-2;

thread_local std::random_device dev;
thread_local std::mt19937 rng(dev());
thread_local std::uniform_real_distribution<float> dist(0.f, 1.f); // distribution in range [0, 1]

// The main render function. This where we iterate over all pixels in the image,
// generate primary rays and cast these rays into the scene. The content of the
// framebuffer is saved to a file.
std::vector<Vector3f> Renderer::OnePass(const Scene &scene, int id, bool msaaEnabled)
{
    std::vector<Vector3f> framebuffer(scene.width * scene.height);

    float scale = tan(deg2rad(scene.fov * 0.5));
    float imageAspectRatio = scene.width / (float)scene.height;
    int m = 0;

    updateThreadProgress(id, 0.f);
    for (uint32_t j = 0; j < scene.height; ++j)
    {
        for (uint32_t i = 0; i < scene.width; ++i)
        {

            // generate primary ray direction
            Vector3f msaa4x_buffer;
            if (msaaEnabled)
            {
                float dy = 0.25f;
                for (int a = 0; a < 2; ++a, dy += 0.5f)
                {
                    float dx = 0.25f;
                    for (int b = 0; b < 2; ++b, dx += 0.5f)
                    {
                        float x = (2 * (i + dx) / (float)scene.width - 1) *
                                  imageAspectRatio * scale;
                        float y = (1 - 2 * (j + dy) / (float)scene.height) * scale;

                        Vector3f dir = normalize(Vector3f(-x, y, 1));
                        Ray ray(eyePos, dir);
                        msaa4x_buffer += scene.castRay(ray, 0).regularized() / 4.f;
                    }
                }
            }
            else
            {

                float x = (2 * (i + .5f) / (float)scene.width - 1) *
                          imageAspectRatio * scale;
                float y = (1 - 2 * (j + .5f) / (float)scene.height) * scale;

                Vector3f dir = normalize(Vector3f(-x, y, 1));
                Ray ray(eyePos, dir);
                msaa4x_buffer += scene.castRay(ray, 0).regularized();
            }

            framebuffer[m] = msaa4x_buffer;
            m++;
        }
        if (m % 100 == 0)
            updateThreadProgress(id, j / (float)scene.height);
    }
    updateThreadProgress(id, 1.f);

    return framebuffer;
}

// Use multi-threading to render the scene. Repeatedly call OnePass() in parallel
void Renderer::render(const Scene &scene, bool useMultiThread, bool msaaEnabled)
{
    int numThread = std::thread::hardware_concurrency();
    std::cout << "\n";
    std::cout << " - Renderer::render() - invoked\n";
    std::cout << " - Renderer::render() - resolution = " << scene.width << "x" << scene.height << "\n";
    std::cout << " - Renderer::render() - SPP = " << spp << "\n";
    std::cout << " - Renderer::render() - MSAA Enabled = " << msaaEnabled << "\n";
    std::cout << " - Renderer::render() - using #thread = " << numThread << "\n";
    std::cout << " - Renderer::render() - #pass = " << spp << "\n";
    initializeProgress(scene);
    std::vector<std::future<std::vector<Vector3f>>> futures;
    std::vector<std::vector<Vector3f>> samples;
    std::vector<Vector3f> framebuffer(scene.width * scene.height);

    // issue worker threads
    if (useMultiThread)
    {
        for (int i = 0; i < spp; i += numThread, currentMin += numThread)
        {
            for (int j = i; j < i + numThread && j < spp; ++j)
            {
                futures.emplace_back(std::async([this, &scene, j, msaaEnabled]()
                                                { return OnePass(scene, j, msaaEnabled); }));
            }

            for (int j = i; j < i + numThread && j < spp; ++j)
            {
                futures[j].wait();
                samples.push_back(std::move(futures[j].get()));
            }
        }
    }
    else
    {
        for (int i = 0; i < spp; ++i)
            samples.push_back(OnePass(scene, i, msaaEnabled));
    }

    int m = 0;

    for (uint32_t j = 0; j < scene.height; ++j)
    {
        for (uint32_t i = 0; i < scene.width; ++i)
        {
            Vector3f sample;
            for (int i = 0; i < spp; ++i)
            {
                sample += samples[i][m] / spp;
            }
            framebuffer[m] = sample;
            m++;
        }
        updateCombineProgress(j / (float)scene.height);
    }
    updateCombineProgress(1.f);

    // save framebuffer to file
    FILE *fp = fopen("binary.ppm", "wb");
    (void)fprintf(fp, "P6\n%d %d\n255\n", scene.width, scene.height);

    for (int i = 0; i < scene.height * scene.width; ++i)
    {
        static unsigned char color[3];
        color[0] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].x), 0.6f));
        color[1] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].y), 0.6f));
        color[2] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].z), 0.6f));
        fwrite(color, 1, 3, fp);
    }
    fclose(fp);
}

void get_terminal_size(int &width, int &height)
{
#if defined(_WIN32)
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    width = (int)(csbi.srWindow.Right - csbi.srWindow.Left + 1);
    height = (int)(csbi.srWindow.Bottom - csbi.srWindow.Top + 1);
#elif defined(__linux__)
    struct winsize w;
    ioctl(fileno(stdout), TIOCGWINSZ, &w);
    width = (int)(w.ws_col);
    height = (int)(w.ws_row);
#endif
}

void Renderer::initializeProgress(const Scene &scene)
{
    // Gather console infos
    get_terminal_size(cliWidth, cliHeight);
    nSlot = spp;
    previousMin = -1;
    currentMin = 0;
    status = std::vector<int>(nSlot, 0);
    progresses = std::vector<float>(nSlot, 0.f);
    isCombining = false;
    coutAvail = true;
    hasStartedRender = false;

    // std::cout << "\033[2J\033[1;1H";
}

inline void resetCliScreen(int rows)
{
    for (int i = 0; i < rows; ++i)
        std::cout << "\033[F";
}

inline void UpdateProgress(float progress)
{
    int barWidth = 70;

    std::cout << "[";
    int pos = barWidth * progress;
    for (int i = 0; i < barWidth; ++i)
    {
        if (i < pos)
            std::cout << "=";
        else if (i == pos)
            std::cout << ">";
        else
            std::cout << " ";
    }
    std::cout << "] " << int(progress * 100.0) << " %";
};

void Renderer::updateThreadProgress(int slot, float progress)
{
    progresses[slot] = progress;
    if (progress == 1.f)
        status[slot] = 1;

    std::unique_lock<std::mutex> lock(lock_);
    if (not coutAvail)
    {
        condVar_.wait(lock, [&]()
                      { return coutAvail; });
    }
    coutAvail = false;
    int threshold = std::min(nSlot - currentMin, cliHeight - 2);

    if (currentMin == previousMin)
        resetCliScreen(threshold);
    else
        previousMin = currentMin;

    for (int i = currentMin; i < currentMin + threshold; ++i)
    {
        UpdateProgress(progresses[i]);
        std::cout << " - Pass [" << i << "] " << (progresses[i] >= 1.f ? "[Done]     " : "[Rendering]") << "\n";
    }
    std::cout.flush();

    // if (progress == 1.f && slot >= currentMin) // Update overall progress
    // {
    //     int i;
    //     for (i = currentMin; i <= std::min(slot + 1, nSlot + 1); ++i)
    //         if (i > nSlot || status[i] != 1)
    //             break;
    //     currentMin = i;
    // }

    coutAvail = true;
    condVar_.notify_one();
}

void Renderer::updateCombineProgress(float progress)
{
    if (not isCombining)
        isCombining = true;
    else
        resetCliScreen(1);

    UpdateProgress(progress);
    std::cout << " - Combining " << (progress >= 1.f ? "[Done]     " : "[Combining]") << "\n";
    std::cout.flush();
}