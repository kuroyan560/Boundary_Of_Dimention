#pragma once
#include"KuroEngine.h"
#include"Common/Singleton.h"
#include"ForUser/Debugger.h"

class OperationConfig : public KuroEngine::DesignPattern::Singleton<OperationConfig>, public KuroEngine::Debugger
{
	friend class KuroEngine::DesignPattern::Singleton<OperationConfig>;
	OperationConfig();

	enum INPUT_DEVICE { KEY_BOARD_MOUSE, CONTROLLER, NUM }m_nowInputDevice = INPUT_DEVICE::KEY_BOARD_MOUSE;
	const std::array<std::string, INPUT_DEVICE::NUM>m_inputDeviceNames =
	{
		"KEY_BOARD_MOUSE","CONTROLLER"
	};

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

	bool m_isActive = false;

	void OnImguiItems()override;

	//�Ō�Ɏg�������̓f�o�C�X���L�^
	void RegisterLatestDevice(INPUT_DEVICE arg_device)
	{
		m_nowInputDevice = arg_device;
	}

public:
	void SetActive(bool arg_active) { m_isActive = arg_active; }

	/// <summary>
	/// ���͂ɂ��XZ���ʈړ������i�����w��j
	/// </summary>
	/// <param name="arg_rotate">�ړ��Ώۂ̉�]��</param>
	/// <returns>�ړ��̒P�ʃx�N�g��</returns>
	KuroEngine::Vec3<float>GetMoveVec(KuroEngine::Quaternion arg_rotate);
	KuroEngine::Vec3<float>GetMoveVecFuna(KuroEngine::Quaternion arg_rotate);

	/// <summary>
	/// ���͂ɂ�鎋���ړ��ʂ̎擾
	/// </summary>
	/// <param name="arg_sensitivity">���x</param>
	/// <returns>�����ړ��p�x�i���W�A���j</returns>
	KuroEngine::Vec3<float>GetScopeMove();

	//����{�^��
	bool InputDone();
	//�L�����Z���{�^��
	bool InputCancel();
};