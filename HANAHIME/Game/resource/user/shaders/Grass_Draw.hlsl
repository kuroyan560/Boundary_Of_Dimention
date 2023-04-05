#include"../../engine/Camera.hlsli"
#include"Grass.hlsli"

//���_�f�[�^
struct VSOutput
{
    float3 position : POSITION;
    uint instanceID : INSTANCE_ID;
};

//�W�I���g���V�F�[�_�[��ʂ����f�[�^
struct GSOutput
{
    float4 position : SV_POSITION;
    float4 worldPosition : WORLD_POSITION;
    float3 normal : NORMAL;
    float2 toUV : TOUV; //���݂̊p�x���狁�߂���UV
    float2 fromUV : FROMUV; //�O��g�p����Ă���UV ��Ԃ����邽�߂Ɏg�p
    float uvLerpAmount : UVLERP; //UV�̕�ԗ�
    uint texID : TexID; //�g�p����e�N�X�`����ID
    float depthInView : CAM_Z; //�J�����܂ł̋����i�[�x�j
};

//�s�N�Z���V�F�[�_�[��ʂ����f�[�^�i�����_�[�^�[�Q�b�g�ɏ������ރf�[�^�j
struct PSOutput
{
    float4 color : SV_Target0;
    float4 emissive : SV_Target1;
    float depth : SV_Target2;
    float4 normal : SV_Target3;
    float4 edgeColor : SV_Target4;
};

RWStructuredBuffer<PlantGrass> aliveGrassBuffer : register(u0);

//�萔�o�b�t�@�i�J�������j
cbuffer cbuff0 : register(b0)
{
    Camera cam;
};

//�萔�o�b�t�@�i�D���Ȃ̓���Ăˁj
cbuffer cbuff1 : register(b1)
{
    CommonGrassInfo commonInfo;
}

cbuffer cbuff2 : register(b2)
{
    TransformData otherTransformData;
}

//�e�N�X�`��
Texture2D<float4> tex_0 : register(t0);
Texture2D<float4> tex_1 : register(t1);
Texture2D<float4> tex_2 : register(t2);
Texture2D<float4> tex_3 : register(t3);
Texture2D<float4> tex_4 : register(t4);
Texture2D<float4> normalTex_0 : register(t5);
Texture2D<float4> normalTex_1 : register(t6);
Texture2D<float4> normalTex_2 : register(t7);
Texture2D<float4> normalTex_3 : register(t8);
Texture2D<float4> normalTex_4 : register(t9);

//�T���v���[
SamplerState smp : register(s0);
    
VSOutput VSmain(float3 pos : POSITION, uint instanceID : SV_InstanceID)
{
    VSOutput output;
    output.position = pos;
    output.instanceID = instanceID;
    return output;
}

