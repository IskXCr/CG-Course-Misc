# GAMES101 Lecture 07 - Shading 1 (Illumination, Shading and Graphics Pipeline)

## I. Illumination and Shading

- **Shading**: In this course, *the process of **applying a material** to an object.*



### Bling-Phong Reflectance Model

*A simplified reflectance model.*

**Definitions**:

- $\bold{v}$ - viewer direction

- $\bold{n}$ - surface normal

- $\bold{l}$ - light direction

- $I$ - intensity of light

- Surface parameters: color, shininess, ...

  

**Characteristics**:

- **Local**: No shadows will be generated.



**Components**:

- **Diffuse Reflection**: Light is scattered uniformly in all directions.
  $$
  L_d = \frac{k_d I}{r^2} \max(0, \bold{n} \cdot \bold{l})
  $$
  where

  - $L_d$ is the diffusely reflected light, and

  - $k_d$ is the diffuse coefficient (**color**).

-  