#pragma once
#include"KuroEngine.h"
#include"Common/Singleton.h"
#include"ForUser/Debugger.h"
#include"FrameWork/UsersInput.h"

class OperationConfig : public KuroEngine::DesignPattern::Singleton<OperationConfig>, public KuroEngine::Debugger
{
public:
	//���̓p�^�[��
	enum INPUT_PATTERN { HOLD, ON_TRIGGER, OFF_TRIGGER, ON_OFF_TRIGGER };

	//������͎��
	enum OPERATION_TYPE
	{
		DONE,	//����
		CANCEL,	//�L�����Z��
		CAM_DIST_MODE_CHANGE_FPS,	//�J�����̋������[�h�؂�ւ�
		CAM_RESET,	//�J�������Z�b�g
		SINK_GROUND,	//�n���ɐ���
		RIDE_ZIP_LINE,	//�W�b�v���C���ɏ��
		PAUSE,	//�|�[�Y��ʂ�
		OPERATION_TYPE_NUM
	};

	//�I����͕���
	enum SELECT_VEC
	{
		SELECT_VEC_UP,
		SELECT_VEC_DOWN,
		SELECT_VEC_LEFT,
		SELECT_VEC_RIGHT
	};

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

	//���̓f�o�C�X���Ƃ̃p�����[�^
	struct Parameter
	{
		float m_camSensitivity = 0.2f;

		Parameter(float arg_camSensitivity)
			:m_camSensitivity(arg_camSensitivity) {}
	};

	//�L�[�}�E���R���g���[���[�ňႤ�p�����[�^
	std::array<Parameter, static_cast<int>(INPUT_DEVICE::NUM)>m_params =
	{
		Parameter(0.2f),
		Parameter(0.1f),
	};

	//�L�[�{�[�h���͂̂Ƃ��̊����L�[
	std::array<int, OPERATION_TYPE_NUM>m_operationKeyCode;
	//�R���g���[���[���͂̂Ƃ��̊����{�^��
	std::array<KuroEngine::XBOX_BUTTON, OPERATION_TYPE_NUM>m_operationButton;

	//�C���Q�[���̑�����͂��󂯕t���Ă��邩
	bool m_isInGameOperationActive = true;
	//�C���Q�[��������͂̔z��
	const std::vector<OPERATION_TYPE>m_inGameOperationArray =
	{
		CAM_DIST_MODE_CHANGE_FPS,
		CAM_RESET,
		SINK_GROUND,
		RIDE_ZIP_LINE,
	};
	//�S�Ă̓��͂��󂯂Ď󂯂Ă��邩
	bool m_isAllInputActive = true;

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
	void SetInGameOperationActive(bool arg_active) { m_isInGameOperationActive = arg_active; }
	void SetAllInputActive(bool arg_active) { m_isAllInputActive = arg_active; }

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

	//�I�����ڕ�������
	bool GetSelectVec(SELECT_VEC arg_vec);

	//�������
	bool GetOperationInput(OPERATION_TYPE arg_operation, INPUT_PATTERN arg_pattern);

	//������͂���������
	bool CheckAllOperationInput();

	//�f�o�b�O�p�̃L�[����
	bool DebugKeyInputOnTrigger(int arg_keyCode);
};