#include <algorithm>
#include <cassert>
#include <chrono>
#include "BVH.hpp"

BVHAccel::BVHAccel(std::vector<Object *> p, int maxPrimsInNode,
                   SplitMethod splitMethod)
    : maxPrimsInNode(std::min(255, maxPrimsInNode)), splitMethod(splitMethod),
      primitives(std::move(p))
{
    if (primitives.empty())
        return;

    std::cout << " - Generating BVH...";
    auto start = std::chrono::high_resolution_clock::now();
    if (splitMethod == SplitMethod::NAIVE)
        root = recursiveBuild(primitives);
    else if (splitMethod == SplitMethod::SAH)
        root = recursiveBuildSAH(primitives);
    auto stop = std::chrono::high_resolution_clock::now();

    std::cout << " Time elapsed: " << std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count() << " milliseconds\n";
    // std::cout << "             : " << std::chrono::duration_cast<std::chrono::seconds>(stop - start).count() << " seconds\n";
    // std::cout << "             : " << std::chrono::duration_cast<std::chrono::minutes>(stop - start).count() << " minutes\n";
    // std::cout << "             : " << std::chrono::duration_cast<std::chrono::hours>(stop - start).count() << " hours\n";
    // std::cout << "\n";
}

BVHBuildNode *BVHAccel::recursiveBuild(std::vector<Object *> objects)
{
    BVHBuildNode *node = new BVHBuildNode();

    // Compute bounds of all primitives in BVH node
    Bounds3 bounds;
    for (int i = 0; i < objects.size(); ++i)
        bounds = Union(bounds, objects[i]->getBounds());
    if (objects.size() == 1)
    {
        // Create leaf _BVHBuildNode_
        node->bounds = objects[0]->getBounds();
        node->object = objects[0];
        node->left = nullptr;
        node->right = nullptr;
        node->area = objects[0]->getArea();
        node->nPrimitives = 1;
        return node;
    }
    else if (objects.size() == 2)
    {
        node->left = recursiveBuild(std::vector{objects[0]});
        node->right = recursiveBuild(std::vector{objects[1]});

        node->bounds = Union(node->left->bounds, node->right->bounds);
        node->area = node->left->area + node->right->area;
        node->nPrimitives = 2;
        return node;
    }
    else
    {
        Bounds3 centroidBounds;
        for (int i = 0; i < objects.size(); ++i)
            centroidBounds =
                Union(centroidBounds, objects[i]->getBounds().Centroid());
        int dim = centroidBounds.maxExtent();
        auto beginning = objects.begin();
        auto middling = objects.begin() + (objects.size() / 2);
        auto ending = objects.end();

        std::nth_element(beginning, middling, ending, [=](auto f1, auto f2)
                         { return f1->getBounds().Centroid()[dim] <
                                  f2->getBounds().Centroid()[dim]; });

        auto leftshapes = std::vector<Object *>(beginning, middling);
        auto rightshapes = std::vector<Object *>(middling, ending);

        assert(objects.size() == (leftshapes.size() + rightshapes.size()));

        node->left = recursiveBuild(leftshapes);
        node->right = recursiveBuild(rightshapes);

        node->bounds = Union(node->left->bounds, node->right->bounds);
        node->area = node->left->area + node->right->area;
        node->nPrimitives = objects.size();
    }

    return node;
}

BucketInfo buckets[nBuckets];

