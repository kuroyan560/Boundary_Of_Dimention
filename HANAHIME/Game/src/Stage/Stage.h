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
	//�n�`���z��
	std::vector<Terrian>m_terrianArray;
	//�M�~�b�N�z��i�v�f�̃T�C�Y���قȂ邽��list�𗘗p�j
	std::list<std::shared_ptr<StageParts>>m_gimmickArray;

//���f��
	//�n�`���f���̑��݂���f�B���N�g��
	static std::string s_terrianModelDir;
	//�X�J�C�h�[��
	std::shared_ptr<KuroEngine::Model>m_skydomeModel;
	//�X�щ~��
	std::shared_ptr<KuroEngine::Model>m_woodsCylinderModel;

//�摜
	//�n��
	std::shared_ptr<KuroEngine::TextureBuffer>m_groundTex;

	//�L�[��json�t�@�C���Ɋ܂܂�Ă��邩�A�܂܂�Ă��Ȃ�������G���[�ŏI��
	bool CheckJsonKeyExist(std::string arg_fileName, nlohmann::json arg_json, std::string arg_key);

public:
	Stage();

	
	/// <summary>
	/// �n�`�̃g�����X�t�H�[��������
	/// </summary>
	/// <param name="arg_scaling">�X�P�[�����O</param>
	void TerrianInit(float arg_scaling);

	//�n�`�̕`��
	void TerrianDraw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr);

	//�X�e�[�W���ǂݍ���
	void Load(std::string arg_dir, std::string arg_fileName);

	//�ʏ�̒n�`�̔z��擾
	const std::vector<Terrian>& GetTerrianArray()const { return m_terrianArray; }

//���f���̃Q�b�^
	//�X�J�C�h�[��
	std::weak_ptr<KuroEngine::Model>GetSkydomeModel() { return m_skydomeModel; }
	//�X�щ~��
	std::weak_ptr<KuroEngine::Model>GetWoodsCylinderModel() { return m_woodsCylinderModel; }
	//�n��
	std::weak_ptr<KuroEngine::TextureBuffer>GetGroundTex() { return m_groundTex; }


private:

};