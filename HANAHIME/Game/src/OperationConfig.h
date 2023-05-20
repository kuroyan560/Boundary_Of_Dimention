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
	
	//デバッグのキー入力を受け付けるか
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

	//最後に使った入力デバイスを記録
	void RegisterLatestDevice(INPUT_DEVICE arg_device)
	{
		m_nowInputDevice = arg_device;
	}

	//コントローラーによる入力
	bool ControllerInput(INPUT_PATTERN arg_pattern, KuroEngine::XBOX_BUTTON arg_xboxButton);
	//キーによる入力
	bool KeyInput(INPUT_PATTERN arg_pattern, int arg_keyCode);

public:
	void SetActive(bool arg_active) { m_isActive = arg_active; }

	/// <summary>
	/// 入力によるXZ平面移動方向（方向指定）
	/// </summary>
	/// <param name="arg_rotate">移動対象の回転量</param>
	/// <returns>移動の単位ベクトル</returns>
	KuroEngine::Vec3<float>GetMoveVec(KuroEngine::Quaternion arg_rotate);
	KuroEngine::Vec3<float>GetMoveVecFuna(KuroEngine::Quaternion arg_rotate);

	/// <summary>
	/// 入力による視線移動量の取得
	/// </summary>
	/// <param name="arg_sensitivity">感度</param>
	/// <returns>視線移動角度（ラジアン）</returns>
	KuroEngine::Vec3<float>GetScopeMove();

	//決定ボタン
	bool InputDone(INPUT_PATTERN arg_pattern = ON_TRIGGER);
	//キャンセルボタン
	bool InputCancel(INPUT_PATTERN arg_pattern = ON_TRIGGER);

	//カメラの距離モード切り替えボタン
	bool InputCamDistModeChange(INPUT_PATTERN arg_pattern = ON_TRIGGER);
	//カメラリセット
	bool InputCamReset(INPUT_PATTERN arg_pattern = ON_TRIGGER);

	//潜るアクションボタン
	bool InputSink(INPUT_PATTERN arg_pattern = HOLD);

	//ジップライン
	bool InputRideZipLine(INPUT_PATTERN arg_pattern = ON_TRIGGER);

	//リトライボタン（ポーズ画面から選択してリトライするようになるかも、なくなる可能性ある）
	bool InputRetry(INPUT_PATTERN arg_pattern = ON_TRIGGER);

	//デバッグ用のキー入力
	bool DebugKeyInputOnTrigger(int arg_keyCode);
};