// todo: add support for SAH
BVHBuildNode *BVHAccel::recursiveBuildSAH(std::vector<Object *> objects)
{
    BVHBuildNode *node = new BVHBuildNode();

    // Compute bounds of all primitives in BVH node
    Bounds3 bounds;
    for (int i = 0; i < objects.size(); ++i)
        bounds = Union(bounds, objects[i]->getBounds());
    if (objects.size() == 1)
    {
        // Create leaf _BVHBuildNode_
        node->bounds = objects[0]->getBounds();
        node->object = objects[0];
        node->left = nullptr;
        node->right = nullptr;
        node->area = objects[0]->getArea();
        node->nPrimitives = 1;
        return node;
    }
    else if (objects.size() == 2)
    {
        node->left = recursiveBuildSAH(std::vector{objects[0]});
        node->right = recursiveBuildSAH(std::vector{objects[1]});

        node->bounds = Union(node->left->bounds, node->right->bounds);
        node->area = node->left->area + node->right->area;
        node->nPrimitives = 2;
        return node;
    }
    else if (objects.size() <= 4)
    {
        Bounds3 centroidBounds;
        for (int i = 0; i < objects.size(); ++i)
            centroidBounds =
                Union(centroidBounds, objects[i]->getBounds().Centroid());

        int dim = centroidBounds.maxExtent(); // Choose the longest axis to split

        std::sort(objects.begin(), objects.end(), [=](auto f1, auto f2)
                  { return f1->getBounds().Centroid()[dim] <
                           f2->getBounds().Centroid()[dim]; });

        auto beginning = objects.begin();
        auto middling = objects.begin() + (objects.size() / 2);
        auto ending = objects.end();

        auto leftshapes = std::vector<Object *>(beginning, middling);
        auto rightshapes = std::vector<Object *>(middling, ending);

        assert(objects.size() == (leftshapes.size() + rightshapes.size()));

        node->left = recursiveBuildSAH(leftshapes);
        node->right = recursiveBuildSAH(rightshapes);

        node->bounds = Union(node->left->bounds, node->right->bounds);
        node->area = node->left->area + node->right->area;
        node->nPrimitives = objects.size();
    }
    else
    {
        // Choose the axis
        Bounds3 centroidBounds;
        for (int i = 0; i < objects.size(); ++i)
            centroidBounds =
                Union(centroidBounds, objects[i]->getBounds().Centroid());
        int dim = centroidBounds.maxExtent();

        // Divide the range along the chosen axis into a small number of buckets of equal extent
        resetAllBuckets();
        for (int i = 0; i < objects.size(); ++i)
        {
            // Which bucket does the current object belong to?
            int b = nBuckets * centroidBounds.Offset(objects[i]->getBounds().Centroid())[dim];
            if (b == nBuckets)
                b = nBuckets - 1;

            buckets[b].count++;
            buckets[b].bounds = Union(buckets[b].bounds, objects[i]->getBounds());
        }

        constexpr float traversalCost = .125f;
        constexpr float intersectCost = 1.f;
        Bounds3 forwardBounds[nBuckets - 1];
        Bounds3 backwardBounds[nBuckets - 1];
        int forwardCounts[nBuckets - 1];
        int backwardCounts[nBuckets - 1];

        // Forward pass
        forwardBounds[0] = buckets[0].bounds;
        forwardCounts[0] = buckets[0].count;
        for (int i = 1; i < nBuckets - 1; ++i)
        {
            forwardBounds[i] = Union(forwardBounds[i - 1], buckets[i].bounds);
            forwardCounts[i] = forwardCounts[i - 1] + buckets[i].count;
        }

        // Backward pass
        backwardBounds[nBuckets - 2] = buckets[nBuckets - 1].bounds;
        backwardCounts[nBuckets - 2] = buckets[nBuckets - 1].count;
        for (int i = nBuckets - 3; i >= 0; --i)
        {
            backwardBounds[i] = Union(backwardBounds[i + 1], buckets[i + 1].bounds);
            backwardCounts[i] = backwardCounts[i + 1] + buckets[i + 1].count;
        }

        // Choose the plane to minimize SAH cost
        float minCost = traversalCost +
                        (forwardCounts[0] * forwardBounds[0].SurfaceArea() + backwardCounts[0] * backwardBounds[0].SurfaceArea()) / bounds.SurfaceArea();
        int minCostSplitBucket = 0;
        for (int i = 1; i < nBuckets - 1; ++i)
        {
            float cost = traversalCost +
                         (forwardCounts[i] * forwardBounds[i].SurfaceArea() + backwardCounts[i] * backwardBounds[i].SurfaceArea()) / bounds.SurfaceArea();
            if (cost < minCost)
            {
                minCost = cost;
                minCostSplitBucket = i;
            }
        }

        // Finally we only create one node for each primitive
        auto beginning = objects.begin();
        auto middling = std::partition(objects.begin(), objects.end(),
                                       [=](Object *obj)
                                       {
                                           int b = nBuckets * centroidBounds.Offset(obj->getBounds().Centroid())[dim];
                                           if (b == nBuckets)
                                               b = nBuckets - 1;
                                           return b <= minCostSplitBucket;
                                       }); // Partition into two sets
        auto ending = objects.end();

        auto leftshapes = std::vector<Object *>(beginning, middling);
        auto rightshapes = std::vector<Object *>(middling, ending);

        assert(objects.size() == (leftshapes.size() + rightshapes.size()));

        node->left = recursiveBuildSAH(leftshapes);
        node->right = recursiveBuildSAH(rightshapes);

        node->bounds = Union(node->left->bounds, node->right->bounds);
        node->area = node->left->area + node->right->area;
        node->nPrimitives = objects.size();
    }

    return node;
}

