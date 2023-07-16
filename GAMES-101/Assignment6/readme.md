# GAMES101 Assignment 6

## Features Implemented

| Name                                  | Score |
| ------------------------------------- | ----- |
| Runnable Implementation               | 5     |
| BBox Intersection                     | 20    |
| Intersection Search: BVH Traversal    | 15    |
| Intersection Search: SAH Acceleration | 20    |



## Comparing Naive BVH and SAH

The specification of the test platform is listed as follows.

| Subject | Value                                       |
| ------- | ------------------------------------------- |
| CPU     | AMD Ryzen 7 5800H @3.20 GHz, 8c16t          |
| RAM     | DDR4 3200 MHz SODIMM                        |
| OS      | Ubuntu 22.04.2 LTS on Windows 10 x86_64     |
| Kernel  | 5.15.90.1-microsoft-standard-WSL2           |
| g++     | g++ (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0 |



We list the time taken using different methods to build the BVH and render the scene.

| Method                                             | Building #1 | Building #2 | Building #3 | Building Average (ms) |
| -------------------------------------------------- | ----------- | ----------- | ----------- | --------------------- |
| Naive: Split on Longest Axis (`std::sort`)         | 209         | 202         | 202         | 204.3                 |
| Naive: Split on Longest Axis (`std::nth_element`)  | 89          | 98          | 92          | 93                    |
| SAH: Surface Area Heuristics ($O(n)$ optimization) | 78          | 79          | 80          | 79                    |

*Building BVH of given mesh.*

- Considering the `std::sort` inside the naïve implementation, the actual building time can be significantly reduced. 
  - Finding the median of such can be achieved using `std::nth_element` which directly partitions the array without additional cost.



| Method                       | Rendering #1 | Rendering #2 | Rendering #3 | Rendering Average (ms) |
| ---------------------------- | ------------ | ------------ | ------------ | ---------------------- |
| Naive: Split on Longest Axis | 9044         | 9000         | 9020         | 9021.3                 |
| SAH: Surface Area Heuristics | 8002         | 8130         | 8224         | 8118.6                 |

*Time unit: millisecond*



### How is SAH implemented?

Check `../Notes/Lecture14.md`, Appendix A.

Also check the reference at [Bounding Volume Hierarchies (pbr-book.org)](https://pbr-book.org/3ed-2018/Primitives_and_Intersection_Acceleration/Bounding_Volume_Hierarchies#sec:sah).



## Demonstration

### BVH

![bvh](images/bvh.png)

### SAH

![sah](images/sah.png)



## Trivia

Similar implementations can be found in book *PBRT*.

- The naming convention is basically the same.
  - Well, basically copy-and-paste.

