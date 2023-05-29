#include"../../engine/Math.hlsli"
#include"../../engine/ColorProcess.hlsli"
#include"BasicDraw.hlsli"

static const int MAX_NUM = 1024;
cbuffer cbuff2 : register(b2)
{
    matrix world[MAX_NUM];
}

cbuffer cbuff9 : register(b9)
{
    float noiseTimer;
}

StructuredBuffer<float> smokeAlpha : register(t8);

struct VSOutput
{
    float4 svpos : SV_POSITION;
    float3 worldpos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float depthInView : CAM_Z;
    int instanceIndex : INSTANCE;
};

//3D�̃����_���n�b�V���֐�
float3 Random3(float3 st)
{
    float3 seed = float3(dot(st, float3(127.1, 311.7, 523.3)), dot(st, float3(269.5, 183.3, 497.5)), dot(st, float3(419.2, 371.9, 251.6)));
    return -1.0 + 2.0 * frac(sin(seed) * 43758.5453123);
}

//3D�O���f�B�G���g�m�C�Y�֐�
float Noise(float3 st)
{
    float3 i = floor(st);
    float3 f = frac(st);

    //���̗אړ_�̍��W�����߂�
    float3 u = f * f * (3.0 - 2.0 * f);

    float a = dot(Random3(i), f - float3(0, 0, 0));
    float b = dot(Random3(i + float3(1, 0, 0)), f - float3(1, 0, 0));
    float c = dot(Random3(i + float3(0, 1, 0)), f - float3(0, 1, 0));
    float d = dot(Random3(i + float3(1, 1, 0)), f - float3(1, 1, 0));
    float e = dot(Random3(i + float3(0, 0, 1)), f - float3(0, 0, 1));
    float f1 = dot(Random3(i + float3(1, 0, 1)), f - float3(1, 0, 1));
    float g = dot(Random3(i + float3(0, 1, 1)), f - float3(0, 1, 1));
    float h = dot(Random3(i + float3(1, 1, 1)), f - float3(1, 1, 1));

    //�m�C�Y�l���Ԃ���
    float x1 = lerp(a, b, u.x);
    float x2 = lerp(c, d, u.x);
    float y1 = lerp(e, f1, u.x);
    float y2 = lerp(g, h, u.x);

    float xy1 = lerp(x1, x2, u.y);
    float xy2 = lerp(y1, y2, u.y);

    return lerp(xy1, xy2, u.z);
}

//3D�p�[�����m�C�Y�֐��i���̕\���t���j
float3 PerlinNoiseWithWind(float3 st, int octaves, float persistence, float lacunarity, float windStrength, float windSpeed, float t, float3 worldPos, float threshold)
{
    float amplitude = 1.0;

    // ���̉e�����v�Z
    float3 windDirection = normalize(float3(1, 0, 0)); // ���̕�����ݒ�i���̏ꍇ�� (1, 0, 0) �̕����j
    float3 windEffect = windDirection * windStrength * (t * windSpeed);

    // �v���C���[�̃��[���h���W�Ɋ�Â��m�C�Y����
    float3 worldSpaceCoords = st + worldPos / 100.0f;

    float3 noiseValue = float3(0, 0, 0);

    for (int j = 0; j < 3; ++j)
    {
        float frequency = pow(2.0, float(j));
        float localAmplitude = amplitude;
        float sum = 0.0;
        float maxValue = 0.0;
        
        for (int i = 0; i < octaves; ++i)
        {
            float3 dist = (worldSpaceCoords + windEffect) * frequency + (t + windEffect.x);
            float radius = length(dist);
            sum += localAmplitude * (1.0 - radius) * Noise(dist); // �m�C�Y�֐��ɋ����ɂ��e���𓝍�
            maxValue += localAmplitude;

            localAmplitude *= persistence;
            frequency *= lacunarity;
        }

        noiseValue[j] = (sum / maxValue + 1.0) * 0.5; // �m�C�Y�l��0.0����1.0�͈̔͂ɍă}�b�s���O

        if (noiseValue[j] <= threshold)
        {
            noiseValue[j] = 0.0;
        }
    }

    return noiseValue;
}

VSOutput VSmain(Vertex input, uint instanceID : SV_InstanceID)
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
    float4 wpos = mul(world[instanceID], resultPos); //���[���h�ϊ�
    output.svpos = mul(cam.view, wpos); //�r���[�ϊ�
    output.depthInView = output.svpos.z; //�J�������猩��Z
    output.svpos = mul(cam.proj, output.svpos); //�v���W�F�N�V�����ϊ�
    output.worldpos = wpos;
    output.normal = normalize(mul(world[instanceID], float4(input.normal, 1))).xyz;
    output.uv = input.uv;
    output.instanceIndex = instanceID;
    return output;
}

struct ParticlePSOutput
{
    float4 color : SV_Target0;
    float4 emissive : SV_Target1;
    float depth : SV_Target2;
};

