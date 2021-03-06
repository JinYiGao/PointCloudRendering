#version 450

#extension GL_NV_gpu_shader5 : enable

#define InPolygon 0
#define OutPolygon 1

// 设置本地工作组大小
layout(local_size_x = 1024, local_size_y = 1) in;

uniform int SelectMode;
uniform int maxPointsPerBuffer;
uniform mat4 transform;

struct Point{
    float x;
    float y;
};
layout(std430, binding = 0) buffer Polygon{
    int num;
    Point polygon[];
};

struct VertexS{
	float ux;
	float uy;
	float uz;
	uint color;
	int show;
};

layout(std430, binding = 3) buffer ssSource0{
	VertexS vbo0[];
};

layout(std430, binding = 4) buffer ssSource1{
	VertexS vbo1[];
};

layout(std430, binding = 5) buffer ssSource2{
	VertexS vbo2[];
};

layout(std430, binding = 6) buffer ssSource3{
	VertexS vbo3[];
};

layout(std430, binding = 7) buffer ssSource4{
	VertexS vbo4[];
};

layout(std430, binding = 8) buffer ssSource5{
	VertexS vbo5[];
};

layout(std430, binding = 9) buffer ssSource6{
	VertexS vbo6[];
};

layout(std430, binding = 10) buffer ssSource7{
	VertexS vbo7[];
};

bool inPolygon(vec4 pos){
    vec4 position = transform * pos;
    vec4 point = position / position.w;
    // if(position.x > polygon[0].x){
    //     return true;
    // }
    // return false;

    bool inPolygon = false;
    for (int i = 0, j = num - 1; i < num; j = i++)
    {
        if (((polygon[i].x <= point.x && polygon[j].x > point.x) || (polygon[j].x <= point.x && polygon[i].x > point.x))
            && ((polygon[j].y - polygon[i].y) / (polygon[j].x - polygon[i].x) * (point.x - polygon[i].x) + polygon[i].y > point.y))
        {
            inPolygon = !inPolygon;
        }
    }
    if(SelectMode == InPolygon){
        return inPolygon;
    }
    else if(SelectMode == OutPolygon){
        return !inPolygon;
    }
}

void main() {
	uint index = gl_GlobalInvocationID.x; //当前执行单元在全局工作组中的位置

    // 从原始vbo中读取数据, 即在打乱后的缓冲区内
	VertexS vs;
	if(index < 1u * maxPointsPerBuffer){
		vs = vbo0[index];
        if(inPolygon(vec4(vs.ux, vs.uy, vs.uz, 1.0))){
            vs.show = 0;
            vbo0[index] = vs;
        }
	}else if(index < 2u * maxPointsPerBuffer){
		vs = vbo1[index - 1u * maxPointsPerBuffer];
        if(inPolygon(vec4(vs.ux, vs.uy, vs.uz, 1.0))){
            vs.show = 0;
            vbo0[index - 1u * maxPointsPerBuffer] = vs;
        }
	}else if(index < 3u * maxPointsPerBuffer){
		vs = vbo2[index - 2u * maxPointsPerBuffer];
        if(inPolygon(vec4(vs.ux, vs.uy, vs.uz, 1.0))){
            vs.show = 0;
            vbo0[index - 2u * maxPointsPerBuffer] = vs;
        }
	}else if(index < 4u * maxPointsPerBuffer){
		vs = vbo3[index - 3u * maxPointsPerBuffer];
        if(inPolygon(vec4(vs.ux, vs.uy, vs.uz, 1.0))){
            vs.show = 0;
            vbo0[index - 3u * maxPointsPerBuffer] = vs;
        }
	}else if(index < 5u * maxPointsPerBuffer){
		vs = vbo4[index - 4u * maxPointsPerBuffer];
        if(inPolygon(vec4(vs.ux, vs.uy, vs.uz, 1.0))){
            vs.show = 0;
            vbo0[index - 4u * maxPointsPerBuffer] = vs;
        }
	}else if(index < 6u * maxPointsPerBuffer){
		vs = vbo5[index - 5u * maxPointsPerBuffer];
        if(inPolygon(vec4(vs.ux, vs.uy, vs.uz, 1.0))){
            vs.show = 0;
            vbo0[index - 5u * maxPointsPerBuffer] = vs;
        }
	}else if(index < 7u * maxPointsPerBuffer){
		vs = vbo6[index - 6u * maxPointsPerBuffer];
        if(inPolygon(vec4(vs.ux, vs.uy, vs.uz, 1.0))){
            vs.show = 0;
            vbo0[index - 6u * maxPointsPerBuffer] = vs;
        }
	}else if(index < 8u * maxPointsPerBuffer){
		vs = vbo7[index - 7u * maxPointsPerBuffer];
        if(inPolygon(vec4(vs.ux, vs.uy, vs.uz, 1.0))){
            vs.show = 0;
            vbo0[index - 7u * maxPointsPerBuffer] = vs;
        }
	}
}
