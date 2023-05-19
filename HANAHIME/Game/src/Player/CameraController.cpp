#include "CameraController.h"
#include"Render/RenderObject/Camera.h"
#include"../../../../src/engine/ForUser/Object/Model.h"
#include"CollisionDetectionOfRayAndMesh.h"
#include"FrameWork/UsersInput.h"
#include"../Stage/StageManager.h"

void CameraController::OnImguiItems()
{
	ImGui::Text("NowParameter");
	ImGui::SameLine();

	//パラメータ初期化ボタン
	if (ImGui::Button("Initialize"))
	{
		m_nowParam = m_initializedParam;
	}

	//現在のパラメータ表示
	if (ImGui::BeginChild("NowParam"))
	{
		ImGui::Text("posOffsetZ : %.2f", m_nowParam.m_posOffsetZ);
		float degree = static_cast<float>(KuroEngine::Angle::ConvertToDegree(m_nowParam.m_xAxisAngle));
		ImGui::Text("xAxisAngle : %.2f", degree);
		degree = static_cast<float>(KuroEngine::Angle::ConvertToDegree(m_nowParam.m_yAxisAngle));
		ImGui::Text("yAxisAngle : %.2f", degree);
		ImGui::EndChild();
	}

}

CameraController::CameraController()
	:KuroEngine::Debugger("CameraController", true, true)
{
	AddCustomParameter("posOffsetZ", { "InitializedParameter","posOffsetZ" }, PARAM_TYPE::FLOAT, &m_initializedParam.m_posOffsetZ, "InitializedParameter");
	AddCustomParameter("xAxisAngle", { "InitializedParameter","xAxisAngle" }, PARAM_TYPE::FLOAT, &m_initializedParam.m_xAxisAngle, "InitializedParameter");

	AddCustomParameter("gazePointOffset", { "gazePointOffset" }, PARAM_TYPE::FLOAT_VEC3, &m_gazePointOffset, "UpdateParameter");
	AddCustomParameter("posOffsetDepthMin", { "posOffsetDepth","min" }, PARAM_TYPE::FLOAT, &m_posOffsetDepthMin, "UpdateParameter");
	AddCustomParameter("posOffsetDepthMax", { "posOffsetDepth","max" }, PARAM_TYPE::FLOAT, &m_posOffsetDepthMax, "UpdateParameter");
	//AddCustomParameter("xAxisAngleMin", { "xAxisAngle","min" }, PARAM_TYPE::FLOAT, &m_xAxisAngleMin, "UpdateParameter");
	//AddCustomParameter("xAxisAngleMax", { "xAxisAngle","max" }, PARAM_TYPE::FLOAT, &m_xAxisAngleMax, "UpdateParameter");
	AddCustomParameter("camFowardPosLerpRate", { "PosLerpRate" }, PARAM_TYPE::FLOAT, &m_camForwardPosLerpRate, "UpdateParameter");
	AddCustomParameter("camFollowLerpRate", { "FollowLerpRate" }, PARAM_TYPE::FLOAT, &m_camFollowLerpRate, "UpdateParameter");

	LoadParameterLog();
}

void CameraController::AttachCamera(std::shared_ptr<KuroEngine::Camera> arg_cam)
{
	//操作対象となるカメラのポインタを保持
	m_attachedCam = arg_cam;
	//コントローラーのトランスフォームを親として設定
	m_cameraLocalTransform.SetParent(&m_camParentTransform);
}

void CameraController::Init()
{
	m_nowParam = m_initializedParam;
	m_verticalControl = ANGLE;
	m_rotateZ = 0;
	m_rotateYLerpAmount = 0;
	m_cameraXAngleLerpAmount = 0;
	m_isHitUnderGroundTerrian = false;
	m_playerOldPos = KuroEngine::Vec3<float>();
}

