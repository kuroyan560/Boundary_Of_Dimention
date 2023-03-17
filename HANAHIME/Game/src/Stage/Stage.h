#pragma once
#include"Common/Transform.h"
#include<vector>

#include<memory>
namespace KuroEngine
{
	class Model;
	class TextureBuffer;
}

//地形情報
struct Terrian
{
	//地形名
	std::string m_name;
	//モデルポインタ
	std::weak_ptr<KuroEngine::Model>m_model;
	//トランスフォーム
	KuroEngine::Transform m_transform;
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

	//ステージのロード
	void Load(std::string arg_dir, std::string arg_fileName);

	std::vector<Terrian>& GetTerrianArray() { return m_terrianArray; }

//モデルのゲッタ
	//スカイドーム
	std::weak_ptr<KuroEngine::Model>GetSkydomeModel() { return m_skydomeModel; }
	//森林円柱
	std::weak_ptr<KuroEngine::Model>GetWoodsCylinderModel() { return m_woodsCylinderModel; }
	//地面
	std::weak_ptr<KuroEngine::TextureBuffer>GetGroundTex() { return m_groundTex; }

};