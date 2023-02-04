//通常のスプライト描画

cbuffer cbuff0 : register(b0)
{
    matrix parallelProjMat; //平行投影行列
};

cbuffer cbuff1 : register(b1)
{
    matrix mat; //３Ｄ変換行列
    float4 color; //色(RGBA)
}

Texture2D<float4> tex : register(t0); //０番スロットに設定されたテクスチャ
SamplerState smp : register(s0); //０番スロットに設定されたサンプラー

//頂点シェーダーからピクセルシェーダーへのやり取りに使用する構造体
struct VSOutput
{
    float4 svpos : SV_POSITION; //システム用頂点座標
    float2 uv : TEXCOORD; //uv値
};

//頂点シェーダー
VSOutput VSmain(float4 pos : POSITION, float2 uv : TEXCOORD)
{
    VSOutput output; //ピクセルシェーダに渡す値
    output.svpos = mul(mat, pos);
    output.svpos = mul(parallelProjMat, output.svpos);
    pos.z = 0.0f;
    output.uv = uv;
    return output;
}

//ピクセルシェーダー
float4 PSmain(VSOutput input) : SV_TARGET
{
    return tex.Sample(smp, input.uv) * color;
}

float4 main(float4 pos : POSITION) : SV_POSITION
{
    return pos;
}