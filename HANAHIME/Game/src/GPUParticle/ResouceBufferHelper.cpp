//#include "ResouceBufferHelper.h"
//
//const int ResouceBufferHelper::SWAPCHAIN_NUM = 1;
//
//ResouceBufferHelper::ResouceBufferHelper() :counterBufferData(
//	CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
//	D3D12_HEAP_FLAG_NONE,
//	CD3DX12_RESOURCE_DESC::Buffer(sizeof(UINT)),
//	D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
//	nullptr,
//	"CounterBuffer"
//)
//{
//	counterBufferData.resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
//}
//
//KazGPUParticle::RESOURCE_HANDLE  ResouceBufferHelper::CreateBuffer(UINT STRUCTURE_BYTE_STRIDE,  KazPipelineData::GraphicsRangeType RANGE, KazPipelineData::GraphicsRangeType ROOTPARAM, UINT ELEMENT_NUM, bool GENERATE_COUNTER_BUFFER_FLAG)
//{
//	KazGPUParticle::RESOURCE_HANDLE  lHandle = handle.GetHandle();
//	KazGPUParticle::RESOURCE_HANDLE  lViewHandle = 0;
//	UINT lBufferSize = STRUCTURE_BYTE_STRIDE * ELEMENT_NUM;
//
//	bufferArrayData.emplace_back(ResouceBufferHelper::BufferData());
//	bufferArrayData[lHandle].rangeType = RANGE;
//	bufferArrayData[lHandle].rootParamType = ROOTPARAM;
//	bufferArrayData[lHandle].bufferSize = lBufferSize;
//	bufferArrayData[lHandle].elementNum = ELEMENT_NUM;
//
//	switch (RANGE)
//	{
//	case KazPipelineData::GRAPHICS_RANGE_TYPE_CBV_VIEW:
//		bufferArrayData[lHandle].bufferWrapper.CreateBuffer(KazBufferHelper::SetConstBufferData(lBufferSize));
//		break;
//	case KazPipelineData::GRAPHICS_RANGE_TYPE_UAV_VIEW:
//		bufferArrayData[lHandle].bufferWrapper.CreateBuffer(KazBufferHelper::SetRWStructuredBuffer(lBufferSize));
//		break;
//	case KazPipelineData::GRAPHICS_RANGE_TYPE_UAV_DESC:
//		bufferArrayData[lHandle].bufferWrapper.CreateBuffer(KazBufferHelper::SetRWStructuredBuffer(lBufferSize));
//
//
//		std::vector<KazGPUParticle::RESOURCE_HANDLE >lViewHandleArray;
//		for (int i = 0; i < SWAPCHAIN_NUM; ++i)
//		{
//			lViewHandleArray.emplace_back(UavViewHandleMgr::Instance()->GetHandle());
//			bufferArrayData[lHandle].CreateViewHandle(lViewHandleArray[i]);
//
//			if (GENERATE_COUNTER_BUFFER_FLAG)
//			{
//				bufferArrayData[lHandle].counterWrapper.CreateBuffer(counterBufferData);
//
//				DescriptorHeapMgr::Instance()->CreateBufferView(
//					lViewHandleArray[i],
//					KazBufferHelper::SetUnorderedAccessView(STRUCTURE_BYTE_STRIDE, ELEMENT_NUM),
//					bufferArrayData[lHandle].bufferWrapper.GetBuffer(i).Get(),
//					bufferArrayData[lHandle].counterWrapper.GetBuffer(i).Get()
//				);
//			}
//			else
//			{
//				DescriptorHeapMgr::Instance()->CreateBufferView(
//					lViewHandleArray[i],
//					KazBufferHelper::SetUnorderedAccessView(STRUCTURE_BYTE_STRIDE, ELEMENT_NUM),
//					bufferArrayData[lHandle].bufferWrapper.GetBuffer(i).Get(),
//					nullptr
//				);
//			}
//		}
//		break;
//	}
//
//	return lHandle;
//}
//
//KazGPUParticle::RESOURCE_HANDLE  ResouceBufferHelper::CreateBuffer(const KazBufferHelper::BufferResourceData &BUFFER_OPTION_DATA,  KazPipelineData::GraphicsRangeType RANGE, KazPipelineData::GraphicsRangeType ROOTPARAM, UINT STRUCTURE_BYTE_STRIDE, UINT ELEMENT_NUM, bool GENERATE_COUNTER_BUFFER_FLAG)
//{
//	KazGPUParticle::RESOURCE_HANDLE  lHandle = handle.GetHandle();
//
//	bufferArrayData.push_back(ResouceBufferHelper::BufferData());
//	bufferArrayData[lHandle].rangeType = RANGE;
//	bufferArrayData[lHandle].rootParamType = ROOTPARAM;
//	bufferArrayData[lHandle].bufferSize = STRUCTURE_BYTE_STRIDE * ELEMENT_NUM;
//	bufferArrayData[lHandle].bufferWrapper.CreateBuffer(BUFFER_OPTION_DATA);
//
//	switch (RANGE)
//	{
//	case KazPipelineData::GRAPHICS_RANGE_TYPE_UAV_DESC:
//		std::vector<KazGPUParticle::RESOURCE_HANDLE >lViewHandleArray;
//		for (int i = 0; i < SWAPCHAIN_NUM; ++i)
//		{
//			lViewHandleArray.emplace_back(UavViewHandleMgr::Instance()->GetHandle());
//			bufferArrayData[lHandle].CreateViewHandle(lViewHandleArray[i]);
//
//			if (GENERATE_COUNTER_BUFFER_FLAG)
//			{
//				bufferArrayData[lHandle].counterWrapper.CreateBuffer(counterBufferData);
//
//				DescriptorHeapMgr::Instance()->CreateBufferView(
//					lViewHandleArray[i],
//					KazBufferHelper::SetUnorderedAccessView(STRUCTURE_BYTE_STRIDE, ELEMENT_NUM),
//					bufferArrayData[lHandle].bufferWrapper.GetBuffer(i).Get(),
//					bufferArrayData[lHandle].counterWrapper.GetBuffer(i).Get()
//				);
//			}
//			else
//			{
//				DescriptorHeapMgr::Instance()->CreateBufferView(
//					lViewHandleArray[i],
//					KazBufferHelper::SetUnorderedAccessView(STRUCTURE_BYTE_STRIDE, ELEMENT_NUM),
//					bufferArrayData[lHandle].bufferWrapper.GetBuffer(i).Get(),
//					nullptr
//				);
//			}
//		}
//		break;
//	}
//
//	return lHandle;
//}
//
//ResouceBufferHelper::BufferData ResouceBufferHelper::CreateAndGetBuffer(UINT STRUCTURE_BYTE_STRIDE,  KazPipelineData::GraphicsRangeType RANGE, KazPipelineData::GraphicsRangeType ROOTPARAM, UINT ELEMENT_NUM, bool GENERATE_COUNTER_BUFFER_FLAG)
//{
//	ResouceBufferHelper::BufferData lBufferData;
//
//	KazGPUParticle::RESOURCE_HANDLE  lViewHandle = -1;
//	UINT lBufferSize = STRUCTURE_BYTE_STRIDE * ELEMENT_NUM;
//
//	lBufferData.rangeType = RANGE;
//	lBufferData.rootParamType = ROOTPARAM;
//	lBufferData.bufferSize = lBufferSize;
//	lBufferData.elementNum = ELEMENT_NUM;
//
//	switch (RANGE)
//	{
//	case KazPipelineData::GRAPHICS_RANGE_TYPE_CBV_VIEW:
//		lBufferData.bufferWrapper.CreateBuffer(KazBufferHelper::SetConstBufferData(lBufferSize));
//		break;
//	case KazPipelineData::GRAPHICS_RANGE_TYPE_UAV_VIEW:
//		lBufferData.bufferWrapper.CreateBuffer(KazBufferHelper::SetRWStructuredBuffer(lBufferSize));
//		break;
//	case KazPipelineData::GRAPHICS_RANGE_TYPE_UAV_DESC:
//		lBufferData.bufferWrapper.CreateBuffer(KazBufferHelper::SetRWStructuredBuffer(lBufferSize));
//
//		std::vector<KazGPUParticle::RESOURCE_HANDLE >lViewHandleArray;
//		for (int i = 0; i < SWAPCHAIN_NUM; ++i)
//		{
//			lViewHandleArray.emplace_back(UavViewHandleMgr::Instance()->GetHandle());
//			lBufferData.CreateViewHandle(lViewHandleArray[i]);
//
//			if (GENERATE_COUNTER_BUFFER_FLAG)
//			{
//				lBufferData.counterWrapper.CreateBuffer(counterBufferData);
//
//				DescriptorHeapMgr::Instance()->CreateBufferView(
//					lViewHandleArray[i],
//					KazBufferHelper::SetUnorderedAccessView(STRUCTURE_BYTE_STRIDE, ELEMENT_NUM),
//					lBufferData.bufferWrapper.GetBuffer(i).Get(),
//					lBufferData.counterWrapper.GetBuffer(i).Get()
//				);
//			}
//			else
//			{
//				DescriptorHeapMgr::Instance()->CreateBufferView(
//					lViewHandleArray[i],
//					KazBufferHelper::SetUnorderedAccessView(STRUCTURE_BYTE_STRIDE, ELEMENT_NUM),
//					lBufferData.bufferWrapper.GetBuffer(i).Get(),
//					nullptr
//				);
//			}
//		}
//
//		break;
//	}
//
//	return lBufferData;
//}
//
//ResouceBufferHelper::BufferData ResouceBufferHelper::CreateAndGetBuffer(const KazBufferHelper::BufferResourceData &BUFFER_OPTION_DATA,  KazPipelineData::GraphicsRangeType RANGE, KazPipelineData::GraphicsRangeType ROOTPARAM, UINT STRUCTURE_BYTE_STRIDE, UINT ELEMENT_NUM, bool GENERATE_COUNTER_BUFFER_FLAG)
//{
//	KazGPUParticle::RESOURCE_HANDLE  lViewHandle = 0;
//	UINT lBufferSize = STRUCTURE_BYTE_STRIDE * ELEMENT_NUM;
//
//	ResouceBufferHelper::BufferData lBufferData;
//	lBufferData.rangeType = RANGE;
//	lBufferData.rootParamType = ROOTPARAM;
//	lBufferData.bufferSize = lBufferSize;
//	lBufferData.elementNum = ELEMENT_NUM;
//
//	switch (RANGE)
//	{
//	case KazPipelineData::GRAPHICS_RANGE_TYPE_CBV_VIEW:
//		lBufferData.bufferWrapper.CreateBuffer(KazBufferHelper::SetConstBufferData(lBufferSize));
//		break;
//	case KazPipelineData::GRAPHICS_RANGE_TYPE_UAV_VIEW:
//		lBufferData.bufferWrapper.CreateBuffer(KazBufferHelper::SetRWStructuredBuffer(lBufferSize));
//		break;
//	case KazPipelineData::GRAPHICS_RANGE_TYPE_UAV_DESC:
//		lBufferData.bufferWrapper.CreateBuffer(KazBufferHelper::SetRWStructuredBuffer(lBufferSize));
//
//		std::vector<KazGPUParticle::RESOURCE_HANDLE >lViewHandleArray;
//		for (int i = 0; i < SWAPCHAIN_NUM; ++i)
//		{
//			lViewHandleArray.emplace_back(UavViewHandleMgr::Instance()->GetHandle());
//			lBufferData.CreateViewHandle(lViewHandleArray[i]);
//
//			if (GENERATE_COUNTER_BUFFER_FLAG)
//			{
//				lBufferData.counterWrapper.CreateBuffer(counterBufferData);
//
//				DescriptorHeapMgr::Instance()->CreateBufferView(
//					lViewHandleArray[i],
//					KazBufferHelper::SetUnorderedAccessView(STRUCTURE_BYTE_STRIDE, ELEMENT_NUM),
//					lBufferData.bufferWrapper.GetBuffer(i).Get(),
//					lBufferData.counterWrapper.GetBuffer(i).Get()
//				);
//			}
//			else
//			{
//				DescriptorHeapMgr::Instance()->CreateBufferView(
//					lViewHandleArray[i],
//					KazBufferHelper::SetUnorderedAccessView(STRUCTURE_BYTE_STRIDE, ELEMENT_NUM),
//					lBufferData.bufferWrapper.GetBuffer(i).Get(),
//					nullptr
//				);
//			}
//		}
//		break;
//	}
//
//	return lBufferData;
//}
//
//KazGPUParticle::RESOURCE_HANDLE  ResouceBufferHelper::SetBuffer(const ResouceBufferHelper::BufferData &DATA, KazPipelineData::GraphicsRangeType ROOTPARAM)
//{
//	KazGPUParticle::RESOURCE_HANDLE  lHandle = handle.GetHandle();
//	if (lHandle <= bufferArrayData.size())
//	{
//		bufferArrayData.push_back(DATA);
//	}
//	else
//	{
//		bufferArrayData[lHandle] = DATA;
//	}
//	bufferArrayData[lHandle].rootParamType = ROOTPARAM;
//	return lHandle;
//}
//
//void ResouceBufferHelper::TransData(KazGPUParticle::RESOURCE_HANDLE  HANDLE, void *TRANS_DATA, UINT TRANSMISSION_DATA_SIZE)
//{
//	bufferArrayData[HANDLE].bufferWrapper.TransData(TRANS_DATA, TRANSMISSION_DATA_SIZE);
//}
//
//void ResouceBufferHelper::StackToCommandListAndCallDispatch(ComputePipeLineNames NAME, const DispatchCallData &DISPATCH_DATA, UINT ADJ_NUM)
//{
//	GraphicsPipeLineMgr::Instance()->SetComputePipeLineAndRootSignature(NAME);
//	std::vector<RootSignatureParameter>lParamData = GraphicsRootSignature::Instance()->GetRootParam(static_cast<int>(NAME + ADJ_NUM));
//
//	for (int i = 0; i < bufferArrayData.size(); ++i)
//	{
//		const int L_PARAM = KazRenderHelper::SetBufferOnCmdList(lParamData, bufferArrayData[i].rangeType, bufferArrayData[i].rootParamType);
//
//		//デスクリプタヒープにコマンドリストに積む。余りが偶数ならデスクリプタヒープだと判断する
//		if (bufferArrayData[i].rangeType % 2 == 0)
//		{
//			KuroEngine::D3D12App::Instance()->GetCmdList()->SetComputeRootDescriptorTable(L_PARAM, DescriptorHeapMgr::Instance()->GetGpuDescriptorView(bufferArrayData[i].GetViewHandle()));
//			continue;
//		}
//
//		//ビューで積む際はそれぞれの種類に合わせてコマンドリストに積む
//		switch (bufferArrayData[i].rangeType)
//		{
//		case KazPipelineData::GRAPHICS_RANGE_TYPE_SRV_VIEW:
//			KuroEngine::D3D12App::Instance()->GetCmdList()->SetComputeRootShaderResourceView(L_PARAM, bufferArrayData[i].bufferWrapper.GetGpuAddress());
//			break;
//		case KazPipelineData::GRAPHICS_RANGE_TYPE_UAV_VIEW:
//			KuroEngine::D3D12App::Instance()->GetCmdList()->SetComputeRootUnorderedAccessView(L_PARAM, bufferArrayData[i].bufferWrapper.GetGpuAddress());
//			break;
//		case KazPipelineData::GRAPHICS_RANGE_TYPE_CBV_VIEW:
//			KuroEngine::D3D12App::Instance()->GetCmdList()->SetComputeRootConstantBufferView(L_PARAM, bufferArrayData[i].bufferWrapper.GetGpuAddress());
//			break;
//		default:
//			break;
//		}
//	}
//
//	KuroEngine::D3D12App::Instance()->GetCmdList()->Dispatch(DISPATCH_DATA.x, DISPATCH_DATA.y, DISPATCH_DATA.z);
//}
//
//void ResouceBufferHelper::DeleteAllData()
//{
//	handle.DeleteAllHandle();
//	bufferArrayData.clear();
//	bufferArrayData.shrink_to_fit();
//}
//
//void ResouceBufferHelper::InitCounterBuffer(const Microsoft::WRL::ComPtr<ID3D12Resource> &INIT_DATA)
//{
//	for (int i = 0; i < bufferArrayData.size(); ++i)
//	{
//		if (bufferArrayData[i].counterWrapper.GetBuffer())
//		{
//			KuroEngine::D3D12App::Instance()->GetCmdList()->ResourceBarrier(
//				1,
//				&CD3DX12_RESOURCE_BARRIER::Transition(bufferArrayData[i].counterWrapper.GetBuffer().Get(),
//					D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
//					D3D12_RESOURCE_STATE_COPY_DEST
//				)
//			);
//
//			KuroEngine::D3D12App::Instance()->GetCmdList()->CopyResource(bufferArrayData[i].counterWrapper.GetBuffer().Get(), INIT_DATA.Get());
//
//			KuroEngine::D3D12App::Instance()->GetCmdList()->ResourceBarrier(
//				1,
//				&CD3DX12_RESOURCE_BARRIER::Transition(bufferArrayData[i].counterWrapper.GetBuffer().Get(),
//					D3D12_RESOURCE_STATE_COPY_DEST,
//					D3D12_RESOURCE_STATE_UNORDERED_ACCESS
//				)
//			);
//		}
//	}
//}
//
//const ResouceBufferHelper::BufferData &ResouceBufferHelper::GetBufferData(KazGPUParticle::RESOURCE_HANDLE  HANDLE)const
//{
//	return bufferArrayData[HANDLE];
//}
//
//void ResouceBufferHelper::SetRootParam(KazGPUParticle::RESOURCE_HANDLE  HANDLE, KazPipelineData::GraphicsRangeType ROOT_PARAM)
//{
//	bufferArrayData[HANDLE].rootParamType = ROOT_PARAM;
//}