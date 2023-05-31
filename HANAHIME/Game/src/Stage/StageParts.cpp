#include"StageParts.h"
#include"StageParts.h"
#include"ForUser/Object/Model.h"
#include"../Graphics/BasicDraw.h"
#include"../Player/Player.h"
#include"ForUser/DrawFunc/3D/DrawFunc3D.h"
#include"Switch.h"
#include"ForUser/DrawFunc/BillBoard/DrawFuncBillBoard.h"
#include"../SoundConfig.h"
#include"../Player/PlayerCollision.h"
#include"../TimeScaleMgr.h"
#include"GateManager.h"
#include"CheckPointHitFlag.h"
#include"../OperationConfig.h"
#include"../System/SaveDataManager.h"
#include"StageManager.h"

std::array<std::string, StageParts::STAGE_PARTS_TYPE::NUM>StageParts::s_typeKeyOnJson =
{
	"Terrian","Start","Goal","Appearance","MoveScaffold","Lever","Ivy_Zipline","IvyBlock","SplatoonFence","Gate","CheckPoint","StarCoin","BackGround","Kinoko",
	"MiniBug","DossunRing","Battery",
};

const std::string &StageParts::GetTypeKeyOnJson(STAGE_PARTS_TYPE arg_type)
{
	return s_typeKeyOnJson[arg_type];
}

void StageParts::Init()
{
	//m_transform.SetPos(m_initializedTransform.GetPos());
	//m_transform.SetScale(m_initializedTransform.GetScale());
	//m_transform.SetRotate(m_initializedTransform.GetRotate());
	m_transform = m_initializedTransform;
	OnInit();
}

void StageParts::Draw(KuroEngine::Camera &arg_cam, KuroEngine::LightManager &arg_ligMgr)
{
	BasicDraw::Instance()->Draw(
		arg_cam,
		arg_ligMgr,
		m_model.lock(),
		m_transform);
}

void TerrianMeshCollider::BuilCollisionMesh(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_transform)
{
	//�����蔻��p�̃��b�V�������f���̃��b�V���ɍ��킹��B
	int meshNum = static_cast<int>(arg_model.lock()->m_meshes.size());
	m_collisionMesh.resize(meshNum);

	//�����蔻��p���b�V�����쐬�B
	for (int meshIdx = 0; meshIdx < meshNum; ++meshIdx)
	{
		auto &mesh = arg_model.lock()->m_meshes[meshIdx].mesh;

		/*-- �@ ���f����񂩂瓖���蔻��p�̃|���S�������o�� --*/

	//�����蔻��p�|���S��
		struct TerrianHitPolygon {
			bool m_isActive;					//���̃|���S�����L��������Ă��邩�̃t���O
			KuroEngine::ModelMesh::Vertex m_p0;	//���_0
			KuroEngine::ModelMesh::Vertex m_p1;	//���_1
			KuroEngine::ModelMesh::Vertex m_p2;	//���_2
		};

		//�����蔻��p�|���S���R���e�i���쐬�B
		m_collisionMesh[meshIdx].resize(mesh->indices.size() / static_cast<size_t>(3));

		//�����蔻��p�|���S���R���e�i�Ƀf�[�^�����Ă����B
		for (auto &index : m_collisionMesh[meshIdx]) {

			// ���݂�Index���B
			int nowIndex = static_cast<int>(&index - &m_collisionMesh[meshIdx][0]);

			// ���_����ۑ��B
			index.m_p0 = mesh->vertices[mesh->indices[nowIndex * 3 + 0]];
			index.m_p1 = mesh->vertices[mesh->indices[nowIndex * 3 + 1]];
			index.m_p2 = mesh->vertices[mesh->indices[nowIndex * 3 + 2]];

			// �|���S����L�����B
			index.m_isActive = true;

		}

		/*-- �A �|���S�������[���h�ϊ����� --*/
		//���[���h�s��
		DirectX::XMMATRIX targetWorldMat = arg_transform.GetMatWorld();
		for (auto &index : m_collisionMesh[meshIdx]) {
			//���_��ϊ�
			index.m_p0.pos = KuroEngine::Math::TransformVec3(index.m_p0.pos, targetWorldMat);
			index.m_p1.pos = KuroEngine::Math::TransformVec3(index.m_p1.pos, targetWorldMat);
			index.m_p2.pos = KuroEngine::Math::TransformVec3(index.m_p2.pos, targetWorldMat);
			//�@������]�s�񕪂����ϊ�
			index.m_p0.normal = KuroEngine::Math::TransformVec3(index.m_p0.normal, arg_transform.GetRotateWorld());
			index.m_p0.normal.Normalize();
			index.m_p1.normal = KuroEngine::Math::TransformVec3(index.m_p1.normal, arg_transform.GetRotateWorld());
			index.m_p1.normal.Normalize();
			index.m_p2.normal = KuroEngine::Math::TransformVec3(index.m_p2.normal, arg_transform.GetRotateWorld());
			index.m_p2.normal.Normalize();
		}
	}

	//��L�ō���������蔻��p�|���S�������ɁA�����蔻��p��BOX�����B
	m_aabb.clear();
	for (auto &stage : m_collisionMesh) {

		//�i�[����f�[�^��ۑ��B
		m_aabb.emplace_back();

		for (auto &index : stage) {

			//�����蔻��BOX������f�[�^
			m_aabb.back().emplace_back(CreateCubeFromPolygon(index.m_p0.pos, index.m_p1.pos, index.m_p2.pos, index.m_p0.normal));

		}

	}

}

