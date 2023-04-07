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

//RGBをHSVに変換
float3 ConvertToHSV(float3 color)
{
    // 0 ~ 255 に変換
    float3 rgb = color * 255;
    
    float3 hsv = float3(0, 0, 0);
    
    //RGBのうちの最大値、最小値
    float maxRGB = max(rgb.r, max(rgb.g, rgb.b));
    float minRGB = min(rgb.r, min(rgb.g, rgb.b));
    
    //色相を求める
    //Rが最大
    if (rgb.g < rgb.r && rgb.b < rgb.r)
    {
        hsv.x = 60 * ((rgb.g - rgb.b) / (maxRGB - minRGB));
    }
    //Gが最大
    if (rgb.r < rgb.g && rgb.b < rgb.g)
    {
        hsv.x = 60 * ((rgb.b - rgb.r) / (maxRGB - minRGB)) + 120;
    }
    //Bが最大
    if (rgb.r < rgb.b && rgb.g < rgb.b)
    {
        hsv.x = 60 * ((rgb.r - rgb.g) / (maxRGB - minRGB)) + 240;
    }
    
    //彩度を求める
    hsv.y = (maxRGB - minRGB) / maxRGB * 255;
    
    //明度を求める
    hsv.z = maxRGB;
    return hsv;
}

float3 ConvertToRGB(float3 hsv)
{
    //RGBに戻す
    float maxRGB = hsv.z;
    float minRGB = maxRGB - ((hsv.y / 255) * maxRGB);
    
    float3 rgb = float3(0, 0, 0);
    
    if(0 <= hsv.x && hsv.x <= 60)
    {
        rgb.r = maxRGB;
        rgb.g = (hsv.x / 60) * (maxRGB - minRGB) + minRGB;
        rgb.b = minRGB;
    }
    else if(60 <= hsv.x && hsv.x <= 120)
    {
        rgb.r = ((120 - hsv.x) / 60) * (maxRGB - minRGB) + minRGB;
        rgb.g = maxRGB;
        rgb.b = minRGB;
    }
    else if (120 <= hsv.x && hsv.x <= 180)
    {
        rgb.r = minRGB;
        rgb.g = maxRGB;
        rgb.b = ((hsv.x - 120) / 60) * (maxRGB - minRGB) + minRGB;
    }
    else if(180 <= hsv.x && hsv.x <= 240)
    {
        rgb.r = minRGB;
        rgb.g = ((240 - hsv.x) / 60) * (maxRGB - minRGB) + minRGB;
        rgb.b = maxRGB;
    }
    else if(240 <= hsv.x && hsv.x <= 300)
    {
        rgb.r = ((hsv.x - 240) / 60) * (maxRGB - minRGB) + minRGB;
        rgb.g = minRGB;
        rgb.b = maxRGB;
    }
    else if(300 <= hsv.x && hsv.x <= 360)
    {
        rgb.r = maxRGB;
        rgb.g = minRGB;
        rgb.b = ((360 - hsv.x) / 60) * (maxRGB - minRGB) + minRGB;
    }
    
    //0~1の範囲に変換
    return rgb / 255.0f;
}

//色相ずらし
float3 ChangeHue(float3 color, float param)
{
    //HSVに変換
    float3 hsv = ConvertToHSV(color);
    
    //色相をずらす
    hsv.x += param;
    
    //色相を 0 ~ 360 の範囲に収める
    while (hsv.x < 0)
        hsv.x += 360;
    while (360 < hsv.x)
        hsv.x -= 360;
 
    return ConvertToRGB(hsv);
}

//モノクロ化
float3 Monochrome(float3 color)
{
    float gray = color.r * 0.298912f + color.g * 0.586611f + color.b * 0.114478f;
    return float3(gray, gray, gray);
}