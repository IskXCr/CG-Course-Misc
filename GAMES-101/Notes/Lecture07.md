# GAMES101 Lecture 07 - Shading 1 (Illumination, Shading and Graphics Pipeline)

## I. Illumination and Shading

- **Shading**: In this course, *the process of **applying a material** to an object.*



### Bling-Phong Reflectance Model

*A simplified reflectance model.*

**Definitions**:

- $\bold{v}$ - viewer direction (unit vector)

- $\bold{n}$ - surface normal (unit vector)

- $\bold{l}$ - light direction (unit vector)

- $I$ - intensity of light

- Surface parameters: color, shininess, ...

  

**Characteristics**:

- **Local**: No shadows will be generated.



**Components**:

![img-1](images/Lecture07-img-1.png)

- **Diffuse Reflection**: Light is scattered uniformly in all directions.
  $$
  L_d = \frac{k_d I}{r^2} \max(0, \bold{n} \cdot \bold{l})
  $$
  where

  - $L_d$ is the diffusely reflected light, and

  - $k_d$ is the diffuse coefficient (**color**).

- **Specular Reflection**: Light intensity *depends* on view direction. Bright near mirror reflection direction.
  $$
  L_s = \frac{k_s I}{r^2} \max(0, \bold{n}\cdot\bold{h})^p
  $$
  where

  - $L_s$ is the specularly reflected light, 

  - $k_s$ is the specular coefficient,

  - $\bold{h} = \frac{\bold{v} + \bold{l}}{\lVert \bold{v} + \bold{l} \rVert}$ is the bisector of $\bold{v}$ and $\bold{l}$, and

  - $p$ is used control the width of the reflection lobe.

    ![img-2](images/Lecture07-img-2.png)

- **Ambient Reflection**: Constant color to account for disregarded illumination and fill in black shadows. **This is only an approximation.**
  $$
  L_a = k_a I_a
  $$
  where

  - $L_a$ is the reflected ambient light, and
  - $k_a$ is the ambient coefficient.