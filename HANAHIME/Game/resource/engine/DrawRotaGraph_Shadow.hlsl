#include"LightInfo.hlsli"

cbuffer cbuff0 : register(b0)
{
    matrix parallelProjMat; //平行投影行列
};

cbuffer cbuff1 : register(b1)
{
    float3 eyePos;
}

cbuffer cbuff2 : register(b2)
{
    LightInfo ligNum; //アクティブ中のライトの数の情報
}

struct VSOutput
{
    float4 center : CENTER;
    float2 extRate : EXT_RATE;
    float radian : RADIAN;
    float2 rotaCenterUV : ROTA_CENTER_UV;
    int2 miror : MIROR;
    float depth : DEPTH; //スプライトのZ設定値
    float diffuse : DIFFUSE; //ディヒューズの影響度
    float specular : SPECULAR; //スペキュラーの影響度
    float lim : LIM; //リムライトの影響度
};

VSOutput VSmain(VSOutput input)
{
    return input;
}

struct GSOutput
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
    float4 worldPos : WORLD_POS;
    float depth : DEPTH; //スプライトのZ設定値
    float diffuse : DIFFUSE; //ディヒューズの影響度
    float specular : SPECULAR; //スペキュラーの影響度
    float lim : LIM; //リムライトの影響度
};

StructuredBuffer<DirectionLight> dirLight : register(t0);
StructuredBuffer<PointLight> pointLight : register(t1);
StructuredBuffer<SpotLight> spotLight : register(t2);
StructuredBuffer<HemiSphereLight> hemiSphereLight : register(t3);

Texture2D<float4> tex : register(t4);
Texture2D<float4> normalMap : register(t5);
Texture2D<float4> emissiveMap : register(t6);
SamplerState smp : register(s0);

float2 RotateFloat2(float2 Pos, float Radian)
{
    float2 result;
    result.x = Pos.x * cos(Radian) - Pos.y * sin(Radian);
    result.y = Pos.y * cos(Radian) + Pos.x * sin(Radian);
    return result;
}

[maxvertexcount(4)]
void GSmain(
	point VSOutput input[1],
	inout TriangleStream<GSOutput> output
)
{
    uint2 texSize;
    tex.GetDimensions(texSize.x, texSize.y);
    
    float width_h = texSize.x * input[0].extRate.x / 2.0f;
    float height_h = texSize.y * input[0].extRate.y / 2.0f;
    
    float2 rotateCenter = input[0].center.xy;
    rotateCenter += texSize.xy * input[0].extRate * (input[0].rotaCenterUV - float2(0.5f, 0.5f));
    
    GSOutput element;
    element.depth = input[0].depth;
    element.diffuse = input[0].diffuse;
    element.specular = input[0].specular;
    element.lim = input[0].lim;
        
    //左下
    element.pos = input[0].center;
    element.pos.x -= width_h;
    element.pos.y += height_h;
    element.pos.xy = rotateCenter + RotateFloat2(element.pos.xy - rotateCenter, input[0].radian);
    element.worldPos = element.pos;
    element.pos = mul(parallelProjMat, element.pos);
    element.uv = float2(0.0f + input[0].miror.x, 1.0f - input[0].miror.y);
    output.Append(element);
    
    //左上
    element.pos = input[0].center;
    element.pos.x -= width_h;
    element.pos.y -= height_h;
    element.pos.xy = rotateCenter + RotateFloat2(element.pos.xy - rotateCenter, input[0].radian);
    element.worldPos = element.pos;
    element.pos = mul(parallelProjMat, element.pos);
    element.uv = float2(0.0f + input[0].miror.x, 0.0f + input[0].miror.y);
    output.Append(element);
    
     //右下
    element.pos = input[0].center;
    element.pos.x += width_h;
    element.pos.y += height_h;
    element.pos.xy = rotateCenter + RotateFloat2(element.pos.xy - rotateCenter, input[0].radian);
    element.worldPos = element.pos;
    element.pos = mul(parallelProjMat, element.pos);
    element.uv = float2(1.0f - input[0].miror.x, 1.0f - input[0].miror.y);
    output.Append(element);
    
    //右上
    element.pos = input[0].center;
    element.pos.x += width_h;
    element.pos.y -= height_h;
    element.pos.xy = rotateCenter + RotateFloat2(element.pos.xy - rotateCenter, input[0].radian);
    element.worldPos = element.pos;
    element.pos = mul(parallelProjMat, element.pos);
    element.uv = float2(1.0f - input[0].miror.x, 0.0f + input[0].miror.y);
    output.Append(element);
}

struct PSOutput
{
    float4 color : SV_Target0;
    float4 emissive : SV_Target1;
};

PSOutput PSmain(GSOutput input) : SV_TARGET
{
    //ポジション
    float3 pos = float3(input.worldPos.xy, input.depth);
    
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
    for (int i = 0; i < ligNum.dirLigNum; ++i)
    {
        float3 dir = dirLight[i].direction;
        float3 ligCol = dirLight[i].color.xyz * dirLight[i].color.w;
        ligEffect += CalcLambertDiffuse(dir, ligCol, normal) * input.diffuse;
        ligEffect += CalcPhongSpecular(dir, ligCol, normal, pos, eyePos) * input.specular;
        ligEffect += CalcLimLight(dir, ligCol, normal, vnormal) * input.lim;
    }
    //ポイントライト
    for (int i = 0; i < ligNum.ptLigNum; ++i)
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
        
        ligEffect += diffPoint * input.diffuse;
        ligEffect += specPoint * input.specular;
        ligEffect += CalcLimLight(dir, ligCol, normal, vnormal) * input.lim;
    }
    //スポットライト
    for (int i = 0; i < ligNum.spotLigNum; ++i)
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
        
        ligEffect += diffSpotLight * affect * input.diffuse;
        ligEffect += specSpotLight * affect * input.specular;
        ligEffect += spotlim * affect * input.lim;
    }
    //天球
    for (int i = 0; i < ligNum.hemiSphereNum; ++i)
    {
        float t = dot(normal.xyz, hemiSphereLight[i].groundNormal);
        t = (t + 1.0f) / 2.0f;
        float3 hemiLight = lerp(hemiSphereLight[i].groundColor, hemiSphereLight[i].skyColor, t);
        ligEffect += hemiLight;
    }
    
    float4 result = tex.Sample(smp, input.uv);
    result.xyz *= ligEffect;
    
    PSOutput output;
    output.color = result;
    
    output.emissive = emissiveMap.Sample(smp, input.uv);
    
    //明るさ計算
    float bright = dot(result.xyz, float3(0.2125f, 0.7154f, 0.0721f));
    if (1.0f < bright)
        output.emissive += result;
    output.emissive.w = result.w;
    
    return output;
}

float4 main(float4 pos : POSITION) : SV_POSITION
{
    return pos;
}