ParticlePSOutput PSmain(VSOutput input) : SV_TARGET
{
    float3 normal = input.normal;
    float3 vnormal = normalize(mul(cam.view, normal));
    
    //���C�g�̉e��
    float3 diffuseEf = { 0, 0, 0 };
    float3 specularEf = { 0, 0, 0 };
    float3 limEf = { 0, 0, 0 };
    
    //�f�B���N�V�������C�g
    for (int i = 0; i < ligNum.dirLigNum; ++i)
    {
        if (!dirLight[i].active)
            continue;
        
        float3 dir = dirLight[i].direction;
        float3 ligCol = dirLight[i].color.xyz * dirLight[i].color.w;
        diffuseEf += CalcLambertDiffuse(dir, ligCol, normal) * (material.diffuse * material.diffuseFactor);
        specularEf += CalcPhongSpecular(dir, ligCol, normal, input.worldpos, cam.eyePos) * (material.specular * material.specularFactor);
        limEf += CalcLimLight(dir, ligCol, normal, vnormal);
    }
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
        float3 specPoint = CalcPhongSpecular(dir, ligCol, normal, input.worldpos, cam.eyePos);
        
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
        specPoint *= affect;
        
        diffuseEf += diffPoint * (material.diffuse * material.diffuseFactor);
        specularEf += specPoint * (material.specular * material.specularFactor);
        limEf += CalcLimLight(dir, ligCol, normal, vnormal);
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
        float3 specSpotLight = CalcPhongSpecular(ligDir, ligCol, normal, input.worldpos, cam.eyePos);
        
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
        specSpotLight *= affect;
    
        float3 spotlim = CalcLimLight(ligDir, ligCol, normal, vnormal) * affect;
        
        float3 dir = normalize(spotLight[i].target - spotLight[i].pos);
        float angle = dot(ligDir, dir);
        angle = abs(acos(angle));
        affect = 1.0f - 1.0f / spotLight[i].angle * angle;
        if (affect < 0.0f)
            affect = 0.0f;
        affect = pow(affect, 0.5f);
        
        diffuseEf += diffSpotLight * affect * (material.diffuse * material.diffuseFactor);
        specularEf += specSpotLight * affect * (material.specular * material.specularFactor);
        limEf += spotlim * affect;
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
    
    float3 ligEffect = diffuseEf + specularEf + limEf;
    
    
    
    
    
    
    float4 texCol = baseTex.Sample(smp, input.uv);
    
    float3 st = float3(input.uv, 0.5f) * 1.0f; // �X�P�[������
    int octaves = 4; // �I�N�^�[�u��
    float persistence = 0.8f; // �����x
    float lacunarity = 2.5f; // ���N�i���e�B
    float speed = 0.05f; // ����鑬��
    

    float3 perlinValue = PerlinNoiseWithWind(st, octaves, persistence, lacunarity, 0.1f, speed, noiseTimer, input.worldpos + float3(input.instanceIndex, input.instanceIndex, input.instanceIndex), clamp(1.0f - smokeAlpha[input.instanceIndex], 0.0f, 0.2f)) * 0.3f;

    // �e�N�X�`���̒��S����̋������v�Z
    float2 dist = input.uv - float2(0.5f, 0.5f);
    float radius = length(dist) / 0.5f;

    // �����ɉ����ăm�C�Y�̋��x������������
    float falloff = 1.0f - Easing_Cubic_In(radius, 1.0f, 0.0f, 1.0f);
    perlinValue *= falloff;

    float3 weights = float3(0.6f, 0.2f, 0.2f); // �e�m�C�Y�̏d��
    float fogDensity = dot(perlinValue, weights);
    
    //�F��΂ɂ���B
    texCol.xyz = float3(fogDensity, fogDensity, fogDensity) * float3(0.35f, 0.90f, 0.57f);
    texCol.w = fogDensity * smokeAlpha[input.instanceIndex];
    
    if (texCol.w <= 0.1f)
    {
        discard;
    }
    
    
    
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
    
    int isBright = 0;
    
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
        
    }
    //�A����ɐB������X�|�b�g���C�g
    for (int i = 0; i < ligNum_Plant.spotLigNum; ++i)
    {
        if (!spotLight_Plant[i].m_active)
            continue;
    }
    
    isBright = min(isBright, 1);
    result.xyz *= lerp(0.5f, 1.0f, isBright);
    
    //�����������Ă��Ȃ��Ȃ烂�m�N����
    result.xyz = lerp(lerp(result.xyz, Monochrome(result.xyz), toonCommonParam.m_monochromeRate), result.xyz, isBright);
    
    //�A���t�@�l�K�p
    result.w *= toonIndividualParam.m_alpha;
    
    ParticlePSOutput output;
    output.color = result;
    
    //���邳�v�Z
    output.emissive = output.color;
    // float bright = dot(result.xyz, float3(0.2125f, 0.7154f, 0.0721f));
    // if (1.0f < bright)
    //    output.emissive += result;
    // output.emissive.w = result.w;
    
    //output.depth = input.depthInView;
    
    return output;
}

float4 main(float4 pos : POSITION) : SV_POSITION
{
    return pos;
}