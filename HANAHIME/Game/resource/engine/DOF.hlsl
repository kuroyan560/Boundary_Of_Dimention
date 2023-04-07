cbuffer cbuff0 : register(b0)
{
    float nearPint;
    float farPint;
    float pintLength;
};

Texture2D<float4> blurImg : register(t0);
Texture2D<float4> srcImg : register(t1);
Texture2D<float> depthMap : register(t2);
RWTexture2D<float4> outputImage : register(u0);

[numthreads(32, 32, 1)]
void CSmain(uint2 DTid : SV_DispatchThreadID)
{
    float4 src = srcImg[DTid];
    float4 blur = blurImg[DTid];
    float depth = depthMap[DTid];
    
    float rate = 0.0f;
    
     //深度値から不透明度を計算
    if (depth < nearPint)
    {
        float frontLimit = nearPint - pintLength;
        if (frontLimit < 0.0f)
            frontLimit = 0.0f;
        rate = min(1.0f, (nearPint - depth) / (nearPint - frontLimit));
    }
    else if (farPint < depth)
    {
        float backLimit = farPint + pintLength;
        rate = min(1.0f, (depth - farPint) / (backLimit - farPint));
    }
    
    outputImage[DTid.xy] = lerp(blur, src, rate);
}