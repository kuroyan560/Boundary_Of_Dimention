#pragma once
#include"Common/Transform.h"
#include"../../../../src/engine/Render/RenderObject/ModelInfo/ModelMesh.h"
#include<vector>
#include"StageParts.h"
#include"json.hpp"
#include<memory>
#include<map>

namespace KuroEngine
{
	class Model;
	class TextureBuffer;
	class LightManager;
	class Camera;
}

class Switch;

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

	//全てをオン状態にすることがクリア条件となるレバーの識別番号
	int m_goalLeverID = Lever::INVALID_ID;
	//クリアのスイッチ
	std::shared_ptr<Switch>m_goalSwitch;

	//キーがjsonファイルに含まれているか、含まれていなかったらエラーで終了
	bool CheckJsonKeyExist(std::string arg_fileName, nlohmann::json arg_json, std::string arg_key);

	//座標系を考慮した座標読み込み
	KuroEngine::Vec3<float>GetConsiderCoordinate(nlohmann::json arg_json);

	//座標配列の読み込み
	bool LoadTranslationArray(std::string arg_fileName,
		std::vector<KuroEngine::Vec3<float>>* arg_result,
		nlohmann::json arg_json);

	//種別に応じて読み込みを分岐させる
	void LoadWithType(std::string arg_fileName, std::string arg_typeKey, nlohmann::json arg_json);

public:
	Stage();
	
	/// <summary>
	/// 地形のトランスフォーム初期化
	/// </summary>
	void GimmickInit();

	void GimmickUpdate(Player& arg_player);

	//クリア判定
	bool IsClear()const;

	//地形の描画
	void TerrianDraw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr);

	//ステージ情報読み込み
	void Load(std::string arg_dir, std::string arg_fileName, float arg_terrianScaling, bool arg_hasGoal = true);

	//通常の地形の配列取得
	const std::vector<Terrian>& GetTerrianArray()const { return m_terrianArray; }
	std::list<std::shared_ptr<StageParts>>& GetGimmickArray(){ return m_gimmickArray; }

	//プレイヤーの初期化トランスフォーム
	KuroEngine::Transform GetPlayerSpawnTransform()const
	{
		if (m_startPoint)return m_startPoint->GetTransform();
		return KuroEngine::Transform();
	}

	KuroEngine::Transform GetGoalTransform()const
	{
		if (m_goalPoint)return m_goalPoint->GetTransform();
		return KuroEngine::Transform();
	};

	std::shared_ptr<GoalPoint>GetGoalModel()
	{
		return m_goalPoint;
	}

//モデルのゲッタ
	//スカイドーム
	std::weak_ptr<KuroEngine::Model>GetSkydomeModel() { return m_skydomeModel; }
	//森林円柱
	std::weak_ptr<KuroEngine::Model>GetWoodsCylinderModel() { return m_woodsCylinderModel; }
	//地面
	std::weak_ptr<KuroEngine::TextureBuffer>GetGroundTex() { return m_groundTex; }


};