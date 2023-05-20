#include "OperationInfoUI.h"
#include"DirectX12/D3D12App.h"
#include"ForUser/DrawFunc/2D/DrawFunc2D.h"
#include"../OperationConfig.h"

OperationInfoUI::OperationInfoUI()
{
	using namespace KuroEngine;

	std::string dir = "resource/user/tex/in_game/operation/";
	m_opeBaseTex = D3D12App::Instance()->GenerateTextureBuffer(dir + "base.png");

	std::array<std::string, BUTTON_NUM>BUTTON_FILE_NAME = { "X","LT","RT" };
	for (int buttonIdx = 0; buttonIdx < BUTTON_NUM; ++buttonIdx)
	{
		D3D12App::Instance()->GenerateTextureBuffer(
			m_opeButtonTexArray[buttonIdx].data(),
			dir + BUTTON_FILE_NAME[buttonIdx] + ".png",
			INPUT_STATUS_NUM, 
			{ INPUT_STATUS_NUM,1 });
	}
}

void OperationInfoUI::Draw()
{
	using namespace KuroEngine;

	//入力方法画像のベース
	static const Vec2<float>OPE_BASE_POS = { 151.0f,504.0f };
	DrawFunc2D::DrawRotaGraph2D(OPE_BASE_POS, { 1.0f,1.0f }, 0.0f, m_opeBaseTex);

	//Xボタン
	static const Vec2<float>X_BUTTON_POS = { 124.0f,428.0f };
	DrawFunc2D::DrawRotaGraph2D(X_BUTTON_POS, { 1.0f,1.0f }, 0.0f,
		m_opeButtonTexArray[X][OperationConfig::Instance()->InputCamDistModeChange(OperationConfig::HOLD) ? OFF : ON]);

	//LTボタン
	static const Vec2<float>LT_BUTTON_POS = { 110.0f,528.0f };
	DrawFunc2D::DrawRotaGraph2D(LT_BUTTON_POS, { 1.0f,1.0f }, 0.0f,
		m_opeButtonTexArray[LT][OperationConfig::Instance()->InputCamReset(OperationConfig::HOLD) ? OFF : ON]);

	//RTボタン
	static const Vec2<float>RT_BUTTON_POS = { 110.0f,604.0f };
	DrawFunc2D::DrawRotaGraph2D(RT_BUTTON_POS, { 1.0f,1.0f }, 0.0f,
		m_opeButtonTexArray[RT][OperationConfig::Instance()->InputSink(OperationConfig::HOLD) ? OFF : ON]);
}
