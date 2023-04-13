#pragma once
#include"Gate.h"
#include"../Movie/StageChange.h"
#include"../Movie/MovieCamera.h"

/// <summary>
/// ホーム画面
/// </summary>
class HomeStageSelect
{
public:
	HomeStageSelect();
	void Init();
	void Update();
	void Draw(KuroEngine::Camera &arg_cam, KuroEngine::LightManager &arg_ligMgr);

	/// <summary>
	/// 扉と判定を取り、番号を受け取る
	/// </summary>
	/// <param name="player_pos">プレイヤーの座標</param>
	/// <returns>ステージ番号</returns>
	int GetStageNumber(const KuroEngine::Vec3<float> &player_pos);
private:
	std::vector<std::unique_ptr<Gate>>m_gateArray;
	std::vector<GateData>m_gateDataArray;

	MovieCameraData titleCameraData;
};

