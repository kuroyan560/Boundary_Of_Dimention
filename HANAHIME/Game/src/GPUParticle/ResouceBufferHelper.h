//#pragma once
//#include"GPUParticleData.h"
//#include"HandleMaker.h"
//#include"DirectX12/D3D12Data.h"
//#include<vector>
//
//struct DispatchCallData
//{
//	UINT x, y, z;
//};
//
//class ResouceBufferHelper
//{
//public:
//
//	struct BufferData
//	{
//		KazGPUParticle::ID3D12ResourceWrapper bufferWrapper;
//		KazGPUParticle::ID3D12ResourceWrapper counterWrapper;
//		 KazPipelineData::GraphicsRangeType rangeType;
//		KazPipelineData::GraphicsRangeType rootParamType;
//		UINT bufferSize;
//		UINT elementNum;
//
//		BufferData(const KazBufferHelper::BufferResourceData &BUFFER_DATA) :rangeType(KazPipelineData::GRAPHICS_RANGE_TYPE_NONE), rootParamType(KazPipelineData::GRAPHICS_PRAMTYPE_NONE), bufferSize(0), elementNum(0)
//		{
//			bufferWrapper.CreateBuffer(BUFFER_DATA);
//		}
//		BufferData() :rangeType(KazPipelineData::GRAPHICS_RANGE_TYPE_NONE), rootParamType(KazPipelineData::GRAPHICS_PRAMTYPE_NONE), bufferSize(0), elementNum(0)
//		{
//		}
//
//		void CreateViewHandle(std::vector<KazGPUParticle::RESOURCE_HANDLE >HANDLE_ARRAY)
//		{
//			viewHandle = HANDLE_ARRAY;
//		}
//		void CreateViewHandle(KazGPUParticle::RESOURCE_HANDLE  HANDLE)
//		{
//			viewHandle.emplace_back(HANDLE);
//		}
//		const KazGPUParticle::RESOURCE_HANDLE  &GetViewHandle()const
//		{
//			return viewHandle[0];
//		}
//
//		void operator=(const BufferData &rhs)
//		{
//			rangeType = rhs.rangeType;
//			rootParamType = rhs.rootParamType;
//			bufferSize = rhs.bufferSize;
//			bufferWrapper = rhs.bufferWrapper;
//			counterWrapper = rhs.counterWrapper;
//			viewHandle = rhs.viewHandle;
//			elementNum = rhs.elementNum;
//		};
//
//	private:
//		std::vector<KazGPUParticle::RESOURCE_HANDLE > viewHandle;
//	};
//
//	ResouceBufferHelper();
//
//	KazGPUParticle::RESOURCE_HANDLE  CreateBuffer(UINT STRUCTURE_BYTE_STRIDE, KazPipelineData::GraphicsRangeType RANGE, KazPipelineData::GraphicsRangeType ROOTPARAM, UINT ELEMENT_NUM, bool GENERATE_COUNTER_BUFFER_FLAG = false);
//	KazGPUParticle::RESOURCE_HANDLE  CreateBuffer(const KazBufferHelper::BufferResourceData &BUFFER_OPTION_DATA,  KazPipelineData::GraphicsRangeType RANGE, KazPipelineData::GraphicsRangeType ROOTPARAM, UINT STRUCTURE_BYTE_STRIDE, UINT ELEMENT_NUM, bool GENERATE_COUNTER_BUFFER_FLAG = false);
//
//	ResouceBufferHelper::BufferData CreateAndGetBuffer(UINT STRUCTURE_BYTE_STRIDE,  KazPipelineData::GraphicsRangeType RANGE, KazPipelineData::GraphicsRangeType ROOTPARAM, UINT ELEMENT_NUM, bool GENERATE_COUNTER_BUFFER_FLAG = false);
//	ResouceBufferHelper::BufferData CreateAndGetBuffer(const KazBufferHelper::BufferResourceData &BUFFER_OPTION_DATA,  KazPipelineData::GraphicsRangeType RANGE, KazPipelineData::GraphicsRangeType ROOTPARAM, UINT STRUCTURE_BYTE_STRIDE, UINT ELEMENT_NUM, bool GENERATE_COUNTER_BUFFER_FLAG = false);
//
//	KazGPUParticle::RESOURCE_HANDLE  SetBuffer(const ResouceBufferHelper::BufferData &DATA, KazPipelineData::GraphicsRangeType ROOTPARAM);
//
//	void TransData(KazGPUParticle::RESOURCE_HANDLE  HANDLE, void *TRANS_DATA, UINT TRANSMISSION_DATA_SIZE);
//
//	void StackToCommandListAndCallDispatch(ComputePipeLineNames NAME, const DispatchCallData &DISPATCH_DATA, UINT ADJ_NUM = 0);
//
//	void DeleteAllData();
//
//
//	void InitCounterBuffer(const Microsoft::WRL::ComPtr<ID3D12Resource> &INIT_DATA);
//	const ResouceBufferHelper::BufferData &GetBufferData(KazGPUParticle::RESOURCE_HANDLE  HANDLE)const;
//	void SetRootParam(KazGPUParticle::RESOURCE_HANDLE  HANDLE, KazPipelineData::GraphicsRangeType ROOT_PARAM);
//
//private:
//	std::vector<ResouceBufferHelper::BufferData>bufferArrayData;
//	HandleMaker handle;
//
//	static const int SWAPCHAIN_NUM;
//
//
//	KazBufferHelper::BufferResourceData counterBufferData;
//
//	std::vector<KuroEngine::RegisterDescriptorData>registerDataArray;
//};