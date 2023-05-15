#include"../../engine/Camera.hlsli"
#include"../../engine/ModelInfo.hlsli"
#include"Grass.hlsli"

//���_�f�[�^
struct VSOutput
{
    float4 svpos : SV_POSITION;
    float3 worldpos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float depthInView : CAM_Z;
};

//�s�N�Z���V�F�[�_�[��ʂ����f�[�^�i�����_�[�^�[�Q�b�g�ɏ������ރf�[�^�j
struct PSOutput
{
    float4 color : SV_Target0;
    float4 emissive : SV_Target1;
    float depth : SV_Target2;
    float4 edgeColor : SV_Target3;
    float4 bright : SV_Target4;
};

StructuredBuffer<matrix> grassWorldMatArrayBuffer : register(t0);

//�萔�o�b�t�@�i�J�������j
cbuffer cbuff0 : register(b0)
{
    Camera cam;
};

cbuffer cbuff1 : register(b1)
{
    Material material;
}

cbuffer cbuff2 : register(b2)
{
    PlayerInfo player;
}

//�e�N�X�`��
Texture2D<float4> baseTex : register(t1);

//�T���v���[
SamplerState smp : register(s0);
    
VSOutput VSmain(Vertex input, uint instanceID : SV_InstanceID)
{
    VSOutput output;
    float4 wpos = mul(grassWorldMatArrayBuffer[instanceID], input.pos);
    output.svpos = mul(cam.view, wpos);
    output.depthInView = output.svpos.z;
    output.svpos = mul(cam.proj, output.svpos);
    output.worldpos = wpos;
    output.normal = input.normal;
    output.uv = input.uv;
    return output;
}

PSOutput PSmain(VSOutput input)
{
    PSOutput output;
    
    //�@�����擾����B
    float3 normal = input.normal;
    
    //�v���C���[�ƕ`�悷����W�̃x�N�g��
    //float3 playerDir = normalize(input.worldPosition.xyz - commonInfo.m_playerPos);
    float3 lightDir = normalize(player.m_pos - cam.eyePos);
    
    //���������߂�B
    float distance = length(input.worldpos.xyz - player.m_pos);
    
    //�����ɂ���Ė��邳�̊�����ς���B
    const float DISTANCE = 15.0f;
    const float OFFSET_DISTANCE_LUMI = 0.2f; //��������������Ă��Ă�������x���邳���o�����߂̃I�t�Z�b�g�B
    float distanceRate = clamp(step(distance, DISTANCE), OFFSET_DISTANCE_LUMI, 1.0f);
    
    //���邳�����߂�B
    float lumi = dot(normal, lightDir) * distanceRate;
    
    //���邳�̃I�t�Z�b�g
    const float OFFSET_LUMI = 0.3f;
    lumi = clamp(lumi + OFFSET_LUMI, OFFSET_LUMI, 1.0f);
    
    //�����ɂ���čŏI�I�Ȗ��邳���N�����v����B
    const float IN_CIRCLE_LUMI = 0.5f;
    lumi = clamp(lumi, step(distance, DISTANCE) * IN_CIRCLE_LUMI, 1.0f);
    
    //float bright = distance;

    float4 color = baseTex.Sample(smp, input.uv);
    //�F��ۑ�����B
    color.xyz *= lumi;
    output.color = color;

    output.bright = float4(0, 0, 0, 0);
    output.depth = input.depthInView;
    output.edgeColor = float4(0.13, 0.53, 0.40, 1);
    output.emissive = float4(0, 0, 0, 0);
    if (distance < DISTANCE)
    {
        output.emissive = output.color / 3.0f;
    }
 
    return output;
}