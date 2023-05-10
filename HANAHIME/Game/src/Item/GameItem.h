#pragma once
#include"KuroEngine.h"
#include<vector>
#include<string>
#include<typeinfo>
#include<iostream>
#include"Common/Transform.h"
#include"Common/Singleton.h"
#include"../CollisionDetection.h"
#include"DirectX12/D3D12App.h"
#include"Render/RenderObject/Camera.h"

enum ItemType
{
	ITEM_NONE = -1,
	ITEM_HEAL,
	ITEM_BUFF,
	ITEM_MAX
};

struct HealData
{
	int m_heal = 120;
};

struct BuffData
{
	int m_buff = 60;
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
		m_itemArray[ITEM_BUFF] = std::make_unique<ItemInterface<BuffData>>(ITEM_BUFF);
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
	class ItemData
	{
	public:
		ItemType m_itemEnum;				//アイテムの種類
		bool m_enbaleToGetFlag;				//判定を有効にするかどうか
		KuroEngine::Transform m_transform;	//座標
		float m_radius;						//アイテムの判定
		Sphere m_hitBox;					//アイテムの判定
		IDatabase *m_itemInfomation;		//アイテム情報

		ItemData(IDatabase *itemInfo, ItemType type) :m_itemInfomation(itemInfo), m_enbaleToGetFlag(false), m_itemEnum(type), m_radius(5.0f)
		{
			m_hitBox.m_centerPos = &m_transform.GetPos();
			m_hitBox.m_radius = &m_radius;

			switch (m_itemEnum)
			{
			case ITEM_NONE:
				break;
			case ITEM_HEAL:
				m_itemTex = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/reaction/Find.png");
				break;
			case ITEM_BUFF:
				m_itemTex = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/reaction/hatena.png");
				break;
			case ITEM_MAX:
				break;
			default:
				break;
			}
		}

		//アイテムの仮描画
		std::shared_ptr<KuroEngine::TextureBuffer>m_itemTex;

		void Update();
		void Draw(KuroEngine::Camera &camera);
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
		for (auto &obj : m_itemArray)
		{
			obj->Update();
		}
	};

	void Draw(KuroEngine::Camera &camera)
	{
		for (auto &obj : m_itemArray)
		{
			obj->Draw(camera);
		}
	};


private:
	std::vector<std::shared_ptr<ItemData>>m_itemArray;
};