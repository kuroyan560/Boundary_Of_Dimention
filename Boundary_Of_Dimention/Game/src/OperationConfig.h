#pragma once
#include"KuroEngine.h"
#include"Common/Singleton.h"

class OperationConfig : public KuroEngine::DesignPattern::Singleton<OperationConfig>
{
	friend class KuroEngine::DesignPattern::Singleton<OperationConfig>;
	OperationConfig();

	//コントローラー操作か
	bool m_controller = false;

	//カメラ感度
	float m_camSensitivity = 0.2f;

public:
	/// <summary>
	/// 入力による移動量の取得
	/// </summary>
	/// <param name="arg_moveScalar">移動量</param>
	/// <returns>移動ベクトル</returns>
	KuroEngine::Vec3<float> GetMove(float arg_moveScalar);
};