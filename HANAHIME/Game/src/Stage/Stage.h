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

//�n�`���
struct Terrian
{
	//�n�`��
	std::string m_name;
	//���f���|�C���^
	std::weak_ptr<KuroEngine::Model>m_model;
	//�f�t�H���g�i���f�[�^�j�̃g�����X�t�H�[��
	const KuroEngine::Transform m_initializedTransform;
	//�g�����X�t�H�[��
	KuroEngine::Transform m_transform;

	Terrian(std::string arg_name, std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform)
		:m_name(arg_name), m_model(arg_model), m_initializedTransform(arg_initTransform) {}
};

class Stage
{
private:
	//�n�`���z��
	std::vector<Terrian>m_terrianArray;

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

public:
	Stage();

	
	/// <summary>
	/// �n�`�̃g�����X�t�H�[��������
	/// </summary>
	/// <param name="arg_scaling">�X�P�[�����O</param>
	void TerrianInit(float arg_scaling);

	//�n�`�̕`��
	void TerrianDraw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr);

	//�X�e�[�W�̃��[�h
	void Load(std::string arg_dir, std::string arg_fileName);

	const std::vector<Terrian>& GetTerrianArray()const { return m_terrianArray; }

//���f���̃Q�b�^
	//�X�J�C�h�[��
	std::weak_ptr<KuroEngine::Model>GetSkydomeModel() { return m_skydomeModel; }
	//�X�щ~��
	std::weak_ptr<KuroEngine::Model>GetWoodsCylinderModel() { return m_woodsCylinderModel; }
	//�n��
	std::weak_ptr<KuroEngine::TextureBuffer>GetGroundTex() { return m_groundTex; }

};