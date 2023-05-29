#include "Enemy.h"
#include"FrameWork/UsersInput.h"
#include"../../Graphics/BasicDraw.h"
#include"../../OperationConfig.h"
#include"../../Player/Player.h"

int MiniBug::ENEMY_MAX_ID = 0;


#pragma region MiniBug
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

	m_dashEffect.Finalize();
	m_finalizeFlag = false;


	m_sightArea.Init(&m_transform);
	track.Init(0.5f);

	m_nowStatus = SEARCH;
	m_prevStatus = NONE;
	m_limitIndex = 0;
	m_deadFlag = false;
	m_startDeadMotionFlag = false;
	m_deadTimer.Reset(120);

	m_hitBoxSize = m_transform.GetScale().Length() * DebugEnemy::Instance()->HitBox(ENEMY_MINIBUG);
	m_hitBox.m_centerPos = &m_transform.GetPos();
	m_hitBox.m_radius = &m_hitBoxSize;

	m_shadowInfluenceRange = SHADOW_INFLUENCE_RANGE;

	m_animator->Play("Wing", true, false, KuroEngine::GetRand(5.0f));
	m_animator->SetStartPosture("To_Angry");

	m_knockBackTime = 10;
}

void MiniBug::Update(Player &arg_player)
{
#ifdef _DEBUG
	m_hitBoxSize = m_transform.GetScale().Length() * DebugEnemy::Instance()->HitBox(ENEMY_MINIBUG);
#endif // _DEBUG


	m_dashEffect.Update(m_larpPos, m_nowStatus == MiniBug::ATTACK && m_jumpMotion.IsDone());
	m_eyeEffect.Update(m_larpPos);

	//共通処理
	if (m_deadFlag)
	{
		m_reaction->Update(m_pos);

		if (!m_finalizeFlag)
		{
			m_dashEffect.Finalize();
			m_finalizeFlag = true;
		}
		return;
	}
	else
	{
	}


	//死亡準備処理
	if (m_startDeadMotionFlag && !m_deadFlag)
	{
		//死んでいたら丸影を小さくする。
		m_shadowInfluenceRange = KuroEngine::Math::Lerp(m_shadowInfluenceRange, 0.0f, 0.01f);

		if (m_deadTimer.UpdateTimer() && m_deadTimer.GetElaspedTime() != 0.0f)
		{
			m_deadTimer.Reset(120);
			m_deadFlag = true;
		}

		//死亡時の座標
		KuroEngine::Vec3<float>vel = m_initializedTransform.GetUp();
		m_pos += vel * 0.1f;
		m_transform.SetPos(m_pos);

		//死亡時のスケール
		m_scale = KuroEngine::Math::Ease(KuroEngine::Out, KuroEngine::Back, m_deadTimer.GetTimeRate(), m_initializedTransform.GetScale().x, 0.0f);
		m_transform.SetScale(m_scale);

		//死亡時の回転
		DirectX::XMVECTOR vec = { 0.0f,0.0f,1.0f,1.0f };
		m_larpRotation = DirectX::XMQuaternionRotationAxis(vec, KuroEngine::Angle::ConvertToRadian(360.0f));
		KuroEngine::Quaternion rotation = m_transform.GetRotate();
		rotation = DirectX::XMQuaternionSlerp(m_transform.GetRotate(), m_larpRotation, 0.1f);
		m_transform.SetRotate(rotation);

		m_reaction->Update(m_pos);

		return;
	}

	//生きていたら丸影を元に戻す。
	m_shadowInfluenceRange = KuroEngine::Math::Lerp(m_shadowInfluenceRange, SHADOW_INFLUENCE_RANGE, 0.1f);

	////敵発見時(プレイヤーが視界に入った)
	//if (OperationConfig::Instance()->DebugKeyInputOnTrigger(DIK_1))
	//{
	//	OnInit();
	//}

	bool findFlag = m_sightArea.IsFind(arg_player.GetTransform().GetPos(), 180.0f);
	//プレイヤーが違う法線の面にいたら見ないようにする。
	bool isDifferentWall = m_transform.GetUp().Dot(arg_player.GetTransform().GetUpWorld()) <= 0.5f;
	bool isPlayerWallChange = arg_player.GetIsJump();
	bool isAttackOrNotice = m_nowStatus == MiniBug::ATTACK || m_nowStatus == MiniBug::NOTICE;
	if ((isDifferentWall || isPlayerWallChange) && isAttackOrNotice) {
		findFlag = false;
		m_nowStatus = MiniBug::NOTICE;
	}
	const bool isMoveFlag = 0.1f < KuroEngine::Vec3<float>(arg_player.GetNowPos() - arg_player.GetOldPos()).Length();
	if (findFlag && arg_player.GetIsUnderGround() && m_nowStatus != MiniBug::RETURN && isMoveFlag)
	{
		m_nowStatus = MiniBug::NOTICE;
	}
	else if (findFlag && !arg_player.GetIsUnderGround() && !isDifferentWall && m_nowStatus != MiniBug::KNOCK_BACK)
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
			track.Init(0.5f);

			m_jumpMotion.Init(m_pos, m_pos + KuroEngine::Vec3<float>(0.0f, 5.0f, 0.0f), 0.5f);

			//Doneフラグをfalseにして、演出が終わってない状態にする。
			m_jumpMotion.UnDone();

			//怒り目
			m_animator->SetEndPosture("To_Angry");

			break;
		case MiniBug::NOTICE:
			m_reaction->Init(LOOK, m_transform.GetUp());
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

			m_trackPlayer.Init(m_pos, m_bPointPos, 0.5f);

			m_limitIndex = index;
			break;

		case MiniBug::KNOCK_BACK:
		{
			KuroEngine::Vec3<float>dir(m_pos - arg_player.GetTransform().GetPos());
			dir.y = 0.0f;
			dir.Normalize();
			m_knockBack.Init(m_pos, dir, m_knockBackTime);
			m_pos = m_larpPos;
		}
		break;

		case MiniBug::HEAD_ATTACK:
		{
			KuroEngine::Vec3<float>dir(m_pos - arg_player.GetTransform().GetPos());
			dir.y = arg_player.GetTransform().GetUp().Dot(arg_player.GetTransform().GetUpWorld());
			dir.Normalize();
			m_headAttack.Init({}, dir);
		}
		default:
			break;
		}

		//通常目
		if (m_nowStatus != MiniBug::ATTACK)m_animator->SetStartPosture("To_Angry");

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
			m_reaction->Init(FIND, m_transform.GetUp());
			break;
		}
		else
		{
			//終わってる状態にする。
			m_jumpMotion.Done();
		}

		distance = arg_player.GetTransform().GetPos().Distance(m_pos);

		m_attackFlag = false;
		//プレイヤーと一定以上距離が離れた場合
		if (125.0f <= distance)
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
			m_thinkTimer.Reset(60);
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
			m_reaction->Init(LOOK, m_transform.GetUp());
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

	case MiniBug::KNOCK_BACK:
		vel = m_knockBack.Update();
		if (m_knockBack.IsDone())
		{
			vel = {};
		}
		//攻撃が当たった時のクールタイム
		if (m_attackCoolTimer.UpdateTimer())
		{
			m_nowStatus = MiniBug::RETURN;
		}
		break;
	case MiniBug::HEAD_ATTACK:
	{
		HeadAttackData data = m_headAttack.Update();
		vel = data.m_dir;
		m_larpRotation = data.m_rotation;
		if (m_headAttack.IsDone())
		{
			m_startDeadMotionFlag = true;
		}
	}
	break;
	default:
		break;
	}
	//更新処理---------------------------------------


	//プレイヤーと敵の判定
	if (!arg_player.GetIsUnderGround() && Collision::Instance()->CheckCircleAndCircle(arg_player.m_sphere, m_hitBox))
	{
		m_nowStatus = MiniBug::KNOCK_BACK;
		m_knockBackTime = 10;
		m_attackCoolTimer.Reset(120);
		arg_player.Damage();
	}

	//草の当たり判定
	if (arg_player.CheckHitGrassSphere(m_transform.GetPosWorld(), m_transform.GetUpWorld(), m_transform.GetScale().Length()) != Player::CHECK_HIT_GRASS_STATUS::NOHIT && !m_startDeadMotionFlag)
	{
		m_startDeadMotionFlag = true;
	}

	m_reaction->Update(m_pos);
	if (1.0f <= m_initializedTransform.GetUp().x)
	{
		vel.x = 0.0f;
		m_dir.x = 0.0f;
	}
	if (1.0f <= m_initializedTransform.GetUp().y)
	{
		vel.y = 0.0f;
		m_dir.y = 0.0f;
	}
	if (1.0f <= m_initializedTransform.GetUp().z)
	{
		vel.z = 0.0f;
		m_dir.z = 0.0f;
	}


	//座標移動
	m_pos += vel;
	m_prevPos = m_pos;

	KuroEngine::Vec3<float>frontVec = m_transform.GetFront();

	//移動方向と正面ベクトルを敵基準の姿勢に投影。
	KuroEngine::Vec2<float> frontVec2D = Project3Dto2D(frontVec, m_transform.GetFront(), m_transform.GetRight());
	KuroEngine::Vec2<float> moveDir2D = Project3Dto2D(m_dir, m_transform.GetFront(), m_transform.GetRight());

	float rptaVel = acosf(frontVec2D.Dot(moveDir2D));
	rptaVel *= (0 < frontVec2D.Cross(moveDir2D)) ? 1.0f : -1.0f;

	//プレイヤーが違う面にるか、ジャンプで壁面移動中はプレイヤーの方を見ない。
	if ((isDifferentWall || isPlayerWallChange) && isAttackOrNotice) {

	}
	else
	{



		//現在の座標からプレイヤーに向かう回転を求める。
		KuroEngine::Vec3<float> axisZ = m_dir;
		axisZ.Normalize();

		//プレイヤーの法線との外積から仮のXベクトルを得る。
		KuroEngine::Vec3<float> axisX = m_initializedTransform.GetUp().Cross(axisZ);

		//Xベクトルから上ベクトルを得る。
		KuroEngine::Vec3<float> axisY = axisZ.Cross(axisX);

		//姿勢を得る。
		DirectX::XMMATRIX matWorld = DirectX::XMMatrixIdentity();
		matWorld.r[0] = { axisX.x, axisX.y, axisX.z, 0.0f };
		matWorld.r[1] = { axisY.x, axisY.y, axisY.z, 0.0f };
		matWorld.r[2] = { axisZ.x, axisZ.y, axisZ.z, 0.0f };

		XMVECTOR rotate, scale, position;
		DirectX::XMMatrixDecompose(&scale, &rotate, &position, matWorld);

		m_transform.SetRotate(DirectX::XMQuaternionSlerp(m_transform.GetRotate(), rotate, 0.08f));
	}


	m_larpPos = KuroEngine::Math::Lerp(m_larpPos, m_pos, 0.1f);

	m_transform.SetPos(m_larpPos);

	m_animator->Update(TimeScaleMgr::s_inGame.GetTimeScale());
}

