#include "Stage.h"
#include"FrameWork/Importer.h"
#include"ForUser/JsonData.h"
#include"FrameWork/Importer.h"
#include"../Graphics/BasicDraw.h"
#include"../../../../src/engine/ForUser/Object/Model.h"

std::string Stage::s_terrianModelDir = "resource/user/model/terrian/";

Stage::Stage()
{
	using namespace KuroEngine;

	//�f�t�H���g�̃��f��
		//�X�J�C�h�[��
	static std::shared_ptr<Model>s_defaultSkydomeModel
		= Importer::Instance()->LoadModel("resource/user/model/", "Skydome.glb");
	//�X�щ~��
	static std::shared_ptr<Model>s_defaultWoodsCylinderModel
		= Importer::Instance()->LoadModel("resource/user/model/", "Woods.glb");

	//�f�t�H���g�̉摜
		//�n��
	static std::shared_ptr<TextureBuffer>s_defaultGroundTex
		= D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/ground.png");

	//�f�t�H���g�l�ݒ�
	m_skydomeModel = s_defaultSkydomeModel;
	m_woodsCylinderModel = s_defaultWoodsCylinderModel;
	m_groundTex = s_defaultGroundTex;
}

void Stage::TerrianInit(float arg_scaling)
{
	for (auto& terrian : m_terrianArray)
	{
		terrian.m_transform.SetPos(terrian.m_initializedTransform.GetPos() * arg_scaling);
		terrian.m_transform.SetScale(terrian.m_initializedTransform.GetScale() * arg_scaling);
		terrian.m_transform.SetRotate(terrian.m_initializedTransform.GetRotate());

		//�����蔻��p�̃��b�V�������f���̃��b�V���ɍ��킹��B
		terrian.m_collisionMesh.resize(static_cast<int>(terrian.m_model.lock()->m_meshes.size()));

		//�����蔻��p���b�V�����쐬�B
		for (auto& index : terrian.m_model.lock()->m_meshes) {

			BuilCollisionMesh(terrian, index, static_cast<int>(&index - &(terrian.m_model.lock()->m_meshes[0])));

		}

	}

}

void Stage::TerrianDraw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)
{
	for (auto& terrian : m_terrianArray)
	{
		BasicDraw::Instance()->Draw(
			arg_cam,
			arg_ligMgr,
			terrian.m_model.lock(),
			terrian.m_transform);
	}
}

void Stage::Load(std::string arg_dir, std::string arg_fileName)
{
	using namespace KuroEngine;

	//�n�`���N���A
	m_terrianArray.clear();

	JsonData jsonData(arg_dir, arg_fileName);
	for (auto& obj : jsonData.m_jsonData["objects"])
	{
		//�n�`�̖��O�̃p�����[�^���Ȃ�
		if (!obj.contains("name"))continue;

		//���f���̖��O�̃p�����[�^���Ȃ�
		if (!obj.contains("file_name"))continue;

		//�g�����X�t�H�[���̃p�����[�^���Ȃ�
		if (!obj.contains("transform"))continue;

		//�n�`�̖��O�ݒ�
		auto name = obj["name"].get<std::string>();

		//���f���ݒ�
		auto model = Importer::Instance()->LoadModel(s_terrianModelDir, obj["file_name"].get<std::string>() + ".glb");

		//�g�����X�t�H�[���擾
		auto transformObj = obj["transform"];

		//���s�ړ�
		Vec3<float>translation = { -(float)transformObj["translation"][0],(float)transformObj["translation"][2],-(float)transformObj["translation"][1] };

		//��]
		Vec3<float>rotate = { -(float)transformObj["rotation"][1],-(float)transformObj["rotation"][2], (float)transformObj["rotation"][0] };
		//���W�A���ɒ���
		rotate.x = Angle::ConvertToRadian(rotate.x);
		rotate.y = Angle::ConvertToRadian(rotate.y);
		rotate.z = Angle::ConvertToRadian(rotate.z);

		//�X�P�[�����O
		Vec3<float>scaling = { (float)transformObj["scaling"][0],(float)transformObj["scaling"][2] ,(float)transformObj["scaling"][1] };

		//�g�����X�t�H�[���ݒ�
		Transform transform;
		transform.SetPos(translation);
		transform.SetRotate(XMQuaternionRotationRollPitchYaw(rotate.z, rotate.y, rotate.x));
		transform.SetScale(scaling);

		//�n�`�ǉ�
		m_terrianArray.emplace_back(name, model, transform);
	}
}

void Stage::BuilCollisionMesh(Terrian& arg_terrian, KuroEngine::ModelMesh& arg_mesh, int arg_meshIndex) {

	/*-- �@ ���f����񂩂瓖���蔻��p�̃|���S�������o�� --*/

	//�����蔻��p�|���S��
	struct Polygon {
		bool m_isActive;					//���̃|���S�����L��������Ă��邩�̃t���O
		KuroEngine::ModelMesh::Vertex m_p0;	//���_0
		KuroEngine::ModelMesh::Vertex m_p1;	//���_1
		KuroEngine::ModelMesh::Vertex m_p2;	//���_2
	};

	//�����蔻��p�|���S���R���e�i���쐬�B
	arg_terrian.m_collisionMesh[arg_meshIndex].resize(arg_mesh.mesh->indices.size() / static_cast<size_t>(3));

	//�����蔻��p�|���S���R���e�i�Ƀf�[�^�����Ă����B
	for (auto& index : arg_terrian.m_collisionMesh[arg_meshIndex]) {

		// ���݂�Index���B
		int nowIndex = static_cast<int>(&index - &arg_terrian.m_collisionMesh[0][0]);
		
		// ���_����ۑ��B
		index.m_p0 = arg_mesh.mesh->vertices[arg_mesh.mesh->indices[nowIndex * 3 + 0]];
		index.m_p1 = arg_mesh.mesh->vertices[arg_mesh.mesh->indices[nowIndex * 3 + 1]];
		index.m_p2 = arg_mesh.mesh->vertices[arg_mesh.mesh->indices[nowIndex * 3 + 2]];

		// �|���S����L�����B
		index.m_isActive = true;

	}


	/*-- �A �|���S�������[���h�ϊ����� --*/

	//���[���h�s��
	DirectX::XMMATRIX targetRotMat = DirectX::XMMatrixRotationQuaternion(arg_terrian.m_transform.GetRotate());
	DirectX::XMMATRIX targetWorldMat = DirectX::XMMatrixIdentity();
	targetWorldMat *= DirectX::XMMatrixScaling(arg_terrian.m_transform.GetScale().x, arg_terrian.m_transform.GetScale().y, arg_terrian.m_transform.GetScale().z);
	targetWorldMat *= targetRotMat;
	targetWorldMat.r[3].m128_f32[0] = arg_terrian.m_transform.GetPos().x;
	targetWorldMat.r[3].m128_f32[1] = arg_terrian.m_transform.GetPos().y;
	targetWorldMat.r[3].m128_f32[2] = arg_terrian.m_transform.GetPos().z;
	for (auto& index : arg_terrian.m_collisionMesh[arg_meshIndex]) {
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
