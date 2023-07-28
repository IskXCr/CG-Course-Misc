#include <iostream>
#include <vector>

#include "CGL/vector2D.h"

#include "mass.h"
#include "rope.h"
#include "spring.h"

namespace CGL
{

    Rope::Rope(Vector2D start, Vector2D end, int num_nodes, float node_mass, float k, vector<int> pinned_nodes)
    {
        if (num_nodes <= 1)
            throw std::runtime_error("?");

        Vector2D step = (end - start) / (num_nodes - 1);
        Vector2D current = start + step;
        masses.push_back(new Mass(start, node_mass, false));

        for (int i = 1; i < num_nodes; ++i)
        {
            masses.push_back(new Mass(current, node_mass, false));
            springs.push_back(new Spring(masses[i - 1], masses[i], k));
            current += step;
        }

        for (auto &i : pinned_nodes)
        {
            masses[i]->pinned = true;
        }
    }

    double eulerDampingFactor = 0.00005;
    double verletDampingFactor = 0.00005;

    void Rope::simulateEuler(float delta_t, Vector2D gravity)
    {
        for (auto &s : springs)
        {
            // CHECK (Part 2): Use Hooke's law to calculate the force on a node
            // CHECK (Part 2): Add global damping
            Vector2D a2b = s->m2->position - s->m1->position;
            Vector2D forceOnA = s->k * (a2b.norm() - s->rest_length) * a2b.unit();
            s->m1->forces += forceOnA;
            s->m2->forces -= forceOnA;

            Vector2D dampingOnB = -eulerDampingFactor * a2b.unit() * dot(s->m2->velocity - s->m1->velocity, a2b.unit());
            s->m1->forces -= dampingOnB;
            s->m2->forces += dampingOnB;
        }

        for (auto &m : masses)
        {
            if (!m->pinned)
            {
                // CHECK (Part 2): Add the force due to gravity, then compute the new velocity and position
                m->forces += gravity;
                Vector2D a = m->forces / m->mass;
                Vector2D vp = m->velocity + a * delta_t;
                // Vector2D xp = m->position + m->velocity * delta_t; // explicit
                Vector2D xp = m->position + vp * delta_t; // semi-implicit

                m->velocity = vp;
                m->position = xp;
            }

            // Reset all forces on each mass
            m->forces = Vector2D(0, 0);
        }
    }

    void Rope::simulateVerlet(float delta_t, Vector2D gravity)
    {
        for (auto &s : springs)
        {
            // CHECK (Part 3): Simulate one timestep of the rope using explicit Verlet （solving constraints)
            Vector2D a2b = s->m2->position - s->m1->position;
            Vector2D forceOnA = s->k * (a2b.norm() - s->rest_length) * a2b.unit();
            s->m1->forces += forceOnA;
            s->m2->forces -= forceOnA;
        }

        for (auto &m : masses)
        {
            if (!m->pinned)
            {
                m->forces += gravity;
                Vector2D a = m->forces / m->mass;
                // CHECK (Part 3.1): Set the new position of the rope mass
                Vector2D temp_position = m->position;
                m->position += (1 - verletDampingFactor) * (m->position - m->last_position) + a * delta_t * delta_t;
                m->last_position = temp_position;
                // CHECK (Part 4): Add global Verlet damping
            }
            m->forces = Vector2D(0, 0);
        }
    }
}
