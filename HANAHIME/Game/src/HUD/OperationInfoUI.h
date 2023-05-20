#pragma once
#include<array>
#include<memory>

namespace KuroEngine
{
	class TextureBuffer;
}

class OperationInfoUI
{
	//操作表記のベース
	std::shared_ptr<KuroEngine::TextureBuffer>m_opeBaseTex;

	//入力しているかどうか
	enum INPUT_STATUS { ON, OFF, INPUT_STATUS_NUM };
	enum BUTTON { X, LT, RT, BUTTON_NUM, };
	//入力しているときとしていないときの画像各２枚分
	std::array<std::array<std::shared_ptr<KuroEngine::TextureBuffer>, INPUT_STATUS_NUM>, BUTTON_NUM>m_opeButtonTexArray;
public:
	OperationInfoUI();
	void Draw();
};

