#pragma once
#include"Common/Vec.h"
#include"Common/ValueMinMax.h"
#include<vector>
#include"Common/Transform.h"
#include<memory>
#include"DirectX12/D3D12Data.h"
#include"CollisionResultInfo.h"

namespace KuroEngine
{
	class Camera;

	class CollisionPrimitive
	{
	private:
		friend class Collider;
		friend class CollisionPoint;
		friend class CollisionLine;
		friend class CollisionSphere;
		friend class CollisionPlane;
		friend class CollisionCapsule;
		friend class CollisionAABB;
		friend class CollisionMesh;
		friend class CollisionFloorMesh;

	protected:
		//基本的なプリミティブ当たり判定のデバッグ描画
		static std::shared_ptr<GraphicsPipeline>GetPrimitivePipeline();
		//定数バッファ用データ
		struct ConstData
		{
			Matrix m_parent = XMMatrixIdentity();
			unsigned int m_hit = 0;
		};

		CollisionPrimitive() {}
		CollisionPrimitive(CollisionPrimitive&& arg) = delete;
		CollisionPrimitive(const CollisionPrimitive& arg) = delete;

		//当たり判定の可視化
		virtual void DebugDraw(const bool& arg_hit, Camera& arg_cam, const Matrix& arg_parentMat) {};

		/*
		ダブルディスパッチでプリミティブ形状判定して関数呼び出し
		※当たり判定が用意されていなかった場合エラー
		*/
		virtual bool HitCheck(const Matrix& arg_myMat, const Matrix& arg_otherMat, CollisionPoint* arg_other, CollisionResultInfo* arg_info) { assert(0); return false; }
		virtual bool HitCheck(const Matrix& arg_myMat, const Matrix& arg_otherMat, CollisionLine* arg_other, CollisionResultInfo* arg_info) { assert(0); return false; }
		virtual bool HitCheck(const Matrix& arg_myMat, const Matrix& arg_otherMat, CollisionSphere* arg_other, CollisionResultInfo* arg_info) { assert(0); return false; }
		virtual bool HitCheck(const Matrix& arg_myMat, const Matrix& arg_otherMat, CollisionPlane* arg_other, CollisionResultInfo* arg_info) { assert(0); return false; }
		virtual bool HitCheck(const Matrix& arg_myMat, const Matrix& arg_otherMat, CollisionCapsule* arg_other, CollisionResultInfo* arg_info) { assert(0); return false; }
		virtual bool HitCheck(const Matrix& arg_myMat, const Matrix& arg_otherMat, CollisionAABB* arg_other, CollisionResultInfo* arg_info) { assert(0); return false; }
		virtual bool HitCheck(const Matrix& arg_myMat, const Matrix& arg_otherMat, CollisionMesh* arg_other, CollisionResultInfo* arg_info) { assert(0); return false; }
		virtual bool HitCheck(const Matrix& arg_myMat, const Matrix& arg_otherMat, CollisionFloorMesh* arg_other, CollisionResultInfo* arg_info) { assert(0); return false; }

	public:
		virtual ~CollisionPrimitive() {}

		//クローンの生成
		virtual CollisionPrimitive* Clone() = 0;

		virtual bool HitCheckDispatch(const Matrix& arg_myMat, const Matrix& arg_otherMat, CollisionPrimitive* arg_other, CollisionResultInfo* arg_info) = 0;
	};

	//ポイント
	class CollisionPoint : public CollisionPrimitive
	{
	private:
		std::shared_ptr<ConstantBuffer>m_constBuff;
		void DebugDraw(const bool& arg_hit, Camera& arg_cam, const Matrix& arg_parentMat)override;
		bool HitCheck(const Matrix& arg_myMat, const Matrix& arg_otherMat, CollisionFloorMesh* arg_other, CollisionResultInfo* arg_info)override;

	public:
		Vec3<float>m_offset;
		CollisionPoint(const Vec3<float>& Pos) { m_offset = Pos; }
		Vec3<float>GetWorldPos(const Matrix& arg_parentMat)
		{
			return KuroEngine::Math::TransformVec3(m_offset, arg_parentMat);
		}

