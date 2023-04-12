#include "Select.h"

int Select::selectNum = -1;		//select��init����+1����̂�-1�ɂ��Ă��܂��B

Select::Select()
{
}

void Select::Init(int MAX_SELECT_NUM)
{
	maxSelectNum = MAX_SELECT_NUM;
	decreFlag = false;
	increFlag = false;
	selectNum = 0;
}

void Select::Finalize()
{
}

void Select::Update()
{
	if (0 < selectNum && decreFlag)
	{
		selectNum--;
		decreFlag = false;
	}
	else if (selectNum < maxSelectNum && increFlag)
	{
		selectNum++;
		increFlag = false;
	}
	else
	{
		decreFlag = false;
		increFlag = false;
	}
}

void Select::Draw()
{
}

void Select::Input(bool INCRE, bool DECRE)
{
	if (INCRE)
	{
		increFlag = true;
	}
	if (DECRE)
	{
		decreFlag = true;
	}
}

int Select::GetNumber()
{
	return selectNum;
}

int Select::GetMaxNumber()
{
	return maxSelectNum;
}

void Select::SelectNextStage()
{
	selectNum++;
	//�ő�l�𒴂��Ă�����ő�ɐݒ肷��
	if (selectNum > maxSelectNum) {
		selectNum = maxSelectNum;
	}
}
