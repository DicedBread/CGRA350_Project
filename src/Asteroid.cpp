
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
#include "opengl.hpp"

// header
#include "Asteroid.hpp"

using namespace std;
using namespace cgra;
using namespace glm;

Asteroid::Asteroid(const siv::PerlinNoise::seed_type seed,
                   AsteroidMeshConfig *asteroidMeshConfig) {
    this->asteroidMeshConfig = asteroidMeshConfig;

    position = vec3(0, 0, 0);
    velocity = vec3(0, 0, 0);
    rotation_axis = vec3(0, 1, 0);
    rotation_angle = 0.0;
    rotation_velocity = 0.0;

    this->load_shader();

    this->regenerate_mesh(seed);

    this->color = vec3(0.5);

    this->modelTransform = glm::scale(mat4(1.0), vec3(0.1));
}

void Asteroid::regenerate_mesh(const siv::PerlinNoise::seed_type seed) {
    mesh_builder mb;

    const siv::PerlinNoise perlin{seed};

    int width_of_points = asteroidMeshConfig->num_verts;

    // - Generate point cloud -

    std::vector<std::vector<std::vector<double>>> point_cloud(
        width_of_points,
        std::vector<std::vector<double>>(
            width_of_points, std::vector<double>(width_of_points, 0.0)));

    std::vector<std::vector<std::vector<vec3>>> point_cloud_grads(
        width_of_points,
        std::vector<std::vector<vec3>>(
            width_of_points,
            std::vector<vec3>(width_of_points, vec3(0, 0, 0))));

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

    // -- Calculate the point cloud center --

    // This calculated center will be used when generating the mesh.
    // Is is the average position of all points avove the cutoff.
    vec3 center = vec3(0, 0, 0);
    int num_points = 0;

    for (int i = 0; i < width_of_points; i++) {
        for (int j = 0; j < width_of_points; j++) {
            for (int k = 0; k < width_of_points; k++) {
                if (point_cloud[i][j][k] > asteroidMeshConfig->cutoff) {
                    double x = (i - (double)width_of_points / 2) *
                               asteroidMeshConfig->edge_length;
                    double y = (j - (double)width_of_points / 2) *
                               asteroidMeshConfig->edge_length;
                    double z = (k - (double)width_of_points / 2) *
                               asteroidMeshConfig->edge_length;

                    center += vec3(x, y, z);
                    num_points++;
                }
            }
        }
    }

    center /= (num_points > 0 ? num_points : 1);

    // -- Calculate gradients --

    for (int i = 0; i < width_of_points; i++) {
        for (int j = 0; j < width_of_points; j++) {
            for (int k = 0; k < width_of_points; k++) {
                // The edge case, literally.
                if (i == 0 || i == width_of_points - 1 || j == 0 ||
                    j == width_of_points - 1 || k == 0 ||
                    k == width_of_points - 1) {
                    point_cloud_grads[i][j][k] = vec3(0, 0, 0);
                    continue;
                }

                float Gx =
                    (point_cloud[i + 1][j][k] - point_cloud[i - 1][j][k]) / 2 *
                    asteroidMeshConfig->edge_length;
                float Gy =
                    (point_cloud[i][j + 1][k] - point_cloud[i][j - 1][k]) / 2 *
                    asteroidMeshConfig->edge_length;
                float Gz =
                    (point_cloud[i][j][k + 1] - point_cloud[i][j][k - 1]) / 2 *
                    asteroidMeshConfig->edge_length;

                point_cloud_grads[i][j][k] = vec3(Gx, Gy, Gz);
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

                int mc_case =
                    (points[0] > asteroidMeshConfig->cutoff ? (1 << 0) : 0) +
                    (points[1] > asteroidMeshConfig->cutoff ? (1 << 1) : 0) +
                    (points[2] > asteroidMeshConfig->cutoff ? (1 << 2) : 0) +
                    (points[3] > asteroidMeshConfig->cutoff ? (1 << 3) : 0) +
                    (points[4] > asteroidMeshConfig->cutoff ? (1 << 4) : 0) +
                    (points[5] > asteroidMeshConfig->cutoff ? (1 << 5) : 0) +
                    (points[6] > asteroidMeshConfig->cutoff ? (1 << 6) : 0) +
                    (points[7] > asteroidMeshConfig->cutoff ? (1 << 7) : 0);

                vec3 position =
                    (float)asteroidMeshConfig->edge_length *
                        vec3(x - width_of_points / 2, y - width_of_points / 2,
                             z - width_of_points / 2) -
                    center;

                const int *tris = marching_cubes_tris(mc_case);
                int tri_index = 0;

                while (tris[tri_index] != -1) {
                    vec3 vert0 =
                        this->marching_cubes_edge(tris[tri_index + 0], points,
                                                  asteroidMeshConfig->cutoff);
                    vec3 vert1 =
                        this->marching_cubes_edge(tris[tri_index + 1], points,
                                                  asteroidMeshConfig->cutoff);
                    vec3 vert2 =
                        this->marching_cubes_edge(tris[tri_index + 2], points,
                                                  asteroidMeshConfig->cutoff);

                    // Norm of each edge is the average of the gradients of the
                    // points either side of the edge.
                    vec3 norm0 = normalize(marching_cubes_grad(
                        x, y, z, tris[tri_index + 0], points,
                        asteroidMeshConfig->cutoff, point_cloud_grads));
                    vec3 norm1 = normalize(marching_cubes_grad(
                        x, y, z, tris[tri_index + 1], points,
                        asteroidMeshConfig->cutoff, point_cloud_grads));
                    vec3 norm2 = normalize(marching_cubes_grad(
                        x, y, z, tris[tri_index + 2], points,
                        asteroidMeshConfig->cutoff, point_cloud_grads));

                    mb.push_index(vert_index++);
                    mb.push_vertex(mesh_vertex{
                        position +
                            (float)asteroidMeshConfig->edge_length * vert0,
                        -norm0, vec2(0, 0)});

                    mb.push_index(vert_index++);
                    mb.push_vertex(mesh_vertex{
                        position +
                            (float)asteroidMeshConfig->edge_length * vert1,
                        -norm1, vec2(0, 0)});

                    mb.push_index(vert_index++);
                    mb.push_vertex(mesh_vertex{
                        position +
                            (float)asteroidMeshConfig->edge_length * vert2,
                        -norm2, vec2(0, 0)});

                    tri_index += 3;
                }

                free((void *)tris);
            }
        }
    }

    this->mesh = mb.build();
}

