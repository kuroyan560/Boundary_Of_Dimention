#include"../../engine/ColorProcess.hlsli"
static const int THREAD_PER_NUM = 32;

Texture2D<float4> baseTex : register(t0);
Texture2D<int> maskTex : register(t1);
Texture2D<float> noiseTex : register(t2);
RWTexture2D<float4> outputImage : register(u0);

bool InMaskRange(in uint2 idx, in int expand)
{
    for (int y = -expand; y <= expand; ++y)
    {
        for (int x = -expand; x <= expand; ++x)
        {
            uint2 offsetIdx = idx;
            offsetIdx.x += x;
            offsetIdx.y += y;
            int mask = maskTex[offsetIdx];
            if (mask)
            {
                return true;
            }
        }
    }
    return false;
}

[numthreads(THREAD_PER_NUM, THREAD_PER_NUM, 1)]
void CSmain(uint2 DTid : SV_DispatchThreadID)
{
    float4 base = baseTex[DTid];
    int mask = maskTex[DTid];
    float noise = noiseTex[DTid];
    
    if (InMaskRange(DTid,8))
    {
        outputImage[DTid] = base;
    }
    else
    {
        outputImage[DTid] = float4(ChangeHue(base.rgb, -141), base.w);
    }
};