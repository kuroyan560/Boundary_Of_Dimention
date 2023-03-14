#pragma once
#include"Common/Singleton.h"
#include"ForUser/Debugger.h"

#include<memory>
namespace KuroEngine
{
	class Model;
	class Camera;
}

class Stage;

class StageManager : public KuroEngine::DesignPattern::Singleton<StageManager>,public KuroEngine::Debugger
{
	friend class KuroEngine::DesignPattern::Singleton<StageManager>;
	StageManager();

	//スカイドームの大きさ
	float m_skydomeScaling = 1.0f;
	//森林円柱の半径
	float m_woodsRadius = 1.0f;
	//森林円柱の高さ
	float m_woodsHeight = 1.0f;
	//地面の大きさ
	float m_groundScaling = 1.0f;

	////スカイドームの大きさ
	//float m_skydomeScaling = 500.0f;
	////森林円柱の半径
	//float m_woodsRadius = 309.0f;
	////森林円柱の高さ
	//float m_woodsHeight = 186.0f;
	////地面の大きさ
	//float m_groundScaling = 311.4f;

	//デバッグ用テストステージ
	std::shared_ptr<Stage>m_testStage;

	//現在のステージ
	std::shared_ptr<Stage>m_nowStage;

	//Imguiデバッグ関数オーバーライド
	void OnImguiItems()override;

public:
	void Draw(KuroEngine::Camera& arg_cam);
};

