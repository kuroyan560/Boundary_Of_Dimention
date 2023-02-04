//ノーマルマップで陰影スプライト描画
#include"LightInfo.hlsli"

cbuffer cbuff0 : register(b0)
{
    matrix parallelProjMat; //平行投影行列
};

cbuffer cbuff1 : register(b1)
{
    matrix mat; //３Ｄ変換行列
    float4 color; //色(RGBA)
    float depth;  //スプライトのZ設定値
    float diffuse;  //ディヒューズの影響度
    float specular; //スペキュラーの影響度
    float lim;  //リムライトの影響度
}

cbuffer cbuff2 : register(b2)
{
    float3 eyePos;
}

cbuffer cbuff3 : register(b3)
{
    LightInfo ligNum;   //アクティブ中のライトの数の情報
}

StructuredBuffer<DirectionLight> dirLight : register(t0);
StructuredBuffer<PointLight> pointLight : register(t1);
StructuredBuffer<SpotLight> spotLight : register(t2);
StructuredBuffer<HemiSphereLight> hemiSphereLight : register(t3);

Texture2D<float4> tex : register(t4); //テクスチャ
Texture2D<float4> normalMap : register(t5); //ノーマルマップ
Texture2D<float4> emissiveMap : register(t6); //エミッシブマップ

SamplerState smp : register(s0);

//頂点シェーダーからピクセルシェーダーへのやり取りに使用する構造体
struct VSOutput
{
    float4 svpos : SV_POSITION; //システム用頂点座標
    float2 uv : TEXCOORD; //uv値
    float4 worldPos : WORLD_POS;
};

//頂点シェーダー
VSOutput VSmain(float4 pos : POSITION, float2 uv : TEXCOORD)
{
    VSOutput output; //ピクセルシェーダに渡す値
    output.svpos = mul(mat, pos);
    output.worldPos = output.svpos;
    output.svpos = mul(parallelProjMat, output.svpos);
    output.uv = uv;
    return output;
}

struct PSOutput
{
    float4 color : SV_Target0;
    float4 emissive : SV_Target1;
};

//ピクセルシェーダー
PSOutput PSmain(VSOutput input)
{
    //ポジション
    float3 pos = float3(input.worldPos.xy, depth);
    
    //ノーマルマップから法線を取得
    float3 normal = normalMap.Sample(smp, input.uv).xyz;
    normal = (normal - 0.5f) * 2.0f;
    normal.y *= -1.0f;
    normal.z *= -1.0f;
    
    float3 vnormal = mul(parallelProjMat, float4(normal, 1.0f)).xyz;
    
    //ライトの影響
    //float3 ligEffect = { 0.3f, 0.3f, 0.3f };
    float3 ligEffect = { 0.0f, 0.0f, 0.0f };
    
    //ディレクションライト
    for (int i = 0; i < ligNum.dirLigNum;++i)
    {
        float3 dir = dirLight[i].direction;
        float3 ligCol = dirLight[i].color.xyz * dirLight[i].color.w;
        ligEffect += CalcLambertDiffuse(dir, ligCol, normal) * diffuse;
        ligEffect += CalcPhongSpecular(dir, ligCol, normal, pos, eyePos) * specular;
        ligEffect += CalcLimLight(dir, ligCol, normal, vnormal) * lim;
    }
    //ポイントライト
    for (int i = 0; i < ligNum.ptLigNum;++i)
    {
        float3 dir = pos - pointLight[i].pos;
        dir = normalize(dir);
        float3 ligCol = pointLight[i].color.xyz * pointLight[i].color.w;
        
        //減衰なし状態
        float3 diffPoint = CalcLambertDiffuse(dir, ligCol, normal);
        float3 specPoint = CalcPhongSpecular(dir, ligCol, normal, pos, eyePos);
        
        //距離による減衰
        float3 distance = length(pos - pointLight[i].pos);
		//影響率は距離に比例して小さくなっていく
        float affect = 1.0f - 1.0f / pointLight[i].influenceRange * distance;
		//影響力がマイナスにならないように補正をかける
        if (affect < 0.0f)
            affect = 0.0f;
		//影響を指数関数的にする
        affect = pow(affect, 3.0f);
        diffPoint *= affect;
        specPoint *= affect;
        
        ligEffect += diffPoint * diffuse;
        ligEffect += specPoint * specular;
        ligEffect += CalcLimLight(dir, ligCol, normal, vnormal) * lim;
    }
    //スポットライト
    for (int i = 0; i < ligNum.spotLigNum;++i)
    {
        float3 ligDir = pos - spotLight[i].pos;
        ligDir = normalize(ligDir);
        float3 ligCol = spotLight[i].color.xyz * spotLight[i].color.w;
        
        //減衰なし状態
        float3 diffSpotLight = CalcLambertDiffuse(ligDir, ligCol, normal);
        float3 specSpotLight = CalcPhongSpecular(ligDir, ligCol, normal, pos, eyePos);
        
        //スポットライトとの距離を計算
        float3 distance = length(pos - spotLight[i].pos);
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
        
        ligEffect += diffSpotLight * affect * diffuse;
        ligEffect += specSpotLight * affect * specular;
        ligEffect += spotlim * affect * lim;
    }
    //天球
    for (int i = 0; i < ligNum.hemiSphereNum;++i)
    {
        float t = dot(normal.xyz, hemiSphereLight[i].groundNormal);
        t = (t + 1.0f) / 2.0f;
        float3 hemiLight = lerp(hemiSphereLight[i].groundColor, hemiSphereLight[i].skyColor, t);
        ligEffect += hemiLight;
    }
    
    float4 result = tex.Sample(smp, input.uv);
    result.xyz *= ligEffect;
    
    PSOutput output;
    output.color = result * color;
    
    output.emissive = emissiveMap.Sample(smp, input.uv);
    
    //明るさ計算
    float bright = dot(result.xyz, float3(0.2125f, 0.7154f, 0.0721f));
    if (1.0f < bright)
        output.emissive += result;
    output.emissive.w = result.w;
    output.emissive *= color;
    
    return output;
}

float4 main(float4 pos : POSITION) : SV_POSITION
{
    return pos;
}