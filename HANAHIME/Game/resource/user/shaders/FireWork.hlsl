#include"GPUParticle.hlsli"
#include"../../engine/Math.hlsli"

struct FireFlyParticleData
{
    float3 startPos;
    float3 endPos;
    float3 startColor;
    float3 endColor;
    float3 nowColor;
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

Texture2D<float4> tex : register(t0);
SamplerState smp : register(s0);
//蛍パーティクル初期化
[numthreads(1024, 1, 1)]
void InitMain(uint3 groupId : SV_GroupID, uint groupIndex : SV_GroupIndex,uint3 groupThreadID : SV_GroupThreadID)
{
    int index = GetIndex(groupId,groupThreadID);
    fireFlyDataBuffer[index].startPos = emittPos;

    int mulNum = index / 1024;
    int randomTableIndex = index - 1024 * mulNum;
    int randomTableMaxIndex = 1024 * 1;

    float2 angle =
    float2(
        Rand(randomTable[randomTableIndex],360.0f,0.0f),
        Rand(randomTable[randomTableMaxIndex - randomTableIndex],360.0f,0.0f)
    );
    float radius = Rand(randomTable[randomTableIndex], 5.0f,50.0f);
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

    fireFlyDataBuffer[index].startColor = tex.SampleLevel(smp,float2(0.9f,0.9f),0);
    fireFlyDataBuffer[index].endColor = tex.SampleLevel(smp,float2(0.1f,0.1f),0);
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

    //色の補間
    fireFlyDataBuffer[index].nowColor = Easing_Cubic_Out(startTimer,maxTimer,fireFlyDataBuffer[index].startColor,fireFlyDataBuffer[index].endColor);

    //出力--------------------------------------------

    float alpha = (float)(fireFlyDataBuffer[index].timer) / (float)(TIMER);
    ParticleData outputMat;
    matrix mat = SetScaleInMatrix(float3(1.0f,1.0f,1.0f));
    outputMat.world = SetPosInMatrix(mat,pos);
    outputMat.color.xyz = fireFlyDataBuffer[index].nowColor.xyz;
    outputMat.color.a = alpha;
    particleData.Append(outputMat);
    //出力--------------------------------------------
}