void Asteroid::draw(const glm::mat4 &view, const glm::mat4 proj,
                    double deformation, double greenCov) {
    mat4 modelview = view * modelTransform;

    glUseProgram(shader); // load shader and variables
    glUniformMatrix4fv(glGetUniformLocation(shader, "uProjectionMatrix"), 1,
                       false, value_ptr(proj));
    glUniformMatrix4fv(glGetUniformLocation(shader, "uModelViewMatrix"), 1,
                       false, value_ptr(modelview));
    glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, value_ptr(color));

    glUniform1f(glGetUniformLocation(shader, "uRoughness"), 1.0);
    glUniform1f(glGetUniformLocation(shader, "uE_0"), 5.0);

    int is_deformation = fabs(deformation) > 1E-3 ? 1 : 0;
    glUniform1i(glGetUniformLocation(shader, "uIsDeformation"), is_deformation);
    glUniform1f(glGetUniformLocation(shader, "uDeformation"), deformation);

    glUniform1f(glGetUniformLocation(shader, "uCovDensity"), greenCov);

    mesh.draw(); // draw
}

vec3 Asteroid::marching_cubes_edge(const int edge_num, const double *points,
                                   const double cutoff) {
    switch (edge_num) {
    default:
    case 0:
        return vec3(inverse_lerp(points[0], points[1], cutoff), 0, 0);
    case 1:
        return vec3(1, inverse_lerp(points[1], points[3], cutoff), 0);
    case 2:
        return vec3(inverse_lerp(points[2], points[3], cutoff), 1, 0);
    case 3:
        return vec3(0, inverse_lerp(points[0], points[2], cutoff), 0);
    case 4:
        return vec3(inverse_lerp(points[4], points[5], cutoff), 0, 1);
    case 5:
        return vec3(1, inverse_lerp(points[5], points[7], cutoff), 1);
    case 6:
        return vec3(inverse_lerp(points[6], points[7], cutoff), 1, 1);
    case 7:
        return vec3(0, inverse_lerp(points[4], points[6], cutoff), 1);
    case 8:
        return vec3(0, 0, inverse_lerp(points[0], points[4], cutoff));
    case 9:
        return vec3(1, 0, inverse_lerp(points[1], points[5], cutoff));
    case 10:
        return vec3(1, 1, inverse_lerp(points[3], points[7], cutoff));
    case 11:
        return vec3(0, 1, inverse_lerp(points[2], points[6], cutoff));
    }
}

