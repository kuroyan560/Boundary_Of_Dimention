#include"Common/Vec.h"
#include"Framework/UsersInput.h"
#include<vector>
#include<array>
#include"Select.h"
#include"ForUser/DrawFunc/2D/DrawFunc2D.h"
#include"DirectX12/D3D12App.h"

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
private:

	struct StageData
	{
		bool m_isClearFlag;
		bool enableFlag;	//遊べるステージかどうか
		StageData() :m_isClearFlag(false)
		{};
	};
	std::array<std::array<StageData, 4>, 4>m_stageSelectArray;
	KuroEngine::Vec2<int> m_nowStageNum;

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

};