#pragma once
#include<memory>
namespace KuroEngine
{
	class Model;
	class TextureBuffer;
}

class Stage
{

//���f��
	//�X�J�C�h�[��
	std::shared_ptr<KuroEngine::Model>m_skydomeModel;
	//�X�щ~��
	std::shared_ptr<KuroEngine::Model>m_woodsCylinderModel;

//�摜
	//�n��
	std::shared_ptr<KuroEngine::TextureBuffer>m_groundTex;

public:
	Stage();

//���f���̃Q�b�^
	//�X�J�C�h�[��
	std::weak_ptr<KuroEngine::Model>GetSkydomeModel() { return m_skydomeModel; }
	//�X�щ~��
	std::weak_ptr<KuroEngine::Model>GetWoodsCylinderModel() { return m_woodsCylinderModel; }
	//�n��
	std::weak_ptr<KuroEngine::TextureBuffer>GetGroundTex() { return m_groundTex; }

};