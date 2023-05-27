#include"GPUParticle.hlsli"

struct FireFlyParticleData
{
    float3 pos;
    float3 vel;
    float2 scale;
    float4 color;
    //x..timer,y...revFlag
    uint2 flashTimer;
    //x...angle,y...vel
    float2 angle;
    float3 basePos;
};

RWStructuredBuffer<FireFlyParticleData> fireFlyDataBuffer : register(u0);
AppendStructuredBuffer<ParticleData> particleData : register(u1);

cbuffer RootConstants : register(b0)
{    
    matrix rotate;
};

cbuffer PlayerBuffer : register(b1)
{    
    float3 playerPos;
};

float ConvertToRadian(float Degree)
{
    return 3.14f / 180.0f * Degree;
}

//蛍パーティクル初期化
[numthreads(1024, 1, 1)]
void InitMain(uint3 groupId : SV_GroupID, uint groupIndex : SV_GroupIndex,uint3 groupThreadID : SV_GroupThreadID)
{
    uint index = (groupThreadID.y * 1204) + groupThreadID.x + groupThreadID.z;
    index += 1024 * groupId.x;

    const int PARTICLE_NUM_MAX = 10000;
    if(PARTICLE_NUM_MAX <= index)
    {
        return;
    }
    
    //初期値生成----------------------------------------
    //ステージの範囲外かつ天球の間

    //300 ~ 500
    float radius = Rand(index * 100,500,0);
    float radian = AngleToRadian(Rand(index * 50,360,0));
    float3 pos = float3(cos(radian) * radius, Rand(index * 10,400,0),sin(radian) * radius);

    float4 color = float4(0.12f, 0.97f, 0.8f,1);
    //初期値生成----------------------------------------

    //出力--------------------------------------------
    FireFlyParticleData outputMat;
    outputMat.pos = pos;
    outputMat.vel = float3(0.0f,-Rand(index,0.5f,0.1f),0.0f);
    float scale = Rand(index,5.0f,3.0f);
    outputMat.scale = float2(scale,scale);
    outputMat.color = color;
    outputMat.flashTimer = uint2(Rand(index,50,0),1);
    outputMat.angle.x = 0.0f;
    outputMat.angle.y = Rand(index,5.0f,0.5f);
    if( outputMat.angle.y <= 0.0f)
    {
        outputMat.angle.y = 0.5f;
    }

    outputMat.basePos = outputMat.pos;
    fireFlyDataBuffer[index] = outputMat;
    //出力--------------------------------------------
}

//蛍パーティクル更新
[numthreads(1024, 1, 1)]
void UpdateMain(uint3 groupId : SV_GroupID, uint groupIndex : SV_GroupIndex,uint3 groupThreadID : SV_GroupThreadID)
{
    uint index = (groupThreadID.y * 1204) + groupThreadID.x + groupThreadID.z;
    index += 1024 * groupId.x;

    const int PARTICLE_NUM_MAX = 10000;
    if(PARTICLE_NUM_MAX <= index)
    {
        return;
    }

    //プレイヤーに近づいたらパーティクルの拡縮を早める
    int vel = 1;

    float distanceNum = distance(playerPos,fireFlyDataBuffer[index].pos);
    float maxLength = 50.0f;
    float2 particleScaleOffset = float2(1.0f,1.0f);
    if(distanceNum <= maxLength)
    {
        float rate = distanceNum / maxLength;
        vel = 1 + 5 * (1.0f - rate);

        float scale = 1.0f + 1 * (1.0f - rate);
        particleScaleOffset = float2(scale,scale);
    }


    if(fireFlyDataBuffer[index].flashTimer.y)
    {
        fireFlyDataBuffer[index].flashTimer.x += vel;
    }
    else
    {
        fireFlyDataBuffer[index].flashTimer.x -= vel;
    }

    if(60 <= fireFlyDataBuffer[index].flashTimer.x)
    {
        fireFlyDataBuffer[index].flashTimer.y = 0;        
        fireFlyDataBuffer[index].flashTimer.x = 60;
    }
    else if(fireFlyDataBuffer[index].flashTimer.x <= 0)
    {
        fireFlyDataBuffer[index].flashTimer.y = 1;
        fireFlyDataBuffer[index].flashTimer.x = 0;
    }

    float rate = fireFlyDataBuffer[index].flashTimer.x / 60.0f;
    fireFlyDataBuffer[index].color.a = rate;
    float2 scale = fireFlyDataBuffer[index].scale * float2(rate,rate);
    scale *= particleScaleOffset;

    fireFlyDataBuffer[index].angle.x += fireFlyDataBuffer[index].angle.y;
    fireFlyDataBuffer[index].pos.x = fireFlyDataBuffer[index].basePos.x + sin(ConvertToRadian(fireFlyDataBuffer[index].angle.x)) * 5.0f;
    fireFlyDataBuffer[index].pos += fireFlyDataBuffer[index].vel;
    if(fireFlyDataBuffer[index].pos.y <= fireFlyDataBuffer[index].basePos.y - 200.0f)
    {
        fireFlyDataBuffer[index].pos = fireFlyDataBuffer[index].basePos;
    }



    //出力--------------------------------------------
    ParticleData outputMat;
    matrix mat = SetScaleInMatrix(float3(scale.x,scale.y,scale.y));
    outputMat.world = SetPosInMatrix(mat,fireFlyDataBuffer[index].pos);
    outputMat.color = fireFlyDataBuffer[index].color;
    particleData.Append(outputMat);
    //出力--------------------------------------------
}
