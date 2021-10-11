#version 450
#extension GL_NV_gpu_shader5 : enable

// **************************************************
// 用于按照一定规则选取点云子集
// **************************************************

#define InPolygon 0
#define OutPolygon 1
#define InPolyline 2

// 设置本地工作组大小
layout(local_size_x = 1024, local_size_y = 1) in;

uniform int SelectMode; // 选择模式
uniform int maxPointsPerBuffer;
uniform mat4 transform;

struct Point{
    float x;
    float y;
};

// 标准化设备坐标 多边形buffer
layout(std430, binding = 0) buffer Polygon{
    int num;
    Point polygon[];
};

// 该次裁剪选择的点索引
layout(std430, binding = 1) buffer selectBuffer0{
    uint count; // 选取的点云数量
    int selectIndex0[];
};

layout(std430, binding = 2) buffer Polyline{
    int screenWidth;
    int screenHeight;
    int pnum;
    Point polyline[];
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

// 判断点是否在多边形内
bool isInPolygon(vec4 pos){
    vec4 position = transform * pos;
    vec4 point = position / position.w;
    bool inPolygon = false;
    for (int i = 0, j = num - 1; i < num; j = i++)
    {
        if (((polygon[i].x <= point.x && polygon[j].x > point.x) || (polygon[j].x <= point.x && polygon[i].x > point.x))
            && ((polygon[j].y - polygon[i].y) / (polygon[j].x - polygon[i].x) * (point.x - polygon[i].x) + polygon[i].y > point.y))
        {
            inPolygon = !inPolygon;
        }
    }

    return inPolygon;
}

// 判断点是否在多段线内, 光栅化后 进行计算判断 若在 并且z值最小 返回当前polyline栅格索引
int isInPolyline(vec4 pos){
    vec4 position = transform * pos;
    vec4 point = position / position.w;
    ivec2 screen2d = ivec2(0, 0);
    // screen2d.x = (int)((point.x + 1.0) / 2.0 * screenWidth);
    // screen2d.y = (int)((-point.y + 1.0) / 2.0 * screenHeight);
    screen2d.x = (int)(point.x * screenWidth / 2.0);
    screen2d.y = (int)(point.y * screenHeight / 2.0);
    for(int i = 0; i < pnum; i++){
        if(screen2d.x == (int)(polyline[i].x) && screen2d.y == (int)(polyline[i].y)){
            // 当前栅格对应已有点数据 比较
            if(selectIndex0[i] != -1){
                VertexS vs;
                int index = selectIndex0[i];
                if(index < 1u * maxPointsPerBuffer){
                    vs = vbo0[index];
                }else if(index < 2u * maxPointsPerBuffer){
                    vs = vbo1[index - 1u * maxPointsPerBuffer];
                }else if(index < 3u * maxPointsPerBuffer){
                    vs = vbo2[index - 2u * maxPointsPerBuffer];
                }else if(index < 4u * maxPointsPerBuffer){
                    vs = vbo3[index - 3u * maxPointsPerBuffer];
                }else if(index < 5u * maxPointsPerBuffer){
                    vs = vbo4[index - 4u * maxPointsPerBuffer];
                }else if(index < 6u * maxPointsPerBuffer){
                    vs = vbo5[index - 5u * maxPointsPerBuffer];
                }else if(index < 7u * maxPointsPerBuffer){
                    vs = vbo6[index - 6u * maxPointsPerBuffer];
                }else if(index < 8u * maxPointsPerBuffer){
                    vs = vbo7[index - 7u * maxPointsPerBuffer];
                }
                vec4 oldPosition = transform * vec4(vs.ux, vs.uy, vs.uz, 1.0);
                vec4 oldPoint = oldPosition / oldPosition.w;
                if(point.z < oldPoint.z){
                    return i; // i 为对应polyline数组索引
                }
                else{
                    return -1;
                }
            }
            else{
                return i;
            }
        }
    }

    return -1;
}

void main() {
	uint index = gl_GlobalInvocationID.x; //当前执行单元在全局工作组中的位置

    // 从原始vbo中读取数据, 即在打乱后的缓冲区内
	VertexS vs;
	if(index < 1u * maxPointsPerBuffer){
		vs = vbo0[index];
	}else if(index < 2u * maxPointsPerBuffer){
		vs = vbo1[index - 1u * maxPointsPerBuffer];
	}else if(index < 3u * maxPointsPerBuffer){
		vs = vbo2[index - 2u * maxPointsPerBuffer];
	}else if(index < 4u * maxPointsPerBuffer){
		vs = vbo3[index - 3u * maxPointsPerBuffer];
	}else if(index < 5u * maxPointsPerBuffer){
		vs = vbo4[index - 4u * maxPointsPerBuffer];
	}else if(index < 6u * maxPointsPerBuffer){
		vs = vbo5[index - 5u * maxPointsPerBuffer];
	}else if(index < 7u * maxPointsPerBuffer){
		vs = vbo6[index - 6u * maxPointsPerBuffer];
	}else if(index < 8u * maxPointsPerBuffer){
		vs = vbo7[index - 7u * maxPointsPerBuffer];
	}

    if(SelectMode == InPolygon){
        if(isInPolygon(vec4(vs.ux, vs.uy, vs.uz, 1.0))){
            uint counter = atomicAdd(count, 1); // 返回 +1 之前的count
            selectIndex0[counter] = (int)index;
        }
    }else if(SelectMode == InPolyline){
        int i = isInPolyline(vec4(vs.ux, vs.uy, vs.uz, 1.0));
        if(i != -1){
            selectIndex0[i] = (int)index;
        }
    }
}