AABB TerrianMeshCollider::CreateCubeFromPolygon(const KuroEngine::Vec3<float> &arg_v1, const KuroEngine::Vec3<float> &arg_v2, const KuroEngine::Vec3<float> &arg_v3, const KuroEngine::Vec3<float> &arg_normal) {

	KuroEngine::Vec3<float> edge1 = arg_v2 - arg_v1;
	KuroEngine::Vec3<float> edge2 = arg_v3 - arg_v1;

	KuroEngine::Vec3<float> inward_normal = arg_normal * (-1);
	KuroEngine::Vec3<float> inward_offset = inward_normal * edge1.Length() / std::sqrt(2.0f);

	std::array<KuroEngine::Vec3<float>, 8> cubeVertices;
	cubeVertices[0] = arg_v1;
	cubeVertices[1] = arg_v2;
	cubeVertices[2] = arg_v3;
	cubeVertices[3] = arg_v1 + edge2;
	cubeVertices[4] = arg_v1 + inward_offset;
	cubeVertices[5] = arg_v2 + inward_offset;
	cubeVertices[6] = arg_v3 + inward_offset;
	cubeVertices[7] = arg_v1 + edge2 + inward_offset;

	AABB aabb;
	aabb.m_min = cubeVertices[0];
	aabb.m_max = cubeVertices[0];

	for (auto &index : cubeVertices) {
		aabb.m_min.x = std::min(aabb.m_min.x, index.x);
		aabb.m_min.y = std::min(aabb.m_min.y, index.y);
		aabb.m_min.z = std::min(aabb.m_min.z, index.z);
		aabb.m_max.x = std::max(aabb.m_max.x, index.x);
		aabb.m_max.y = std::max(aabb.m_max.y, index.y);
		aabb.m_max.z = std::max(aabb.m_max.z, index.z);
	}

	return aabb;
}

std::optional<AABB::CollisionInfo> AABB::CheckAABBCollision(const AABB &arg_aabb1) {
	KuroEngine::Vec3<float> pushBack(0.0f, 0.0f, 0.0f);
	float minOverlap = std::numeric_limits<float>::max();
	int minOverlapAxis = -1;

	for (int i = 0; i < 3; ++i) {
		float aabb1Max = (i == 0) ? m_max.x : (i == 1) ? m_max.y : m_max.z;
		float aabb1Min = (i == 0) ? m_min.x : (i == 1) ? m_min.y : m_min.z;
		float aabb2Max = (i == 0) ? arg_aabb1.m_max.x : (i == 1) ? arg_aabb1.m_max.y : arg_aabb1.m_max.z;
		float aabb2Min = (i == 0) ? arg_aabb1.m_min.x : (i == 1) ? arg_aabb1.m_min.y : arg_aabb1.m_min.z;

		if (aabb1Max < aabb2Min || aabb2Max < aabb1Min) {
			return std::nullopt; // �Փ˂��Ă��Ȃ�
		}

		float overlap1 = aabb1Max - aabb2Min;
		float overlap2 = aabb2Max - aabb1Min;

		if (std::abs(overlap1) < std::abs(overlap2)) {
			if (std::abs(overlap1) < minOverlap) {
				minOverlap = std::abs(overlap1);
				minOverlapAxis = i;
				pushBack = (i == 0) ? KuroEngine::Vec3<float>{overlap1, 0.0f, 0.0f} :
					(i == 1) ? KuroEngine::Vec3<float>{0.0f, overlap1, 0.0f} :
					KuroEngine::Vec3<float>{ 0.0f, 0.0f, overlap1 };
			}
		}
		else {
			if (std::abs(overlap2) < minOverlap) {
				minOverlap = std::abs(overlap2);
				minOverlapAxis = i;
				pushBack = (i == 0) ? KuroEngine::Vec3<float>{-overlap2, 0.0f, 0.0f} :
					(i == 1) ? KuroEngine::Vec3<float>{0.0f, -overlap2, 0.0f} :
					KuroEngine::Vec3<float>{ 0.0f, 0.0f, -overlap2 };
			}
		}
	}

	return CollisionInfo{ pushBack };
}

GoalPoint::GoalPoint(std::weak_ptr<KuroEngine::Model> arg_model, KuroEngine::Transform arg_initTransform)
	:StageParts(GOAL_POINT, arg_model, arg_initTransform)
{
	m_saplingModel = KuroEngine::Importer::Instance()->LoadModel("resource/user/model/stage/", "Goal.glb");
	m_woodModel = KuroEngine::Importer::Instance()->LoadModel("resource/user/model/stage/", "Goal_Wood.glb");
}

void GoalPoint::Update(Player &arg_player)
{
	using namespace KuroEngine;

	const float HIT_RADIUS = 10.0f;

	//�v���C���[�Ƃ̓����蔻��
	float dist = arg_player.GetTransform().GetPosWorld().Distance(m_transform.GetPosWorld() + (-m_transform.GetUpWorld() * HIT_RADIUS));
	if (!m_hitPlayer)
	{
		m_hitPlayer = (dist < HIT_RADIUS);
	}

	if (m_isGrowUp)
	{
		m_growUpTimer.UpdateTimer(TimeScaleMgr::s_inGame.GetTimeScale());
		m_transform.SetScale(Math::Ease(Out, Elastic, m_growUpTimer.GetTimeRate(), 0.0f, 1.0f));
	}
}

void GoalPoint::Draw(KuroEngine::Camera &arg_cam, KuroEngine::LightManager &arg_ligMgr)
{
	using namespace KuroEngine;

	static const Vec3<float> MODEL_OFFSET = m_transform.GetUpWorld() * -20.0f;

	Transform drawTransform;
	drawTransform.SetPos(m_transform.GetPosWorld() + m_offset.GetPosWorld() + MODEL_OFFSET);
	drawTransform.SetRotate(m_transform.GetRotate());
	drawTransform.SetScale(m_transform.GetScaleWorld() + m_offset.GetScaleWorld());

	BasicDraw::Instance()->Draw(
		arg_cam,
		arg_ligMgr,
		m_model.lock(),
		drawTransform);
}

std::map<std::string, std::weak_ptr<KuroEngine::Model>>Appearance::s_models;

void Appearance::ModelsUvUpdate(float arg_timeScale)
{
	//UV�A�j���[�V����
	for (auto &modelPtr : s_models)
	{
		auto model = modelPtr.second.lock();

		for (auto &mesh : model->m_meshes)
		{
			for (auto &vertex : mesh.mesh->vertices)
			{
				vertex.uv.y += 0.002f * TimeScaleMgr::s_inGame.GetTimeScale();
				//if (1.0f < vertex.uv.y)vertex.uv.y -= 1.0f;
			}
			mesh.mesh->Mapping();
		}
	}
}

