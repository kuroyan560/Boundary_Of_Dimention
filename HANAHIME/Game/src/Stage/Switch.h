#pragma once
#include<vector>
#include<memory>
class Lever;

//スイッチ（レバーが全てオンで起動）
class Switch
{
	friend class Stage;

	//レバーの識別番号
	int m_leverID = -1;

	//レバー配列
	std::vector<std::weak_ptr<Lever>>m_leverArray;

	//アクティブ状態（オンオフ切り替わる状態か）
	bool m_isFixed = false;

public:
	void SetFixed(bool arg_flg)
	{
		m_isFixed = arg_flg;
	}

	//全てのレバーがオンで起動中か
	bool IsBooting()const;

	//状態が固定されているか（レバーを動かせない）
	const bool& IsFixed()const
	{
		return m_isFixed;
	}
};
