#pragma once
#include"DirectX12/D3D12Data.h"
#include"KuroEngine.h"

namespace KuroEngine
{
	//揃え方
	enum struct HORIZONTAL_ALIGN { LEFT, CENTER, RIGHT };
	enum struct VERTICAL_ALIGN { TOP, CENTER, BOTTOM };

	class DrawFunc2D
	{
		//DrawLine
		static int s_DrawLineCount;	//呼ばれた回数

		//DrawBox
		static int s_DrawBoxCount;

		//DrawCircle
		static int s_DrawCircleCount;

		//DrawExtendGraph
		static int s_DrawExtendGraphCount;

		//DrawRotaGraph
		static int s_DrawRotaGraphCount;

		//DrawRadialWipeGraph2D
		static int s_DrawRadialWipeGraphCount;

	public:
		//呼び出しカウントリセット
		static void CountReset()
		{
			s_DrawLineCount = 0;
			s_DrawBoxCount = 0;
			s_DrawCircleCount = 0;
			s_DrawExtendGraphCount = 0;
			s_DrawRotaGraphCount = 0;
			s_DrawRadialWipeGraphCount = 0;
		}

		/// <summary>
		/// 直線の描画
		/// </summary>
		/// <param name="FromPos">起点座標</param>
		/// <param name="ToPos">終点座標</param>
		/// <param name="Color">色</param>
		/// <param name="BlendMode">ブレンドモード</param>
		static void DrawLine2D(const Vec2<float>& FromPos, const Vec2<float>& ToPos, const Color& LineColor, const AlphaBlendMode& BlendMode = AlphaBlendMode_Trans);

		/// <summary>
		/// 直線の描画（画像）
		/// </summary>
		/// <param name="FromPos">起点座標</param>
		/// <param name="ToPos">終点座標</param>
		/// <param name="Tex">テクスチャ</param>
		/// <param name="Thickness">線の太さ</param>
		/// <param name="Alpha">アルファ</param>
		/// <param name="BlendMode">ブレンドモード</param>
		/// <param name="Miror">反転フラグ</param>
		static void DrawLine2DGraph(const Vec2<float>& FromPos, const Vec2<float>& ToPos, const std::shared_ptr<TextureBuffer>& Tex, const int& Thickness, const float& Alpha = 1.0f, const AlphaBlendMode& BlendMode = AlphaBlendMode_Trans, const Vec2<bool>& Mirror = { false,false });

		/// <summary>
		/// 2D四角形の描画
		/// </summary>
		/// <param name="LeftUpPos">左上座標</param>
		/// <param name="RightBottomPos">右下座標</param>
		/// <param name="Color">色</param>
		/// <param name="FillFlg">中を塗りつぶすか</param>
		/// <param name="BlendMode">ブレンドモード</param>
		static void DrawBox2D(const Vec2<float>& LeftUpPos, const Vec2<float>& RightBottomPos, const Color& BoxColor, const bool& FillFlg = false, const AlphaBlendMode& BlendMode = AlphaBlendMode_Trans);

		/// <summary>
		/// ２D円の描画
		/// </summary>
		/// <param name="Center">中心座標</param>
		/// <param name="Radius">半径</param>
		/// <param name="Color">色</param>
		/// <param name="FillFlg">中を塗りつぶすか</param>
		/// <param name="LineThickness">中を塗りつぶさない場合の線の太さ</param>
		/// <param name="BlendMode">ブレンドモード</param>
		static void DrawCircle2D(const Vec2<float>& Center, const float& Radius, const Color& CircleColor, const bool& FillFlg = false, const int& LineThickness = 1, const AlphaBlendMode& BlendMode = AlphaBlendMode_Trans);