Appearance::Appearance(std::weak_ptr<KuroEngine::Model> arg_model, KuroEngine::Transform arg_initTransform, std::weak_ptr<KuroEngine::Model>arg_collisionModel)
	:StageParts(APPEARANCE, arg_model, arg_initTransform)
{
	if (!s_models.contains(arg_model.lock()->m_header.fileName))s_models[arg_model.lock()->m_header.fileName] = arg_model;

	m_collider.BuilCollisionMesh(arg_collisionModel, arg_initTransform);
}

void MoveScaffold::OnInit()
{
	m_isActive = false;
	m_isOldActive = false;
	m_prevOnPlayer = false;
	m_onPlayer = false;
	m_isStop = false;
	m_isOldStop = false;
	m_isOder = true;
	m_nowTranslationIndex = 0;
	m_nextTranslationIndex = 1;
	m_moveDir = KuroEngine::Vec3<float>(m_translationArray[m_nextTranslationIndex] - m_translationArray[m_nowTranslationIndex]).GetNormal();
	m_moveLength = KuroEngine::Vec3<float>(m_translationArray[m_nextTranslationIndex] - m_translationArray[m_nowTranslationIndex]).Length();
	m_nowMoveLength = 0;

	m_moveStartTimer = KuroEngine::Timer(MOVE_START_TIMER);

	m_transform.SetPos(m_translationArray[0]);

	m_oldPos = m_translationArray[0];
	m_nowPos = m_translationArray[0];

	//�����蔻��\�z�B
	ReBuildCollisionMesh();
}

void MoveScaffold::Draw(KuroEngine::Camera &arg_cam, KuroEngine::LightManager &arg_ligMgr)
{

	StageParts::Draw(arg_cam, arg_ligMgr);

	//�ړ��o�H���Ȃ��������΂��B
	if (m_maxTranslation < 0) return;

	//�ړ��o�H��`�悷��B
	for (int index = 1; index <= m_maxTranslation; ++index) {
		KuroEngine::DrawFunc3D::DrawLine(arg_cam, m_translationArray[index - 1], m_translationArray[index], KuroEngine::Color(31, 247, 205, 255), 2.0f);
	}

}

void MoveScaffold::Update(Player &arg_player)
{

	m_moveAmount = {};

	//�t���O��ۑ��B
	m_prevOnPlayer = m_onPlayer;

	if (!m_onPlayer) {
		m_moveStartTimer.Reset();
	}

	//SE�̏���
	if ((!m_isOldActive && m_isActive) || (!m_isOldStop && m_isStop)) {
		SoundConfig::Instance()->Play(SoundConfig::SE_MOVE_SCAFFOLD_START);
	}
	if ((m_isOldActive && !m_isActive) || (m_isOldStop && !m_isStop)) {
		SoundConfig::Instance()->Play(SoundConfig::SE_MOVE_SCAFFOLD_STOP);
	}
	m_isOldActive = m_isActive;
	m_isOldStop = m_isStop;

	//�L��������Ă��Ȃ������珈�����΂��B
	if (!m_isActive) return;

	//�ꎞ��~���������珈�����΂��B
	if (m_isStop) return;

	//���[�g���ݒ肳��Ă��Ȃ��B
	assert(m_maxTranslation != 0);

	//���W��ۑ�
	m_oldPos = m_nowPos;

	//�ړ������ʂ�ۑ��B
	float moveSpeed = MOVE_SPEED * TimeScaleMgr::s_inGame.GetTimeScale();
	m_nowMoveLength += moveSpeed;

	//�ړ������ʂ��K��l�𒴂��Ă�����A�I���������B
	bool isFinish = false;
	if (m_moveLength < m_nowMoveLength) {

		isFinish = true;

		//�I�[�o�[�����������������B
		moveSpeed = m_moveLength - m_nowMoveLength;

	}

	//���̒n�_�֌������ē������B
	m_moveAmount = m_moveDir * moveSpeed;
	m_transform.SetPos(m_transform.GetPos() + m_moveAmount);
	m_nowPos = m_transform.GetPos() + m_moveAmount;

	//�v���C���[���������B
	if (arg_player.GetOnGimmick()) {
		arg_player.SetGimmickVel(m_moveAmount);
	}

	//�v���C���[���W�����v����������A�W�����v�n�_���������B
	if (arg_player.GetIsJump()) {
		arg_player.SetJumpEndPos(arg_player.GetJumpEndPos() + m_moveAmount);
	}

	//���낢��Ə��������Ď����������������߂�B
	if (isFinish) {

		//���Ԃ��ł��Ȃ��悤�ɉ����߂��B
		m_transform.SetPos(m_translationArray[m_nextTranslationIndex]);
		m_nowPos = m_translationArray[m_nextTranslationIndex];

		//���̕����ɐi�ރt���O��������
		if (m_isOder) {

			//����Index��
			m_nowTranslationIndex = m_nextTranslationIndex;
			++m_nextTranslationIndex;
			if (m_maxTranslation < m_nextTranslationIndex) {

				//�I����Ă�����
				m_nextTranslationIndex = m_maxTranslation;
				m_isOder = false;
				m_isActive = false;

			}

		}
		//���̕����ɐi�ރt���O��������
		else {

			//����Index��
			m_nowTranslationIndex = m_nextTranslationIndex;
			--m_nextTranslationIndex;
			if (m_nextTranslationIndex < 0) {

				//�I����Ă�����
				m_nextTranslationIndex = 0;
				m_isOder = true;
				m_isActive = false;

			}

		}

		//�ړ���������Ɨʂ����߂�B
		m_moveDir = KuroEngine::Vec3<float>(m_translationArray[m_nextTranslationIndex] - m_translationArray[m_nowTranslationIndex]).GetNormal();
		m_moveLength = KuroEngine::Vec3<float>(m_translationArray[m_nextTranslationIndex] - m_translationArray[m_nowTranslationIndex]).Length();
		m_nowMoveLength = 0;

		m_moveStartTimer.Reset();


	}

	//�����蔻����č\�z�B
	ReBuildCollisionMesh();
}

