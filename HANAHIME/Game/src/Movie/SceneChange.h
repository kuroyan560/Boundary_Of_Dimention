#pragma once
#include"Common/Transform.h"
#include"Render/RenderObject/SpriteMesh.h"
#include"ForUser/Timer.h"

/// <summary>
/// ���ړ����̑J�ډ��o
/// </summary>
class SceneChange
{
public:
	SceneChange();

	void Update();
	void Draw();

	void Start();
	bool IsHide();
	bool IsAppear();

	const bool& IsActive()const { return m_startFlag; }

private:
	bool m_startFlag;	//�J�n�t���O
	bool m_blackOutFlag;//�Ó]�����u��
	bool m_appearFlag;	//���]

	//�Ó]�Ɩ��]�̎��Ԋ֘A----------------------------------------
	KuroEngine::Timer m_time;
	int m_countTimeUpNum;
	const float SCENE_CHANGE_TIME;
	//�Ó]�Ɩ��]�̎��Ԋ֘A----------------------------------------

	//�e�N�X�`���֘A�̏��----------------------------------------
	float m_alpha, m_alphaOffset;
	KuroEngine::Vec2<float>m_size;
	std::shared_ptr<KuroEngine::TextureBuffer> m_blackTexBuff;
	//�e�N�X�`���֘A�̏��----------------------------------------

};
