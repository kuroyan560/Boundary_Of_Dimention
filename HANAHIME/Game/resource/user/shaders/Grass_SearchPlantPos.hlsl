#include"../../engine/Math.hlsli"
#include"../../engine/Camera.hlsli"

//���茋��
struct CheckResult
{
    float3 m_plantPos;
    int m_isSuccess;
    float3 m_plantNormal;
};

struct SearchPlantPosConstData
{
    int m_grassCount;
    float m_seed;
};

//�����_��
float RandomIntInRange(float arg_seed)
{
    return frac(sin(dot(float2(arg_seed, arg_seed + 1.0f), float2(12.9898f, 78.233f))) * 43758.5453f);
}

RWStructuredBuffer<CheckResult> searchPlantResultBuffer : register(u0);
StructuredBuffer<float3> grassPosArrayBuffer : register(t0);
Texture2D<float4> g_worldMap : register(t1);
Texture2D<float4> g_normalMap : register(t2);
Texture2D<float4> g_brightMap : register(t3);


cbuffer cbuff0 : register(b0)
{
    SearchPlantPosConstData constData;
}

[numthreads(16, 16, 1)]
void SearchPlantPos(uint3 GlobalID : SV_DispatchThreadID, uint3 GroupID : SV_GroupID, uint3 LocalID : SV_GroupThreadID)
{
    
    //�O���[�o��ID�̌v�Z
    const int GRASS_SPAN = 10;
    uint index = GlobalID.y * (1280 / GRASS_SPAN) + GlobalID.x;
    
    //�X�N���[�����W���烏�[���h���W�֕ϊ��B
    CheckResult result = searchPlantResultBuffer[index];
    
    //�T���񐔁B
    //uint2 screenPos = (GroupID.xy * uint2(16, 16) + LocalID.xy) * uint2(GRASS_SPAN, GRASS_SPAN);
    uint2 screenPos = uint2(1280 / 2, 720 / 2);
    
    //�����_���ŎU�炷�B
    uint randomScatter = 200;
    uint2 random = uint2(RandomIntInRange(constData.m_seed * LocalID.x * GlobalID.y) * (randomScatter * 2), RandomIntInRange(constData.m_seed * LocalID.y * GlobalID.x) * (randomScatter * 2));
    random -= uint2(randomScatter, randomScatter);
    screenPos += random;
    
    //�T���v�����O�������W�����C�g�ɓ������Ă���ʒu���ǂ����𔻒f�B
    result.m_isSuccess = step(0.9f, g_brightMap[screenPos].x);
        
    //�T���v�����O�Ɏ��s�����玟�ցB
    if (result.m_isSuccess == 0)
    {
        searchPlantResultBuffer[index] = result;
        return;
    }
        
    //�T���v�����O�ɐ��������烏�[���h���W�����߂�B
    result.m_plantPos = g_worldMap[screenPos].xyz;
        
    //�@�������߂�B
    result.m_plantNormal = g_normalMap[screenPos].xyz;
    
    //���łɐ����Ă���Ƃ���ɂ�����x���₵�Ă��Ȃ������`�F�b�N�B
    for (int grassIndex = 0; grassIndex < constData.m_grassCount; ++grassIndex)
    {
        float distance = length(result.m_plantPos - grassPosArrayBuffer[grassIndex]);
        if (distance < 1.9f)
        {
            result.m_isSuccess = 0;
            break;
        }

    }
        
    searchPlantResultBuffer[index] = result;
    
}