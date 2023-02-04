#pragma once
#include"DirectX12/D3D12Data.h"
#include<memory>
#include<vector>
#include"KuroEngine.h"
#include<array>

namespace KuroEngine
{
	//画像にクリッピングベタ塗り
	class DrawFunc2D_Color
	{
	public:
		//塗り方
		enum struct FILL_MDOE
		{
			NONE,	//ベタ塗り
			MUL,	//乗算
			NUM,
		};

	private:
		//各塗り方のエントリポイント名
		static std::array<std::string, static_cast<int>(FILL_MDOE::NUM)> s_fillModeEntoryPoint;

		//DrawExtendGraph
		static int s_DrawExtendGraphCount;

		//DrawRotaGraph
		static int s_DrawRotaGraphCount;

	public:
		//呼び出しカウントリセット
		static void CountReset()
		{
			s_DrawExtendGraphCount = 0;
			s_DrawRotaGraphCount = 0;
		}

		/// <summary>
		/// 画像描画
		/// </summary>
		/// <param name="LeftUpPos">矩形の左上座標</param>
		/// <param name="Tex">テクスチャ</param>
		/// <param name="Paint">塗りつぶす色</param>
		/// <param name="Miror">矩形の右上座標</param>
		/// <param name="LeftUpPaintUV">塗りつぶす範囲の左上UV</param>
		/// <param name="RightBottomPaintUV">塗りつぶす範囲の右下UV</param>
		static void DrawGraph(const Vec2<float>& LeftUpPos, const std::shared_ptr<TextureBuffer>& Tex, const Color& Paint, const Vec2<bool>& Miror = { false,false },
			const Vec2<float>& LeftUpPaintUV = { 0.0f,0.0f }, const Vec2<float>& RightBottomPaintUV = { 1.0f,1.0f });

		/// <summary>
		/// 拡大縮小描画
		/// </summary>
		/// <param name="LeftUpPos">矩形の左上座標</param>
		/// <param name="RightBottomPos">矩形の右上座標</param>
		/// <param name="Tex">テクスチャ</param>
		/// <param name="Paint">塗りつぶす色</param>
		/// <param name="Miror">反転フラグ</param>
		/// <param name="LeftUpPaintUV">塗りつぶす範囲の左上UV</param>
		/// <param name="RightBottomPaintUV">塗りつぶす範囲の右下UV</param>
		/// <param name="FillMode">塗りつぶし方</param>
		static void DrawExtendGraph2D(const Vec2<float>& LeftUpPos, const Vec2<float>& RightBottomPos,
			const std::shared_ptr<TextureBuffer>& Tex, const Color& Paint, const Vec2<bool>& Miror = { false,false },
			const Vec2<float>& LeftUpPaintUV = { 0.0f,0.0f }, const Vec2<float>& RightBottomPaintUV = { 1.0f,1.0f },
			const FILL_MDOE& FillMode = FILL_MDOE::NONE);

		/// <summary>
		/// ２D画像回転描画
		/// </summary>
		/// <param name="Center">中心座標</param>
		/// <param name="ExtRate">拡大率</param>
		/// <param name="Radian">回転角度</param>
		/// <param name="Tex">テクスチャ</param>
		/// <param name="Paint">塗りつぶす色</param>
		static void DrawRotaGraph2D(const Vec2<float>& Center, const Vec2<float>& ExtRate, const float& Radian,
			const std::shared_ptr<TextureBuffer>& Tex, const Color& Paint, const Vec2<float>& RotaCenterUV = { 0.5f,0.5f }, const Vec2<bool>& Miror = { false,false },
			const Vec2<float>& LeftUpPaintUV = { 0.0f,0.0f }, const Vec2<float>& RightBottomPaintUV = { 1.0f,1.0f });

		/// <summary>
		/// 直線の描画（画像）
		/// </summary>
		/// <param name="FromPos">起点座標</param>
		/// <param name="ToPos">終点座標</param>
		/// <param name="Tex">テクスチャ</param>
		/// <param name="Paint">塗りつぶす色</param>
		/// <param name="Thickness">線の太さ</param>
		/// <param name="BlendMode">ブレンドモード</param>
		/// <param name="Miror">反転フラグ</param>
		static void DrawLine2DGraph(const Vec2<float>& FromPos, const Vec2<float>& ToPos, const std::shared_ptr<TextureBuffer>& Tex, const Color& Paint, const int& Thickness, const Vec2<bool>& Mirror = { false,false });
	};
}