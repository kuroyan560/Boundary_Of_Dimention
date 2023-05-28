#pragma once
#include"Common/Singleton.h"
#include"ForUser/Debugger.h"
#include"Common/Transform.h"
#include"StageParts.h"
#include"../HUD/MapPinUI.h"
#include"../HUD/CheckPointUI.h"
#include"../Effect/GuideInsect.h"

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

	//ホームステージ
	std::shared_ptr<Stage>m_homeStage;

	//デバッグ用テストステージ
	std::vector<std::shared_ptr<Stage>>m_stageArray;

	//現在のステージ
	std::shared_ptr<Stage>m_nowStage;
	int m_nowStageIdx;

	//マップピンUI
	MapPinUI m_mapPinUI;
	//現在マップピンが指す地点のインデックス
	int m_nowMapPinPointIdx;
public:
	void SetStage(int stage_num = -1);

	void Update(Player& arg_player);
	void Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr);
	void DrawUI(KuroEngine::Camera& arg_cam, KuroEngine::Vec3<float>arg_playerPos);

	//現在のステージのゲッタ
	std::weak_ptr<Stage>GetNowStage() { return m_nowStage; }
	const int& GetNowStageIdx()const{ return m_nowStageIdx; }

	KuroEngine::Transform GetGateTransform(int arg_stageIdx, int arg_gateID)const;

	//ステージの数
	int GetAllStageNum()
	{
		return static_cast<int>(m_stageArray.size());
	}

	//クリア判定
	bool IsClearNowStage()const;

	//プレイヤーの初期化トランスフォーム
	KuroEngine::Transform GetStartPointTransform()const;

	bool GetNowMapPinTransform(KuroEngine::Transform* arg_destPos);

	KuroEngine::Transform GetGoalTransform()const;

	std::shared_ptr<GoalPoint>GetGoalModel();

	//入手したスターコインの数
	int GetStarCoinNum()const;
	//存在するスターコインの数
	int ExistStarCoinNum()const;

	//スカイドームのスケーリングゲッタ
	const float& GetSkyDomeScaling()const { return m_skydomeScaling; }

	//解放済のチェックポイントのトランスフォーム配列
	//std::vector<std::vector<KuroEngine::Transform>>GetUnlockedCheckPointTransformArray()const;
	bool GetUnlockedCheckPointInfo(std::vector<std::vector<KuroEngine::Transform>>* arg_transformArray, int* arg_recentStageNum, int* arg_recentIdx)const;
};