void MoveScaffold::PushBack(KuroEngine::Vec3<float> arg_pushBack) {

	//�����߂��B
	m_nowMoveLength -= arg_pushBack.Length();
	m_transform.SetPos(m_transform.GetPos() - m_moveDir * arg_pushBack);

}

void MoveScaffold::ReBuildCollisionMesh() {
	m_collider.BuilCollisionMesh(m_collisionModel, m_transform);
}

void MoveScaffold::OnPlayer() {

	m_onPlayer = true;

	//������g���K�[�̎��Ƀ^�C�}�[���N���B
	if (m_onPlayer && !m_prevOnPlayer) {
		m_moveStartTimer.UpdateTimer();
	}

	//�����ă^�C�}�[�������Ă�����^�C�}�[�𓮂����B(�S�[�����ĂĂ�������܂܂ł����Ɠ���������̂�����邽�߂ɁA������u�Ԃ����^�C�}�[�𓮂����Ȃ��悤�ɂ��Ă���B)
	if (0.0f < m_moveStartTimer.GetTimeRate()) {
		m_moveStartTimer.UpdateTimer();
	}

	if (m_moveStartTimer.IsTimeUp()) {

		//�ꎞ��~����������
		if (m_isStop) {

			//���ݐi��ł���ʂ𔽓]������B
			m_nowMoveLength = m_moveLength - m_nowMoveLength;

			//�ړ����������]
			m_moveDir *= -1.0f;

			//�t���O�ƃC���f�b�N�X�����]�B
			std::swap(m_nowTranslationIndex, m_nextTranslationIndex);
			m_isOder = !m_isOder;

			m_isStop = false;

			//�߂荞��ł���̂ł�����ƈړ�������B
			m_nowMoveLength += KuroEngine::Vec3<float>(m_moveDir * MOVE_SPEED).Length();
			m_transform.SetPos(m_transform.GetPos() + m_moveDir * MOVE_SPEED);

		}
		else {
			//���ʂɗL����
			Activate();
		}

	}

}

const std::string Lever::TURN_ON_ANIM_NAME = "turn_on";
const std::string Lever::TURN_OFF_ANIM_NAME = "turn_off";

Lever::Lever(std::weak_ptr<KuroEngine::Model> arg_model, KuroEngine::Transform arg_initTransform, int arg_id, bool arg_initFlg)
	:StageParts(LEVER, arg_model, arg_initTransform), m_id(arg_id), m_initFlg(arg_initFlg)
{
	auto &anims = arg_model.lock()->m_skelton->animations;
	if (anims.find(TURN_OFF_ANIM_NAME) == anims.end() && anims.find(TURN_ON_ANIM_NAME) == anims.end())
	{

		KuroEngine::AppearMessageBox("Lever �R���X�g���N�^���s", "���f��(" + arg_model.lock()->m_header.fileName + ")�� turn_on �� turn_off �̃A�j���[�V����������ĂȂ���B");
		exit(1);
	}

	m_boxCollider.m_center = arg_initTransform.GetPosWorld();
	m_boxCollider.m_size = arg_initTransform.GetScale();
	m_modelAnimator = std::make_shared<KuroEngine::ModelAnimator>(arg_model);
}

void Lever::OnInit()
{
	m_flg = m_initFlg;
	m_isHit = false;
	m_isOldHit = false;
	m_modelAnimator->SetStartPosture(TURN_ON_ANIM_NAME);
}

void Lever::Update(Player &arg_player)
{
	//�X�C�b�`�̏�Ԃ��Œ肳��Ă���
	if (m_parentSwitch->IsFixed())return;

	//�Փ˃t���O��ۑ��B
	m_isOldHit = m_isHit;
	m_isHit = false;

	//�A���ɐB���Ƃ̓����蔻��
	for (auto &lig : GrowPlantLight::GrowPlantLightArray())
	{
		//���ςŃ��C�g�̖@���ƃv���C���[�̏���W������Ă��疳��������B
		if (DirectX::XM_PIDIV4 < acosf(arg_player.GetTransform().GetUpWorld().Dot(m_transform.GetUpWorld()))) continue;

		if (lig->HitCheckWithBox(m_transform.GetPosWorld(), m_boxCollider.m_size))
		{
			//���o�[����ŃI���I�t�؂�ւ�
			m_isHit = true;
			break;
		}
	}

	//�Փ˂̃g���K�[���肾������t���O��؂�ւ���B
	if (m_isHit && !m_isOldHit) {
		m_flg = !m_flg;

		//�I��
		if (m_flg)
		{
			SoundConfig::Instance()->Play(SoundConfig::SE_LEVER_ON);
			m_modelAnimator->Play(TURN_ON_ANIM_NAME, false, false);
		}
		//�I�t
		else
		{
			SoundConfig::Instance()->Play(SoundConfig::SE_LEVER_OFF);
			m_modelAnimator->Play(TURN_OFF_ANIM_NAME, false, false);
		}
	}

	m_modelAnimator->Update(TimeScaleMgr::s_inGame.GetTimeScale());
}

void Lever::Draw(KuroEngine::Camera &arg_cam, KuroEngine::LightManager &arg_ligMgr)
{
	BasicDraw::Instance()->Draw(
		arg_cam,
		arg_ligMgr,
		m_model.lock(),
		m_transform,
		KuroEngine::AlphaBlendMode_None,
		m_modelAnimator->GetBoneMatBuff());

#ifdef _DEBUG

	KuroEngine::DrawFunc3D::DrawLine(arg_cam, m_transform.GetPosWorld(), m_transform.GetPosWorld() + m_transform.GetUpWorld() * 10.0f, KuroEngine::Color(0.95f, 0.00f, 0.51f, 1.0f), 1.0f);

#endif // _DEBUG

}

void IvyZipLine::Draw(KuroEngine::Camera &arg_cam, KuroEngine::LightManager &arg_ligMgr)
{

	//�ӂ�`��
	const int INDEX = static_cast<int>(m_translationArray.size());
	for (int index = 0; index < INDEX - 1; ++index) {

		KuroEngine::DrawFunc3D::DrawLine(arg_cam, m_translationArray[index], m_translationArray[index + 1], KuroEngine::Color(0.35f, 0.91f, 0.55f, 1.0f), 1.0f);

	}

	for (auto index : m_translationArray) {
		KuroEngine::DrawFunc3D::DrawLine(arg_cam, index, index + KuroEngine::Vec3<float>(1.0f, 1.0f, 1.0f), KuroEngine::Color(0.35f, 0.91f, 0.55f, 1.0f), JUMP_SCALE);
	}

}

