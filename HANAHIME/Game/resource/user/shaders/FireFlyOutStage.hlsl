struct ParticleData
{
    float3 pos;
    float4 color;
};

AppendStructuredBuffer<ParticleData> particleData : register(u0);

[numthreads(1024, 1, 1)]
void CSmain(uint3 groupId : SV_GroupID)
{
    uint index = (groupId.y * 1024) + groupId.x + groupId.z;
    index += 1024 * groupId.x;

    index = groupId.x;

    const int PARTICLE_NUM_MAX = 10000;
    if(PARTICLE_NUM_MAX <= index)
    {
        return;
    }

    float3 pos = float3(0,0,0);
    float4 color = float4(1,1,1,1);

    //o—Í--------------------------------------------
    ParticleData outputMat;
    outputMat.pos = pos;
    outputMat.color = color;
    particleData.Append(outputMat);
    //o—Í--------------------------------------------
}