		/// <summary>
		/// 画像描画
		/// </summary>
		/// <param name="LeftUpPos">矩形の左上座標</param>
		/// <param name="Tex">テクスチャ</param>
		/// <param name="Alpha">アルファ</param>
		/// <param name="Scale">スケール</param>
		/// <param name="BlendMode">ブレンドモード</param>
		static void DrawGraph(const Vec2<float>& LeftUpPos, const std::shared_ptr<TextureBuffer>& Tex, const float& Alpha = 1.0f, const float& Scale = 1.0f, const AlphaBlendMode& BlendMode = AlphaBlendMode_Trans, const Vec2<bool>& Miror = { false,false });

		/// <summary>
		/// 拡大縮小描画
		/// </summary>
		/// <param name="LeftUpPos">矩形の左上座標</param>
		/// <param name="RightBottomPos">矩形の右上座標</param>
		/// <param name="Tex">テクスチャ</param>
		/// <param name="Alpha">アルファ</param>
		/// <param name="BlendMode">ブレンドモード</param>
		static void DrawExtendGraph2D(const Vec2<float>& LeftUpPos, const Vec2<float>& RightBottomPos,
			const std::shared_ptr<TextureBuffer>& Tex, const float& Alpha = 1.0f,
			const AlphaBlendMode& BlendMode = AlphaBlendMode_Trans, const Vec2<bool>& Miror = { false,false });

		/// <summary>
		/// ２D画像回転描画
		/// </summary>
		/// <param name="Center">中心座標</param>
		/// <param name="ExtRate">拡大率</param>
		/// <param name="Radian">回転角度</param>
		/// <param name="Tex">テクスチャ</param>
		/// <param name="Alpha">アルファ</param>
		/// <param name="BlendMode">ブレンドモード</param>
		/// <param name="LRTurn">左右反転フラグ</param>
		static void DrawRotaGraph2D(const Vec2<float>& Center, const Vec2<float>& ExtRate, const float& Radian,
			const std::shared_ptr<TextureBuffer>& Tex, const float& Alpha = 1.0f, const Vec2<float>& RotaCenterUV = { 0.5f,0.5f },
			const AlphaBlendMode& BlendMode = AlphaBlendMode_Trans, const Vec2<bool>& Mirror = { false,false });

		/// <summary>
		/// 放射状ワイプ画像描画
		/// </summary>
		/// <param name="arg_center">中心座標</param>
		/// <param name="arg_extRate">拡大率</param>
		/// <param name="arg_startAngle">放射状ワイプの始端角度</param>
		/// <param name="arg_endAngle">放射状ワイプの終端角度</param>
		/// <param name="arg_anchor">放射状ワイプのアンカーポイント</param>
		/// <param name="arg_tex">テクスチャ</param>
		/// <param name="BlendMode">ブレンドモード</param>
		static void DrawRadialWipeGraph2D(
			const Vec2<float>& arg_center,
			const Vec2<float>& arg_extRate,
			const float& arg_startRadian,
			const float& arg_endRadian,
			const Vec2<float>& arg_anchor,
			const std::shared_ptr<TextureBuffer>& arg_tex,
			const AlphaBlendMode& BlendMode = AlphaBlendMode_Trans);

		/// <summary>
		/// 2D数値描画
		/// </summary>
		/// <param name="arg_num">数字</param>
		/// <param name="arg_pos">座標</param>
		/// <param name="arg_numTexArray">数字テクスチャ配列</param>
		/// <param name="arg_expRate">拡大率</param>
		/// <param name="arg_letterSpace">字間</param>
		/// <param name="arg_horizontalAlign">水平方向アライメント（揃え方）</param>
		/// <param name="arg_verticalAlign">垂直方向アライメント（揃え方）</param>
		static void DrawNumber2D(
			const int& arg_num,
			const Vec2<float>& arg_pos,
			const std::shared_ptr<TextureBuffer>* arg_numTexArray,
			const Vec2<float>& arg_expRate = { 1.0f,1.0f },
			const float& arg_letterSpace = 0.0f,
			const HORIZONTAL_ALIGN& arg_horizontalAlign = HORIZONTAL_ALIGN::LEFT,
			const VERTICAL_ALIGN& arg_verticalAlign = VERTICAL_ALIGN::TOP);
	};
}