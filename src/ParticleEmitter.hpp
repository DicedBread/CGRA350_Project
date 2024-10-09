#pragma once

// glm
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp> // Add this line

#include "opengl.hpp"
#include "cgra/cgra_geometry.hpp"
#include "cgra/cgra_gui.hpp"
#include "cgra/cgra_shader.hpp"
#include "cgra/cgra_wavefront.hpp"



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

    GLuint texture;

    bool shouldUpdatePosition = false;
    glm::vec3 updatePos = glm::vec3(0);

    void initShaders();

public:
    // emitter propertys
    int emitCount = 1;
    float emitTime = 0.01;
    glm::vec3 emitterVelocity = glm::vec3(0, 0, 0);
    float emitterSpeed = 0;
    float spawnRadius = 1;

    // particle props
    float initBillboardSize = 1;
    float endBillboardSize= 0.1;
    float lifeTime = 20;
    float speed = 3;
    float speedDropPercent = 0.5;
    glm::vec3 initVelocity = glm::vec3(0, 1, 0);
    glm::vec3 endColor = glm::vec3(1, 1, 1); 
    glm::vec3 initColor = glm::vec3(1, 1, 1);
    

    bool isOneOff = false;
    bool shouldEmitOneOff = false;

    ParticleEmitter();
    ~ParticleEmitter();

    void InitParticleSystem(const glm::vec3& pos); 
    // void draw(double delta, const glm::mat4& veiw, const glm::mat4 proj); 

    void updateParticles(double delta);
    void render(const glm::mat4& view, const glm::mat4 proj); 
    void updatePosition(const glm::vec3& pos);

    void emitOneOff();

    void destroy();
};

