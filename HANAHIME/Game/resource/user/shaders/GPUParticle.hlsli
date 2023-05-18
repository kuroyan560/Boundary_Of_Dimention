struct ParticleData
{
    matrix world;
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

matrix SetPosInMatrix(matrix mat,float3 pos)
{
    matrix result = mat;
    result[0][3] = pos.x;
    result[1][3] = pos.y;
    result[2][3] = pos.z;
    return result;
};

matrix SetScaleInMatrix(float3 scale)
{
    matrix matScale;
    matScale[0] = float4(scale.x, 0.0f, 0.0f, 0.0f);
    matScale[1] = float4(0.0f, scale.y, 0.0f, 0.0f);
    matScale[2] = float4(0.0f, 0.0f, scale.z, 0.0f);
    matScale[3] = float4(0.0f, 0.0f, 0.0f, 1.0f);
    return matScale;
};

uint GetIndex(uint3 groupID,uint3 groupThreadID)
{
    uint index = (groupThreadID.y * 1204) + groupThreadID.x + groupThreadID.z;
    index += 1024 * groupID.x;
    return index;
}

float3 Larp(float3 BASE_POS,float3 POS,float MUL)
{
    float3 distance = BASE_POS - POS;
    distance *= MUL;
    return POS + distance;
}

float2 LarpFloat2(float2 BASE_POS,float2 POS,float MUL)
{
    float2 distance = BASE_POS - POS;
    distance *= MUL;
    return POS + distance;
}

float GetRate(float min,float max)
{
    return min / max;
}


uint rand_xorshift(uint seed)
{
    uint rng_state = seed;
    // Xorshift algorithm from George Marsaglia's paper
    rng_state ^= (rng_state << 13);
    rng_state ^= (rng_state >> 17);
    rng_state ^= (rng_state << 5);
    return rng_state;
}

float RandXorShift(uint seed,float MAX,float MIN)
{
    float f0 = float(rand_xorshift(seed * 42949)) * (1.0 / 4294967296.0);
    float result = (MAX + abs(MIN)) * f0 - abs(MIN);
    return result;
}

int parkMiller(int seed)
{
    const int a = 16807;
    const int m = 2147483647; // 2^31 - 1
    const int q = 127773;
    const int r = 2836;

    int hi = seed / q;
    int lo = seed % q;
    int test = a * lo - r * hi;

    if (test > 0) {
        seed = test;
    } else {
        seed = test + m;
    }
    return seed;
}

float randomParkMiller(int seed)
{
    seed = parkMiller(seed);
    return float(seed) / 2147483647.0;
}

float RandFrac(float2 uv)
{
    return frac(sin(dot(uv,float2(12.9898,78.233))*43758.5453123));
}

float3 IsNanToZero(float3 vec)
{
    float3 result = vec;
    if(isnan(result.x))
    {
        result.x = 0.0f;
    }
    if(isnan(result.y))
    {
        result.y = 0.0f;
    }
     if(isnan(result.z))
    {
        result.z = 0.0f;
    }
    return result;
}