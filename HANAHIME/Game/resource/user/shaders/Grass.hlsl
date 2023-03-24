#include"../../engine/Camera.hlsli"

//�ő�C���X�^���X���@��CPU���ƍ��킹��
static const int INSTANCE_MAX = 1024;

//���_�f�[�^
struct VSOutput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
};

//�W�I���g���V�F�[�_�[��ʂ����f�[�^
struct GSOutput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
};

//�s�N�Z���V�F�[�_�[��ʂ����f�[�^�i�����_�[�^�[�Q�b�g�ɏ������ރf�[�^�j
struct PSOutput
{
    float4 color : SV_Target0;
    float4 emissive : SV_Target1;
    float depth : SV_Target2;
    float4 edgeColor : SV_Target3;
};

//�萔�o�b�t�@�i�J�������j
cbuffer cbuff0 : register(b0)
{
    Camera cam;
};

//�萔�o�b�t�@�i�D���Ȃ̓���Ăˁj
cbuffer cbuff1 : register(b1)
{
    
}

//�e�N�X�`��
Texture2D<float4> tex_0 : register(t0);
Texture2D<float4> tex_1 : register(t1);
Texture2D<float4> tex_2 : register(t2);
Texture2D<float4> tex_3 : register(t3);
Texture2D<float4> tex_4 : register(t4);

//�T���v���[
SamplerState smp : register(s0);
    
VSOutput VSmain(VSOutput input)
{
    return input;
}

[maxvertexcount(4)]
void GSmain(
	point VSOutput input[1],
	inout TriangleStream<GSOutput> output
)
{
    GSOutput element;
        
    //����
    element.position = float4(input[0].position.xyz, 1.0f);
    output.Append(element);
    
    //����
    element.position = float4(input[0].position.xyz, 1.0f);
    output.Append(element);
    
     //�E��
    element.position = float4(input[0].position.xyz, 1.0f);
    output.Append(element);
    
    //�E��
    element.position = float4(input[0].position.xyz, 1.0f);
    output.Append(element);
}

PSOutput PSmain(GSOutput input)
{
    PSOutput output;
 
    return output;
}