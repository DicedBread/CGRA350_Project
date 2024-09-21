#pragma once

// glm
#include <glm/glm.hpp>

#include "opengl.hpp"
#include "cgra/cgra_geometry.hpp"
#include "cgra/cgra_gui.hpp"
#include "cgra/cgra_shader.hpp"
#include "cgra/cgra_wavefront.hpp"
#include <glm/gtc/type_ptr.hpp>



class goTest
{
private:
    /* data */
    GLuint vao;
    GLuint vbo;
    GLuint tfb;

    GLuint geoShader;

    bool m_isFirst = true;

public:
    goTest();
    ~goTest();

    void init();

    void update();
};

