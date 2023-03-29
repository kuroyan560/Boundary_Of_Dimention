#pragma once
#include<memory>
#include"ForUser/Debugger.h"
#include<vector>
#include"Common/Vec.h"
#include"ForUser/Timer.h"

namespace KuroEngine
{
	class VertexBuffer;
	class TextureBuffer;
	class ConstantBuffer;
	class StructuredBuffer;
	class RWStructuredBuffer;

	class GraphicsPipeline;
	class ComputePipeline;

	class DepthStencil;
	class Camera;
}

//水彩画風に２つのテクスチャをブレンド
class WaterPaintBlend : public KuroEngine::Debugger
{
	static const int THREAD_PER_NUM = 32;
	
	//インクテクスチャ枚数
	static const int INK_TEX_NUM = 3;
	//インクテクスチャ
	std::array<std::shared_ptr<KuroEngine::TextureBuffer>, INK_TEX_NUM>m_inkTexArray;

	//定数バッファ
	struct ConstData
	{
		//マスクインクのスケール
		float m_initScale = 2.0f;
		//座標ズレ最大
		float m_posOffsetMax = 0.3f;
		//インクテクスチャ数
		int m_texMax = INK_TEX_NUM;
	}m_constData;
	std::shared_ptr<KuroEngine::ConstantBuffer>m_constBuffer;

	//一度に生成できる最大数
	static const int GENERATE_MAX_ONCE = 20;
	//生成予定のインクの座標配列
	std::vector<KuroEngine::Vec3<float>>m_appearInkPosArray;
	//生成予定のインクをスタックしておくバッファ
	std::shared_ptr<KuroEngine::StructuredBuffer>m_stackInkBuffer;

	//マスクレイヤーにこぼしたインク
	struct MaskInk
	{
		KuroEngine::Vec3<float>m_pos;
		float m_scale;
		KuroEngine::Vec3<float>m_posOffset;
		int m_texIdx;
	};
	//生成したインクのバッファー
	std::shared_ptr<KuroEngine::RWStructuredBuffer>m_aliveInkBuffer;
	//生成したインクのカウンターバッファー
	std::shared_ptr<KuroEngine::RWStructuredBuffer>m_aliveInkCounterBuffer;

	//生成できるマスクインクの最大数
	int m_aliveInkMax = 10000;

	//マスクインクの更新するスパン
	int m_updateSpan = 3;
	KuroEngine::Timer m_updateTimer;

	//マスクインク描画用の板ポリ頂点バッファ
	static std::shared_ptr<KuroEngine::VertexBuffer>s_maskInkPolygon;

	//マスクインクを初期化するパイプライン
	static std::shared_ptr<KuroEngine::ComputePipeline>s_initInkPipeline;
	//マスクインクを生成するパイプライン
	static std::shared_ptr<KuroEngine::ComputePipeline>s_appearInkPipeline;
	//マスクインクを更新するパイプライン
	static std::shared_ptr<KuroEngine::ComputePipeline>s_updateInkPipeline;
	//マスクインクを描画するパイプライン
	static std::shared_ptr<KuroEngine::GraphicsPipeline>s_drawInkPipeline;

	//水彩画風にするパイプライン
	static std::shared_ptr<KuroEngine::ComputePipeline>s_waterPaintPipeline;
	void GeneratePipeline();

	//結果の描画先
	std::shared_ptr<KuroEngine::TextureBuffer>m_resultTex;

	//マスクレイヤーレンダーターゲット
	std::shared_ptr<KuroEngine::RenderTarget>m_maskLayer;

	void OnImguiItems()override;

public:
	WaterPaintBlend();

	//初期化処理
	void Init();

	//マスクレイヤーにインク垂らす
	void DropMaskInk(KuroEngine::Vec3<float>arg_pos);

	//グラフィックスマネージャに登録
	void Register(std::shared_ptr<KuroEngine::TextureBuffer>arg_baseTex, KuroEngine::Camera& arg_cam, std::weak_ptr<KuroEngine::DepthStencil>arg_depthStencil);

	//結果のテクスチャ取得
	std::shared_ptr<KuroEngine::TextureBuffer>& GetResultTex() { return m_resultTex; }
};