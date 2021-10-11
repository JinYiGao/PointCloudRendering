#version 450

#extension GL_NV_gpu_shader5 : enable

uniform int MaxPointsPerBuffer;

layout(location = 2) uniform int uNumPoints;
layout(location = 3) uniform int uPrime;
layout(location = 4) uniform int uOffset;

// 设置本地工作组大小
layout(local_size_x = 32, local_size_y = 1) in;

// 顶点
struct Vertex{
    float ux;
    float uy;
    float uz;
    uint color;
	bool show;
};
// 程序内传入的一个chunk的数据
layout(std430, binding = 0) buffer ssInputBuffer{
	Vertex inputBuffer[];
};
// 程序内传入的仅仅是开辟好内存空间的缓冲区，有着色器内部进行数据填充
layout(std430, binding = 2) buffer ssTargetBuffer0{
	Vertex targetBuffer0[];
};
layout(std430, binding = 3) buffer ssTargetBuffer1{
	Vertex targetBuffer1[];
};
layout(std430, binding = 4) buffer ssTargetBuffer2{
	Vertex targetBuffer2[];
};
layout(std430, binding = 5) buffer ssTargetBuffer3{
	Vertex targetBuffer3[];
};
layout(std430, binding = 6) buffer ssTargetBuffer4{
	Vertex targetBuffer4[];
};
layout(std430, binding = 7) buffer ssTargetBuffer5{
	Vertex targetBuffer5[];
};
layout(std430, binding = 8) buffer ssTargetBuffer6{
	Vertex targetBuffer6[];
};
layout(std430, binding = 9) buffer ssTargetBuffer7{
	Vertex targetBuffer7[];
};

// 打乱后索引表 536000000
layout(std430, binding = 10) buffer indexTableBuffer0{
	uint indexTable0[];
};

int64_t permuteI(int64_t number, int64_t prime){
	if(number > prime){
		return number;
	}
	int64_t q = number * number;
	int64_t d = q / prime;
	int64_t residue = q - d * prime;
	if(number <= prime / 2){
		return residue;
	}else{
		return prime - residue;
	}
}

void main(){
	uint inputIndex = gl_GlobalInvocationID.x; //当前执行单元在全局工作组中的位置

	if(inputIndex >= uNumPoints){
		return;
	}

	uint globalInputIndex = inputIndex + uOffset; // 该点在全局索引

	int64_t primeI64 = int64_t(uPrime);
	// globalInputIndex ———— t 一一映射
	int64_t t = permuteI(int64_t(globalInputIndex), primeI64);
	t = permuteI(t, primeI64);
	uint targetIndex = uint(t);
	indexTable0[targetIndex] = globalInputIndex; // target ———— input 索引映射

	Vertex v = inputBuffer[inputIndex];

	if(targetIndex < MaxPointsPerBuffer){
		targetBuffer0[targetIndex] = v;
	}else if(targetIndex < 2u * MaxPointsPerBuffer){
		targetIndex = targetIndex - 1u * MaxPointsPerBuffer;
		targetBuffer1[targetIndex] = v;
	}else if(targetIndex < 3u * MaxPointsPerBuffer){
		targetIndex = targetIndex - 2u * MaxPointsPerBuffer;
		targetBuffer2[targetIndex] = v;
	}else if(targetIndex < 4u * MaxPointsPerBuffer){
		targetIndex = targetIndex - 3u * MaxPointsPerBuffer;
		targetBuffer3[targetIndex] = v;
	}else if(targetIndex < 5u * MaxPointsPerBuffer){
		targetIndex = targetIndex - 4u * MaxPointsPerBuffer;
		targetBuffer4[targetIndex] = v;
	}else if(targetIndex < 6u * MaxPointsPerBuffer){
		targetIndex = targetIndex - 5u * MaxPointsPerBuffer;
		targetBuffer5[targetIndex] = v;
	}else if(targetIndex < 7u * MaxPointsPerBuffer){
		targetIndex = targetIndex - 6u * MaxPointsPerBuffer;
		targetBuffer6[targetIndex] = v;
	}else if(targetIndex < 8u * MaxPointsPerBuffer){
		targetIndex = targetIndex - 7u * MaxPointsPerBuffer;
		targetBuffer7[targetIndex] = v;
	}
}