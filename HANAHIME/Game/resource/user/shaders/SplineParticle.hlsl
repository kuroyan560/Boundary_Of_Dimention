#include"GPUParticle.hlsli"

//�X�v���C���Ȑ�
float3 SplinePosition(RWStructuredBuffer<float3> LIMIT_INDEX_ARRAY,int START_INDEX,float RATE,int INDEX_MAX)
{
    if (START_INDEX < 1)
	{
		return LIMIT_INDEX_ARRAY[1];
	}
    if(START_INDEX > INDEX_MAX - 3)
    {
        return LIMIT_INDEX_ARRAY[INDEX_MAX - 3];
    }
	float3 p0 = LIMIT_INDEX_ARRAY[START_INDEX - 1];
	float3 p1 = LIMIT_INDEX_ARRAY[START_INDEX];
	float3 p2 = LIMIT_INDEX_ARRAY[START_INDEX + 1];
	float3 p3 = LIMIT_INDEX_ARRAY[START_INDEX + 2];

    float3 resultPos;
    resultPos = 0.5 * ((2 * p1  + (-p0 + p2) * RATE) + (2 * p0 - 5 * p1 + 4 * p2 - p3) * (RATE * RATE) + (-p0 + 3 * p1 - 3 * p2 + p3) * (RATE * RATE * RATE));
    return resultPos;
};

cbuffer RootConstants : register(b0)
{
    uint limitIndexMaxNum;
};

struct SplineData
{
    float3 pos;
    float3 vel;
    float4 color;       //�p�[�e�B�N���̐F
    int startIndex;
    float rate;
};

//������----------------------------------------
RWStructuredBuffer<SplineData> worldPosData : register(u0);
RWStructuredBuffer<float3> LimitPosData : register(u1);

[numthreads(32, 32, 1)]
void SplineInitMain(uint3 groupId : SV_GroupID, uint groupIndex : SV_GroupIndex,uint3 groupThreadID : SV_GroupThreadID)
{
    uint index = (groupThreadID.y * 32) + groupThreadID.x + groupThreadID.z;
    index += 1024 * groupId.x;

    float2 uv = float2(groupThreadID.x,groupThreadID.y) / 512.0f;

    int startIndex = (float(index) / 1024.0f) * limitIndexMaxNum;
    //�����_���ŋȐ��͈͓̔��ŗ����ō��W�����߂�

    float rateMax = float(1024) / float(limitIndexMaxNum);
    float rateMin = index;

    for(;rateMax < rateMin;)
    {
        rateMin -= rateMax;
    }
    float rate = rateMin / rateMax;


    worldPosData[index].pos = SplinePosition(LimitPosData,startIndex,rate,limitIndexMaxNum).xyz;
    worldPosData[index].vel = float3(RandXorShift(index,1.0f,0.1f),RandXorShift(index,1.0f,0.1f),RandXorShift(index,1.0f,0.1f));
	worldPosData[index].color = float4(1,1,1,0);
    worldPosData[index].startIndex = startIndex;
    worldPosData[index].rate = rate;
}


//�X�V--------------------------------------------------------------------------------------

cbuffer RootConstants : register(b0)
{
    matrix scaleRotate;
    uint startIndex;
    float rate;
};

AppendStructuredBuffer<ParticleData> worldPosColorData : register(u1);

[numthreads(1024, 1, 1)]
void SplineUpdateMain(uint3 groupId : SV_GroupID, uint groupIndex : SV_GroupIndex,uint3 groupThreadID : SV_GroupThreadID)
{
    uint index = GetIndex(groupId,groupThreadID);

    uint particleStartIndex = worldPosData[index].startIndex;
    float particleRate = worldPosData[index].rate;
  
    worldPosData[index].color.a = 0.0f;
    //CPU�ł̃C���f�b�N�X�����̃C���f�b�N�X�Ȃ�`�悷��
    if(particleStartIndex < startIndex)
    {
        worldPosData[index].color.a = 1.0f;
    }
    //CPU�ł̃C���f�b�N�X�Ɠ����l��Rate�����Ȃ�`�悷��
    if(particleStartIndex == startIndex && particleRate < rate)
    {
        worldPosData[index].color.a = 1.0f;
    }


    float3 particlePos = worldPosData[index].pos;
    //�s��v�Z-------------------------
    matrix pMatWorld = scaleRotate;
    pMatWorld[0][3] = particlePos.x;
    pMatWorld[1][3] = particlePos.y;
    pMatWorld[2][3] = particlePos.z;
    //�s��v�Z-------------------------

    //�o��--------------------------------------------
    ParticleData outputMat;
    outputMat.world = pMatWorld;
    outputMat.color = worldPosData[index].color;
    worldPosColorData.Append(outputMat);
    //�o��--------------------------------------------
}

//�X�V--------------------------------------------------------------------------------------

