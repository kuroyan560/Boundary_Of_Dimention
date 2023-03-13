#include"Math.hlsli"
#include"ColorProcess.hlsli"

struct VSOutput
{
    float4 svpos : SV_POSITION; //システム用頂点座標
    float2 uv : TEXCOORD; //uv値
};

cbuffer cbuff0 : register(b0)
{
    matrix parallelProjMat; //平行投影行列
}

cbuffer cbuff1 : register(b1)
{
    //画面サイズ
    float2 screenSize;
	//画面歪み強さ
    float screenDistort;
    //ガウシアンフィルタをかけてサンプリングする際の重み
    float gaussianSampleWeight;
    //オーバーレイ色
    float4 overlayColor;
    //オーバーレイの明るさしきい値
    float overlayThreshold;
    //彩度を底上げ
    float saturationRate;
}

Texture2D<float4> tex : register(t0);
SamplerState smp : register(s0);

VSOutput VSmain(float4 pos : POSITION, float2 uv : TEXCOORD)
{
    VSOutput output; //ピクセルシェーダに渡す値
    output.svpos = mul(parallelProjMat, pos);
    pos.z = 0.0f;
    output.uv = uv;
    return output;
}

// 2次元ベクトルをシード値とした乱数
float Rand(float2 co)
{
    return frac(sin(dot(co.xy, float2(12.9898, 78.233))) * 43578.5453);
}

//3x3のガウシアンフィルタをかけてサンプリング
float4 GaussianSample(float2 uv, float2 dx, float2 dy, float weight)
{
    float4 col = float4(0, 0, 0, 0);
    //左上
    col += tex.Sample(smp, uv - dx - dy) * (weight / pow(2, 4)) / weight;
    //左
    col += tex.Sample(smp, uv - dx) * (weight / pow(2, 3)) / weight;
    //左下
    col += tex.Sample(smp, uv - dx + dy) * (weight / pow(2, 4)) / weight;
    //上
    col += tex.Sample(smp, uv - dy) * (weight / pow(2, 3)) / weight;
    //真ん中
    col += tex.Sample(smp, uv) * (weight / pow(2, 2)) / weight;
    //下
    col += tex.Sample(smp, uv + dy) * (weight / pow(2, 3)) / weight;
    //右上
    col += tex.Sample(smp, uv + dx - dy) * (weight / pow(2, 4)) / weight;
    //右
    col += tex.Sample(smp, uv + dx) * (weight / pow(2, 3)) / weight;
    //右下
    col += tex.Sample(smp, uv + dx + dy) * (weight / pow(2, 4)) / weight;
    return col;
}

float CRTEase(const float x,const float base,const float offset)
{
    float tmp = frac(x + offset);
    float xx = 1 - abs(tmp * 2 - 1);
    float ease = Easing_Cubic_InOut(xx, 1.0f, 0.0f, 1.0f);
    return ease * base + base * 0.8f;
}

float4 PSmain(VSOutput input) : SV_TARGET
{
    float2 inUV = input.uv;
    
    const float floorX = frac(inUV.x * screenSize.x / 3.0f);
    const float isR = step(floorX, 0.3f);
    const float isG = step(floorX, 0.6f) * (1 - isR);
    const float isB = step(0.6f, floorX) * (1 - isR) * (1 - isG);
    
    const float2 dx = float2(1.0f / screenSize.x, 0.0f);
    const float2 dy = float2(0.0f, 1.0f / screenSize.y);
    
    float2 texUV = inUV;
    texUV += isR * -1.0f * dy;
    texUV += isG * 0.0f * dy;
    texUV += isB * 1.0f * dy;
    
    float4 col = GaussianSample(texUV, dx, dy, gaussianSampleWeight);
    
    const float floorY = frac(texUV.y * screenSize.y / 6.0f);
    const float easeR = CRTEase(floorY, col.r, Rand(texUV) * 0.1f);
    const float easeG = CRTEase(floorY, col.g, Rand(texUV) * 0.1f);
    const float easeB = CRTEase(floorY, col.b, Rand(texUV) * 0.1f);
    
    float r = isR * easeR;
    float g = isG * easeG;
    float b = isB * easeB;
    
    return float4(r, g, b, col.w);
}