		//クローンの生成
		CollisionPrimitive* Clone()override
		{
			return new CollisionPoint(m_offset);
		}

		bool HitCheckDispatch(const Matrix& arg_myMat, const Matrix& arg_otherMat, CollisionPrimitive* arg_other, CollisionResultInfo* arg_info)override
		{
			return arg_other->HitCheck(arg_myMat, arg_otherMat, this, arg_info);
		}
	};

	//線分
	class CollisionLine : public CollisionPrimitive
	{
	private:
		void DebugDraw(const bool& arg_hit, Camera& arg_cam, const Matrix& arg_parentMat)override;

	public:
		//始点座標
		Vec3<float>m_start;
		//方向
		Vec3<float>m_dir;
		//最大距離
		float m_len = FLT_MAX;

		CollisionLine(const Vec3<float>& Start, const Vec3<float>& Dir, const float& MaxDistance)
			:m_start(Start), m_dir(Dir), m_len(MaxDistance) {}

		Vec3<float>GetStartWorldPos(const Matrix& arg_parentMat)
		{
			return KuroEngine::Math::TransformVec3(m_start, arg_parentMat);
		}
		Vec3<float>GetEndWorldPos(const Matrix& arg_parentMat)
		{
			return GetStartWorldPos(arg_parentMat) + m_dir * m_len;
		}

		//クローンの生成
		CollisionPrimitive* Clone()override
		{
			return new CollisionLine(m_start, m_dir, m_len);
		}

		bool HitCheckDispatch(const Matrix& arg_myMat, const Matrix& arg_otherMat, CollisionPrimitive* arg_other, CollisionResultInfo* arg_info)override
		{
			return arg_other->HitCheck(arg_otherMat, arg_myMat, this, arg_info);
		}
	};

	//球
	class CollisionTriangle;
	class CollisionSphere : public CollisionPrimitive
	{
	private:
		friend class Collision;

	private:
		std::shared_ptr<ConstantBuffer>m_constBuff;
		void DebugDraw(const bool& arg_hit, Camera& arg_cam, const Matrix& arg_parentMat)override;

		bool HitCheck(const Matrix& arg_myMat, const Matrix& arg_otherMat, CollisionSphere* arg_other, CollisionResultInfo* arg_info)override;
		bool HitCheck(const Matrix& arg_myMat, const Matrix& arg_otherMat, CollisionPlane* arg_other, CollisionResultInfo* arg_info)override;
		bool HitCheck(const Matrix& arg_myMat, const Matrix& arg_otherMat, CollisionLine* arg_other, CollisionResultInfo* arg_info)override;
		bool HitCheck(const Matrix& arg_myMat, const Matrix& arg_otherMat, CollisionAABB* arg_other, CollisionResultInfo* arg_info)override;
		Vec3<float> ClosestPtPoint2Triangle(const Vec3<float>& Pt, const CollisionTriangle& Tri, const Matrix& MeshWorld);
		bool HitCheck(const Matrix& arg_myMat, const Matrix& arg_otherMat, CollisionMesh* arg_other, CollisionResultInfo* arg_info)override;

	public:
		Vec3<float>m_offset = { 0,0,0 };
		float m_hitBoxRadius;					//半径
		CollisionSphere(const float& Radius, const Vec3<float>& Offset)
			:m_offset(Offset), m_hitBoxRadius(Radius) {}
		Vec3<float>GetCenter(const Matrix& arg_parentMat)
		{
			return KuroEngine::Math::TransformVec3(m_offset, arg_parentMat);
		}

		//クローンの生成
		CollisionPrimitive* Clone()override
		{
			return new CollisionSphere(m_hitBoxRadius, m_offset);
		}

		bool HitCheckDispatch(const Matrix& arg_myMat, const Matrix& arg_otherMat, CollisionPrimitive* arg_other, CollisionResultInfo* arg_info)override
		{
			return arg_other->HitCheck(arg_otherMat, arg_myMat, this, arg_info);
		}
	};

