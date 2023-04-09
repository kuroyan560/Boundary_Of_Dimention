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


struct pcg32_random_t
{
    uint state;
    uint inc;
} ;
uint pcg32_random_r(pcg32_random_t rng)
{
    uint oldstate = rng.state;
    // Advance internal state
    rng.state = oldstate * 6364136223846793005 + (rng.inc | 1);
    // Calculate output function (XSH RR), uses old state for max ILP
    uint xorshifted = ((oldstate >> 18) ^ oldstate) >> 27;
    uint rot = oldstate >> 59;
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

uint XorShit()
{
    uint x = 123456789;
    uint y = 362436069;
    uint z = 521288629;
    uint w = 88675123; 
    uint t = 0;

    t = x ^ (x << 11);
    x = y; y = z; z = w;
    uint result = (w ^ (w >> 19)) ^ (t ^ (t >> 8));
    return result;
}