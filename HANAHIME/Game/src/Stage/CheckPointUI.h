#pragma once
#include"ForUser/Timer.h"
#include"Common/Vec.h"
#include<memory>
namespace KuroEngine
{
	class TextureBuffer;
};

class CheckPointUI
{
	//�u�`�F�b�N�|�C���g��������܂����v�摜
	std::shared_ptr<KuroEngine::TextureBuffer>m_unlockStrTex;
	//�u�`�F�b�N�|�C���g��������܂����v�`����W
	KuroEngine::Vec2<float>m_unlockStrPos;

	//�C���A���_�[���C���摜
	std::shared_ptr<KuroEngine::TextureBuffer>m_accUnderLineTex;
	//�C���A���_�[���C���`����W
	KuroEngine::Vec2<float>m_accUnderLinePos;

	//���o�̃g�[�^������
	static const int STAGING_TOTAL_TIME = 120;

	//���o�^�C�}�[
	KuroEngine::Timer m_timer;

	//�`��A���t�@�l
	float m_alpha = 1.0f;

public:
	CheckPointUI();
	void Init();
	void Update();
	void Draw();

	void Start() { m_timer.Reset(STAGING_TOTAL_TIME); }
};

