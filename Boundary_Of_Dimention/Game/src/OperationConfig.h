#pragma once
#include"KuroEngine.h"
#include"Common/Singleton.h"

class OperationConfig : public KuroEngine::DesignPattern::Singleton<OperationConfig>
{
	friend class KuroEngine::DesignPattern::Singleton<OperationConfig>;
	OperationConfig();

	//�R���g���[���[���삩
	bool m_controller = false;

	//�J�������x
	float m_camSensitivity = 0.2f;

public:
	/// <summary>
	/// ���͂ɂ��ړ��ʂ̎擾
	/// </summary>
	/// <param name="arg_moveScalar">�ړ���</param>
	/// <returns>�ړ��x�N�g��</returns>
	KuroEngine::Vec3<float> GetMove(float arg_moveScalar);
};