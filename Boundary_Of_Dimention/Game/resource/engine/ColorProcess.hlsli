float GetColorBright(float3 arg_rgb)
{
    return dot(arg_rgb, float3(0.2125f, 0.7154f, 0.0721f));
}

//��Z
float3 ColorMultiplication(float3 arg_baseColor,float3 arg_synthesisColor)
{
    return arg_baseColor * arg_synthesisColor;
}

//�X�N���[��
float3 ColorScreen(float3 arg_baseColor, float3 arg_synthesisColor)
{
    return arg_baseColor + arg_synthesisColor - arg_baseColor * arg_synthesisColor;
}

//��{�F�̒��Ԓl�ŏ�������
float3 ColorOverlay(float3 arg_baseColor, float3 arg_synthesisColor, float brightThreshold)
{
    float baseBright = GetColorBright(arg_baseColor);
    //���邢���Â���
    float isBright = step(brightThreshold, baseBright);

    float3 result = { 0, 0, 0 };
    //���Â��i��Z�j
    result += ColorMultiplication(arg_baseColor, arg_synthesisColor) * 2.0f * (1 - isBright);
    //��薾�邭�i�X�N���[���j
    result += (2.0f * ColorScreen(arg_baseColor, arg_synthesisColor) - 1.0f) * isBright;
    return result;
}

//�����F�̒��Ԓl�ŏ�������
float3 ColorHardLight(float3 arg_baseColor, float3 arg_synthesisColor, float brightThreshold)
{
    float synthesisBright = GetColorBright(arg_synthesisColor);
    //���邢���Â���
    float isBright = step(brightThreshold, synthesisBright);

    float3 result = { 0, 0, 0 };
    //���Â��i��Z�j
    result += ColorMultiplication(arg_baseColor, arg_synthesisColor) * 2.0f * (1 - isBright);
    //��薾�邭�i�X�N���[���j
    result += (2.0f * ColorScreen(arg_baseColor, arg_synthesisColor) - 1.0f) * isBright;
    return result;
}