void CameraController::Update(KuroEngine::Vec3<float>arg_scopeMove, KuroEngine::Transform arg_targetPos, float& arg_playerRotY, float arg_cameraZ, const std::weak_ptr<Stage>arg_nowStage, bool arg_isCameraUpInverse, bool arg_isCameraDefaultPos, bool& arg_isHitUnderGround, bool arg_isMovePlayer, bool arg_isPlayerJump, KuroEngine::Quaternion arg_cameraQ)
{
	using namespace KuroEngine;

	//カメラがアタッチされていない
	if (m_attachedCam.expired())return;

	//トランスフォームを保存。
	m_oldCameraWorldPos = m_attachedCam.lock()->GetTransform().GetPos();

	//回転を適用する前のX回転
	float fromXAngle = m_nowParam.m_xAxisAngle;

	//横の壁に居るときの注視点移動の場合、Y軸回転を動かす。
	if (m_isHitUnderGroundTerrian && fabs(arg_targetPos.GetUp().y) < 0.9f) {

		//壁にあたっていないときに保存した回転角と現在の回転角の差分を求めて、規定値以上動かないようにする。
		float subAngleY = arg_playerRotY - m_playerRotYStorage;

		//既定値を超えていたら。
		if (PLAYER_TARGET_MOVE_SIDE < fabs(subAngleY)) {

			//回転量を押し戻す。
			arg_playerRotY = m_playerRotYStorage + (signbit(subAngleY) ? -1.0f : 1.0f) * PLAYER_TARGET_MOVE_SIDE;

		}

	}

	//カメラが地上に当たっているときに感度を下げる。
	if (m_isHitUnderGroundTerrian) {
		arg_scopeMove *= 0.8f;
	}

	//左右カメラ操作
	m_nowParam.m_yAxisAngle = arg_playerRotY;

	//上下カメラ操作
	switch (m_verticalControl)
	{
	case ANGLE:
		m_nowParam.m_xAxisAngle -= arg_scopeMove.y * 0.3f;
		//if (m_nowParam.m_xAxisAngle <= m_xAxisAngleMin)m_verticalControl = DIST;
		break;

	case DIST:
		m_nowParam.m_posOffsetZ += arg_scopeMove.y * 6.0f;
		if (m_nowParam.m_posOffsetZ <= m_posOffsetDepthMin)m_verticalControl = ANGLE;
		break;
	}

	//カメラが地上にあたっていて、その当たっている面が横の壁だったら回転を保存しておく。
	if (!m_isHitUnderGroundTerrian) {
		m_playerRotYStorage = arg_playerRotY;
	}

	//上限値超えないようにする
	m_nowParam.m_posOffsetZ = arg_cameraZ;
	m_nowParam.m_xAxisAngle = std::clamp(m_nowParam.m_xAxisAngle, m_xAxisAngleMin, m_xAxisAngleMax);

	//床にカメラが当たっているときにプレイヤーが動いていたら、床がカメラに当たらないようにX軸回転を元に戻す。
	if ((m_isHitUnderGroundTerrian && arg_isMovePlayer) || (m_isHitUnderGroundTerrian && arg_isCameraDefaultPos)) {

		//横の壁にいたら 
		if (fabs(arg_targetPos.GetUp().y) < 0.9f && fabs(m_playerRotYLerp) < 0.1f) {

			//Y軸回転を押し戻す量。
			float rotYScale = (arg_playerRotY - m_playerRotYStorage);
			//Y軸回転をぴったり押し戻すと地面スレスレになってしまうので、多少オフセットを設けることで良い感じの高さにする。
			const float ROTY_OFFSET = 0.8f;
			float rotYOffset = (std::signbit(rotYScale) ? -1.0f : 1.0f) * ROTY_OFFSET + rotYScale;
			m_playerRotYLerp -= rotYOffset;

		}
		//上下の壁にいたら
		else if (0.9f < fabs(arg_targetPos.GetUp().y)) {

			//カメラが反転しているかしていないかによって入れる値を決める。
			if (arg_isCameraUpInverse) {
				m_cameraXAngleLerpAmount = m_xAxisAngleMin;
			}
			else {
				m_cameraXAngleLerpAmount = m_xAxisAngleMax;
			}

		}

	}

	//マップピンの座標の受け皿
	KuroEngine::Vec3<float>mapPinPos;
	//カメラを初期位置に戻すか。
	if (arg_isCameraDefaultPos && !m_isHitUnderGroundTerrian && StageManager::Instance()->GetNowMapPinPos(&mapPinPos)) {

		//カメラの正面ベクトル
		KuroEngine::Vec3<float> cameraDir = (arg_targetPos.GetPosWorld() - m_attachedCam.lock()->GetTransform().GetPos()).GetNormal();
		//目標地点 いい感じの視点にするために、注視点を少し下にずらす。
		KuroEngine::Vec3<float> targetPos = mapPinPos;

		//目標地点までのベクトル
		KuroEngine::Vec3<float> targetDir = (targetPos - m_attachedCam.lock()->GetTransform().GetPos()).GetNormal();

		//各ベクトル間の法線を求める。法線が存在しなかったら補間する必要はない。
		KuroEngine::Vec3<float> upVec = cameraDir.Cross(targetDir);
		if (0 < upVec.Length()) {

			//カメラ基準のプレイヤーの姿勢を求める。
			KuroEngine::Vec3<float> upVec(0, 1, 0);

			//プレイヤーの法線との外積からXベクトルを得る。
			Vec3<float> axisX = upVec.Cross(cameraDir);

			//プレイヤーのZ座標
			Vec3<float> axisZ = axisX.Cross(upVec);

			//まずはY軸回転を求める。
			Vec2<float> cameraDir2DY = Project3Dto2D(cameraDir, axisX, axisZ);
			cameraDir2DY.Normalize();
			Vec2<float> targetDir2DY = Project3Dto2D(targetDir, axisX, axisZ);
			targetDir2DY.Normalize();

			//回転量を求める。
			float angle = acos(cameraDir2DY.Dot(targetDir2DY));
			float cross = cameraDir2DY.Cross(targetDir2DY);
			m_rotateYLerpAmount = angle * (cross < 0 ? 1.0f : -1.0f);

			////次にX軸軸回転を求める。
			//Vec2<float> cameraDir2DX = Project3Dto2D(cameraDir, axisX, axisZ);
			//cameraDir2DX.Normalize();
			//Vec2<float> targetDir2DX = Project3Dto2D(targetDir, axisX, axisZ);
			//targetDir2DX.Normalize();

			////回転量を求める。
			//angle = acos(cameraDir2DX.Dot(targetDir2DX)) * 1.0f;
			//cross = cameraDir2DX.Cross(targetDir2DX);
			//m_cameraXAngleLerpAmount = angle * (cross < 0 ? -1.0f : 1.0f);

		}

	}

	//Y軸補間量があったらいい感じに補間する。
	if (0 < fabs(m_playerRotYLerp)) {
		float lerp = m_playerRotYLerp - KuroEngine::Math::Lerp(m_playerRotYLerp, 0.0f, 0.08f);
		m_playerRotYLerp -= lerp;
		arg_playerRotY += lerp;
		m_nowParam.m_yAxisAngle = arg_playerRotY;
	}

	//カメラを初期位置に戻す量が0以上だったらカメラの回転量を補間。
	if (0 < fabs(m_rotateYLerpAmount)) {
		float lerp = m_rotateYLerpAmount - KuroEngine::Math::Lerp(m_rotateYLerpAmount, 0.0f, 0.08f);
		m_rotateYLerpAmount -= lerp;
		arg_playerRotY += lerp;
		m_nowParam.m_yAxisAngle = arg_playerRotY;
	}
	if (0 < fabs(m_cameraXAngleLerpAmount)) {
		float lerp = m_cameraXAngleLerpAmount - KuroEngine::Math::Lerp(m_cameraXAngleLerpAmount, 0.0f, 0.08f);
		m_cameraXAngleLerpAmount = (fabs(m_cameraXAngleLerpAmount) - fabs(lerp)) * (signbit(m_cameraXAngleLerpAmount) ? -1.0f : 1.0f);
		m_nowParam.m_xAxisAngle += lerp;
	}

	//操作するカメラのトランスフォーム（前後移動）更新
	Vec3<float> localPos = { 0,0,0 };
	localPos.z = m_nowParam.m_posOffsetZ;
	localPos.y = m_gazePointOffset.y + tan(-m_nowParam.m_xAxisAngle) * m_nowParam.m_posOffsetZ;
	m_cameraLocalTransform.SetPos(Math::Lerp(m_cameraLocalTransform.GetPos(), localPos, m_camForwardPosLerpRate));
	m_cameraLocalTransform.SetRotate(Vec3<float>::GetXAxis(), m_nowParam.m_xAxisAngle);

	//コントローラーのトランスフォーム（対象の周囲、左右移動）更新
	m_camParentTransform.SetRotate(Vec3<float>::GetYAxis(), m_nowParam.m_yAxisAngle);
	m_camParentTransform.SetPos(Math::Lerp(m_camParentTransform.GetPos(), arg_targetPos.GetPosWorld(), m_camFollowLerpRate));


	//使用するカメラの座標を補間して適用。
	Vec3<float> pushBackPos = m_cameraLocalTransform.GetPosWorldByMatrix();

	//当たり判定用のレイを打つ方向を決める。
	Vec3<float> checkHitRay = m_cameraLocalTransform.GetPosWorldByMatrix() - m_oldCameraWorldPos;	//まずはデフォルトのレイに設定。
	Vec3<float> playerDir = arg_targetPos.GetPos() - m_cameraLocalTransform.GetPosWorldByMatrix();

	//カメラの押し戻し判定に使用する姿勢を取得。
	Vec3<float> cameraAxisZ = playerDir.GetNormal();
	Vec3<float> cameraAxisY = Vec3<float>(0, 1, 0);
	Vec3<float> cameraAxisX = cameraAxisY.Cross(cameraAxisZ);
	cameraAxisZ = cameraAxisY.Cross(cameraAxisX);
	DirectX::XMMATRIX cameraMatWorld = DirectX::XMMatrixIdentity();
	cameraMatWorld.r[0] = { cameraAxisX.x, cameraAxisX.y, cameraAxisX.z, 0.0f };
	cameraMatWorld.r[1] = { cameraAxisY.x, cameraAxisY.y, cameraAxisY.z, 0.0f };
	cameraMatWorld.r[2] = { cameraAxisZ.x, cameraAxisZ.y, cameraAxisZ.z, 0.0f };
	XMVECTOR rotate, scale, position;
	DirectX::XMMatrixDecompose(&scale, &rotate, &position, cameraMatWorld);
	//カメラのトランスフォーム
	KuroEngine::Transform cameraT = arg_targetPos;
	cameraT.SetRotate(rotate);

	m_debugTransform = cameraT;

	//通常の地形を走査
	m_isHitTerrian = false;
	m_isHitUnderGroundTerrian = false;
	auto& cameraTransform = m_attachedCam.lock()->GetTransform();
	for (auto& terrian : arg_nowStage.lock()->GetTerrianArray())
	{
		//モデル情報取得
		auto model = terrian.GetModel().lock();

		//メッシュを走査
		for (auto& modelMesh : model->m_meshes)
		{

			//当たり判定に使用するメッシュ
			auto checkHitMesh = terrian.GetCollisionMesh()[static_cast<int>(&modelMesh - &model->m_meshes[0])];

			//判定↓============================================


			//純粋な地形とレイの当たり判定を実行
			CollisionDetectionOfRayAndMesh::MeshCollisionOutput output = CollisionDetectionOfRayAndMesh::Instance()->MeshCollision(m_oldCameraWorldPos, checkHitRay.GetNormal(), checkHitMesh);
			if (output.m_isHit && 0 < output.m_distance && output.m_distance < checkHitRay.Length()) {

				pushBackPos = output.m_pos + output.m_normal;
				m_isHitTerrian = true;

				PushBackGround(output, pushBackPos, arg_targetPos, arg_playerRotY, arg_isCameraUpInverse);

			}

			//プレイヤーの右側にレイを飛ばす。
			Vec3<float> playerDir = (cameraT.GetPos()) + (cameraT.GetRight() * 1.0f) - (pushBackPos);
			output = CollisionDetectionOfRayAndMesh::Instance()->MeshCollision(pushBackPos, playerDir.GetNormal(), checkHitMesh);
			if (output.m_isHit && 0 < output.m_distance && output.m_distance < playerDir.Length()) {

				//プレイヤーが乗っている面だったら無効化。
				bool onPlayer = 0.0f < output.m_normal.Dot(arg_targetPos.GetUp());

				if (!onPlayer) {

					//角度を押し戻す。
					m_nowParam.m_yAxisAngle += 0.1f;
					arg_playerRotY += 0.1f;

				}

			}

			//プレイヤーの左側にレイを飛ばす。
			playerDir = (cameraT.GetPos()) - (cameraT.GetRight() * 1.0f) - (pushBackPos);
			output = CollisionDetectionOfRayAndMesh::Instance()->MeshCollision(pushBackPos, playerDir.GetNormal(), checkHitMesh);
			if (output.m_isHit && 0 < output.m_distance && output.m_distance < playerDir.Length()) {

				//プレイヤーが乗っている面だったら無効化。
				bool onPlayer = 0.0f < output.m_normal.Dot(arg_targetPos.GetUp());

				if (!onPlayer) {

					m_nowParam.m_yAxisAngle -= 0.1f;
					arg_playerRotY -= 0.1f;

				}

			}

			//=================================================
		}
	}

	//地上にあたっていたら地形と押し戻す前の座標からの回転を求めることで、注視点を上に向ける。
	if (m_isHitUnderGroundTerrian) {

		//カメラまでのベクトル。
		KuroEngine::Vec3<float> cameraDir = (m_attachedCam.lock()->GetTransform().GetPos() - arg_targetPos.GetPosWorld()).GetNormal();

		//カメラが動いた量回転させる。

		//下の面にいる場合
		if (0.9f < arg_targetPos.GetUp().y) {
			cameraDir = KuroEngine::Math::TransformVec3(cameraDir, DirectX::XMQuaternionRotationAxis(arg_targetPos.GetUp(), arg_scopeMove.x));
		}
		//上の面にいる場合
		else if (arg_targetPos.GetUp().y < -0.9f) {
			cameraDir = KuroEngine::Math::TransformVec3(cameraDir, DirectX::XMQuaternionRotationAxis(arg_targetPos.GetUp(), -arg_scopeMove.x));
		}
		//横の面にいる場合
		else if (0.9f < arg_targetPos.GetUp().y) {
			cameraDir = KuroEngine::Math::TransformVec3(cameraDir, DirectX::XMQuaternionRotationAxis(arg_targetPos.GetUp(), arg_scopeMove.y));
		}

		//座標を動かす。
		m_attachedCam.lock()->GetTransform().SetPos(arg_targetPos.GetPosWorld() + cameraDir * fabs(arg_cameraZ));

		//現在の座標からプレイヤーに向かう回転を求める。
		Vec3<float> axisZ = arg_targetPos.GetPos() - m_cameraLocalTransform.GetPosWorldByMatrix();
		axisZ.Normalize();

		//プレイヤーの法線との外積から仮のXベクトルを得る。
		Vec3<float> axisX = Vec3<float>(0, 1, 0).Cross(axisZ);

		//Xベクトルから上ベクトルを得る。
		Vec3<float> axisY = axisZ.Cross(axisX);

		//姿勢を得る。
		DirectX::XMMATRIX matWorld = DirectX::XMMatrixIdentity();
		matWorld.r[0] = { axisX.x, axisX.y, axisX.z, 0.0f };
		matWorld.r[1] = { axisY.x, axisY.y, axisY.z, 0.0f };
		matWorld.r[2] = { axisZ.x, axisZ.y, axisZ.z, 0.0f };

		XMVECTOR rotate, scale, position;
		DirectX::XMMatrixDecompose(&scale, &rotate, &position, matWorld);

		//回転を反転させる。
		if (arg_isCameraUpInverse) {
			m_rotateZ = KuroEngine::Math::Lerp(m_rotateZ, DirectX::XM_PI, 0.08f);
		}
		else {
			m_rotateZ = KuroEngine::Math::Lerp(m_rotateZ, 0.0f, 0.1f);
		}
		rotate = DirectX::XMQuaternionMultiply(rotate, DirectX::XMQuaternionRotationAxis(axisZ, m_rotateZ));

		rotate = DirectX::XMQuaternionNormalize(rotate);

		//回転を適用。
		m_attachedCam.lock()->GetTransform().SetRotate(rotate);

	}
	else {

		//補間する。
		m_attachedCam.lock()->GetTransform().SetPos(KuroEngine::Math::Lerp(m_attachedCam.lock()->GetTransform().GetPos(), pushBackPos, 0.3f));

		//現在の座標からプレイヤーに向かう回転を求める。
		Vec3<float> axisZ = arg_targetPos.GetPos() - m_attachedCam.lock()->GetTransform().GetPosWorld();
		axisZ.Normalize();

		//プレイヤーの法線との外積から仮のXベクトルを得る。
		Vec3<float> axisX = Vec3<float>(0, 1, 0).Cross(axisZ);

		//Xベクトルから上ベクトルを得る。
		Vec3<float> axisY = axisZ.Cross(axisX);

		//姿勢を得る。
		DirectX::XMMATRIX matWorld = DirectX::XMMatrixIdentity();
		matWorld.r[0] = { axisX.x, axisX.y, axisX.z, 0.0f };
		matWorld.r[1] = { axisY.x, axisY.y, axisY.z, 0.0f };
		matWorld.r[2] = { axisZ.x, axisZ.y, axisZ.z, 0.0f };

		XMVECTOR rotate, scale, position;
		DirectX::XMMatrixDecompose(&scale, &rotate, &position, matWorld);

		//回転を反転させる。
		if (arg_isCameraUpInverse) {
			m_rotateZ = KuroEngine::Math::Lerp(m_rotateZ, DirectX::XM_PI, 0.08f);
		}
		else {
			m_rotateZ = KuroEngine::Math::Lerp(m_rotateZ, 0.0f, 0.08f);
		}
		rotate = DirectX::XMQuaternionMultiply(rotate, DirectX::XMQuaternionRotationAxis(axisZ, m_rotateZ));

		rotate = DirectX::XMQuaternionNormalize(rotate);

		//回転を適用。
		m_attachedCam.lock()->GetTransform().SetRotate(rotate);

	}

	////カメラが引っ掛かってどうしようもなくなったら地形を貫通させる。
	//float distance = KuroEngine::Vec3<float>(m_attachedCam.lock()->GetTransform().GetPos() - arg_targetPos.GetPos()).Length();
	//if (fabs(arg_cameraZ) <= distance) {
	//	m_attachedCam.lock()->GetTransform().SetPos(arg_targetPos.GetPos() + KuroEngine::Vec3<float>(m_attachedCam.lock()->GetTransform().GetPos() - arg_targetPos.GetPos()).GetNormal() * fabs(arg_cameraZ));
	//}

	//地上にあたっているフラグを保存。これがtrueだと注視点モードになるので、プレイヤーが居る面によってカメラの回転を打ち消す。
	arg_isHitUnderGround = m_isHitUnderGroundTerrian;

	m_playerOldPos = arg_targetPos.GetPos();



}


