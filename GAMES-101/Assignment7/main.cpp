#include "Renderer.hpp"
#include "Scene.hpp"
#include "Triangle.hpp"
#include "Sphere.hpp"
#include "Vector.hpp"
#include "global.hpp"
#include <chrono>

// In the main function of the program, we create the scene (create objects and
// lights) as well as set the options for the render (image width and height,
// maximum recursion depth, field-of-view, etc.). We then call the render
// function().
int main(int argc, char **argv)
{
    int width, height, spp;
    bool msaaEnable;

    // width = 1024, height = 1024;
    width = 784, height = 784;
    // width = 512, height = 512;

    // spp = 1;
    spp = 16;
    // spp = 256;
    // spp = 512;
    // spp = 1024;

    msaaEnable = false;
    // msaaEnable = true;

    if (argc == 5)
    {
        width = std::atoi(argv[1]);
        height = std::atoi(argv[2]);
        spp = std::atoi(argv[3]);
        msaaEnable = std::atoi(argv[4]) == 1;
    }

    // Change the definition here to change resolution
    Scene scene(width, height);

    Material *red = new Material(DIFFUSE, Vector3f(0.0f));
    red->Kd = Vector3f(0.63f, 0.065f, 0.05f);
    Material *green = new Material(DIFFUSE, Vector3f(0.0f));
    green->Kd = Vector3f(0.14f, 0.45f, 0.091f);
    Material *white = new Material(DIFFUSE, Vector3f(0.0f));
    white->Kd = Vector3f(0.725f, 0.71f, 0.68f);
    Material *light = new Material(DIFFUSE, (8.0f * Vector3f(0.747f + 0.058f, 0.747f + 0.258f, 0.747f) + 15.6f * Vector3f(0.740f + 0.287f, 0.740f + 0.160f, 0.740f) + 18.4f * Vector3f(0.737f + 0.642f, 0.737f + 0.159f, 0.737f)));
    light->Kd = Vector3f(0.65f);

    MeshTriangle floor("../models/cornellbox/floor.obj", white);
    MeshTriangle shortbox("../models/cornellbox/shortbox.obj", white);
    MeshTriangle tallbox("../models/cornellbox/tallbox.obj", white);
    MeshTriangle left("../models/cornellbox/left.obj", red);
    MeshTriangle right("../models/cornellbox/right.obj", green);
    MeshTriangle light_("../models/cornellbox/light.obj", light);

    // ================= Begin Test =================== //
    // Tests Microfacet
    Material *purple = new Material(DIFFUSE, Vector3f(0.0f));
    purple->Kd = Vector3f(0.137f, 0.0f, 0.4509f);

    float alpha = MicrofacetDistribution::roughtnessToAlpha(1.0);
    BeckmannDistribution *bd = new BeckmannDistribution(alpha, alpha);
    Material *mf = new Material(MICROFACET, Vector3f(0.f), bd);
    mf->ior = 100.f;
    Sphere testSphere(Vector3f{400.f, 75.f, 100.f}, 75.f, mf);
    // scene.Add(&testSphere);
    // =================  End Test  =================== //

    scene.Add(&floor);
    scene.Add(&shortbox);
    scene.Add(&tallbox);
    scene.Add(&left);
    scene.Add(&right);
    scene.Add(&light_);

    scene.buildBVH();

    Renderer r;
    r.setSPP(spp);
    r.setEyePos(Vector3f(278, 273, -800));

    auto start = std::chrono::system_clock::now();
    r.Render(scene, true, msaaEnable);
    auto stop = std::chrono::system_clock::now();

    std::cout << "\nRendering complete. \n";
    std::cout << "Time taken: " << std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count() << " milliseconds\n";
    std::cout << "          : " << std::chrono::duration_cast<std::chrono::seconds>(stop - start).count() << " seconds\n";
    std::cout << "          : " << std::chrono::duration_cast<std::chrono::minutes>(stop - start).count() << " minutes\n";
    std::cout << "          : " << std::chrono::duration_cast<std::chrono::hours>(stop - start).count() << " hours\n";

    return 0;
}