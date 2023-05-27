#include"GPUParticle.hlsli"
#include"../../engine/Math.hlsli"

struct FireFlyParticleData
{
    float3 startPos;
    float3 endPos;
    int timer;
    int isAliveFlag;
};

RWStructuredBuffer<FireFlyParticleData> fireFlyDataBuffer : register(u0);
RWStructuredBuffer<uint> randomTable : register(u1);
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
    fireFlyDataBuffer[index].startPos = emittPos;
    
    float2 angle =
    float2(
        Rand(randomTable[index],360.0f,0.0f),
        Rand(randomTable[1024 - index],360.0f,0.0f)
    );
    float radius = 30.0f;
    float diamator = radius * 2.0f;
    float3 pos =
    float3
    (
        radius * sin(ConvertToRadian(angle.x)) * cos(ConvertToRadian(angle.y)),
        radius * sin(ConvertToRadian(angle.x)) * sin(ConvertToRadian(angle.y)),
        radius * cos(ConvertToRadian(angle.x))
    );

    fireFlyDataBuffer[index].endPos = emittPos + pos;
    fireFlyDataBuffer[index].timer = TIMER;
}

//蛍パーティクル更新
[numthreads(1024, 1, 1)]
void UpdateMain(uint3 groupId : SV_GroupID, uint groupIndex : SV_GroupIndex,uint3 groupThreadID : SV_GroupThreadID)
{
    int index = GetIndex(groupId,groupThreadID);

    --fireFlyDataBuffer[index].timer;
    float maxTimer = TIMER;
    float startTimer = TIMER - fireFlyDataBuffer[index].timer;
    float3 pos = Easing_Cubic_Out(startTimer,maxTimer,fireFlyDataBuffer[index].startPos,fireFlyDataBuffer[index].endPos);

    //出力--------------------------------------------
    ParticleData outputMat;
    matrix mat = SetScaleInMatrix(float3(1.0f,1.0f,1.0f));
    outputMat.world = SetPosInMatrix(mat,pos);
    outputMat.color = float4(0.12f, 0.97f, 0.8f,(float)(fireFlyDataBuffer[index].timer) / (float)(TIMER));
    particleData.Append(outputMat);
    //出力--------------------------------------------
}
