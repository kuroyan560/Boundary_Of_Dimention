#include"../../engine/Camera.hlsli"
#include"Grass.hlsli"


RWStructuredBuffer<bool> checkBrightResultBuffer : register(u0);
StructuredBuffer<float3> grassPosArrayBuffer : register(t0);
Texture2D<float4> g_brightMap : register(t1);

cbuffer cbuff0 : register(b0)
{
    Camera cam;
}

cbuffer cbuff1 : register(b1)
{
    PlayerInfo player;
}

[numthreads(1, 1, 1)]
void CheckBright(uint DTid : SV_DispatchThreadID)
{
    //���̍��W�擾
    float3 grassPos = grassPosArrayBuffer[DTid];
    
    //���݈ʒu�Ƀ��C�g���������Ă��邩���m�F����B
    float4 viewPos = mul(cam.view, float4(grassPos, 1.0f));
    float4 clipPos = mul(cam.proj, viewPos);
    float3 ndcPos = clipPos.xyz / clipPos.w;
    uint2 screenPos = round(float2((ndcPos.x * 0.5f + 0.5f) * 1280.0f, (1.0f - (ndcPos.y * 0.5f + 0.5f)) * 720.0f));
    float texColor = g_brightMap[screenPos].x;
    
    //�v���C���[�Ƃ̋���
    float playerDistance = length(grassPos - player.m_pos);
    bool isNear = playerDistance <= player.m_plantLightRange;
  
    bool isBright = false;
    
    //���ɂ������Ă����瑐�𐶂₵�A �������Ă��Ȃ�������C�[�W���O�^�C�}�[�����炵�đ����͂炷�B
    if (0.9f < texColor || isNear)
        isBright = true;
    
    //���ʂ̊i�[
    checkBrightResultBuffer[DTid] = isBright;
};