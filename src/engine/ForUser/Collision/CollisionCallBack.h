#pragma once
#include"Common/Vec.h"
#include<memory>

namespace KuroEngine
{
	class Collider;
	class CollisionResultInfo;

	//衝突判定があった際に呼び出される
	class CollisionCallBack
	{
	private:
		friend class CollisionManager;

	protected:
		/// <summary>
		/// 衝突中呼び出される
		/// </summary>
		/// <param name="arg_info">衝突結果情報</param>
		/// <param name="arg_myCollider">自分側のコライダー</param>
		/// <param name="arg_otherCollider">相手側のコライダー</param>
		virtual void OnCollisionEnter(
			const CollisionResultInfo& arg_info,
			std::weak_ptr<Collider>arg_myCollider,
			std::weak_ptr<Collider>arg_otherCollider) = 0;

		/// <summary>
		/// 衝突した瞬間呼び出される
		/// </summary>
		/// <param name="arg_info">衝突結果情報</param>
		/// <param name="arg_myCollider">自分側のコライダー</param>
		/// <param name="arg_otherCollider">相手側のコライダー</param>
		virtual void OnCollisionTrigger(
			const CollisionResultInfo& arg_info,
			std::weak_ptr<Collider>arg_myCollider,
			std::weak_ptr<Collider>arg_otherCollider) = 0;

	public:
		virtual ~CollisionCallBack() {}
		virtual CollisionCallBack* CallBackClone()
		{
			//設定なし、クローン生成出来ない
			assert(0);
			return nullptr;
		}
	};
}