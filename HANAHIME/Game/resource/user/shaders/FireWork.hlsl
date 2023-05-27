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
    fireFlyDataBuffer[index].vel = float3(Rand(index + 12,1.0f,0.0f),Rand(index * 100,1.0f,0.0f),Rand(index * 50,1.0f,0.0f));
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

    if(0 < fireFlyDataBuffer[index].timer)
    {
        //出力--------------------------------------------
        ParticleData outputMat;
        matrix mat = SetScaleInMatrix(float3(1.0f,1.0f,1.0f));
        outputMat.world = SetPosInMatrix(mat,fireFlyDataBuffer[index].pos);
        outputMat.color = float4(1.0f,1.0f,1.0f,(float)(fireFlyDataBuffer[index].timer) / (float)(TIMER));
        particleData.Append(outputMat);
        //出力--------------------------------------------
    }
}
