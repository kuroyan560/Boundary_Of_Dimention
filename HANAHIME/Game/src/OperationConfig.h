#pragma once
#include"KuroEngine.h"
#include"Common/Singleton.h"
#include"ForUser/Debugger.h"
#include"FrameWork/UsersInput.h"

class OperationConfig : public KuroEngine::DesignPattern::Singleton<OperationConfig>, public KuroEngine::Debugger
{
public:
	//入力パターン
	enum INPUT_PATTERN { HOLD, ON_TRIGGER, OFF_TRIGGER, ON_OFF_TRIGGER };

	//操作入力種別
	enum OPERATION_TYPE
	{
		DONE,	//決定
		CANCEL,	//キャンセル
		CAM_DIST_MODE_CHANGE_FPS,	//カメラの距離モード切り替え
		CAM_RESET,	//カメラリセット
		SINK_GROUND,	//地中に潜る
		RIDE_ZIP_LINE,	//ジップラインに乗る
		PAUSE,	//ポーズ画面へ
		OPERATION_TYPE_NUM
	};

	//選択入力方向
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
	
	//デバッグのキー入力を受け付けるか
	bool m_isDebug = true;

	enum INPUT_DEVICE { KEY_BOARD_MOUSE, CONTROLLER, NUM }m_nowInputDevice = INPUT_DEVICE::KEY_BOARD_MOUSE;
	const std::array<std::string, INPUT_DEVICE::NUM>m_inputDeviceNames =
	{
		"KEY_BOARD_MOUSE","CONTROLLER"
	};

	//入力デバイスごとのパラメータ
	struct Parameter
	{
		float m_camSensitivity = 0.2f;

		Parameter(float arg_camSensitivity)
			:m_camSensitivity(arg_camSensitivity) {}
	};

	//キーマウかコントローラーで違うパラメータ
	std::array<Parameter, static_cast<int>(INPUT_DEVICE::NUM)>m_params =
	{
		Parameter(0.2f),
		Parameter(0.1f),
	};

	//キーボード入力のときの割当キー
	std::array<int, OPERATION_TYPE_NUM>m_operationKeyCode;
	//コントローラー入力のときの割当ボタン
	std::array<KuroEngine::XBOX_BUTTON, OPERATION_TYPE_NUM>m_operationButton;

	//インゲームの操作入力を受け付けているか
	bool m_isInGameOperationActive = true;
	//インゲーム操作入力の配列
	const std::vector<OPERATION_TYPE>m_inGameOperationArray =
	{
		CAM_DIST_MODE_CHANGE_FPS,
		CAM_RESET,
		SINK_GROUND,
		RIDE_ZIP_LINE,
	};
	//全ての入力を受けて受けているか
	bool m_isAllInputActive = true;

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
	void SetInGameOperationActive(bool arg_active) { m_isInGameOperationActive = arg_active; }
	void SetAllInputActive(bool arg_active) { m_isAllInputActive = arg_active; }

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

	//選択項目方向入力
	bool GetSelectVec(SELECT_VEC arg_vec);

	//操作入力
	bool GetOperationInput(OPERATION_TYPE arg_operation, INPUT_PATTERN arg_pattern);

	//操作入力があったか
	bool CheckAllOperationInput();

	//デバッグ用のキー入力
	bool DebugKeyInputOnTrigger(int arg_keyCode);
};