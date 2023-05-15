#include "Enemy.h"
#include"../../Player/Player.h"
#include"FrameWork/UsersInput.h"
#include"../../Graphics/BasicDraw.h"

void MiniBug::OnInit()
{
	m_nowStatus = SEARCH;
	m_prevStatus = SEARCH;
	m_limitIndex = 0;
	m_deadFlag = false;
	m_startDeadMotionFlag = false;
	m_deadTimer.Reset(120);
	m_scale = 1.0f;

	m_hitBox.m_centerPos = &m_pos;
	m_hitBox.m_radius = &m_scale;

	m_shadowInfluenceRange = SHADOW_INFLUENCE_RANGE;

	m_patrol->Init(m_limitIndex);
	m_pos = m_patrol->GetLimitPos(m_limitIndex);
}

void MiniBug::Update(Player& arg_player)
{
	//共通処理
	if (m_deadFlag)
	{

		m_reaction->Update(m_pos);

		return;
	}
	else {

	}


	//死亡準備処理
	if (m_startDeadMotionFlag && !m_deadFlag)
	{
		m_scale = KuroEngine::Math::Ease(KuroEngine::Out, KuroEngine::Back, m_deadTimer.GetTimeRate(), 1.0f, 0.0f);
		m_transform.SetScale(m_scale);

		//死んでいたら丸影を小さくする。
		m_shadowInfluenceRange = KuroEngine::Math::Lerp(m_shadowInfluenceRange, 0.0f, 0.01f);

		if (m_deadTimer.UpdateTimer() && m_deadTimer.GetElaspedTime() != 0.0f)
		{
			m_deadTimer.Reset(120);
			m_deadFlag = true;
		}

		DirectX::XMVECTOR vec = { 0.0f,0.0f,1.0f,1.0f };
		m_larpRotation = DirectX::XMQuaternionRotationAxis(vec, KuroEngine::Angle::ConvertToRadian(90.0f));
		KuroEngine::Quaternion rotation = m_transform.GetRotate();
		rotation = DirectX::XMQuaternionSlerp(m_transform.GetRotate(), m_larpRotation, 0.1f);
		m_transform.SetRotate(rotation);

		m_reaction->Update(m_pos);

		return;
	}

	//生きていたら丸影を元に戻す。
	m_shadowInfluenceRange = KuroEngine::Math::Lerp(m_shadowInfluenceRange, SHADOW_INFLUENCE_RANGE, 0.1f);

	//敵発見時(プレイヤーが視界に入った)
	if (KuroEngine::UsersInput::Instance()->KeyOnTrigger(DIK_1))
	{
		OnInit();
	}

	bool findFlag = m_sightArea.IsFind(arg_player.GetTransform().GetPos(), 180.0f);
	//プレイヤーが違う法線の面にいたら見ないようにする。
	bool isDifferentWall = m_transform.GetUp().Dot(arg_player.GetTransform().GetUpWorld()) <= 0.5f;
	bool isPlayerWallChange = arg_player.GetIsJump();
	bool isAttackOrNotice = m_nowStatus == MiniBug::ATTACK || m_nowStatus == MiniBug::NOTICE;
	if ((isDifferentWall || isPlayerWallChange) && isAttackOrNotice) {
		findFlag = false;
		m_nowStatus = MiniBug::NOTICE;
	}
	const bool isMoveFlag = arg_player.GetNowPos() != arg_player.GetOldPos();
	if (findFlag && arg_player.GetIsUnderGround() && m_nowStatus != MiniBug::RETURN && isMoveFlag)
	{
		m_nowStatus = MiniBug::NOTICE;
	}
	else if (findFlag && !arg_player.GetIsUnderGround() && !isDifferentWall)
	{
		m_nowStatus = MiniBug::ATTACK;
	}



	//初期化---------------------------------------------------
	if (m_nowStatus != m_prevStatus)
	{
		int index = 0;
		float min = std::numeric_limits<float>().max();
		float prevMin = 0.0f;
		switch (m_nowStatus)
		{
			//最も近い制御地点からループする
		case MiniBug::SEARCH:

			//プレイヤーが違う法線の面にいたら見ないようにする。
			if (m_transform.GetUp().Dot(arg_player.GetTransform().GetUpWorld()) <= 0.5f) break;

			m_patrol->Init(m_limitIndex);
			m_pos = m_patrol->GetLimitPos(m_limitIndex);

			break;
		case MiniBug::ATTACK:
			m_attackIntervalTimer.Reset(120);
			m_readyToGoToPlayerTimer.Reset(120);
			m_sightArea.Init(&m_transform);
			track.Init(0.1f);

			m_jumpMotion.Init(m_pos, m_pos + KuroEngine::Vec3<float>(0.0f, 5.0f, 0.0f), 0.5f);

			//Doneフラグをfalseにして、演出が終わってない状態にする。
			m_jumpMotion.UnDone();

			break;
		case MiniBug::NOTICE:
			m_reaction->Init(LOOK);
			break;
		case MiniBug::RETURN:
			//最も近い制御点に戻る
			m_aPointPos = m_pos;
			for (int i = 0; i < m_posArray.size(); ++i)
			{
				min = std::min(m_posArray[i].Distance(m_pos), min);
				if (min != prevMin)
				{
					index = i;
					prevMin = min;
				}
			}
			m_bPointPos = m_posArray[index];

			m_trackPlayer.Init(m_pos, m_bPointPos, 0.1f);

			m_limitIndex = index;
			break;
		default:
			break;
		}
		m_thinkTimer.Reset(120);
		m_prevStatus = m_nowStatus;
	}
	//初期化---------------------------------------------------


	float distance = 0.0f;

	//更新処理---------------------------------------
	KuroEngine::Vec3<float>vel = { 0.0f,0.0f,0.0f };
	switch (m_nowStatus)
	{
	case MiniBug::SEARCH:

		//プレイヤーが違う法線の面にいたら見ないようにする。
		//if (m_transform.GetUp().Dot(arg_player.GetTransform().GetUpWorld()) <= 0.5f) break;

		vel = m_patrol->Update(m_pos);
		m_dir = vel;

		{
			//プレイヤーを回転させる。
			KuroEngine::Vec3<float> defVec = m_transform.GetFront();
			KuroEngine::Vec3<float> axis = defVec.Cross(vel.GetNormal());
			float rad = std::acosf(defVec.Dot(vel.GetNormal()));
			if (0 < axis.Length() && !std::isnan(rad)) {
				auto q = DirectX::XMQuaternionRotationAxis(axis, rad);

				//qが補間先
				q = DirectX::XMQuaternionMultiply(m_transform.GetRotate(), q);

				//補間する。
				m_transform.SetRotate(DirectX::XMQuaternionSlerp(m_transform.GetRotate(), q, 0.1f));

			}
		}

		break;
	case MiniBug::ATTACK:

		//見つけた時のリアクション時間
		//if (!m_readyToGoToPlayerTimer.UpdateTimer()) 時間で切り替える
		if (arg_player.GetIsUnderGround())
		{
			m_nowStatus = MiniBug::NOTICE;
		}

		if (!m_jumpMotion.IsDone())	//モーションで切り替える
		{
			//注視
			//何かしらのアクションを書く
			vel = m_jumpMotion.GetVel(m_pos);
			m_dir = KuroEngine::Vec3<float>(arg_player.GetTransform().GetPos() - m_pos).GetNormal();
			m_dir.y = 0.0f;
			m_reaction->Init(FIND);
			break;
		}
		else
		{
			//終わってる状態にする。
			m_jumpMotion.Done();
		}


		distance = arg_player.GetTransform().GetPos().Distance(m_pos);


		//プレイヤーと一定距離まで近づいたら攻撃予備動作を入れる
		if (m_attackCoolTimer.UpdateTimer() && distance <= 5.0f && !m_attackFlag)
		{
			m_attackFlag = true;
			m_reaction->Init(HIT);
			m_attackMotion.Init(m_pos, m_pos + KuroEngine::Vec3<float>(0.0f, 2.0f, 0.0f), 0.5f);
		}

		//攻撃予備動作が終わって攻撃を行った。
		if (m_attackFlag && m_attackIntervalTimer.UpdateTimer())
		{
			//プレイヤーと敵の判定
			if (Collision::Instance()->CheckCircleAndCircle(arg_player.m_sphere, m_hitBox))
			{
				arg_player.Damage();
			}

			//プレイヤーと敵の当たり判定の処理をここに書く
			m_attackIntervalTimer.Reset(120);
			m_attackCoolTimer.Reset(120);
			m_attackFlag = false;
		}

		//プレイヤーと一定以上距離が離れた場合
		if (30.0f <= distance)
		{
			//暫く止まり、何もなければ思考を切り替える。
			if (m_thinkTimer.UpdateTimer())
			{
				m_nowStatus = MiniBug::RETURN;
			}
			m_aPointPos = m_pos;
		}
		//プレイヤーを追尾中
		else if (!m_attackFlag)
		{
			m_thinkTimer.Reset(120);
			vel = track.Update(m_pos, arg_player.GetTransform().GetPos());
			m_dir = track.Update(m_pos, arg_player.GetTransform().GetPos()).GetNormal();
		}

		break;
	case MiniBug::NOTICE:
		//暫く待って動かなかったら別の場所に向かう
		if (m_thinkTimer.UpdateTimer())
		{
			m_nowStatus = MiniBug::RETURN;
		}
		//動いたら注視する
		if (isMoveFlag)
		{
			m_dir = KuroEngine::Vec3<float>(arg_player.GetTransform().GetPos() - m_pos).GetNormal();
			m_thinkTimer.Reset(120);
			m_reaction->Init(LOOK);
		}
		break;
	case MiniBug::RETURN:
		//期間中
		m_thinkTimer.Reset(120);
		vel = m_trackPlayer.Update();
		m_dir = vel.GetNormal();
		if (m_trackPlayer.IsArrive(arg_player.GetTransform().GetPos()))
		{
			m_nowStatus = MiniBug::SEARCH;
		}
		break;
	default:
		break;
	}
	//更新処理---------------------------------------


	//草の当たり判定
	if (arg_player.CheckHitGrassSphere(m_transform.GetPosWorld(), m_transform.GetUpWorld(), m_transform.GetScale().Length()) != Player::CHECK_HIT_GRASS_STATUS::NOHIT && !m_startDeadMotionFlag)
	{
		m_reaction->Init(DEAD);
		m_startDeadMotionFlag = true;
	}

	m_reaction->Update(m_pos);

	//座標移動
	m_pos += vel;
	m_prevPos = m_pos;

	KuroEngine::Vec3<float>frontVec(0.0f, 0.0f, 1.0f);

	m_dir.y = 0.0f;
	KuroEngine::Vec3<float>axis = frontVec.Cross(m_dir);
	float rptaVel = acosf(frontVec.Dot(m_dir));

	if (axis.IsZero())
	{
		//m_larpRotation = DirectX::XMQuaternionIdentity();
	}
	//プレイヤーが違う面にるか、ジャンプで壁面移動中はプレイヤーの方を見ない。
	else if ((isDifferentWall || isPlayerWallChange) && isAttackOrNotice) {

	}
	else
	{
		DirectX::XMVECTOR dirVec = { axis.x,axis.y,axis.z,1.0f };
		m_larpRotation = DirectX::XMQuaternionRotationAxis(dirVec, rptaVel);
		KuroEngine::Quaternion rotation = m_transform.GetRotate();

		//見つけた時のジャンプ中じゃなかったらプレイヤーの方向を向く。
		if (m_jumpMotion.IsDone() || m_nowStatus != MiniBug::ATTACK)
		{
			rotation = DirectX::XMQuaternionSlerp(m_transform.GetRotate(), m_larpRotation, 0.1f);
		}
		//m_transform.SetRotate(rotation);
	}

	m_larpPos = KuroEngine::Math::Lerp(m_larpPos, m_pos, 0.1f);

	m_transform.SetPos(m_larpPos);

}