void IvyZipLine::CheckHit(bool arg_isHitStartPos) {


	m_isHitStartPos = arg_isHitStartPos;
	m_isActive = true;
	m_isReadyPlayer = false;

	//�W�b�v���C���ړ��ɕK�v�ȕϐ�������
	if (arg_isHitStartPos) {
		m_nowTranslationIndex = 0;
		m_nextTranslationIndex = 1;
		m_moveLength = (m_translationArray[1] - m_translationArray.front()).Length();
		m_moveDir = (m_translationArray[1] - m_translationArray.front()).GetNormal();
	}
	else {
		m_nowTranslationIndex = m_maxTranslation;
		m_nextTranslationIndex = m_maxTranslation - 1;
		m_moveLength = (m_translationArray[m_maxTranslation - 1] - m_translationArray.back()).Length();
		m_moveDir = (m_translationArray[m_maxTranslation - 1] - m_translationArray.back()).GetNormal();
	}
	m_nowMoveLength = 0;

}

KuroEngine::Vec3<float> IvyZipLine::GetPoint(bool arg_isEaseStart) {
	if (arg_isEaseStart) {
		if (m_isHitStartPos) {
			return m_translationArray.front();
		}
		else {
			return m_translationArray.back();
		}
	}
	else {
		if (m_isHitStartPos) {
			return m_translationArray.back();
		}
		else {
			return m_translationArray.front();
		}
	}
}

void IvyZipLine::Update(Player &arg_player)
{

	//�W�b�v���C�����L��������Ă��� ���� �v���C���[�̈ړ��������o���Ă�����v���C���[�𓮂����B
	if (!m_isActive) return;
	if (!m_isReadyPlayer) return;

	//�ړ������ʂ�ۑ��B
	m_nowMoveLength += ZIPLINE_SPEED;

	//�ړ������ʂ��K��l�𒴂��Ă�����A�I���������B
	float moveSpeed = ZIPLINE_SPEED;
	bool isFinish = false;
	if (m_moveLength < m_nowMoveLength) {

		isFinish = true;

		//�I�[�o�[�����������������B
		moveSpeed = m_moveLength - m_nowMoveLength;

	}

	//���̒n�_�֌������ē������B
	arg_player.GetTransform().SetPos(arg_player.GetTransform().GetPos() + m_moveDir * moveSpeed);

	//�v���C���[���������B
	if (arg_player.GetOnGimmick()) {
		arg_player.SetGimmickVel(m_moveDir * moveSpeed);
	}

	//�v���C���[���W�����v����������A�W�����v�n�_���������B
	if (arg_player.GetIsJump()) {
		arg_player.SetJumpEndPos(arg_player.GetJumpEndPos() + m_moveDir * moveSpeed);
	}

	//���낢��Ə��������Ď����������������߂�B
	if (isFinish) {

		//���̕����ɐi�ރt���O��������
		if (m_isHitStartPos) {

			//����Index��
			m_nowTranslationIndex = m_nextTranslationIndex;
			++m_nextTranslationIndex;
			if (m_maxTranslation < m_nextTranslationIndex) {

				//�I����Ă�����
				m_nextTranslationIndex = m_maxTranslation;
				m_isActive = false;
				arg_player.m_collision.FinishGimmickMove();
			}
		}
		//���̕����ɐi�ރt���O��������
		else {

			//����Index��
			m_nowTranslationIndex = m_nextTranslationIndex;
			--m_nextTranslationIndex;
			if (m_nextTranslationIndex < 0) {

				//�I����Ă�����
				m_nextTranslationIndex = 0;
				m_isActive = false;
				arg_player.m_collision.FinishGimmickMove();

			}

		}

		//�ړ���������Ɨʂ����߂�B
		m_moveDir = KuroEngine::Vec3<float>(m_translationArray[m_nextTranslationIndex] - m_translationArray[m_nowTranslationIndex]).GetNormal();
		m_moveLength = KuroEngine::Vec3<float>(m_translationArray[m_nextTranslationIndex] - m_translationArray[m_nowTranslationIndex]).Length();
		m_nowMoveLength = 0;

	}

}

IvyBlock::IvyBlock(std::weak_ptr<KuroEngine::Model> arg_model, KuroEngine::Transform arg_initTransform, KuroEngine::Vec3<float> arg_leftTopFront, KuroEngine::Vec3<float> arg_rightBottomBack, std::weak_ptr<KuroEngine::Model>arg_collisionModel)
	:StageParts(IVY_BLOCK, arg_model, arg_initTransform), m_leftTopFront(arg_leftTopFront), m_rightBottomBack(arg_rightBottomBack)
{
	m_nonExistModel = std::shared_ptr<KuroEngine::Model>(new KuroEngine::Model(*arg_model.lock()));
	m_nonExistMaterial = std::shared_ptr<KuroEngine::Material>(new KuroEngine::Material(*arg_model.lock()->m_meshes[0].material));
	m_invisibleIvyBlockTex = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/invisibleIvyBlock.png");
	m_invisibleIvyBlockReadyTex = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/invisibleIvyBlock_Ready.png");
	m_nonExistMaterial->texBuff[KuroEngine::MATERIAL_TEX_TYPE::COLOR_TEX] = m_invisibleIvyBlockReadyTex;
	for (auto &mesh : m_nonExistModel->m_meshes)
	{
		mesh.material = m_nonExistMaterial;
	}
	m_collisionModel = arg_collisionModel;
	OnInit();
}

