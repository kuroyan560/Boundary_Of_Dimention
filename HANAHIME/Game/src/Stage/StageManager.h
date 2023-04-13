#pragma once
#include"Common/Singleton.h"
#include"ForUser/Debugger.h"
#include"Common/Transform.h"

#include<memory>
namespace KuroEngine
{
	class Model;
	class Camera;
	class LightManager;
}

class Stage;
class Player;

class StageManager : public KuroEngine::DesignPattern::Singleton<StageManager>, public KuroEngine::Debugger
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

	//ホームステージ
	std::shared_ptr<Stage>m_homeStage;

	//デバッグ用テストステージ
	std::vector<std::shared_ptr<Stage>>m_stageArray;

	//現在のステージ
	std::shared_ptr<Stage>m_nowStage;

public:
	void SetStage(int stage_num = -1);

	void Update(Player& arg_player);
	void Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr);

	//現在のステージのゲッタ
	std::weak_ptr<Stage>GetNowStage() { return m_nowStage; }
	int GetAllStageNum()
	{
		return static_cast<int>(m_stageArray.size());
	}

	//プレイヤーの初期化トランスフォーム
	KuroEngine::Transform GetPlayerSpawnTransform()const;

	KuroEngine::Transform GetGoalTransform()const;
};