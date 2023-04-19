#pragma once
#include<vector>
#include<memory>
class Lever;

//�X�C�b�`�i���o�[���S�ăI���ŋN���j
class Switch
{
	friend class Stage;

	//���o�[�̎��ʔԍ�
	int m_leverID = -1;

	//���o�[�z��
	std::vector<std::weak_ptr<Lever>>m_leverArray;

	//�A�N�e�B�u��ԁi�I���I�t�؂�ւ���Ԃ��j
	bool m_isFixed = false;

public:
	void SetFixed(bool arg_flg)
	{
		m_isFixed = arg_flg;
	}

	//�S�Ẵ��o�[���I���ŋN������
	bool IsBooting()const;

	//��Ԃ��Œ肳��Ă��邩�i���o�[�𓮂����Ȃ��j
	const bool& IsFixed()const
	{
		return m_isFixed;
	}
};
