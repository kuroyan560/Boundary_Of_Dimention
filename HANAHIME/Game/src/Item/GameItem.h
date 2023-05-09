#pragma once
#include"KuroEngine.h"
#include<vector>
#include<string>
#include<typeinfo>
#include<iostream>
#include"Common/Transform.h"
#include"Common/Singleton.h"

enum Item
{
	ITEM_NONE = -1,
	ITEM_HEAL,
	ITEM_BUFF,
	ITEM_MAX
};

struct HealData
{
	int m_heal;
};

class IDatabase
{
public:
	virtual void Get() = 0;

	enum class OWNERSHIP
	{
		NONE,
		OWER_ENEMY,
		OWER_PLAYER,
		OWER_FILED,
		OWER_MAX
	};

	OWNERSHIP ownerShip;
};


template<typename T>
class ItemInterface :public IDatabase
{
public:
	ItemInterface()
	{
		m_className = typeid(T).name();
	};
	T m_itemData;
	std::string m_className;

	void Get() {};
};

/// <summary>
/// アイテムの情報
/// </summary>
class ItemDatabase :public KuroEngine::DesignPattern::Singleton<ItemDatabase>
{
public:

	ItemDatabase()
	{
		m_itemArray.resize(ITEM_MAX);
		m_itemArray[ITEM_HEAL] = std::make_unique<ItemInterface<HealData>>();
	}

	IDatabase *GetData(Item Get)const
	{
		return m_itemArray[Get].get();
	};

private:
	std::vector<std::unique_ptr<IDatabase>>m_itemArray;
};

/// <summary>
/// フィールド内で生成されるアイテム
/// </summary>
class ItemOnGame :public KuroEngine::DesignPattern::Singleton<ItemOnGame>
{
public:
	ItemOnGame() {};

	/// <summary>
	/// ゲーム内のアイテムの固有情報+アイテム情報
	/// </summary>
	struct ItemData
	{
		Item m_itemEnum;
		bool m_enbaleToGetFlag;
		KuroEngine::Transform m_transform;
		IDatabase *m_itemInfomation;
		ItemData(IDatabase *itemInfo, Item type) :m_itemInfomation(itemInfo), m_enbaleToGetFlag(false), m_itemEnum(type)
		{
		}
	};

	std::shared_ptr<ItemData>Generate(IDatabase *itemData)
	{
		m_itemArray.emplace_back(std::make_shared<ItemData>(itemData, ITEM_BUFF));
		return m_itemArray.back();
	};

	bool Hit(KuroEngine::Transform transform, int *hitIndex)
	{
		for (auto &obj : m_itemArray)
		{
			//アイテムが有効でなければ判定は取らない
			if (!obj->m_enbaleToGetFlag)
			{
				continue;
			}

			//衝突判定
			if (false)
			{
				*hitIndex = static_cast<int>(&obj - &m_itemArray[0]);
				return true;
			}
		}
		return false;
	};


private:
	std::vector<std::shared_ptr<ItemData>>m_itemArray;
};