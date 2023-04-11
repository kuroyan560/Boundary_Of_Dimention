#include "Stage.h"
#include"FrameWork/Importer.h"
#include"ForUser/JsonData.h"
#include"FrameWork/Importer.h"
#include"ForUser/Object/Model.h"
#include "StageParts.h"

std::string Stage::s_terrianModelDir = "resource/user/model/terrian/";

bool Stage::CheckJsonKeyExist(std::string arg_fileName, nlohmann::json arg_json, std::string arg_key)
{
	bool exist = arg_json.contains(arg_key);
	if (!exist)
	{
		KuroEngine::AppearMessageBox("Stage : CheckJsonKeyExist() 失敗", arg_fileName + " に\"" + arg_key + "\"が含まれてないよ。");
	}
	return exist;
}

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
		terrian.Init(arg_scaling);
	}

}

void Stage::TerrianDraw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)
{
	for (auto& terrian : m_terrianArray)
	{
		terrian.Draw(arg_cam, arg_ligMgr);
	}
}

void Stage::Load(std::string arg_dir, std::string arg_fileName)
{
	using namespace KuroEngine;

	//地形情報クリア
	m_terrianArray.clear();
	m_gimmickArray.clear();

	JsonData jsonData(arg_dir, arg_fileName);

	//ステージ情報でない
	if (!CheckJsonKeyExist(arg_fileName,jsonData.m_jsonData, "stage"))return;

	auto stageJsonData = jsonData.m_jsonData["stage"];
	for (auto& obj : stageJsonData["objects"])
	{
		//種別のパラメータがない
		if (!CheckJsonKeyExist(arg_fileName,obj, "type"))break;

		//モデルの名前のパラメータがない
		if (!CheckJsonKeyExist(arg_fileName,obj, "file_name"))break;

		//トランスフォームのパラメータがない
		if (!CheckJsonKeyExist(arg_fileName,obj, "transform"))break;

		//モデル設定
		auto model = Importer::Instance()->LoadModel(s_terrianModelDir, obj["file_name"].get<std::string>() + ".glb");

		//トランスフォーム取得
		auto transformObj = obj["transform"];

		//平行移動
		Vec3<float>translation = { -(float)transformObj["translation"][0],(float)transformObj["translation"][2],(float)transformObj["translation"][1] };

		//回転
		XMVECTOR quaternion = { (float)transformObj["rotation"][0],(float)transformObj["rotation"][2], -(float)transformObj["rotation"][1],(float)transformObj["rotation"][3] };

		//スケーリング
		Vec3<float>scaling = { (float)transformObj["scaling"][0],(float)transformObj["scaling"][2] ,(float)transformObj["scaling"][1] };

		//トランスフォーム設定
		Transform transform;
		transform.SetPos(translation);
		transform.SetRotate(quaternion);
		transform.SetScale(scaling);

		//地形追加
		m_terrianArray.emplace_back(model, transform);
	}
}