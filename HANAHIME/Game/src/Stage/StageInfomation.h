#pragma once
#include"Common/Transform.h"
#include"ForUser/JsonData.h"
#include<vector>

class ConvertModelToGateData
{
public:
	static std::vector<KuroEngine::Transform>GetMeshPos(std::string arg_dir, std::string arg_fileName, std::string mesh_name, float terrian_Scale)
	{
		std::vector<KuroEngine::Transform> gateDataArray;

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

			if (name.find(mesh_name) == std::string::npos)continue;


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
			transform.SetRotate(DirectX::XMQuaternionRotationRollPitchYaw(rotate.z, rotate.y, rotate.x));
			transform.SetScale(scaling);

			gateDataArray.emplace_back(transform);
		}
		return gateDataArray;
	};
};
