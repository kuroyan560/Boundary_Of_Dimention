#pragma once
#include"KuroEngine.h"
#include"Common/Singleton.h"
#include"ForUser/Debugger.h"

class OperationConfig : public KuroEngine::DesignPattern::Singleton<OperationConfig>, public KuroEngine::Debugger
{
	friend class KuroEngine::DesignPattern::Singleton<OperationConfig>;
	OperationConfig();

	//コントローラー操作か
	bool m_controller = false;

	//カメラ感度
	float m_camSensitivity = 0.2f;

	void OnImguiItems()override;

public:
	/// <summary>
	/// 入力による移動量の取得
	/// </summary>
	/// <param name="arg_moveScalar">移動量</param>
	/// <returns>移動ベクトル</returns>
	KuroEngine::Vec3<float> GetMove(float arg_moveScalar);

	/// <summary>
	/// 入力による視線移動量の取得
	/// </summary>
	/// <param name="arg_sensitivity">感度</param>
	/// <returns>視線移動角度</returns>
	KuroEngine::Vec3<KuroEngine::Angle>GetScopeMove(float arg_sensitivity);
};