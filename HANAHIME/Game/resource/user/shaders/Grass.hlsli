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
};

//���ނ�ȊO�̊O���I�u�W�F�N�g�̃g�����X�t�H�[�����
struct TransformData
{
    float3 m_camPos;
    float3 m_checkPlantPos;
};
