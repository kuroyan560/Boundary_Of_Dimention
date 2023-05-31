#include"GPUParticle.hlsli"

struct OutputData
{
    matrix mat;
    float4 color;
};

RWStructuredBuffer<ParticleData> worldPosColorArrayData : register(u0);
AppendStructuredBuffer<OutputData> matrixData : register(u1);

cbuffer RootConstants : register(b0)
{    
    matrix scaleRotateBillboardMat;
    matrix viewProjection;
    uint particleMax;
};

[numthreads(1024, 1, 1)]
void CSmain(uint3 groupId : SV_GroupID, uint groupIndex : SV_GroupIndex,uint3 groupThreadID : SV_GroupThreadID)
{
    uint index = (groupThreadID.y * 1204) + groupThreadID.x + groupThreadID.z;
    index += 1024 * groupId.x;

    if(particleMax <= index)
    {
        return;
    }

    //行列計算-------------------------
    matrix pMatWorld = worldPosColorArrayData[index].world;
    pMatWorld = mul(pMatWorld,scaleRotateBillboardMat);
    //行列計算-------------------------

    //出力--------------------------------------------
    OutputData outputMat;
    outputMat.mat = mul(viewProjection,pMatWorld);     
    outputMat.color = worldPosColorArrayData[index].color;
    matrixData.Append(outputMat);
    //出力--------------------------------------------
}