	//平面
	class CollisionPlane : public CollisionPrimitive
	{
	private:
		friend class Collision;

	private:
		void DebugDraw(const bool& arg_hit, Camera& arg_cam, const Matrix& arg_parentMat)override;

	public:
		Vec3<float>m_normal = { 0,1,0 };	//板ポリの法線
		float m_distance = 0.0f;				//原点からの距離
		CollisionPlane(const Vec3<float>& Normal = { 0,1,0 }, const float& Distance = 0.0f)
			:m_normal(Normal), m_distance(Distance) {}


		//クローンの生成
		CollisionPrimitive* Clone()override
		{
			return new CollisionPlane(m_normal, m_distance);
		}

		bool HitCheckDispatch(const Matrix& arg_myMat, const Matrix& arg_otherMat, CollisionPrimitive* arg_other, CollisionResultInfo* arg_info)override
		{
			return arg_other->HitCheck(arg_myMat, arg_otherMat, this, arg_info);
		}
	};

	//カプセル
	class CollisionCapsule : public CollisionPrimitive
	{
	private:
		friend class Collision;

	private:
		std::shared_ptr<ConstantBuffer>m_constBuff;
		void DebugDraw(const bool& arg_hit, Camera& arg_cam, const Matrix& arg_parentMat)override;

	public:
		Vec3<float>m_offset;
		Vec3<float>m_sPoint;	//始点
		Vec3<float>m_ePoint;	//終点
		float m_hitBoxRadius;
		CollisionCapsule(const Vec3<float>& StartPt, const Vec3<float>& EndPt, const float& Radius, const Vec3<float>& Offset = Vec3<float>(0, 0, 0))
			:m_offset(Offset), m_sPoint(StartPt), m_ePoint(EndPt), m_hitBoxRadius(Radius) {}

		//クローンの生成
		CollisionPrimitive* Clone()override
		{
			return new CollisionCapsule(m_sPoint, m_ePoint, m_hitBoxRadius, m_offset);
		}

		bool HitCheckDispatch(const Matrix& arg_myMat, const Matrix& arg_otherMat, CollisionPrimitive* arg_other, CollisionResultInfo* arg_info)override
		{
			return arg_other->HitCheck(arg_myMat, arg_otherMat, this, arg_info);
		}
	};

	//AABB(軸並行境界ボックス）、色んな軸で回転すると判定がだめになる
	class CollisionAABB : public CollisionPrimitive
	{
	private:
		friend class Collision;
		enum VERT_IDX {
			LU_NZ, RU_NZ, RB_NZ, LB_NZ,	//手前
			LU_FZ, RU_FZ, RB_FZ, LB_FZ,	//奥
			VERT_NUM
		};

	private:
		std::shared_ptr<ConstantBuffer>m_constBuff;
		void DebugDraw(const bool& arg_hit, Camera& arg_cam, const Matrix& arg_parentMat)override;

		bool HitCheck(const Matrix& arg_myMat, const Matrix& arg_otherMat, CollisionSphere* arg_other, CollisionResultInfo* arg_info)override
		{
			return this->HitCheckDispatch(arg_myMat, arg_otherMat, arg_other, arg_info);
		}
		bool HitCheck(const Matrix& arg_myMat, const Matrix& arg_otherMat, CollisionLine* arg_other, CollisionResultInfo* arg_info)override;
		bool HitCheck(const Matrix& arg_myMat, const Matrix& arg_otherMat, CollisionAABB* arg_other, CollisionResultInfo* arg_info)override;


		//頂点バッファ
		std::shared_ptr<VertexBuffer>m_vertBuff;
		//各軸の最小値と最大値
		Vec3<ValueMinMax>m_pValues;

	public:
		CollisionAABB(const Vec3<ValueMinMax>& PValues) { StructBox(PValues); }

