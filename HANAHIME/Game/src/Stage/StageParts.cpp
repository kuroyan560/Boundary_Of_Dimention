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

std::array<std::string, StageParts::STAGE_PARTS_TYPE::NUM>StageParts::s_typeKeyOnJson =
{
	"Terrian","Start","Goal","Appearance","MoveScaffold","Lever","Ivy","IvyBlock"
};

const std::string& StageParts::GetTypeKeyOnJson(STAGE_PARTS_TYPE arg_type)
{
	return s_typeKeyOnJson[arg_type];
}

void StageParts::Init()
{
	m_transform.SetPos(m_initializedTransform.GetPos());
	m_transform.SetScale(m_initializedTransform.GetScale());
	m_transform.SetRotate(m_initializedTransform.GetRotate());
	OnInit();
}

void StageParts::Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)
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
		auto& mesh = arg_model.lock()->m_meshes[meshIdx].mesh;

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
		for (auto& index : m_collisionMesh[meshIdx]) {

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
		DirectX::XMMATRIX targetRotMat = DirectX::XMMatrixRotationQuaternion(arg_transform.GetRotate());
		DirectX::XMMATRIX targetWorldMat = DirectX::XMMatrixIdentity();
		targetWorldMat *= DirectX::XMMatrixScaling(arg_transform.GetScale().x, arg_transform.GetScale().y, arg_transform.GetScale().z);
		targetWorldMat *= targetRotMat;
		targetWorldMat.r[3].m128_f32[0] = arg_transform.GetPos().x;
		targetWorldMat.r[3].m128_f32[1] = arg_transform.GetPos().y;
		targetWorldMat.r[3].m128_f32[2] = arg_transform.GetPos().z;
		for (auto& index : m_collisionMesh[meshIdx]) {
			//���_��ϊ�
			index.m_p0.pos = KuroEngine::Math::TransformVec3(index.m_p0.pos, targetWorldMat);
			index.m_p1.pos = KuroEngine::Math::TransformVec3(index.m_p1.pos, targetWorldMat);
			index.m_p2.pos = KuroEngine::Math::TransformVec3(index.m_p2.pos, targetWorldMat);
			//�@������]�s�񕪂����ϊ�
			index.m_p0.normal = KuroEngine::Math::TransformVec3(index.m_p0.normal, targetRotMat);
			index.m_p0.normal.Normalize();
			index.m_p1.normal = KuroEngine::Math::TransformVec3(index.m_p1.normal, targetRotMat);
			index.m_p1.normal.Normalize();
			index.m_p2.normal = KuroEngine::Math::TransformVec3(index.m_p2.normal, targetRotMat);
			index.m_p2.normal.Normalize();
		}
	}

	//��L�ō���������蔻��p�|���S�������ɁA�����蔻��p��BOX�����B
	m_aabb.clear();
	for (auto& stage : m_collisionMesh) {

		//�i�[����f�[�^��ۑ��B
		m_aabb.emplace_back();

		for (auto& index : stage) {

			//�����蔻��BOX������f�[�^
			m_aabb.back().emplace_back(CreateCubeFromPolygon(index.m_p0.pos, index.m_p1.pos, index.m_p2.pos, index.m_p0.normal));

		}

	}

}

AABB TerrianMeshCollider::CreateCubeFromPolygon(const KuroEngine::Vec3<float>& arg_v1, const KuroEngine::Vec3<float>& arg_v2, const KuroEngine::Vec3<float>& arg_v3, const KuroEngine::Vec3<float>& arg_normal) {

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

	for (auto& index : cubeVertices) {
		aabb.m_min.x = std::min(aabb.m_min.x, index.x);
		aabb.m_min.y = std::min(aabb.m_min.y, index.y);
		aabb.m_min.z = std::min(aabb.m_min.z, index.z);
		aabb.m_max.x = std::max(aabb.m_max.x, index.x);
		aabb.m_max.y = std::max(aabb.m_max.y, index.y);
		aabb.m_max.z = std::max(aabb.m_max.z, index.z);
	}

	return aabb;
}