int *Asteroid::marching_cubes_tris(const int case_num) {
    /*     +---2---+
     *    /|      /|
     *  11 3    10 1
     *  /  |    /  |
     * +---6---+-0-+
     * |  /    |  /
     * 7 8     5 9
     * |/      |/
     * +---4---+
     *
     */
    switch (case_num) {
    default:
    case 0:
        return new int[1]{-1};
    case 1:
        return new int[4]{0, 3, 8, -1};
    case 2:
        return new int[4]{0, 9, 1, -1};
    case 3:
        return new int[7]{3, 8, 1, 1, 8, 9, -1};
    case 4:
        return new int[4]{2, 11, 3, -1};
    case 5:
        return new int[7]{8, 0, 11, 11, 0, 2, -1};
    case 6:
        return new int[7]{3, 2, 11, 1, 0, 9, -1};
    case 7:
        return new int[10]{11, 1, 2, 11, 9, 1, 11, 8, 9, -1};
    case 8:
        return new int[4]{1, 10, 2, -1};
    case 9:
        return new int[7]{0, 3, 8, 2, 1, 10, -1};
    case 10:
        return new int[7]{10, 2, 9, 9, 2, 0, -1};
    case 11:
        return new int[10]{8, 2, 3, 8, 10, 2, 8, 9, 10, -1};
    case 12:
        return new int[7]{11, 3, 10, 10, 3, 1, -1};
    case 13:
        return new int[10]{10, 0, 1, 10, 8, 0, 10, 11, 8, -1};
    case 14:
        return new int[10]{9, 3, 0, 9, 11, 3, 9, 10, 11, -1};
    case 15:
        return new int[7]{8, 9, 11, 11, 9, 10, -1};
    case 16:
        return new int[4]{4, 8, 7, -1};
    case 17:
        return new int[7]{7, 4, 3, 3, 4, 0, -1};
    case 18:
        return new int[7]{4, 8, 7, 0, 9, 1, -1};
    case 19:
        return new int[10]{1, 4, 9, 1, 7, 4, 1, 3, 7, -1};
    case 20:
        return new int[7]{8, 7, 4, 11, 3, 2, -1};
    case 21:
        return new int[10]{4, 11, 7, 4, 2, 11, 4, 0, 2, -1};
    case 22:
        return new int[10]{0, 9, 1, 8, 7, 4, 11, 3, 2, -1};
    case 23:
        return new int[13]{7, 4, 11, 11, 4, 2, 2, 4, 9, 2, 9, 1, -1};
    case 24:
        return new int[7]{4, 8, 7, 2, 1, 10, -1};
    case 25:
        return new int[10]{7, 4, 3, 3, 4, 0, 10, 2, 1, -1};
    case 26:
        return new int[10]{10, 2, 9, 9, 2, 0, 7, 4, 8, -1};
    case 27:
        return new int[13]{10, 2, 3, 10, 3, 4, 3, 7, 4, 9, 10, 4, -1};
    case 28:
        return new int[10]{1, 10, 3, 3, 10, 11, 4, 8, 7, -1};
    case 29:
        return new int[13]{10, 11, 1, 11, 7, 4, 1, 11, 4, 1, 4, 0, -1};
    case 30:
        return new int[13]{7, 4, 8, 9, 3, 0, 9, 11, 3, 9, 10, 11, -1};
    case 31:
        return new int[10]{7, 4, 11, 4, 9, 11, 9, 10, 11, -1};
    case 32:
        return new int[4]{9, 4, 5, -1};
    case 33:
        return new int[7]{9, 4, 5, 8, 0, 3, -1};
    case 34:
        return new int[7]{4, 5, 0, 0, 5, 1, -1};
    case 35:
        return new int[10]{5, 8, 4, 5, 3, 8, 5, 1, 3, -1};
    case 36:
        return new int[7]{9, 4, 5, 11, 3, 2, -1};
    case 37:
        return new int[10]{2, 11, 0, 0, 11, 8, 5, 9, 4, -1};
    case 38:
        return new int[10]{4, 5, 0, 0, 5, 1, 11, 3, 2, -1};
    case 39:
        return new int[13]{5, 1, 4, 1, 2, 11, 4, 1, 11, 4, 11, 8, -1};
    case 40:
        return new int[7]{1, 10, 2, 5, 9, 4, -1};
    case 41:
        return new int[10]{9, 4, 5, 0, 3, 8, 2, 1, 10, -1};
    case 42:
        return new int[10]{2, 5, 10, 2, 4, 5, 2, 0, 4, -1};
    case 43:
        return new int[13]{10, 2, 5, 5, 2, 4, 4, 2, 3, 4, 3, 8, -1};
    case 44:
        return new int[10]{11, 3, 10, 10, 3, 1, 4, 5, 9, -1};
    case 45:
        return new int[13]{4, 5, 9, 10, 0, 1, 10, 8, 0, 10, 11, 8, -1};
    case 46:
        return new int[13]{11, 3, 0, 11, 0, 5, 0, 4, 5, 10, 11, 5, -1};
    case 47:
        return new int[10]{4, 5, 8, 5, 10, 8, 10, 11, 8, -1};
    case 48:
        return new int[7]{8, 7, 9, 9, 7, 5, -1};
    case 49:
        return new int[10]{3, 9, 0, 3, 5, 9, 3, 7, 5, -1};
    case 50:
        return new int[10]{7, 0, 8, 7, 1, 0, 7, 5, 1, -1};
    case 51:
        return new int[7]{7, 5, 3, 3, 5, 1, -1};
    case 52:
        return new int[10]{5, 9, 7, 7, 9, 8, 2, 11, 3, -1};
    case 53:
        return new int[13]{2, 11, 7, 2, 7, 9, 7, 5, 9, 0, 2, 9, -1};
    case 54:
        return new int[13]{2, 11, 3, 7, 0, 8, 7, 1, 0, 7, 5, 1, -1};
    case 55:
        return new int[10]{2, 11, 1, 11, 7, 1, 7, 5, 1, -1};
    case 56:
        return new int[10]{8, 7, 9, 9, 7, 5, 2, 1, 10, -1};
    case 57:
        return new int[13]{10, 2, 1, 3, 9, 0, 3, 5, 9, 3, 7, 5, -1};
    case 58:
        return new int[13]{7, 5, 8, 5, 10, 2, 8, 5, 2, 8, 2, 0, -1};
    case 59:
        return new int[10]{10, 2, 5, 2, 3, 5, 3, 7, 5, -1};
    case 60:
        return new int[13]{8, 7, 5, 8, 5, 9, 11, 3, 10, 3, 1, 10, -1};
    case 61:
        return new int[10]{5, 11, 7, 10, 11, 5, 1, 9, 0, -1};
    case 62:
        return new int[10]{11, 5, 10, 7, 5, 11, 8, 3, 0, -1};
    case 63:
        return new int[7]{5, 11, 7, 10, 11, 5, -1};
    case 64:
        return new int[4]{6, 7, 11, -1};
    case 65:
        return new int[7]{7, 11, 6, 3, 8, 0, -1};
    case 66:
        return new int[7]{6, 7, 11, 0, 9, 1, -1};
    case 67:
        return new int[10]{9, 1, 8, 8, 1, 3, 6, 7, 11, -1};
    case 68:
        return new int[7]{3, 2, 7, 7, 2, 6, -1};
    case 69:
        return new int[10]{0, 7, 8, 0, 6, 7, 0, 2, 6, -1};
    case 70:
        return new int[10]{6, 7, 2, 2, 7, 3, 9, 1, 0, -1};
    case 71:
        return new int[13]{6, 7, 8, 6, 8, 1, 8, 9, 1, 2, 6, 1, -1};
    case 72:
        return new int[7]{11, 6, 7, 10, 2, 1, -1};
    case 73:
        return new int[10]{3, 8, 0, 11, 6, 7, 10, 2, 1, -1};
    case 74:
        return new int[10]{0, 9, 2, 2, 9, 10, 7, 11, 6, -1};
    case 75:
        return new int[13]{6, 7, 11, 8, 2, 3, 8, 10, 2, 8, 9, 10, -1};
    case 76:
        return new int[10]{7, 10, 6, 7, 1, 10, 7, 3, 1, -1};
    case 77:
        return new int[13]{8, 0, 7, 7, 0, 6, 6, 0, 1, 6, 1, 10, -1};
    case 78:
        return new int[13]{7, 3, 6, 3, 0, 9, 6, 3, 9, 6, 9, 10, -1};
    case 79:
        return new int[10]{6, 7, 10, 7, 8, 10, 8, 9, 10, -1};
    case 80:
        return new int[7]{11, 6, 8, 8, 6, 4, -1};
    case 81:
        return new int[10]{6, 3, 11, 6, 0, 3, 6, 4, 0, -1};
    case 82:
        return new int[10]{11, 6, 8, 8, 6, 4, 1, 0, 9, -1};
    case 83:
        return new int[13]{1, 3, 9, 3, 11, 6, 9, 3, 6, 9, 6, 4, -1};
    case 84:
        return new int[10]{2, 8, 3, 2, 4, 8, 2, 6, 4, -1};
    case 85:
        return new int[7]{4, 0, 6, 6, 0, 2, -1};
    case 86:
        return new int[13]{9, 1, 0, 2, 8, 3, 2, 4, 8, 2, 6, 4, -1};
    case 87:
        return new int[10]{9, 1, 4, 1, 2, 4, 2, 6, 4, -1};
    case 88:
        return new int[10]{4, 8, 6, 6, 8, 11, 1, 10, 2, -1};
    case 89:
        return new int[13]{1, 10, 2, 6, 3, 11, 6, 0, 3, 6, 4, 0, -1};
    case 90:
        return new int[13]{11, 6, 4, 11, 4, 8, 10, 2, 9, 2, 0, 9, -1};
    case 91:
        return new int[10]{10, 4, 9, 6, 4, 10, 11, 2, 3, -1};
    case 92:
        return new int[13]{4, 8, 3, 4, 3, 10, 3, 1, 10, 6, 4, 10, -1};
    case 93:
        return new int[10]{1, 10, 0, 10, 6, 0, 6, 4, 0, -1};
    case 94:
        return new int[10]{4, 10, 6, 9, 10, 4, 0, 8, 3, -1};
    case 95:
        return new int[7]{4, 10, 6, 9, 10, 4, -1};
    case 96:
        return new int[7]{6, 7, 11, 4, 5, 9, -1};
    case 97:
        return new int[10]{4, 5, 9, 7, 11, 6, 3, 8, 0, -1};
    case 98:
        return new int[10]{1, 0, 5, 5, 0, 4, 11, 6, 7, -1};
    case 99:
        return new int[13]{11, 6, 7, 5, 8, 4, 5, 3, 8, 5, 1, 3, -1};
    case 100:
        return new int[10]{3, 2, 7, 7, 2, 6, 9, 4, 5, -1};
    case 101:
        return new int[13]{5, 9, 4, 0, 7, 8, 0, 6, 7, 0, 2, 6, -1};
    case 102:
        return new int[13]{3, 2, 6, 3, 6, 7, 1, 0, 5, 0, 4, 5, -1};
    case 103:
        return new int[10]{6, 1, 2, 5, 1, 6, 4, 7, 8, -1};
    case 104:
        return new int[10]{10, 2, 1, 6, 7, 11, 4, 5, 9, -1};
    case 105:
        return new int[13]{0, 3, 8, 4, 5, 9, 11, 6, 7, 10, 2, 1, -1};
    case 106:
        return new int[13]{7, 11, 6, 2, 5, 10, 2, 4, 5, 2, 0, 4, -1};
    case 107:
        return new int[10]{8, 4, 7, 5, 10, 6, 3, 11, 2, -1};
    case 108:
        return new int[13]{9, 4, 5, 7, 10, 6, 7, 1, 10, 7, 3, 1, -1};
    case 109:
        return new int[10]{10, 6, 5, 7, 8, 4, 1, 9, 0, -1};
    case 110:
        return new int[10]{4, 3, 0, 7, 3, 4, 6, 5, 10, -1};
    case 111:
        return new int[7]{10, 6, 5, 8, 4, 7, -1};
    case 112:
        return new int[10]{9, 6, 5, 9, 11, 6, 9, 8, 11, -1};
    case 113:
        return new int[13]{11, 6, 3, 3, 6, 0, 0, 6, 5, 0, 5, 9, -1};
    case 114:
        return new int[13]{11, 6, 5, 11, 5, 0, 5, 1, 0, 8, 11, 0, -1};
    case 115:
        return new int[10]{11, 6, 3, 6, 5, 3, 5, 1, 3, -1};
    case 116:
        return new int[13]{9, 8, 5, 8, 3, 2, 5, 8, 2, 5, 2, 6, -1};
    case 117:
        return new int[10]{5, 9, 6, 9, 0, 6, 0, 2, 6, -1};
    case 118:
        return new int[10]{1, 6, 5, 2, 6, 1, 3, 0, 8, -1};
    case 119:
        return new int[7]{1, 6, 5, 2, 6, 1, -1};
    case 120:
        return new int[13]{2, 1, 10, 9, 6, 5, 9, 11, 6, 9, 8, 11, -1};
    case 121:
        return new int[10]{9, 0, 1, 3, 11, 2, 5, 10, 6, -1};
    case 122:
        return new int[10]{11, 0, 8, 2, 0, 11, 10, 6, 5, -1};
    case 123:
        return new int[7]{3, 11, 2, 5, 10, 6, -1};
    case 124:
        return new int[10]{1, 8, 3, 9, 8, 1, 5, 10, 6, -1};
    case 125:
        return new int[7]{6, 5, 10, 0, 1, 9, -1};
    case 126:
        return new int[7]{8, 3, 0, 5, 10, 6, -1};
    case 127:
        return new int[4]{6, 5, 10, -1};
    case 128:
        return new int[4]{10, 5, 6, -1};
    case 129:
        return new int[7]{0, 3, 8, 6, 10, 5, -1};
    case 130:
        return new int[7]{10, 5, 6, 9, 1, 0, -1};
    case 131:
        return new int[10]{3, 8, 1, 1, 8, 9, 6, 10, 5, -1};
    case 132:
        return new int[7]{2, 11, 3, 6, 10, 5, -1};
    case 133:
        return new int[10]{8, 0, 11, 11, 0, 2, 5, 6, 10, -1};
    case 134:
        return new int[10]{1, 0, 9, 2, 11, 3, 6, 10, 5, -1};
    case 135:
        return new int[13]{5, 6, 10, 11, 1, 2, 11, 9, 1, 11, 8, 9, -1};
    case 136:
        return new int[7]{5, 6, 1, 1, 6, 2, -1};
    case 137:
        return new int[10]{5, 6, 1, 1, 6, 2, 8, 0, 3, -1};
    case 138:
        return new int[10]{6, 9, 5, 6, 0, 9, 6, 2, 0, -1};
    case 139:
        return new int[13]{6, 2, 5, 2, 3, 8, 5, 2, 8, 5, 8, 9, -1};
    case 140:
        return new int[10]{3, 6, 11, 3, 5, 6, 3, 1, 5, -1};
    case 141:
        return new int[13]{8, 0, 1, 8, 1, 6, 1, 5, 6, 11, 8, 6, -1};
    case 142:
        return new int[13]{11, 3, 6, 6, 3, 5, 5, 3, 0, 5, 0, 9, -1};
    case 143:
        return new int[10]{5, 6, 9, 6, 11, 9, 11, 8, 9, -1};
    case 144:
        return new int[7]{5, 6, 10, 7, 4, 8, -1};
    case 145:
        return new int[10]{0, 3, 4, 4, 3, 7, 10, 5, 6, -1};
    case 146:
        return new int[10]{5, 6, 10, 4, 8, 7, 0, 9, 1, -1};
    case 147:
        return new int[13]{6, 10, 5, 1, 4, 9, 1, 7, 4, 1, 3, 7, -1};
    case 148:
        return new int[10]{7, 4, 8, 6, 10, 5, 2, 11, 3, -1};
    case 149:
        return new int[13]{10, 5, 6, 4, 11, 7, 4, 2, 11, 4, 0, 2, -1};
    case 150:
        return new int[13]{4, 8, 7, 6, 10, 5, 3, 2, 11, 1, 0, 9, -1};
    case 151:
        return new int[10]{1, 2, 10, 11, 7, 6, 9, 5, 4, -1};
    case 152:
        return new int[10]{2, 1, 6, 6, 1, 5, 8, 7, 4, -1};
    case 153:
        return new int[13]{0, 3, 7, 0, 7, 4, 2, 1, 6, 1, 5, 6, -1};
    case 154:
        return new int[13]{8, 7, 4, 6, 9, 5, 6, 0, 9, 6, 2, 0, -1};
    case 155:
        return new int[10]{7, 2, 3, 6, 2, 7, 5, 4, 9, -1};
    case 156:
        return new int[13]{4, 8, 7, 3, 6, 11, 3, 5, 6, 3, 1, 5, -1};
    case 157:
        return new int[10]{5, 0, 1, 4, 0, 5, 7, 6, 11, -1};
    case 158:
        return new int[10]{9, 5, 4, 6, 11, 7, 0, 8, 3, -1};
    case 159:
        return new int[7]{11, 7, 6, 9, 5, 4, -1};
    case 160:
        return new int[7]{6, 10, 4, 4, 10, 9, -1};
    case 161:
        return new int[10]{6, 10, 4, 4, 10, 9, 3, 8, 0, -1};
    case 162:
        return new int[10]{0, 10, 1, 0, 6, 10, 0, 4, 6, -1};
    case 163:
        return new int[13]{6, 10, 1, 6, 1, 8, 1, 3, 8, 4, 6, 8, -1};
    case 164:
        return new int[10]{9, 4, 10, 10, 4, 6, 3, 2, 11, -1};
    case 165:
        return new int[13]{2, 11, 8, 2, 8, 0, 6, 10, 4, 10, 9, 4, -1};
    case 166:
        return new int[13]{11, 3, 2, 0, 10, 1, 0, 6, 10, 0, 4, 6, -1};
    case 167:
        return new int[10]{6, 8, 4, 11, 8, 6, 2, 10, 1, -1};
    case 168:
        return new int[10]{4, 1, 9, 4, 2, 1, 4, 6, 2, -1};
    case 169:
        return new int[13]{3, 8, 0, 4, 1, 9, 4, 2, 1, 4, 6, 2, -1};
    case 170:
        return new int[7]{6, 2, 4, 4, 2, 0, -1};
    case 171:
        return new int[10]{3, 8, 2, 8, 4, 2, 4, 6, 2, -1};
    case 172:
        return new int[13]{4, 6, 9, 6, 11, 3, 9, 6, 3, 9, 3, 1, -1};
    case 173:
        return new int[10]{8, 6, 11, 4, 6, 8, 9, 0, 1, -1};
    case 174:
        return new int[10]{11, 3, 6, 3, 0, 6, 0, 4, 6, -1};
    case 175:
        return new int[7]{8, 6, 11, 4, 6, 8, -1};
    case 176:
        return new int[10]{10, 7, 6, 10, 8, 7, 10, 9, 8, -1};
    case 177:
        return new int[13]{3, 7, 0, 7, 6, 10, 0, 7, 10, 0, 10, 9, -1};
    case 178:
        return new int[13]{6, 10, 7, 7, 10, 8, 8, 10, 1, 8, 1, 0, -1};
    case 179:
        return new int[10]{6, 10, 7, 10, 1, 7, 1, 3, 7, -1};
    case 180:
        return new int[13]{3, 2, 11, 10, 7, 6, 10, 8, 7, 10, 9, 8, -1};
    case 181:
        return new int[10]{2, 9, 0, 10, 9, 2, 6, 11, 7, -1};
    case 182:
        return new int[10]{0, 8, 3, 7, 6, 11, 1, 2, 10, -1};
    case 183:
        return new int[7]{7, 6, 11, 1, 2, 10, -1};
    case 184:
        return new int[13]{2, 1, 9, 2, 9, 7, 9, 8, 7, 6, 2, 7, -1};
    case 185:
        return new int[10]{2, 7, 6, 3, 7, 2, 0, 1, 9, -1};
    case 186:
        return new int[10]{8, 7, 0, 7, 6, 0, 6, 2, 0, -1};
    case 187:
        return new int[7]{7, 2, 3, 6, 2, 7, -1};
    case 188:
        return new int[10]{8, 1, 9, 3, 1, 8, 11, 7, 6, -1};
    case 189:
        return new int[7]{11, 7, 6, 1, 9, 0, -1};
    case 190:
        return new int[7]{6, 11, 7, 0, 8, 3, -1};
    case 191:
        return new int[4]{11, 7, 6, -1};
    case 192:
        return new int[7]{7, 11, 5, 5, 11, 10, -1};
    case 193:
        return new int[10]{10, 5, 11, 11, 5, 7, 0, 3, 8, -1};
    case 194:
        return new int[10]{7, 11, 5, 5, 11, 10, 0, 9, 1, -1};
    case 195:
        return new int[13]{7, 11, 10, 7, 10, 5, 3, 8, 1, 8, 9, 1, -1};
    case 196:
        return new int[10]{5, 2, 10, 5, 3, 2, 5, 7, 3, -1};
    case 197:
        return new int[13]{5, 7, 10, 7, 8, 0, 10, 7, 0, 10, 0, 2, -1};
    case 198:
        return new int[13]{0, 9, 1, 5, 2, 10, 5, 3, 2, 5, 7, 3, -1};
    case 199:
        return new int[10]{9, 7, 8, 5, 7, 9, 10, 1, 2, -1};
    case 200:
        return new int[10]{1, 11, 2, 1, 7, 11, 1, 5, 7, -1};
    case 201:
        return new int[13]{8, 0, 3, 1, 11, 2, 1, 7, 11, 1, 5, 7, -1};
    case 202:
        return new int[13]{7, 11, 2, 7, 2, 9, 2, 0, 9, 5, 7, 9, -1};
    case 203:
        return new int[10]{7, 9, 5, 8, 9, 7, 3, 11, 2, -1};
    case 204:
        return new int[7]{3, 1, 7, 7, 1, 5, -1};
    case 205:
        return new int[10]{8, 0, 7, 0, 1, 7, 1, 5, 7, -1};
    case 206:
        return new int[10]{0, 9, 3, 9, 5, 3, 5, 7, 3, -1};
    case 207:
        return new int[7]{9, 7, 8, 5, 7, 9, -1};
    case 208:
        return new int[10]{8, 5, 4, 8, 10, 5, 8, 11, 10, -1};
    case 209:
        return new int[13]{0, 3, 11, 0, 11, 5, 11, 10, 5, 4, 0, 5, -1};
    case 210:
        return new int[13]{1, 0, 9, 8, 5, 4, 8, 10, 5, 8, 11, 10, -1};
    case 211:
        return new int[10]{10, 3, 11, 1, 3, 10, 9, 5, 4, -1};
    case 212:
        return new int[13]{3, 2, 8, 8, 2, 4, 4, 2, 10, 4, 10, 5, -1};
    case 213:
        return new int[10]{10, 5, 2, 5, 4, 2, 4, 0, 2, -1};
    case 214:
        return new int[10]{5, 4, 9, 8, 3, 0, 10, 1, 2, -1};
    case 215:
        return new int[7]{2, 10, 1, 4, 9, 5, -1};
    case 216:
        return new int[13]{8, 11, 4, 11, 2, 1, 4, 11, 1, 4, 1, 5, -1};
    case 217:
        return new int[10]{0, 5, 4, 1, 5, 0, 2, 3, 11, -1};
    case 218:
        return new int[10]{0, 11, 2, 8, 11, 0, 4, 9, 5, -1};
    case 219:
        return new int[7]{5, 4, 9, 2, 3, 11, -1};
    case 220:
        return new int[10]{4, 8, 5, 8, 3, 5, 3, 1, 5, -1};
    case 221:
        return new int[7]{0, 5, 4, 1, 5, 0, -1};
    case 222:
        return new int[7]{5, 4, 9, 3, 0, 8, -1};
    case 223:
        return new int[4]{5, 4, 9, -1};
    case 224:
        return new int[10]{11, 4, 7, 11, 9, 4, 11, 10, 9, -1};
    case 225:
        return new int[13]{0, 3, 8, 11, 4, 7, 11, 9, 4, 11, 10, 9, -1};
    case 226:
        return new int[13]{11, 10, 7, 10, 1, 0, 7, 10, 0, 7, 0, 4, -1};
    case 227:
        return new int[10]{3, 10, 1, 11, 10, 3, 7, 8, 4, -1};
    case 228:
        return new int[13]{3, 2, 10, 3, 10, 4, 10, 9, 4, 7, 3, 4, -1};
    case 229:
        return new int[10]{9, 2, 10, 0, 2, 9, 8, 4, 7, -1};
    case 230:
        return new int[10]{3, 4, 7, 0, 4, 3, 1, 2, 10, -1};
    case 231:
        return new int[7]{7, 8, 4, 10, 1, 2, -1};
    case 232:
        return new int[13]{7, 11, 4, 4, 11, 9, 9, 11, 2, 9, 2, 1, -1};
    case 233:
        return new int[10]{1, 9, 0, 4, 7, 8, 2, 3, 11, -1};
    case 234:
        return new int[10]{7, 11, 4, 11, 2, 4, 2, 0, 4, -1};
    case 235:
        return new int[7]{4, 7, 8, 2, 3, 11, -1};
    case 236:
        return new int[10]{9, 4, 1, 4, 7, 1, 7, 3, 1, -1};
    case 237:
        return new int[7]{7, 8, 4, 1, 9, 0, -1};
    case 238:
        return new int[7]{3, 4, 7, 0, 4, 3, -1};
    case 239:
        return new int[4]{7, 8, 4, -1};
    case 240:
        return new int[7]{11, 10, 8, 8, 10, 9, -1};
    case 241:
        return new int[10]{0, 3, 9, 3, 11, 9, 11, 10, 9, -1};
    case 242:
        return new int[10]{1, 0, 10, 0, 8, 10, 8, 11, 10, -1};
    case 243:
        return new int[7]{10, 3, 11, 1, 3, 10, -1};
    case 244:
        return new int[10]{3, 2, 8, 2, 10, 8, 10, 9, 8, -1};
    case 245:
        return new int[7]{9, 2, 10, 0, 2, 9, -1};
    case 246:
        return new int[7]{8, 3, 0, 10, 1, 2, -1};
    case 247:
        return new int[4]{2, 10, 1, -1};
    case 248:
        return new int[10]{2, 1, 11, 1, 9, 11, 9, 8, 11, -1};
    case 249:
        return new int[7]{11, 2, 3, 9, 0, 1, -1};
    case 250:
        return new int[7]{11, 0, 8, 2, 0, 11, -1};
    case 251:
        return new int[4]{3, 11, 2, -1};
    case 252:
        return new int[7]{1, 8, 3, 9, 8, 1, -1};
    case 253:
        return new int[4]{1, 9, 0, -1};
    case 254:
        return new int[4]{8, 3, 0, -1};
    case 255:
        return new int[1]{-1};
    }
}

