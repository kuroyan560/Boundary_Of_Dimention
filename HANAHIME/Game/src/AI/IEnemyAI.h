#pragma once
#include"Common/Transform.h"
#include"DirectX12/D3D12Data.h"
#include<memory>
#include<vector>

enum ReactionEnum
{
	FIND,
	HIT,
	LOOK,
	FAR_AWAY,
	DEAD,
	MAX
};


class Reaction
{
public:
	Reaction(std::vector<std::shared_ptr<KuroEngine::TextureBuffer>> buffer)
	{
		m_timer.Reset(120);
		m_timer.ForciblyTimeUp();
		m_tex = buffer;
	}

	void Init(int index, const KuroEngine::Vec3<float> &upVec)
	{
		m_index = index;
		m_timer.Reset(120);
		finishFlag = false;
		m_appearFlag = true;
		m_scaleTimer.Reset(5);

		m_baseScale = { 2.0f,5.0f };
		m_downScale = m_baseScale;
		m_upVec = upVec;
	}

	void Update(const KuroEngine::Vec3<float> &pos)
	{
		m_pos = KuroEngine::Math::Ease(KuroEngine::Out, KuroEngine::Back, 1.0f, pos, pos + KuroEngine::Vec3<float>(10.0f, 10.0f, 10.0f) * m_upVec);
		m_timer.UpdateTimer();

		Find();
		m_scaleTimer.UpdateTimer();
	}

	void Draw(KuroEngine::Camera &camera)
	{
		if (!finishFlag)
		{
			KuroEngine::Transform transform;
			transform.SetPos(m_pos);
			transform.SetScale({ m_upScale.x,m_upScale.y,1.0f });
			BasicDraw::Instance()->DrawBillBoard(camera, m_pos, m_upScale, m_downScale, m_tex[m_index]);
		}
		else
		{
			m_pos = { 0.0f,0.0f,0.0f };
		}
	}

	bool Done()
	{
		return finishFlag;
	}

private:
	int m_index;
	KuroEngine::Vec3<float>m_pos;
	KuroEngine::Vec3<float>m_upVec;
	KuroEngine::Vec2<float>m_upScale;
	KuroEngine::Vec2<float>m_downScale;
	KuroEngine::Vec2<float>m_baseScale;
	KuroEngine::Timer m_timer;
	std::vector<std::shared_ptr<KuroEngine::TextureBuffer>>m_tex;


	KuroEngine::Timer m_scaleTimer;
	bool m_appearFlag;			//ìoèÍ
	bool m_showEffectFlag;		//âΩÇ™ãNÇ´ÇΩÇÃÇ©ÇµÇ¡Ç©ÇËå©ÇπÇÈ
	bool m_preDissappearFlag;	//è¡Ç¶ÇÈèÄîı
	bool m_disappearFlag;		//è¡ñ≈

	bool finishFlag;

	void Find()
	{
		KuroEngine::Vec2<float>appearScale(1.5f, 0.0f);
		KuroEngine::Vec2<float>showScale(1.0f, 1.0f);
		KuroEngine::Vec2<float>preDisappearScale(1.5f, 0.0f);
		KuroEngine::Vec2<float>disappearScale(0.0f, 1.0f);

		//ìoèÍ
		if (m_appearFlag)
		{
			m_upScale = KuroEngine::Math::Ease(KuroEngine::Out, KuroEngine::Circ, m_scaleTimer.GetTimeRate(), { 1.5f,-m_baseScale.y }, showScale * m_baseScale);
			if (m_scaleTimer.IsTimeUp())
			{
				m_appearFlag = false;
				m_showEffectFlag = true;
				m_scaleTimer.Reset(30);
			}
		}
		//ébÇ≠å©ÇπÇÈ
		if (m_showEffectFlag)
		{
			if (m_scaleTimer.IsTimeUp())
			{
				m_showEffectFlag = false;
				m_preDissappearFlag = true;
				m_scaleTimer.Reset(5);
			}
		}
		//âBÇÍÇÈê°ëO
		if (m_preDissappearFlag)
		{
			m_upScale = KuroEngine::Math::Ease(KuroEngine::Out, KuroEngine::Circ, m_scaleTimer.GetTimeRate(), showScale * m_baseScale, preDisappearScale * m_baseScale);
			m_downScale.x = m_upScale.x;
			if (m_scaleTimer.IsTimeUp())
			{
				m_preDissappearFlag = false;
				m_disappearFlag = true;
				m_scaleTimer.Reset(5);
			}
		}
		//è¡ñ≈
		if (m_disappearFlag)
		{
			m_upScale = KuroEngine::Math::Ease(KuroEngine::Out, KuroEngine::Circ, m_scaleTimer.GetTimeRate(), preDisappearScale * m_baseScale, disappearScale * m_baseScale);
			m_downScale.x = m_upScale.x;
			if (m_scaleTimer.IsTimeUp())
			{
				m_disappearFlag = false;
				m_scaleTimer.Reset(60);
			}
		}
	}
};

class IEnemyAI
{
public:

	IEnemyAI()
	{
		m_tex.reserve(MAX);
		m_tex.resize(MAX);
		m_tex[FIND] = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/reaction/Find.png");
		m_tex[HIT] = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/reaction/Attack.png");
		m_tex[LOOK] = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/reaction/hatena.png");
		m_tex[FAR_AWAY] = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/reaction/hatena.png");
		m_tex[DEAD] = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/reaction/dead.png");

		m_reaction = std::make_unique<Reaction>(m_tex);
	}

	bool IsActive(const KuroEngine::Transform &arg_enemyTrasform, const KuroEngine::Transform &arg_playerTransform)
	{
		bool isTrueFlag = arg_enemyTrasform.GetUp().Dot(arg_playerTransform.GetUpWorld()) <= 0.5f;
		return isTrueFlag;
	}

	//RotaitonÇ©ÇÁè„ÉxÉNÉgÉãÇìæÇÈ
	KuroEngine::Vec3<float>GetUpNomal(const KuroEngine::Quaternion &quaternion)
	{
		KuroEngine::Vec3<float>vec;
		vec.x = -std::clamp(quaternion.m128_f32[0], -1.0f, 1.0f);
		vec.y = -std::clamp(quaternion.m128_f32[1], -1.0f, 1.0f);
		vec.z = -std::clamp(quaternion.m128_f32[2], -1.0f, 1.0f);
		return vec;
	}


	void DeadMotion(KuroEngine::Vec3<float> &pos, KuroEngine::Vec3<float> &vec, KuroEngine::Quaternion &rotation)
	{
		pos += KuroEngine::Vec3<float>(1.0f, 1.0f, 1.0f) * vec;
		rotation.m128_f32[0] += 1.0f;
	}


	std::unique_ptr<Reaction> m_reaction;
	std::vector<std::shared_ptr<KuroEngine::TextureBuffer>>m_tex;

};