void MiniBug::Draw(KuroEngine::Camera &arg_cam, KuroEngine::LightManager &arg_ligMgr)
{
	if (m_deadFlag)
	{
		return;
	}

	IndividualDrawParameter edgeColor = IndividualDrawParameter::GetDefault();
	edgeColor.m_edgeColor = KuroEngine::Color(0.54f, 0.14f, 0.33f, 1.0f);

	KuroEngine::Vec3<float>offset(5.0f, 5.0f, 5.0f);
	offset *= m_transform.GetUp();
	KuroEngine::Transform drawTransform(m_transform);
	drawTransform.SetPos(m_transform.GetPos() + offset);

	BasicDraw::Instance()->Draw_NoGrass(
		arg_cam,
		arg_ligMgr,
		m_model,
		drawTransform,
		edgeColor,
		KuroEngine::AlphaBlendMode_None,
		m_animator->GetBoneMatBuff());

	m_reaction->Draw(arg_cam);

	//m_dashEffect.Draw(arg_cam);
	m_eyeEffect.Draw(arg_cam);

	if (DebugEnemy::Instance()->VisualizeEnemyHitBox())
	{
		m_debugHitBox->Draw(arg_cam, arg_ligMgr);
	}
	if (DebugEnemy::Instance()->VisualizeEnemySight())
	{
		m_sightArea.DebugDraw(arg_cam);
	}
	DebugDraw(arg_cam);

}

