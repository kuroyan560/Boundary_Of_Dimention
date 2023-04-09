#include"../../engine/ModelInfo.hlsli"
#include"../../engine/Camera.hlsli"
#include"../../engine/LightInfo.hlsli"
#include"../../engine/Math.hlsli"
#include"../../engine/ColorProcess.hlsli"

struct ToonCommonParameter
{
    float m_brightThresholdLow;
    float m_brightThresholdRange;
    float m_monochromeRate;
};

struct ToonIndividualParameter
{
    float4 m_fillColor;
    float4 m_brightMulColor;
    float4 m_darkMulColor;
    float4 m_edgeColor;
    int m_drawMask;
    int m_isPlayer;
};

struct PlayerInfo
{
    float3 m_worldPos;
    float m_depthInView;
    float2 m_screenPos;
};

cbuffer cbuff0 : register(b0)
{
    Camera cam;
}

cbuffer cbuff1 : register(b1)
{
    LightInfo ligNum; //�A�N�e�B�u���̃��C�g�̐��̏��
}

StructuredBuffer<DirectionLight> dirLight : register(t0);
StructuredBuffer<PointLight> pointLight : register(t1);
StructuredBuffer<SpotLight> spotLight : register(t2);
StructuredBuffer<HemiSphereLight> hemiSphereLight : register(t3);


cbuffer cbuff2 : register(b2)
{
    matrix world;
}

cbuffer cbuff3 : register(b3)
{
    matrix bones[256]; //�{�[���s��
}

Texture2D<float4> baseTex : register(t4);
SamplerState smp : register(s0);

cbuffer cbuff4 : register(b4)
{
    Material material;
}

cbuffer cbuff5 : register(b5)
{
    ToonCommonParameter toonCommonParam;
}

cbuffer cbuff6 : register(b6)
{
    ToonIndividualParameter toonIndividualParam;
}

cbuffer cbuff7 : register(b7)
{
    PlayerInfo player;
}

Texture2D<float> cloneDepthMap : register(t5);

struct VSOutput
{
    float4 svpos : SV_POSITION;
    float3 worldpos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float depthInView : CAM_Z;
};

VSOutput VSmain(Vertex input)
{
    float4 resultPos = input.pos;
	
	//�{�[���s��
	//BDEF4(�{�[��4����3�ƁA���ꂼ��̃E�F�C�g�l�B�E�F�C�g���v��1.0�ł���ۏ�͂��Ȃ�)
    if (input.boneNo[2] != NO_BONE)
    {
        int num;
        
        if (input.boneNo[3] != NO_BONE)    //�{�[���S��
        {
            num = 4;
        }
        else //�{�[���R��
        {
            num = 3;
        }
        
        matrix mat = bones[input.boneNo[0]] * input.weight[0];
        for (int i = 1; i < num; ++i)
        {
            mat += bones[input.boneNo[i]] * input.weight[i];
        }
        resultPos = mul(mat, input.pos);
    }
	//BDEF2(�{�[��2�ƁA�{�[��1�̃E�F�C�g�l(PMD����))
    else if (input.boneNo[1] != NO_BONE)
    {
        matrix mat = bones[input.boneNo[0]] * input.weight[0];
        mat += bones[input.boneNo[1]] * (1 - input.weight[0]);
        
        resultPos = mul(mat, input.pos);
    }
	//BDEF1(�{�[���̂�)
    else if (input.boneNo[0] != NO_BONE)
    {
        resultPos = mul(bones[input.boneNo[0]], input.pos);
    }
	
    VSOutput output;
    float4 wpos = mul(world, resultPos); //���[���h�ϊ�
    output.svpos = mul(cam.view, wpos); //�r���[�ϊ�
    output.depthInView = output.svpos.z;    //�J�������猩��Z
    output.svpos = mul(cam.proj, output.svpos); //�v���W�F�N�V�����ϊ�
    output.worldpos = wpos;
    output.normal = normalize(mul(world, input.normal));
    output.uv = input.uv;
    return output;
}

