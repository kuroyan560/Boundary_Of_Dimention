#include "MapPinUI.h"
#include"KuroEngine.h"
#include"Render/RenderObject/Camera.h"
#include"FrameWork/WinApp.h"
#include"DirectX12/D3D12App.h"
#include"ForUser/DrawFunc/2D/DrawFunc2D.h"

MapPinUI::MapPinUI()
{
	using namespace KuroEngine;

	std::string texDirPath = "resource/user/tex/in_game/map_pin/";
	//ピン
	m_smallSquare = std::make_shared<Content>(texDirPath + "small_square.png", &m_canvasTransform);
	m_middleSquare = std::make_shared<Content>(texDirPath + "middle_square.png", &m_canvasTransform);
	m_largeSquare = std::make_shared<Content>(texDirPath + "large_square.png", &m_canvasTransform);

	//距離の数字のテクスチャ読み込み
	D3D12App::Instance()->GenerateTextureBuffer(m_numTex.data(), texDirPath + "num.png", 10, Vec2<int>(10, 1));
	for (auto& distNum : m_distanceNum)distNum = std::make_shared<Content>(m_numTex[0], &m_canvasTransform);
	m_meter = std::make_shared<Content>(texDirPath + "meter.png", &m_canvasTransform);

	//画面内に目標地点が映っているときのUI
	m_mapPinUI[PIN_MODE_IN_SCREEN].emplace_back(m_largeSquare);
	m_mapPinUI[PIN_MODE_IN_SCREEN].emplace_back(m_middleSquare);

	//画面内に目標地点が映っていないときのUI

	//共通
	for (int pinMode = 0; pinMode < PIN_MODE_NUM; ++pinMode)
	{
		m_mapPinUI[pinMode].emplace_back(m_smallSquare);
		for (auto& distNum : m_distanceNum)
		{
			m_mapPinUI[pinMode].emplace_back(distNum);
		}
		m_mapPinUI[pinMode].emplace_back(m_meter);
	}
}

void MapPinUI::UpdateDistance(PIN_MODE arg_pinMode, float arg_distance)
{
	//小数点切り捨て
	int dist = static_cast<int>(arg_distance);

	//桁取得
	int digit = KuroEngine::GetDigit(dist);

	//桁が最大より大きいときはカンストさせる
	if (NUM_DIGIT_MAX < digit)
	{
		dist = 0;
		for (int digitIdx = 0; digitIdx < NUM_DIGIT_MAX; ++digitIdx)
		{
			dist += 9 * static_cast<int>(pow(10, digitIdx));
		}
	}

	//表記の横幅の合計計算
	auto numTexWidth = m_numTex[0]->GetGraphSize().x;
	auto meterTexWidth = m_meter->m_tex->GetGraphSize().x;
	//距離表記の左端
	float leftPosX = digit * numTexWidth + (digit - 1) * m_distStrDrawSpace + m_meterStrDrawSpace + m_meter->m_tex->GetGraphSize().x;
	leftPosX *= -0.5f;

	//座標オフセットY
	float offsetY = arg_pinMode == PIN_MODE_IN_SCREEN ? m_largeSquare->m_tex->GetGraphSize().y * 0.5f : m_smallSquare->m_tex->GetGraphSize().y * 1.5f;
	offsetY += m_meterDrawOffsetY;

	for (int digitIdx = 0; digitIdx < NUM_DIGIT_MAX; ++digitIdx)
	{
		//必要のない桁数
		if (digit <= digitIdx)
		{
			m_distanceNum[digitIdx]->m_active = false;	//非表示
			continue;
		}

		m_distanceNum[digitIdx]->m_active = true;	//表示

		//数字テクスチャアタッチ
		m_distanceNum[digitIdx]->m_tex = m_numTex[KuroEngine::GetSpecifiedDigitNum(dist, digitIdx, true)];
		//座標セット
		m_distanceNum[digitIdx]->m_transform.SetPos({ leftPosX + numTexWidth * 0.5f, offsetY });
		//Xずらし
		leftPosX += numTexWidth + m_distStrDrawSpace;
	}

	//m表記
	m_meter->m_transform.SetPos({ leftPosX + m_meterStrDrawSpace + meterTexWidth * 0.5f,offsetY });
}

void MapPinUI::Draw(KuroEngine::Camera& arg_cam, KuroEngine::Vec3<float> arg_destinationPos, KuroEngine::Vec3<float>arg_playerPos)
{
	using namespace KuroEngine;

	//画面サイズ取得
	const auto winSize = WinApp::Instance()->GetExpandWinSize();
	const auto winCenter = winSize * 0.5f;

	//目的地の３D座標を２Dに変換
	float camDist = 0.0f;
	auto destPos2D = ConvertWorldToScreen(arg_destinationPos, arg_cam.GetViewMat(), arg_cam.GetProjectionMat(), winSize, &camDist);

	//カメラが反対向き
	if (camDist < 0.0f)
	{
		//Yのみ補正
		if (destPos2D.y < winCenter.y)destPos2D.y = 0.0f;
		else destPos2D.y = winSize.y;
	}

	//画面外か（正確にはピンUIが画面内に入るかも考慮している）
	float pinSizeHalf = m_pinSize * 0.5f;
	bool isOutOfScreen = destPos2D.x < pinSizeHalf || winSize.x - pinSizeHalf < destPos2D.x || destPos2D.y < pinSizeHalf || winSize.y - pinSizeHalf < destPos2D.y;

	//クランプに用いるオフセット決定
	float clampOffset = isOutOfScreen ? m_arrowClampOffset : pinSizeHalf;
	//UIに合わせて座標をクランプ
	destPos2D.x = std::clamp(destPos2D.x, clampOffset, winSize.x - clampOffset);
	destPos2D.y = std::clamp(destPos2D.y, clampOffset, winSize.y - clampOffset);

	//UI全体の中心座標を設定
	m_canvasTransform.SetPos(destPos2D);

	//表示するUIの決定
	PIN_MODE mode = isOutOfScreen ? PIN_MODE_OUT_SCREEN : PIN_MODE_IN_SCREEN;

	//距離の数字更新
	UpdateDistance(mode, arg_destinationPos.Distance(arg_playerPos));

	//UI描画
	for (auto& uiPtr : m_mapPinUI[mode])
	{
		auto& ui = uiPtr.lock();
		if (!ui->m_active)continue;
		auto& transform = uiPtr.lock()->m_transform;
		DrawFunc2D::DrawRotaGraph2D(transform.GetPosWorld(), transform.GetScaleWorld(), 0.0f, ui->m_tex, ui->m_alpha);
	}
}

MapPinUI::Content::Content(std::string arg_texPath, KuroEngine::Transform2D* arg_parent)
{
	m_tex = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer(arg_texPath);
	m_transform.SetParent(arg_parent);
}