[maxvertexcount(4)]
void GSmain(
	point VSOutput input[1],
	inout TriangleStream<GSOutput> output)
{
    PlantGrass grass = aliveGrassBuffer[input[0].instanceID];
    
    GSOutput element;
    element.texID = grass.m_texIdx;
    element.normal = grass.m_normal;
    float3 position = grass.m_pos;

    //�r���{�[�h�̃T�C�Y
    const float2 PolygonSize = float2(0.75f, 3.0f);
    
    //�f�t�H���g���Ə��������Ă��܂��Ă���̂�1.5�������߂�B
    position -= grass.m_normal * 1.5f;

    //�J���������x�N�g��
    float3 cameraVec = normalize(otherTransformData.m_camPos - position);

    //�E�����x�N�g��
    float3 rightVec = normalize(cross(cameraVec, grass.m_normal));

    //���ʃx�N�g��
    float3 forwardVec = normalize(cross(rightVec, grass.m_normal));

    //�f�t�H���g�̐��ʃx�N�g���ƌ��݂̐��ʃx�N�g���̊p�x�����߂�B
    float3 defForwardVec = float3(0, 0, 1);
    float cosTheta = dot(defForwardVec, forwardVec) / (length(defForwardVec) * length(forwardVec));
    float angle = acos(cosTheta) * (180 / 3.14159265);
    //2�̃x�N�g���̈ʒu�֌W�ɂ���āA�p�x��0~360�x�͈̔͂ɏC������
    if (cross(defForwardVec, forwardVec).y < 0)
    {
        angle = 360 - angle;
    }
    float angle01 = angle / 360.0f; //0~1�̊p�x

    //1���������U���T�C�Y
    float textureSizeU = 1.0f / 20.0f;

    //��Ԑ��UV�����߂�B
    float invStep = 1.0f / textureSizeU;
    float toUVOffset = floor(angle01 * invStep) / invStep;

    //��Ԍ���UV�����߂�B
    float fromUVOFfset = toUVOffset - textureSizeU;
    if (fromUVOFfset < 0)
    {
        fromUVOFfset = 1.0f - textureSizeU;
    }

    //uvOffset�̏����_���ʂ����Ԃ̊��������߂�B Imposter�Ɋ܂܂�Ă���摜�̐���20���Ȃ̂ŁA0.05�Ԋu�Ŋ��������߂�B
    element.uvLerpAmount = abs(angle01 - toUVOffset) / textureSizeU;

    //���̋��� �o���x�����𕗂̑傫���ɂ����邱�Ƃŏo�������͗h��Ȃ��悤�ɂ���B
    float windPower = grass.m_sineLength * grass.m_appearY;
    float4 windPos = float4(float3(0, 0, 1) * (sin(commonInfo.m_sineWave) * windPower), 0.0f);

    //���̍��� �o���x�����𕗂̍����ɂ����邱�Ƃł��񂾂񐶂���悤�ɂ���B
    float grassHeight = grass.m_appearY * PolygonSize.y;
    
    //�@�����ɋ��߂����̂Ń��[���h���W�����O�ɋ��߂�
    float4 worldPos[4];
    //����
    worldPos[0] = float4(position, 1.0f); //���_��������
    worldPos[0] += float4(rightVec, 0) * -PolygonSize.x; //�������Ɉړ�������B
    //����
    worldPos[1] = float4(position, 1.0f); //���_���������B
    worldPos[1] += float4(rightVec, 0) * -PolygonSize.x; //���ֈړ�������B
    worldPos[1] += float4(grass.m_normal, 0) * grassHeight; //��ֈړ�������B
    worldPos[1] += windPos; //����h�炷�B
    //�E��
    worldPos[2] = float4(position, 1.0f); //���_��������
    worldPos[2] += float4(rightVec, 0) * PolygonSize.x; //�E�ֈړ�������B
    //�E��
    worldPos[3] = float4(position, 1.0f); //���_���������B
    worldPos[3] += float4(rightVec, 0) * PolygonSize.x; //�E�ֈړ�������B
    worldPos[3] += float4(grass.m_normal, 0) * grassHeight; //��ֈړ�������B
    worldPos[3] += windPos; //����h�炷�B
    
    //�@�������߂�
    float3 vecA = worldPos[1].xyz - worldPos[0].xyz;
    float3 vecB = worldPos[2].xyz - worldPos[0].xyz;
    element.normal.xyz = normalize(cross(vecA, vecB));
    
    /*-- ���� --*/
    //���W�����߂�B
    element.worldPosition = worldPos[0];
    element.position = mul(cam.view, worldPos[0]); //�J�������W��
    element.depthInView = element.position.z;
    element.position = mul(cam.proj, element.position);
    //UV�����߂�B
    element.toUV = float2(toUVOffset, 1); //��Ԑ��UV
    element.fromUV = float2(fromUVOFfset, 1); //��Ԍ���UV
    output.Append(element);
    
    /*-- ���� --*/
    //���W�����߂�B
    element.worldPosition = worldPos[1];
    element.position = mul(cam.view, worldPos[1]); //�J�������W��
    element.depthInView = element.position.z;
    element.position = mul(cam.proj, element.position);
    //UV�����߂�B
    element.toUV = float2(toUVOffset, (1.0f - grass.m_appearY)); //��Ԑ��UV
    element.fromUV = float2(fromUVOFfset, (1.0f - grass.m_appearY)); //��Ԍ���UV
    output.Append(element);
    
    /*-- �E�� --*/
    //���W�����߂�B
    element.worldPosition = worldPos[2];
    element.position = mul(cam.view, worldPos[2]); //�J�������W��
    element.depthInView = element.position.z;
    element.position = mul(cam.proj, element.position);
    //UV�����߂�B
    element.toUV = float2(toUVOffset + textureSizeU, 1); //��Ԑ��UV
    element.fromUV = float2(fromUVOFfset + textureSizeU, 1); //��Ԍ���UV
    output.Append(element);
    
    /*-- �E�� --*/
    //���W�����߂�B
    element.worldPosition = worldPos[3];
    element.position = mul(cam.view, worldPos[3]); //�J�������W��
    element.depthInView = element.position.z;
    element.position = mul(cam.proj, element.position);
    //UV�����߂�B
    element.toUV = float2(toUVOffset + textureSizeU, (1.0f - grass.m_appearY)); //��Ԑ��UV
    element.fromUV = float2(fromUVOFfset + textureSizeU, (1.0f - grass.m_appearY)); //��Ԍ���UV
    output.Append(element);
}

