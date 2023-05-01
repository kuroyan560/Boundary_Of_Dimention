#include"CameraData.h"


bool CheckJsonKeyExist(std::string arg_fileName, nlohmann::json arg_json, std::string arg_key)
{
	bool exist = arg_json.contains(arg_key);
	if (!exist)
	{
		KuroEngine::AppearMessageBox("CheckJsonKeyExist() 失敗", arg_fileName + " に\"" + arg_key + "\"が含まれてないよ。");
	}
	return exist;
}

//左手座標系
KuroEngine::Vec3<float> GetConsiderCoordinate(nlohmann::json arg_json)
{
	//return KuroEngine::Vec3<float>((float)arg_json[0], (float)arg_json[1], (float)arg_json[2]);
	return KuroEngine::Vec3<float>(-(float)arg_json[0], (float)arg_json[2], -(float)arg_json[1]);
}

//右手座標(角度)
KuroEngine::Vec3<float> GetConsiderCoordinate2(nlohmann::json arg_json)
{
	//return KuroEngine::Vec3<float>((float)arg_json[0], (float)arg_json[1], (float)arg_json[2]);
	return KuroEngine::Vec3<float>(-(float)arg_json[1], (float)arg_json[2], (float)arg_json[0]);
}

// pos, pos_, filename ...
bool LoadArray(const char *keyA, const char *key2, std::string arg_fileName, std::vector<KuroEngine::Vec3<float>> *arg_result, nlohmann::json arg_json)
{
	using namespace KuroEngine;

	if (!CheckJsonKeyExist(arg_fileName, arg_json, keyA))return false;

	nlohmann::json jsonArray = arg_json[keyA];

	std::vector<Vec3<float>>translationArray;
	int idx = 0;
	std::string key = key2 + std::to_string(idx);

	while (jsonArray.contains(key))
	{
		translationArray.emplace_back();
		//平行移動
		translationArray.back() = GetConsiderCoordinate(jsonArray[key]);
		//translationArray.back() *= m_terrianScaling;

		key = key2 + std::to_string(++idx);
	}

	*arg_result = translationArray;

	return true;
}


template<typename T>
bool LoadArray2(const char *keyA, const char *key2, std::string arg_fileName, std::vector<T> *arg_result, nlohmann::json arg_json)
{
	using namespace KuroEngine;

	if (!CheckJsonKeyExist(arg_fileName, arg_json, keyA))return false;

	nlohmann::json jsonArray = arg_json[keyA];

	std::vector<T>translationArray;
	int idx = 0;
	std::string key = key2 + std::to_string(idx);

	while (jsonArray.contains(key))
	{
		translationArray.emplace_back();
		//平行移動
		translationArray.back() = T(jsonArray[key]);

		key = key2 + std::to_string(++idx);
	}

	*arg_result = translationArray;

	return true;
}

bool LoadArray3(const char *keyA, const char *key2, std::string arg_fileName, std::vector<KuroEngine::Vec3<float>> *arg_result, nlohmann::json arg_json)
{
	using namespace KuroEngine;

	if (!CheckJsonKeyExist(arg_fileName, arg_json, keyA))return false;

	nlohmann::json jsonArray = arg_json[keyA];

	std::vector<Vec3<float>>translationArray;
	int idx = 0;
	std::string key = key2 + std::to_string(idx);

	while (jsonArray.contains(key))
	{
		translationArray.emplace_back();
		//平行移動
		translationArray.back() = GetConsiderCoordinate2(jsonArray[key]);
		//translationArray.back() *= m_terrianScaling;

		key = key2 + std::to_string(++idx);
	}

	*arg_result = translationArray;

	return true;
}



