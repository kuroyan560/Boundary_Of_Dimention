#include "StageInfoUI.h"
#include"DirectX12/D3D12App.h"
#include"ForUser/DrawFunc/2D/DrawFunc2D.h"

void StageInfoUI::SetUIStatus(STATUS arg_status)
{
	using namespace KuroEngine;

	static const std::array<float, STATUS_NUM>INTERVALS =
	{
		35.0f,	//登場
		250.0f,	//通常描画
		35.0f,	//退場
	};

	//演出時間セット
	m_timer.Reset(INTERVALS[arg_status]);
	//ステータス更新
	m_status = arg_status;
}

void StageInfoUI::Appear()
{
	if (m_status != DISAPPEAR)return;
	SetUIStatus(APPEAR);
}

void StageInfoUI::Disappear()
{
	if (m_status != DRAW)return;
	SetUIStatus(DISAPPEAR);
}

StageInfoUI::StageInfoUI()
{
	using namespace KuroEngine;

	//ステージ関連のテクスチャのディレクトリ
	std::string stageTexDir = "resource/user/tex/stage/";
	//ステージ名テクスチャ
	int stageIdx = 0;
	while (1)
	{
		std::string path = stageTexDir + std::to_string(stageIdx++) + ".png";
		if (!ExistFile(path))break;

		m_stageNameTex.emplace_back(D3D12App::Instance()->GenerateTextureBuffer(path));
	}
	m_stageNameDefaultTex = D3D12App::Instance()->GenerateTextureBuffer(stageTexDir + "default.png");
	//ステージ名の装飾下線画像
	m_underLineTex = D3D12App::Instance()->GenerateTextureBuffer(stageTexDir + "under_line.png");

	//収集花のテクスチャのディレクトリ
	std::string flowerTexDir = "resource/user/tex/in_game/flower/";
	//花の画像（小）
	m_miniFlowerTex = D3D12App::Instance()->GenerateTextureBuffer(flowerTexDir + "flower_mini.png");
	//収集した花の数字（最後のインデックスの画像は 「 / 」）
	D3D12App::Instance()->GenerateTextureBuffer(m_flowerNumTex.data(), flowerTexDir + "flower_num.png", FLOWER_NUM_TEX_SIZE, { FLOWER_NUM_TEX_SIZE ,1 });
}

void StageInfoUI::Init(int arg_stageNum, int arg_getFlowerNum)
{
	m_stageNameIdx = arg_stageNum;
	SetUIStatus(APPEAR);
	m_oldGetFlowerNum = arg_getFlowerNum;
	m_addFlowerNum = 0;
}

void StageInfoUI::Update(float arg_timeScale, int arg_getFlowerNum)
{
	using namespace KuroEngine;

	const float OFFSET_X_MAX = 300.0f;
	const float ADD_FLOWER_OFFSET_Y_MAX = -80.0f;
	const float ADD_FLOWER_APEEAR_TIME = 20.0f;

	m_timer.UpdateTimer(arg_timeScale);

	if (m_status == APPEAR)
	{
		m_offsetX = Math::Ease(Out, Back, m_timer.GetTimeRate(), OFFSET_X_MAX, 0.0f);
		m_alpha = Math::Lerp(0.0f, 1.0f, m_timer.GetTimeRate());
		if (m_timer.IsTimeUp())
		{
			SetUIStatus(DRAW);
		}
	}
	else if (m_status == DRAW)
	{
		m_addFlowerNumTimer.UpdateTimer(arg_timeScale);
		m_addFlowerAlpha = Math::Ease(Out, Exp, m_addFlowerNumTimer.GetTimeRate(), 0.0f, 1.0f);
		m_addFlowerOffsetY = Math::Ease(Out, Back, m_addFlowerNumTimer.GetTimeRate(), ADD_FLOWER_OFFSET_Y_MAX, 0.0f);

		if (m_addFlowerNum && m_timer.IsTimeUp())
		{
			m_addFlowerNum = 0;
			SetUIStatus(DISAPPEAR);
		}
	}
	else if (m_status == DISAPPEAR)
	{
		m_offsetX = Math::Ease(In, Back, m_timer.GetTimeRate(), 0.0f, OFFSET_X_MAX);
		m_alpha = Math::Lerp(1.0f, 0.0f, m_timer.GetTimeRate());
	}

	if (m_oldGetFlowerNum < arg_getFlowerNum)
	{
		m_addFlowerNumTimer.Reset(ADD_FLOWER_APEEAR_TIME);
		Appear();
		m_addFlowerNum = arg_getFlowerNum - m_oldGetFlowerNum;
		m_addFlowerAlpha = 0.0f;
		m_addFlowerOffsetY = ADD_FLOWER_OFFSET_Y_MAX;
	}
	m_oldGetFlowerNum = arg_getFlowerNum;
}