vec3 Asteroid::marching_cubes_grad(const int i, const int j, const int k,
                                   const int edge_num, const double *points,
                                   const double cutoff,
                                   const vector<vector<vector<vec3>>> &grads) {
    float t = 0;

    switch (edge_num) {
    default:
    case 0:
        t = inverse_lerp(points[0], points[1], cutoff);
        return (1 - t) * grads[i][j][k] + t * grads[i + 1][j][k];
    case 1:
        t = inverse_lerp(points[1], points[3], cutoff);
        return (1 - t) * grads[i + 1][j][k] + t * grads[i + 1][j + 1][k];
    case 2:
        t = inverse_lerp(points[2], points[3], cutoff);
        return (1 - t) * grads[i][j + 1][k] + t * grads[i + 1][j + 1][k];
    case 3:
        t = inverse_lerp(points[0], points[2], cutoff);
        return (1 - t) * grads[i][j][k] + t * grads[i][j + 1][k];
    case 4:
        t = inverse_lerp(points[4], points[5], cutoff);
        return (1 - t) * grads[i][j][k + 1] + t * grads[i + 1][j][k + 1];
    case 5:
        t = inverse_lerp(points[5], points[7], cutoff);
        return (1 - t) * grads[i + 1][j][k + 1] +
               t * grads[i + 1][j + 1][k + 1];
    case 6:
        t = inverse_lerp(points[6], points[7], cutoff);
        return (1 - t) * grads[i][j + 1][k + 1] +
               t * grads[i + 1][j + 1][k + 1];
    case 7:
        t = inverse_lerp(points[4], points[6], cutoff);
        return (1 - t) * grads[i][j][k + 1] + t * grads[i][j + 1][k + 1];
    case 8:
        t = inverse_lerp(points[0], points[4], cutoff);
        return (1 - t) * grads[i][j][k] + t * grads[i][j][k + 1];
    case 9:
        t = inverse_lerp(points[1], points[5], cutoff);
        return (1 - t) * grads[i + 1][j][k] + t * grads[i + 1][j][k + 1];
    case 10:
        t = inverse_lerp(points[3], points[7], cutoff);
        return (1 - t) * grads[i + 1][j + 1][k] +
               t * grads[i + 1][j + 1][k + 1];
    case 11:
        t = inverse_lerp(points[2], points[6], cutoff);
        return (1 - t) * grads[i][j + 1][k] + t * grads[i][j + 1][k + 1];
    }
}

GLuint Asteroid::shader = 0;
void Asteroid::load_shader() {
    if (Asteroid::shader != 0) {
        // Shader already loaded
        return;
    }

    shader_builder sb;
    sb.set_shader(GL_VERTEX_SHADER,
                  CGRA_SRCDIR + std::string("//res//shaders//color_vert.glsl"));
    sb.set_shader(GL_FRAGMENT_SHADER,
                  CGRA_SRCDIR +
                      std::string("//res//shaders//color_frag_orennayar.glsl"));
    sb.set_shader(GL_FRAGMENT_SHADER,
                  CGRA_SRCDIR + std::string("//res//shaders//color_frag.glsl"));
    GLuint shader = sb.build();

    Asteroid::shader = shader;
}

void Asteroid::update_model_transform(const double dt) {
    position += velocity * (float)dt;
    rotation_angle += rotation_velocity * dt;

    mat4 rotation_mat =
        glm::rotate(mat4(1), (float)rotation_angle, rotation_axis);
    mat4 translation_mat = glm::translate(mat4(1), position);
    mat4 scale_mat = glm::scale(mat4(1), glm::vec3(0.1));

    modelTransform = translation_mat * rotation_mat * scale_mat;
}
