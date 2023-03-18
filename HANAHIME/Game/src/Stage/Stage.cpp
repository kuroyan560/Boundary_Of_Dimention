#include "Stage.h"
#include"FrameWork/Importer.h"
#include"ForUser/JsonData.h"
#include"FrameWork/Importer.h"
#include"../Graphics/BasicDraw.h"

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

void Stage::TerrianInit(float arg_scaling)
{
	for (auto& terrian : m_terrianArray)
	{
		terrian.m_transform.SetPos(terrian.m_initializedTransform.GetPos() * arg_scaling);
		terrian.m_transform.SetScale(terrian.m_initializedTransform.GetScale() * arg_scaling);
		terrian.m_transform.SetRotate(terrian.m_initializedTransform.GetRotate());
	}
}

void Stage::TerrianDraw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)
{
	for (auto& terrian : m_terrianArray)
	{
		BasicDraw::Instance()->Draw(
			arg_cam,
			arg_ligMgr,
			terrian.m_model.lock(),
			terrian.m_transform);
	}
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

		//地形の名前設定
		auto name = obj["name"].get<std::string>();

		//モデル設定
		auto model = Importer::Instance()->LoadModel(s_terrianModelDir, obj["file_name"].get<std::string>() + ".glb");

		//トランスフォーム取得
		auto transformObj = obj["transform"];

		//平行移動
		Vec3<float>translation = { -(float)transformObj["translation"][0],(float)transformObj["translation"][2],-(float)transformObj["translation"][1] };

		//回転
		Vec3<float>rotate = { -(float)transformObj["rotation"][1],-(float)transformObj["rotation"][2], (float)transformObj["rotation"][0] };
		//ラジアンに直す
		rotate.x = Angle::ConvertToRadian(rotate.x);
		rotate.y = Angle::ConvertToRadian(rotate.y);
		rotate.z = Angle::ConvertToRadian(rotate.z);

		//スケーリング
		Vec3<float>scaling = { (float)transformObj["scaling"][0],(float)transformObj["scaling"][2] ,(float)transformObj["scaling"][1] };

		//トランスフォーム設定
		Transform transform;
		transform.SetPos(translation);
		transform.SetRotate(XMQuaternionRotationRollPitchYaw(rotate.z, rotate.y, rotate.x));
		transform.SetScale(scaling);

		//地形追加
		m_terrianArray.emplace_back(name, model, transform);
	}
}
