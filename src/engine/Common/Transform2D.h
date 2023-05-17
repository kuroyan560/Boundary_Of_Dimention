#pragma once
#include"Common/Vec.h"
#include<cmath>
#include"KuroEngine.h"
#include"Common/Angle.h"
#include<list>

namespace KuroEngine
{
	class Transform2D
	{
		static std::list<Transform2D*> s_transform2DList;
	public:
		static void DirtyReset()
		{
			for (auto& trans : s_transform2DList)
			{
				trans->m_frameDirty = false;
			}
		}
	private:
		Transform2D* m_parent = nullptr;

		Matrix m_worldMat = XMMatrixIdentity();
		Matrix m_localMat = XMMatrixIdentity();
		Vec2<float>m_pos = { 0,0 };
		Vec2<float>m_scale = { 1,1 };
		Matrix m_rotate = XMMatrixIdentity();

		bool m_frameDirty = true;
		bool m_dirty = true;

		void MatReset()
		{
			m_frameDirty = true;
			m_dirty = true;
		}

		void CalculateMat();

	public:
		Transform2D(Transform2D* Parent = nullptr) {
			SetParent(Parent);
			s_transform2DList.emplace_back(this);
		}
		~Transform2D() {
			(void)s_transform2DList.remove_if([this](Transform2D* tmp) {
				return tmp == this;
				});
		}
		void SetParent(Transform2D* Parent) {
			m_parent = Parent;
			MatReset();
		}

		//ゲッタ
		const Vec2<float>& GetPos() { return m_pos; }
		const Vec2<float>& GetScale() { return m_scale; }
		const Vec3<Angle>& GetAngle() {
			auto sy = m_rotate.r[0].m128_f32[2];
			auto unlocked = std::abs(sy) < 0.99999f;
			return Vec3<Angle>(
				unlocked ? std::atan2(-m_rotate.r[1].m128_f32[2], m_rotate.r[2].m128_f32[2]) : std::atan2(m_rotate.r[2].m128_f32[1], m_rotate.r[1].m128_f32[1]),
				std::asin(sy),
				unlocked ? std::atan2(-m_rotate.r[0].m128_f32[1], m_rotate.r[0].m128_f32[0]) : 0);
		}
		const XMVECTOR& GetQuaternion() {
			return XMQuaternionRotationMatrix(m_rotate);
		}
		const Vec3<float>GetAxis() {
			XMVECTOR quat = GetQuaternion();
			return Vec3<float>(quat.m128_f32[0], quat.m128_f32[1], quat.m128_f32[2]);
		}

		//親のトランスフォームも考慮したゲッタ
		Vec2<float>GetPosWorld()
		{
			auto worldMat = GetMatWorld();
			return { worldMat.r[3].m128_f32[0],worldMat.r[3].m128_f32[1] };
		}
		Vec2<float>GetScaleWorld()
		{
			const auto& worldMat = GetMatWorld();
			return { worldMat.r[0].m128_f32[0],worldMat.r[1].m128_f32[1] };
		}

		//セッタ
		void SetPos(const Vec2<float> Pos) {
			if (m_pos == Pos)return;
			m_pos = Pos;
			MatReset();
		}
		void SetScale(const Vec2<float> Scale) {
			if (m_scale == Scale)return;
			m_scale = Scale;
			MatReset();
		}
		void SetScale(float Scale) {
			if (m_scale.x == Scale && m_scale.y == Scale)return;
			m_scale = Vec2<float>(Scale, Scale);
			MatReset();
		}
		void SetRotate(const Vec3<Angle>& Rotate) {
			m_rotate = XMMatrixRotationRollPitchYaw(Rotate.x, Rotate.y, Rotate.z);
			MatReset();
		}
		void SetRotate(const XMVECTOR& Quaternion) {
			m_rotate = XMMatrixRotationQuaternion(Quaternion);
			MatReset();
		}
		void SetRotate(const Vec3<float>& Axis, const Angle& Angle) {
			Vec3<float>axis = Axis;
			if (1.0f < axis.Length())axis.Normalize();
			XMVECTOR vec = XMVectorSet(axis.x, axis.y, axis.z, 1.0f);
			m_rotate = XMMatrixRotationQuaternion(XMQuaternionRotationAxis(vec, Angle));
			MatReset();
		}

		//ローカル行列ゲッタ
		const Matrix& GetMatLocal();
		//ワールド行列ゲッタ
		const Matrix& GetMatWorld();
		//Dirtyフラグゲッタ
		bool IsDirty()const
		{
			return m_dirty || (m_parent != nullptr && (m_parent->IsFrameDirty() || m_parent->IsDirty()));
		}
		//フレーム単位のDirtyフラグゲッタ
		bool IsFrameDirty()const
		{
			return m_frameDirty || (m_parent != nullptr && m_parent->IsFrameDirty());
		}
	};
}