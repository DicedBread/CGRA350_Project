
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
    GLfloat type;
    glm::vec3 pos; 
    glm::vec3 vel; 
    GLfloat age;
};

ParticleEmitter::ParticleEmitter(){}
ParticleEmitter::~ParticleEmitter(){}

void ParticleEmitter::InitParticleSystem(const vec3 &pos)
{
    m_isFirst = true;
    initShaders();

    texture = rgba_image(CGRA_SRCDIR + std::string("//res//textures//radGrad.png")).uploadTexture();

    Particle data[5000];
    data[0].type = 1;
    data[0].pos = pos;
    data[0].vel = vec3(0,0,0);
    data[0].age = 0;

    glGenVertexArrays(2, updateVao);
    glGenVertexArrays(2, renderVao);
    glGenBuffers(2, m_particleBuffer);
    glGenTransformFeedbacks(2, m_transformFeedback);

    for(int i = 0; i < 2; i++){
        glBindVertexArray(updateVao[i]);
            glBindBuffer(GL_ARRAY_BUFFER, m_particleBuffer[i]);
                glEnableVertexAttribArray(0);
                glEnableVertexAttribArray(1);
                glEnableVertexAttribArray(2);
                glEnableVertexAttribArray(3);
                glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)(offsetof(Particle, type))); // type
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)(offsetof(Particle, pos))); // position
                glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)(offsetof(Particle, vel))); // velocity
                glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)(offsetof(Particle, age))); // lifetime
                glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_DYNAMIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        glBindVertexArray(renderVao[i]);
            glBindBuffer(GL_ARRAY_BUFFER, m_particleBuffer[i]);
                glEnableVertexAttribArray(0);
                glEnableVertexAttribArray(1);
                glEnableVertexAttribArray(2);
                glEnableVertexAttribArray(3);
                glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)(offsetof(Particle, type))); // type
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)(offsetof(Particle, pos))); // position
                glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)(offsetof(Particle, vel))); // velocity
                glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)(offsetof(Particle, age))); // lifetime
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, m_transformFeedback[i]);
            glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, m_particleBuffer[i]);
        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
    }

    srand(time(0));
}

void ParticleEmitter::initShaders(){
    shader_builder geoShaderBuild;
    geoShaderBuild.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//particle_update_vertex.glsl"));
    geoShaderBuild.set_shader(GL_GEOMETRY_SHADER, CGRA_SRCDIR + std::string("//res//shaders//particle_update_geometry.glsl"));
    geoShader = geoShaderBuild.build();

    shader_builder renderShaderBuild;
    renderShaderBuild.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//particle_render_vertex.glsl"));
    renderShaderBuild.set_shader(GL_GEOMETRY_SHADER, CGRA_SRCDIR + std::string("//res//shaders//particle_render_point_to_quad.glsl"));
	renderShaderBuild.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//particle_render_fragment.glsl"));
    renderShader = renderShaderBuild.build();

    const GLchar* varyings[4];
    varyings[0] = "type1";
    varyings[1] = "position1";
    varyings[2] = "velocity1";
    varyings[3] = "age1";
    glTransformFeedbackVaryings(geoShader, sizeof(varyings) / sizeof(GLchar*), varyings, GL_INTERLEAVED_ATTRIBS);
    glLinkProgram(geoShader);
}

// void ParticleEmitter::draw(double delta, const mat4 &view, const mat4 proj)
// {
//     updateParticles(delta);
//     render(view, proj);
// }

