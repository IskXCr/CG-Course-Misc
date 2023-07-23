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
    orange->Kd = Vector3f(1.0, 0.26f, 0.0f);

    Material *black = new Material(DIFFUSE, Vector3f::zero());
    black->Kd = Vector3f(0.0);

    Material *yellow = new Material(DIFFUSE, Vector3f::zero());
    yellow->Kd = Vector3f(1.0, 1.0, 0);

    // Material *specularTest = new Material(SPECULAR_TEST, Vector3f(0.0f));
    // specularTest->Krefl = Vector3f(1.0f);

    // Material *dielectricTest = new Material(DIELETRIC_TEST, Vector3f(0.0f));
    // dielectricTest->eta = Vector3f(1.5f);

    Vector3f etaRefl(1.10f), etaK(1.00f);
    FresnelConductor fresnelConductor = FresnelConductor(etaRefl, etaK);
    Material *fresnelReflection = new Material(FRESNEL_REFLECTION, Vector3f::zero());
    fresnelReflection->fresnel = &fresnelConductor;
    fresnelReflection->eta = etaRefl;

    Vector3f etaTrans(1.33f);
    FresnelDielectric fresnelDielectric = FresnelDielectric(etaTrans);
    Material *fresnelTransmission = new Material(FRESNEL_TRANSMISSION, Vector3f::zero());
    fresnelTransmission->fresnel = &fresnelDielectric;
    fresnelTransmission->eta = etaTrans;

    Vector3f etaSpec(1.13f);
    FresnelDielectric fresnelDielectricSpec = FresnelDielectric(etaSpec);
    Material *fresnelSpecular = new Material(FRESNEL_SPECULAR, Vector3f::zero());
    fresnelSpecular->fresnel = &fresnelDielectricSpec;
    fresnelSpecular->eta = etaSpec;

    FresnelNoOp fresnelNoOp;

    float alpha = MicrofacetDistribution::roughnessToAlpha(0.01);
    BeckmannDistribution *bd = new BeckmannDistribution(alpha, alpha);
    TrowbridgeReitzDistribution *trd = new TrowbridgeReitzDistribution(alpha, alpha);

    Material *microfacetReflection = new Material(MICROFACET_REFLECTION, Vector3f(0.0f));
    microfacetReflection->mfDist = bd;
    // microfacetReflection->mfDist = trd;
    microfacetReflection->fresnel = &fresnelConductor;
    microfacetReflection->eta = etaRefl;

    Material *microfacetTransmission = new Material(MICROFACET_TRANSMISSION, Vector3f(0.0f));
    microfacetTransmission->mfDist = bd;
    // microfacetReflection->mfDist = trd;
    microfacetTransmission->fresnel = &fresnelDielectric;
    microfacetTransmission->eta = etaTrans;
    
    Material *microfacetSpecular = new Material(MICROFACET_SPECULAR, Vector3f(0.0f));
    microfacetSpecular->mfDist = bd;
    // microfacetReflection->mfDist = trd;
    microfacetSpecular->fresnel = &fresnelDielectric;
    microfacetSpecular->eta = etaSpec;

    // Tests Microfacet

    // Sphere testSphere0(Vector3f(400, 50, 135), 50, purple);
    // Sphere testSphere1(Vector3f(288, 288, 375), 10, purple);
    Sphere testSphere2(Vector3f(400, 25, 425), 25, orange);
    // Sphere testSphere3(Vector3f(263, 288, 375), 10, yellow);
    // Sphere testSphere4(Vector3f(263, 263, 375), 10, black);
    // Sphere testSphericalLen(Vector3f(278, 273, 100), 100, fresnelSpecular);
    // Sphere testSphereMicrofacet(Vector3f(270, 200, 380), 100, microfacetReflection);
    // Sphere testSphereMicrofacet(Vector3f(270, 200, 80), 100, microfacetTransmission);

    // scene.add(&testSphere0);
    // scene.add(&testSphere1);
    scene.add(&testSphere2);
    // scene.add(&testSphere3);
    // scene.add(&testSphere4);
    // scene.add(&testSphericalLen);
    // scene.add(&testSphereMicrofacet);

    // Material *testLight = new Material(DIFFUSE, (8.0f * Vector3f(0.747f + 0.058f, 0.747f + 0.258f, 0.747f) + 15.6f * Vector3f(0.740f + 0.287f, 0.740f + 0.160f, 0.740f) + 18.4f * Vector3f(0.737f + 0.642f, 0.737f + 0.159f, 0.737f)));
    // testLight->Kd = Vector3f(0.65f);
    // Sphere testLightSource(Vector3f{278.0f, 273.0f, -1300.0f}, 200.f, testLight);
    // scene.add(&testLightSource);

    // =================  End Test  =================== //

    MeshTriangle floor("../models/cornellbox/floor.obj", white);         // white
    MeshTriangle tallbox("../models/cornellbox/tallbox.obj", purple);     // white
    MeshTriangle shortbox("../models/cornellbox/shortbox.obj", white);   // white
    MeshTriangle left("../models/cornellbox/left.obj", red);             // red
    MeshTriangle right("../models/cornellbox/right.obj", green);         // green
    MeshTriangle light_("../models/cornellbox/light.obj", light);        // light
    MeshTriangle bunny("../models/bunny/bunny1.obj", microfacetTransmission); // bunny

    scene.add(&floor);
    scene.add(&tallbox);
    // scene.add(&shortbox);
    scene.add(&left);
    scene.add(&right);
    scene.add(&light_);
    scene.add(&bunny);

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