void IvyBlock::Update(Player &arg_player)
{

	m_prevOnPlayer = m_onPlayer;

	//�C�[�W���O�^�C�}�[���X�V�B
	m_easingTimer = std::clamp(m_easingTimer + EASING_TIMER * TimeScaleMgr::s_inGame.GetTimeScale(), 0.0f, 1.0f);

	if (m_easingTimer < 1.0f) {

		ReuilCollisionMesh();

	}

	//�o������������B
	if (m_isAppear) {

		float easingValue = m_easingTimer;
		easingValue = KuroEngine::Math::Ease(KuroEngine::Out, KuroEngine::Back, easingValue, 0.0f, 1.0f);

		//�X�P�[�����O
		auto scale = SCALE_DEF * easingValue;
		m_transform.SetScale(scale);

		m_nonExistDrawParam.m_alpha = easingValue;
	}
	else {

		float easingValue = m_easingTimer;
		easingValue = KuroEngine::Math::Ease(KuroEngine::In, KuroEngine::Back, easingValue, 0.0f, 1.0f);

		//�X�P�[�����O
		auto scale = SCALE_DEF - SCALE_DEF * easingValue;
		m_transform.SetScale(scale);

		m_nonExistDrawParam.m_alpha = easingValue;
	}

}

void IvyBlock::Draw(KuroEngine::Camera &arg_cam, KuroEngine::LightManager &arg_ligMgr)
{
	BasicDraw::Instance()->Draw(
		arg_cam,
		arg_ligMgr,
		m_model.lock(),
		m_transform);

	if (m_onPlayer) {
		m_nonExistMaterial->texBuff[KuroEngine::MATERIAL_TEX_TYPE::COLOR_TEX] = m_invisibleIvyBlockReadyTex;
	}
	else {
		m_nonExistMaterial->texBuff[KuroEngine::MATERIAL_TEX_TYPE::COLOR_TEX] = m_invisibleIvyBlockTex;
	}
	for (auto &mesh : m_nonExistModel->m_meshes)
	{
		mesh.material = m_nonExistMaterial;
	}

	KuroEngine::Transform invisibleTrans = m_transform;
	invisibleTrans.SetScale(SCALE_DEF);
	BasicDraw::Instance()->Draw(
		arg_cam,
		arg_ligMgr,
		m_nonExistModel,
		invisibleTrans,
		m_nonExistDrawParam,
		KuroEngine::AlphaBlendMode_Trans,
		nullptr,
		1);
}

void IvyBlock::Appear()
{

	m_isAppear = true;
	m_easingTimer = 0;

}

void IvyBlock::Disappear()
{
	m_onPlayer = false;
	m_isAppear = false;
	m_easingTimer = 0;
}

void SplatoonFence::Draw(KuroEngine::Camera &arg_cam, KuroEngine::LightManager &arg_ligMgr)
{
	StageParts::Draw(arg_cam, arg_ligMgr);
}

void Terrian::Draw(KuroEngine::Camera &arg_cam, KuroEngine::LightManager &arg_ligMgr)
{

	IndividualDrawParameter param = IndividualDrawParameter::GetDefault();

	BasicDraw::Instance()->Draw_Stage(
		arg_cam,
		arg_ligMgr,
		m_model.lock(),
		m_transform,
		IndividualDrawParameter::GetDefault());

}

std::array<std::shared_ptr<KuroEngine::TextureBuffer>, Gate::GATE_TEX_ARRAY_SIZE>Gate::s_texArray;

Gate::Gate(std::weak_ptr<KuroEngine::Model> arg_model, KuroEngine::Transform arg_initTransform, int arg_id, int arg_destStageNum, int arg_destGateId, std::shared_ptr<GuideInsect::CheckPointData>checkPointData)
	:StageParts(GATE, arg_model, arg_initTransform), m_id(arg_id), m_destStageNum(arg_destStageNum), m_destGateId(arg_destGateId), m_guideData(checkPointData)
{
	//�e�N�X�`���ǂݍ���
	if (!s_texArray[0])
	{
		KuroEngine::D3D12App::Instance()->GenerateTextureBuffer(
			s_texArray.data(), "resource/user/tex/stage/gate.png", GATE_TEX_ARRAY_SIZE, { GATE_TEX_ARRAY_SIZE,1 });
	}
	m_guideData->m_pos = m_transform.GetPosWorld();

	static const float TEX_ANIM_INTERVAL = 4.0f;
	m_animTimer.Reset(TEX_ANIM_INTERVAL);
}

void Gate::Update(Player &arg_player)
{
	using namespace KuroEngine;
	static const float HIT_RADIUS = 8.0f;
	static const float HIT_SPHERE_OFFSET_Y = -4.0f;
	static const float EFFECT_INTERVAL = 60.0f;
	static const float EFFECT_SCALE_OFFSET = 0.1f;

	if (IsExit())return;

	const float timeScale = TimeScaleMgr::s_inGame.GetTimeScale();

	float dist = (m_transform.GetPosWorld() + m_transform.GetUpWorld() * HIT_SPHERE_OFFSET_Y).Distance(arg_player.GetNowPos());
	bool enter = dist < HIT_RADIUS *m_transform.GetScaleWorld().x;
	GateManager::Instance()->SetEnter(enter, m_destStageNum, m_destGateId);

	//�e�N�X�`���A�j���[�V����
	if (m_animTimer.UpdateTimer(timeScale))
	{
		//���[�v
		if (GATE_TEX_ARRAY_SIZE <= ++m_texIdx)m_texIdx = 0;
		m_animTimer.Reset();
	}

	m_effectSinCurveAngle += Angle::ROUND() / EFFECT_INTERVAL;
	m_effectScale = 1.0f + powf(sin(m_effectSinCurveAngle), 2.0f) * EFFECT_SCALE_OFFSET;
}

void Gate::Draw(KuroEngine::Camera &arg_cam, KuroEngine::LightManager &arg_ligMgr)
{
	static const float SIZE_HALF = 10.0f;
	static const float DRAW_POS_OFFSET = -5.0f;

	static const KuroEngine::Vec2<float>PLANE_SIZE = { SIZE_HALF * 2.0f,SIZE_HALF * 2.0f };

	if (IsExit())return;

	BasicDraw::Instance()->DrawBillBoard(arg_cam,
		m_transform.GetPosWorld() + m_transform.GetUpWorld() * DRAW_POS_OFFSET,
		PLANE_SIZE * m_effectScale,
		PLANE_SIZE * m_effectScale,
		s_texArray[m_texIdx]);
}

