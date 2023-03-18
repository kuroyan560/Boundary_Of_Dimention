#pragma once
#include"Common/Transform.h"
#include<vector>

#include<memory>
namespace KuroEngine
{
	class Model;
	class TextureBuffer;
	class LightManager;
	class Camera;
}

//地形情報
struct Terrian
{
	//地形名
	std::string m_name;
	//モデルポインタ
	std::weak_ptr<KuroEngine::Model>m_model;
	//デフォルト（元データ）のトランスフォーム
	const KuroEngine::Transform m_initializedTransform;
	//トランスフォーム
	KuroEngine::Transform m_transform;

	Terrian(std::string arg_name, std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform)
		:m_name(arg_name), m_model(arg_model), m_initializedTransform(arg_initTransform) {}
};

class Stage
{
private:
	//地形情報配列
	std::vector<Terrian>m_terrianArray;

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

public:
	Stage();

	
	/// <summary>
	/// 地形のトランスフォーム初期化
	/// </summary>
	/// <param name="arg_scaling">スケーリング</param>
	void TerrianInit(float arg_scaling);

	//地形の描画
	void TerrianDraw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr);

	//ステージのロード
	void Load(std::string arg_dir, std::string arg_fileName);

	const std::vector<Terrian>& GetTerrianArray()const { return m_terrianArray; }

//モデルのゲッタ
	//スカイドーム
	std::weak_ptr<KuroEngine::Model>GetSkydomeModel() { return m_skydomeModel; }
	//森林円柱
	std::weak_ptr<KuroEngine::Model>GetWoodsCylinderModel() { return m_woodsCylinderModel; }
	//地面
	std::weak_ptr<KuroEngine::TextureBuffer>GetGroundTex() { return m_groundTex; }

};