#include"Camera.hlsli"

cbuffer cbuff0 : register(b0)
{
    Camera cam;
}

struct VSInput
{
    float4 from : FROM_POS;
    float4 to : TO_POS;
    float4 color : COLOR;
    float thickness : THICKNESS;
    //min16uint shadingFlg : SHADING_FLG;
    //float4 emissiveColor : EMISSIVE_COLOR;
};

VSInput VSmain(VSInput input)
{
    return input;
}

struct GSOutput
{
    float4 pos : SV_POSITION;
    float4 col : COLOR;
    //float4 emissiveCol : EMISSIVE_COLOR;
    //float depthInView : CAM_Z;
};

//VecA‚ğVecB‚ÉŒX‚¯‚é‰ñ“]s—ñ
matrix Rotate(float3 VecA, float3 VecB)
{
    float3 a = VecA;
    float3 b = VecB;
    a = normalize(a);
    b = normalize(b);
    float4 q = float4(0, 0, 0, 0);
    float3 c = cross(a, b);
    float d = length(c);
    c = normalize(c);
    
    float epsilon = 0.0002f;
    float ip = dot(a, b);
    
    if (-epsilon < d || 1.0f < ip)
    {
        if (ip < (epsilon - 1.0f))
        {
            float3 a2 = float3(-a.y, a.z, a.x);
            c = normalize(cross(a2, a));
            q.x = c.x;
            q.y = c.y;
            q.z = c.z;
            q.w = 0.0f;
        }
        else
        {
            q = float4(0.0f, 0.0f, 0.0f, 1.0f);
        }
    }
    else
    {
        float3 e = c * sqrt(0.5f * (1.0f - ip));
        q.x = e.x;
        q.y = e.y;
        q.z = e.z;
        q.w = sqrt(0.5f * (1.0f + ip));
    }
    
    matrix result;
    result[0][0] = 1 - 2 * q.y * q.y - 2 * q.z * q.z;
    result[0][1] = 2 * q.x * q.y + 2 * q.w * q.z;
    result[0][2] = 2 * q.x * q.z - 2 * q.w * q.y;
    result[0][3] = 0.0f;
    
    result[1][0] = 2 * q.x * q.y - 2 * q.w * q.z;
    result[1][1] = 1 - 2 * q.x * q.x - 2 * q.z * q.z;
    result[1][2] = 2 * q.y * q.z + 2 * q.w * q.x;
    result[1][3] = 0.0f;
    
    result[2][0] = 2 * q.x * q.z + 2 * q.w * q.y;
    result[2][1] = 2 * q.y * q.z - 2 * q.w * q.x;
    result[2][2] = 1 - 2 * q.x * q.x - 2 * q.y * q.y;
    result[2][3] = 0.0f;
    
    result[3][0] = 0.0f;
    result[3][1] = 0.0f;
    result[3][2] = 0.0f;
    result[3][3] = 1.0f;
    
    return result;
};

GSOutput Get(float4 Pos, float3 Offset, matrix Rotate, float4 Color)
{
    GSOutput element;
    
    float4 offset = float4(Offset, 0.0f);
    float4 p = Pos + mul(Rotate, offset);
    //float4 p = Pos + offset;
    //element.depthInView = mul(cam.view, p).z;
    element.pos = mul(cam.proj, mul(cam.view, p));
    
    element.col = Color;
    //element.emissiveCol = EmissiveColor;
    return element;
}

