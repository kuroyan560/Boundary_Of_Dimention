struct Camera
{
    matrix view;
    matrix proj;
    matrix billBoard;
    matrix billBoardY;
    float3 eyePos;
    float nearClip;
    float farClip;
};