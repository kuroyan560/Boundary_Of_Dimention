#include"GPUParticle.hlsli"

//出力
AppendStructuredBuffer<ParticleData>  ParticleBuffer: register(u2);

struct SignSpotParticleData
{
    float3 pos;
    float3 vel;
    float speed;
};
RWStructuredBuffer<SignSpotParticleData>  SignSpotBuffer: register(u0);
RWStructuredBuffer<SignSpotParticleData>  LarpBuffer: register(u1);

cbuffer RootConstants : register(b0)
{    
    matrix scaleRotateBillboardMat;
    float3 dirPos;
    float alpha;
};

[numthreads(1024, 1, 1)]
void InitMain(uint3 groupId : SV_GroupID, uint groupIndex : SV_GroupIndex,uint3 groupThreadID : SV_GroupThreadID)
{
    uint index = (groupThreadID.y * 1204) + groupThreadID.x + groupThreadID.z;
    index += 1024 * groupId.x;

    float offset = 15.0f;
    SignSpotBuffer[index].pos = float3(RandXorShift(index,offset,-offset),RandXorShift(index,offset,-offset),RandXorShift(index,offset,-offset));
    SignSpotBuffer[index].vel = float3(0,0,0);
    SignSpotBuffer[index].speed = 0.0f;
    LarpBuffer[index] = SignSpotBuffer[index];
}

[numthreads(1024, 1, 1)]
void UpdateMain(uint3 groupId : SV_GroupID, uint groupIndex : SV_GroupIndex,uint3 groupThreadID : SV_GroupThreadID)
{
    uint index = (groupThreadID.y * 1204) + groupThreadID.x + groupThreadID.z;
    index += 1024 * groupId.x;

    SignSpotParticleData particle = SignSpotBuffer[index];

    particle.pos = IsNanToZero(particle.pos);
    particle.vel = IsNanToZero(particle.vel);

    float3 dirNormal = dirPos - particle.pos;
    dirNormal = normalize(dirNormal);

    particle.speed = LarpFloat1(0.0f,particle.speed,0.01f);
    float3 vel = dirNormal * 5.0f;
    particle.vel = vel;
    //particle.vel = Larp(vel,particle.vel,0.01f);
    //一生たどり着かない挙動
    particle.pos += particle.vel;
    SignSpotBuffer[index] = particle;
    LarpBuffer[index].pos = Larp(SignSpotBuffer[index].pos,LarpBuffer[index].pos,0.1f);

    //行列計算-------------------------
    matrix pMatWorld = scaleRotateBillboardMat;
    pMatWorld[0][3] = LarpBuffer[index].pos.x;
    pMatWorld[1][3] = LarpBuffer[index].pos.y;
    pMatWorld[2][3] = LarpBuffer[index].pos.z;
    //行列計算-------------------------

    //出力--------------------------------------------
    ParticleData outputMat;
    outputMat.world = pMatWorld;     
    outputMat.color = float4(1,1,1,alpha);
    ParticleBuffer.Append(outputMat);
    //出力--------------------------------------------

}