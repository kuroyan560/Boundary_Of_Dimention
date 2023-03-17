#pragma once
#include"KuroEngine.h"
#include"Common/Singleton.h"
#include"ForUser/Debugger.h"

class OperationConfig : public KuroEngine::DesignPattern::Singleton<OperationConfig>, public KuroEngine::Debugger
{
	friend class KuroEngine::DesignPattern::Singleton<OperationConfig>;
	OperationConfig();

	enum INPUT_DEVICE { KEY_BOARD_MOUSE, CONTROLLER, NUM }m_nowInputDevice = INPUT_DEVICE::KEY_BOARD_MOUSE;

	struct Parameter
	{
		float m_camSensitivity = 0.2f;

		Parameter(float arg_camSensitivity)
			:m_camSensitivity(arg_camSensitivity) {}
	};

	std::array<Parameter, static_cast<int>(INPUT_DEVICE::NUM)>m_params =
	{
		Parameter(0.2f),
		Parameter(0.1f),
	};

	void OnImguiItems()override;

public:
	/// <summary>
	/// ���͂ɂ��XZ���ʈړ������i�����w��j
	/// </summary>
	/// <param name="arg_rotate">�ړ��Ώۂ̉�]��</param>
	/// <returns>�ړ��̒P�ʃx�N�g��</returns>
	KuroEngine::Vec3<float>GetMoveVec(KuroEngine::Quaternion arg_rotate);

	/// <summary>
	/// ���͂ɂ�鎋���ړ��ʂ̎擾
	/// </summary>
	/// <param name="arg_sensitivity">���x</param>
	/// <returns>�����ړ��p�x</returns>
	KuroEngine::Vec3<KuroEngine::Angle>GetScopeMove();
};