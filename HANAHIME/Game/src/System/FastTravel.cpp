#include "FastTravel.h"
#include "../OperationConfig.h"
#include"ForUser/DrawFunc/2D/DrawFunc2D.h"
#include"../Stage/StageManager.h"
#include"../SoundConfig.h"

void FastTravel::GetTargetPosAndRotate(KuroEngine::Quaternion* arg_resultRotate, KuroEngine::Vec3<float>* arg_resultPos)
{
	using namespace KuroEngine;

	//カメラの座標を決定。
	const float CAMERA_Y = 50.0f;
	const float CAMERA_DISTANCE = 100.0f;
	Vec3<float> baseCameraPos = m_checkPointVector[m_nowStageNum][m_nowTargetCheckPoint].GetPos() + m_rotate.GetFront() * CAMERA_DISTANCE + Vec3<float>(0, CAMERA_Y, 0);

	//カメラの回転を取得。
	Vec3<float> axisZ = Vec3<float>(m_checkPointVector[m_nowStageNum][m_nowTargetCheckPoint].GetPos() - baseCameraPos).GetNormal();
	Vec3<float> axisY = Vec3<float>(0, 1, 0);
	Vec3<float> axisX = axisY.Cross(axisZ);
	axisY = axisZ.Cross(axisX);

	//姿勢を得る。
	DirectX::XMMATRIX matWorld = DirectX::XMMatrixIdentity();
	matWorld.r[0] = { axisX.x, axisX.y, axisX.z, 0.0f };
	matWorld.r[1] = { axisY.x, axisY.y, axisY.z, 0.0f };
	matWorld.r[2] = { axisZ.x, axisZ.y, axisZ.z, 0.0f };
	XMVECTOR rotate, scale, position;
	DirectX::XMMatrixDecompose(&scale, &rotate, &position, matWorld);

	if (arg_resultRotate)*arg_resultRotate = rotate;
	if (arg_resultPos)*arg_resultPos = baseCameraPos;
}

FastTravel::FastTravel()
{
	const std::string STAGE_NAME_DIR = "resource/user/tex/stage/fast_travel/";
	int stageIdx = 0;
	std::string path = STAGE_NAME_DIR + std::to_string(stageIdx) + ".png";
	while (KuroEngine::ExistFile(path))
	{
		m_stageNameTexArray.emplace_back(KuroEngine::D3D12App::Instance()->GenerateTextureBuffer(path));
		path = STAGE_NAME_DIR + std::to_string(++stageIdx) + ".png";
	}

	m_fastTravelCamera = std::make_shared<KuroEngine::Camera>("FastTravelCamera");
	m_underLineTex = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/stage/under_line.png");

	KuroEngine::D3D12App::Instance()->GenerateTextureBuffer(m_numMainTexArray.data(), "resource/user/tex/stage_select/stage_num_main.png", 10, { 10,1 });

}

void FastTravel::Init(std::vector<std::vector<KuroEngine::Transform>>arg_checkPointVector, int arg_selectStageNum, int arg_selectTransIdx)
{
	//チェックポイントを保存。
	m_checkPointVector = arg_checkPointVector;
	m_nowStageNum = arg_selectStageNum;
	m_nowTargetCheckPoint = arg_selectTransIdx;
	m_rotate.SetRotate(DirectX::XMQuaternionRotationAxis(KuroEngine::Vec3<float>(0, 0, 1), DirectX::XM_PIDIV2));

	//カメラの座標と回転を設定。
	KuroEngine::Quaternion camRotate;
	GetTargetPosAndRotate(&camRotate, &m_cameraPos);
	m_fastTravelCamera->GetTransform().SetPos(m_cameraPos);
	m_fastTravelCamera->GetTransform().SetRotate(camRotate);

	m_beforeStageNum = StageManager::Instance()->GetNowStageIdx();
	StageManager::Instance()->SetStage(arg_selectStageNum);
}

