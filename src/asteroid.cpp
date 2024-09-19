
// std
#include <chrono>
#include <cmath>
#include <glm/geometric.hpp>
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
#include "cgra/cgra_mesh.hpp"
#include "cgra/cgra_shader.hpp"
#include "cgra/cgra_wavefront.hpp"

// perlin noise
#include "PerlinNoise.hpp"

// header
#include "asteroid.hpp"

using namespace std;
using namespace cgra;
using namespace glm;

Asteroid::Asteroid(GLuint shader, const siv::PerlinNoise::seed_type seed) {
    this->shader = shader;

    mesh_builder mb;

    const siv::PerlinNoise perlin{seed};

    int index = 0;

    int width_on_points = 100;

    std::vector<double> point_cloud;

    for (int i = -width_on_points / 2; i < width_on_points / 2; i++) {
        for (int j = -width_on_points / 2; j < width_on_points / 2; j++) {
            for (int k = -width_on_points / 2; k < width_on_points / 2; k++) {
                double x = (float)i;
                double y = (float)j;
                double z = (float)k;

                double d =
                    perlin.octave3D_01(x / width_on_points, y / width_on_points,
                                       z / width_on_points, 5);

                // This shapes the noise into a sphere.
                double dist = sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
                d *= -pow(2 * dist / width_on_points, 2) + 1;

                point_cloud.push_back(d);

                if (d < 0.5) {
                    continue;
                }

                mb.push_index(index++);
                mb.push_vertex(mesh_vertex{
                    vec3(x + d, y, z), normalize(vec3(1, 1, 1)), vec2(0, 0)});

                mb.push_index(index++);
                mb.push_vertex(mesh_vertex{
                    vec3(x, y + d, z), normalize(vec3(1, 1, 1)), vec2(0, 0)});

                mb.push_index(index++);
                mb.push_vertex(mesh_vertex{
                    vec3(x, y, z + d), normalize(vec3(1, 1, 1)), vec2(0, 0)});
            }
        }
    }

    this->mesh = mb.build();

    this->color = vec3(1, 0, 0);

    this->modelTransform = glm::scale(mat4(1.0), vec3(0.1));
}

void Asteroid::draw(const glm::mat4 &view, const glm::mat4 proj) {
    mat4 modelview = view * modelTransform;

    glUseProgram(shader); // load shader and variables
    glUniformMatrix4fv(glGetUniformLocation(shader, "uProjectionMatrix"), 1,
                       false, value_ptr(proj));
    glUniformMatrix4fv(glGetUniformLocation(shader, "uModelViewMatrix"), 1,
                       false, value_ptr(modelview));
    glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, value_ptr(color));

    mesh.draw(); // draw
}