Intersection BVHAccel::Intersect(const Ray &ray) const
{
    Intersection isect;
    if (!root)
        return isect;
    isect = BVHAccel::getIntersection(root, ray);
    return isect;
}

Intersection BVHAccel::getIntersection(BVHBuildNode *node, const Ray &ray) const
{
    // TODO Traverse the BVH to find intersection
    // std::cout << "Current recursion depth: " << node->nPrimitives << std::endl;

    const Vector3f &dir = ray.direction;

    if (not node->bounds.IntersectP(ray, ray.direction_inv, {int(dir.x < 0), int(dir.y < 0), int(dir.z < 0)}))
        return Intersection();

    // If is a leaf node
    if (node->object != nullptr)
    {
        return node->object->getIntersection(ray);
    }
    else
    {
        // Compare two intersections
        // By the construction there exists no node that has only one child. If it is not
        // a leaf node, then both children must exist.
        Intersection intl = getIntersection(node->left, ray);
        Intersection intr = getIntersection(node->right, ray);
        // because each primitive reports intersection iff t > 0, so we can directly do this
        if (!intr.happened || intl.happened && intl.distance < intr.distance)
            return intl;
        else
            return intr;
    }
}

// Sample the BVH by traversing.
// After selected an object, the sampling is done on that object.
// \param p determines the particular object to sample.
void BVHAccel::getSample(BVHBuildNode *node, float p, Intersection &pos, float &pdf)
{
    if (node->left == nullptr || node->right == nullptr)
    {
        node->object->sample(pos, pdf);
        pdf *= node->area;
        return;
    }
    if (p < node->left->area)
        getSample(node->left, p, pos, pdf);
    else
        getSample(node->right, p - node->left->area, pos, pdf);
}

// Sample the bounding box at the point.
// \return modified Intersection object containing position information and normal information,
//         and modified pdf for division.
void BVHAccel::sample(Intersection &pos, float &pdf)
{
    float p = std::sqrt(get_random_float()) * root->area;
    getSample(root, p, pos, pdf);
    pdf /= root->area;
}

void resetAllBuckets()
{
    for (int i = 0; i < nBuckets; ++i)
    {
        buckets[i].count = 0;
        buckets[i].bounds = Bounds3();
    }
}

void traverse(BVHBuildNode *node)
{
    std::cout << "Node with " << node->nPrimitives << " primitives\n";
    if (node->object == nullptr)
    {
        traverse(node->left);
        traverse(node->right);
    }
}

void BVHAccel::printHiearchy()
{
    traverse(root);
}