bool Gate::CheckID(int arg_id)
{
	return m_id == arg_id;
}

std::shared_ptr<CheckPointUI>CheckPoint::s_ui;
KuroEngine::Transform CheckPoint::s_latestVisitTransform;
bool CheckPoint::s_visit = false;

CheckPoint::CheckPoint(std::weak_ptr<KuroEngine::Model> arg_model, KuroEngine::Transform arg_initTransform, int arg_order, std::shared_ptr<GuideInsect::CheckPointData>checkPointData)
	:StageParts(CHECK_POINT, arg_model, arg_initTransform), m_order(arg_order), m_guideData(checkPointData), m_fireWork(GPUParticleRender::Instance()->GetStackBuffer())
{
	//UI�������Ȃ琶��
	if (!s_ui)s_ui = std::make_shared<CheckPointUI>();
	KuroEngine::Vec3<float>vec(m_transform.GetUpWorld());

	if (1.0f <= abs(vec.x))
	{
		vec.x *= 10.0f;
	}
	if (1.0f <= abs(vec.y))
	{
		vec.y *= 10.0f;
	}
	if (1.0f <= abs(vec.z))
	{
		vec.z *= 10.0f;
	}

	m_bloomingFlowerModel = KuroEngine::Importer::Instance()->LoadModel("resource/user/model/Stage/", "CheckPoint_Unlocked.glb");
	m_guideData->m_pos = m_transform.GetPosWorld() + vec;
}

void CheckPoint::OnInit()
{
}

void CheckPoint::Update(Player &arg_player)
{
	static const float CHECK_POINT_RADIUS = 13.0f;

	m_fireWork.Update();

	//�N���ςȂ���ɉ������Ȃ�
	if (m_touched)return;

	//�~�̏Փ�
	bool isHit = arg_player.GetTransform().GetPosWorld().DistanceSq(m_transform.GetPosWorld()) < (CHECK_POINT_RADIUS * CHECK_POINT_RADIUS);
	//�Փ˂����u��
	if (!m_touched && isHit)
	{
		m_guideData->m_isHitFlag = true;
		//UI�o��
		s_ui->Start();
		//�Ō�ɖK�ꂽ�`�F�b�N�|�C���g�̃g�����X�t�H�[�����L�^
		s_latestVisitTransform = m_transform;
		s_visit = true;
		CheckPointHitFlag::Instance()->m_isHitCheckPointTrigger = true;
		SaveDataManager::Instance()->SaveCheckPointOrder(m_order);

		//SE�Đ�
		SoundConfig::Instance()->Play(SoundConfig::SE_UNLOCK_CHECK_POINT);

		m_fireWork.Init(m_transform.GetPosWorld());
	}

	m_touched = isHit;
	m_guideData->m_isHitFlag = isHit;
}

void CheckPoint::Draw(KuroEngine::Camera &arg_cam, KuroEngine::LightManager &arg_ligMgr)
{
	if (m_touched) {
		BasicDraw::Instance()->Draw_NoGrass(
			arg_cam,
			arg_ligMgr,
			m_bloomingFlowerModel,
			m_transform,
			IndividualDrawParameter::GetDefault());
	}
	else {
		BasicDraw::Instance()->Draw(
			arg_cam,
			arg_ligMgr,
			m_model.lock(),
			m_transform,
			IndividualDrawParameter::GetDefault());
	}
}

int StarCoin::s_id = 0;
int StarCoin::GET_SUM = 0;

StarCoin::StarCoin(std::weak_ptr<KuroEngine::Model> arg_model, KuroEngine::Transform arg_initTransform)
	:StageParts(CHECK_POINT, arg_model, arg_initTransform), m_id(s_id++)
{
	m_getModel = KuroEngine::Importer::Instance()->LoadModel("resource/user/model/stage/", "StarCoin_Get.glb");
}

void StarCoin::OnInit()
{
	m_touched = false;
	m_get = SaveDataManager::Instance()->IsGetStarCoin(StageManager::Instance()->GetNowStageIdx(), m_id);
	m_basePos = m_transform.GetPos();
	m_angle = 0.0f;
	m_isFinishEffectFlag = false;
	m_scale = m_transform.GetScale().x;
}

void StarCoin::Update(Player &arg_player)
{
	const float STAR_COIN_RADIUS = 10.0f;
	const float STAR_COIN_OFFSET_Y = -5.0f;

	//�E��ꂽ�Ȃ牽�����Ȃ�
	if (m_isFinishEffectFlag)return;

	//�v���C���[�Ƃ̓����蔻��
	KuroEngine::Vec3<float>playerPos = arg_player.GetTransform().GetPosWorld();
	KuroEngine::Vec3<float>myPos = m_transform.GetPosWorld() + m_transform.GetUpWorld() * STAR_COIN_OFFSET_Y;
	bool isHit = playerPos.Distance(myPos) < STAR_COIN_RADIUS;

	//�Փ˂����u��
	if (!m_touched && isHit)
	{
		//���߂ē���
		if (!m_get)
		{
			GET_SUM++;
			SaveDataManager::Instance()->SaveStarCoin(StageManager::Instance()->GetNowStageIdx(), m_id);
		}
		//SE�Đ�
		SoundConfig::Instance()->Play(SoundConfig::SE_GET_STAR_COIN);

		m_basePos = m_transform.GetPos();
		m_baseAngle = m_angle;
		m_timer.Reset(60 * 1.5f);
		m_disappearTimer.Reset(30);
		m_touched = true;
	}

	//�ҋ@��
	if (!m_touched)
	{
		m_angle += 3.0f;

		KuroEngine::Vec3<float>pos(m_basePos);
		pos.y = m_basePos.y + sinf(KuroEngine::Angle::ConvertToRadian(m_angle)) * 2.0f;
		m_transform.SetPos(pos);
	}
	//�G�ꂽ
	else
	{
		m_angle = m_baseAngle + KuroEngine::Math::Ease(KuroEngine::Out, KuroEngine::Cubic, m_timer.GetTimeRate(), 0.0f, 360.0f * 5);

		DirectX::XMVECTOR axis = { 0.0f,1.0f,0.0f,0.0f };
		KuroEngine::Quaternion rotation = DirectX::XMQuaternionRotationAxis(axis, KuroEngine::Angle::ConvertToRadian(m_angle));

		KuroEngine::Vec3<float>pos(m_basePos);
		pos.y = m_basePos.y + KuroEngine::Math::Ease(KuroEngine::Out, KuroEngine::Cubic, m_timer.GetTimeRate(), 0.0f, 10.0f);
		m_transform.SetPos(pos);
		m_transform.SetRotate(rotation);

		if (m_timer.UpdateTimer())
		{
			//���ŏ���
			m_transform.SetScale(
				m_scale - KuroEngine::Math::Ease(KuroEngine::In, KuroEngine::Back, m_disappearTimer.GetTimeRate(), 0.0f, m_scale)
			);
			//���ł�����`���؂�
			if (m_disappearTimer.UpdateTimer())
			{
				m_isFinishEffectFlag = true;
			}
		}
	}


	//m_touched = isHit;
	if (isHit)m_get = true;
}

