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

    // ================= Begin Test =================== //
    Material *purple = new Material(DIFFUSE, Vector3f::zero());
    purple->Kd = Vector3f(0.137f, 0.0f, 0.4509f);

    Material *orange = new Material(DIFFUSE, Vector3f::zero());
    orange->Kd = Vector3f(1.0, 0.64f, 0.0f);

    // Material *specularTest = new Material(SPECULAR_TEST, Vector3f(0.0f));
    // specularTest->Krefl = Vector3f(1.0f);

    // Material *dielectricTest = new Material(DIELETRIC_TEST, Vector3f(0.0f));
    // dielectricTest->eta = Vector3f(1.5f);

    // float alpha = MicrofacetDistribution::roughtnessToAlpha(1.0);
    // BeckmannDistribution *bd = new BeckmannDistribution(alpha, alpha);
    // Material *mf = new Material(MICROFACET, Vector3f(0.0f));
    // mf->mfDist = bd;
    // mf->eta = Vector3f(100.0f);

    // float etaRefl = 1.10f, etaK = 1.00f;
    // Material *fresnelReflection = new Material(FRESNEL_REFLECTION, Vector3f::zero());
    // fresnelReflection->fresnel = new FresnelConductor(Vector3f{etaRefl}, Vector3f{etaK});
    // fresnelReflection->eta = {etaRefl};

    // float etaTrans = 1.55f;
    // Material *fresnelTransmission = new Material(FRESNEL_TRANSMISSION, Vector3f::zero());
    // fresnelTransmission->fresnel = new FresnelDielectric(Vector3f{etaTrans});
    // fresnelTransmission->eta = {etaTrans};

    float etaSpec = 2.1f;
    Material *fresnelSpecular = new Material(FRESNEL_SPECULAR, Vector3f::zero());
    fresnelSpecular->fresnel = new FresnelDielectric(Vector3f{etaSpec});
    fresnelSpecular->eta = {etaSpec};

    // Tests Microfacet

    Sphere testSphere(Vector3f(400, 25, 140), 25, purple);
    // Sphere testSphere2(Vector3f(400, 25, 500), 25, orange);
    // Sphere testSphere3(Vector3f(278, 273, -400), 100, fresnelSpecular);

    scene.add(&testSphere);
    // scene.add(&testSphere2);
    // scene.add(&testSphere3);

    // Material *testLight = new Material(DIFFUSE, (120.0f * Vector3f(0.747f, 0.233f, 0.233f)));
    // testLight->Kd = Vector3f(0.65f);
    // Sphere testLightSource(Vector3f{278.0f, 273.0f, -1300.0f}, 200.f, testLight);
    // scene.add(&testLightSource);

    // =================  End Test  =================== //

    MeshTriangle floor("../models/cornellbox/floor.obj", white);               // white
    MeshTriangle tallbox("../models/cornellbox/tallbox.obj", fresnelSpecular); // white
    MeshTriangle shortbox("../models/cornellbox/shortbox.obj", orange);        // white
    MeshTriangle left("../models/cornellbox/left.obj", red);                   // red
    MeshTriangle right("../models/cornellbox/right.obj", green);               // green
    MeshTriangle light_("../models/cornellbox/light.obj", light);              // light

    scene.add(&floor);
    scene.add(&tallbox);
    scene.add(&shortbox);
    scene.add(&left);
    scene.add(&right);
    scene.add(&light_);

    scene.buildBVH();

    Renderer r;
    r.setSPP(spp);
    r.setEyePos(Vector3f(278, 273, -800));

    auto start = std::chrono::system_clock::now();
    r.render(scene, true, msaaEnable);
    auto stop = std::chrono::system_clock::now();

    std::cout << "\nRendering complete. \n";
    std::cout << "Time taken: " << std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count() << " milliseconds\n";
    std::cout << "          : " << std::chrono::duration_cast<std::chrono::seconds>(stop - start).count() << " seconds\n";
    std::cout << "          : " << std::chrono::duration_cast<std::chrono::minutes>(stop - start).count() << " minutes\n";
    std::cout << "          : " << std::chrono::duration_cast<std::chrono::hours>(stop - start).count() << " hours\n";

    return 0;
}