void CameraController::PushBackGround(const CollisionDetectionOfRayAndMesh::MeshCollisionOutput& arg_output, const KuroEngine::Vec3<float> arg_pushBackPos, const KuroEngine::Transform& arg_targetPos, float& arg_playerRotY, bool arg_isCameraUpInverse) {

	using namespace KuroEngine;

	//プレイヤーの法線と比べて同じだったら地上に当たった判定にする。
	float dot = arg_output.m_normal.Dot(arg_targetPos.GetUp());
	if (0.9f < dot) {

		//地上にあたっている。
		m_isHitUnderGroundTerrian = true;

	}
	//地上にいなかったら押し戻し。当たっている面によってX軸とY軸のどちらかを押し戻す。
	else {

		//当たっている壁が上下だったらだったら。
		if (0.9f < std::fabs(arg_output.m_normal.y)) {

			//左右判定を行う。
			Vec3<float> pushBackVec = KuroEngine::Vec3<float>(arg_pushBackPos - arg_targetPos.GetPos()).GetNormal();

			//プレイヤーの右ベクトルと現在のカメラベクトルを比べ、0以上だったら右。
			if (0 < arg_targetPos.GetRight().Dot(pushBackVec) && (!arg_isCameraUpInverse)) {

				//現在のX角度を求める。
				Vec2<float> nowXVec = Project3Dto2D(KuroEngine::Vec3<float>(m_cameraLocalTransform.GetPosWorldByMatrix() - arg_targetPos.GetPos()).GetNormal(), arg_targetPos.GetFront(), arg_targetPos.GetRight());
				Vec2<float> pushBackVec = Project3Dto2D(KuroEngine::Vec3<float>(arg_pushBackPos - arg_targetPos.GetPos()).GetNormal(), arg_targetPos.GetFront(), arg_targetPos.GetRight());

				//角度の差
				float nowAngle = atan2f(nowXVec.y, nowXVec.x);
				float pushBackAngle = atan2f(pushBackVec.y, pushBackVec.x);
				float divAngle = pushBackAngle - nowAngle;

				//角度を押し戻す。
				m_nowParam.m_xAxisAngle += divAngle;

			}
			else {

				//現在のX角度を求める。
				Vec2<float> nowXVec = Project3Dto2D(KuroEngine::Vec3<float>(m_cameraLocalTransform.GetPosWorldByMatrix() - arg_targetPos.GetPos()).GetNormal(), arg_targetPos.GetRight(), arg_targetPos.GetFront());
				Vec2<float> pushBackVec = Project3Dto2D(KuroEngine::Vec3<float>(arg_pushBackPos - arg_targetPos.GetPos()).GetNormal(), arg_targetPos.GetRight(), arg_targetPos.GetFront());

				//角度の差
				float nowAngle = atan2f(nowXVec.y, nowXVec.x);
				float pushBackAngle = atan2f(pushBackVec.y, pushBackVec.x);
				float divAngle = pushBackAngle - nowAngle;

				//角度を押し戻す。
				m_nowParam.m_xAxisAngle += divAngle;

			}

		}
		else {

			//左右判定を行う。
			Vec3<float> pushBackVec = KuroEngine::Vec3<float>(arg_pushBackPos - arg_targetPos.GetPos()).GetNormal();

			//プレイヤーの右ベクトルと現在のカメラベクトルを比べ、0以上だったら右。
			float dot = arg_targetPos.GetRight().Dot(pushBackVec);
			if (arg_isCameraUpInverse ? (dot < 0) : (0 < dot)) {

				//現在のY角度を求める。
				Vec2<float> nowYVec = Project3Dto2D(KuroEngine::Vec3<float>(m_cameraLocalTransform.GetPosWorldByMatrix() - arg_targetPos.GetPos()).GetNormal(), arg_targetPos.GetFront(), arg_targetPos.GetUp());
				Vec2<float> pushBackVec = Project3Dto2D(KuroEngine::Vec3<float>(arg_pushBackPos - arg_targetPos.GetPos()).GetNormal(), arg_targetPos.GetFront(), arg_targetPos.GetUp());


				//角度の差
				float nowAngle = atan2f(nowYVec.y, nowYVec.x);
				float pushBackAngle = atan2f(pushBackVec.y, pushBackVec.x);
				float divAngle = pushBackAngle - nowAngle;

				//角度を押し戻す。
				m_nowParam.m_yAxisAngle += divAngle;
				arg_playerRotY += divAngle;

			}
			else {

				//現在のY角度を求める。
				Vec2<float> nowYVec = Project3Dto2D(KuroEngine::Vec3<float>(m_cameraLocalTransform.GetPosWorldByMatrix() - arg_targetPos.GetPos()).GetNormal(), arg_targetPos.GetUp(), arg_targetPos.GetFront());
				Vec2<float> pushBackVec = Project3Dto2D(KuroEngine::Vec3<float>(arg_pushBackPos - arg_targetPos.GetPos()).GetNormal(), arg_targetPos.GetUp(), arg_targetPos.GetFront());


				//角度の差
				float nowAngle = atan2f(nowYVec.y, nowYVec.x);
				float pushBackAngle = atan2f(pushBackVec.y, pushBackVec.x);
				float divAngle = pushBackAngle - nowAngle;

				//角度を押し戻す。
				m_nowParam.m_yAxisAngle += divAngle;
				arg_playerRotY += divAngle;

			}

		}

	}

}