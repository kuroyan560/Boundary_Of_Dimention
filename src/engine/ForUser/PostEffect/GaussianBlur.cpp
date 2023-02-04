#include "GaussianBlur.h"
#include"KuroEngineDevice.h"
#include"ForUser/DrawFunc/2D/DrawFunc2D.h"

void KuroEngine::GaussianBlur::GeneratePipeline(std::array<std::shared_ptr<GraphicsPipeline>, PROCESS_NUM - 1>& arg_destPipeline, const DXGI_FORMAT& arg_format)
{
    PipelineInitializeOption option(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    option.m_depthWriteMask = false;
    option.m_dsvFormat = DXGI_FORMAT_UNKNOWN;

    std::vector<RootParam>rootParam =
    {
        RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"平行投影行列"),
        RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"重みテーブル"),
        RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,"ソース画像バッファ"),
    };

    std::array<std::string, PROCESS_NUM - 1>vsEntoryPoint = { "HorizontalMain","VerticalMain" };

    std::vector<RenderTargetInfo>renderTarget = { RenderTargetInfo(arg_format,AlphaBlendMode_None) };

    auto sampler = WrappedSampler(false, true);


    for (int processIdx = 0; processIdx < PROCESS_NUM - 1; ++processIdx)
    {
        Shaders shaders;
        shaders.m_vs = D3D12App::Instance()->CompileShader("resource/engine/GaussianBlur.hlsl", vsEntoryPoint[processIdx], "vs_6_4");
        shaders.m_ps = D3D12App::Instance()->CompileShader("resource/engine/GaussianBlur.hlsl", "PSmain", "ps_6_4");

        arg_destPipeline[processIdx] = D3D12App::Instance()->GenerateGraphicsPipeline(
            option, 
            shaders,
            KuroEngine::SpriteMesh::Vertex::GetInputLayout(),
            rootParam,
            renderTarget,
            { WrappedSampler(false,true) }
        );
    }
}

KuroEngine::GaussianBlur::GaussianBlur(const Vec2<int>& arg_size, const DXGI_FORMAT& arg_format, const float& arg_blurPower)
{
    GeneratePipeline(m_gPipeline, arg_format);

    //重みテーブル定数バッファ
    m_weightConstBuff = D3D12App::Instance()->GenerateConstantBuffer(sizeof(float), s_weightNum, nullptr, "GaussianBlur - weight");
    SetBlurPower(arg_blurPower);

    //縦横ブラーの結果描画先
    Vec2<int> xBlurTexSize = { arg_size.x / 2 , arg_size.y };
    m_blurResult[X_BLUR] = D3D12App::Instance()->GenerateRenderTarget(arg_format, Color(0, 0, 0, 0), xBlurTexSize, L"GaussianBlur_HorizontalBlur_RenderTarget");
    m_spriteMeshes[X_BLUR] = std::make_unique<SpriteMesh>("GaussianBlur_HorizontalBlur");
    m_spriteMeshes[X_BLUR]->SetSize(arg_size.Float());

    Vec2<int> yBlurTexSize = { arg_size.x / 2 , arg_size.y / 2 };
    m_blurResult[Y_BLUR] = D3D12App::Instance()->GenerateRenderTarget(arg_format, Color(0, 0, 0, 0), yBlurTexSize, L"GaussianBlur_VerticalBlur_RenderTarget");
    m_spriteMeshes[Y_BLUR] = std::make_unique<SpriteMesh>("GaussianBlur_VerticalBlur");

    m_spriteMeshes[Y_BLUR]->SetSize(arg_size.Float());

    m_blurResult[FINAL] = D3D12App::Instance()->GenerateRenderTarget(arg_format, Color(0, 0, 0, 0), arg_size, L"GaussianBlur_Result_RenderTarget");
}

void KuroEngine::GaussianBlur::SetBlurPower(const float& arg_blurPower)
{
    // 重みの合計を記録する変数を定義する
    float total = 0;

    // ここからガウス関数を用いて重みを計算している
    // ループ変数のxが基準テクセルからの距離
    for (int x = 0; x < s_weightNum; x++)
    {
        m_weights[x] = expf(-0.5f * (float)(x * x) / arg_blurPower);
        total += 2.0f * m_weights[x];
    }

    // 重みの合計で除算することで、重みの合計を1にしている
    for (int i = 0; i < s_weightNum; i++)
    {
        m_weights[i] /= total;
    }

    m_weightConstBuff->Mapping(&m_weights[0]);
}

#include"ForUser/DrawFunc/2D/DrawFunc2D.h"
void KuroEngine::GaussianBlur::Register(const std::shared_ptr<TextureBuffer>& arg_srcTex)
{
    const auto& sDesc = arg_srcTex->GetDesc();
    const auto& fDesc = m_blurResult[FINAL]->GetDesc();
    assert(sDesc.Width == fDesc.Width && sDesc.Height == fDesc.Height && sDesc.Format == fDesc.Format);

    std::vector<RegisterDescriptorData>descDatas =
    {
        {KuroEngine::KuroEngineDevice::Instance()->GetParallelMatProjBuff(),CBV},
        {m_weightConstBuff,CBV},
        {arg_srcTex,SRV},
    };

    for (auto& rt : m_blurResult)KuroEngine::KuroEngineDevice::Instance()->Graphics().ClearRenderTarget(rt);

    for (int processIdx = 0; processIdx < PROCESS_NUM - 1; ++processIdx)
    {
        KuroEngine::KuroEngineDevice::Instance()->Graphics().SetGraphicsPipeline(m_gPipeline[processIdx]);

        KuroEngine::KuroEngineDevice::Instance()->Graphics().SetRenderTargets({ m_blurResult[processIdx] });

        if (0 < processIdx)
            descDatas[2].m_descData = m_blurResult[processIdx - 1];

        m_spriteMeshes[processIdx]->Render(descDatas);
    }

    KuroEngine::KuroEngineDevice::Instance()->Graphics().SetRenderTargets({ m_blurResult[FINAL] });
    KuroEngine::DrawFunc2D::DrawExtendGraph2D(
        { 0,0 },
        m_blurResult[FINAL]->GetGraphSize().Float(),
        m_blurResult[FINAL - 1],1.0f,AlphaBlendMode_None);
}

void KuroEngine::GaussianBlur::DrawResult(const AlphaBlendMode& arg_alphaBlend)
{
    KuroEngine::DrawFunc2D::DrawExtendGraph2D({ 0,0 }, KuroEngine::WinApp::Instance()->GetExpandWinSize(), m_blurResult[FINAL], 1.0f, arg_alphaBlend);
}
