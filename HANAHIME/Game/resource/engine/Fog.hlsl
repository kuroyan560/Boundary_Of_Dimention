static const int THREAD_PER_NUM = 32;

//フォグの設定
struct Config
{
	//強さ
    float m_intensity;
	//不透明度最大
    float m_opacityMax;
	//フォグがかかる最大深度
    float m_distMax;
	//フォグがかかる最小深度
    float m_distMin;
};

//色に関して
struct ColorParameter
{
	//テクスチャを使わない場合のフォグの色
    float4 m_fogColorNear;
    float4 m_fogColorFar;
	//テクスチャを使うか
    unsigned int m_useTex;
    int m_isGameScene;
    float2 m_pad;
};

cbuffer cbuff0 : register(b0)
{
    Config config;
}

cbuffer cbuff1 : register(b1)
{
    ColorParameter colorConfig;
}

bool IsColorEqual(float3 color1, float3 color2, float epsilon)
{
    return all(abs(color1 - color2) < epsilon);
}

Texture2D<float4> gradation : register(t0);
Texture2D<float4> mainTex : register(t1);
Texture2D<float> edgeDepthMap : register(t2);
Texture2D<float> depthMap : register(t3);
Texture2D<float4> maskTex : register(t4);

RWTexture2D<float4> resultTex : register(u0);

[numthreads(THREAD_PER_NUM, THREAD_PER_NUM, 1)]
void CSmain(uint2 DTid : SV_DispatchThreadID)
{
    float depth = edgeDepthMap[DTid].x;
    
    float4 result = mainTex[DTid];
    
    depth = clamp(depth * (config.m_distMin / config.m_distMax), 0.0f, 1.0f);
    float4 fogColor = lerp(lerp(colorConfig.m_fogColorNear, colorConfig.m_fogColorFar, depth), gradation[DTid], colorConfig.m_useTex);
    
    result.xyz = lerp(result.xyz, fogColor.xyz, min(depth * config.m_intensity, config.m_opacityMax));
    //result.xyz = lerp(result.xyz, mainTex[DTid].xyz, maskTex[DTid].x);

    resultTex[DTid] = result;
};