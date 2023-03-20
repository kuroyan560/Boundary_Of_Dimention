#include "DebugCamera.h"
#include"FrameWork/WinApp.h"
#include"FrameWork/UsersInput.h"

KuroEngine::DebugCamera::DebugCamera()
{
	m_cam = std::make_shared<Camera>("DebugCamera");

	auto& transform = m_cam->GetTransform();
	transform.SetParent(&m_posAngleTransform);
	m_posAngleTransform.SetParent(&m_gazePointTransform);

	//画面サイズに対する相対的なスケールに調整
	m_scale.x = 1.0f / (float)KuroEngine::WinApp::Instance()->GetExpandWinSize().x;
	m_scale.y = 1.0f / (float)KuroEngine::WinApp::Instance()->GetExpandWinSize().y;
}

void KuroEngine::DebugCamera::Init(const Vec3<float>& InitPos)
{
	m_cam->GetTransform().SetPos(InitPos);
	m_cam->GetTransform().SetRotate(XMQuaternionIdentity());
	m_posAngleTransform.SetRotate(XMQuaternionIdentity());
	m_gazePointTransform.SetPos(InitPos + KuroEngine::Vec3<float>(0.0f, 0.0f, 3.0f));
}

void KuroEngine::DebugCamera::Move()
{
	bool moveDirty = false;
	float angleX = 0;
	float angleY = 0;

	//マウスの入力を取得
	KuroEngine::UsersInput::MouseMove mouseMove = KuroEngine::UsersInput::Instance()->GetMouseMove();

	//マウス左クリックでカメラ回転
	if (KuroEngine::UsersInput::Instance()->MouseInput(MOUSE_BUTTON::RIGHT))
	{
		float dx = mouseMove.m_inputY * m_scale.x;
		float dy = mouseMove.m_inputX * m_scale.y;

		angleX = -dx * XM_PI * 0.65f;
		angleY = -dy * XM_PI * 0.65f;

		auto rotate = m_posAngleTransform.GetRotate();
		rotate = XMQuaternionMultiply(XMQuaternionRotationRollPitchYaw(-angleX, -angleY, 0.0f), rotate);
		m_posAngleTransform.SetRotate(rotate);
	}

	//マウス中クリックでカメラ平行移動
	if (KuroEngine::UsersInput::Instance()->MouseInput(MOUSE_BUTTON::CENTER))
	{
		float dx = mouseMove.m_inputX / 100.0f;
		float dy = mouseMove.m_inputY / 100.0f;

		auto move = Math::TransformVec3({ -dx,+dy,0 }, m_cam->GetTransform().GetRotateWorld());

		m_cam->GetTransform().SetPos(m_cam->GetTransform().GetPos() + move);
		m_gazePointTransform.SetPos(m_gazePointTransform.GetPos() + move);

		moveDirty = true;
	}

	//ホイール入力で距離を変更
	if (mouseMove.m_inputZ != 0)
	{
		float inputZ = mouseMove.m_inputZ / 50.0f;
		auto pos = m_cam->GetTransform().GetPos();
		pos.z += inputZ;
		pos.z = std::min(pos.z, -1.0f);
		m_cam->GetTransform().SetPos(pos);
	}
}
