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
};

//�S���ނ�ŋ��ʂ���萔�o�b�t�@�p�̃p�����[�^
struct CommonGrassInfo
{
    //�r���[�s��
    matrix matView;
    //�v���W�F�N�V�����s��
    matrix matProjection;
    //�J�������W�i���[���h���W�j
    float3 eye;
    //������΂�����
    float m_checkClipOffset;
    //�v���C���[�̍��W
    float3 m_playerPos;
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
};

//���ނ�ȊO�̊O���I�u�W�F�N�g�̃g�����X�t�H�[�����
struct TransformData
{
    float3 m_camPos;
    float m_seed;
    int m_grassCount;
    int m_plantOnceCount;
};

//�����_��
float RandomIntInRange(float arg_seed)
{   
    return frac(sin(dot(float2(arg_seed, arg_seed + 1.0f), float2(12.9898f, 78.233f))) * 43758.5453f);
}