struct ParticleData
{
    float3 pos;
    float4 color;
};

struct OutputData
{
    matrix mat;
    float4 color;
};

RWStructuredBuffer<ParticleData> worldPosColorArrayData : register(u0);
RWStructuredBuffer<OutputData> matrixData : register(u1);

cbuffer RootConstants : register(b0)
{    
    matrix scaleRotateBillboardMat;
    matrix viewProjection;
};

[numthreads(1024, 1, 1)]
void CSmain(uint3 groupId : SV_GroupID, uint groupIndex : SV_GroupIndex,uint3 groupThreadID : SV_GroupThreadID)
{
    uint index = (groupId.y * 1024) + groupId.x + groupId.z;
    index += 1024 * groupId.x;

    const int PARTICLE_NUM_MAX = 100;
    if(PARTICLE_NUM_MAX <= index)
    {
        return;
    }

    //�s��v�Z-------------------------
    matrix pMatWorld = scaleRotateBillboardMat;
    pMatWorld[0][3] = 0.0f;
    pMatWorld[1][3] = 0.0f;
    pMatWorld[2][3] = 0.0f;
    //�s��v�Z-------------------------

    //�o��--------------------------------------------
    OutputData outputMat;
    outputMat.mat = mul(viewProjection,pMatWorld);     
    outputMat.color = worldPosColorArrayData[index].color;
    matrixData[index] = outputMat;
    //�o��--------------------------------------------
}
