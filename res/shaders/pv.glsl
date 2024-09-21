#version 330


layout (location = 0) in int type;

out int type0;

void main(){
    type0 = type;
}