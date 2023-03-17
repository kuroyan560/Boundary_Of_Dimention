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
	/// 入力によるXZ平面移動方向（方向指定）
	/// </summary>
	/// <param name="arg_rotate">移動対象の回転量</param>
	/// <returns>移動の単位ベクトル</returns>
	KuroEngine::Vec3<float>GetMoveVec(KuroEngine::Quaternion arg_rotate);

	/// <summary>
	/// 入力による視線移動量の取得
	/// </summary>
	/// <param name="arg_sensitivity">感度</param>
	/// <returns>視線移動角度</returns>
	KuroEngine::Vec3<KuroEngine::Angle>GetScopeMove();
};