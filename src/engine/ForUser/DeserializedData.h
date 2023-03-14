#pragma once
#include"json.hpp"
#include<string>
#include<vector>

class DeserializedData
{
public:
	//JSON���f�V���A���C�Y�����f�[�^
	nlohmann::json m_data;

	template<typename T>
	T* GetData(std::initializer_list<std::string>arg_keyArray);
};

template<typename T>
inline T* DeserializedData::GetData(std::initializer_list<std::string> arg_keyArray)
{
	//�f�[�^����
	if (m_data.empty())return;

	//�������̃f�[�^
	nlohmann::json scanData = m_data;

	//�Ō�ɍ~�肽�K�w�̃L�[
	std::string latestKey;

	//�L�[�̔z�񂩂�A�f�[�^�K�w�������Ă���
	for (auto& key : arg_keyArray)
	{
		//�w�肳�ꂽ�L�[��json�I�u�W�F�N�g���Ȃ�
		if (!scanData.contains(key))return;

		//�K�w���~���
		scanData = m_data[key];

		//�K�w�̃L�[���L�^
		latestKey = key;
	}

	return &scanData[key];
}