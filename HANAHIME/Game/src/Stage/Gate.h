#pragma once
#include"Common/Transform.h"
#include"ForUser/JsonData.h"
#include"Render/RenderObject/Light.h"
#include"../Graphics/BasicDraw.h"
#include"FrameWork/Importer.h"

/// <summary>
/// ���̏��������
/// </summary>
struct GateData
{
	GateData(KuroEngine::Transform transform = {}, int stage_num = -1) :
		m_transform(transform), m_stageNum(stage_num)
	{};

	KuroEngine::Transform m_transform;
	int m_stageNum;
};

class ConvertModelToGateData
{
public:
	static std::vector<GateData>Convert(std::string arg_dir, std::string arg_fileName, float terrian_Scale)
	{
		std::vector<GateData> gateDataArray;

		KuroEngine::JsonData jsonData(arg_dir, arg_fileName);
		for (auto &obj : jsonData.m_jsonData["objects"])
		{
			//�n�`�̖��O�̃p�����[�^���Ȃ�
			if (!obj.contains("name"))continue;
			//���f���̖��O�̃p�����[�^���Ȃ�
			if (!obj.contains("file_name"))continue;
			//�g�����X�t�H�[���̃p�����[�^���Ȃ�
			if (!obj.contains("transform"))continue;

			//�n�`�̖��O�ݒ�
			auto name = obj["name"].get<std::string>();

			if (name.find("Gate") == std::string::npos)continue;


			//�g�����X�t�H�[���擾
			auto transformObj = obj["transform"];

			//���s�ړ�
			KuroEngine::Vec3<float>translation = { -(float)transformObj["translation"][0] * terrian_Scale,(float)transformObj["translation"][2] * terrian_Scale + 25.0f,(float)transformObj["translation"][1] * terrian_Scale };

			//��]
			KuroEngine::Vec3<float>rotate = { -(float)transformObj["rotation"][1],-(float)transformObj["rotation"][2], (float)transformObj["rotation"][0] };
			//���W�A���ɒ���
			rotate.x = KuroEngine::Angle::ConvertToRadian(rotate.x);
			rotate.y = KuroEngine::Angle::ConvertToRadian(rotate.y);
			rotate.z = KuroEngine::Angle::ConvertToRadian(rotate.z);

			//�X�P�[�����O
			KuroEngine::Vec3<float>scaling = { (float)transformObj["scaling"][0],(float)transformObj["scaling"][2] ,(float)transformObj["scaling"][1] };

			//�g�����X�t�H�[���ݒ�
			KuroEngine::Transform transform;
			transform.SetPos(translation);
			transform.SetRotate(XMQuaternionRotationRollPitchYaw(rotate.z, rotate.y, rotate.x));
			transform.SetScale(scaling);

			gateDataArray.emplace_back(transform, 0);
		}
		return gateDataArray;
	};
};

/// <summary>
/// �ʂ̃X�e�[�W�ړ�����p�̔�
/// </summary>
class Gate
{
public:
	/// <summary>
	/// ������
	/// </summary>
	/// <param name="transform">���W�A�傫���A�p�x</param>
	/// <param name="stage_num">�ړ�����X�e�[�W�ԍ�</param>
	Gate(const KuroEngine::Transform &transform, int stage_num);
	void Update();

	/// <summary>
	/// ���Ƃ̔���
	/// </summary>
	/// <param name="player_pos">�v���C���[�̍��W</param>
	/// <returns>true...���ƐڐG,false...�ڐG���Ă��Ȃ�</returns>
	bool IsHit(const KuroEngine::Vec3<float> &player_pos);

	int GetStageNum() { return m_stageNum; };

	KuroEngine::Vec3<float>ForceVel();

	/// <summary>
	/// �����蔻�����
	/// </summary>
	void DebugDraw(KuroEngine::Camera &arg_cam, KuroEngine::LightManager &arg_ligMgr);

private:
	std::shared_ptr<KuroEngine::Model>m_model;
	KuroEngine::Transform m_transform;
	int m_stageNum;
};

