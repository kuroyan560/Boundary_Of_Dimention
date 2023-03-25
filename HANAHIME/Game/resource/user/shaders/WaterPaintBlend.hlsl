Texture2D<float4> baseTex : register(t0);
Texture2D<float4> blendTex : register(t1);
Texture2D<int> maskTex : register(t2);
Texture2D<float> noiseTex : register(t3);
RWTexture2D<float4> outputImage : register(u0);

[numthreads(32, 32, 1)]
void CSmain(uint2 DTid : SV_DispatchThreadID)
{
    float4 base = baseTex[DTid];
    float4 blend = blendTex[DTid];
    int mask = maskTex[DTid];
    float noise = noiseTex[DTid];
    
    outputImage[DTid.xy] = float4(noise, noise, noise, 1.0f);
}