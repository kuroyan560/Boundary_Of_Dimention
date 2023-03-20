#pragma once
#include<memory>
#include<vector>
#include<string>
#include"Common/Vec.h"
#include"Common/Transform.h"
#include<map>

namespace KuroEngine
{
	class Camera;
	class CollisionPrimitive;
	class CollisionCallBack;
	class ColliderParentObject;
	class CollisionResultInfo;

	class Collider
	{
	private:
		friend class CollisionManager;

		//付与する識別番号
		static int s_id;

		//自身がアタッチされる親オブジェクト
		ColliderParentObject* m_parentObj = nullptr;

		//識別番号
		int m_id = -1;

		//コライダー名
		std::string m_name;

		//衝突判定用プリミティブ配列
		std::vector<std::shared_ptr<CollisionPrimitive>>m_primitiveArray;

		//有効フラグ
		bool m_isActive = true;

		//過去の当たり判定フラグ記録
		bool m_oldIsHit = false;
		//当たり判定があったかフラグ
		bool m_isHit = false;

		//タグ
		std::vector<std::string> m_tags;
		//コールバックリスト、キーは相手のタグ名
		std::map<std::string, std::vector<CollisionCallBack*>>m_callBackList;

		//親トランスフォーム
		Transform* m_parentTransform;

	public:
		Collider() :m_id(s_id++) {}

		//トランスフォームだけ変えてクローンする
		Collider Clone(Transform* arg_parent, ColliderParentObject* arg_parentObj = nullptr);

		~Collider() {}

		/// <summary>
		/// コライダー初期化
		/// </summary>
		/// <param name="arg_name">コライダー名</param>
		/// <param name="arg_tags">コライダータグ名配列</param>
		/// <param name="arg_primitiveArray">コライダーにアタッチされるプリミティブ配列</param>
		void Generate(const std::string& arg_name,
			const std::vector<std::string>& arg_tags,
			const std::vector<std::shared_ptr<CollisionPrimitive>>& arg_primitiveArray);

		//当たり判定（衝突点を返す）
		bool CheckHitCollision(std::weak_ptr<Collider> arg_other, CollisionResultInfo* arg_info = nullptr);

		//当たり判定描画
		void DebugDraw(Camera& arg_cam);

		/// <summary>
		/// コールバックのセット
		/// </summary>
		/// <param name="arg_otherTag">コールバックを呼び出す条件となる相手のタグ名</param>
		/// <param name="arg_callBack">セットするコールバック</param>
		void SetCallBack(std::string arg_otherTag, CollisionCallBack* arg_callBack);
		//親トランスフォームをアタッチされている全プリミティブにセット
		void SetParentTransform(Transform* arg_parent);
		//親オブジェクトのセット
		void SetParentObject(ColliderParentObject* arg_parent);

		//アクティブフラグのセット
		void SetActive(const bool& arg_active) { m_isActive = arg_active; }

		/// <summary>
		/// 引数に渡したタグ名が付与されているか
		/// </summary>
		/// <param name="arg_tag">付与を確認するタグ名</param>
		/// <returns>結果</returns>
		bool HaveTag(std::string arg_tag)
		{
			//タグ配列を走査
			for (auto& tag : m_tags)
			{
				if (tag.compare(arg_tag) == 0)return true;
			}
			return false;
		}

		/// <summary>
		/// 引数に渡したコライダーも対象のコールバックを持っているか
		/// </summary>
		/// <param name="arg_other">タグの確認をとる対象となるコライダー</param>
		/// <returns>引数に渡したコライダーも対象のコールバックを持っているか</returns>
		bool HaveCallBack(std::weak_ptr<Collider>arg_other);

		//当たり判定結果の取得
		const bool& GetIsHit()const { return m_isHit; }
		//親トランスフォームより行列取得（アタッチされていなければ単位行列）
		Matrix GetTransformMat()
		{
			return m_parentTransform ? m_parentTransform->GetMatWorld() : XMMatrixIdentity();
		}
		//親トランスフォームより深度取得（アタッチされていなければ０）
		float GetDepth()
		{
			return m_parentTransform ? m_parentTransform->GetPos().z : 0.0f;
		}

		//コライダーがアタッチされてる親オブジェクト取得
		template<typename T>
		T* GetParentObject()
		{
			//アタッチされていない
			if (m_parentObj == nullptr)
			{
				printf("This collider's parent object hasn't be attatched. It's nullptr.\n");
				assert(0);
			}

			return dynamic_cast<T*>(m_parentObj);
		}

		bool operator== (const Collider& arg_other)const
		{
			return this->m_id == arg_other.m_id;
		}
	};
}