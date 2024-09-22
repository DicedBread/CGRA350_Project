#pragma once

// glm
#include <glm/glm.hpp>

#include "opengl.hpp"
#include "cgra/cgra_geometry.hpp"
#include "cgra/cgra_gui.hpp"
#include "cgra/cgra_shader.hpp"
#include "cgra/cgra_wavefront.hpp"
#include <glm/gtc/type_ptr.hpp>



class ParticleEmitter
{
private:
    int maxEmitOutput = GL_MAX_GEOMETRY_OUTPUT_VERTICES; 

    bool m_isFirst;
    unsigned int m_currReadBuff = 0;
    unsigned int m_currWriteBuff = 1;

    GLuint m_particleBuffer[2];
    GLuint m_transformFeedback[2];
    GLuint updateVao[2];
    GLuint renderVao[2];

    GLuint geoShader;
    GLuint renderShader;
    
    int maxParticles = 1000;

    float emitTime = 1;
    int emitCount = 1;
    float particleLifeTime = 1;

    void initShaders();

public:
    ParticleEmitter();
    ~ParticleEmitter();

    void InitParticleSystem(const glm::vec3& pos); 
    void draw(double delta, const glm::mat4& veiw, const glm::mat4 proj); 

    void updateParticles(double delta);
    void render(const glm::mat4& view, const glm::mat4 proj); 
};

