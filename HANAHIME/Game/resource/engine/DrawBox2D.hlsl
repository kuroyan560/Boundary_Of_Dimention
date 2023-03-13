cbuffer cbuff0 : register(b0)
{
    matrix parallelProjMat; //平行投影行列
};

struct VSOutput
{
    float4 leftUpPos : POSITION_L_U;
    float4 rightBottomPos : POSITION_R_B;
    float4 color : COLOR;
};

VSOutput VSmain(VSOutput input)
{
    return input;
}

struct GSOutput
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};



[maxvertexcount(4)]
void GSmain(
	point VSOutput input[1], 
	inout TriangleStream< GSOutput > output
)
{
    float width = input[0].rightBottomPos.x - input[0].leftUpPos.x;
    
    GSOutput element;
    element.color = input[0].color;
        
    //左下
    element.pos = input[0].rightBottomPos;
    element.pos.x -= width;
    element.pos = mul(parallelProjMat, element.pos);
    output.Append(element);
    
    //左上
    element.pos = input[0].leftUpPos;
    element.pos = mul(parallelProjMat, element.pos);
    output.Append(element);
    
     //右下
    element.pos = input[0].rightBottomPos;
    element.pos = mul(parallelProjMat, element.pos);
    output.Append(element);
    
    //右上
    element.pos = input[0].leftUpPos;
    element.pos.x += width;
    element.pos = mul(parallelProjMat, element.pos);
    output.Append(element);
}

float4 PSmain(GSOutput input) : SV_TARGET
{
    return input.color;
}

float4 main(float4 pos : POSITION) : SV_POSITION
{
    return pos;
}