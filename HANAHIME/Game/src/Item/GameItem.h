#pragma once
#include"KuroEngine.h"
#include<vector>
#include<string>
#include<typeinfo>
#include<iostream>
#include"Common/Transform.h"
#include"Common/Singleton.h"
#include"../CollisionDetection.h"

enum ItemType
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

	ItemType item;
	OWNERSHIP ownerShip;
};


template<typename T>
class ItemInterface :public IDatabase
{
public:
	ItemInterface(ItemType itemType)
	{
		item = itemType;
	};
	T m_itemData;

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
		m_itemArray[ITEM_HEAL] = std::make_unique<ItemInterface<HealData>>(ITEM_HEAL);
	}

	IDatabase *GetData(ItemType Get)const
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
		ItemType m_itemEnum;
		bool m_enbaleToGetFlag;
		KuroEngine::Transform m_transform;
		float m_radius;
		Sphere m_hitBox;
		IDatabase *m_itemInfomation;
		ItemData(IDatabase *itemInfo, ItemType type) :m_itemInfomation(itemInfo), m_enbaleToGetFlag(false), m_itemEnum(type), m_radius(5.0f)
		{
			m_hitBox.m_centerPos = &m_transform.GetPos();
			m_hitBox.m_radius = &m_radius;
		}
	};

	std::shared_ptr<ItemData>Generate(IDatabase *itemData)
	{
		m_itemArray.emplace_back(std::make_shared<ItemData>(itemData, itemData->item));
		return m_itemArray.back();
	};

	bool Hit(const Sphere &playerHitBox, int *hitIndex)
	{
		for (auto &obj : m_itemArray)
		{
			//アイテムが有効でなければ判定は取らない
			if (!obj->m_enbaleToGetFlag)
			{
				continue;
			}

			//衝突判定
			if (Collision::Instance()->CheckCircleAndCircle(playerHitBox, obj->m_hitBox))
			{
				*hitIndex = static_cast<int>(&obj - &m_itemArray[0]);
				return true;
			}
		}
		return false;
	};

	std::shared_ptr<ItemData>GetData(int index)
	{
		return m_itemArray[index];
	}

	void Update()
	{

	};

	void Draw()
	{

	};


private:
	std::vector<std::shared_ptr<ItemData>>m_itemArray;
};