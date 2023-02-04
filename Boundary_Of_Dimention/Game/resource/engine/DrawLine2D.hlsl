cbuffer cbuff0 : register(b0)
{
    matrix parallelProjMat; //平行投影行列
};

struct VSOutput
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

//頂点シェーダー
VSOutput VSmain(float4 pos : POSITION, float4 color : COLOR)
{
    VSOutput output;
    output.pos = mul(parallelProjMat, pos);
    output.color = color;
    return output;
}

float4 PSmain(VSOutput input) : SV_TARGET
{
    return input.color;
}

float4 main( float4 pos : POSITION ) : SV_POSITION
{
	return pos;
}