void MiniBug::DebugDraw(KuroEngine::Camera &camera)
{
#ifdef _DEBUG

	//return;

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

	if (DebugEnemy::Instance()->VisualizeEnemySight())
	{
		m_sightArea.DebugDraw(camera);
	}


#endif // _DEBUG

}
#pragma endregion


#pragma region Dossun
void DossunRing::OnInit()
{
	m_attackHitBoxRadius = 0.0f;
	m_findPlayerFlag = false;
	m_preFindPlayerFlag = false;


	switch (m_nowStatus)
	{
	case ENEMY_ATTACK_PATTERN_NORMAL:
		DebugEnemy::Instance()->Stack(m_initializedTransform, ENEMY_DOSSUN_NORMAL);
		SetParam();
		break;
	case ENEMY_ATTACK_PATTERN_ALWAYS:
		DebugEnemy::Instance()->Stack(m_initializedTransform, ENEMY_DOSSUN_ALLWAYS);
		SetParam();
		break;
	case ENEMY_ATTACK_PATTERN_INVALID:
		break;
	default:
		break;
	}



	m_enemyHitBox.m_centerPos = &m_transform.GetPos();
	m_enemyHitBox.m_radius = &m_radius;

	//視界の判定---------------------------------------
	m_sightHitBox.m_centerPos = &m_transform.GetPos();
	m_sightHitBox.m_radius = &m_sightRange;
	m_sightArea.Init(m_sightHitBox);
	//視界の判定---------------------------------------

	m_attackInterval.Reset(m_maxAttackIntervalTime);
	m_attackTimer.Reset(m_maxAttackTime);

	//死亡処理---------------------------------------
	m_deadFlag = false;
	m_startDeadMotionFlag = false;
	m_deadTimer.Reset(120);
	//死亡処理---------------------------------------

	m_hitBox.m_centerPos = &m_transform.GetPos();
	m_hitBox.m_radius = &m_attackHitBoxRadius;

	m_ringColor.m_r = 1.0f;
	m_ringColor.m_g = 1.0f;
	m_ringColor.m_b = 1.0f;
	m_ringColor.m_a = 1.0f;

	m_intervalFlag = false;
}

