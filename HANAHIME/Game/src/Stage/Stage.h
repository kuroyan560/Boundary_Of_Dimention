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
	//地形情報配列
	std::vector<Terrian>m_terrianArray;
	//ギミック配列（要素のサイズが異なるためlistを利用）
	std::list<std::shared_ptr<StageParts>>m_gimmickArray;

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

public:
	Stage();

	
	/// <summary>
	/// 地形のトランスフォーム初期化
	/// </summary>
	/// <param name="arg_scaling">スケーリング</param>
	void TerrianInit(float arg_scaling);

	//地形の描画
	void TerrianDraw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr);

	//ステージ情報読み込み
	void Load(std::string arg_dir, std::string arg_fileName);

	//通常の地形の配列取得
	const std::vector<Terrian>& GetTerrianArray()const { return m_terrianArray; }

//モデルのゲッタ
	//スカイドーム
	std::weak_ptr<KuroEngine::Model>GetSkydomeModel() { return m_skydomeModel; }
	//森林円柱
	std::weak_ptr<KuroEngine::Model>GetWoodsCylinderModel() { return m_woodsCylinderModel; }
	//地面
	std::weak_ptr<KuroEngine::TextureBuffer>GetGroundTex() { return m_groundTex; }


private:

};