std::vector<MovieCameraData> LoadWithType(std::string arg_fileName, nlohmann::json arg_json)
{
	using namespace KuroEngine;

	auto &obj = arg_json;

	//共通パラメータ
	//種別
	auto typeKey = obj["type"].get<std::string>();


	//種別に応じて変わるパラメータ
		//通常の地形
	if (typeKey == "NONE_TYPE")
	{
		// Pos
		std::vector<KuroEngine::Vec3<float>> PosArray;
		LoadArray("pos", "pos_", arg_fileName, &PosArray, obj);
		// Angle
		std::vector<KuroEngine::Vec3<float>> AngleArray;
		LoadArray3("angle", "angle_", arg_fileName, &AngleArray, obj);
		// Sec
		std::vector<float> SecArray;
		LoadArray2("sec", "sec_", arg_fileName, &SecArray, obj);
		// Sec
		std::vector<float> WaitArray;
		LoadArray2("wait", "wait_", arg_fileName, &WaitArray, obj);
		// poscomp
		std::vector<int> PoscompChangeArray;
		LoadArray2("posChangeComp", "posChangeComp_", arg_fileName, &PoscompChangeArray, obj);
		std::vector<int> PoscompEaseArray;
		LoadArray2("posEaseComp", "posEaseComp_", arg_fileName, &PoscompEaseArray, obj);
		// anglecomp
		std::vector<int> AnglecompChangeArray;
		LoadArray2("angleChangeComp", "angleChangeComp_", arg_fileName, &AnglecompChangeArray, obj);
		std::vector<int> AnglecompEaseArray;
		LoadArray2("angleEaseComp", "angleEaseComp_", arg_fileName, &AnglecompEaseArray, obj);
		// Params
		bool ParamsArray = obj["IsLoop"].get<bool>();


		std::vector<MovieCameraData> Data;
		for (int i = 0; i < PosArray.size(); i++) {
			Data.emplace_back();
			Data[i].transform.SetPos(PosArray[i]);
			Data[i].transform.SetRotate(KuroEngine::Angle::ConvertToRadian(AngleArray[i].x), KuroEngine::Angle::ConvertToRadian(AngleArray[i].y), KuroEngine::Angle::ConvertToRadian(AngleArray[i].z));
			Data[i].interpolationTimer = static_cast<int>(SecArray[i]);
			Data[i].afterStopTimer = 0;
			Data[i].preStopTimer = static_cast<int>(WaitArray[i]);

			MovieCameraEaseData hoge_1;
			hoge_1.easeChangeType = KuroEngine::EASE_CHANGE_TYPE(PoscompChangeArray[i]);
			hoge_1.easeType = KuroEngine::EASING_TYPE(PoscompEaseArray[i]);
			Data[i].easePosData = hoge_1;

			MovieCameraEaseData hoge_2;
			hoge_2.easeChangeType = KuroEngine::EASE_CHANGE_TYPE(AnglecompChangeArray[i]);
			hoge_2.easeType = KuroEngine::EASING_TYPE(AnglecompEaseArray[i]);
			Data[i].easeRotaData = hoge_2;
		}
		return Data;
	}
	return std::vector<MovieCameraData>();
}

std::vector<MovieCameraData> Load(std::string arg_dir, std::string arg_fileName, float arg_terrianScaling, bool arg_hasGoal = true) {

	KuroEngine::JsonData jsonData(arg_dir, arg_fileName);

	//正しいファイルではない
	if (!CheckJsonKeyExist(arg_fileName, jsonData.m_jsonData, "stage"))return std::vector<MovieCameraData>();

	auto stageJsonData = jsonData.m_jsonData["stage"];
	for (auto &obj : stageJsonData["objects"])
	{
		return LoadWithType(arg_fileName, obj);
	}
	return std::vector<MovieCameraData>();
}


CameraData::CameraData()
{

}

void CameraData::RegistCameraData(std::string filepass)
{
	std::vector<MovieCameraData>cameraData = Load("resource/user/camera/", "CopyFile.json",5.0f);

}

bool CameraData::CheckJsonKeyExist(std::string arg_fileName, nlohmann::json arg_json, std::string arg_key)
{
	bool exist = arg_json.contains(arg_key);
	if (!exist)
	{
		KuroEngine::AppearMessageBox("Stage : CheckJsonKeyExist() 失敗", arg_fileName + " に\"" + arg_key + "\"が含まれてないよ。");
	}
	return exist;
}
