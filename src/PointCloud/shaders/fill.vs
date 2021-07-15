#version 450 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in int aValue;
layout(location = 2) in int show;

uniform mat4 uWorldViewProj;
uniform int uOffset; // 当前渲染的缓冲区在整个点云缓冲区中的偏移(由于缓冲区分块的原因)
uniform float pointSize;


layout(binding = 0) uniform sampler2D uGradient;

// 色带
layout(std140, binding = 1) uniform colorStrip{
	vec4 colors[343];
	int maxValue;
	int minValue;
}colorstrip;

uniform int uAttributeMode;

#define ATT_MODE_SCALAR 0
#define ATT_MODE_VECTOR 1
#define NOT_SHOW_COLOR 2
#define SHOW_COLORFROMLABEL 3
#define SHOW_COLORFROMIDENSITY 4

out vec3 vColor;
out vec4 vVertexID;
out float vShow;

vec3 getColorFromV1(){
	float w = intBitsToFloat(aValue);
	w = clamp(w, 0, 1);
	vec3 v = texture(uGradient, vec2(w, 0.0)).rgb;
	return v;
}

vec3 getColorFromV3(){
	vec3 v = vec3(
		(aValue >>   0) & 0xFF,
		(aValue >>   8) & 0xFF,
		(aValue >>  16) & 0xFF
	);
	v = v / 255.0;
	return v;
}

// mode = 3
// 根据分类着色
vec3 getColorFromLabels(){
	vec3 v = vec3(1.0, 1.0, 1.0);
	switch(aValue){
		case 0:
			v = vec3(1.0 , 1.0, 1.0);
			break;
		case 1:
			v = vec3(1.0,1.0,0.0);
			break;
		case 2:
			v = vec3(1.0,0.0,0.0);
			break;
		case 3:
			v = vec3(205.0 / 255.0, 133.0 / 255.0, 63.0 / 255.0);
			break;
		case 4:
			v = vec3(0.0, 1.0, 0.0);
			break;
		case 5:
			v = vec3(0.5, 0.0, 0.5);
			break;
		case 6:
			v = vec3(1.0, 0.0, 1.0);
			break;
		case 7:
			v = vec3(135.0 / 255.0, 206.0 / 255.0, 250.0 / 255.0);
			break;
		case 8:
			v = vec3(0.0, 0.0, 1.0);
			break;
		case 9:
			v = vec3(1.0, 0.5, 0.0);
			break;
	}
	return v;
}

// mode = 4
// 根据强度着色
vec3 getColorFromIdensity(){
	vec3 v = vec3(1.0,1.0,1.0);
	if(colorstrip.maxValue == 0 && colorstrip.minValue == 0){
		return v;
	}
	int index = int((aValue - colorstrip.minValue) / float((colorstrip.maxValue - colorstrip.minValue)) * 342);
	v = vec3(colorstrip.colors[index]);
	// v = v / 255.0;
	return v;
}

void main() {
	gl_Position = uWorldViewProj * vec4(aPosition, 1.0);
	gl_PointSize = pointSize;
	
	if(uAttributeMode == ATT_MODE_VECTOR){
		vColor = getColorFromV3();
	}else if(uAttributeMode == ATT_MODE_SCALAR){
		vColor = getColorFromV1();	
	}
    else if(uAttributeMode == NOT_SHOW_COLOR){
		vColor = vec3(1.0, 1.0, 1.0);
	}
	else if(uAttributeMode == SHOW_COLORFROMLABEL){
		vColor = getColorFromLabels();
	}
	else if(uAttributeMode == SHOW_COLORFROMIDENSITY){
		vColor = getColorFromIdensity();
	}

	int vertexID = gl_VertexID + uOffset; // 已处理的顶点数量 = 该次渲染处理顶点 + 历史处理(缓冲区偏移)
	vVertexID = vec4(
		float((vertexID >>  0) & 0xFF) / 255.0,
		float((vertexID >>  8) & 0xFF) / 255.0,
		float((vertexID >> 16) & 0xFF) / 255.0,
		float((vertexID >> 24) & 0xFF) / 255.0
	);

    vShow = float(show);
}