struct PSOutput
{
    float4 color : SV_Target0;
    float4 emissive : SV_Target1;
    float depth : SV_Target2;
    float4 normal : SV_Target3;
    float4 edgeColor : SV_Target4;
    float4 bright : SV_Target5;
};

void PSmain(VSOutput input, out PSOutput output) : SV_TARGET
{    
    float3 normal = input.normal;
    float3 vnormal = normalize(mul(cam.view, normal));
    
    //���C�g�̉e��
    float3 diffuseEf = { 0, 0, 0 };
    
    //�f�B���N�V�������C�g
    for (int i = 0; i < ligNum.dirLigNum; ++i)
    {
        if (!dirLight[i].active)continue;
        
        float3 dir = dirLight[i].direction;
        float3 ligCol = dirLight[i].color.xyz * dirLight[i].color.w;
        diffuseEf += CalcLambertDiffuse(dir, ligCol, normal) * (material.diffuse * material.diffuseFactor);
    }
    //�|�C���g���C�g
    for (int i = 0; i < ligNum.ptLigNum; ++i)
    {
        if (!pointLight[i].active)continue;
        
        float3 dir = input.worldpos - pointLight[i].pos;
        dir = normalize(dir);
        float3 ligCol = pointLight[i].color.xyz * pointLight[i].color.w;
        
        //�����Ȃ����
        float3 diffPoint = CalcLambertDiffuse(dir, ligCol, normal);
        
        //�����ɂ�錸��
        float3 distance = length(input.worldpos - pointLight[i].pos);
		//�e�����͋����ɔ�Ⴕ�ď������Ȃ��Ă���
        float affect = 1.0f - 1.0f / pointLight[i].influenceRange * distance;
		//�e���͂��}�C�i�X�ɂȂ�Ȃ��悤�ɕ␳��������
        if (affect < 0.0f)
            affect = 0.0f;
		//�e�����w���֐��I�ɂ���
        affect = pow(affect, 3.0f);
        diffPoint *= affect;
        
        diffuseEf += diffPoint * (material.diffuse * material.diffuseFactor);
    }
    //�X�|�b�g���C�g
    for (int i = 0; i < ligNum.spotLigNum; ++i)
    {
        if (!spotLight[i].active)continue;
        
        float3 ligDir = input.worldpos - spotLight[i].pos;
        ligDir = normalize(ligDir);
        float3 ligCol = spotLight[i].color.xyz * spotLight[i].color.w;
        
        //�����Ȃ����
        float3 diffSpotLight = CalcLambertDiffuse(ligDir, ligCol, normal);
        
        //�X�|�b�g���C�g�Ƃ̋������v�Z
        float3 distance = length(input.worldpos - spotLight[i].pos);
       	//�e�����͋����ɔ�Ⴕ�ď������Ȃ��Ă���
        float affect = 1.0f - 1.0f / spotLight[i].influenceRange * distance;
        //�e���͂��}�C�i�X�ɂȂ�Ȃ��悤�ɕ␳��������
        if (affect < 0.0f)
            affect = 0.0f;
    //�e�����w���֐��I�ɂ���
        affect = pow(affect, 3.0f);
        diffSpotLight *= affect;
    
        float3 spotlim = CalcLimLight(ligDir, ligCol, normal, vnormal) * affect;
        
        float3 dir = normalize(spotLight[i].target - spotLight[i].pos);
        float angle = dot(ligDir, dir);
        angle = abs(acos(angle));
        affect = 1.0f - 1.0f / spotLight[i].angle * angle;
        if (affect < 0.0f)
            affect = 0.0f;
        affect = pow(affect, 0.5f);
        
        diffuseEf += diffSpotLight * affect * (material.diffuse * material.diffuseFactor);
    }
    //�V��
    for (int i = 0; i < ligNum.hemiSphereNum; ++i)
    {
        if (!hemiSphereLight[i].active)continue;
        
        float t = dot(normal.xyz, hemiSphereLight[i].groundNormal);
        t = (t + 1.0f) / 2.0f;
        float3 hemiLight = lerp(hemiSphereLight[i].groundColor, hemiSphereLight[i].skyColor, t);
        diffuseEf += hemiLight;
    }
    
    float3 ligEffect = diffuseEf;
    
    float4 texCol = baseTex.Sample(smp, input.uv);
    texCol.xyz += material.baseColor.xyz;
    float4 ligEffCol = texCol;
    ligEffCol.xyz = ((material.ambient * material.ambientFactor) + ligEffect) * ligEffCol.xyz;
    ligEffCol.w *= (1.0f - material.transparent);
    
    //�A�j�����g�D�[�����H========================================================
    
    //�g�D�[���ɂ��F
    float4 toonCol = ligEffCol;
    
    //���邳�Z�o�i�Ɩ��e�����j
    float lightEffectBright = GetColorBright(ligEffect.xyz);

    //���邳�̂������l�ɉ����ĐF�����߂�
    float thresholdResult = smoothstep(toonCommonParam.m_brightThresholdLow, toonCommonParam.m_brightThresholdLow + toonCommonParam.m_brightThresholdRange, lightEffectBright);
    float4 brightCol = texCol * toonIndividualParam.m_brightMulColor * thresholdResult;
    float4 darkCol = texCol * toonIndividualParam.m_darkMulColor * (1.0f - thresholdResult);
    toonCol.xyz = brightCol + darkCol;
    float4 result = toonCol;

    //=========================================================================

    //�h��Ԃ�
    result.xyz = toonIndividualParam.m_fillColor.xyz * toonIndividualParam.m_fillColor.w + result.xyz * (1.0f - toonIndividualParam.m_fillColor.w);
    
    //�v���C���[�������Ƃ����ꍇ�̌��̓����������߂�
    float3 ligRay = input.worldpos - player.m_worldPos;
    float bright = dot(-normalize(ligRay), input.normal);
    //-1 ~ 1 ���� 0 ~ 1�͈̔͂Ɏ��߂�
    bright = saturate(bright);
    //���C�g�ɂ������Ă��邩�ǂ����������Ŕ��f
    float brightRange = 8.0f;
    int isBright = 1.0f - step(brightRange, length(ligRay) * int(bright == 0 ? brightRange : 1));
    if (toonIndividualParam.m_isPlayer)
        isBright = 1;
    result.xyz *= lerp(0.5f, 1.0f, isBright);
    
    //�����������Ă��Ȃ��Ȃ烂�m�N����
    result.xyz = lerp(lerp(result.xyz, Monochrome(result.xyz), toonCommonParam.m_monochromeRate), result.xyz, isBright);

    //���ɕ`�悳��Ă���s�N�Z���̐[�x�擾
    float2 texCoord = input.svpos.xy / float2(1280.0f, 720.0f);
    float recentDepth = cloneDepthMap.Sample(smp, texCoord).r;
    
    //�X�N���[�����W��Ńv���C���[�̍��W�𒆐S�Ƃ����~������
    if (!toonIndividualParam.m_isPlayer 
        && length(input.svpos.xy - player.m_screenPos) < 128.0f 
        && input.depthInView < recentDepth 
        && input.depthInView < player.m_depthInView)
    {
        discard;
    }
    
 //   if (toonIndividualParam.m_isPlayer || )
 //   {
 //   }
    
    output.color = result;
    
    //���邳�v�Z
// float bright = dot(result.xyz, float3(0.2125f, 0.7154f, 0.0721f));
// if (1.0f < bright)
//    output.emissive += result;
// output.emissive.w = result.w;
    output.emissive = float4(0, 0, 0, 0);
    
    output.depth = input.depthInView;
    
    output.normal.xyz = input.normal;

    output.edgeColor = toonIndividualParam.m_edgeColor * lerp(0.2f, 1.0f, isBright);
    
    output.bright.x = isBright;
}