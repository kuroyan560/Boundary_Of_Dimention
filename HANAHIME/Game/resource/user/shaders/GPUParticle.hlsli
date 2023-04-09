struct ParticleData
{
    float3 pos;
    float4 color;
};

//https://www.reedbeta.com/blog/quick-and-easy-gpu-random-numbers-in-d3d11/
uint wang_hash(uint seed)
{
	seed = (seed ^ 61) ^ (seed >> 16);
	seed *= 9;
	seed = seed ^ (seed >> 4);
	seed *= 0x27d4eb2d;
	seed = seed ^ (seed >> 15);
	return seed;
}

float3 RandVec3(float SEED,float MAX,float MIN)
{
    uint rand = wang_hash(SEED * 1847483629);
    float3 result;
    result.x = (rand % 1024) / 1024.0f;
    rand /= 1024;
    result.y = (rand % 1024) / 1024.0f;
    rand /= 1024;
    result.z = (rand % 1024) / 1024.0f;

    result.x = (MAX + abs(MIN)) * result.x - abs(MIN);
    result.y = (MAX + abs(MIN)) * result.y - abs(MIN);
    result.z = (MAX + abs(MIN)) * result.z - abs(MIN);

    if(result.x <= MIN)
    {
        result.x = MIN;        
    }
    if(result.y <= MIN)
    {
        result.y = MIN;        
    }
    if(result.z <= MIN)
    {
        result.z = MIN;        
    }
    return result;
}

float Rand(uint SEED,float MAX,float MIN)
{
    uint rand = wang_hash(SEED * 1847483629);
    float result = (rand % 1024) / 1024.0f;
    float plus = MAX + abs(MIN);
    float mul = plus * result;
    float sub = mul - abs(MIN);
    result = sub;
    if(result <= MIN)
    {
        result = MIN;        
    }
    return result;
}

float AngleToRadian(float ANGLE)
{
	float radian = ANGLE * (3.14f / 180.0f);
	return radian;
}