		//ゲッタ
		const Vec3<ValueMinMax>& GetPtValue() { return m_pValues; }
		Vec3<float>GetTransformedSize(const Matrix& arg_mat)const
		{
			auto max = GetTransformedMax(arg_mat);
			auto min = GetTransformedMin(arg_mat);
			return max - min;
		}
		Vec3<float>GetTransformedMin(const Matrix& arg_mat)const
		{
			return KuroEngine::Math::TransformVec3({ m_pValues.x.m_min,m_pValues.y.m_min,m_pValues.z.m_min }, arg_mat);
		}
		Vec3<float>GetTransformedMax(const Matrix& arg_mat)const
		{
			return KuroEngine::Math::TransformVec3({ m_pValues.x.m_max,m_pValues.y.m_max,m_pValues.z.m_max }, arg_mat);
		}

		//セッタ
		void StructBox(const Vec3<ValueMinMax>& PValues);

		//クローンの生成
		CollisionPrimitive* Clone()override
		{
			return new CollisionAABB(m_pValues);
		}

		bool HitCheckDispatch(const Matrix& arg_myMat, const Matrix& arg_otherMat, CollisionPrimitive* arg_other, CollisionResultInfo* arg_info)override
		{
			return arg_other->HitCheck(arg_otherMat, arg_myMat, this, arg_info);
		}
	};

	//法線つき三角形（ローカル座標）
	class CollisionTriangle
	{
	private:
		bool m_invalidNormal = true;
		Vec3<float>m_normal;

	public:
		Vec3<float>m_p0;
		Vec3<float>m_p1;
		Vec3<float>m_p2;

		//法線セット
		void SetNormal(const Vec3<float>& Normal)
		{
			m_normal = Normal;
			m_invalidNormal = false;
		}
		//法線の計算
		void CalculateNormal()
		{
			Vec3<float>p0_p1 = m_p1 - m_p0;
			Vec3<float>p0_p2 = m_p2 - m_p0;

			//外積により、２辺に垂直なベクトルを算出
			SetNormal(p0_p1.Cross(p0_p2).GetNormal());
		}

		//法線ゲッタ
		const Vec3<float>& GetNormal()
		{
			if (m_invalidNormal)CalculateNormal();
			assert(!m_normal.IsZero());
			return m_normal;
		}
	};

	class CollisionMesh : public CollisionPrimitive
	{
	private:
		friend class Collision;

	private:
		std::shared_ptr<VertexBuffer>m_vertBuff;
		std::shared_ptr<ConstantBuffer>m_constBuff;
		std::vector<CollisionTriangle>m_triangles;

		bool HitCheck(const Matrix& arg_myMat, const Matrix& arg_otherMat, CollisionSphere* arg_other, CollisionResultInfo* arg_info)override
		{
			return this->HitCheckDispatch(arg_myMat, arg_otherMat, arg_other, arg_info);
		}
		void DebugDraw(const bool& arg_hit, Camera& arg_cam, const Matrix& arg_parentMat)override;

	public:
		CollisionMesh(const std::vector<CollisionTriangle>& Triangles)
		{
			SetTriangles(Triangles);
		}

		//ゲッタ
		std::vector<CollisionTriangle>& GetTriangles() { return m_triangles; }

		//セッタ
		void SetTriangles(const std::vector<CollisionTriangle>& Triangles);

		//クローンの生成
		CollisionPrimitive* Clone()override
		{
			return new CollisionMesh(m_triangles);
		}
		virtual bool HitCheckDispatch(const Matrix& arg_myMat, const Matrix& arg_otherMat, CollisionPrimitive* arg_other, CollisionResultInfo* arg_info)override
		{
			return arg_other->HitCheck(arg_myMat, arg_otherMat, this, arg_info);
		}

	};

	class CollisionFloorMesh : public CollisionMesh
	{
	public:
		bool HitCheckDispatch(const Matrix& arg_myMat, const Matrix& arg_otherMat, CollisionPrimitive* arg_other, CollisionResultInfo* arg_info)override
		{
			return arg_other->HitCheck(arg_myMat, arg_otherMat, this, arg_info);
		}
	};
}