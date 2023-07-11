# GAMES101 Assignment 5

## Features Implemented

| Name                                            | Score |
| ----------------------------------------------- | ----- |
| Runnable Implementation                         | 5     |
| Ray Generation                                  | 10    |
| Implementation of the Moller-Trumbore Algorithm | 15    |



## Demonstration

![img-1](images/test.png)

## Computational Tricks
 - Computing the roots of a quadratic polynomial:
   ```c++
   float q = (b > 0) ? -0.5 * (b + sqrt(discr)) : -0.5 * (b - sqrt(discr));
   x0 = q / a;
   x1 = c / q;
   ```

