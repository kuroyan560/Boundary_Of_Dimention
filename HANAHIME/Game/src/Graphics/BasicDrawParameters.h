#pragma once
#include"Common/Color.h"
//トゥーンシェーダーの個別のパラメータ
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

	//塗りつぶし色
	KuroEngine::Color m_fillColor = KuroEngine::Color(1.0f, 1.0f, 1.0f, 0.0f);
	//明るい部分に乗算する色
	KuroEngine::Color m_brightMulColor = KuroEngine::Color(1.0f, 1.0f, 1.0f, 1.0f);
	//暗い部分に乗算する色
	KuroEngine::Color m_darkMulColor = KuroEngine::Color(0.3f, 0.3f, 0.3f, 1.0f);
	//エッジ色
	KuroEngine::Color m_edgeColor = KuroEngine::Color(0.0f, 0.0f, 0.0f, 1.0f);
	
	//マスクレイヤーに描画するか
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