void StarCoin::Draw(KuroEngine::Camera &arg_cam, KuroEngine::LightManager &arg_ligMgr)
{
	//�E��ꂽ��`�悵�Ȃ�
	if (m_isFinishEffectFlag)return;

	//���肵�����Ƃ��Ȃ��Ȃ�ʏ�`��
	if (!m_get)
	{
		BasicDraw::Instance()->Draw(
			arg_cam,
			arg_ligMgr,
			m_model.lock(),
			m_transform);
		StageParts::Draw(arg_cam, arg_ligMgr);
	}
	//���肵�����Ƃ�����Ȃ甼����
	else
	{
		auto drawTransform = m_transform;
		drawTransform.SetRotate(XMQuaternionIdentity());

		static IndividualDrawParameter halfAlphaParam;
		halfAlphaParam.m_alpha = 0.5f;
		BasicDraw::Instance()->Draw(
			arg_cam,
			arg_ligMgr,
			m_getModel,
			m_transform,
			halfAlphaParam);
	}
}

BackGround::BackGround(std::weak_ptr<KuroEngine::Model> arg_model, KuroEngine::Transform arg_initTransform, bool arg_shadowShader)
	:StageParts(BACKGROUND, arg_model, arg_initTransform), m_shadowShader(arg_shadowShader)
{
	m_backGroundObjectTexBuffer = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/BackGroundObjectTexture.png");
}

void BackGround::Draw(KuroEngine::Camera &arg_cam, KuroEngine::LightManager &arg_ligMgr)
{

	if (m_shadowShader)
	{
		BasicDraw::Instance()->Draw_BackGround(
			m_backGroundObjectTexBuffer,
			arg_cam,
			arg_ligMgr,
			m_model,
			m_transform,
			IndividualDrawParameter::GetDefault());
	}
	else
	{
		BasicDraw::Instance()->Draw(
			arg_cam,
			arg_ligMgr,
			m_model,
			m_transform,
			IndividualDrawParameter::GetDefault());
	}
}

std::array<std::shared_ptr<KuroEngine::Model>,Kinoko::KINOKO_PATTERN_NUM>Kinoko::s_kinokoModel;
std::array<std::shared_ptr<KuroEngine::Model>,Kinoko::KINOKO_PATTERN_NUM>Kinoko::s_brightKinokoModel;

void Kinoko::OnInit()
{
	m_isHit = false;
	m_patternIdx = std::clamp(KuroEngine::GetRand(KINOKO_PATTERN_NUM), 0, KINOKO_PATTERN_NUM - 1);
	m_expandTimer.Reset(0.0f);
}

Kinoko::Kinoko(KuroEngine::Transform arg_initTransform)
	:StageParts(KINOKO, s_kinokoModel[0], arg_initTransform)
{
	using namespace KuroEngine;
	if (!s_kinokoModel[0])
	{
		std::string dir = "resource/user/model/stage/kinoko/";

		for (int kinokoIdx = 0; kinokoIdx < KINOKO_PATTERN_NUM; ++kinokoIdx)
		{
			s_kinokoModel[kinokoIdx] = Importer::Instance()->LoadModel(dir, std::to_string(kinokoIdx) + ".glb");
			s_kinokoModel[kinokoIdx]->m_meshes[0].material->constData.lambert.emissive *= 0.2f;
			s_kinokoModel[kinokoIdx]->m_meshes[0].material->Mapping();
			s_brightKinokoModel[kinokoIdx] = Importer::Instance()->LoadModel(dir, "bright_" + std::to_string(kinokoIdx) + ".glb");
			s_brightKinokoModel[kinokoIdx]->m_meshes[0].material->constData.lambert.emissive = { 0.52f,0.98f,1.0f };
			s_brightKinokoModel[kinokoIdx]->m_meshes[0].material->constData.lambert.emissive *= 0.4f;
			s_brightKinokoModel[kinokoIdx]->m_meshes[0].material->Mapping();
		}
	}
	OnInit();
}

void Kinoko::Update(Player& arg_player)
{
	using namespace KuroEngine;

	m_expandTimer.UpdateTimer(TimeScaleMgr::s_inGame.GetTimeScale());

	m_transform.SetScale(KuroEngine::Math::Ease(Out, Elastic, m_expandTimer.GetTimeRate(), m_smallScale, 1.0f));

	static const float HIT_RADIUS = 8.0f;
	bool hit = arg_player.GetTransform().GetPosWorld().DistanceSq(m_transform.GetPosWorld()) < HIT_RADIUS * HIT_RADIUS;
	if (!oldHit && hit)
	{
		m_expandTimer.Reset(30.0f);
		m_smallScale = 0.6f;
		if (!m_isHit)
		{
			m_isHit = true;
			m_smallScale = 0.0f;
		}
	}
	oldHit = hit;
}

void Kinoko::Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)
{
	auto model = m_isHit ? s_brightKinokoModel[m_patternIdx] : s_kinokoModel[m_patternIdx];

	BasicDraw::Instance()->Draw(
		arg_cam,
		arg_ligMgr,
		model,
		m_transform);
}
