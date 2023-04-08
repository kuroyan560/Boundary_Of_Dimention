#pragma once
#include"ResouceBufferHelper.h"
#include"ForUser/DrawFunc/3D/DrawFunc3D.h"
#include"GPUParticleData.h"
#include"DrawExcuteIndirect.h"

//演出で使われている全てのパーティクルの情報をスタックして一気に描画するクラスです
class GPUParticleRender
{
public:
	GPUParticleRender(int MAXNUM = 3000000);

	void InitCount();
	void Draw(const DirectX::XMUINT3 &NUM = { 3000,1,1 });

	const ResouceBufferHelper::BufferData &GetStackBuffer()const;

	struct InputData
	{
		DirectX::XMMATRIX worldMat;
		DirectX::XMFLOAT4 color;
	};
private:

	
	struct OutputData
	{
		DirectX::XMMATRIX mat;
		DirectX::XMFLOAT4 color;
	};


	int particleMaxNum = 3000000;
	ResouceBufferHelper computeCovertWorldMatToDrawMat;
	KazGPUParticle::RESOURCE_HANDLE  worldMatHandle, outputHandle, viewProjMatHandle;

	DirectX::XMMATRIX viewProjMat;

	std::unique_ptr<DrawExcuteIndirect> excuteIndirect;
	KazGPUParticle::RESOURCE_HANDLE  vertexBufferHandle;
	KazGPUParticle::RESOURCE_HANDLE  indexBufferHandle;
	std::unique_ptr<KazGPUParticle::ID3D12ResourceWrapper> vertexBuffer, indexBuffer;

	KazGPUParticle::ID3D12ResourceWrapper gpuVertexBuffer, gpuIndexBuffer;
	KazGPUParticle::ID3D12ResourceWrapper copyBuffer;
};

