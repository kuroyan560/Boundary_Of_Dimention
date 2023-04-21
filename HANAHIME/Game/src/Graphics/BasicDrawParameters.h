#pragma once
#include"Common/Color.h"
//�g�D�[���V�F�[�_�[�̌ʂ̃p�����[�^
class IndividualDrawParameter
{
public:
	static IndividualDrawParameter& GetDefault()
	{
		static IndividualDrawParameter defaultParams =
		{
			KuroEngine::Color(1.0f,1.0f,1.0f,1.0f),
			KuroEngine::Color(200,142,237,255),
			KuroEngine::Color(0.0f,0.0f,0.0f,1.0f),
		};
		return defaultParams;
	}

	//�h��Ԃ��F
	KuroEngine::Color m_fillColor = KuroEngine::Color(1.0f, 1.0f, 1.0f, 0.0f);
	//���邢�����ɏ�Z����F
	KuroEngine::Color m_brightMulColor = KuroEngine::Color(1.0f, 1.0f, 1.0f, 1.0f);
	//�Â������ɏ�Z����F
	KuroEngine::Color m_darkMulColor = KuroEngine::Color(0.3f, 0.3f, 0.3f, 1.0f);
	//�G�b�W�F
	KuroEngine::Color m_edgeColor = KuroEngine::Color(0.0f, 0.0f, 0.0f, 1.0f);
	
	//�}�X�N���C���[�ɕ`�悷�邩
	int m_drawMask = 0;

	int pad[3];
	
	IndividualDrawParameter() { *this = GetDefault(); }
	IndividualDrawParameter(
		KuroEngine::Color arg_brightMulColor,
		KuroEngine::Color arg_darkMulColor,
		KuroEngine::Color arg_edgeMulColor)
		:m_brightMulColor(arg_brightMulColor), m_darkMulColor(arg_darkMulColor), m_edgeColor(arg_edgeMulColor) {}
	void ImguiDebugItem();
};