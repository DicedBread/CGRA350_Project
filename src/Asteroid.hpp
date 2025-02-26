#pragma once

// std
#include <chrono>
#include <iostream>
#include <string>

// glm
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

// project
#include "application.hpp"
#include "cgra/cgra_geometry.hpp"
#include "cgra/cgra_gui.hpp"
#include "cgra/cgra_image.hpp"
#include "cgra/cgra_shader.hpp"
#include "cgra/cgra_wavefront.hpp"

#include "PerlinNoise.hpp"

using namespace std;
using namespace cgra;
using namespace glm;

typedef struct {
    float cutoff;
    float edge_length;
    int num_verts;
} AsteroidMeshConfig;

class Asteroid {
  public:
    Asteroid(const siv::PerlinNoise::seed_type seed,
             AsteroidMeshConfig *asteroidMeshConfig);
    void draw(const glm::mat4 &view, const glm::mat4 proj);
    void update_model_transform(const double dt);
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 rotation_axis;
    double rotation_velocity;
    void regenerate_mesh(const siv::PerlinNoise::seed_type seed);

  private:
    cgra::gl_mesh mesh;
    glm::mat4 modelTransform;
    glm::vec3 color;
    double rotation_angle;
    AsteroidMeshConfig *asteroidMeshConfig;

    // Load the shader program, if it hasn't been loaded already.
    // The shader program is stored in a static variable, so it is shared
    // between all instances.
    static GLuint shader;
    static void load_shader();

    static GLuint texture;
    static void load_texture();

    static vec2 xyzToUv(vec3 xyz) {
        vec3 n = normalize(xyz);

        float longitude = atan2(n.z, n.x);
        float latitude = asin(n.y);

        float u =
            1.0f - (longitude + glm::pi<float>()) / (2.0f * glm::pi<float>());
        float v = (latitude + glm::half_pi<float>()) / glm::pi<float>();

        return glm::vec2(u, v);
    }

    // A helper function for finding when a the t value of when a linear
    // interpolation crosses a cutoff value.
    double inverse_lerp(const double a, const double b, const double x) {
        return (a - x) / (a - b);
    }

    // Returns a vec3 containing the offsets from the origin to the vertex
    // withing the given edge number
    vec3 marching_cubes_edge(const int edge_num, const double *points,
                             const double cutoff);

    // Returns a vector of the vertices of the triangles that make up a
    // single case for marching cubes. The returned array will list the
    // verts in triplets, capped off with a -1.
    int *marching_cubes_tris(const int case_num);

    // Returns the calculated normal of a vertex lying on the edge of a
    // marching cube for smooth shading.
    vec3 marching_cubes_grad(const int i, const int j, const int k,
                             const int edge_num, const double *points,
                             const double cutoff,
                             const vector<vector<vector<vec3>>> &grads);
};
