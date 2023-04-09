//�A�������̏��
struct PlantGrass
{
    float3 m_pos;
    int m_texIdx;
    float3 m_normal;
    float m_sineLength;
    float m_appearY;
    float m_appearYTimer;
};

//�S���ނ�ŋ��ʂ���萔�o�b�t�@�p�̃p�����[�^
struct CommonGrassInfo
{
    //������΂�����
    float m_checkClipOffset;
	//���ӂɊ��ɑ��������Ă��邩�m�F����ۂ͈̔�
    float m_checkRange;
    //���ނ�o�ꎞ�̃C�[�W���O���x
    float m_appearEaseSpeed;
    //����h�炷�ۂ�Sine�� �܂蕗
    float m_sineWave;
    //�v���C���[�̍��W
    float3 m_playerPos;
};

//���ނ�ȊO�̊O���I�u�W�F�N�g�̃g�����X�t�H�[�����
struct TransformData
{
    float3 m_camPos;
    int m_seed;
    int m_grassCount;
};

//�����_��
int RandomIntInRange(int arg_minVal, int arg_maxVal, int arg_seed)
{
    return frac(sin(dot(float2(arg_seed, arg_seed + 1), float2(12.9898, 78.233))) * 43758.5453) * (arg_maxVal - arg_minVal + 1) + arg_minVal;
}