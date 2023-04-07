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

class ConvertModelToGateData
{
public:
	static std::vector<GateData>Convert(std::string arg_dir, std::string arg_fileName, float terrian_Scale)
	{
		std::vector<GateData> gateDataArray;

		KuroEngine::JsonData jsonData(arg_dir, arg_fileName);
		for (auto &obj : jsonData.m_jsonData["objects"])
		{
			//地形の名前のパラメータがない
			if (!obj.contains("name"))continue;
			//モデルの名前のパラメータがない
			if (!obj.contains("file_name"))continue;
			//トランスフォームのパラメータがない
			if (!obj.contains("transform"))continue;

			//地形の名前設定
			auto name = obj["name"].get<std::string>();

			if (name.find("Gate") == std::string::npos)continue;


			//トランスフォーム取得
			auto transformObj = obj["transform"];

			//平行移動
			KuroEngine::Vec3<float>translation = { -(float)transformObj["translation"][0] * terrian_Scale,(float)transformObj["translation"][2] * terrian_Scale + 25.0f,(float)transformObj["translation"][1] * terrian_Scale };

			//回転
			KuroEngine::Vec3<float>rotate = { -(float)transformObj["rotation"][1],-(float)transformObj["rotation"][2], (float)transformObj["rotation"][0] };
			//ラジアンに直す
			rotate.x = KuroEngine::Angle::ConvertToRadian(rotate.x);
			rotate.y = KuroEngine::Angle::ConvertToRadian(rotate.y);
			rotate.z = KuroEngine::Angle::ConvertToRadian(rotate.z);

			//スケーリング
			KuroEngine::Vec3<float>scaling = { (float)transformObj["scaling"][0],(float)transformObj["scaling"][2] ,(float)transformObj["scaling"][1] };

			//トランスフォーム設定
			KuroEngine::Transform transform;
			transform.SetPos(translation);
			transform.SetRotate(XMQuaternionRotationRollPitchYaw(rotate.z, rotate.y, rotate.x));
			transform.SetScale(scaling);

			gateDataArray.emplace_back(transform, 0);
		}
		return gateDataArray;
	};
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

