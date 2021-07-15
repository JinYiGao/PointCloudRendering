#version 450 core

layout(location = 0) out vec4 out_color;
layout(location = 1) out vec4 out_indices;

in vec3 vColor;
in vec4 vVertexID;
in float vShow;

void main() {
	if(vShow == 0){
		discard;
	}
	out_color = vec4(vColor, 1.0);
	out_indices = vVertexID;
}