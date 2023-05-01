#pragma once
#include"Common/Singleton.h"
#include"json.hpp"
#include"ForUser/JsonData.h"
#include"KuroEngine.h"
#include"MovieCamera.h"

class CameraData : public KuroEngine::DesignPattern::Singleton<CameraData>
{
public:
	CameraData();
	void RegistCameraData(std::string filepass);
	void GetCameraData();

private:
	std::vector<std::vector<MovieCameraData>>m_movieCameraDataArray;


	//�L�[��json�t�@�C���Ɋ܂܂�Ă��邩�A�܂܂�Ă��Ȃ�������G���[�ŏI��
	bool CheckJsonKeyExist(std::string arg_fileName, nlohmann::json arg_json, std::string arg_key);
};