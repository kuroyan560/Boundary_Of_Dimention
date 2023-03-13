cbuffer cbuff0 : register(b0)
{
    matrix parallelProjMat; //平行投影行列
};

struct VSInput
{
    float4 m_center : CENTER;
    float2 m_extRate : EXT_RATE;
    float m_sRadian : START_RADIAN;
    float m_eRadian : END_RADIAN;
    float2 m_anchor : ANCHOR;
};

struct VSOutput
{
    float4 m_center : CENTER;
    float2 m_extRate : EXT_RATE;
    //ワイプの中心
    float2 m_anchor : ANCHOR;
    //ワイプ扇の方向ベクトル
    float2 m_fanVec : FAN_VEC;
    //ワイプ扇の角度
    float m_fanRangeRadian : FAN_RANGE_RADIAN;
};

VSOutput VSmain(VSInput input)
{
    VSOutput output;
    output.m_center = input.m_center;
    output.m_extRate = input.m_extRate;
    output.m_anchor = input.m_anchor;
    
    //ワイプ扇が向いてる角度を求める
    output.m_fanRangeRadian = input.m_eRadian - input.m_sRadian;
    float fanRadian = input.m_sRadian + output.m_fanRangeRadian / 2.0f;
    output.m_fanVec.x = cos(fanRadian);
    output.m_fanVec.y = sin(fanRadian);
    return output;
}

struct GSOutput
{
    float4 m_pos : SV_POSITION;
    float2 m_uv : TEXCOORD;
    //ワイプの中心
    float2 m_anchor : ANCHOR;
    //ワイプ扇の方向ベクトル
    float2 m_fanVec : FAN_VEC;
    //ワイプ扇の角度
    float m_fanRangeRadian : FAN_RANGE_RADIAN;
};

Texture2D<float4> tex : register(t0);
SamplerState smp : register(s0);

float2 RotateFloat2(float2 Pos,float Radian)
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
    
    float width_h = texSize.x * input[0].m_extRate.x / 2.0f;
    float height_h = texSize.y * input[0].m_extRate.y / 2.0f;
    
    GSOutput element;
    element.m_anchor = input[0].m_anchor;
    element.m_fanVec = input[0].m_fanVec;
    element.m_fanRangeRadian = input[0].m_fanRangeRadian;
    
    //左下
    element.m_pos = input[0].m_center;
    element.m_pos.x -= width_h;
    element.m_pos.y += height_h;
    element.m_pos = mul(parallelProjMat, element.m_pos);
    element.m_uv = float2(0.0f, 1.0f);
    output.Append(element);
    
    //左上
    element.m_pos = input[0].m_center;
    element.m_pos.x -= width_h;
    element.m_pos.y -= height_h;
    element.m_pos = mul(parallelProjMat, element.m_pos);
    element.m_uv = float2(0.0f, 0.0f);
    output.Append(element);
    
     //右下
    element.m_pos = input[0].m_center;
    element.m_pos.x += width_h;
    element.m_pos.y += height_h;
    element.m_pos = mul(parallelProjMat, element.m_pos);
    element.m_uv = float2(1.0f, 1.0f);
    output.Append(element);
    
    //右上
    element.m_pos = input[0].m_center;
    element.m_pos.x += width_h;
    element.m_pos.y -= height_h;
    element.m_pos = mul(parallelProjMat, element.m_pos);
    element.m_uv = float2(1.0f, 0.0f);
    output.Append(element);
}

float4 PSmain(GSOutput input) : SV_TARGET
{
    float2 vecFanToPoint = normalize(input.m_uv - input.m_anchor);
    
    float uvFanDot = dot(vecFanToPoint, input.m_fanVec);
    float fanCos = cos(input.m_fanRangeRadian / 2.0f);
    
    clip(uvFanDot < fanCos ? -1 : 1);
    
    float4 result = tex.Sample(smp, input.m_uv);
    return result;
}

float4 main(float4 pos : POSITION) : SV_POSITION
{
    return pos;
}