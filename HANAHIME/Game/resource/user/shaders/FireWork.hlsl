#include"GPUParticle.hlsli"

struct FireFlyParticleData
{
    float3 pos;
    float3 vel;
    int timer;
    int isAliveFlag;
};

RWStructuredBuffer<FireFlyParticleData> fireFlyDataBuffer : register(u0);
AppendStructuredBuffer<ParticleData> particleData : register(u1);

cbuffer EmittBuffer : register(b0)
{    
    float3 emittPos;
};

float ConvertToRadian(float Degree)
{
    return 3.14f / 180.0f * Degree;
}

static const int TIMER = 60;

//蛍パーティクル初期化
[numthreads(1024, 1, 1)]
void InitMain(uint3 groupId : SV_GroupID, uint groupIndex : SV_GroupIndex,uint3 groupThreadID : SV_GroupThreadID)
{
    int index = GetIndex(groupId,groupThreadID);
    fireFlyDataBuffer[index].pos = emittPos;
    
    float3 angle =
    float3(
    Rand(index * 100 + groupThreadID.x * 2,360.0f,0.0f),
    Rand(index * 100 + groupThreadID.x * 102,360.0f,0.0f),
    Rand(index * 100 + groupThreadID.x * 102,360.0f,0.0f)
    );

    float2 xRadian = float2(cos(ConvertToRadian(angle.x)),sin(ConvertToRadian(angle.x)));
    float2 yRadian = float2(cos(ConvertToRadian(angle.y)),sin(ConvertToRadian(angle.y)));

    float3 xVel = float3(xRadian.x,xRadian.y,0.0f);
    float3 yVel = float3(0.0f,yRadian.x,yRadian.y);
    fireFlyDataBuffer[index].vel = xVel + yVel;

    fireFlyDataBuffer[index].timer = TIMER;
}

//蛍パーティクル更新
[numthreads(1024, 1, 1)]
void UpdateMain(uint3 groupId : SV_GroupID, uint groupIndex : SV_GroupIndex,uint3 groupThreadID : SV_GroupThreadID)
{
    int index = GetIndex(groupId,groupThreadID);
    fireFlyDataBuffer[index].vel = Larp(float3(0.0f,0.0f,0.0f),fireFlyDataBuffer[index].vel,0.01f);
    fireFlyDataBuffer[index].pos += fireFlyDataBuffer[index].vel;
    --fireFlyDataBuffer[index].timer;

    //出力--------------------------------------------
    ParticleData outputMat;
    matrix mat = SetScaleInMatrix(float3(1.0f,1.0f,1.0f));
    outputMat.world = SetPosInMatrix(mat,fireFlyDataBuffer[index].pos);
    outputMat.color = float4(0.12f, 0.97f, 0.8f,(float)(fireFlyDataBuffer[index].timer) / (float)(TIMER));
    particleData.Append(outputMat);
    //出力--------------------------------------------
    
}
