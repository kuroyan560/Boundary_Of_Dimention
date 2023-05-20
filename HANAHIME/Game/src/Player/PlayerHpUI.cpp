#include "PlayerHpUI.h"
#include"KuroEngine.h"
#include"DirectX12/D3D12App.h"
#include"FrameWork/WinApp.h"
#include"ForUser/DrawFunc/2D/DrawFunc2D.h"

void PlayerHpUI::SetHpUIStatus(HP_UI_STATUS arg_status)
{
	using namespace KuroEngine;

	if (arg_status == HP_UI_APPEAR)
	{
		m_hpUiTimer.Reset(60);
		m_hpUiStatus = HP_UI_APPEAR;
	}
	else if (arg_status == HP_UI_DRAW)
	{
		m_hpUiTimer.Reset(300);
		m_hpUiStatus = HP_UI_DRAW;
	}
	else if (arg_status == HP_UI_DISAPPEAR && m_hpUiStatus == HP_UI_DRAW)
	{
		m_hpUiTimer.Reset(60);
		m_hpUiStatus = HP_UI_DISAPPEAR;
	}
	else if (arg_status == HP_UI_DAMAGE)
	{
		m_hpUiTimer.Reset(30);
		m_hpUiStatus = HP_UI_DAMAGE;
		m_hpTexExpand = 1.0f;
		m_hpCenterOffset = { 0,0 };
		m_hpRadiusExpand = 1.0f;
		m_hpAngle = Angle(0);
		m_hpUiShake.Shake(30.0f, 1.0f, 32.0f, 64.0f);
	}
}

PlayerHpUI::PlayerHpUI()
	:m_hpUiShake({ 1.0f,1.0f,1.0f })
{
	using namespace KuroEngine;

	std::string dir = "resource/user/tex/in_game/hp/";

	for (int leafIdx = 0; leafIdx < LEAF_NUM; ++leafIdx)
	{
		m_leafTexArray[leafIdx] = D3D12App::Instance()->GenerateTextureBuffer(dir + "leaf_" + std::to_string(leafIdx) + ".png");
	}

	D3D12App::Instance()->GenerateTextureBuffer(
		m_numTexArray.data(), dir + "num_until_" + std::to_string(LEAF_NUM) + ".png", LEAF_NUM, { LEAF_NUM,1 });

	m_hpStrTex = D3D12App::Instance()->GenerateTextureBuffer(dir + "str.png");
}

void PlayerHpUI::Init()
{
	//HP��UI������
	m_hpUiShake.Init();
	SetHpUIStatus(HP_UI_APPEAR);
	m_hpUiBeatTimer.Reset(0.0f);
	m_damageFlash = true;
}

void PlayerHpUI::Update(float arg_timeScale, int arg_defaultHp, int arg_nowHp, const KuroEngine::Timer& arg_noDamageTimer)
{
	using namespace KuroEngine;

	m_hpUiTimer.UpdateTimer(arg_timeScale);

	if (m_hpUiStatus == HP_UI_APPEAR)
	{
		m_hpRadiusExpand = Math::Ease(Out, Quart, m_hpUiTimer.GetTimeRate(), 0.1f, 1.0f);
		m_hpTexExpand = Math::Ease(Out, Quart, m_hpUiTimer.GetTimeRate(0.7f), 0.0f, 1.0f);
		m_hpAngle = Math::Ease(Out, Quart, m_hpUiTimer.GetTimeRate(), Angle(-360 * 2), 0.0f);
		m_hpCenterOffset = Math::Ease(Out, Exp, m_hpUiTimer.GetTimeRate(0.8f), { -300.0f,0.0f }, { 0.0f,0.0f });
		if (m_hpUiTimer.IsTimeUp())
		{
			SetHpUIStatus(HP_UI_DRAW);
		}
	}
	else if (m_hpUiStatus == HP_UI_DAMAGE)
	{
		m_hpUiShake.Update(arg_timeScale);
		if (m_hpUiTimer.IsTimeUp())
		{
			SetHpUIStatus(HP_UI_DRAW);
		}
	}
	else if (m_hpUiStatus == HP_UI_DRAW)
	{
		//HP��MAX�̂Ƃ��͏�����
		if (m_hpUiTimer.IsTimeUp() && arg_defaultHp <= arg_nowHp)
		{
			SetHpUIStatus(HP_UI_DISAPPEAR);
		}
	}
	else if (m_hpUiStatus == HP_UI_DISAPPEAR)
	{
		m_hpRadiusExpand = Math::Ease(In, Quart, m_hpUiTimer.GetTimeRate(), 1.0f, 0.1f);
		m_hpTexExpand = Math::Ease(In, Quart, m_hpUiTimer.GetTimeRate(0.7f), 1.0f, 0.0f);
		m_hpAngle = Math::Ease(In, Quart, m_hpUiTimer.GetTimeRate(), 0.0f, Angle(-360 * 2));
		m_hpCenterOffset = Math::Ease(In, Quart, m_hpUiTimer.GetTimeRate(0.8f), { 0.0f,0.0f }, { -300.0f,0.0f });
	}

	//�S�����o
	if (m_hpUiBeatTimer.UpdateTimer(arg_timeScale))
	{
		m_hpUiBeatTimer.Reset(Math::Ease(InOut, Cubic, static_cast<float>(arg_nowHp - 1) / (arg_defaultHp - 1), 45.0f, 100.0f));
	}

	//���G���Ԓ��̃_���[�W�t�̓_��
	if (m_damageFlashTimer.UpdateTimer(arg_timeScale) || (!m_isDamageFlash && !arg_noDamageTimer.IsTimeUp()))
	{
		m_damageFlashTimer.Reset(10.0f);

		if ((!m_isDamageFlash && !arg_noDamageTimer.IsTimeUp()))
		{
			m_damageFlash = true;
		}
		else
		{
			m_damageFlash = !m_damageFlash;
		}
	}
	m_isDamageFlash = !arg_noDamageTimer.IsTimeUp();
}

