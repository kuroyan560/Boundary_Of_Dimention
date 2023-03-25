Texture2D<float4> baseTex : register(t0);
Texture2D<float4> blendTex : register(t1);
Texture2D<float> maskTex : register(t2);
Texture2D<float> noiseTex : register(t3);

[numthreads(32, 32, 1)]
void CSmain(uint2 DTid : SV_DispatchThreadID)
{
    float4 base = baseTex[DTid];
    float4 blend = blendTex[DTid];
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