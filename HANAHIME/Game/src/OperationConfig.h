#pragma once
#include"KuroEngine.h"
#include"Common/Singleton.h"
#include"ForUser/Debugger.h"
#include"FrameWork/UsersInput.h"

class OperationConfig : public KuroEngine::DesignPattern::Singleton<OperationConfig>, public KuroEngine::Debugger
{
public:
	enum INPUT_PATTERN { HOLD, ON_TRIGGER, OFF_TRIGGER, ON_OFF_TRIGGER };

private:
	friend class KuroEngine::DesignPattern::Singleton<OperationConfig>;
	OperationConfig();
	
	//�f�o�b�O�̃L�[���͂��󂯕t���邩
	bool m_isDebug = true;

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

	//�R���g���[���[�ɂ�����
	bool ControllerInput(INPUT_PATTERN arg_pattern, KuroEngine::XBOX_BUTTON arg_xboxButton);
	//�L�[�ɂ�����
	bool KeyInput(INPUT_PATTERN arg_pattern, int arg_keyCode);

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
	bool InputDone(INPUT_PATTERN arg_pattern = ON_TRIGGER);
	//�L�����Z���{�^��
	bool InputCancel(INPUT_PATTERN arg_pattern = ON_TRIGGER);

	//�J�����̋������[�h�؂�ւ��{�^��
	bool InputCamDistModeChange(INPUT_PATTERN arg_pattern = ON_TRIGGER);
	//�J�������Z�b�g
	bool InputCamReset(INPUT_PATTERN arg_pattern = ON_TRIGGER);

	//����A�N�V�����{�^��
	bool InputSink(INPUT_PATTERN arg_pattern = HOLD);

	//�W�b�v���C��
	bool InputRideZipLine(INPUT_PATTERN arg_pattern = ON_TRIGGER);

	//���g���C�{�^���i�|�[�Y��ʂ���I�����ă��g���C����悤�ɂȂ邩���A�Ȃ��Ȃ�\������j
	bool InputRetry(INPUT_PATTERN arg_pattern = ON_TRIGGER);

	//�f�o�b�O�p�̃L�[����
	bool DebugKeyInputOnTrigger(int arg_keyCode);
};