void PlayerHpUI::Draw(int arg_defaultHp, int arg_nowHp, bool arg_isHitStop)
{
	using namespace KuroEngine;

	//HP���O�Ȃ��\��
	if (arg_nowHp <= 0)return;
	
	//�t�S�̂̒��S���W
	static const Vec2<float>LEAF_CENTER_POS = { 160.0f,178.0f };

	//�e�t�̒��S���W
	static const std::array<Vec2<float>,LEAF_NUM>EACH_LEAF_CENTER_POS_ARRAY = 
	{
		Vec2<float>(173.0f,124.0f),
		Vec2<float>(214.0f,157.0f),
		Vec2<float>(177.0f,229.0f),
		Vec2<float>(118.0f,229.0f),
		Vec2<float>(122.0f,167.0f),
	};

	//�e�t�̊O���Ɍ������x�N�g��
	static const std::array<Vec2<float>, LEAF_NUM>EACH_LEAF_OUTER_VEC =
	{
		EACH_LEAF_CENTER_POS_ARRAY[0] - LEAF_CENTER_POS,
		EACH_LEAF_CENTER_POS_ARRAY[1] - LEAF_CENTER_POS,
		EACH_LEAF_CENTER_POS_ARRAY[2] - LEAF_CENTER_POS,
		EACH_LEAF_CENTER_POS_ARRAY[3] - LEAF_CENTER_POS,
		EACH_LEAF_CENTER_POS_ARRAY[4] - LEAF_CENTER_POS,
	};

	//�e�t�̒ʏ펞�̔��a
	static const std::array<float, LEAF_NUM>EACH_LEAF_DEFAULT_RADIUS =
	{
		EACH_LEAF_OUTER_VEC[0].Length(),
		EACH_LEAF_OUTER_VEC[1].Length(),
		EACH_LEAF_OUTER_VEC[2].Length(),
		EACH_LEAF_OUTER_VEC[3].Length(),
		EACH_LEAF_OUTER_VEC[4].Length(),
	};

	//�ʏ펞�̃A���t�@
	static const float DEFAULT_ALPHA = 1.0f;
	//��A�N�e�B�u�̗t�̃A���t�@
	static const float LOW_ALPHA = 0.5f;

	//���ꂽHP�t�̃X�P�[��
	static const float DAMAGE_LEAF_EXPAND = 0.9f;
	//���ꂽHP�t�̃I�t�Z�b�gY
	static const float DAMAGE_LEAF_OFFSET_Y = 6.0f;
	
	//HPUI�̒��S���W
	const auto hpCenterPos = LEAF_CENTER_POS + m_hpCenterOffset + (!arg_isHitStop ? Vec2<float>(m_hpUiShake.GetOffset().x, m_hpUiShake.GetOffset().y) : Vec2<float>(0, 0));

	//HPUI�S�����o�̏��
	const auto hpBeat = Math::Ease(Out, Elastic, m_hpUiBeatTimer.GetTimeRate(0.9f), 0.9f, 1.0f);

	//HPUI�摜�̊g�嗦
	const Vec2<float>hpTexExpand = Vec2<float>(1.0f, 1.0f) * m_hpTexExpand * hpBeat;

	//�v���C���[�̂QD���W
	//HP��UI�`��
	for (int hpIdx = arg_defaultHp - 1; 0 <= hpIdx; --hpIdx)
	{
		auto pos = hpCenterPos;
		auto texExpand = hpTexExpand;

		//�x�N�g����]
		auto vec = Math::RotateVec2(EACH_LEAF_OUTER_VEC[hpIdx], m_hpAngle).GetNormal();
		pos += vec * (EACH_LEAF_DEFAULT_RADIUS[hpIdx] * m_hpRadiusExpand * hpBeat);

		float alpha = DEFAULT_ALPHA;
		//���ꂽHP
		if (arg_nowHp - 1 < hpIdx)
		{
			alpha = LOW_ALPHA;
			texExpand *= DAMAGE_LEAF_EXPAND;
			pos.y += DAMAGE_LEAF_OFFSET_Y;
		}
		//����������ꂽHP�͓_��
		if (m_isDamageFlash && arg_nowHp == hpIdx)
		{
			alpha = m_damageFlash ? LOW_ALPHA : DEFAULT_ALPHA;
		}

		DrawFunc2D::DrawRotaGraph2D(pos, texExpand, m_hpAngle, m_leafTexArray[hpIdx], alpha);
	}
}
