#include "StageManager.h"
#include"Stage.h"
#include"ForUser/DrawFunc/3D/DrawFunc3D.h"
#include"../Graphics/BasicDraw.h"
#include"../Movie/CameraData.h"
#include"../Player/Player.h"
#include"FrameWork/UsersInput.h"
#include"CheckPointHitFlag.h"
#include"../System/SaveDataManager.h"
#include"StageParts.h"

StageManager::StageManager()
	:KuroEngine::Debugger("StageManager", true, true)
{
	//デバッガでのカスタムパラメータ追加
	AddCustomParameter("Skydome", { "scaling", "skydome" }, PARAM_TYPE::FLOAT, &m_skydomeScaling, "Scaling");
	LoadParameterLog();

	//ステージのjsonファイルの所在
	std::string stageDir = "resource/user/level/";

	float terrianScaling = 1.5f;

	//ホームステージ
	m_homeStage = std::make_shared<Stage>();
	//m_homeStage->Load(0, stageDir, "New_Home.json", 5.0f, false);
	//m_homeStage->Load(0, stageDir, "P_Stage_1.json", terrianScaling, false);

	//パズルステージ一括読み込み
	int loadPazzleIdx = 1;
	while (KuroEngine::ExistFile(stageDir + "P_Stage_" + std::to_string(loadPazzleIdx) + ".json"))
	{
		DebugEnemy::Instance()->StackStage();
		m_stageArray.emplace_back(std::make_shared<Stage>());
		m_stageArray.back()->Load(loadPazzleIdx, stageDir, "P_Stage_" + std::to_string(loadPazzleIdx++) + ".json", terrianScaling, false);
	}

	//データからチェックポイントの解放を設定
	int unlockedStageNum, unlockedCheckPointOrder;
	if (SaveDataManager::Instance()->LoadStageSaveData(&unlockedStageNum, &unlockedCheckPointOrder))
	{
		for (int stageIdx = 0; stageIdx <= unlockedStageNum; ++stageIdx)
		{
			for (auto& checkPoint : m_stageArray[stageIdx]->GetCheckPointArray())
			{
				bool touched = checkPoint.lock()->GetOrder() <= unlockedCheckPointOrder;
				if (!touched)touched = stageIdx < unlockedStageNum;

				//チェックポイントを触った状態にする
				if (touched)checkPoint.lock()->SetTouch(true);
			}
		}
	}

	//現在のステージ指定（デフォルトはホーム用ステージ）
	m_nowStageIdx = 0;
	m_nowStage = m_stageArray[m_nowStageIdx];

	CameraData::Instance()->RegistCameraData("");
}

void StageManager::SetStage(int stage_num)
{
	if (stage_num == -1)
	{
		m_nowStage = m_homeStage;
	}
	else
	{
		m_nowStage = m_stageArray[stage_num];
	}
	m_nowStage->Init();
	m_nowStageIdx = stage_num;

	//チェックポイントUI初期化
	CheckPoint::UI().lock()->Init();
}

void StageManager::Update(Player& arg_player)
{
	CheckPointHitFlag::Instance()->m_isHitCheckPointTrigger = false;

	m_nowStage->Update(arg_player);
	Appearance::ModelsUvUpdate(TimeScaleMgr::s_inGame.GetTimeScale());

	//マップピンが示す目的地との当たり判定
	const auto& mapPinPointArray = m_nowStage->GetMapPinPointArray();
	//マップピンインデックスが範囲外でない
	if (!(m_nowMapPinPointIdx < 0 || static_cast<int>(mapPinPointArray.size()) <= m_nowMapPinPointIdx))
	{
		int oldMapPinPointIdx = m_nowMapPinPointIdx;

		for (int pointIdx = m_nowMapPinPointIdx; pointIdx < static_cast<int>(mapPinPointArray.size()); ++pointIdx)
		{
			//目的地点座標取得
			const auto destPos = mapPinPointArray[pointIdx].lock()->GetTransform().GetPosWorld();
			//マップピンの当たり判定半径
			const float MAP_PIN_RADIUS = 10.0f;
			if (destPos.DistanceSq(arg_player.GetTransform().GetPosWorld()) < MAP_PIN_RADIUS * MAP_PIN_RADIUS)
			{
				//マップピンを次の目的地に変更
				m_nowMapPinPointIdx = pointIdx + 1;

				//全ての目的地を巡回完了
				if (static_cast<int>(mapPinPointArray.size()) <= m_nowMapPinPointIdx)m_nowStage->SetCompleteMapPinFlg(true);
			}
		}
	}

	KuroEngine::Transform mapPinPos;
	if (GetNowMapPinTransform(&mapPinPos))
	{
		m_nowStage->SetMapPIN(mapPinPos.GetPos());
	}

	//チェックポイントUI更新
	CheckPoint::UI().lock()->Update();

}

