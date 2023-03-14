#include "Stage.h"
#include"FrameWork/ModelImporter.h"

Stage::Stage()
{
	using namespace KuroEngine;

//デフォルトのモデル
	//スカイドーム
	static std::shared_ptr<Model>s_defaultSkydomeModel
		= ModelImporter::Instance()->LoadModel("resource/user/model/", "Skydome.glb");
	//森林円柱
	static std::shared_ptr<Model>s_defaultWoodsCylinderModel
		= ModelImporter::Instance()->LoadModel("resource/user/model/", "Woods.glb");

//デフォルトの画像
	//地面
	static std::shared_ptr<TextureBuffer>s_defaultGroundTex
		= D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/ground.png");

//デフォルト値設定
	m_skydomeModel = s_defaultSkydomeModel;
	m_woodsCylinderModel = s_defaultWoodsCylinderModel;
	m_groundTex = s_defaultGroundTex;
}
