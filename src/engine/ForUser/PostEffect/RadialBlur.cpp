#include "RadialBlur.h"
#include"KuroEngineDevice.h"

std::shared_ptr<KuroEngine::ComputePipeline>KuroEngine::RadialBlur::RADIAL_PIPELINE;

void KuroEngine::RadialBlur::GeneratePipeline()
{
	if (!RADIAL_PIPELINE)
	{
        std::vector<RootParam>rootParam =
        {
            RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"ブラー情報"),
            RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,"ソース画像バッファ"),
            RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_UAV,"描き込み先バッファ")
        };

        auto cs = D3D12App::Instance()->CompileShader("resource/engine/RadialBlur.hlsl", "CSmain", "cs_5_0");
        RADIAL_PIPELINE = D3D12App::Instance()->GenerateComputePipeline(cs, rootParam, { WrappedSampler(false, true) });
	}
}

KuroEngine::RadialBlur::RadialBlur(const Vec2<int>& Size, const DXGI_FORMAT& Format, const float& BlurPower, const Vec2<float>& CenterUV)
{
    GeneratePipeline();

    //重みテーブル定数バッファ
    constData.texSize = Size;
    constData.blurPower = BlurPower;
    constData.centerUV = CenterUV;
    constBuff = D3D12App::Instance()->GenerateConstantBuffer(sizeof(constData), 1, &constData, "RadialBlur - ConstBuffer");

    //最終合成結果描画先
    finalResult = D3D12App::Instance()->GenerateTextureBuffer(Size, Format, "RadialBlur");
}

void KuroEngine::RadialBlur::SetBlurPower(const float& BlurPower)
{
    constData.blurPower = BlurPower;
    constBuff->Mapping(&constData);
}

void KuroEngine::RadialBlur::SetCenterUV(const Vec2<float>& CenterUV)
{
    constData.centerUV = CenterUV;
    constBuff->Mapping(&constData);
}

void KuroEngine::RadialBlur::Excute(const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& CmdList, const std::shared_ptr<TextureBuffer>& SourceTex)
{
    const auto& sDesc = SourceTex->GetDesc();
    const auto& fDesc = finalResult->GetDesc();

    static const int DIV = 4;

    //方向ブラー
    RADIAL_PIPELINE->SetPipeline(CmdList);
    constBuff->SetComputeDescriptorBuffer(CmdList, CBV, 0);
    SourceTex->SetComputeDescriptorBuffer(CmdList, SRV, 1);
    finalResult->SetComputeDescriptorBuffer(CmdList, UAV, 2);
    CmdList->Dispatch(static_cast<UINT>(finalResult->GetDesc().Width / DIV + 1), static_cast<UINT>(finalResult->GetDesc().Height / DIV + 1), 1);
}