void StageManager::Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)
{
	using namespace KuroEngine;

	Transform transform;

	m_nowStage->Draw(arg_cam, arg_ligMgr);
}

void StageManager::DrawUI(KuroEngine::Camera& arg_cam, KuroEngine::Vec3<float>arg_playerPos)
{
	//まだ全ての目的地を巡回していない
	KuroEngine::Transform mapPinPos;
	if (GetNowMapPinTransform(&mapPinPos))
	{
		const auto& mapPinPointArray = m_nowStage->GetMapPinPointArray();

		m_mapPinUI.Draw(arg_cam, mapPinPos.GetPosWorld(), arg_playerPos);
	}

	//チェックポイントUI描画
	CheckPoint::UI().lock()->Draw();
}

bool StageManager::GetNowMapPinTransform(KuroEngine::Transform* arg_destPos)
{
	if (m_nowStage->GetCompleteMapPin())return false;
	const auto& mapPinPointArray = m_nowStage->GetMapPinPointArray();
	if (m_nowMapPinPointIdx < 0 || mapPinPointArray.size() <= m_nowMapPinPointIdx)return false;
	if (arg_destPos)*arg_destPos = mapPinPointArray[m_nowMapPinPointIdx].lock()->GetTransform();
	return true;
}

KuroEngine::Transform StageManager::GetGateTransform(int arg_stageIdx, int arg_gateID) const
{
	return m_stageArray[arg_stageIdx]->GetGateTransform(arg_gateID);
}

bool StageManager::IsClearNowStage() const
{
	return m_nowStage->IsClear();
}

KuroEngine::Transform StageManager::GetStartPointTransform() const
{
	return m_stageArray[0]->GetStartPointTransform();
}

KuroEngine::Transform StageManager::GetGoalTransform() const
{
	return m_nowStage->GetGoalTransform();
}

std::shared_ptr<GoalPoint>StageManager::GetGoalModel()
{
	return m_nowStage->GetGoalModel();
}

int StageManager::GetStarCoinNum() const
{
	return m_nowStage->GetStarCoinNum();
}

int StageManager::ExistStarCoinNum() const
{
	return m_nowStage->ExistStarCoinNum();
}

bool StageManager::GetUnlockedCheckPointInfo(std::vector<std::vector<KuroEngine::Transform>>* arg_transformArray, int* arg_recentStageNum, int* arg_recentIdx) const
{
	if (!arg_transformArray || !arg_recentStageNum || !arg_recentIdx)return false;

	int reachStageNum, reachCheckPointOrder;
	if (!SaveDataManager::Instance()->LoadStageSaveData(&reachStageNum, &reachCheckPointOrder))return false;

	arg_transformArray->clear();

	for (int stageIdx = 0; stageIdx <= reachStageNum; ++stageIdx)
	{
		arg_transformArray->emplace_back();
		for (auto& checkPoint : m_stageArray[stageIdx]->GetCheckPointArray())
		{
			if (stageIdx == reachStageNum && reachCheckPointOrder < checkPoint.lock()->GetOrder())break;

			arg_transformArray->back().emplace_back(checkPoint.lock()->GetInitTransform());
			if (stageIdx == reachStageNum && reachCheckPointOrder == checkPoint.lock()->GetOrder())*arg_recentIdx = static_cast<int>(arg_transformArray->back().size()) - 1;
		}
	}

	*arg_recentStageNum = reachStageNum;

	return true;
}

void StageManager::AllStageCheckPointReset()
{
	for (auto& stage : m_stageArray)stage->CheckPointReset();
}