void FastTravel::Update()
{
	using namespace KuroEngine;

	if (!m_isActive)return;

	//要素が1個も存在していなかったら処理を飛ばす。
	if (static_cast<int>(m_checkPointVector.size()) <= 0) return;

	//入力
	bool leftInput = OperationConfig::Instance()->GetSelectVec(OperationConfig::SELECT_VEC_LEFT);
	bool rightInput = OperationConfig::Instance()->GetSelectVec(OperationConfig::SELECT_VEC_RIGHT);
	bool cancelInput = OperationConfig::Instance()->GetOperationInput(OperationConfig::CANCEL, OperationConfig::ON_TRIGGER)
		|| OperationConfig::Instance()->GetOperationInput(OperationConfig::PAUSE, OperationConfig::ON_TRIGGER);
	bool doneInput = OperationConfig::Instance()->GetOperationInput(OperationConfig::DONE, OperationConfig::ON_TRIGGER);

	//変化前のステージ番号を記録しておく
	const int oldStageNum = m_nowStageNum;
	//変化前のチェックポイント番号を記録しておく
	const int oldPointNum = m_nowTargetCheckPoint;

	//選択する。
	int selectMin = 0;
	int selectMax = static_cast<int>(m_checkPointVector[m_nowStageNum].size()) - 1;
	if (rightInput) {
		++m_nowTargetCheckPoint;
		if (selectMax < m_nowTargetCheckPoint) {
			//次のステージへ
			if (m_nowStageNum < static_cast<int>(m_checkPointVector.size() - 1)) {
				++m_nowStageNum;
				m_nowTargetCheckPoint = 0;
			}
			//最後まで到達
			else {
				m_nowTargetCheckPoint = selectMax;
			}
		}
	}
	if (leftInput) {
		--m_nowTargetCheckPoint;
		if (m_nowTargetCheckPoint < selectMin) {
			//前のステージへ
			if (0 < m_nowStageNum) {
				--m_nowStageNum;
				m_nowTargetCheckPoint = static_cast<int>(m_checkPointVector[m_nowStageNum].size()) - 1;
			}
			//最初まで到達
			else {
				m_nowTargetCheckPoint = selectMin;
			}
		}
	}

	//ステージ変化
	if (oldStageNum != m_nowStageNum)StageManager::Instance()->SetStage(m_nowStageNum);

	//選択SE
	if (leftInput || rightInput)
	{
		SoundConfig::Instance()->Play((oldStageNum != m_nowStageNum || oldPointNum != m_nowTargetCheckPoint) ? SoundConfig::SE_SELECT : SoundConfig::SE_CANCEL);
	}

	//閉じる
	if (cancelInput)
	{
		m_isActive = false;
		SoundConfig::Instance()->Play(SoundConfig::SE_CANCEL);
		//元のステージに戻しておく
		StageManager::Instance()->SetStage(m_beforeStageNum);
	}

	//決定
	if (doneInput)
	{
		SoundConfig::Instance()->Play(SoundConfig::SE_DONE);
	}

	//回転させる。
	m_rotate.SetRotate(DirectX::XMQuaternionMultiply(m_rotate.GetRotate(), DirectX::XMQuaternionRotationAxis(Vec3<float>(0, 1, 0), ADD_XZANGLE)));

	//カメラの座標と回転を決定。
	Vec3<float> baseCameraPos;
	XMVECTOR rotate;
	GetTargetPosAndRotate(&rotate, &baseCameraPos);

	//カメラを補間。
	m_cameraPos = KuroEngine::Math::Lerp(m_cameraPos, baseCameraPos, 0.15f);

	//カメラの座標と回転を設定。
	m_fastTravelCamera->GetTransform().SetPos(m_cameraPos);
	m_fastTravelCamera->GetTransform().SetRotate(DirectX::XMQuaternionSlerp(m_fastTravelCamera->GetTransform().GetRotate(), rotate, 0.08f));
}

void FastTravel::Draw(KuroEngine::Camera& arg_cam)
{
	if (!m_isActive)return;

	static KuroEngine::Vec2<float> stageNameCenterPos(435,527);
	static KuroEngine::Vec2<float> underLinePos(524, 611);
	static KuroEngine::Vec2<float> fontPos(771, 541);

	//数字のフォント関連。
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

	KuroEngine::DrawFunc2D::DrawRotaGraph2D(stageNameCenterPos, KuroEngine::Vec2<float>(1.0f, 1.0f), 0.0f, m_stageNameTexArray[m_nowStageNum]);
	KuroEngine::DrawFunc2D::DrawRotaGraph2D(underLinePos, KuroEngine::Vec2<float>(2.0f, 2.0f), 0.0f, m_underLineTex);

	//ステージ数の桁数を取得。
	int nowTargetCheckPoinit = m_nowTargetCheckPoint + 1;
	int stageNumDisit = static_cast<int>(std::to_string(nowTargetCheckPoinit).size());

	//桁数分描画
	//for (int index = 0; index < stageNumDisit; ++index) {

	//	KuroEngine::DrawFunc2D::DrawRotaGraph2D(fontPos + KuroEngine::Vec2<float>(FONT_SIZE * index, 0.0f), KuroEngine::Vec2<float>(FONT_SCALE, FONT_SCALE), 0.0f, m_numMainTexArray[GetDisit(nowTargetCheckPoinit, index)]);
	//}
	KuroEngine::DrawFunc2D::DrawNumber2D(m_nowTargetCheckPoint + 1, fontPos, m_numMainTexArray.data(), 
		{ FONT_SCALE ,FONT_SCALE }, 0.0f, KuroEngine::HORIZONTAL_ALIGN::LEFT, KuroEngine::VERTICAL_ALIGN::CENTER);

}