void MiniBug::Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)
{

	IndividualDrawParameter edgeColor = IndividualDrawParameter::GetDefault();
	edgeColor.m_edgeColor = KuroEngine::Color(0.0f, 0.0f, 0.0f, 1.0f);

	BasicDraw::Instance()->Draw_Player(
		arg_cam,
		arg_ligMgr,
		m_model,
		m_transform,
		edgeColor);

	m_reaction->Draw(arg_cam);

}

void MiniBug::DebugDraw(KuroEngine::Camera& camera)
{
#ifdef _DEBUG

	switch (m_nowStatus)
	{
	case MiniBug::SEARCH:
		m_patrol->DebugDraw();
		break;
	case MiniBug::ATTACK:
		break;
	case MiniBug::NOTICE:
		break;
	case MiniBug::RETURN:
		break;
	default:
		break;
	}

	m_sightArea.DebugDraw(camera);

#endif // _DEBUG

}

void DossunRing::Update(Player& arg_player)
{
	if (m_deadFlag)
	{

		return;
	}

	//死亡準備処理
	if (m_startDeadMotionFlag && !m_deadFlag)
	{
		m_scale = KuroEngine::Math::Ease(KuroEngine::Out, KuroEngine::Back, m_deadTimer.GetTimeRate(), 1.0f, 0.0f);
		m_transform.SetScale(m_scale);

		//死んでいたら丸影を小さくする。
		m_shadowInfluenceRange = KuroEngine::Math::Lerp(m_shadowInfluenceRange, 0.0f, 0.01f);

		if (m_deadTimer.UpdateTimer() && m_deadTimer.GetElaspedTime() != 0.0f)
		{
			m_deadTimer.Reset(120);
			m_deadFlag = true;
		}

		DirectX::XMVECTOR vec = { 0.0f,0.0f,1.0f,1.0f };
		m_larpRotation = DirectX::XMQuaternionRotationAxis(vec, KuroEngine::Angle::ConvertToRadian(90.0f));
		KuroEngine::Quaternion rotation = m_transform.GetRotate();
		rotation = DirectX::XMQuaternionSlerp(m_transform.GetRotate(), m_larpRotation, 0.1f);
		m_transform.SetRotate(rotation);

		return;
	}


	//プレイヤーと敵の当たり判定の処理をここに書く
	if (arg_player.CheckHitGrassSphere(m_transform.GetPosWorld(), m_transform.GetUpWorld(), m_transform.GetScale().Length()) != Player::CHECK_HIT_GRASS_STATUS::NOHIT)
	{
		m_startDeadMotionFlag = true;
		return;
	}

	//生きていたら丸影を元に戻す。
	m_shadowInfluenceRange = KuroEngine::Math::Lerp(m_shadowInfluenceRange, SHADOW_INFLUENCE_RANGE, 0.1f);


	if (m_sightArea.IsFind(arg_player.m_sphere) && !arg_player.GetIsUnderGround())
	{
		m_findPlayerFlag = true;
	}
	//攻撃が終わってプレイヤーが見えなくなったら攻撃終了
	else if (!m_attackFlag)
	{
		m_findPlayerFlag = false;
	}

	//亜種の場合は常に攻撃するモーションを入れる
	switch (m_nowStatus)
	{
	case ENEMY_ATTACK_PATTERN_ALWAYS:
		m_findPlayerFlag = true;
		break;
	default:
		break;
	}

	if (!m_findPlayerFlag)
	{
		return;
	}
	//以降プレイヤーが発見された処理---------------------------------------

	//攻撃予備動作中
	if (m_attackInterval.UpdateTimer() && !m_attackFlag)
	{
		m_attackTimer.Reset(m_maxAttackTime);
		m_attackFlag = true;
	}

	//攻撃開始
	if (m_attackFlag)
	{
		//当たり判定の広がり
		m_hitBoxRadius = m_attackTimer.GetTimeRate() * m_hitBoxRadiusMax;
		//広がり切ったらインターバルに戻る
		if (m_attackTimer.UpdateTimer())
		{
			m_hitBoxRadius = 0.0f;
			m_attackInterval.Reset(m_maxAttackIntervalTime);
			m_attackFlag = false;
		}

		if (Collision::Instance()->CheckCircleAndCircle(arg_player.m_sphere, m_hitBox) && !arg_player.GetIsUnderGround())
		{
			arg_player.Damage();
		}
	}


}

void DossunRing::Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)
{
	IndividualDrawParameter edgeColor = IndividualDrawParameter::GetDefault();
	edgeColor.m_edgeColor = KuroEngine::Color(0.0f, 0.0f, 0.0f, 1.0f);

	BasicDraw::Instance()->Draw_Player(
		arg_cam,
		arg_ligMgr,
		m_model,
		m_transform,
		edgeColor);
}

void DossunRing::DebugDraw(KuroEngine::Camera& camera)
{
#ifdef _DEBUG
	if (m_attackFlag)
	{
		KuroEngine::Transform transform;
		transform.SetPos(*m_hitBox.m_centerPos);
		transform.SetScale(*m_hitBox.m_radius);
		KuroEngine::DrawFunc3D::DrawNonShadingModel(
			m_hitBoxModel,
			transform.GetMatWorld(),
			camera,
			0.5f
		);
	}
	else
	{
		m_sightArea.DebugDraw(camera);
	}

#endif // _DEBUG

}