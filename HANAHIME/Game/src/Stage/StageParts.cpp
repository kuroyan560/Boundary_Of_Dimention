#include"StageParts.h"
#include"ForUser/Object/Model.h"
#include"../Graphics/BasicDraw.h"
#include"../Player/Player.h"
#include"../../../../src/engine/ForUser/DrawFunc/3D/DrawFunc3D.h"

std::array<std::string, StageParts::STAGE_PARTS_TYPE::NUM>StageParts::s_typeKeyOnJson =
{
	"Terrian","Start","Goal","Appearance","MoveScaffold"
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

	OnDraw(arg_cam, arg_ligMgr);
}

void StageParts::BuilCollisionMesh()
{
	//�����蔻��p�̃��b�V�������f���̃��b�V���ɍ��킹��B
	int meshNum = static_cast<int>(m_model.lock()->m_meshes.size());
	m_collisionMesh.resize(meshNum);

	//�����蔻��p���b�V�����쐬�B
	for (int meshIdx = 0; meshIdx < meshNum; ++meshIdx)
	{
		auto& mesh = m_model.lock()->m_meshes[meshIdx].mesh;

		/*-- �@ ���f����񂩂瓖���蔻��p�̃|���S�������o�� --*/

	//�����蔻��p�|���S��
		struct Polygon {
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
		DirectX::XMMATRIX targetRotMat = DirectX::XMMatrixRotationQuaternion(m_transform.GetRotate());
		DirectX::XMMATRIX targetWorldMat = DirectX::XMMatrixIdentity();
		targetWorldMat *= DirectX::XMMatrixScaling(m_transform.GetScale().x, m_transform.GetScale().y, m_transform.GetScale().z);
		targetWorldMat *= targetRotMat;
		targetWorldMat.r[3].m128_f32[0] = m_transform.GetPos().x;
		targetWorldMat.r[3].m128_f32[1] = m_transform.GetPos().y;
		targetWorldMat.r[3].m128_f32[2] = m_transform.GetPos().z;
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
}


void MoveScaffold::OnInit()
{
	m_isActive = false;
	m_isOder = true;
	m_nowTranslationIndex = 0;
	m_nextTranslationIndex = 0;
	m_moveLength = 0;
	m_nowMoveLength = 0;
	m_moveDir = KuroEngine::Vec3<float>();
	
	m_transform.SetPos(m_translationArray[0]);

	//�����蔻����č\�z�B
	BuilCollisionMesh();
}

void MoveScaffold::OnDraw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)
{

	//�ړ��o�H���Ȃ��������΂��B
	if (m_maxTranslation < 0) return;

	//�ړ��o�H��`�悷��B
	for (int index = 1; index <= m_maxTranslation; ++index) {
		KuroEngine::DrawFunc3D::DrawLine(arg_cam, m_translationArray[index - 1], m_translationArray[index], KuroEngine::Color(255, 255, 255, 255), 0.1f);
	}

}

void MoveScaffold::Update(Player& arg_player)
{

	//�L��������Ă��Ȃ������珈�����΂��B
	if (!m_isActive) return;

	//���[�g���ݒ肳��Ă��Ȃ��B
	assert(m_maxTranslation != 0);

	//�ړ������ʂ�ۑ��B
	m_nowMoveLength += MOVE_SPEED;

	//�ړ������ʂ��K��l�𒴂��Ă�����A�I���������B
	float moveSpeed = MOVE_SPEED;
	bool isFinish = false;
	if (m_moveLength < m_nowMoveLength) {

		isFinish = true;

		//�I�[�o�[�����������������B
		moveSpeed = m_moveLength - m_nowMoveLength;

	}

	//���̒n�_�֌������ē������B
	m_transform.SetPos(m_transform.GetPos() + m_moveDir * moveSpeed);

	//�v���C���[���������B
	if (arg_player.GetOnGimmick()) {
		arg_player.SetGimmickVel(m_moveDir * moveSpeed);
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
	BuilCollisionMesh();

}