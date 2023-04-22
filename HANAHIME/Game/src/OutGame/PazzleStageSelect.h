#pragma once
#include"Common/Vec.h"
#include"Framework/UsersInput.h"
#include<vector>
#include<array>
#include"Select.h"
#include"ForUser/DrawFunc/2D/DrawFunc2D.h"
#include"DirectX12/D3D12App.h"
#include"ForUser/Timer.h"

/// <summary>
/// タイトルから直接ステージを選択する
/// </summary>
class PazzleStageSelect
{
public:
	PazzleStageSelect();

	void Init();
	void Update();
	void Draw();

	int GetNumber();
	bool IsEnableToSelect()
	{
		return m_stageSelectArray[m_nowStageNum.y][m_nowStageNum.x].enableFlag;
	}
	void Stop()
	{
		m_stopFlag = true;
	}
private:

	struct StageData
	{
		bool m_isClearFlag;
		bool enableFlag;	//遊べるステージかどうか
		StageData() :m_isClearFlag(false)
		{};
	};
	std::array<std::array<StageData, 30>, 1>m_stageSelectArray;
	KuroEngine::Vec2<int> m_nowStageNum;

	KuroEngine::Vec2<float> m_prevContollerLeftStick;

	std::vector<std::shared_ptr<KuroEngine::TextureBuffer>>m_numTexArray;
	std::vector<int> CountNumber(int TIME)
	{
		std::vector<int> Number(2);
		Number[0] = -1;
		Number[1] = -1;

		int tmp = TIME;
		//スコア計算
		for (int i = 0; tmp > 0; i++)
		{
			int result = tmp % 10;
			//Number.push_back(result);
			Number[i] = result;
			tmp /= 10;
		}
		//0埋め
		for (int i = 0; i < Number.size(); i++)
		{
			if (Number[i] == -1)
			{
				Number[i] = 0;
			}
		}
		std::reverse(Number.begin(), Number.end());
		return Number;
	}

	//ステージ選択画面を何処に描画するか(左上座標)
	KuroEngine::Vec2<float> m_baseStageSelectPos;
	bool m_stopFlag;

	std::shared_ptr<KuroEngine::TextureBuffer>m_selectTex;




	//上下の帯
	class Band
	{
	public:

		Band(const KuroEngine::Vec2<float> &pos, const KuroEngine::Vec2<float> &size, const KuroEngine::Vec2<float> &easeVel, float flame) :
			m_pos(pos), m_size(size), m_easeVel(easeVel), m_appearTimer(flame), m_disappearTimer(flame), m_appearFlag(true)
		{
		};

		void Update()
		{
			if (m_appearFlag)
			{
				m_disappearTimer.Reset();
				m_hideVel = KuroEngine::Math::Ease(KuroEngine::Out, KuroEngine::Circ, m_appearTimer.GetTimeRate(), KuroEngine::Vec2<float>(0.0f, 0.0f), m_easeVel);
				m_appearTimer.UpdateTimer();
			}
			else
			{
				m_appearTimer.Reset();
				m_hideVel = KuroEngine::Math::Ease(KuroEngine::Out, KuroEngine::Circ, m_disappearTimer.GetTimeRate(), m_easeVel, KuroEngine::Vec2<float>(0.0f, 0.0f));
				m_disappearTimer.UpdateTimer();
			}
			m_appearFlag = false;
		};

		void Draw()
		{
			KuroEngine::DrawFunc2D::DrawBox2D(m_pos + m_hideVel, m_pos + m_size, KuroEngine::Color(18, 43, 38, 255), true);
		};

		void Appear()
		{
			m_appearFlag = true;
		};

	private:
		KuroEngine::Vec2<float> m_pos;
		KuroEngine::Vec2<float> m_size;
		KuroEngine::Vec2<float> m_easeVel;
		KuroEngine::Vec2<float> m_hideVel;
		KuroEngine::Timer m_appearTimer;
		KuroEngine::Timer m_disappearTimer;

		bool m_appearFlag;
	};
	std::array<std::unique_ptr<Band>, 2>m_bandArray;
};