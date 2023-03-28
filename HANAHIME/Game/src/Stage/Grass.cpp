#include "Grass.h"
#include"DirectX12/D3D12App.h"
#include"FrameWork/Importer.h"
#include"../Graphics/BasicDraw.h"
#include"../Graphics/WaterPaintBlend.h"
#include"KuroEngineDevice.h"
#include"Render/RenderObject/Camera.h"

Grass::Grass()
{
	using namespace KuroEngine;

	//仮====================================
		//仮置きの草ブロックモデル
	m_grassBlockModel = Importer::Instance()->LoadModel("resource/user/model/", "GrassBlock.gltf");
	//=====================================

		//パイプライン生成
	{
		//パイプライン設定
		PipelineInitializeOption pipelineOption(D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT, D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

		//シェーダー情報
		Shaders shaders;
		shaders.m_vs = D3D12App::Instance()->CompileShader("resource/user/shaders/Grass.hlsl", "VSmain", "vs_6_4");
		shaders.m_gs = D3D12App::Instance()->CompileShader("resource/user/shaders/Grass.hlsl", "GSmain", "gs_6_4");
		shaders.m_ps = D3D12App::Instance()->CompileShader("resource/user/shaders/Grass.hlsl", "PSmain", "ps_6_4");

		//インプットレイアウト
		std::vector<InputLayoutParam>inputLayout =
		{
			InputLayoutParam("POSITION",DXGI_FORMAT_R32G32B32_FLOAT),
			InputLayoutParam("TexID",DXGI_FORMAT_R8_UINT),
			InputLayoutParam("NORMAL",DXGI_FORMAT_R32G32B32_FLOAT),
			InputLayoutParam("IsAlive",DXGI_FORMAT_R8_UINT),
			InputLayoutParam("SINELENGTH",DXGI_FORMAT_R32_FLOAT),
			InputLayoutParam("APPEARY",DXGI_FORMAT_R32_FLOAT)
		};

		//ルートパラメータ
		std::vector<RootParam>rootParam =
		{
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"カメラ情報"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"（好きなの入れてね）"),
		};
		//テクスチャバッファ用ルートパラメータ設定
		for (int texIdx = 0; texIdx < s_textureNumMax; ++texIdx)
		{
			rootParam.emplace_back(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, "テクスチャ情報");
		}

		//レンダーターゲット描画先情報
		std::vector<RenderTargetInfo>renderTargetInfo =
		{
			RenderTargetInfo(D3D12App::Instance()->GetBackBuffFormat(), AlphaBlendMode_Trans),	//通常描画
			RenderTargetInfo(DXGI_FORMAT_R32G32B32A32_FLOAT, AlphaBlendMode_Trans),	//エミッシブマップ
			RenderTargetInfo(DXGI_FORMAT_R16_FLOAT, AlphaBlendMode_None),	//深度マップ
			RenderTargetInfo(DXGI_FORMAT_R16G16B16A16_FLOAT, AlphaBlendMode_None),	//ノーマルマップ
			RenderTargetInfo(D3D12App::Instance()->GetBackBuffFormat(), AlphaBlendMode_None),	//エッジカラーマップ
		};

		//設定を基にパイプライン生成
		m_pipeline = D3D12App::Instance()->GenerateGraphicsPipeline(
			pipelineOption,
			shaders,
			inputLayout,
			rootParam,
			renderTargetInfo,
			{ WrappedSampler(true,true) });
	}

	//頂点バッファ
	m_vertBuffer = D3D12App::Instance()->GenerateVertexBuffer(
		sizeof(Vertex),
		s_vertexMax,
		m_vertices.data(),
		"Grass - VertexBuffer");

	//行列以外のデータ用構造体
	m_constBuffer = D3D12App::Instance()->GenerateConstantBuffer(
		sizeof(CBVdata),
		1,
		nullptr,
		"Grass - Free - ConstantBuffer");

	//テクスチャ
	m_texBuffer[0] = D3D12App::Instance()->GenerateTextureBuffer("resource/user/imposter/Imposter01.png");
	m_texBuffer[1] = D3D12App::Instance()->GenerateTextureBuffer("resource/user/imposter/Imposter02.png");
	m_texBuffer[2] = D3D12App::Instance()->GenerateTextureBuffer("resource/user/imposter/Imposter03.png");
	m_texBuffer[3] = D3D12App::Instance()->GenerateTextureBuffer("resource/user/imposter/Imposter04.png");
	m_texBuffer[4] = D3D12App::Instance()->GenerateTextureBuffer("resource/user/imposter/Imposter01.png");
}

