#include "Stage.h"
#include"FrameWork/ModelImporter.h"

Stage::Stage()
{
	using namespace KuroEngine;

//�f�t�H���g�̃��f��
	//�X�J�C�h�[��
	static std::shared_ptr<Model>s_defaultSkydomeModel
		= ModelImporter::Instance()->LoadModel("resource/user/model/", "Skydome.glb");
	//�X�щ~��
	static std::shared_ptr<Model>s_defaultWoodsCylinderModel
		= ModelImporter::Instance()->LoadModel("resource/user/model/", "Woods.glb");

//�f�t�H���g�̉摜
	//�n��
	static std::shared_ptr<TextureBuffer>s_defaultGroundTex
		= D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/ground.png");

//�f�t�H���g�l�ݒ�
	m_skydomeModel = s_defaultSkydomeModel;
	m_woodsCylinderModel = s_defaultWoodsCylinderModel;
	m_groundTex = s_defaultGroundTex;
}
