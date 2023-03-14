#pragma once
#include"json.hpp"
#include<string>
#include<vector>

class DeserializedData
{
public:
	//JSONをデシリアライズしたデータ
	nlohmann::json m_data;

	template<typename T>
	T* GetData(std::initializer_list<std::string>arg_keyArray);
};

template<typename T>
inline T* DeserializedData::GetData(std::initializer_list<std::string> arg_keyArray)
{
	//データが空
	if (m_data.empty())return;

	//走査中のデータ
	nlohmann::json scanData = m_data;

	//最後に降りた階層のキー
	std::string latestKey;

	//キーの配列から、データ階層を下っていく
	for (auto& key : arg_keyArray)
	{
		//指定されたキーのjsonオブジェクトがない
		if (!scanData.contains(key))return;

		//階層を降りる
		scanData = m_data[key];

		//階層のキーを記録
		latestKey = key;
	}

	return &scanData[key];
}