#include "Stage.h"
#include"FrameWork/Importer.h"
#include"ForUser/JsonData.h"
#include"FrameWork/Importer.h"

std::string Stage::s_terrianModelDir = "resource/user/model/terrian/";

Stage::Stage()
{
	using namespace KuroEngine;

//デフォルトのモデル
	//スカイドーム
	static std::shared_ptr<Model>s_defaultSkydomeModel
		= Importer::Instance()->LoadModel("resource/user/model/", "Skydome.glb");
	//森林円柱
	static std::shared_ptr<Model>s_defaultWoodsCylinderModel
		= Importer::Instance()->LoadModel("resource/user/model/", "Woods.glb");

//デフォルトの画像
	//地面
	static std::shared_ptr<TextureBuffer>s_defaultGroundTex
		= D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/ground.png");

//デフォルト値設定
	m_skydomeModel = s_defaultSkydomeModel;
	m_woodsCylinderModel = s_defaultWoodsCylinderModel;
	m_groundTex = s_defaultGroundTex;
}

void Stage::Load(std::string arg_dir, std::string arg_fileName)
{
	using namespace KuroEngine;

	//地形情報クリア
	m_terrianArray.clear();

	JsonData jsonData(arg_dir, arg_fileName);
	for (auto& obj : jsonData.m_jsonData["objects"])
	{
		//地形の名前のパラメータがない
		if (!obj.contains("name"))continue;

		//モデルの名前のパラメータがない
		if (!obj.contains("file_name"))continue;

		//トランスフォームのパラメータがない
		if (!obj.contains("transform"))continue;

		//地形追加
		m_terrianArray.emplace_back();
		//新しい地形情報の参照
		auto& newTerrian = m_terrianArray.back();

		//地形の名前設定
		newTerrian.m_name = obj["name"].get<std::string>();

		//モデル設定
		newTerrian.m_model = Importer::Instance()->LoadModel(s_terrianModelDir, obj["file_name"].get<std::string>() + ".glb");

		//トランスフォーム取得
		auto transform = obj["transform"];

		//平行移動
		Vec3<float>translation = { -(float)transform["translation"][0],(float)transform["translation"][2],-(float)transform["translation"][1] };

		//回転
		Vec3<float>rotate = { -(float)transform["rotation"][1],-(float)transform["rotation"][2], (float)transform["rotation"][0] };
		//ラジアンに直す
		rotate.x = Angle::ConvertToRadian(rotate.x);
		rotate.y = Angle::ConvertToRadian(rotate.y);
		rotate.z = Angle::ConvertToRadian(rotate.z);

		//スケーリング
		Vec3<float>scaling = { (float)transform["scaling"][0],(float)transform["scaling"][2] ,(float)transform["scaling"][1] };

		//トランスフォーム設定
		newTerrian.m_transform.SetPos(translation);
		newTerrian.m_transform.SetRotate(XMQuaternionRotationRollPitchYaw(rotate.z, rotate.y, rotate.x));
		newTerrian.m_transform.SetScale(scaling);
	}
}