void Grass::Init()
{
	using namespace KuroEngine;

	//ワールド行列配列初期化
	m_grassWorldMatArray.clear();

	m_oldPlayerPos = { -1000,-1000,-1000 };
	m_plantTimer.Reset(0);

	for (int vertIdx = 0; vertIdx < m_deadVertexIdx; ++vertIdx)
	{
		m_vertices[vertIdx].m_isAlive = 0;
	}
	m_deadVertexIdx = 0;
}

void Grass::Update(const float arg_timeScale, const KuroEngine::Vec3<float> arg_playerPos, const KuroEngine::Quaternion arg_playerRotate, KuroEngine::Transform arg_camTransform, KuroEngine::Vec2<float> arg_grassPosScatter, WaterPaintBlend& arg_waterPaintBlend)
{
	using namespace KuroEngine;

	//プレイヤーが移動した and 周りに草がない。
	bool isMovePlayer = !((arg_playerPos - m_oldPlayerPos).Length() < 0.1f);
	bool isGrassAround = IsGrassAround(arg_playerPos);
	if (isMovePlayer && !isGrassAround)
	{
		if (m_plantTimer.IsTimeUp())
		{
			Transform grassTransform;
			grassTransform.SetPos(arg_playerPos);
			grassTransform.SetRotate(arg_playerRotate);
			grassTransform.SetScale({ 1.0f,1.0f,1.0f });
			PlantGrassBlock(grassTransform, arg_grassPosScatter, arg_waterPaintBlend);
			m_plantTimer.Reset(3);
		}
		m_plantTimer.UpdateTimer();
	}

	m_oldPlayerPos = arg_playerPos;

	for (auto& index : m_vertices) {
		if (!index.m_isAlive) continue;

		//イージングタイマーを更新。
		auto& easingTimer = m_appearYTimer[static_cast<int>(&index - &m_vertices[0])];
		easingTimer = std::clamp(easingTimer + 0.05f, 0.0f, 1.0f);

		//イージング量を求める。
		float easingAmount = KuroEngine::Math::Ease(In, Cubic, easingTimer, 0.0f, 1.0f);

		index.m_appearY = easingAmount;

	}

	//頂点データを更新。
	m_vertBuffer->Mapping(m_vertices.data());

	//定数バッファ1のカメラ座標を更新。
	DirectX::XMMATRIX camMatWorld = arg_camTransform.GetMatWorld();
	m_constData.m_pos = KuroEngine::Vec3<float>(camMatWorld.r[3].m128_f32[0], camMatWorld.r[3].m128_f32[1], camMatWorld.r[3].m128_f32[2]);

	//定数バッファ1の草の揺れ具合を更新。
	m_constData.m_sineWave += 0.02f;

	//定数バッファ1をGPUに転送。
	m_constBuffer->Mapping(&m_constData);

}

