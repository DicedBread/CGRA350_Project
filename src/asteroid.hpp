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

class Asteroid {
  public:
    Asteroid(GLuint shader, const siv::PerlinNoise::seed_type seed);
    const void draw(const glm::mat4 &view, const glm::mat4 proj);

  private:
    cgra::gl_mesh mesh;
    glm::mat4 modelTransform;
    GLuint shader;
    glm::vec3 color;

    // Returns a vec3 containing the offsets from the origin to the vertex
    // withing the given edge number
    const vec3 marching_cubes_edge(int edge_num);

    // Returns a vector of the vertices of the triangles that make up a single
    // case for marching cubes. The returned array will list the verts in
    // triplets, capped off with a -1.
    const int *marching_cubes_tris(int case_num);
};
