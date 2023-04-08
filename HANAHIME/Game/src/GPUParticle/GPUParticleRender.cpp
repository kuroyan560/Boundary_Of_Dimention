#include "GPUParticleRender.h"
#include"../KazLibrary/RenderTarget/RenderTargetStatus.h"

GPUParticleRender::GPUParticleRender(int MAXNUM)
{
	particleMaxNum = MAXNUM;

	worldMatHandle = computeCovertWorldMatToDrawMat.CreateBuffer(
		KazBufferHelper::SetOnlyReadStructuredBuffer(sizeof(InputData) * particleMaxNum),
		GRAPHICS_RANGE_TYPE_UAV_DESC,
		GRAPHICS_PRAMTYPE_DATA,
		sizeof(InputData),
		particleMaxNum,
		true);

	outputHandle = computeCovertWorldMatToDrawMat.CreateBuffer(
		KazBufferHelper::SetOnlyReadStructuredBuffer(sizeof(OutputData) * particleMaxNum),
		GRAPHICS_RANGE_TYPE_UAV_DESC,
		GRAPHICS_PRAMTYPE_DATA2,
		sizeof(OutputData),
		particleMaxNum,
		true);

	viewProjMatHandle = computeCovertWorldMatToDrawMat.CreateBuffer(
		sizeof(DirectX::XMMATRIX),
		GRAPHICS_RANGE_TYPE_CBV_VIEW,
		GRAPHICS_PRAMTYPE_DATA3,
		1,
		false);






	std::array<Vertex, 4>lVerticesArray;
	std::array<USHORT, 6> lIndicesArray;
	lIndicesArray = KazRenderHelper::InitIndciesForPlanePolygon();
	KazRenderHelper::InitVerticesPos(&lVerticesArray[0].pos, &lVerticesArray[1].pos, &lVerticesArray[2].pos, &lVerticesArray[3].pos, { 0.5f,0.5f });
	KazRenderHelper::InitUvPos(&lVerticesArray[0].uv, &lVerticesArray[1].uv, &lVerticesArray[2].uv, &lVerticesArray[3].uv);

	BUFFER_SIZE lVertBuffSize = KazBufferHelper::GetBufferSize<BUFFER_SIZE>(lVerticesArray.size(), sizeof(Vertex));
	BUFFER_SIZE lIndexBuffSize = KazBufferHelper::GetBufferSize<BUFFER_SIZE>(lIndicesArray.size(), sizeof(UINT));


	vertexBuffer = std::make_unique<KazRenderHelper::ID3D12ResourceWrapper>();
	indexBuffer = std::make_unique<KazRenderHelper::ID3D12ResourceWrapper>();


	vertexBuffer->CreateBuffer(KazBufferHelper::SetVertexBufferData(lVertBuffSize));
	indexBuffer->CreateBuffer(KazBufferHelper::SetIndexBufferData(lIndexBuffSize));

	gpuVertexBuffer.CreateBuffer(KazBufferHelper::SetGPUBufferData(lVertBuffSize));
	gpuIndexBuffer.CreateBuffer(KazBufferHelper::SetGPUBufferData(lIndexBuffSize));

	vertexBuffer->TransData(lVerticesArray.data(), lVertBuffSize);
	indexBuffer->TransData(lIndicesArray.data(), lIndexBuffSize);

	gpuVertexBuffer.CopyBuffer(vertexBuffer->GetBuffer(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_DEST);
	gpuIndexBuffer.CopyBuffer(indexBuffer->GetBuffer(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_DEST);



	InitDrawIndexedExcuteIndirect lInitData;
	lInitData.vertexBufferView = KazBufferHelper::SetVertexBufferView(gpuVertexBuffer.GetGpuAddress(), lVertBuffSize, sizeof(lVerticesArray[0]));
	lInitData.indexBufferView = KazBufferHelper::SetIndexBufferView(gpuIndexBuffer.GetGpuAddress(), lIndexBuffSize);
	lInitData.indexNum = static_cast<UINT>(lIndicesArray.size());
	lInitData.elementNum = particleMaxNum;
	lInitData.updateView = computeCovertWorldMatToDrawMat.GetBufferData(outputHandle).bufferWrapper.GetBuffer()->GetGPUVirtualAddress();
	lInitData.rootsignatureName = ROOTSIGNATURE_DATA_DRAW_UAV;

	std::array<D3D12_INDIRECT_ARGUMENT_DESC, 2> args{};
	args[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_UNORDERED_ACCESS_VIEW;
	args[0].UnorderedAccessView.RootParameterIndex = 0;
	args[1].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;

	lInitData.argument.push_back(args[0]);
	lInitData.argument.push_back(args[1]);
	excuteIndirect = std::make_unique<DrawExcuteIndirect>(lInitData);

	UINT lNum = 0;
	KazBufferHelper::BufferResourceData lBufferData
	(
		CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		CD3DX12_RESOURCE_DESC::Buffer(sizeof(UINT)),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		"CopyCounterBuffer"
	);

	copyBuffer.CreateBuffer(lBufferData);
	copyBuffer.TransData(&lNum, sizeof(UINT));
}

void GPUParticleRender::InitCount()
{
	computeCovertWorldMatToDrawMat.InitCounterBuffer(copyBuffer.GetBuffer());
}

void GPUParticleRender::Draw(const DirectX::XMUINT3 &NUM)
{
	viewProjMat = CameraMgr::Instance()->GetViewMatrix() * CameraMgr::Instance()->GetPerspectiveMatProjection();
	computeCovertWorldMatToDrawMat.TransData(viewProjMatHandle, &viewProjMat, sizeof(DirectX::XMMATRIX));

	computeCovertWorldMatToDrawMat.StackToCommandListAndCallDispatch(PIPELINE_COMPUTE_NAME_CONVERT_WORLDMAT_TO_DRAWMAT, { NUM.x,NUM.y,NUM.z });

	excuteIndirect->Draw(PIPELINE_NAME_GPUPARTICLE, nullptr);
}

const ResouceBufferHelper::BufferData &GPUParticleRender::GetStackBuffer()const
{
	return computeCovertWorldMatToDrawMat.GetBufferData(worldMatHandle);
}