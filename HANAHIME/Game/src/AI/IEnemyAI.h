#pragma once
#include"Common/Transform.h"
#include"DirectX12/D3D12Data.h"
#include<memory>
#include<vector>
#include"../AI/EnemyStatus.h"
#include"../Stage/StageManager.h"

enum ReactionEnum
{
	FIND,
	LOOK,
	FAR_AWAY,
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
	bool m_appearFlag;			//登場
	bool m_showEffectFlag;		//何が起きたのかしっかり見せる
	bool m_preDissappearFlag;	//消える準備
	bool m_disappearFlag;		//消滅

	bool finishFlag;

	void Find()
	{
		KuroEngine::Vec2<float>appearScale(1.5f, 0.0f);
		KuroEngine::Vec2<float>showScale(1.0f, 1.0f);
		KuroEngine::Vec2<float>preDisappearScale(1.5f, 0.0f);
		KuroEngine::Vec2<float>disappearScale(0.0f, 1.0f);

		//登場
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
		//暫く見せる
		if (m_showEffectFlag)
		{
			if (m_scaleTimer.IsTimeUp())
			{
				m_showEffectFlag = false;
				m_preDissappearFlag = true;
				m_scaleTimer.Reset(5);
			}
		}
		//隠れる寸前
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
		//消滅
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
		m_tex[LOOK] = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/reaction/hatena.png");
		m_tex[FAR_AWAY] = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/reaction/hatena.png");

		m_reaction = std::make_unique<Reaction>(m_tex);
	}

	bool IsActive(const KuroEngine::Transform &arg_enemyTrasform, const KuroEngine::Transform &arg_playerTransform)
	{
		bool isTrueFlag = arg_enemyTrasform.GetUp().Dot(arg_playerTransform.GetUpWorld()) <= 0.5f;
		return isTrueFlag;
	}

	//Rotaitonから上ベクトルを得る
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


	bool IsHitWall(KuroEngine::Transform arg_transform, const KuroEngine::Vec3<float> &sightVec, float sightLength)
	{
		auto nowStage = StageManager::Instance()->GetNowStage();

		//フェンスとの当たり判定
		for (auto &terrian : nowStage.lock()->GetGimmickArray())
		{
			//動く足場でない
			if (terrian->GetType() != StageParts::SPLATOON_FENCE)continue;

			//距離によってカリング
			const float DEADLINE = terrian->GetTransform().GetScale().Length() * 5.0f;
			float distance = KuroEngine::Vec3<float>(terrian->GetTransform().GetPosWorld() - arg_transform.GetPosWorld()).Length();
			if (DEADLINE < distance) continue;

			//動く足場としてキャスト
			auto ivyBlock = dynamic_pointer_cast<SplatoonFence>(terrian);

			//モデル情報取得
			auto model = terrian->GetModel();

			//メッシュを走査
			for (auto &modelMesh : model.lock()->m_meshes)
			{

				//CastRayに渡す引数を更新。
				auto hitmesh = ivyBlock->GetCollisionMesh()[static_cast<int>(&modelMesh - &model.lock()->m_meshes[0])];

				//判定↓============================================

				//当たり判定を実行
				CollisionDetectionOfRayAndMesh::MeshCollisionOutput output = CollisionDetectionOfRayAndMesh::Instance()->MeshCollision(arg_transform.GetPosWorld(), sightVec, hitmesh);

				//当たり判定を大きくしたい場合は↓を変えて下さい！
				if (output.m_isHit && fabs(output.m_distance) < sightLength) {


					return true;

				}

				//=================================================
			}
		}


		return false;
	}


	EnemyHeadAttack m_deadMotion;
	std::unique_ptr<Reaction> m_reaction;
	std::vector<std::shared_ptr<KuroEngine::TextureBuffer>>m_tex;

};