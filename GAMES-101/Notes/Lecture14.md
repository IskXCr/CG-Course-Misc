# GAMES101 Lecture 14 - Ray Tracing 2 (Acceleration and Radiometry)

## I. Spatial Partitions

### Trivial Partitions

#### Uniform Spatial Partitions (Grid)

- **Preprocessing - Build Acceleration Grid**
  - Find bounding box
  - Create grid
  - Store each object in overlapping cells

- **Grid Resolution Heuristic**

  - $\text{\#cells} = C * \text{\#objects}$

  - $ C \approx 27$ in 3D

- *When do they work well*: When objects are approximately **evenly distributed in size and space** in the scene.

- *When do they fail*: **"Teapot in a stadium"** problem



### Tree-Shaped Partitions

![img-1](images/Lecture14-img-1.png)

#### Oct-Tree

*Recursively divide the current octant into **8 sub-octants** until the current working space is empty or the number of objects contained reaches certain minimum.*

- **Cons**:
  - Too many branches



#### BSP-Tree (Binary Space Partitioning)

*Recursively divide the space using a hyperplane.*



#### KD-Tree

![img-2](images/Lecture14-img-2.png)

*Recursively divide the space along **alternating** axes ($x \to y \to z$ loop) using a hyperplane, creating a binary tree structure.*

*Separations don't have to be **even***.



**Data Structure:**

- **Internal Nodes**:
  - Store:
    - *Split axis*
    - *Split position*
    - *Children*
    - ***No objects are stored***
- **Leaf Nodes**:
  - Store:
    - *List of objects contained*



**Traversing a KD-Tree**: If the ray has intersected with the current node, recursively check all its child nodes.

- If the current node is a leaf node, test intersection with all contained objects.



### General Problems

- *Inside a partition, there may be an triangle which passes through this partition but has none of its vertices inside this partition.*
- *An object may be contained inside multiple partitions, leading to **memory inefficiency***.
  - Ideally we want each object stored in a **single** node only.



## II. Object Partitions

### Bounding Volume Hierarchy (BVH)

**Summary**: 

- Find bounding box
- Recursively split set of objects into two subsets
- **Recompute** the bounding box of the subsets
- Stop when necessary
- Store objects in each leaf node



**How to subdivide a node**? *Make the split as separated and evenly-sized as possible.*

- Choose a dimension to split
- **Heuristic #1**: Always choose the **longest** axis in node
  - To make the partitions more evenly-spaced
- **Heuristic #2**: Split node at location of **median** object
  - To make the tree **balanced**, resulting in less depth of the hierarchy
  - This can be done in $O(n)$ on average using randomized algorithms, and strictly in $O(n)$ using a type of deterministic algorithm at the cost of a significantly higher constant factor.

*Heuristics are of great research interest.*



**Termination criteria**?

- Heuristic: Stop when the number of elements contained in the current node reaches certain minimum.



**Data Structure:**

- **Internal Nodes**:
  - Store:
    - Bounding box
    - *Children*
    - ***No objects are stored***
- **Leaf Nodes**:
  - Store:
    - Bounding box
    - *List of objects contained*

*All objects are in subtrees.*



**Traversing a BVH**:

```pseudocode
Intersect(Ray ray, BVH node) {
	if (ray misses node.bbox) return;
	
	if (node is a leaf node) {
		test intersection with all objects;
		return the closest intersection;
	}
	
	hit1 = Intersect(ray, node.child1);
	hit2 = Intersect(ray, node.child2);
	
	return the closer of hit1, hit2;
}
```

<img src="images/Lecture14-img-3.png" alt="img-3" style="zoom:50%;" />





### Spatial vs Object Partitions

- **Spatial Partition**
  - Partition space into **non-overlapping** regions
  - An object can be contained in multiple regions
- **Object Partition**
  - Partition set of objects into **disjoint subsets**
  - Bounding boxes for each set may overlap in space



## III. Basic Radiometry

**Motivations and stuff to learn**: *Describe the light in a precise manner*.

- Measure system and units for illumination
- **Accurately measure** the spatial properties of light
  - Radiant Flux
  - Intensity
  - Irradiance
  - Radiance
- Perform lighting calculations **in a physically correct manner**.
- *Still based on **Geometric Optics***.



### Radiant Energy and Flux (Power)

***Definition***: **Radiant energy** is the energy of electromagnetic radiation. It is measured in *units of joules*, and denoted by the symbol

$$
Q \quad[\text{J $=$ Joule}]
$$

***Definition***: **Radiant flux** (power) is the energy emitted, reflected, transmitted or received, *per unit time*.
$$
\Phi \equiv \dv{Q}{t} \quad \text{[W = Watt] [lm = lumen]}
$$



### Radiant Intensity

<img src="images/Lecture14-img-4.png" alt="img-4" style="zoom:50%;" />

***Definition***: The **radiant (luminous) intensity** is the power per *unit solid angle* emitted by a point light source.

$$
I(\omega) \equiv \dv{\Phi}{\omega}
\quad
\text{
	$\left[\frac{\text{W}}{\text{sr}}\right]$
	 
	$\left[\frac{\text{lm}}{\text{sr}} = \text{cd} = \text{candela} \right]$
}
$$

where *candela* is one of the seven SI base units.



### Angles, Solid Angles and Direction Vectors

***Definition***: **Angle** is the *ratio* of subtended arc length on a circle to the radius
$$
\theta = \frac{l}{r}
$$
A circle has $2\pi$ **radians**.

***Definition***: **Solid angle** is the ratio of subtended *area* on a sphere to the radius *squared*
$$
\Omega = \frac{A}{r^2}
$$
A sphere has $4\pi$ **steradians**.

$\omega$ will be used to denote a **direction vector** of unit length.

<img src="images\Lecture14-img-5.png" alt="img-5" style="zoom: 33%;" />



### Differential Solid Angle

<img src="images/Lecture14-img-6.png" alt="img-6" style="zoom:50%;" />
$$
\begin{align}
\dd{A} &= (r \dd{\theta}) (r \sin{\theta} \dd{\phi}) \\
&= r^2 \sin{\theta} \dd{\theta} \dd{\phi}
\end{align}
$$

$$
\dd{\omega} = \frac{\dd{A}}{r^2} = \sin{\theta} \dd{\theta} \dd{\phi}
$$



### Isotropic Point Source

<img src="images/Lecture14-img-7.png" alt="img-7" style="zoom:50%;" />
$$
\begin{align}
\Phi &= \int_{S^2} I \dd{\omega} \\
&= 4 \pi I
\end{align}
$$
If uniform, 
$$
I = \frac{\Phi}{4\pi}
$$