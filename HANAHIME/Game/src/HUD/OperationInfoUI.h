#pragma once
#include<array>
#include<memory>

namespace KuroEngine
{
	class TextureBuffer;
}

class OperationInfoUI
{
	//����\�L�̃x�[�X
	std::shared_ptr<KuroEngine::TextureBuffer>m_opeBaseTex;

	//���͂��Ă��邩�ǂ���
	enum INPUT_STATUS { ON, OFF, INPUT_STATUS_NUM };
	enum BUTTON { X, LT, RT, BUTTON_NUM, };
	//���͂��Ă���Ƃ��Ƃ��Ă��Ȃ��Ƃ��̉摜�e�Q����
	std::array<std::array<std::shared_ptr<KuroEngine::TextureBuffer>, INPUT_STATUS_NUM>, BUTTON_NUM>m_opeButtonTexArray;
public:
	OperationInfoUI();
	void Draw();
};

