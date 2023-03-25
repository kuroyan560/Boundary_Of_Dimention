#include"../../engine/Camera.hlsli"

//�ő�C���X�^���X���@��CPU���ƍ��킹��
static const int INSTANCE_MAX = 1024;

//���_�f�[�^
struct VSOutput
{
    float3 position : POSITION;
    uint texID : TexID;
    float3 normal : NORMAL;
    uint isAlive : IsAlive;
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
    float3 cameraPos;
    uint pad;
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

//�x�N�g���ɃX�J���[�l��������B
float4 Scale(float4 In, float Scalar){
    return float4(In.x * Scalar,In.y * Scalar,In.z * Scalar,In.w * Scalar);
}
float4 Scale(float3 In, float Scalar){
    return float4(In.x * Scalar,In.y * Scalar,In.z * Scalar,1 * Scalar);
}

[maxvertexcount(6)]
void GSmain(
	point VSOutput input[1],
	inout TriangleStream<GSOutput> output
)
{
    GSOutput element;

    //�r���{�[�h�̃T�C�Y
    const float2 PolygonSize = float2(0.5f,2.0f);
    
    //�r���[�v���W�F�N�V����
    matrix viewproj = mul(cam.proj, cam.view);

    //�f�t�H���g���Ə��������Ă��܂��Ă���̂�1�������߂�B
    input[0].position.xyz -= Scale(input[0].normal, 1);

    //�J���������x�N�g��
    float3 cameraVec = normalize(cameraPos - input[0].position);

    //�E�����x�N�g��
    float3 rightVec = -normalize(cross(input[0].normal, cameraVec));

    //����
    element.position = float4(input[0].position.xyz, 1.0f);
    element.position += Scale(float4(rightVec,0), -PolygonSize.x);
    element.position = mul(viewproj, element.position);
    output.Append(element);
    
    //����
    element.position = float4(input[0].position.xyz, 1.0f);
    element.position += Scale(float4(rightVec,0), -PolygonSize.x);
    element.position += Scale(float4(input[0].normal,0), PolygonSize.y);
    element.position = mul(viewproj, element.position);
    output.Append(element);
    
    //�E��
    element.position = float4(input[0].position.xyz, 1.0f);
    element.position += Scale(float4(rightVec,0), PolygonSize.x);
    element.position = mul(viewproj, element.position);
    output.Append(element);

    output.RestartStrip();
    
    //����
    element.position = float4(input[0].position.xyz, 1.0f);
    element.position += Scale(float4(rightVec,0), -PolygonSize.x);
    element.position += Scale(float4(input[0].normal,0), PolygonSize.y);
    element.position = mul(viewproj, element.position);
    output.Append(element);
    
    //�E��
    element.position = float4(input[0].position.xyz, 1.0f);
    element.position += Scale(float4(rightVec,0), PolygonSize.x);
    element.position += Scale(float4(input[0].normal,0), PolygonSize.y);
    element.position = mul(viewproj, element.position);
    output.Append(element);
    
    //�E��
    element.position = float4(input[0].position.xyz, 1.0f);
    element.position += Scale(float4(rightVec,0), PolygonSize.x);
    element.position = mul(viewproj, element.position);
    output.Append(element);

    output.RestartStrip();
}

PSOutput PSmain(GSOutput input)
{
    PSOutput output;

    output.color = float4(1,0,0,1);
    output.emissive = float4(1,0,0,1);
    output.depth = float4(1,0,0,1);
    output.edgeColor = float4(1,0,0,1);
 
    return output;
}