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
    bool m_isFirst;


    unsigned int m_currVB = 0;
    unsigned int m_currTFB = 0;
    GLuint updateVao;
    GLuint renderVao;

    GLuint m_particleBuffer[2];
    GLuint m_transformFeedback[2];
    // PSUpdateTechnique m_updateTechnique;
    // BillboardTechnique m_billboardTechnique;
    // RandomTexture m_randomTexture;
    // Texture* m_pTexture;

    GLuint geoShader;
    GLuint mShader;

    int m_time;

public:
    ParticleEmitter();
    ~ParticleEmitter();

    bool InitParticleSystem(const glm::vec3& pos); 
    void draw(double delta, const glm::mat4& veiw, const glm::mat4 proj); 

    void updateParticles(double delta);
    void render(const glm::mat4& view, const glm::mat4 proj); 
};

