
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

// perlin noise
#include "PerlinNoise.hpp"

// header
#include "asteroid.hpp"

using namespace std;
using namespace cgra;
using namespace glm;

Asteroid::Asteroid() {
    const siv::PerlinNoise::seed_type seed = 237u;

    const siv::PerlinNoise perlin{seed};

    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            const double noise = perlin.octave2D_01((i * 0.01), (j * 0.01), 4);

            std::cout << noise << '\t';
        }
        std::cout << std::endl;
    }
}
