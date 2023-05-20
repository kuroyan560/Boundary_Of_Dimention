#pragma once
#include<memory>
#include"Common/Vec.h"
#include"Common/Angle.h"
#include"ForUser/Timer.h"
#include"ForUser/ImpactShake.h"

namespace KuroEngine
{
	class TextureBuffer;
}

class PlayerHpUI
{
	//HPUI�̉��o�X�e�[�^�X
	enum HP_UI_STATUS { HP_UI_APPEAR, HP_UI_DRAW, HP_UI_DISAPPEAR, HP_UI_DAMAGE }m_hpUiStatus;
	//HP��UI�e�N�X�`��
	std::shared_ptr<KuroEngine::TextureBuffer>m_hpTex;
	std::shared_ptr<KuroEngine::TextureBuffer>m_hpDamageTex;
	//HPUI�̒��S���W
	KuroEngine::Vec2<float>m_hpCenterPos = { 100.0f,110.0f };
	//HPUI�̒��S���W�I�t�Z�b�g
	KuroEngine::Vec2<float>m_hpCenterOffset;
	//HPUI�̔��a�g�嗦
	float m_hpRadiusExpand = 1.0f;
	//HPUI�̉摜�̊g�嗦
	float m_hpTexExpand = 1.0f;
	//HPUI�̉�]�p�x
	KuroEngine::Angle m_hpAngle = KuroEngine::Angle(0);
	//HPUI�̓o�ꉉ�o�^�C�}�[
	KuroEngine::Timer m_hpUiTimer;
	//HPUI�̐U��
	KuroEngine::ImpactShake m_hpUiShake;
	//HPUI�̐S�����o�^�C�}�[
	KuroEngine::Timer m_hpUiBeatTimer;

	void SetHpUIStatus(HP_UI_STATUS arg_status);

public:
	PlayerHpUI();
	void Init();
	void Update(float arg_timeScale, int arg_defaultHp, int arg_nowHp);
	void Draw(int arg_defaultHp, int arg_nowHp, bool arg_isHitStop, const KuroEngine::Timer& arg_noDamageTimer);

	void OnDamage()
	{
		SetHpUIStatus(HP_UI_DAMAGE);
	}
};

