#include "FastTravel.h"
#include "../OperationConfig.h"
#include"ForUser/DrawFunc/2D/DrawFunc2D.h"

FastTravel::FastTravel()
{

	m_fastTravelCamera = std::make_shared<KuroEngine::Camera>("FastTravelCamera");
	m_stageNameTex = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/stage/defaultUpScale.png");
	m_underLineTex = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/stage/under_line.png");

	KuroEngine::D3D12App::Instance()->GenerateTextureBuffer(m_numMainTexArray.data(), "resource/user/tex/stage_select/stage_num_main.png", 10, { 10,1 });

}

void FastTravel::Init(std::vector<KuroEngine::Transform> arg_checkPointVector)
{

	//�`�F�b�N�|�C���g��ۑ��B
	m_checkPointVector = arg_checkPointVector;
	m_nowTargetCheckPoint = 0;
	m_rotate.SetRotate(DirectX::XMQuaternionRotationAxis(KuroEngine::Vec3<float>(0, 0, 1), DirectX::XM_PIDIV2));

}

void FastTravel::Update()
{

	using namespace KuroEngine;

	//�v�f��1�����݂��Ă��Ȃ������珈�����΂��B
	if (static_cast<int>(m_checkPointVector.size()) <= 0) return;

	//�I������B
	int selectMin = 0;
	int selectMax = static_cast<int>(m_checkPointVector.size()) - 1;
	if (OperationConfig::Instance()->GetSelectVec(OperationConfig::SELECT_VEC_RIGHT)) {
		++m_nowTargetCheckPoint;
		if (selectMax < m_nowTargetCheckPoint) {
			m_nowTargetCheckPoint = selectMax;
		}
	}
	if (OperationConfig::Instance()->GetSelectVec(OperationConfig::SELECT_VEC_LEFT)) {
		--m_nowTargetCheckPoint;
		if (m_nowTargetCheckPoint < selectMin) {
			m_nowTargetCheckPoint = selectMin;
		}
	}

	//��]������B
	m_rotate.SetRotate(DirectX::XMQuaternionMultiply(m_rotate.GetRotate(), DirectX::XMQuaternionRotationAxis(Vec3<float>(0, 1, 0), ADD_XZANGLE)));

	//�J�����̍��W������B
	const float CAMERA_Y = 50.0f;
	const float CAMERA_DISTANCE = 100.0f;
	Vec3<float> baseCameraPos = m_checkPointVector[m_nowTargetCheckPoint].GetPos() + m_rotate.GetFront() * CAMERA_DISTANCE + Vec3<float>(0, CAMERA_Y, 0);

	//�J�������ԁB
	m_cameraPos = KuroEngine::Math::Lerp(m_cameraPos, baseCameraPos, 0.15f);

	//�J�����̉�]���擾�B
	Vec3<float> axisZ = Vec3<float>(m_checkPointVector[m_nowTargetCheckPoint].GetPos() - baseCameraPos).GetNormal();
	Vec3<float> axisY = Vec3<float>(0, 1, 0);
	Vec3<float> axisX = axisY.Cross(axisZ);
	axisY = axisZ.Cross(axisX);

	//�p���𓾂�B
	DirectX::XMMATRIX matWorld = DirectX::XMMatrixIdentity();
	matWorld.r[0] = { axisX.x, axisX.y, axisX.z, 0.0f };
	matWorld.r[1] = { axisY.x, axisY.y, axisY.z, 0.0f };
	matWorld.r[2] = { axisZ.x, axisZ.y, axisZ.z, 0.0f };
	XMVECTOR rotate, scale, position;
	DirectX::XMMatrixDecompose(&scale, &rotate, &position, matWorld);

	//�J�����̍��W�Ɖ�]��ݒ�B
	m_fastTravelCamera->GetTransform().SetPos(m_cameraPos);
	m_fastTravelCamera->GetTransform().SetRotate(DirectX::XMQuaternionSlerp(m_fastTravelCamera->GetTransform().GetRotate(), rotate, 0.08f));

}

void FastTravel::Draw(KuroEngine::Camera& arg_cam)
{

	static KuroEngine::Vec2<float> stageNameCenterPos(435,527);
	static KuroEngine::Vec2<float> underLinePos(524, 611);
	static KuroEngine::Vec2<float> fontPos(831, 541);

	//�����̃t�H���g�֘A�B
	static float FONT_SIZE = 64.0f;

	static float FONT_SCALE = 0.68f;

	//ImageAdjust::Adjust(&fontPos, 1.0f);

	//fontPos.y += 1.0f * KuroEngine::UsersInput::Instance()->KeyInput(DIK_DOWN);
	//fontPos.y -= 1.0f * KuroEngine::UsersInput::Instance()->KeyInput(DIK_UP);
	//fontPos.x += 1.0f * KuroEngine::UsersInput::Instance()->KeyInput(DIK_RIGHT);
	//fontPos.x -= 1.0f * KuroEngine::UsersInput::Instance()->KeyInput(DIK_LEFT);

	//FONT_SIZE += 1.0f * KuroEngine::UsersInput::Instance()->KeyInput(DIK_K);
	//FONT_SIZE -= 1.0f * KuroEngine::UsersInput::Instance()->KeyInput(DIK_L);

	//FONT_SCALE += 0.01f * KuroEngine::UsersInput::Instance()->KeyInput(DIK_H);
	//FONT_SCALE -= 0.01f * KuroEngine::UsersInput::Instance()->KeyInput(DIK_J);

	KuroEngine::DrawFunc2D::DrawRotaGraph2D(stageNameCenterPos, KuroEngine::Vec2<float>(1.0f, 1.0f), 0.0f, m_stageNameTex);
	KuroEngine::DrawFunc2D::DrawRotaGraph2D(underLinePos, KuroEngine::Vec2<float>(2.0f, 2.0f), 0.0f, m_underLineTex);

	//�X�e�[�W���̌������擾�B
	int nowTargetCheckPoinit = m_nowTargetCheckPoint + 1;
	int stageNumDisit = static_cast<int>(std::to_string(nowTargetCheckPoinit).size());

	//�������`��
	for (int index = 0; index < stageNumDisit; ++index) {

		KuroEngine::DrawFunc2D::DrawRotaGraph2D(fontPos + KuroEngine::Vec2<float>(FONT_SIZE * index, 0.0f), KuroEngine::Vec2<float>(FONT_SCALE, FONT_SCALE), 0.0f, m_numMainTexArray[GetDisit(nowTargetCheckPoinit, index)]);

	}

}
