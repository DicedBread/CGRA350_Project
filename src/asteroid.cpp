
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
    const static double MC_CUTOFF = 0.5;

    this->shader = shader;

    mesh_builder mb;

    const siv::PerlinNoise perlin{seed};

    int width_of_points = 100;

    // - Generate point cloud -

    std::vector<std::vector<std::vector<double>>> point_cloud(
        width_of_points,
        std::vector<std::vector<double>>(
            width_of_points, std::vector<double>(width_of_points, 0.0)));

    for (int i = 0; i < width_of_points; i++) {
        for (int j = 0; j < width_of_points; j++) {
            for (int k = 0; k < width_of_points; k++) {
                double x = i - (double)width_of_points / 2;
                double y = j - (double)width_of_points / 2;
                double z = k - (double)width_of_points / 2;

                double d =
                    perlin.octave3D_01(x / width_of_points, y / width_of_points,
                                       z / width_of_points, 5);

                // This shapes the noise into a sphere.
                double dist = sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
                d *= -pow(2 * dist / width_of_points, 2) + 1;

                point_cloud[i][j][k] = d;
            }
        }
    }

    // - Generate mesh from point cloud -

    /*
     *   2-----3
     *  /|    /|
     * 6-----7 |
     * | 0---|-1
     * |/    |/
     * 4-----5
     */

    int vert_index = 0;

    for (int x = 0; x < width_of_points - 1; x++) {
        for (int y = 0; y < width_of_points - 1; y++) {
            for (int z = 0; z < width_of_points - 1; z++) {
                double points[] = {point_cloud[x][y][z],
                                   point_cloud[x + 1][y][z],
                                   point_cloud[x][y + 1][z],
                                   point_cloud[x + 1][y + 1][z],
                                   point_cloud[x][y][z + 1],
                                   point_cloud[x + 1][y][z + 1],
                                   point_cloud[x][y + 1][z + 1],
                                   point_cloud[x + 1][y + 1][z + 1]};

                int mc_case = (points[0] > MC_CUTOFF ? (1 << 0) : 0) +
                              (points[1] > MC_CUTOFF ? (1 << 1) : 0) +
                              (points[2] > MC_CUTOFF ? (1 << 2) : 0) +
                              (points[3] > MC_CUTOFF ? (1 << 3) : 0) +
                              (points[4] > MC_CUTOFF ? (1 << 4) : 0) +
                              (points[5] > MC_CUTOFF ? (1 << 5) : 0) +
                              (points[6] > MC_CUTOFF ? (1 << 6) : 0) +
                              (points[7] > MC_CUTOFF ? (1 << 7) : 0);

                vec3 position =
                    vec3(x - width_of_points / 2, y - width_of_points / 2,
                         z - width_of_points / 2);

                switch (mc_case) {
                case 1 << 0:
                    mb.push_index(vert_index++);
                    mb.push_vertex(mesh_vertex{position + vec3(0.5, 0, 0),
                                               normalize(vec3(1, 1, 1)),
                                               vec2(0, 0)});
                    mb.push_index(vert_index++);
                    mb.push_vertex(mesh_vertex{position + vec3(0, 0.5, 0),
                                               normalize(vec3(1, 1, 1)),
                                               vec2(0, 0)});
                    mb.push_index(vert_index++);
                    mb.push_vertex(mesh_vertex{position + vec3(0, 0, 0.5),
                                               normalize(vec3(1, 1, 1)),
                                               vec2(0, 0)});
                    break;
                case 1 << 1:
                    mb.push_index(vert_index++);
                    mb.push_vertex(mesh_vertex{position + vec3(1, 0.5, 0),
                                               normalize(vec3(-1, 1, 1)),
                                               vec2(0, 0)});
                    mb.push_index(vert_index++);
                    mb.push_vertex(mesh_vertex{position + vec3(0.5, 0, 0),
                                               normalize(vec3(-1, 1, 1)),
                                               vec2(0, 0)});
                    mb.push_index(vert_index++);
                    mb.push_vertex(mesh_vertex{position + vec3(1, 0, 0.5),
                                               normalize(vec3(-1, 1, 1)),
                                               vec2(0, 0)});
                    break;
                case 1 << 2:
                    mb.push_index(vert_index++);
                    mb.push_vertex(mesh_vertex{position + vec3(0.5, 1, 0),
                                               normalize(vec3(1, -1, 1)),
                                               vec2(0, 0)});
                    mb.push_index(vert_index++);
                    mb.push_vertex(mesh_vertex{position + vec3(0, 1, 0.5),
                                               normalize(vec3(1, -1, 1)),
                                               vec2(0, 0)});
                    mb.push_index(vert_index++);
                    mb.push_vertex(mesh_vertex{position + vec3(0, 0.5, 0),
                                               normalize(vec3(1, -1, 1)),
                                               vec2(0, 0)});
                    break;
                case 1 << 3:
                    mb.push_index(vert_index++);
                    mb.push_vertex(mesh_vertex{position + vec3(0.5, 1, 0),
                                               normalize(vec3(-1, -1, 1)),
                                               vec2(0, 0)});
                    mb.push_index(vert_index++);
                    mb.push_vertex(mesh_vertex{position + vec3(1, 0.5, 0),
                                               normalize(vec3(-1, -1, 1)),
                                               vec2(0, 0)});
                    mb.push_index(vert_index++);
                    mb.push_vertex(mesh_vertex{position + vec3(1, 1, 0.5),
                                               normalize(vec3(-1, -1, 1)),
                                               vec2(0, 0)});
                    break;
                case 1 << 4:
                    mb.push_index(vert_index++);
                    mb.push_vertex(mesh_vertex{position + vec3(0.5, 0, 1),
                                               normalize(vec3(1, 1, -1)),
                                               vec2(0, 0)});
                    mb.push_index(vert_index++);
                    mb.push_vertex(mesh_vertex{position + vec3(0, 0, 0.5),
                                               normalize(vec3(1, 1, -1)),
                                               vec2(0, 0)});
                    mb.push_index(vert_index++);
                    mb.push_vertex(mesh_vertex{position + vec3(0, 0.5, 1),
                                               normalize(vec3(1, 1, -1)),
                                               vec2(0, 0)});
                    break;
                case 1 << 5:
                    mb.push_index(vert_index++);
                    mb.push_vertex(mesh_vertex{position + vec3(1, 0.5, 1),
                                               normalize(vec3(-1, 1, -1)),
                                               vec2(0, 0)});
                    mb.push_index(vert_index++);
                    mb.push_vertex(mesh_vertex{position + vec3(1, 0, 0.5),
                                               normalize(vec3(-1, 1, -1)),
                                               vec2(0, 0)});
                    mb.push_index(vert_index++);
                    mb.push_vertex(mesh_vertex{position + vec3(0.5, 0, 1),
                                               normalize(vec3(-1, 1, -1)),
                                               vec2(0, 0)});
                    break;
                case 1 << 6:
                    mb.push_index(vert_index++);
                    mb.push_vertex(mesh_vertex{position + vec3(0.5, 1, 1),
                                               normalize(vec3(1, -1, -1)),
                                               vec2(0, 0)});
                    mb.push_index(vert_index++);
                    mb.push_vertex(mesh_vertex{position + vec3(0, 1, 0.5),
                                               normalize(vec3(1, -1, -1)),
                                               vec2(0, 0)});
                    mb.push_index(vert_index++);
                    mb.push_vertex(mesh_vertex{position + vec3(0, 0.5, 1),
                                               normalize(vec3(1, -1, -1)),
                                               vec2(0, 0)});
                    break;
                case 1 << 7:
                    mb.push_index(vert_index++);
                    mb.push_vertex(mesh_vertex{position + vec3(0.5, 1, 1),
                                               normalize(vec3(-1, -1, -1)),
                                               vec2(0, 0)});
                    mb.push_index(vert_index++);
                    mb.push_vertex(mesh_vertex{position + vec3(1, 1, 0.5),
                                               normalize(vec3(-1, -1, -1)),
                                               vec2(0, 0)});
                    mb.push_index(vert_index++);
                    mb.push_vertex(mesh_vertex{position + vec3(1, 0.5, 1),
                                               normalize(vec3(-1, -1, -1)),
                                               vec2(0, 0)});
                    break;
                }
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
