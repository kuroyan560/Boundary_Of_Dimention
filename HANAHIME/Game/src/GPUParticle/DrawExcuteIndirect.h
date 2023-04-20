#pragma once
#include"DirectX12/D3D12Data.h"
#include"KuroEngineDevice.h"

struct InitDrawIndexedExcuteIndirect
{
	std::vector<D3D12_INDIRECT_ARGUMENT_DESC> argument;
	D3D12_GPU_VIRTUAL_ADDRESS updateView;
	UINT elementNum;
	std::shared_ptr<KuroEngine::VertexBuffer> particleVertex;
	std::shared_ptr<KuroEngine::IndexBuffer> particleIndex;
	UINT indexNum;
	Microsoft::WRL::ComPtr<ID3D12RootSignature>rootsignature;
};

struct InitDrawExcuteIndirect
{
	std::vector<D3D12_INDIRECT_ARGUMENT_DESC> argument;
	D3D12_GPU_VIRTUAL_ADDRESS updateView;
	UINT elementNum;
	std::shared_ptr<KuroEngine::VertexBuffer> particleVertex;
	UINT vertNum;
	Microsoft::WRL::ComPtr<ID3D12RootSignature>rootsignature;
};

/// <summary>
/// ExcuteIndirect���ȒP�Ɏg�p����ׂ̃N���X
/// </summary>
class DrawExcuteIndirect
{
public:
	DrawExcuteIndirect(const InitDrawIndexedExcuteIndirect &INIT_DATA);
	DrawExcuteIndirect(const InitDrawExcuteIndirect &INIT_DATA);
	void Draw(std::shared_ptr<KuroEngine::GraphicsPipeline> pipeline, const Microsoft::WRL::ComPtr<ID3D12Resource> &COUNTER_BUFFER);

private:

	struct DrawIndexedIndirectCommand
	{
		D3D12_GPU_VIRTUAL_ADDRESS view;
		D3D12_DRAW_INDEXED_ARGUMENTS drawArguments;
	};
	struct DrawIndirectCommand
	{
		D3D12_GPU_VIRTUAL_ADDRESS view;
		D3D12_DRAW_ARGUMENTS drawArguments;
	};

	std::shared_ptr<KuroEngine::RWStructuredBuffer>m_cmdBuffer;
	std::shared_ptr<KuroEngine::IndirectCommandBuffer>m_indirectCmdBuffer;
	std::shared_ptr<KuroEngine::IndirectDevice> m_indirectDevice;

	Microsoft::WRL::ComPtr<ID3D12CommandSignature> commandSig;
	InitDrawIndexedExcuteIndirect initData;
	D3D12_INDIRECT_ARGUMENT_TYPE drawType;


	std::shared_ptr<KuroEngine::TextureBuffer>m_particleTexture;


};

