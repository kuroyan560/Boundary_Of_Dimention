#include "JsonData.h"
#include"KuroEngine.h"
#include<fstream>

void KuroEngine::JsonData::Export(std::string arg_dir, std::string arg_name, std::string arg_ext, bool arg_safeRewrite)
{
	//空っぽ
	if (m_jsonData.empty())
	{
		AppearMessageBox(arg_name + arg_ext, "このjsonファイル空っぽ。");
		return;
	}

	//上書き保存の確認
	if (arg_safeRewrite && ExistFile(arg_dir + arg_name + arg_ext))
	{
		if (!AppearMessageBoxYN(arg_name + arg_ext, "同名のファイルがあるけど上書きして良い？"))return;
	}

	auto serialized = m_jsonData.dump(2);
	std::ofstream file;
	file.open(arg_dir + arg_name + arg_ext, std::ios::out);
	file << serialized << std::endl;
	file.close();
}

void KuroEngine::JsonData::Import(std::string arg_dir, std::string arg_fileName)
{
	std::ifstream file;
	file.open(arg_dir + arg_fileName);
	if (file.fail())
	{
		AppearMessageBox(arg_fileName, "ファイル開くのに失敗しちゃった。");
		return;
	}
	file >> m_jsonData;
	file.close();
}