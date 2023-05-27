#include"../../engine/Math.hlsli"
#include"../../engine/ColorProcess.hlsli"
#include"BasicDraw.hlsli"

cbuffer cbuff2 : register(b2)
{
    matrix world;
}


//�G�ۉe�p ===================================================
cbuffer cbuff9 : register(b9)
{
    uint circleShadowCount;
}
struct CircleShadowData
{
    float3 pos;
    float3 up;
    float shadowRadius;
};
StructuredBuffer<CircleShadowData> circleShadowData : register(t7);
//�G�ۉe�p ===================================================

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
    output.depthInView = output.svpos.z; //�J�������猩��Z
    output.svpos = mul(cam.proj, output.svpos); //�v���W�F�N�V�����ϊ�
    output.worldpos = wpos;
    output.normal = normalize(mul(world, input.normal));
    output.uv = input.uv;
    return output;
}

struct PSOutputStage
{
    float4 color : SV_Target0;
    float4 emissive : SV_Target1;
    float depth : SV_Target2;
    float4 edgeColor : SV_Target3;
    float4 bright : SV_Target4;
    float4 normal : SV_Target5;
    float4 normalGrass : SV_Target6;
    float4 worldPos : SV_Target7;
};

PSOutputStage PSmain(VSOutput input) : SV_TARGET
{
    
    
    float3 normal = input.normal;
    float3 vnormal = normalize(mul(cam.view, normal));
    
    //���C�g�̉e��
    float3 diffuseEf = { 0, 0, 0 };
    
    ////�f�B���N�V�������C�g
    //for (int i = 0; i < ligNum.dirLigNum; ++i)
    //{
    //    if (!dirLight[i].active)
    //        continue;
        
    //    float3 dir = dirLight[i].direction;
    //    float3 ligCol = dirLight[i].color.xyz * dirLight[i].color.w;
    //    diffuseEf += CalcLambertDiffuse(dir, ligCol, normal) * (material.diffuse * material.diffuseFactor);
    //}
        
    float3 dir = float3(1,1,0);
    float3 ligCol = float3(1, 1, 1);
    diffuseEf += CalcLambertDiffuse(normalize(dir), ligCol, normal) * (material.diffuse * material.diffuseFactor);
        
    dir = float3(-1, -1, 0);
    diffuseEf += CalcLambertDiffuse(normalize(dir), ligCol, normal) * (material.diffuse * material.diffuseFactor);
        
    dir = float3(0, 1, 1);
    diffuseEf += CalcLambertDiffuse(normalize(dir), ligCol, normal) * (material.diffuse * material.diffuseFactor);
        
    dir = float3(0, -1, -1);
    diffuseEf += CalcLambertDiffuse(normalize(dir), ligCol, normal) * (material.diffuse * material.diffuseFactor);
    
    diffuseEf = clamp(diffuseEf, 0.5, 1.0f);
    
    //�|�C���g���C�g
    for (int i = 0; i < ligNum.ptLigNum; ++i)
    {
        if (!pointLight[i].active)
            continue;
        
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
        if (!spotLight[i].active)
            continue;
        
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
        if (!hemiSphereLight[i].active)
            continue;
        
        float t = dot(normal.xyz, hemiSphereLight[i].groundNormal);
        t = (t + 1.0f) / 2.0f;
        float3 hemiLight = lerp(hemiSphereLight[i].groundColor, hemiSphereLight[i].skyColor, t);
        diffuseEf += hemiLight;
    }
    
    float3 ligEffect = diffuseEf;

    float4 texCol = baseTex.Sample(smp, input.uv);
    texCol.xyz += material.baseColor.xyz;
    float4 ligEffCol = texCol;
    //ligEffCol.xyz = ((material.ambient * material.ambientFactor) + ligEffect) * ligEffCol.xyz;
    ligEffCol.xyz = ligEffect * ligEffCol.xyz;
    ligEffCol.w *= (1.0f - material.transparent);
    
    //�A�j�����g�D�[�����H========================================================
    
    ////�g�D�[���ɂ��F
    //float4 toonCol = ligEffCol;
    
    ////���邳�Z�o�i�Ɩ��e�����j
    //float lightEffectBright = GetColorBright(ligEffect.xyz);

    ////���邳�̂������l�ɉ����ĐF�����߂�
    //float thresholdResult = smoothstep(toonCommonParam.m_brightThresholdLow, toonCommonParam.m_brightThresholdLow + toonCommonParam.m_brightThresholdRange, lightEffectBright);
    //float4 brightCol = texCol * toonIndividualParam.m_brightMulColor * thresholdResult;
    //float4 darkCol = texCol * toonIndividualParam.m_darkMulColor * (1.0f - thresholdResult);
    //toonCol.xyz = brightCol + darkCol;
    //float4 result = toonCol;
    float4 result = ligEffCol;

    //=========================================================================

    //�h��Ԃ�
    //result.xyz = toonIndividualParam.m_fillColor.xyz * toonIndividualParam.m_fillColor.w + result.xyz * (1.0f - toonIndividualParam.m_fillColor.w);
    
    int isBright = 0;
    int isBrightDefRange = 0; //�f�t�H���g�̃��C�g�̉e���x�͈̔�
    
    //�A����ɐB������|�C���g���C�g
    for (int i = 0; i < ligNum_Plant.ptLigNum; ++i)
    {
        if (!pointLight_Plant[i].m_active)
            continue;
        
        float3 ligRay = input.worldpos - pointLight_Plant[i].m_pos;
        float bright = dot(-normalize(ligRay), input.normal);
        //-1 ~ 1 ���� 0 ~ 1�͈̔͂Ɏ��߂�
        bright = saturate(bright);
        isBright += 1.0f - step(pointLight_Plant[i].m_influenceRange, length(ligRay) * int(bright == 0 ? pointLight_Plant[i].m_influenceRange : 1));
        isBrightDefRange += 1.0f - step(pointLight_Plant[i].m_defInfluenceRange, length(ligRay) * int(bright == 0 ? pointLight_Plant[i].m_defInfluenceRange : 1));
        
    }
    //�A����ɐB������X�|�b�g���C�g
    for (int i = 0; i < ligNum_Plant.spotLigNum; ++i)
    {
        if (!spotLight_Plant[i].m_active)
            continue;
    }
    
    float3 albedo = result;
    
    isBright = min(isBright, 1);
    isBrightDefRange = min(isBrightDefRange, 1);
    result.xyz *= lerp(0.7f, 1.0f, saturate(isBright + isBrightDefRange));
    
    //�����������Ă��Ȃ��Ȃ烂�m�N����
    result.xyz = lerp(lerp(result.xyz, Monochrome(result.xyz), toonCommonParam.m_monochromeRate), result.xyz, isBright);
    
    //�A���t�@�l�K�p
    result.w *= toonIndividualParam.m_alpha;
    
    
    
    
    //�����ł���ɓG�p�̊ۉe���o���B
    for (int index = 0; index < circleShadowCount; ++index)
    {
    
        //���������ȏ㗣��Ă������΂��B
        float distance = length(input.worldpos - circleShadowData[index].pos);
        if (circleShadowData[index].shadowRadius < distance)
            continue;
        
        float distanceRate = distance / circleShadowData[index].shadowRadius;
        
        float bright = Easing_Cubic_In(distanceRate, 1.0f, 0.0f, 1.0f);
        
        result.xyz *=  bright;        
    }
    
    
    
    
    PSOutputStage output;
    output.color = result;
    
    //���邳�v�Z
    output.emissive = emissiveTex.Sample(smp, input.uv) * 0.5f;
    output.emissive.xyz += material.emissive * material.emissiveFactor;
    output.emissive.xyz *= output.emissive.w;
    output.emissive.w = 1.0f;
    
    output.depth = input.depthInView;

    output.edgeColor = toonIndividualParam.m_edgeColor;
    output.edgeColor.xyz *= lerp(0.9f, 1.0f, isBright);
    
    output.bright.x = isBright;
    output.bright.y = saturate(isBright + isBrightDefRange);
    
    output.worldPos = float4(input.worldpos, 1.0f);
    
    output.normal.xyz = input.normal;
    output.normalGrass.xyz = input.normal;

    return output;
}