std::optional<AABB::CollisionInfo> AABB::CheckAABBCollision(const AABB& arg_aabb1) {
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

void GoalPoint::Update(Player& arg_player)
{
	static const float HIT_RADIUS = 3.0f;
	static const float HIT_OFFSET = 5.0f;

	//�v���C���[�Ƃ̓����蔻��
	//if (!m_hitPlayer)
	m_hitPlayer = (arg_player.GetTransform().GetPosWorld().Distance(m_transform.GetPosWorld() + -m_transform.GetUp() * HIT_OFFSET * m_transform.GetScale().x) < HIT_RADIUS);
}

std::map<std::string, std::weak_ptr<KuroEngine::Model>>Appearance::s_models;

void Appearance::ModelsUpdate()
{
	//UV�A�j���[�V����
	for (auto& modelPtr : s_models)
	{
		auto model = modelPtr.second.lock();

		for (auto& mesh : model->m_meshes)
		{
			for (auto& vertex : mesh.mesh->vertices)
			{
				vertex.uv.y += 0.002f * TimeScaleMgr::s_inGame.GetTimeScale();
				//if (1.0f < vertex.uv.y)vertex.uv.y -= 1.0f;
			}
			mesh.mesh->Mapping();
		}
	}
}

Appearance::Appearance(std::weak_ptr<KuroEngine::Model> arg_model, KuroEngine::Transform arg_initTransform, StageParts* arg_parent)
	:StageParts(APPEARANCE, arg_model, arg_initTransform, arg_parent)
{
	if (!s_models.contains(arg_model.lock()->m_header.fileName))s_models[arg_model.lock()->m_header.fileName] = arg_model;

	m_collider.BuilCollisionMesh(arg_model, arg_initTransform);
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

	m_transform.SetPos(m_translationArray[0]);

	m_oldPos = m_translationArray[0];
	m_nowPos = m_translationArray[0];

	//�����蔻��\�z�B
	m_collider.BuilCollisionMesh(m_model, m_transform);
}

void MoveScaffold::Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)
{
	StageParts::Draw(arg_cam, arg_ligMgr);

	//�ړ��o�H���Ȃ��������΂��B
	if (m_maxTranslation < 0) return;

	//�ړ��o�H��`�悷��B
	for (int index = 1; index <= m_maxTranslation; ++index) {
		KuroEngine::DrawFunc3D::DrawLine(arg_cam, m_translationArray[index - 1], m_translationArray[index], KuroEngine::Color(255, 255, 255, 255), 0.1f);
	}

}

void MoveScaffold::Update(Player& arg_player)
{

	//�t���O��ۑ��B
	m_prevOnPlayer = m_onPlayer;

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
	m_transform.SetPos(m_transform.GetPos() + m_moveDir * moveSpeed);
	m_nowPos = m_transform.GetPos() + m_moveDir * moveSpeed;

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


	}

	//�����蔻����č\�z�B
	m_collider.BuilCollisionMesh(m_model, m_transform);

}

void MoveScaffold::PushBack(KuroEngine::Vec3<float> arg_pushBack) {

	//�����߂��B
	m_nowMoveLength -= arg_pushBack.Length();
	m_transform.SetPos(m_transform.GetPos() - m_moveDir * arg_pushBack);

}

void MoveScaffold::BuildCollisionMesh() {
	m_collider.BuilCollisionMesh(m_model, m_transform);
}

