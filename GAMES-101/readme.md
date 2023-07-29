# GAMES101 - Notes, Assignments and Projects

View reports for assignments and projects at the following links.

| Name                                  | Status      |
| ------------------------------------- | ----------- |
| [Assignment 1](./Reports/Assignment1) | Completed   |
| [Assignment 2](./Reports/Assignment2) | Completed   |
| [Assignment 3](./Reports/Assignment3) | Completed   |
| [Assignment 4](./Reports/Assignment4) | Completed   |
| [Assignment 5](./Reports/Assignment5) | Completed   |
| [Assignment 6](./Reports/Assignment6) | Completed   |
| [Assignment 7](./Reports/Assignment7) | Completed   |
| [Assignment 8](./Reports/Assignment8) | Completed   |
| Project                               | Not Started |



## Academic Integrity (IMPORTANT)

As a course requirement, code using the skeleton provided by the course **cannot and should NOT be published on GitHub**. 

Uploading these assignments are solely for **educational and discussion purposes**, as the online version of this course has stopped accepting new assignment submissions.

FOR ANYONE WHO IS **CURRENTLY** OR IS **TO BE ENROLLED** IN THIS COURSE (OR ITS ENGLISH VERSION), **DO NOT** VIEW OR EVEN COPY-PASTE CODE FOR ASSIGNMENTS PROVIDED INSIDE THIS COURSE REPOSITORY, AS THIS IS A **DIRECT VIOLATION OF ACADEMIC INTEGRITY**.



## Introduction to This Course

GAMES101 (2020-02), or **Introduction to Computer Graphics**, conducted by Prof. Lingqi Yan, is an introductory course that explores the fundamentals of Computer Graphics. This comprehensive offering provides students with a solid foundation in essential principles and techniques used in CG, including rendering (both rasterization and modern ray-tracing), geometry, and animation. Additionally, the course showcases the captivating nature of the field and highlights cutting-edge research conducted by leading experts. Students are expected to gain a deeper understanding of visually stunning virtual environments and stay informed about the latest advancements in CG.

