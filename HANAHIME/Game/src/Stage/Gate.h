#pragma once
#include"Common/Transform.h"
#include"ForUser/JsonData.h"
#include"Render/RenderObject/Light.h"
#include"../Graphics/BasicDraw.h"
#include"FrameWork/Importer.h"

/// <summary>
/// 扉の初期化情報
/// </summary>
struct GateData
{
	GateData(KuroEngine::Transform transform = {}, int stage_num = -1) :
		m_transform(transform), m_stageNum(stage_num)
	{};

	KuroEngine::Transform m_transform;
	int m_stageNum;
};

/// <summary>
/// 別のステージ移動する用の扉
/// </summary>
class Gate
{
public:
	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="transform">座標、大きさ、角度</param>
	/// <param name="stage_num">移動するステージ番号</param>
	Gate(const KuroEngine::Transform &transform, int stage_num);

	void Update();

	/// <summary>
	/// 扉との判定
	/// </summary>
	/// <param name="player_pos">プレイヤーの座標</param>
	/// <returns>true...扉と接触,false...接触していない</returns>
	bool IsHit(const KuroEngine::Vec3<float> &player_pos);

	int GetStageNum() { return m_stageNum; };

	KuroEngine::Vec3<float>ForceVel();

	/// <summary>
	/// 当たり判定可視化
	/// </summary>
	void DebugDraw(KuroEngine::Camera &arg_cam, KuroEngine::LightManager &arg_ligMgr);

private:
	std::shared_ptr<KuroEngine::Model>m_model;
	KuroEngine::Transform m_transform;
	int m_stageNum;
};