void MoveScaffold::OnPlayer() {

	m_onPlayer = true;

	//������g���K�[��������L��������B
	if (!m_prevOnPlayer && m_onPlayer) {

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

Lever::Lever(std::weak_ptr<KuroEngine::Model> arg_model, KuroEngine::Transform arg_initTransform, StageParts* arg_parent, int arg_id, bool arg_initFlg)
	:StageParts(LEVER, arg_model, arg_initTransform, arg_parent), m_id(arg_id), m_initFlg(arg_initFlg)
{
	auto& anims = arg_model.lock()->m_skelton->animations;
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

void Lever::Update(Player& arg_player)
{
	//�X�C�b�`�̏�Ԃ��Œ肳��Ă���
	if (m_parentSwitch->IsFixed())return;

	//�Փ˃t���O��ۑ��B
	m_isOldHit = m_isHit;
	m_isHit = false;

	//�A���ɐB���Ƃ̓����蔻��
	for (auto& lig : GrowPlantLight::GrowPlantLightArray())
	{
		if (DirectX::XM_PIDIV4 / 2.0f < acos(arg_player.GetTransform().GetUpWorld().Dot(m_transform.GetUpWorld()))) break;

		//���ςŃ��C�g�̖@���ƃv���C���[�̏���W������Ă��疳��������B
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

void Lever::Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)
{
	BasicDraw::Instance()->Draw(
		arg_cam,
		arg_ligMgr,
		m_model.lock(),
		m_transform,
		KuroEngine::AlphaBlendMode_None,
		m_modelAnimator->GetBoneMatBuff());
}

void IvyZipLine::Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)
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

void IvyZipLine::Update(Player& arg_player)
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

IvyBlock::IvyBlock(std::weak_ptr<KuroEngine::Model> arg_model, KuroEngine::Transform arg_initTransform, StageParts* arg_parent, KuroEngine::Vec3<float> arg_leftTopFront, KuroEngine::Vec3<float> arg_rightBottomBack)
	:StageParts(IVY_BLOCK, arg_model, arg_initTransform, arg_parent), m_leftTopFront(arg_leftTopFront), m_rightBottomBack(arg_rightBottomBack) 
{
	m_collider.BuilCollisionMesh(arg_model, arg_initTransform);
	OnInit();
	m_nonExistModel = std::shared_ptr<KuroEngine::Model>(new KuroEngine::Model(*arg_model.lock()));
	m_nonExistMaterial = std::shared_ptr<KuroEngine::Material>(new KuroEngine::Material(*arg_model.lock()->m_meshes[0].material));
	m_nonExistMaterial->texBuff[KuroEngine::MATERIAL_TEX_TYPE::COLOR_TEX] = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/invisibleIvyBlock.png");
	for (auto& mesh : m_nonExistModel->m_meshes)
	{
		mesh.material = m_nonExistMaterial;
	}
}

void IvyBlock::Update(Player& arg_player)
{

	m_prevOnPlayer = m_onPlayer;

	//�C�[�W���O�^�C�}�[���X�V�B
	m_easingTimer = std::clamp(m_easingTimer + EASING_TIMER * TimeScaleMgr::s_inGame.GetTimeScale(), 0.0f, 1.0f);


	m_collider.BuilCollisionMesh(m_model, m_transform);

	//�o������������B
	if (m_isAppear) {

		float easingValue = m_easingTimer;
		easingValue = KuroEngine::Math::Ease(KuroEngine::Out, KuroEngine::Back, easingValue, 0.0f, 1.0f);

		//�X�P�[�����O
		auto scale = HIT_SCALE_MIN * easingValue;
		m_transform.SetScale(scale);

		m_nonExistDrawParam.m_alpha = easingValue;
	}
	else {

		float easingValue = m_easingTimer;
		easingValue = KuroEngine::Math::Ease(KuroEngine::In, KuroEngine::Back, easingValue, 0.0f, 1.0f);

		//�X�P�[�����O
		auto scale = HIT_SCALE_MIN - HIT_SCALE_MIN * easingValue;
		m_transform.SetScale(scale);

		m_nonExistDrawParam.m_alpha = easingValue;
	}

}

void IvyBlock::Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)
{
	BasicDraw::Instance()->Draw(
		arg_cam,
		arg_ligMgr,
		m_model.lock(),
		m_transform);
	
	KuroEngine::Transform invisibleTrans = m_transform;
	invisibleTrans.SetScale(HIT_SCALE_MIN);
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


