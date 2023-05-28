#include"GPUParticle.hlsli"

//出力
AppendStructuredBuffer<ParticleData>  ParticleBuffer: register(u1);

struct SignSpotParticleData
{
    float3 pos;
    float3 vel;
};
RWStructuredBuffer<SignSpotParticleData>  SignSpotBuffer: register(u0);

cbuffer RootConstants : register(b0)
{    
    matrix scaleRotateBillboardMat;
    float3 dirPos;
    float alpha;
    float speed;
    float timeScale;
};

cbuffer InitBuffer : register(b0)
{
    float3 emittpos;
};

[numthreads(1024, 1, 1)]
void InitMain(uint3 groupId : SV_GroupID, uint groupIndex : SV_GroupIndex,uint3 groupThreadID : SV_GroupThreadID)
{
    uint index = (groupThreadID.y * 1204) + groupThreadID.x + groupThreadID.z;
    index += 1024 * groupId.x;

    float offset = 20.0f;
    SignSpotBuffer[index].pos = emittpos + float3(Rand(index * 100,offset,-offset),Rand(index*100/2,offset,-offset),Rand(index*450,offset,-offset));
    SignSpotBuffer[index].vel = float3(0,0,0);
}

[numthreads(1024, 1, 1)]
void UpdateMain(uint3 groupId : SV_GroupID, uint groupIndex : SV_GroupIndex,uint3 groupThreadID : SV_GroupThreadID)
{
    uint index = (groupThreadID.y * 1204) + groupThreadID.x + groupThreadID.z;
    index += 1024 * groupId.x;

    SignSpotParticleData particle = SignSpotBuffer[index];

    particle.pos = IsNanToZero(particle.pos);
    particle.vel = IsNanToZero(particle.vel);

    float3 lenght = (dirPos - particle.pos);
    lenght = normalize(lenght);
    float3 vel = lenght * speed;
    particle.vel = Larp(vel,particle.vel,0.01f);

    //一生たどり着かない挙動
    particle.pos += particle.vel * timeScale;


    SignSpotBuffer[index] = particle;

    //行列計算-------------------------
    matrix pMatWorld = scaleRotateBillboardMat;
    pMatWorld[0][3] = SignSpotBuffer[index].pos.x;
    pMatWorld[1][3] = SignSpotBuffer[index].pos.y;
    pMatWorld[2][3] = SignSpotBuffer[index].pos.z;
    //行列計算-------------------------

    //出力--------------------------------------------
    ParticleData outputMat;
    outputMat.world = pMatWorld;     
    outputMat.color = float4(0.12f, 0.97f, 0.80f,alpha);
    ParticleBuffer.Append(outputMat);
    //出力--------------------------------------------

}