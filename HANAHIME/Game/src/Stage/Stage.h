#pragma once
#include"Common/Transform.h"
#include<vector>

#include<memory>
namespace KuroEngine
{
	class Model;
	class TextureBuffer;
}

//�n�`���
struct Terrian
{
	//�n�`��
	std::string m_name;
	//���f���|�C���^
	std::weak_ptr<KuroEngine::Model>m_model;
	//�g�����X�t�H�[��
	KuroEngine::Transform m_transform;
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

	//�X�e�[�W�̃��[�h
	void Load(std::string arg_dir, std::string arg_fileName);

	std::vector<Terrian>& GetTerrianArray() { return m_terrianArray; }

//���f���̃Q�b�^
	//�X�J�C�h�[��
	std::weak_ptr<KuroEngine::Model>GetSkydomeModel() { return m_skydomeModel; }
	//�X�щ~��
	std::weak_ptr<KuroEngine::Model>GetWoodsCylinderModel() { return m_woodsCylinderModel; }
	//�n��
	std::weak_ptr<KuroEngine::TextureBuffer>GetGroundTex() { return m_groundTex; }

};