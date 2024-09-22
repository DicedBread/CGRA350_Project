
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
    GLfloat type;
    vec3 pos;
};

goTest::goTest(){}
goTest::~goTest(){}


void goTest::init(){
    shader_builder sb;
    sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//pv.glsl"));
    sb.set_shader(GL_GEOMETRY_SHADER, CGRA_SRCDIR + std::string("//res//shaders//testgeo.glsl"));
    geoShader = sb.build();

    const GLchar* varyings[2] = {"type1", "pos1"};
    glTransformFeedbackVaryings(geoShader, 2, varyings, GL_INTERLEAVED_ATTRIBS);
    glLinkProgram(geoShader);
    glUseProgram(0);

    Particle data[10];
    for(int i = 0; i < 10; i++){
        data[i].type = i;
        data[i].pos = vec3(5, 6, 7);
    }
    data[0].type = 2;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &vbo2);

    glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glEnableVertexAttribArray(0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)(offsetof(Particle, type)));
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)(offsetof(Particle, pos)));
            glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    for(int i = 0; i < 10; i++){
        data[i].type = 0;
        data[i].pos = vec3(0, 0, 0);
    }

    glBindBuffer(GL_ARRAY_BUFFER, vbo2);
    glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_DYNAMIC_DRAW);
    // glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenTransformFeedbacks(1, &tfb);
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, tfb);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, vbo2);
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);

    cout << "init" << endl;
}

void goTest::update(){
    glEnable(GL_RASTERIZER_DISCARD);

    glUseProgram(geoShader);
    glBindVertexArray(vao);
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, tfb);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, vbo2);

    cout << "list" <<endl;
    Particle ret[10];
    glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, sizeof(ret), ret);
    for(int i = 0; i < 10; i++){
        cout << i << ": ";
        cout << ret[i].type << " "; 
        cout << ret[i].pos.x << ", " << ret[i].pos.y << ", "<< ret[i].pos.z;
        cout << endl;
    }
    cout << endl;

    glBeginTransformFeedback(GL_POINTS);
    if(m_isFirst){
        glDrawArrays(GL_POINTS, 0, 5);
        m_isFirst = false;
    }else{
        glDrawTransformFeedback(GL_POINTS, tfb);
    }
    glEndTransformFeedback();

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glGetBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(ret), ret);
    for(int i = 0; i < 10; i++){
        cout << i << ": ";
        cout << ret[i].type << " "; 
        cout << ret[i].pos.x << ", " << ret[i].pos.y << ", "<< ret[i].pos.z;
        cout << endl;
    }
    cout << endl;

    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, 0);
    glBindVertexArray(0);
    glDisable(GL_RASTERIZER_DISCARD);   
}