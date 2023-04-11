#pragma once
class Select
{
public:
	Select();
	void Init(int MAX_SELECT_NUM);
	void Finalize();
	void Update();
	void Draw();

	void Input(bool INCRE, bool DECRE);
	int GetNumber();
	int GetMaxNumber();

	//次のステージを選択する
	void SelectNextStage();

	static int selectNum;
private:
	
	int maxSelectNum;
	bool increFlag, decreFlag;
};

