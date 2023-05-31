#include"../../engine/Math.hlsli"
#include"../../engine/ColorProcess.hlsli"
#include"BasicDraw.hlsli"

cbuffer cbuff2 : register(b2)
{
    matrix world;
}

struct InSphereData
{
    float3 upVec;
    int inSphre;
    float rate;
};
cbuffer cbuff9 : register(b9)
{
    InSphereData inSphereData;
}

struct VSOutput
{
    float4 svpos : SV_POSITION;
    float3 worldpos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float depthInView : CAM_Z;
};

struct PSOutput_Player
{
    float4 color : SV_Target0;
    float4 emissive : SV_Target1;
    float depth : SV_Target2;
    float4 edgeColor : SV_Target3;
    float4 bright : SV_Target4;
    float4 normal : SV_Target5;
};

VSOutput VSmain(Vertex input)
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
    float4 wpos = mul(world, resultPos); //ワールド変換
    output.svpos = mul(cam.view, wpos); //ビュー変換
    output.depthInView = output.svpos.z; //カメラから見たZ
    output.svpos = mul(cam.proj, output.svpos); //プロジェクション変換
    output.worldpos = wpos;
    output.normal = normalize(mul(world, input.normal));
    output.uv = input.uv;
    return output;
}

PSOutput_Player PSmain(VSOutput input) : SV_TARGET
{
    float3 normal = input.normal;
    float3 vnormal = normalize(mul(cam.view, normal));
    
    //ライトの影響
    float3 diffuseEf = { 0, 0, 0 };
    
    //ディレクションライト
    for (int i = 0; i < ligNum.dirLigNum; ++i)
    {
        if (!dirLight[i].active)
            continue;
        
        float3 dir = dirLight[i].direction;
        float3 ligCol = dirLight[i].color.xyz * dirLight[i].color.w;
        diffuseEf += CalcLambertDiffuse(dir, ligCol, normal) * (material.diffuse * material.diffuseFactor);
    }
    //ポイントライト
    for (int i = 0; i < ligNum.ptLigNum; ++i)
    {
        if (!pointLight[i].active)
            continue;
        
        float3 dir = input.worldpos - pointLight[i].pos;
        dir = normalize(dir);
        float3 ligCol = pointLight[i].color.xyz * pointLight[i].color.w;
        
        //減衰なし状態
        float3 diffPoint = CalcLambertDiffuse(dir, ligCol, normal);
        
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
        
        diffuseEf += diffPoint * (material.diffuse * material.diffuseFactor);
    }
    //スポットライト
    for (int i = 0; i < ligNum.spotLigNum; ++i)
    {
        if (!spotLight[i].active)
            continue;
        
        float3 ligDir = input.worldpos - spotLight[i].pos;
        ligDir = normalize(ligDir);
        float3 ligCol = spotLight[i].color.xyz * spotLight[i].color.w;
        
        //減衰なし状態
        float3 diffSpotLight = CalcLambertDiffuse(ligDir, ligCol, normal);
        
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
    //天球
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
    int isBrightDefRange = 0; //デフォルトのライトの影響度の範囲
    
    //植物を繁殖させるポイントライト
    for (int i = 0; i < ligNum_Plant.ptLigNum; ++i)
    {
        if (!pointLight_Plant[i].m_active)
            continue;
        
        float3 lightPos = float3(pointLight_Plant[i].m_posX, pointLight_Plant[i].m_posY, pointLight_Plant[i].m_posZ);
        float3 ligRay = input.worldpos - lightPos;
        float bright = dot(-normalize(ligRay), input.normal);
        //-1 ~ 1 から 0 ~ 1の範囲に収める
        bright = saturate(bright);
        isBright += 1.0f - step(pointLight_Plant[i].m_influenceRange, length(ligRay) * int(bright == 0 ? pointLight_Plant[i].m_influenceRange : 1));
        isBrightDefRange += 1.0f - step(pointLight_Plant[i].m_defInfluenceRange, length(ligRay) * int(bright == 0 ? pointLight_Plant[i].m_defInfluenceRange : 1));
        
    }
    //植物を繁殖させるスポットライト
    for (int i = 0; i < ligNum_Plant.spotLigNum; ++i)
    {
        if (!spotLight_Plant[i].m_active)
            continue;
    }
    
    float3 albedo = result;
    
    isBright = min(isBright, 1);
    isBrightDefRange = min(isBrightDefRange, 1);
    result.xyz *= lerp(0.7f, 1.0f, saturate(isBright + isBrightDefRange));
    
    //光が当たっていないならモノクロ化
    result.xyz = lerp(lerp(result.xyz, Monochrome(result.xyz), toonCommonParam.m_monochromeRate), result.xyz, isBright);
    
    //アルファ値適用
    result.w *= toonIndividualParam.m_alpha;

    
    
    
    
    //ライティングを行う。
    //if (inSphereData.inSphre)
    //{
        //float3 playerColor = float3(0.35f, 0.90f, 0.57f);
        float3 playerColor = float3(0.45f, 0.02f, 0.27f);
        float rate = 1.0f + 0.5f * inSphereData.rate;
        result.xyz += CalcLimLight(-normalize(input.worldpos - cam.eyePos), playerColor, input.normal, normalize(input.worldpos - cam.eyePos)) * rate;
        //result.xyz += saturate(dot(normalize(inSphereData.upVec + float3(0, 0, 5.0f)), input.normal) * playerColor) * rate;
        //result.xyz += saturate(dot(normalize(inSphereData.upVec + float3(0, 0, -5.0f)), input.normal) * playerColor) * rate;
        
    //}
    
    
    
    
    
    
    PSOutput_Player output;
    output.color = result;
    
    //明るさ計算
    // float bright = dot(result.xyz, float3(0.2125f, 0.7154f, 0.0721f));
    // if (1.0f < bright)
    //    output.emissive += result;
    // output.emissive.w = result.w;
    output.emissive.xyz = material.emissive * material.emissiveFactor + emissiveTex.Sample(smp, input.uv).xyz;
    output.emissive.w = 1.0f;
    
    output.depth = input.depthInView;

    output.edgeColor = toonIndividualParam.m_edgeColor;
    output.edgeColor.xyz *= lerp(0.9f, 1.0f, isBright);
    
    output.bright.x = 0;
    output.bright.y = toonIndividualParam.m_edgeColor.w ? 0 : 1;
    output.bright.z = toonIndividualParam.m_edgeColor.z;
    output.bright.w = toonIndividualParam.m_edgeColor.w;
    
    output.normal.xyz = input.normal;

    return output;

}