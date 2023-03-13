cbuffer cbuff0 : register(b0)
{
    matrix parallelProjMat; //平行投影行列
};

struct VSOutput
{
    float4 leftUpPos : POSITION_L_U;
    float4 rightBottomPos : POSITION_R_B;
    float4 maskLeftUpPos : MASK_POS_LU;
    float4 maskRightBottomPos : MASK_POS_RB;
    int2 miror : MIROR;
    float maskAlpha : MASK_ALPHA;
};

VSOutput VSmain(VSOutput input)
{
    return input;
}

struct GSOutput
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
    float4 maskLeftUpPos : MASK_POS_LU;
    float4 maskRightBottomPos : MASK_POS_RB;
    float maskAlpha : MASK_ALPHA;
};


[maxvertexcount(4)]
void GSmain(
	point VSOutput input[1],
	inout TriangleStream<GSOutput> output
)
{
    float width = input[0].rightBottomPos.x - input[0].leftUpPos.x;
    
    GSOutput element;
    element.maskLeftUpPos = input[0].maskLeftUpPos;
    element.maskRightBottomPos = input[0].maskRightBottomPos;
    element.maskAlpha = input[0].maskAlpha;
    
    //左下
    element.pos = input[0].rightBottomPos;
    element.pos.x -= width;
    element.pos = mul(parallelProjMat, element.pos);
    element.uv = float2(0.0f + input[0].miror.x, 1.0f - input[0].miror.y);
    output.Append(element);
    
    //左上
    element.pos = input[0].leftUpPos;
    element.pos = mul(parallelProjMat, element.pos);
    element.uv = float2(0.0f + input[0].miror.x, 0.0f + input[0].miror.y);
    output.Append(element);
    
     //右下
    element.pos = input[0].rightBottomPos;
    element.pos = mul(parallelProjMat, element.pos);
    element.uv = float2(1.0f - input[0].miror.x, 1.0f - input[0].miror.y);
    output.Append(element);
    
    //右上
    element.pos = input[0].leftUpPos;
    element.pos.x += width;
    element.pos = mul(parallelProjMat, element.pos);
    element.uv = float2(1.0f - input[0].miror.x, 0.0f + input[0].miror.y);
    output.Append(element);
}

Texture2D<float4> tex : register(t0);
SamplerState smp : register(s0);

float4 PSmain(GSOutput input) : SV_TARGET
{
    //マスクを適用するか（maskは、マスク内のとき１、外のとき０になる）
    float mask = step(input.maskLeftUpPos.x, input.pos.x);
    mask *= step(input.pos.x, input.maskRightBottomPos.x);
    mask *= step(input.maskLeftUpPos.y, input.pos.y);
    mask *= step(input.pos.y, input.maskRightBottomPos.y);

    float4 result = tex.Sample(smp, input.uv);
    if (mask)
    {
        return result;
    }
    
    result.w *= input.maskAlpha;
    
    return result;
}

float4 main(float4 pos : POSITION) : SV_POSITION
{
    return pos;
}