void DossunRing::Update(Player &arg_player)
{
	m_reaction->Update(m_transform.GetPos());

	if (m_deadFlag && IsActive(m_transform, arg_player.GetTransform()))
	{
		Attack(arg_player);
		return;
	}
	else
	{
		m_findPlayerFlag = false;
	}
	//死亡準備処理
	if (m_startDeadMotionFlag && !m_deadFlag)
	{
		Attack(arg_player);

		m_deadScale = KuroEngine::Math::Ease(KuroEngine::Out, KuroEngine::Back, m_deadTimer.GetTimeRate(), 1.0f, 0.0f);
		m_transform.SetScale(m_deadScale);

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

	//プレイヤーと敵の判定
	if (!arg_player.GetIsUnderGround() && Collision::Instance()->CheckCircleAndCircle(arg_player.m_sphere, m_enemyHitBox))
	{
		arg_player.Damage();
	}

	//生きていたら丸影を元に戻す。
	m_shadowInfluenceRange = KuroEngine::Math::Lerp(m_shadowInfluenceRange, SHADOW_INFLUENCE_RANGE, 0.1f);


	if (m_sightArea.IsFind(arg_player.m_sphere) && !arg_player.GetIsUnderGround())
	{
		m_findPlayerFlag = true;
		m_intervalFlag = true;
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
//#ifdef _DEBUG
		SetParam();
//#endif
		m_findPlayerFlag = true;
		break;

	case ENEMY_ATTACK_PATTERN_NORMAL:
//#ifdef _DEBUG
		SetParam();
//#endif
		break;
	default:
		break;
	}

	if (!m_findPlayerFlag && !m_attackFlag && !m_intervalFlag)
	{
		return;
	}
	//以降プレイヤーが発見された処理---------------------------------------

	//発見リアクション
	if (m_findPlayerFlag && !m_preFindPlayerFlag)
	{
		m_reaction->Init(FIND, m_transform.GetUp());
	}
	m_preFindPlayerFlag = m_findPlayerFlag;


	float attackScaleOffset = m_attackInterval.GetTimeRate();
	float larpRate = 0.08f;
	//攻撃予備動作中
	if (m_attackInterval.UpdateTimer() && !m_attackFlag)
	{
		m_attackTimer.Reset(m_maxAttackTime);
		m_attackFlag = true;
		m_intervalFlag = false;
		attackScaleOffset = 0.0f;
	}
	else if (m_attackFlag)
	{
		attackScaleOffset = 0.0f;
		larpRate = 0.3f;
	}
	//攻撃する瞬間を取ったもの
	m_scale = m_initializedTransform.GetScale() +
		KuroEngine::Math::Ease(KuroEngine::Out, KuroEngine::Cubic, attackScaleOffset, { 0.0f,0.0f,0.0f }, { 1.3f,1.3f,1.3f });
	m_larpScale = KuroEngine::Math::Lerp(m_larpScale, m_scale, larpRate);
	m_transform.SetScale(m_larpScale);

	Attack(arg_player);

}

void DossunRing::Draw(KuroEngine::Camera &arg_cam, KuroEngine::LightManager &arg_ligMgr)
{
	if (m_deadFlag)
	{
		return;
	}

	IndividualDrawParameter edgeColor = IndividualDrawParameter::GetDefault();
	edgeColor.m_edgeColor = KuroEngine::Color(0.0f, 0.0f, 0.0f, 0.0f);

	BasicDraw::Instance()->Draw_NoGrass(
		arg_cam,
		arg_ligMgr,
		m_model,
		m_transform,
		edgeColor);

	KuroEngine::Transform transform = m_initializedTransform;
	transform.SetPos(*(m_hitBox.m_centerPos));
	float scale = *(m_hitBox.m_radius);
	transform.SetScale(KuroEngine::Vec3<float>(scale, 5.0f, scale));
	BasicDraw::Instance()->Draw_NoGrass(
		arg_cam,
		arg_ligMgr,
		m_attackRingModel,
		transform,
		edgeColor,
		m_ringColor);

	if (DebugEnemy::Instance()->VisualizeEnemySight())
	{
		m_sightArea.DebugDraw(arg_cam, arg_ligMgr);
	}
	if (DebugEnemy::Instance()->VisualizeEnemyHitBox())
	{
		m_debugHitBox->Draw(arg_cam, arg_ligMgr);
	}


	m_reaction->Draw(arg_cam);

	//DebugDraw(arg_cam);
}

void DossunRing::DebugDraw(KuroEngine::Camera &camera)
{
#ifdef _DEBUG

	//return;

	if (m_attackFlag)
	{
		/*KuroEngine::Transform transform;
		transform.SetPos(*m_hitBox.m_centerPos);
		transform.SetScale(*m_hitBox.m_radius);
		KuroEngine::DrawFunc3D::DrawNonShadingModel(
			m_hitBoxModel,
			transform.GetMatWorld(),
			camera,
			0.5f
		);*/
	}
	else
	{
	}

#endif // _DEBUG

}
#pragma endregion

void DossunRing::Attack(Player &arg_player)
{
	//攻撃開始
	if (m_attackFlag)
	{
		//当たり判定の広がり
		m_attackHitBoxRadius = m_attackTimer.GetTimeRate() * m_attackhitBoxRadiusMax;
		m_ringColor.m_a = m_attackTimer.GetInverseTimeRate();

		//広がり切ったらインターバルに戻る
		if (m_attackTimer.UpdateTimer())
		{
			m_attackHitBoxRadius = 0.0f;
			m_attackInterval.Reset(m_maxAttackIntervalTime);
			m_attackFlag = false;
			m_findPlayerFlag = false;
		}

		KuroEngine::Vec3<float>playerPos(arg_player.GetTransform().GetPos());
		KuroEngine::Vec3<float>playerUpVec(arg_player.GetTransform().GetUp());
		KuroEngine::Vec3<float>enemyPlayerVec(arg_player.GetTransform().GetPos() - *(m_hitBox.m_centerPos));
		enemyPlayerVec.Normalize();

		if (!arg_player.GetIsUnderGround() && Collision::Instance()->CheckPointAndEdgeOfCircle(
			m_hitBox,
			playerPos,
			playerUpVec,
			enemyPlayerVec))
		{
			arg_player.Damage();
		}
	}
}


#pragma region Battery
Battery::Battery(std::weak_ptr<KuroEngine::Model> arg_model, KuroEngine::Transform arg_initTransform, std::vector<KuroEngine::Vec3<float>> arg_posArray, float arg_bulletScale, ENEMY_BARREL_PATTERN arg_barrelPattern)
	:StageParts(BATTERY, arg_model, arg_initTransform), m_posArray(arg_posArray), m_barrelPattern(arg_barrelPattern)
{
	//座標配列が空ならその場にとどまる
	if (m_posArray.empty())
	{
		m_posArray.emplace_back(arg_initTransform.GetPosWorld());
		m_pos = m_transform.GetPos();
	}
	else if (2 <= m_posArray.size())
	{
		m_patrol = std::make_unique<PatrolBasedOnControlPoint>(m_posArray, 0, true);
		m_patrol->Init(0);
		m_pos = m_posArray[0];
	}
	m_transform = arg_initTransform;
	m_initTransform = arg_initTransform;
	m_transform.SetPos(m_pos);

	m_upVec = arg_initTransform.GetUp();

	OnInit();

	switch (arg_barrelPattern)
	{
	case ENEMY_BARREL_PATTERN_FIXED:
		DebugEnemy::Instance()->Stack(m_initializedTransform, ENEMY_BATTERY_FIXED);
		break;
	case ENEMY_BARREL_PATTERN_ROCKON:
		DebugEnemy::Instance()->Stack(m_initializedTransform, ENEMY_BATTERY_ROCKON);
		break;
	case ENEMY_BARREL_PATTERN_INVALID:
		break;
	default:
		break;
	}
}

void Battery::OnInit()
{
	m_bulletDir = m_transform.GetFront();
	m_bulletManager.Init(&m_pos, 5.0f, &m_bulletDir, 120.0f);

	m_radius = m_transform.GetScale().x;
	m_hitBox.m_centerPos = &m_pos;
	m_hitBox.m_radius = &m_radius;

	m_startDeadMotionFlag = false;
	m_deadFlag = false;
	m_noticeFlag = false;
}

void Battery::Update(Player &arg_player)
{
	if (m_deadFlag)
	{
		return;
	}
	if (m_startDeadMotionFlag)
	{
		m_deadFlag = true;
	}

	KuroEngine::Vec3<float>vel = {};
	//制御点が二つ以上ある場合は交互に動く
	if (m_patrol)
	{
		vel = m_patrol->Update(m_transform.GetPos());
	}

	KuroEngine::Vec3<float>dir(arg_player.GetTransform().GetPos() - m_transform.GetPos());
	float distance = arg_player.GetTransform().GetPos().Distance(m_transform.GetPos());

	bool isDiffrentWallFlag = m_transform.GetUp().Dot(arg_player.GetTransform().GetUpWorld()) <= 0.5f;

	//射程範囲内、地面に潜ってない、同じ面にいる
	m_bulletManager.Update(120.0f, arg_player.m_sphere,
		distance <= 50.0f &&
		!arg_player.GetIsUnderGround() &&
		!isDiffrentWallFlag
	);

	//敵と弾の判定
	if (m_bulletManager.IsHit())
	{
		arg_player.Damage();
	}
	//敵とプレイヤーの判定
	if (!arg_player.GetIsUnderGround() && Collision::Instance()->CheckCircleAndCircle(arg_player.m_sphere, m_hitBox))
	{
		arg_player.Damage();
	}
	//草の当たり判定
	if (arg_player.CheckHitGrassSphere(m_transform.GetPosWorld(), m_transform.GetUpWorld(), m_transform.GetScale().Length()) != Player::CHECK_HIT_GRASS_STATUS::NOHIT && !m_startDeadMotionFlag)
	{
		m_startDeadMotionFlag = true;
	}

	switch (m_barrelPattern)
	{
	case ENEMY_BARREL_PATTERN_FIXED:
		//方向固定
		break;
	case ENEMY_BARREL_PATTERN_ROCKON:
		//地面に居ない時にプレイヤーの方向を見る------
		if (!arg_player.GetIsUnderGround() && distance <= 50.0f)
		{
			if (!m_noticeFlag)
			{
				m_reaction->Init(FIND, m_transform.GetUp());
				m_noticeFlag = true;
			}

			//敵の方向を向く処理
			dir.Normalize();
			KuroEngine::Vec3<float>frontVec(m_transform.GetFront());
			KuroEngine::Vec3<float>axis = frontVec.Cross(dir);
			float rptaVel = acosf(frontVec.Dot(dir));

			DirectX::XMVECTOR dirVec = { axis.x,axis.y,axis.z,1.0f };
			m_rotation = DirectX::XMQuaternionRotationAxis(dirVec, rptaVel);

			m_bulletDir = dir;
		}
		//地面に居ない時にプレイヤーの方向を見る------
		//見つからない時は別方向を見る------
		else
		{
			m_noticeFlag = false;
			m_larpRotation = {};
		}
		m_larpRotation = DirectX::XMQuaternionSlerp(m_transform.GetRotate(), m_rotation, 0.1f);

		break;
	case ENEMY_BARREL_PATTERN_INVALID:
		break;
	default:
		break;
	}

	m_pos += vel;
	m_transform.SetPos(m_pos);

	m_reaction->Update(m_pos);
	//m_transform.SetRotate(m_larpRotation);

}

void Battery::Draw(KuroEngine::Camera &arg_cam, KuroEngine::LightManager &arg_ligMgr)
{
	if (m_deadFlag)
	{
		return;
	}

	IndividualDrawParameter edgeColor = IndividualDrawParameter::GetDefault();
	edgeColor.m_edgeColor = KuroEngine::Color(0.0f, 0.0f, 0.0f, 1.0f);

	BasicDraw::Instance()->Draw_NoGrass(
		arg_cam,
		arg_ligMgr,
		m_model,
		m_transform,
		edgeColor);

	m_bulletManager.Draw(arg_cam);

	m_reaction->Draw(arg_cam);

	if (DebugEnemy::Instance()->VisualizeEnemySight())
	{

	}
	if (DebugEnemy::Instance()->VisualizeEnemyHitBox())
	{

	}
	//DebugDraw(arg_cam);
}
#pragma endregion