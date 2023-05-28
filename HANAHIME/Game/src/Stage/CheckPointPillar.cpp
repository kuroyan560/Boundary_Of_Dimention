#include "CheckPointPillar.h"
#include "../../../../src/engine/FrameWork/Importer.h"
#include "../Stage/StageManager.h"
#include "../Graphics/BasicDraw.h"
#include "../TimeScaleMgr.h"
#include "../Stage/CheckPointHitFlag.h"
#include "../../../../src/engine/FrameWork/UsersInput.h"

CheckPointPillar::CheckPointPillar()
{

	m_pillarModel = KuroEngine::Importer::Instance()->LoadModel("resource/user/model/Stage/", "PillarLight.glb");
	m_isDraw = true;
	m_status = NORMAL;
	m_appearModeTimer.Reset(APPEAR_MODE_TIMER);
	m_exitModeTimer.Reset(EXIT_MODE_TIMER);
	m_isFirstFrame = false;

}

void CheckPointPillar::Init()
{

	m_isDraw = true;
	m_status = NORMAL;
	m_appearModeTimer.Reset(APPEAR_MODE_TIMER);
	m_exitModeTimer.Reset(EXIT_MODE_TIMER);
	m_isFirstFrame = false;

}

void CheckPointPillar::Update(const KuroEngine::Vec3<float>& arg_playerPos)
{

	//次のチェックポイントの情報を取得
	KuroEngine::Transform nextCheckPointTransform;
	StageManager::Instance()->GetNowMapPinTransform(&nextCheckPointTransform);

	//そのままの座標を保存。
	KuroEngine::Vec3<float> rawPos = nextCheckPointTransform.GetPosWorld();

	//座標を保存。
	if (!m_isFirstFrame) {

		//座標をオフセットをつけて保存。
		const float OFFSET = 20000.0f;
		m_transform.SetPos(nextCheckPointTransform.GetPosWorld() - nextCheckPointTransform.GetUp() * OFFSET);
		m_transform.SetRotate(nextCheckPointTransform.GetRotate());

	}

	//UVアニメーション

	for (auto& mesh : m_pillarModel->m_meshes)
	{
		for (auto& vertex : mesh.mesh->vertices)
		{
			vertex.uv.x += 0.01f * TimeScaleMgr::s_inGame.GetTimeScale();
		}
		mesh.mesh->Mapping();
	}

	//退出、出現するときの広がってるスケール
	const float SCALE_EXIT = 20.0f;
	const float SCALE_DEFAULT = 1.0f;

	switch (m_status)
	{
	case CheckPointPillar::NORMAL:
	{

		//アルファを下げる距離
		const float ALPHA_DEADLINE = 100.0f;
		float distance = KuroEngine::Vec3<float>(arg_playerPos - rawPos).Length();

		//近づいているときはアルファを下げる。
		if (distance < ALPHA_DEADLINE) {

			m_alpha = KuroEngine::Math::Lerp(m_alpha, 0.0f, 0.2f);

		}
		else {

			m_alpha = KuroEngine::Math::Lerp(m_alpha, 1.0f, 0.08f);

		}

		//スケールをセット。
		m_transform.SetScale(KuroEngine::Vec3<float>(1.0f, 100.0f, 1.0f));

		//チェックポイントに当たった瞬間だったらAPPEARの処理を行わわセル。
		if (CheckPointHitFlag::Instance()->m_isHitCheckPointTrigger || KuroEngine::UsersInput::Instance()->KeyInput(DIK_P)) {
			m_status = STATUS::EXIT;
			m_appearModeTimer.Reset();
			m_exitModeTimer.Reset();
		}

	}

	break;
	case CheckPointPillar::EXIT:
	{

		//退出状態での更新処理
		m_exitModeTimer.UpdateTimer(TimeScaleMgr::s_inGame.GetTimeScale());

		//スケールとアルファを変更。
		float timeRate = m_exitModeTimer.GetTimeRate(1.0f);
		float scaleRate = KuroEngine::Math::Ease(KuroEngine::Out, KuroEngine::Cubic, timeRate, 0.0f, 1.0f);

		m_alpha = 1.0f - KuroEngine::Math::Ease(KuroEngine::Out, KuroEngine::Exp, timeRate, 0.0f, 1.0f);
		m_transform.SetScale(KuroEngine::Vec3<float>(SCALE_DEFAULT + scaleRate * (SCALE_EXIT - SCALE_DEFAULT), 100.0f, SCALE_DEFAULT + scaleRate * (SCALE_EXIT - SCALE_DEFAULT)));

		//タイマーが一定時間経過したら次へ
		if (m_exitModeTimer.IsTimeUp()) {

			m_exitModeTimer.Reset();
			m_appearModeTimer.Reset();
			m_status = CheckPointPillar::APPEAR;

			//座標をオフセットをつけて保存。
			const KuroEngine::Vec3<float> OFFSET = KuroEngine::Vec3<float>(0, 20000.0f, 0);
			m_transform.SetPos(nextCheckPointTransform.GetPosWorld() - OFFSET);

		}

	}

	break;
	case CheckPointPillar::APPEAR:
	{

		//出現状態での更新処理
		m_appearModeTimer.UpdateTimer(TimeScaleMgr::s_inGame.GetTimeScale());

		//スケールとアルファを変更。
		float timeRate = m_appearModeTimer.GetTimeRate(1.0f);
		float scaleRate = KuroEngine::Math::Ease(KuroEngine::Out, KuroEngine::Back, timeRate, 0.0f, 1.0f);

		m_alpha = KuroEngine::Math::Ease(KuroEngine::Out, KuroEngine::Sine, timeRate, 0.0f, 1.0f);
		m_transform.SetScale(KuroEngine::Vec3<float>(SCALE_EXIT - scaleRate * (SCALE_EXIT - SCALE_DEFAULT), 100.0f, SCALE_EXIT - scaleRate * (SCALE_EXIT - SCALE_DEFAULT)));

		//タイマーが一定時間経過したら次へ
		if (m_appearModeTimer.IsTimeUp()) {

			m_appearModeTimer.Reset();
			m_exitModeTimer.Reset();
			m_status = CheckPointPillar::NORMAL;

		}

	}

	break;
	default:
		break;
	}

	m_isFirstFrame = true;
}

void CheckPointPillar::Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr, std::weak_ptr<KuroEngine::DepthStencil>arg_ds)
{

	//描画する状態だったら描画
	if (m_isDraw) {

		auto param = IndividualDrawParameter::GetDefault();
		param.m_alpha = m_alpha;
		BasicDraw::Instance()->Draw_NoOutline(arg_ds, arg_cam, arg_ligMgr, m_pillarModel, m_transform, param, KuroEngine::AlphaBlendMode_Trans);

	}

}
