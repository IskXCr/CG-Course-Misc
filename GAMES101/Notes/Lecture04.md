# GAMES101 Lecture 04 - Transformation Cont.

## I. View/Camera Transformation

### MVP Transformation

*Model-Viewing-Projection Transformation.*

- **Camera Transformation**: Converts points in canonical coordinates (or *world coordinates*) to *camera coordinates* or places them in *camera space*.
  - *Camera Space - Eye Space*
- **Projection Transformation**: Moves points from camera space to the *canonical view volume*.
  - Sometimes called *Viewing Transformation*
  - *Canonical View Volume - Clip Space or Normalized Device Coordinates*
- **Viewport Transformation**: Maps the canonical view volume to *screen space*.
  - *Screen Space - Pixel Coordinates*

#### View Transformation

Let the following attributes be that of the camera:

- $\vec{e}$: the position of the camera
- $\hat{g}$: the gaze direction
- $\hat{t}$: up direction, assuming perpendicular to the gaze direction

The target of the view transformation is to transform the coordinate system such that:

- the camera is at the origin, and
- its up direction is the new $y$ axis and it looks at the direction of $-z$.

When deducting the matrix for view transformation, consider the *inverse* rotation which rotates the $x$ axis to $\hat{g} \times \hat{t}$, the $y$ axis to $\hat{t}$ and the $z$ axis to $-\hat{g}$.



