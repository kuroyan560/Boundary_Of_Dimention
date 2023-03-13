cbuffer cb0 : register(b0)
{
    int texHeight;
    int frequency;
}

Texture2D<float4> inputImg : register(t0);
RWTexture2D<float4> outputImg : register(u0);

[numthreads(1, 1, 1)]
void CSmain( uint2 DTid : SV_DispatchThreadID )
{
    float4 col = inputImg[DTid];
    
    //スキャンラインの幅を計算
    float width = texHeight / frequency;
 
    outputImg[DTid] = col;
}