#version 450
#extension GL_NV_gpu_shader5 : enable

// **************************************************
// 恢复裁剪过的点云
// **************************************************

// 设置本地工作组大小
layout(local_size_x = 1024, local_size_y = 1) in;

uniform int maxPointsPerBuffer;

// 该次裁剪选择的点索引
layout(std430, binding = 1) buffer selectBuffer0{
    int selectIndex0[];
};

struct VertexS{
	float ux;
	float uy;
	float uz;
	uint color;
	int show;
};
// 点云数据存储buffer
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

void main() {
	uint index = gl_GlobalInvocationID.x; //当前执行单元在全局工作组中的位置
    if(selectIndex0[index] == int(index)){
         // 从原始vbo中读取数据, 即在打乱后的缓冲区内
        VertexS vs;
        if(index < 1u * maxPointsPerBuffer){
            vs = vbo0[index];
            vs.show = 1;
            vbo0[index] = vs;
        }else if(index < 2u * maxPointsPerBuffer){
            vs = vbo1[index - 1u * maxPointsPerBuffer];
            vs.show = 1;
            vbo1[index - 1u * maxPointsPerBuffer] = vs;
        }else if(index < 3u * maxPointsPerBuffer){
            vs = vbo2[index - 2u * maxPointsPerBuffer];
            vs.show = 1;
            vbo2[index - 2u * maxPointsPerBuffer] = vs;
        }else if(index < 4u * maxPointsPerBuffer){
            vs = vbo3[index - 3u * maxPointsPerBuffer];
            vs.show = 1;
            vbo3[index - 3u * maxPointsPerBuffer] = vs;
        }else if(index < 5u * maxPointsPerBuffer){
            vs = vbo4[index - 4u * maxPointsPerBuffer];
            vs.show = 1;
            vbo4[index - 4u * maxPointsPerBuffer] = vs;
        }else if(index < 6u * maxPointsPerBuffer){
            vs = vbo5[index - 5u * maxPointsPerBuffer];
            vs.show = 1;
            vbo5[index - 5u * maxPointsPerBuffer] = vs;
        }else if(index < 7u * maxPointsPerBuffer){
            vs = vbo6[index - 6u * maxPointsPerBuffer];
            vs.show = 1;
            vbo6[index - 6u * maxPointsPerBuffer] = vs;
        }else if(index < 8u * maxPointsPerBuffer){
            vs = vbo7[index - 7u * maxPointsPerBuffer];
            vs.show = 1;
            vbo7[index - 7u * maxPointsPerBuffer] = vs;
        }
    }
}
