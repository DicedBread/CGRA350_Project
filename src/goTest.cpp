
#include "goTest.hpp"

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
    GLint type;
};

goTest::goTest(){}
goTest::~goTest(){}


void goTest::init(){
    shader_builder sb;
    sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//pv.glsl"));
    // sb.set_shader(GL_GEOMETRY_SHADER, CGRA_SRCDIR + std::string("//res//shaders//testgeo.glsl"));
    geoShader = sb.build();

    Particle Particles[10];

    for(int i = 0; i < 10; i++){
        Particles[i].type = 0;
    }
    Particles[0].type = 2;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glVertexAttribPointer(0, 1, GL_INT, GL_FALSE, sizeof(Particle), (void*)(offsetof(Particle, type)));
    glBindVertexArray(0);

    glGenTransformFeedbacks(1, &tfb);
    glGenBuffers(1, &vbo);
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, tfb);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Particles), Particles, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, vbo);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);


    const GLchar* varyings[1];
    varyings[0] = "type0";
    glTransformFeedbackVaryings(geoShader, 1, varyings, GL_INTERLEAVED_ATTRIBS);
    glLinkProgram(geoShader);
    glUseProgram(0);
    cout << "init" << endl;
}

void goTest::update(){
    glEnable(GL_RASTERIZER_DISCARD);

    
    glUseProgram(geoShader);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, tfb);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, vbo);
    cout << "list" <<endl;
    Particle ret[10];
    int readLen = sizeof(ret) / sizeof(ret[0]);
    glGetBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Particle) * readLen, ret);
    for(int i = 0; i < readLen; i++){
        cout << i << ": ";
        cout << ret[i].type << endl;
    }
    cout << endl;

    glBeginTransformFeedback(GL_POINTS);
    if(m_isFirst){
        glDrawArrays(GL_POINTS, 0, 2);
        m_isFirst = false;
    }else{
        glDrawTransformFeedback(GL_POINTS, tfb);
    }
    glEndTransformFeedback();


    glGetBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Particle) * readLen, ret);
    for(int i = 0; i < readLen; i++){
        cout << i << ": ";
        cout << ret[i].type << endl;
    }
    cout << endl;

    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, 0);
    glBindVertexArray(0);
    glDisable(GL_RASTERIZER_DISCARD);   
}