#pragma once
#include"Common/Transform.h"
#include"../../../../src/engine/Render/RenderObject/ModelInfo/ModelMesh.h"
#include<vector>
#include"StageParts.h"
#include"json.hpp"
#include<memory>

namespace KuroEngine
{
	class Model;
	class TextureBuffer;
	class LightManager;
	class Camera;
}

class Stage
{
private:
	//地形のスケール
	float m_terrianScaling = 1.0f;

	//地形情報配列
	std::vector<Terrian>m_terrianArray;
	//ギミック配列（要素のサイズが異なるためlistを利用）
	std::list<std::shared_ptr<StageParts>>m_gimmickArray;
	//スタート地点
	std::shared_ptr<StartPoint>m_startPoint;
	//ゴール地点
	std::shared_ptr<GoalPoint>m_goalPoint;

//モデル
	//地形モデルの存在するディレクトリ
	static std::string s_terrianModelDir;
	//スカイドーム
	std::shared_ptr<KuroEngine::Model>m_skydomeModel;
	//森林円柱
	std::shared_ptr<KuroEngine::Model>m_woodsCylinderModel;

//画像
	//地面
	std::shared_ptr<KuroEngine::TextureBuffer>m_groundTex;

	//キーがjsonファイルに含まれているか、含まれていなかったらエラーで終了
	bool CheckJsonKeyExist(std::string arg_fileName, nlohmann::json arg_json, std::string arg_key);

	//動く足場の読み込み
	bool LoadMoveScaffold(std::string arg_fileName,
		std::shared_ptr<StageParts>* arg_result, 
		nlohmann::json arg_json,
		std::weak_ptr<KuroEngine::Model>arg_model,
		KuroEngine::Transform arg_initTransform);

	//種別に応じて読み込みを分岐させる
	void LoadWithType(std::string arg_fileName, std::string arg_typeKey, nlohmann::json arg_json);

public:
	Stage();

	
	/// <summary>
	/// 地形のトランスフォーム初期化
	/// </summary>
	void TerrianInit();

	//地形の描画
	void TerrianDraw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr);

	//ステージ情報読み込み
	void Load(std::string arg_dir, std::string arg_fileName, float arg_terrianScaling, bool arg_hasGoal = true);

	//通常の地形の配列取得
	const std::vector<Terrian>& GetTerrianArray()const { return m_terrianArray; }

	//プレイヤーの初期化トランスフォーム
	KuroEngine::Transform GetPlayerSpawnTransform()const
	{
		if (m_startPoint)return m_startPoint->GetTransform();
		return KuroEngine::Transform();
	}

//モデルのゲッタ
	//スカイドーム
	std::weak_ptr<KuroEngine::Model>GetSkydomeModel() { return m_skydomeModel; }
	//森林円柱
	std::weak_ptr<KuroEngine::Model>GetWoodsCylinderModel() { return m_woodsCylinderModel; }
	//地面
	std::weak_ptr<KuroEngine::TextureBuffer>GetGroundTex() { return m_groundTex; }

private:
};