[Course Website - GAMES101](https://sites.cs.ucsb.edu/~lingqi/teaching/games101.html)

The corresponding English version can be found at [Course Website - CS180/CS280 - Introduction to Computer Graphics](https://sites.cs.ucsb.edu/~lingqi/teaching/cs180.html).

### Notes

Notes can be found inside another repository [IskXCr - CG-Note-Misc](https://iskxcr.github.io/CG-Note-Misc/GAMES-101/)

### Assignments & Projects

**Prompts for assignments and projects can be found in the last section of this document.**

Their implementation can be found in the corresponding directory.




## About This Repository

This repository contains my assignments and projects for GAMES101 conducted by Prof. Lingqi Yan. 

The purpose of this repository is to track personal progress.




## Notes, Text and Copyright Disclaimer

COPYRIGHT DISCLAIMER: If not otherwise specifically mentioned, all the figures inside the notes are captured/created from slides. If you have found a figure without reference, then either it is from the slides and made by the author, or the source has been mentioned in the slides (for some reason they are not mentioned inside the notes). **The author of these notes doesn't own the COPYRIGHT of them, and there is NO copyright infringement intended.**



## Project Demo





## Selected Assignment Demos

View [Assignment 7](./Reports/Assignment7) for implementation of Fresnel reflections, transmission, Microfacet material and BVH with SAH (Surface Area Heuristics).




## Appendix: List of Assignments

We list, in the following sections, features of frameworks that are **expected to be implemented by students**. The actual implementation *may differ* and can be found under the corresponding repository. (e.g., extra features)

The original framework are generally modified heavily by us to suit the need. Some of the modifications are specified in their corresponding `readme.md` file.

Bonus points are *italicized*.

### Assignment 1 - MVP Transformation and Triangle Rasterization

Requirements:

- **Implement a rasterizer running on CPU**:
  - Manually compute the transformation matrices:
    - Rotation around any axis.
    - Orthogonal projection and perspective projection.



### Assignment 2 - Further on Triangle Rasterization

- **Implement a rasterizer running on CPU**:
  - Manually compute the transformation matrices:
    - Rotation around any axis.
    - Orthogonal projection and perspective projection.
  - Create bounding boxes for triangles.
  - Fast `inside_triangle` test.
  - Z-Buffer.
  - *Anti-aliasing - supersampling:*
    - 4xMSAA



### Assignment 3 - Shaders

- **Implement a rasterizer running on CPU**:
  - Inherit features from Assignment 2.
  - Inherit features from Assignment 1.
  - Implement Phong fragment shader.
  - Implement texture fragment shader.
  - Implement bump fragment shader.
  - Implement displacement fragment shader.
  - *Implement bilinear interpolation on textures.*



### Assignment 4 - Bezier Curve

- **Implement a rasterizer to draw Bezier curves, running on CPU**:
  - Implement De Casteljau algorithm.
  - *Implement anti-aliasing on the Bezier curve.*



### Assignment 5 - Whitted-Style Ray Tracing

- **Implement a ray-tracer to render a sample scene, running on CPU**:
  - Implement method `render()`, in which computation of ray directions should be done given parameters including `width`, `height`, `viewDepth` and etc.
  - Implement the Moller-Trumbore algorithm.



### Assignment 6 - Further on Whitted-Style Ray Tracing

- **Implement a ray-tracer to render a sample scene, running on CPU**:
  - Inherit features from Assignment 5.
  - Implement method `Bounds3::IntersectP` to detect ray-bounding-box intersections.
  - Implement method `BVHAccel::Intersect()` and `BVHAccel::getIntersection()` to traverse the BVH built.
  - *Implement the Surface Area Heuristic method, which relies on particular assumptions about the distribution of rays.*
    - Recursively build the BVH using SAH.



### Assignment 7 - Physically Based Ray Tracing

- **Implement a ray-tracer to render the Cornell Box, running on CPU**:
  - Inherit intersection tests and BVH methods from Assignment 6.
  - Implement the Path Tracing algorithm from scratch.
  - Implement multi-threading on rendering from scratch.
  - *Implement the microfacet material based on the framework*.

*The framework has been heavily modified. Other materials have also been added.*



### Assignment 8 - Physically Based Animation

- **Implement algorithms to simulate a mass-spring system**:
  - Implement the method `CGL::Rope::Rope()`.
  - Implement explicit Euler method and semi-implicit Euler method.
  - Implement Verlet method.
  - Implement damping.



## Appendix: Project

We list the possible topics that students are expected to implement. The actual topic may differ from the reference or out of this recommendation list.

- **Extension of Assignments**

  - [Adaptive Tessellation of Subdivision Surfaces with Displacement Mapping](https://developer.nvidia.com/gpugems/gpugems2/part-i-geometric-complexity/chapter-7-adaptive-tessellation-subdivision-surfaces) (CPU-only is sufficient)

  - Mesh Simplification and Mesh Regularization

  - [Multiple Importance Sampling](https://www.pbr-book.org/3ed-2018/Monte_Carlo_Integration/Importance_Sampling##MultipleImportanceSampling)
  
  
    - Light Field Camera
  
  
  
    - Ambient Occlusion
  
  
  
    - [Cloth Simulation and Rendering](http://graphics.ucsd.edu/~henrik/papers/practical_microcylinder_appearance_model_for_cloth_rendering.pdf)
  


- **Rendering**
  - Volume Scattering
    - [Light Transport in Participating Media](https://cs.dartmouth.edu/~wjarosz/publications/dissertation/chapter4.pdf)
    - [Rendering Participating Media with Bidirectional Path Tracing](http://luthuli.cs.uiuc.edu/~daf/courses/rendering/papers/lafortune96rendering.pdf)
  - Progressive Photon Mapping
    - [Stochastic Progressive Photon Mapping (pbr-book.org)](https://www.pbr-book.org/3ed-2018/Light_Transport_III_Bidirectional_Methods/Stochastic_Progressive_Photon_Mapping)
    - [Progressive Photon Mapping](https://www.ci.i.u-tokyo.ac.jp/~hachisuka/ppm.pdf)
  - [A Practical Model for Subsurface Light Transport](https://graphics.stanford.edu/papers/bssrdf/bssrdf.pdf)
  - Hair Rendering
    - [Light Scattering from Human Hair Fibers](https://www.cs.cornell.edu/~srm/publications/SG03-hair-abstract.html)
    - [An Energy-Conserving Hair Reflectance Model - Eugene d'Eon, et al.](http://www.eugenedeon.com/project/an-energy-conserving-hair-reflectance-model/)
    - [An Efficient and Practical Near and Far Field Fur Reflectance Model - Lingqi Yan, et al.](https://sites.cs.ucsb.edu/~lingqi/publications/paper_fur2.pdf)
  - Glints
    - [Rendering Glints on High-Resolution Normal-Mapped Specular Surfaces - Lingqi Yan, et al.](https://sites.cs.ucsb.edu/~lingqi/publications/paper_glints.pdf)
    - [Position-Normal Distributions for Efficient Rendering of Specular Microstructure - Lingqi Yan, et al.](https://sites.cs.ucsb.edu/~lingqi/publications/paper_glints2.pdf)
    - [Discrete Stochastic Microfacet Models - Wenzel Jakob, et al.](https://research.cs.cornell.edu/stochastic-sg14/stochastic.pdf)
    - [Tracing Ray Differentials - Homan Igehy](https://graphics.stanford.edu/papers/trd/trd.pdf)
- **Animation**

  - 2D-Grid-Based Fluid Simulation

    - [Navier-Stokes Equations](https://games-cn.org/wp-content/uploads/2020/04/Navier_Stokes_equations.pdf)
  - 3D-Position-Based Fluid Simulation
    - [Position Based Fluids - Miles Macklin, et al.](https://games-cn.org/wp-content/uploads/2020/04/pbf_sig_preprint.pdf)
  - 3D Snow Simulation
    - [Material Point Method](https://games-cn.org/wp-content/uploads/2020/04/siggraph2013_snow.pdf)
  - 3D Melting Simulation
    - [~~Introduction to Melting the Rabbit~~](https://games-cn.org/wp-content/uploads/2020/04/siggraph2014_melt.pdf)
  - Rigid Body Simulation
    - [An Introduction to Physically Based Modeling: Rigid Body Simulation I - Unconstrained Rigid Body Dynamics - David Baraf](https://games-cn.org/wp-content/uploads/2020/04/notesd1.pdf)
- **Others**

  - GPU Path Tracer using NVIDIA OptiX
  - Render the world with **two triangles**:

    - [Rendering Worlds with Two Triangles with raytracing on the GPU in 4096 bytes - Inigo Quilez](http://games-cn.org/wp-content/uploads/2020/04/rwwtt.pdf)
  - Mesh from Point Clouds

    - [The Ball-Pivoting Algorithm for Surface Reconstruction - Fausto Bernardini, et al.](https://games-cn.org/wp-content/uploads/2020/04/The_Ball-Pivoting_Algorithm_for_Surface_Reconstruc.pdf)
    - [Poisson Surface Reconstruction - Michael Kazhdan, et al.](https://games-cn.org/wp-content/uploads/2020/04/poissonrecon.pdf)
    - [The Stanford 3D Scanning Repository](http://graphics.stanford.edu/data/3Dscanrep/)