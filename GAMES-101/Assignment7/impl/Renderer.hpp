//
// Created by goksu on 2/25/20.
//
#pragma once

#include "global.hpp"
#include "Scene.hpp"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <Windows.h>
#elif defined(__linux__)
#include <sys/ioctl.h>
#endif // Windows/Linux

struct hit_payload
{
    float tNear;
    uint32_t index;
    Vector2f uv;
    Object *hit_obj;
};

class Renderer
{
public:
    Renderer() : spp(16), cliWidth(0), cliHeight(0), nSlot(1), isCombining(false), coutAvail(true) {}
    std::vector<Vector3f> OnePass(const Scene &scene, int id, bool msaaEnabled);
    void render(const Scene &scene, bool useMultiThread, bool msaaEnabled);

    void setSPP(int _spp) { spp = _spp; }
    void setEyePos(Vector3f _eyePos) { eyePos = _eyePos; }

private:
    // Initialize the progress printing interface
    void initializeProgress(const Scene &scene);

    // Update the progress for one thread
    void updateThreadProgress(int slot, float progress);

    void resetCombineProgress();

    // Print the progress during combining results
    void updateCombineProgress(int id, float progress);

    int spp;
    Vector3f eyePos;
    int cliWidth;
    int cliHeight;
    int nSlot;
    int previousMin;
    int currentMin;
    std::vector<int> status;
    std::vector<float> progresses;
    bool coutAvail;
    bool hasStartedRender;
    bool isCombining;
    std::mutex lock_;
    std::condition_variable condVar_;
};

extern int numThread;