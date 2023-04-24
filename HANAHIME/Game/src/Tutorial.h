#pragma once
#include"ForUser/Object/Object.h"
#include"ForUser/DrawFunc/BillBoard/DrawFuncBillBoard.h"
#include"GPUParticle/SignSpotFireFly.h"

/// <summary>
/// チュートリアル表示用
/// </summary>
class Tutorial
{
public:
	Tutorial(std::shared_ptr<KuroEngine::RWStructuredBuffer> particleBuffer);

	void Update();
	void Draw(KuroEngine::Camera &camera);

	void Finish();
	void Next();

private:

	struct TutorialData
	{
		KuroEngine::Vec3<float>m_pos;
		std::vector<std::shared_ptr<KuroEngine::TextureBuffer>>m_resource;
		KuroEngine::Timer flameTimer;

		TutorialData(const KuroEngine::Vec3<float> &pos, float flame, std::string filepass, int allNum, const KuroEngine::Vec2<int> &splitNum) :m_pos(pos), flameTimer(flame), texNum(0)
		{
			m_resource.resize(allNum);
			KuroEngine::D3D12App::Instance()->GenerateTextureBuffer(m_resource.data(), filepass, allNum, splitNum);
		};

		int GetTexHandle()
		{
			if (flameTimer.IsTimeUp())
			{
				++texNum;
				flameTimer.Reset();
			}
			if (m_resource.size() <= texNum)
			{
				texNum = 0;
			}
			flameTimer.UpdateTimer();

			return texNum;
		};

		std::shared_ptr<KuroEngine::TextureBuffer> GetTex()
		{
			return m_resource[GetTexHandle()];
		};

		int texNum;
	};
	std::vector<TutorialData>m_tutorialDataArray;


	int m_nowIndex, m_prevNowIndex, m_texPosIndex;
	KuroEngine::Timer m_changeTimer;

	KuroEngine::Vec3<float>m_nowScale;
	KuroEngine::Transform m_nowTransform;

	bool m_changeUIFlag;

	//チュートリアル用の蛍
	SignSpotFireFly m_fireFly;
};