[maxvertexcount(6 * 6)] // 6–Ê
void GSmain(
    point VSInput input[1],
inout TriangleStream<GSOutput> output)
{
    float offset = input[0].thickness / 2.0f;
    float4 col = input[0].color;
    //float4 emissiveCol = input[0].emissiveColor;
    
    float4 from = input[0].from;
    float4 to = input[0].to;
    float4 p = from;
    
    matrix rotate = Rotate(float3(0, 1, 0), to.xyz - from.xyz);
    
    //è‘O
    {
        output.Append(Get(from, float3(offset, 0.0f, -offset), rotate, col));
    
        output.Append(Get(from, float3(-offset, 0.0f, -offset), rotate, col));
    
        output.Append(Get(to, float3(-offset, 0.0f, -offset), rotate, col));

        output.RestartStrip();
    
        output.Append(Get(to, float3(offset, 0.0f, -offset), rotate, col));
    
        output.Append(Get(to, float3(-offset, 0.0f, -offset), rotate, col));
    
        output.Append(Get(from, float3(offset, 0.0f, -offset), rotate, col));
        
        output.RestartStrip();
    }
    //‰œ
    {
        output.Append(Get(from, float3(offset, 0.0f, offset), rotate, col));
    
        output.Append(Get(from, float3(-offset, 0.0f, offset), rotate, col));
    
        output.Append(Get(to, float3(-offset, 0.0f, offset), rotate, col));

        output.RestartStrip();
    
        output.Append(Get(to, float3(offset, 0.0f, offset), rotate, col));
    
        output.Append(Get(to, float3(-offset, 0.0f, offset), rotate, col));
    
        output.Append(Get(from, float3(offset, 0.0f, offset), rotate, col));
        
        output.RestartStrip();
    }
    //¶
    {
        output.Append(Get(from, float3(-offset, 0.0f, offset), rotate, col));
    
        output.Append(Get(from, float3(-offset, 0.0f, -offset), rotate, col));
    
        output.Append(Get(to, float3(-offset, 0.0f, -offset), rotate, col));

        output.RestartStrip();
    
        output.Append(Get(to, float3(-offset, 0.0f, offset), rotate, col));
    
        output.Append(Get(to, float3(-offset, 0.0f, -offset), rotate, col));
    
        output.Append(Get(from, float3(-offset, 0.0f, offset), rotate, col));
        
        output.RestartStrip();
    }
    //‰E
    {
        output.Append(Get(from, float3(offset, 0.0f, offset), rotate, col));
    
        output.Append(Get(from, float3(offset, 0.0f, -offset), rotate, col));
    
        output.Append(Get(to, float3(offset, 0.0f, -offset), rotate, col));

        output.RestartStrip();
    
        output.Append(Get(to, float3(offset, 0.0f, offset), rotate, col));
    
        output.Append(Get(to, float3(offset, 0.0f, -offset), rotate, col));
    
        output.Append(Get(from, float3(offset, 0.0f, offset), rotate, col));
        
        output.RestartStrip();
    }
    //ã–Ê
    {
        output.Append(Get(from, float3(offset, 0.0f, offset), rotate, col));
    
        output.Append(Get(from, float3(offset, 0.0f, -offset), rotate, col));
    
        output.Append(Get(from, float3(-offset, 0.0f, offset), rotate, col));

        output.RestartStrip();
    
        output.Append(Get(from, float3(offset, 0.0f, -offset), rotate, col));
    
        output.Append(Get(from, float3(-offset, 0.0f, -offset), rotate, col));
    
        output.Append(Get(from, float3(-offset, 0.0f, offset), rotate, col));
        
        output.RestartStrip();
    }
    //‰º–Ê
    {
        output.Append(Get(to, float3(offset, 0.0f, offset), rotate, col));
    
        output.Append(Get(to, float3(offset, 0.0f, -offset), rotate, col));
    
        output.Append(Get(to, float3(-offset, 0.0f, offset), rotate, col));

        output.RestartStrip();
    
        output.Append(Get(to, float3(offset, 0.0f, -offset), rotate, col));
    
        output.Append(Get(to, float3(-offset, 0.0f, -offset), rotate, col));
    
        output.Append(Get(to, float3(-offset, 0.0f, offset), rotate, col));
        
        output.RestartStrip();
    }
}

struct PSOutput
{
    float4 color : SV_Target0; //ƒŒƒ“ƒ_[ƒ^[ƒQƒbƒg‚O‚É•`‚«‚İ
    //float depth : SV_Target1; //ƒŒƒ“ƒ_[ƒ^[ƒQƒbƒg‚P‚É•`‚«‚İ
    //float4 emissive : SV_Target2; //ƒŒƒ“ƒ_[ƒ^[ƒQƒbƒg‚Q‚É•`‚«‚İ
};

PSOutput PSmain(GSOutput input)
{
    PSOutput output;
    output.color = input.col;
    //output.depth = input.depthInView;
    
    //output.emissive = input.emissiveCol;
    
    //float3 resultEmissive = float3(0, 0, 0);
    //float t = dot(output.color.xyz, float3(0.2125f, 0.7154f, 0.0721f)); //–¾‚é‚³ŒvZ
    //if (1.0f < t)
    //    resultEmissive = output.color.xyz;
    //output.emissive.xyz += resultEmissive;
    
    //output.emissive.xyz = clamp(output.emissive.xyz, float3(0, 0, 0), float3(1, 1, 1));
    
    return output;
}

float4 main( float4 pos : POSITION ) : SV_POSITION
{
	return pos;
}