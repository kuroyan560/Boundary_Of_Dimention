#include"../../engine/Math.hlsli"
#include"../../engine/ColorProcess.hlsli"
#include"BasicDraw.hlsli"

static const int MAX_NUM = 1024;
cbuffer cbuff2 : register(b2)
{
    matrix world[MAX_NUM];
}

struct VSOutput
{
    float4 svpos : SV_POSITION;
    float3 worldpos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float depthInView : CAM_Z;
};

VSOutput VSmain(Vertex input, uint instanceID : SV_InstanceID)
{
    float4 resultPos = input.pos;
	
	//ボーン行列
	//BDEF4(ボーン4つ又は3つと、それぞれのウェイト値。ウェイト合計が1.0である保障はしない)
    if (input.boneNo[2] != NO_BONE)
    {
        int num;
        
        if (input.boneNo[3] != NO_BONE)    //ボーン４つ
        {
            num = 4;
        }
        else //ボーン３つ
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
	//BDEF2(ボーン2つと、ボーン1のウェイト値(PMD方式))
    else if (input.boneNo[1] != NO_BONE)
    {
        matrix mat = bones[input.boneNo[0]] * input.weight[0];
        mat += bones[input.boneNo[1]] * (1 - input.weight[0]);
        
        resultPos = mul(mat, input.pos);
    }
	//BDEF1(ボーンのみ)
    else if (input.boneNo[0] != NO_BONE)
    {
        resultPos = mul(bones[input.boneNo[0]], input.pos);
    }
	
    VSOutput output;
    float4 wpos = mul(world[instanceID], resultPos); //ワールド変換
    output.svpos = mul(cam.view, wpos); //ビュー変換
    output.depthInView = output.svpos.z;    //カメラから見たZ
    output.svpos = mul(cam.proj, output.svpos); //プロジェクション変換
    output.worldpos = wpos;
    output.normal = normalize(mul(world[instanceID], float4(input.normal, 1))).xyz;
    output.uv = input.uv;
    return output;
}

PSOutput PSmain(VSOutput input) : SV_TARGET
{
    float3 normal = input.normal;
    float3 vnormal = normalize(mul(cam.view, normal));
    
    //ライトの影響
    float3 diffuseEf = { 0, 0, 0 };
    float3 specularEf = { 0, 0, 0 };
    float3 limEf = { 0, 0, 0 };
    
    //ディレクションライト
    for (int i = 0; i < ligNum.dirLigNum; ++i)
    {
        if (!dirLight[i].active)continue;
        
        float3 dir = dirLight[i].direction;
        float3 ligCol = dirLight[i].color.xyz * dirLight[i].color.w;
        diffuseEf += CalcLambertDiffuse(dir, ligCol, normal) * (material.diffuse * material.diffuseFactor);
        specularEf += CalcPhongSpecular(dir, ligCol, normal, input.worldpos, cam.eyePos) * (material.specular * material.specularFactor);
        limEf += CalcLimLight(dir, ligCol, normal, vnormal);
    }
    //ポイントライト
    for (int i = 0; i < ligNum.ptLigNum; ++i)
    {
        if (!pointLight[i].active)continue;
        
        float3 dir = input.worldpos - pointLight[i].pos;
        dir = normalize(dir);
        float3 ligCol = pointLight[i].color.xyz * pointLight[i].color.w;
        
        //減衰なし状態
        float3 diffPoint = CalcLambertDiffuse(dir, ligCol, normal);
        float3 specPoint = CalcPhongSpecular(dir, ligCol, normal, input.worldpos, cam.eyePos);
        
        //距離による減衰
        float3 distance = length(input.worldpos - pointLight[i].pos);
		//影響率は距離に比例して小さくなっていく
        float affect = 1.0f - 1.0f / pointLight[i].influenceRange * distance;
		//影響力がマイナスにならないように補正をかける
        if (affect < 0.0f)
            affect = 0.0f;
		//影響を指数関数的にする
        affect = pow(affect, 3.0f);
        diffPoint *= affect;
        specPoint *= affect;
        
        diffuseEf += diffPoint * (material.diffuse * material.diffuseFactor);
        specularEf += specPoint * (material.specular * material.specularFactor);
        limEf += CalcLimLight(dir, ligCol, normal, vnormal);
    }
    //スポットライト
    for (int i = 0; i < ligNum.spotLigNum; ++i)
    {
        if (!spotLight[i].active)continue;
        
        float3 ligDir = input.worldpos - spotLight[i].pos;
        ligDir = normalize(ligDir);
        float3 ligCol = spotLight[i].color.xyz * spotLight[i].color.w;
        
        //減衰なし状態
        float3 diffSpotLight = CalcLambertDiffuse(ligDir, ligCol, normal);
        float3 specSpotLight = CalcPhongSpecular(ligDir, ligCol, normal, input.worldpos, cam.eyePos);
        
        //スポットライトとの距離を計算
        float3 distance = length(input.worldpos - spotLight[i].pos);
       	//影響率は距離に比例して小さくなっていく
        float affect = 1.0f - 1.0f / spotLight[i].influenceRange * distance;
        //影響力がマイナスにならないように補正をかける
        if (affect < 0.0f)
            affect = 0.0f;
    //影響を指数関数的にする
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
    //天球
    for (int i = 0; i < ligNum.hemiSphereNum; ++i)
    {
        if (!hemiSphereLight[i].active)continue;
        
        float t = dot(normal.xyz, hemiSphereLight[i].groundNormal);
        t = (t + 1.0f) / 2.0f;
        float3 hemiLight = lerp(hemiSphereLight[i].groundColor, hemiSphereLight[i].skyColor, t);
        diffuseEf += hemiLight;
    }
    
    float3 ligEffect = diffuseEf + specularEf + limEf;
    
    float4 texCol = baseTex.Sample(smp, input.uv);
    texCol.xyz += material.baseColor.xyz;
    float4 ligEffCol = texCol;
    ligEffCol.xyz = ((material.ambient * material.ambientFactor) + ligEffect) * ligEffCol.xyz;
    ligEffCol.w *= (1.0f - material.transparent);
    
    //アニメ風トゥーン加工========================================================
    
    //トゥーンによる色
    float4 toonCol = ligEffCol;
    
    //明るさ算出（照明影響より）
    float lightEffectBright = GetColorBright(ligEffect.xyz);

    //明るさのしきい値に応じて色を決める
    float thresholdResult = smoothstep(toonCommonParam.m_brightThresholdLow, toonCommonParam.m_brightThresholdLow + toonCommonParam.m_brightThresholdRange, lightEffectBright);
    float4 brightCol = texCol * toonIndividualParam.m_brightMulColor * thresholdResult;
    float4 darkCol = texCol * toonIndividualParam.m_darkMulColor * (1.0f - thresholdResult);
    toonCol.xyz = brightCol + darkCol;
    float4 result = toonCol;

    //=========================================================================

    //塗りつぶし
    result.xyz = toonIndividualParam.m_fillColor.xyz * toonIndividualParam.m_fillColor.w + result.xyz * (1.0f - toonIndividualParam.m_fillColor.w);
    
    int isBright = 0;
    
    //植物を繁殖させるポイントライト
    for (int i = 0; i < ligNum_Plant.ptLigNum; ++i)
    {
        if (!pointLight_Plant[i].m_active)
            continue;
        
        float3 ligRay = input.worldpos - pointLight_Plant[i].m_pos;
        float bright = dot(-normalize(ligRay), input.normal);
        //-1 ~ 1 から 0 ~ 1の範囲に収める
        bright = saturate(bright);
        isBright += 1.0f - step(pointLight_Plant[i].m_influenceRange, length(ligRay) * int(bright == 0 ? pointLight_Plant[i].m_influenceRange : 1));
        
    }
    //植物を繁殖させるスポットライト
    for (int i = 0; i < ligNum_Plant.spotLigNum; ++i)
    {
        if (!spotLight_Plant[i].m_active)
            continue;
    }
    
    isBright = min(isBright, 1);
    result.xyz *= lerp(0.5f, 1.0f, isBright);
    
    //光が当たっていないならモノクロ化
    result.xyz = lerp(lerp(result.xyz, Monochrome(result.xyz), toonCommonParam.m_monochromeRate), result.xyz, isBright);
    
    //アルファ値適用
    result.w *= toonIndividualParam.m_alpha;
    
    PSOutput output;
    output.color = result;
    
    //明るさ計算
    // float bright = dot(result.xyz, float3(0.2125f, 0.7154f, 0.0721f));
    // if (1.0f < bright)
    //    output.emissive += result;
    // output.emissive.w = result.w;
    output.emissive = float4(0,0,0,0);
    
    //output.depth = input.depthInView;

    output.normal.xyz = input.normal;
        
    output.edgeColor = toonIndividualParam.m_edgeColor;
    
    output.bright.x = isBright;
    
    return output;
}

float4 main( float4 pos : POSITION ) : SV_POSITION
{
	return pos;
}