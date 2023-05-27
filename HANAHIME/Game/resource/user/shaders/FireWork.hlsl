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
    
    float3 angle =
    float3(
        Rand(randomTable[index],360.0f,0.0f),
        Rand(randomTable[1024 - index],360.0f,0.0f),
        Rand(randomTable[1024 - index],360.0f,0.0f)
    );

    float2 xRadian = float2(cos(ConvertToRadian(angle.x)),sin(ConvertToRadian(angle.x)));
    float2 yRadian = float2(cos(ConvertToRadian(angle.y)),sin(ConvertToRadian(angle.y)));

    float3 xVel = float3(xRadian.x,0.0f,xRadian.y);
    float3 yVel = float3(0.0f,yRadian.x,yRadian.y);
    float radius = 60.0f;
    fireFlyDataBuffer[index].endPos = emittPos + (xVel * (radius / 2.0f) + yVel * (radius / 2.0f));

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