void Grass::Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)
{
	if (s_instanceMax <= static_cast<int>(m_grassWorldMatArray.size()))
	{
		KuroEngine::AppearMessageBox("Grass : Draw() 失敗", "インスタンスの上限超えちゃった");
		exit(1);
	}
	if (m_grassWorldMatArray.empty())return;

	using namespace KuroEngine;

	KuroEngineDevice::Instance()->Graphics().SetGraphicsPipeline(m_pipeline);

	std::vector<RegisterDescriptorData>descData =
	{
			{arg_cam.GetBuff(),CBV},
			{m_constBuffer,CBV},
	};
	//テクスチャ情報もセット
	for (int texIdx = 0; texIdx < s_textureNumMax; ++texIdx)descData.emplace_back(m_texBuffer[texIdx], SRV);

	KuroEngineDevice::Instance()->Graphics().ObjectRender(
		m_vertBuffer,
		descData,
		0.0f,
		true);

	m_drawParam = IndividualDrawParameter::GetDefault();
	//マスクレイヤーに描き込む設定にする
	m_drawParam.m_drawMask = 1;

	//BasicDraw::Instance()->InstancingDraw(
	//	arg_cam,
	//	arg_ligMgr,
	//	m_grassBlockModel,
	//	m_grassWorldMatArray,
	//	m_drawParam,
	//	false,
	//	AlphaBlendMode_Trans);
}

void Grass::PlantGrassBlock(KuroEngine::Transform arg_transform, KuroEngine::Vec2<float> arg_grassPosScatter, WaterPaintBlend& arg_waterPaintBlend)
{
	//円柱草を生やす処理
	m_grassWorldMatArray.push_back(arg_transform.GetMatWorld());

	//板ポリを生成。
	for (int count = 0; count < 3; ++count) {
		for (auto& index : m_vertices) {
			//生成済みだったら処理を飛ばす。
			if (index.m_isAlive) continue;

			Plant(arg_transform, arg_grassPosScatter, arg_waterPaintBlend);

			break;

		}
	}

}

void Grass::Plant(KuroEngine::Transform arg_transform, KuroEngine::Vec2<float> arg_grassPosScatter, WaterPaintBlend& arg_waterPaintBlend)
{

	KuroEngine::Vec3<float> pos = arg_transform.GetPos();

	//草を生やす位置をランダムで散らす。
	pos += arg_transform.GetRight() * KuroEngine::GetRand(-arg_grassPosScatter.x, arg_grassPosScatter.x);
	pos += arg_transform.GetFront() * KuroEngine::GetRand(-arg_grassPosScatter.y, arg_grassPosScatter.y);

	m_vertices[m_deadVertexIdx].m_isAlive = 1;
	m_vertices[m_deadVertexIdx].m_pos = pos;
	m_vertices[m_deadVertexIdx].m_normal = arg_transform.GetUp();
	m_vertices[m_deadVertexIdx].m_sineLength = KuroEngine::GetRand(40) / 100.0f;
	m_appearYTimer[m_deadVertexIdx] = 0;
	//とりあえず乱数でテクスチャ決定
	//m_vertices[m_deadVertexIdx].m_texIdx = KuroEngine::GetRand(s_textureNumMax - 1);
	m_vertices[m_deadVertexIdx].m_texIdx = KuroEngine::GetRand(3 - 1);


	m_deadVertexIdx++;

	arg_waterPaintBlend.DropMaskInk(pos + KuroEngine::Vec3<float>(0.0f, 1.0f, 0.0f));
}
bool Grass::IsGrassAround(const KuroEngine::Vec3<float> arg_playerPos)
{

	//t:生えている f:生えていない

	bool isGrassAround = false;

	for (auto& index : m_vertices) {

		//未生成だったら処理を飛ばす。
		if (!index.m_isAlive) continue;

		//ある程度離れていたら飛ばす。
		const float CLIP_OFFSET = 2.0f;
		bool isAwayX = (index.m_pos.x < arg_playerPos.x - CLIP_OFFSET) || (arg_playerPos.x + CLIP_OFFSET < index.m_pos.x);
		bool isAwayY = (index.m_pos.y < arg_playerPos.y - CLIP_OFFSET) || (arg_playerPos.y + CLIP_OFFSET < index.m_pos.y);
		if (isAwayX || isAwayY) continue;

		//距離をはかる。
		const float OFFSET = 1.0f;
		if (OFFSET < index.m_pos.Distance(arg_playerPos)) continue;

		isGrassAround = true;

		break;

	}

	return isGrassAround;

}