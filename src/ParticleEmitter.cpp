
#include "ParticleEmitter.hpp"
#include <algorithm> 

#include "cgra/cgra_geometry.hpp"
#include "cgra/cgra_gui.hpp"
#include "cgra/cgra_image.hpp"
#include "cgra/cgra_shader.hpp"
#include "cgra/cgra_wavefront.hpp"
#include <glm/glm.hpp>
#include "opengl.hpp"

using namespace glm;
using namespace cgra;
using namespace std;

struct Particle{
    GLfloat Type;
    glm::vec3 Pos; 
    glm::vec3 Vel; 
    GLfloat LifetimeMillis;
}; 


ParticleEmitter::ParticleEmitter(){}

ParticleEmitter::~ParticleEmitter(){}

bool ParticleEmitter::InitParticleSystem(const vec3 &pos)
{
    shader_builder sb;
    sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//part_vert.glsl"));
    sb.set_shader(GL_GEOMETRY_SHADER, CGRA_SRCDIR + std::string("//res//shaders//geometry_shader.glsl"));
    geoShader = sb.build();



    shader_builder b;
    b.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//color_vert.glsl"));
	b.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//color_frag.glsl"));
    mShader = b.build();

    Particle Particles[20];
    Particles[0].Type = 1;
    Particles[0].Pos = vec3(20.0f, 0.0f, 0.0f);
    Particles[0].Vel = vec3(0.0f, 0.0f, 0.0f);
    Particles[0].LifetimeMillis = 0.0f;

    Particles[1].Type = 8;
    Particles[1].Pos = vec3(0.0f, 0.0f, 0.0f);
    Particles[1].Vel = vec3(0.0f, 0.0f, 0.0f);
    Particles[1].LifetimeMillis = 0.0f;

    glGenVertexArrays(1, &updateVao);
    glBindVertexArray(updateVao);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)(offsetof(Particle, Type))); // type
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)(offsetof(Particle, Pos))); // position
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)(offsetof(Particle, Vel))); // velocity
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)(offsetof(Particle, LifetimeMillis))); // lifetime 
    glBindVertexArray(0);

    glGenVertexArrays(1, &renderVao);
    glBindVertexArray(renderVao);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)(offsetof(Particle, Pos))); // position
    glBindVertexArray(0);


    glGenTransformFeedbacks(2, m_transformFeedback);
    glGenBuffers(2, m_particleBuffer);
    for(int i = 0; i < 2; i++){  
        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, m_transformFeedback[i]);
        glBindBuffer(GL_ARRAY_BUFFER, m_particleBuffer[i]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Particles), &Particles[0], GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, m_particleBuffer[i]);
    }

    const GLchar* Varyings[4];
    Varyings[0] = "type1";
    Varyings[1] = "position1";
    Varyings[2] = "velocity1";
    Varyings[3] = "age1";
    glTransformFeedbackVaryings(geoShader, 4, Varyings, GL_INTERLEAVED_ATTRIBS);
    // cout << "huh" << endl;
    glLinkProgram(geoShader);

    cout << "init" << endl;

    return false;
}

void ParticleEmitter::draw(double delta, const mat4 &view, const mat4 proj)
{
    m_time += delta;

    updateParticles(delta);
    render(view, proj);

    m_currVB = m_currTFB;
    m_currTFB = (m_currTFB + 1) & 0x1;
}

void ParticleEmitter::updateParticles(double delta)
{
    glEnable(GL_RASTERIZER_DISCARD); 
    glUseProgram(geoShader);
    glUniform1f(glGetUniformLocation(geoShader, "delta"), (GLfloat) delta);

    glBindVertexArray(updateVao);
    glBindBuffer(GL_ARRAY_BUFFER, m_particleBuffer[m_currVB]); 
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, m_transformFeedback[m_currTFB]);  
    glBeginTransformFeedback(GL_POINTS);
    if(m_isFirst){
        glDrawArrays(GL_POINTS, 1, 1); 
        m_isFirst = false;
    }else{
        glDrawTransformFeedback(GL_POINTS, m_transformFeedback[m_currVB]);
    }

    Particle ret[20];
    for(int i = 0; i < 20; i++){
        ret[i].Type = 0;
        ret[i].Pos = vec3(0,0,0);
        ret[i].Vel = vec3(0,0,0);
        ret[i].LifetimeMillis = 0;
    }

    cout << "bound tfbuff index = " << m_currTFB << endl;
    cout << "bound pbuff index = " << m_currVB << endl;
    glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, sizeof(Particle) * 20, ret);
    for(int i = 0; i < 20; i++){
        cout << i << ": ";
        cout << ret[i].Type << " ";
        cout << ret[i].LifetimeMillis << " ";
        cout << ret[i].Pos.x << ", " << ret[i].Pos.y << ", " << ret[i].Pos.z;
        cout << endl;
    } // man idk i think the draw arrays call be funny or something
    cout << endl;

    glEndTransformFeedback();
    glBindVertexArray(0);
    glDisable(GL_RASTERIZER_DISCARD);    
}

void ParticleEmitter::render(const mat4& view, const mat4 proj){
    glBindBuffer(GL_ARRAY_BUFFER, m_particleBuffer[m_currTFB]);

    glUseProgram(mShader); // load shader and variables
	glUniformMatrix4fv(glGetUniformLocation(mShader, "uProjectionMatrix"), 1, false, value_ptr(proj));
	glUniformMatrix4fv(glGetUniformLocation(mShader, "uModelViewMatrix"), 1, false, value_ptr(view));
	glUniform3fv(glGetUniformLocation(mShader, "uColor"), 1, value_ptr(vec3(0, 1, 0)));

    glBindVertexArray(renderVao);
    glDrawTransformFeedback(GL_POINTS, m_transformFeedback[m_currTFB]);
    glBindVertexArray(0);

}