PSOutput PSmain(GSOutput input)
{
    PSOutput output;

    //�F���擾�B
    float4 color;
    float4 normalColor;

    if (input.texID == 0)
    {

        color = tex_0.Sample(smp, input.toUV);
        normalColor = normalTex_0.Sample(smp, input.toUV);

    }
    else if (input.texID == 1)
    {
        
        color = tex_1.Sample(smp, input.toUV);
        normalColor = normalTex_1.Sample(smp, input.toUV);

    }
    else if (input.texID == 2)
    {
        
        color = tex_2.Sample(smp, input.toUV);
        normalColor = normalTex_2.Sample(smp, input.toUV);

    }
    else if (input.texID == 3)
    {
        
        color = tex_3.Sample(smp, input.toUV);
        normalColor = normalTex_3.Sample(smp, input.toUV);

    }
    else
    {

        color = tex_2.Sample(smp, input.toUV);
        normalColor = normalTex_2.Sample(smp, input.toUV);

    }

    //�A���t�@�l�ɂ���ăN���b�v
    clip(color.a - 0.9f);
    
    //�@�����擾����B
    float3 normal = normalColor.xyz * 2.0f - float3(1.0f, 1.0f, 1.0f);
    
    //�v���C���[�ƕ`�悷����W�̃x�N�g��
    float3 playerDir = normalize(input.worldPosition.xyz - commonInfo.m_playerPos);
    
    //���������߂�B
    float distance = length(input.worldPosition.xyz - commonInfo.m_playerPos);
    
    //�����ɂ���Ė��邳�̊�����ς���B
    const float DISTANCE = 10.0f;
    float distanceRate = step(distance, DISTANCE);
    
    //���邳�̃I�t�Z�b�g
    const float OFFSET_LUMI = 0.4f;
    distanceRate = clamp(distanceRate + OFFSET_LUMI, 0.0f, 1.0f);
    
    //���邳�����߂�B
    float lumi = dot(playerDir, normal) * distanceRate;

    //�F��ۑ�����B
    color.xyz *= lumi;
    output.color = color;

    output.emissive = float4(0, 0, 0, 0);
    output.depth = input.depthInView;
    output.normal.xyz = input.normal;
    output.edgeColor = float4(0.13, 0.53, 0.40, 1);
 
    return output;
}