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
			//地形の名前のパラメータがない
			if (!obj.contains("name"))continue;
			//モデルの名前のパラメータがない
			if (!obj.contains("file_name"))continue;
			//トランスフォームのパラメータがない
			if (!obj.contains("transform"))continue;

			//地形の名前設定
			auto name = obj["name"].get<std::string>();

			if (name.find(mesh_name) == std::string::npos)continue;


			//トランスフォーム取得
			auto transformObj = obj["transform"];

			//平行移動
			KuroEngine::Vec3<float>translation = { -(float)transformObj["translation"][0] * terrian_Scale,(float)transformObj["translation"][2] * terrian_Scale + 25.0f,(float)transformObj["translation"][1] * terrian_Scale };

			//回転
			KuroEngine::Vec3<float>rotate = { -(float)transformObj["rotation"][1],-(float)transformObj["rotation"][2], (float)transformObj["rotation"][0] };
			//ラジアンに直す
			rotate.x = KuroEngine::Angle::ConvertToRadian(rotate.x);
			rotate.y = KuroEngine::Angle::ConvertToRadian(rotate.y);
			rotate.z = KuroEngine::Angle::ConvertToRadian(rotate.z);

			//スケーリング
			KuroEngine::Vec3<float>scaling = { (float)transformObj["scaling"][0],(float)transformObj["scaling"][2] ,(float)transformObj["scaling"][1] };

			//トランスフォーム設定
			KuroEngine::Transform transform;
			transform.SetPos(translation);
			transform.SetRotate(DirectX::XMQuaternionRotationRollPitchYaw(rotate.z, rotate.y, rotate.x));
			transform.SetScale(scaling);

			gateDataArray.emplace_back(transform);
		}
		return gateDataArray;
	};
};
