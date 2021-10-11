#version 450

#extension GL_NV_gpu_shader5 : enable

uniform int MaxPointsPerBuffer; // 每个buffer的最大点数

layout(local_size_x = 32, local_size_y = 1) in;

struct Vertex{
	float ux;
	float uy;
	float uz;
	uint value;
	int show;
};

layout(std430, binding = 0) buffer ssInputBuffer{
	uint inputBuffer[];
};

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

layout(location = 2) uniform int uNumPoints;
layout(location = 3) uniform int uPrime;
layout(location = 4) uniform int uOffset;

int64_t permuteI(int64_t number, int64_t prime){
	if(number > prime){
		return number;
	}
	// an int64 workaround of: residue = (number * number) % prime
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
	
	uint inputIndex = gl_GlobalInvocationID.x;

	if(inputIndex >= uNumPoints){
		return;
	}

	uint globalInputIndex = inputIndex + uOffset;

	int64_t primeI64 = int64_t(uPrime);
	int64_t t = permuteI(int64_t(globalInputIndex), primeI64);
	t = permuteI(t, primeI64);
	uint targetIndex = uint(t);

	uint value = inputBuffer[inputIndex];

	if(targetIndex < MaxPointsPerBuffer){
		Vertex v = targetBuffer0[targetIndex];
		v.value = value;
		targetBuffer0[targetIndex] = v;
	}else if(targetIndex < 2 * MaxPointsPerBuffer){
		targetIndex = targetIndex - MaxPointsPerBuffer;
		Vertex v = targetBuffer1[targetIndex];
		v.value = value;
		targetBuffer1[targetIndex] = v;
	}else if(targetIndex < 3 * MaxPointsPerBuffer){
		targetIndex = targetIndex - 2 * MaxPointsPerBuffer;
		Vertex v = targetBuffer2[targetIndex];
		v.value = value;
		targetBuffer2[targetIndex] = v;
	}else if(targetIndex < 4 * MaxPointsPerBuffer){
		targetIndex = targetIndex - 3 * MaxPointsPerBuffer;
		Vertex v = targetBuffer3[targetIndex];
		v.value = value;
		targetBuffer3[targetIndex] = v;
	}else if(targetIndex < 5 * MaxPointsPerBuffer){
		targetIndex = targetIndex - 4 * MaxPointsPerBuffer;
		Vertex v = targetBuffer4[targetIndex];
		v.value = value;
		targetBuffer4[targetIndex] = v;
	}
}



