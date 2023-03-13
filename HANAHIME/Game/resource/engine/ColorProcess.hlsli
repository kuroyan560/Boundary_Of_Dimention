float GetColorBright(float3 arg_rgb)
{
    return dot(arg_rgb, float3(0.2125f, 0.7154f, 0.0721f));
}

//乗算
float3 ColorMultiplication(float3 arg_baseColor,float3 arg_synthesisColor)
{
    return arg_baseColor * arg_synthesisColor;
}

//スクリーン
float3 ColorScreen(float3 arg_baseColor, float3 arg_synthesisColor)
{
    return arg_baseColor + arg_synthesisColor - arg_baseColor * arg_synthesisColor;
}

//基本色の中間値で条件分岐
float3 ColorOverlay(float3 arg_baseColor, float3 arg_synthesisColor, float brightThreshold)
{
    float baseBright = GetColorBright(arg_baseColor);
    //明るいか暗いか
    float isBright = step(brightThreshold, baseBright);

    float3 result = { 0, 0, 0 };
    //より暗く（乗算）
    result += ColorMultiplication(arg_baseColor, arg_synthesisColor) * 2.0f * (1 - isBright);
    //より明るく（スクリーン）
    result += (2.0f * ColorScreen(arg_baseColor, arg_synthesisColor) - 1.0f) * isBright;
    return result;
}

//合成色の中間値で条件分岐
float3 ColorHardLight(float3 arg_baseColor, float3 arg_synthesisColor, float brightThreshold)
{
    float synthesisBright = GetColorBright(arg_synthesisColor);
    //明るいか暗いか
    float isBright = step(brightThreshold, synthesisBright);

    float3 result = { 0, 0, 0 };
    //より暗く（乗算）
    result += ColorMultiplication(arg_baseColor, arg_synthesisColor) * 2.0f * (1 - isBright);
    //より明るく（スクリーン）
    result += (2.0f * ColorScreen(arg_baseColor, arg_synthesisColor) - 1.0f) * isBright;
    return result;
}