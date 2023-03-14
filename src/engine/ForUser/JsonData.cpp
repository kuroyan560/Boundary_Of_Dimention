#include "JsonData.h"
#include"KuroEngine.h"
#include<fstream>

void KuroEngine::JsonData::Export(std::string arg_dir, std::string arg_name, std::string arg_ext)
{
	//空っぽ
	if (m_jsonData.empty())
	{
		AppearMessageBox(arg_name + arg_ext, "このjsonファイル空っぽ。");
		return;
	}

	//上書き保存の確認
	if (ExistFile(arg_dir + arg_name + arg_ext))
	{
		if (!AppearMessageBoxYN(arg_name + arg_ext, "同名のファイルがあるけど上書きして良い？"))return;
	}

	auto serialized = m_jsonData.dump(2);
	std::ofstream file;
	file.open(arg_dir + arg_name + arg_ext, std::ios::out);
	file << serialized << std::endl;
	file.close();
}
