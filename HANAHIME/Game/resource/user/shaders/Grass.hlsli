//�A�������̏��
struct PlantGrass
{
    float3 m_pos;
    int m_texIdx;
    float3 m_normal;
    float m_sineLength;
    float m_appearY;
    float m_appearYTimer;
    int m_isAlive;
    int m_pad;
};

//�S���ނ�ŋ��ʂ���萔�o�b�t�@�p�̃p�����[�^
struct CommonGrassInfo
{
    //�v���C���[�̍��W
    float3 m_playerPos;
    //������΂�����
    float m_checkClipOffset;
	//���ӂɊ��ɑ��������Ă��邩�m�F����ۂ͈̔�
    float m_checkRange;
    //���ނ�o�ꎞ�̃C�[�W���O���x
    float m_appearEaseSpeed;
    //���ނ玀�S���̃C�[�W���O���x
    float m_deadEaseSpeed;
    //����h�炷�ۂ�Sine�� �܂蕗
    float m_sineWave;
    //�����͂炷����
    float m_deathDistance;
    float3 m_pad;
};

//���ނ�ȊO�̊O���I�u�W�F�N�g�̃g�����X�t�H�[�����
struct TransformData
{
    float3 m_camPos;
    int m_seed;
    float2 m_pad;
    int m_grassCount;
    int m_plantOnceCount;
};

//�����_��
int RandomIntInRange(int arg_minVal, int arg_maxVal, int arg_seed)
{
    return frac(sin(dot(float2(arg_seed, arg_seed + 1), float2(12.9898, 78.233))) * 43758.5453) * (arg_maxVal - arg_minVal + 1) + arg_minVal;
}