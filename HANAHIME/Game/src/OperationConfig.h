#pragma once
#include"KuroEngine.h"
#include"Common/Singleton.h"
#include"ForUser/Debugger.h"

class OperationConfig : public KuroEngine::DesignPattern::Singleton<OperationConfig>, public KuroEngine::Debugger
{
	friend class KuroEngine::DesignPattern::Singleton<OperationConfig>;
	OperationConfig();

	//�R���g���[���[���삩
	bool m_controller = false;

	//�J�������x
	float m_camSensitivity = 0.2f;

	void OnImguiItems()override;

public:
	/// <summary>
	/// ���͂ɂ��ړ��ʂ̎擾
	/// </summary>
	/// <param name="arg_moveScalar">�ړ���</param>
	/// <returns>�ړ��x�N�g��</returns>
	KuroEngine::Vec3<float> GetMove(float arg_moveScalar);

	/// <summary>
	/// ���͂ɂ�鎋���ړ��ʂ̎擾
	/// </summary>
	/// <param name="arg_sensitivity">���x</param>
	/// <returns>�����ړ��p�x</returns>
	KuroEngine::Vec3<KuroEngine::Angle>GetScopeMove(float arg_sensitivity);
};