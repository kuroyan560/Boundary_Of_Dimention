#include"GPUParticle.hlsli"

AppendStructuredBuffer<ParticleData> particleData : register(u0);

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
    float radius = Rand(index * 100,500,300);
    float radian = AngleToRadian(Rand(index * 50,360,0));
    float3 pos = float3(cos(radian) * radius, Rand(index * 10,400,0),sin(radian) * radius);

    float4 color = float4(0.12f, 0.97f, 0.8f,1);
    //初期値生成----------------------------------------

    //出力--------------------------------------------
    ParticleData outputMat;
    outputMat.pos = pos;
    outputMat.color = color;
    particleData.Append(outputMat);
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

    float3 pos = float3(0,index,0);
    float4 color = float4(1,1,1,1);

    //出力--------------------------------------------
    ParticleData outputMat;
    outputMat.pos = pos;
    outputMat.color = color;
    particleData.Append(outputMat);
    //出力--------------------------------------------
}
