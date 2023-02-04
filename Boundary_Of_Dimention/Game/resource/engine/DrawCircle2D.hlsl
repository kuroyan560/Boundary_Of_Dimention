cbuffer cbuff0 : register(b0)
{
    matrix parallelProjMat; //平行投影行列
};

struct VSOutput
{
    float4 center : CENTER;
    float radius : RADIUS;
    float4 color : COLOR;
    uint fillFlg : FILL;
    int thickness : THICKNESS;
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

static const int VERT_NUM = 64;
static const float PI = 3.14159265f;
static const float RADIAN_UINT = PI * 2 / (VERT_NUM - 1);

[maxvertexcount(VERT_NUM * 2)]
void GSmain(
	point VSOutput input[1],
	inout TriangleStream<GSOutput> output
)
{
    GSOutput element;
    element.color = input[0].color;
    
    for (int i = 0; i < VERT_NUM - 1;++i)
    {
        float rad = RADIAN_UINT * i;
        
        //内側
        float innerRadius = (input[0].radius - input[0].thickness) * input[0].fillFlg;
        element.pos = input[0].center;
        element.pos.x += cos(rad) * innerRadius;
        element.pos.y += sin(rad) * innerRadius;
        element.pos = mul(parallelProjMat, element.pos);
        output.Append(element);
        
        //外側
        element.pos = input[0].center;
        element.pos.x += cos(rad) * input[0].radius;
        element.pos.y += sin(rad) * input[0].radius;
        element.pos = mul(parallelProjMat, element.pos);
        output.Append(element);
    }
    
    float rad = 0.0f;
    
    //内側
    float innerRadius = (input[0].radius - input[0].thickness) * input[0].fillFlg;
    element.pos = input[0].center;
    element.pos.x += cos(rad) * innerRadius;
    element.pos.y += sin(rad) * innerRadius;
    element.pos = mul(parallelProjMat, element.pos);
    output.Append(element);
        
    //外側
    element.pos = input[0].center;
    element.pos.x += cos(rad) * input[0].radius;
    element.pos.y += sin(rad) * input[0].radius;
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