void StageInfoUI::Draw(int arg_existFlowerNum, int arg_getFlowerNum)
{
	using namespace KuroEngine;

	//ステージ名の右下の座標
	static const Vec2<float>STAGE_NAME_RIGHT_BOTTOM_POS = { 1130.0f,150.0f };

	//オフセット
	const Vec2<float>offsetX = { m_offsetX,0.0f };

	//ステージ名描画
	auto stageNameTex = m_stageNameDefaultTex;
	if (0 <= m_stageNameIdx && m_stageNameIdx < static_cast<int>(m_stageNameTex.size()))stageNameTex = m_stageNameTex[m_stageNameIdx];
	const Vec2<float>stageNamePos = STAGE_NAME_RIGHT_BOTTOM_POS - stageNameTex->GetGraphSize().Float();
	DrawFunc2D::DrawGraph(stageNamePos + offsetX, stageNameTex, m_alpha);

	//ステージ名の装飾下線描画
	static const Vec2<float>UNDER_LINE_CENTER_POS = { 958.0f,161.0f };
	DrawFunc2D::DrawRotaGraph2D(UNDER_LINE_CENTER_POS + offsetX, { 1.0f,1.0f }, 0.0f, m_underLineTex, m_alpha);

	//花アイコンの描画
	static const Vec2<float>MINI_FLOWER_CENTER_POS = { 1012.0f,202.0f };
	DrawFunc2D::DrawRotaGraph2D(MINI_FLOWER_CENTER_POS + offsetX, { 1.0f,1.0f }, 0.0f, m_miniFlowerTex, m_alpha);

	//左の数字の描画
	static const Vec2<float>LEFT_NUM_CENTER_POS = { 1051.0f,205.0f };
	DrawFunc2D::DrawRotaGraph2D(LEFT_NUM_CENTER_POS + offsetX, { 1.0f,1.0f }, 0.0f, m_flowerNumTex[arg_getFlowerNum], m_alpha);

	//「 / 」の描画
	static const Vec2<float>SLASH_CENTER_POS = { 1076.0f,208.0f };
	DrawFunc2D::DrawRotaGraph2D(SLASH_CENTER_POS + offsetX, { 1.0f,1.0f }, 0.0f, m_flowerNumTex[FLOWER_NUM_SLASH_IDX], m_alpha);

	//右の数字の描画
	static const Vec2<float>RIGHT_NUM_CENTER_POS = { 1098.0f,208.0f };
	DrawFunc2D::DrawRotaGraph2D(RIGHT_NUM_CENTER_POS + offsetX, { 1.0f,1.0f }, 0.0f, m_flowerNumTex[arg_existFlowerNum], m_alpha);

	//入手時
	if (m_addFlowerNum)
	{
		const Vec2<float>addFlowerOffsetY = { 0.0f,m_addFlowerOffsetY };

		//「 + 」の描画
		const Vec2<float>PLUS_CENTER_POS = { 1058.0f,256.0f };
		DrawFunc2D::DrawRotaGraph2D(PLUS_CENTER_POS + addFlowerOffsetY, { 1.0f,1.0f }, 0.0f, m_flowerNumTex[FLOWER_NUM_PLUS_IDX], m_addFlowerAlpha);

		//増えた花の数描画
		const Vec2<float>ADD_FLOWER_NUM_CENTER_POS = { 1080.0f,252.0f };
		DrawFunc2D::DrawRotaGraph2D(ADD_FLOWER_NUM_CENTER_POS + addFlowerOffsetY, { 1.0f,1.0f }, 0.0f, m_flowerNumTex[m_addFlowerNum], m_addFlowerAlpha);
	}
}
