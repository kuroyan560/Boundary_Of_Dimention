#pragma once
#include"json.hpp"
#include<string>
#include<initializer_list>

namespace KuroEngine
{
	class JsonData
	{
	public:
		JsonData() {}
		JsonData(std::string arg_dir, std::string arg_fileName)
		{
			Import(arg_dir, arg_fileName);
		}

		//JSONをデシリアライズしたデータ
		nlohmann::json m_jsonData;

		/// <summary>
		/// 値のゲッタ
		/// </summary>
		/// <typeparam name="T">型の指定</typeparam>
		/// <param name="arg_dest">値の格納先</param>
		/// <param name="arg_keyArray">キー指定（順列でキーを見て階層を潜っていく）</param>
		/// <returns>値を格納したか</returns>
		template<typename T>
		bool Get(T& arg_dest, std::initializer_list<std::string>arg_keyArray)const;

		/// <summary>
		/// ファイル出力
		/// </summary>
		/// <param name="arg_dir">ディレクトリパス</param>
		/// <param name="arg_name">ファイル名</param>
		/// <param name="arg_ext">拡張子</param>
		/// <param name="arg_safeRewrite">上書き保存の場合、最終確認するか</param>
		void Export(std::string arg_dir, std::string arg_name, std::string arg_ext, bool arg_safeRewrite = true);

		/// <summary>
		/// ファイル読み込み
		/// </summary>
		/// <param name="arg_dir">ディレクトリパス</param>
		/// <param name="arg_fileName">ファイル名（拡張子含め）</param>
		void Import(std::string arg_dir, std::string arg_fileName);
	};

	template<typename T>
	inline bool JsonData::Get(T& arg_dest, std::initializer_list<std::string> arg_keyArray)const
	{
		//データが空
		if (m_jsonData.empty())return false;

		//走査中のデータ
		nlohmann::json scanData = m_jsonData;

		//キーの配列から、データ階層を下っていく
		for (auto& key : arg_keyArray)
		{
			//指定されたキーのjsonオブジェクトがない
			if (!scanData.contains(key))return false;

			//階層を降りる
			scanData = scanData[key];
		}

		//値の格納
		arg_dest = scanData.get<T>();

		return true;
	}
}