#include "StageInfoUI.h"
#include"DirectX12/D3D12App.h"
#include"ForUser/DrawFunc/2D/DrawFunc2D.h"

void StageInfoUI::SetUIStatus(STATUS arg_status)
{
	using namespace KuroEngine;

	static const std::array<float, STATUS_NUM>INTERVALS =
	{
		45.0f,	//登場
		300.0f,	//通常描画
		45.0f,	//退場
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
	//ステージ名の装飾下線画像
	m_underLineTex = D3D12App::Instance()->GenerateTextureBuffer(stageTexDir + "under_line.png");

	//収集花のテクスチャのディレクトリ
	std::string flowerTexDir = "resource/user/tex/in_game/flower/";
	//花の画像（小）
	m_miniFlowerTex = D3D12App::Instance()->GenerateTextureBuffer(flowerTexDir + "flower_mini.png");
	//収集した花の数字（最後のインデックスの画像は 「 / 」）
	D3D12App::Instance()->GenerateTextureBuffer(m_flowerNumTex.data(), flowerTexDir + "flower_num.png", FLOWER_NUM_TEX_SIZE, { FLOWER_NUM_TEX_SIZE ,1 });
}

void StageInfoUI::Init(int arg_stageNum)
{
	m_stageNameIdx = arg_stageNum;
	SetUIStatus(APPEAR);
}

void StageInfoUI::Update(float arg_timeScale)
{
	using namespace KuroEngine;

	const float OFFSET_X_MAX = 300.0f;

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
		/*if (m_timer.IsTimeUp())
		{
			SetUIStatus(DISAPPEAR);
		}*/
	}
	else if (m_status == DISAPPEAR)
	{
		m_offsetX = Math::Ease(In, Back, m_timer.GetTimeRate(), 0.0f, OFFSET_X_MAX);
		m_alpha = Math::Lerp(1.0f, 0.0f, m_timer.GetTimeRate());
	}
}

void StageInfoUI::Draw(int arg_existFlowerNum, int arg_getFlowerNum)
{
	using namespace KuroEngine;

	//ステージ名の右下の座標
	static const Vec2<float>STAGE_NAME_RIGHT_BOTTOM_POS = { 1130.0f,150.0f };

	//オフセット
	const Vec2<float>offsetX = { m_offsetX,0.0f };

	//ステージ名描画
	const Vec2<float>stageNamePos = STAGE_NAME_RIGHT_BOTTOM_POS - m_stageNameTex[m_stageNameIdx]->GetGraphSize().Float();
	DrawFunc2D::DrawGraph(stageNamePos + offsetX, m_stageNameTex[m_stageNameIdx], m_alpha);

	//ステージ名の装飾下線描画
	const Vec2<float>UNDER_LINE_CENTER_POS = { 958.0f,161.0f };
	DrawFunc2D::DrawRotaGraph2D(UNDER_LINE_CENTER_POS + offsetX, { 1.0f,1.0f }, 0.0f, m_underLineTex, m_alpha);

	//花アイコンの描画
	const Vec2<float>MINI_FLOWER_CENTER_POS = { 1012.0f,202.0f };
	DrawFunc2D::DrawRotaGraph2D(MINI_FLOWER_CENTER_POS + offsetX, { 1.0f,1.0f }, 0.0f, m_miniFlowerTex, m_alpha);

	//左の数字の描画
	const Vec2<float>LEFT_NUM_CENTER_POS = { 1051.0f,205.0f };
	DrawFunc2D::DrawRotaGraph2D(LEFT_NUM_CENTER_POS + offsetX, { 1.0f,1.0f }, 0.0f, m_flowerNumTex[arg_getFlowerNum], m_alpha);

	//「 / 」の描画
	const Vec2<float>SLASH_CENTER_POS = { 1076.0f,208.0f };
	DrawFunc2D::DrawRotaGraph2D(SLASH_CENTER_POS + offsetX, { 1.0f,1.0f }, 0.0f, m_flowerNumTex[FLOWER_NUM_SLASH_IDX], m_alpha);

	//右の数字の描画
	const Vec2<float>RIGHT_NUM_CENTER_POS = { 1098.0f,208.0f };
	DrawFunc2D::DrawRotaGraph2D(RIGHT_NUM_CENTER_POS + offsetX, { 1.0f,1.0f }, 0.0f, m_flowerNumTex[arg_existFlowerNum], m_alpha);
}
