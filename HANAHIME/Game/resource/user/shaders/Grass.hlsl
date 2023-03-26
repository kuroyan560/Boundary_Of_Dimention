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
    float sineLength : SINELENGTH;
};

//�W�I���g���V�F�[�_�[��ʂ����f�[�^
struct GSOutput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 toUV : TOUV;         //���݂̊p�x���狁�߂���UV
    float2 fromUV : FROMUV;         //�O��g�p����Ă���UV ��Ԃ����邽�߂Ɏg�p
    float uvLerpAmount : UVLERP;    //UV�̕�ԗ�
    uint texID : NEXTTexID;
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
    float sineWave;
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
    element.texID = input[0].texID;

    //�r���{�[�h�̃T�C�Y
    const float2 PolygonSize = float2(0.75f,3.0f);
    
    //�r���[�v���W�F�N�V����
    matrix viewproj = mul(cam.proj, cam.view);

    //�f�t�H���g���Ə��������Ă��܂��Ă���̂�1�������߂�B
    input[0].position.xyz -= input[0].normal;

    //�J���������x�N�g��
    float3 cameraVec = normalize(cameraPos - input[0].position);

    //�E�����x�N�g��
    float3 rightVec = normalize(cross(cameraVec, input[0].normal));

    //���ʃx�N�g��
    float3 forwardVec = normalize(cross(rightVec, input[0].normal));

    //�f�t�H���g�̐��ʃx�N�g���ƌ��݂̐��ʃx�N�g���̊p�x�����߂�B
    float3 defForwardVec = float3(0,0,1);
    float cosTheta = dot(defForwardVec, forwardVec) / (length(defForwardVec) * length(forwardVec));
    float angle = acos(cosTheta) * (180 / 3.14159265);
    //2�̃x�N�g���̈ʒu�֌W�ɂ���āA�p�x��0~360�x�͈̔͂ɏC������
    if (cross(defForwardVec, forwardVec).y < 0) {
        angle = 360 - angle;
    }
    float angle01 = angle / 360.0f; //0~1�̊p�x

    //1���������U���T�C�Y
    float textureSizeU = 1.0f / 20.0f;

    //��Ԑ��UV�����߂�B
    float invStep = 1.0f / textureSizeU;
    float toUVOffset =  floor(angle01 * invStep) / invStep;

    //��Ԍ���UV�����߂�B
    float fromUVOFfset = toUVOffset - textureSizeU;
    if(fromUVOFfset < 0){
        fromUVOFfset = 1.0f - textureSizeU;
    }

    //uvOffset�̏����_���ʂ����Ԃ̊��������߂�B Imposter�Ɋ܂܂�Ă���摜�̐���20���Ȃ̂ŁA0.05�Ԋu�Ŋ��������߂�B
    element.uvLerpAmount = abs(angle01 - toUVOffset) / textureSizeU;

    //����
    element.position = float4(input[0].position.xyz, 1.0f);
    element.position += float4(rightVec,0) * -PolygonSize.x;
    element.position = mul(viewproj, element.position);
    element.toUV = float2(toUVOffset,1);
    element.fromUV = float2(fromUVOFfset,1);
    output.Append(element);
    
    //����
    element.position = float4(input[0].position.xyz, 1.0f);         //���_���������B
    element.position += float4(rightVec,0) * -PolygonSize.x;        //���ֈړ�������B
    element.position += float4(input[0].normal,0) * PolygonSize.y;  //��ֈړ�������B
    element.position += float4(float3(0,0,1) * (sin(sineWave) * input[0].sineLength), 0.0f);  //����h�炷�B
    element.position = mul(viewproj, element.position);             //�J�������W��
    element.toUV = float2(toUVOffset,0);
    element.fromUV = float2(fromUVOFfset,0);
    output.Append(element);
    
    //�E��
    element.position = float4(input[0].position.xyz, 1.0f);
    element.position += float4(rightVec,0) * PolygonSize.x;
    element.position = mul(viewproj, element.position);
    element.toUV = float2(toUVOffset + textureSizeU,1);
    element.fromUV = float2(fromUVOFfset + textureSizeU,1);
    output.Append(element);
    
    //�E��
    element.position = float4(input[0].position.xyz, 1.0f);         //���_���������B
    element.position += float4(rightVec,0) * PolygonSize.x;         //�E�ֈړ�������B
    element.position += float4(input[0].normal,0) * PolygonSize.y;  //��ֈړ�������B
    element.position += float4(float3(0,0,1) * (sin(sineWave) * input[0].sineLength), 0.0f);  //����h�炷�B
    element.position = mul(viewproj, element.position);             //�J�������W��
    element.toUV = float2(toUVOffset + textureSizeU,0);
    element.fromUV = float2(fromUVOFfset + textureSizeU,0);
    output.Append(element);
}

PSOutput PSmain(GSOutput input)
{
    PSOutput output;

    //��Ԍ��ƕ�Ԑ�̐F���擾�B
    float4 fromColor;
    float4 toColor;

    if(input.texID == 0){

        toColor = tex_0.Sample(smp, input.toUV);
        fromColor = tex_0.Sample(smp, input.fromUV);

    }else if(input.texID == 1){
        
        toColor = tex_1.Sample(smp, input.toUV);
        fromColor = tex_1.Sample(smp, input.fromUV);

    }else if(input.texID == 2){
        
        toColor = tex_2.Sample(smp, input.toUV);
        fromColor = tex_2.Sample(smp, input.fromUV);

    }else if(input.texID == 3){
        
        toColor = tex_3.Sample(smp, input.toUV);
        fromColor = tex_3.Sample(smp, input.fromUV);

    }else{

        toColor = tex_2.Sample(smp, input.toUV);
        fromColor = tex_2.Sample(smp, input.fromUV);

    }

    //�F���Ԃ���B
    output.color = fromColor * (1.0f - input.uvLerpAmount) + toColor * input.uvLerpAmount;

    //�A���t�@�l�ɂ���ăN���b�v
    clip(output.color.a - 0.9f);

    output.emissive = float4(0,0,0,0);
    output.depth = float4(0,0,0,0);
    output.edgeColor = float4(0.13, 0.53, 0.40,1);
 
    return output;
}