void ParticleEmitter::updateParticles(double delta)
{
    glEnable(GL_RASTERIZER_DISCARD); 
    glBindVertexArray(updateVao[m_currReadBuff]); 
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, m_transformFeedback[m_currWriteBuff]);
    glUseProgram(geoShader);
    glUniform1f(glGetUniformLocation(geoShader, "delta"), delta);
    glUniform3fv(glGetUniformLocation(geoShader, "emitterVelocity"), 1, value_ptr(emitterVelocity));
    glUniform1f(glGetUniformLocation(geoShader, "emitterSpeed"), emitterSpeed);

    glUniform1f(glGetUniformLocation(geoShader, "emitTime"), emitTime);
    glUniform1i(glGetUniformLocation(geoShader, "emitCount"), emitCount);
    glUniform1f(glGetUniformLocation(geoShader, "lifeTime"), lifeTime);
    glUniform1f(glGetUniformLocation(geoShader, "initSpeed"), initSpeed);
    glUniform1f(glGetUniformLocation(geoShader, "maxSpeed"), maxSpeed);

    // glUniform1f(glGetUniformLocation(geoShader, "speedDropPercent"), speedDropPercent);
    glUniform1f(glGetUniformLocation(geoShader, "dragStrength"), dragStrength);


    float randIterator = (rand() / RAND_MAX);
    glUniform1f(glGetUniformLocation(geoShader, "randIteratorIn"), randIterator);
    glUniform1f(glGetUniformLocation(geoShader, "spawnRadius"), spawnRadius);
    glUniform3fv(glGetUniformLocation(geoShader, "initVelocity"), 1, value_ptr(initVelocity));
    glUniform3fv(glGetUniformLocation(geoShader, "velVariance"), 1, value_ptr(velVariance));
    glUniform3fv(glGetUniformLocation(geoShader, "constForceDir"), 1, value_ptr(constForceDir));
    glUniform1f(glGetUniformLocation(geoShader, "constForceStrength"), constForceStrength);




    glUniform1i(glGetUniformLocation(geoShader, "shouldUpdatePosition"), shouldUpdatePosition);
    glUniform3fv(glGetUniformLocation(geoShader, "updatePos"), 1, value_ptr(updatePos));

    glUniform1i(glGetUniformLocation(geoShader, "isOneOff"), isOneOff);
    glUniform1i(glGetUniformLocation(geoShader, "shouldEmitOneOff"), shouldEmitOneOff);


    glBeginTransformFeedback(GL_POINTS);
    if(!m_isFirst){
        glDrawTransformFeedback(GL_POINTS, m_transformFeedback[m_currReadBuff]);
    }else{
        glDrawArrays(GL_POINTS, 0, 1);
        m_isFirst = false;
    }
    glEndTransformFeedback();

    // Particle ret[5];
    // glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, sizeof(ret), ret);
    // for(int i = 0; i < 5; i++){
    //     cout << i << ": ";
    //     cout << ret[i].type << " "; 
    //     cout << "(" << ret[i].pos.x << ", " << ret[i].pos.y << ", "<< ret[i].pos.z << ") ";
    //     cout << "(" << ret[i].vel.x << ", " << ret[i].vel.y << ", "<< ret[i].vel.z << ") ";
    //     cout << ret[i].age << " ";
    //     cout << endl;
    // }
    // cout << endl;

    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
    glBindVertexArray(0); 
    glDisable(GL_RASTERIZER_DISCARD);    

    shouldEmitOneOff = false;
    shouldUpdatePosition = false;
}

void ParticleEmitter::render(const mat4& view, const mat4 proj){
    glBindVertexArray(renderVao[m_currWriteBuff]);
    glUseProgram(renderShader);
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);  
    glUniformMatrix4fv(glGetUniformLocation(renderShader, "uProjectionMatrix"), 1, false, value_ptr(proj));
	glUniformMatrix4fv(glGetUniformLocation(renderShader, "uModelViewMatrix"), 1, false, value_ptr(view));
	glUniform3fv(glGetUniformLocation(renderShader, "uColor"), 1, value_ptr(vec3(0, 1, 0)));
    vec3 camPos = (vec4(0, 0, -1, 0) * inverse(view));
    glUniform3fv(glGetUniformLocation(renderShader, "uCameraPos"), 1, value_ptr(camPos));
    glUniform1f(glGetUniformLocation(renderShader, "initBillboardSize"), initBillboardSize);
    glUniform1f(glGetUniformLocation(renderShader, "endBillboardSize"), endBillboardSize);

    glUniform1f(glGetUniformLocation(renderShader, "totalLifeTime"), lifeTime);
    glUniform3fv(glGetUniformLocation(renderShader, "initColor"), 1, value_ptr(initColor));
    glUniform3fv(glGetUniformLocation(renderShader, "endColor"), 1, value_ptr(endColor));


    glDrawTransformFeedback(GL_POINTS, m_transformFeedback[m_currWriteBuff]);
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);  
    glDisable(GL_DEPTH_TEST);

    glUseProgram(0);
    glBindVertexArray(0);
    
    m_currReadBuff = m_currWriteBuff;
    m_currWriteBuff = (m_currWriteBuff + 1) & 0x1; 
}

void ParticleEmitter::destroy(){
    glDeleteVertexArrays(2, renderVao);
    glDeleteVertexArrays(2, updateVao);
    glDeleteBuffers(2, m_particleBuffer);
    glDeleteTransformFeedbacks(2, m_transformFeedback);
}

void ParticleEmitter::updatePosition(const glm::vec3& pos){
    shouldUpdatePosition = true;
    updatePos = pos;
}

void ParticleEmitter::emitOneOff(){
    shouldEmitOneOff = true;
}