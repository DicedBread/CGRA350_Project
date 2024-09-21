#version 330
#extension GL_ARB_geometry_shader4 : enable

layout(points) in;
layout(points) out;
layout(max_vertices = 20) out;

in int type0[];

out int type1;


void main(){
    // if(type0[0] == 2) return;

    type1 = type0[0];
    EmitVertex();
    EndPrimitive();

    // if(type0[0] == 0){
    //     type1 = 2;
    //     EmitVertex();
    //     EndPrimitive();
    // }
    // return;
}