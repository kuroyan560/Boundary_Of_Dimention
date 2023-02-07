#pragma once
#include<memory>
#include"KuroEngine.h"
#include<cmath>
#include<list>

namespace KuroEngine
{
	class Transform
	{
		static std::list<Transform*> s_transformList;
	public:
		static void DirtyReset()
		{
			for (auto& trans : s_transformList)
			{
				trans->m_frameDirty = false;
			}
		}
		static Transform& GetIdentity()
		{
			static Transform s_identity;
			return s_identity;
		}

	private:
		Transform* m_parent = nullptr;

		Matrix m_worldMat = XMMatrixIdentity();
		Matrix m_localMat = XMMatrixIdentity();
		Vec3<float>m_pos = { 0,0,0 };
		Vec3<float>m_scale = { 1,1,1 };
		Quaternion m_rotate = XMQuaternionIdentity();

		bool m_frameDirty = true;
		bool m_dirty = true;

		void MatReset()
		{
			m_frameDirty = true;
			m_dirty = true;
		}

		void CalculateMat();

	public:
		Transform(Transform* Parent = nullptr) {
			SetParent(Parent);
			s_transformList.emplace_back(this);

			auto front = Vec3<float>::GetZAxis();
			auto up = Vec3<float>::GetYAxis();
			m_rotate = XMQuaternionRotationMatrix(XMMatrixLookToLH(
				XMVectorSet(m_pos.x, m_pos.y, m_pos.z, 1.0f),
				XMVectorSet(front.x, front.y, front.z, 1.0f),
				XMVectorSet(up.x, up.y, up.z, 1.0f)));
		}
		~Transform() {
			(void)s_transformList.remove_if([this](Transform* tmp) {
				return tmp == this;
				});
		}
		void SetParent(Transform* Parent) {

			m_parent = Parent;
			MatReset();
		}

		//ゲッタ
		const Vec3<float>& GetPos()const { return m_pos; }
		const Vec3<float>& GetScale()const { return m_scale; }
		
		//回転クォータニオンゲッタ
		const XMVECTOR& GetRotate()const { return m_rotate; }
		//オイラー角で回転量取得
		Vec3<Angle> GetRotateAsEuler()const {
			auto q0 = m_rotate.m128_f32[0];
			auto q1 = m_rotate.m128_f32[1];
			auto q2 = m_rotate.m128_f32[2];
			auto q3 = m_rotate.m128_f32[3];

			auto roll = atan2(2.0f * (q2 * q3 + q0 * q1), q0 * q0 - q1 * q1 - q2 * q2 + q3 * q3);
			auto pitch = asin(2.0f * (q0 * q2 - q1 * q3));
			auto yaw = atan2(2.0f * (q1 * q2 + q0 * q3), q0 * q0 + q1 * q1 - q2 * q2 - q3 * q3);

			return Vec3<Angle>(-roll, pitch, yaw);
		}
		//前ベクトルゲッタ
		Vec3<float> GetFront()const{
			auto front = XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f);
			front = XMQuaternionMultiply(XMQuaternionMultiply(m_rotate, front), XMQuaternionConjugate(m_rotate));
			return Vec3<float>(front.m128_f32[0], front.m128_f32[1], front.m128_f32[2]);
		}
		//右ベクトルゲッタ
		Vec3<float> GetRight()const {
			auto right = XMVectorSet(1.0f, 0.0f, 1.0f, 1.0f);
			right = XMQuaternionMultiply(XMQuaternionMultiply(m_rotate, right), XMQuaternionConjugate(m_rotate));
			return Vec3<float>(right.m128_f32[0], right.m128_f32[1], right.m128_f32[2]);
		}
		//上ベクトルゲッタ
		Vec3<float> GetUp()const {
			auto up = XMVectorSet(0, 1, 0, 1);
			up = XMQuaternionMultiply(XMQuaternionMultiply(m_rotate, up), XMQuaternionConjugate(m_rotate));
			return Vec3<float>(up.m128_f32[0], up.m128_f32[1], up.m128_f32[2]);
		}

		//セッタ
		void SetPos(const Vec3<float> Pos) {
			if (m_pos == Pos)return;
			m_pos = Pos;
			MatReset();
		}
		void SetScale(const Vec3<float> Scale) {
			if (m_scale == Scale)return;
			m_scale = Scale;
			MatReset();
		}
		void SetScale(const float Scale) {
			auto s = Vec3<float>(Scale, Scale, Scale);
			if (m_scale == s)return;
			m_scale = s;
			MatReset();
		}
		void SetRotate(const Angle& X, const Angle& Y, const Angle& Z) {
			m_rotate = XMQuaternionRotationRollPitchYaw(Y, Z, -X);
			MatReset();
		}
		void SetRotate(const DirectX::XMVECTOR& Quaternion) {
			m_rotate = Quaternion;
			MatReset();
		}
		void SetRotate(const Vec3<float>& Axis, const Angle& Angle) {
			m_rotate = XMQuaternionRotationAxis(XMVectorSet(Axis.x, Axis.y, Axis.z, 1.0f), Angle);
			MatReset();
		}
		void SetRotate(const Matrix& RotateMat) {
			m_rotate = XMQuaternionRotationMatrix(RotateMat);
			MatReset();
		}
		void SetLookAtRotate(const Vec3<float>& Target, const Vec3<float>& UpAxis = Vec3<float>(0, 1, 0)) {
			Vec3<float>z = Vec3<float>(Target - m_pos).GetNormal();
			Vec3<float>x = UpAxis.Cross(z).GetNormal();
			Vec3<float>y = z.Cross(x).GetNormal();

			SetRotate(XMMatrixLookAtLH(
				XMVectorSet(m_pos.x, m_pos.y, m_pos.z, 1.0f),
				XMVectorSet(Target.x, Target.y, Target.z, 1.0f),
				XMVectorSet(UpAxis.x, UpAxis.y, UpAxis.z, 1.0f)));
		}
		void SetUp(const Vec3<float>& Up)
		{
			SetRotate(KuroEngine::Math::RotateMat(Vec3<float>::GetYAxis(), Up));
		}
		void SetFront(const Vec3<float>& Front)
		{
			SetRotate(KuroEngine::Math::RotateMat(Vec3<float>::GetZAxis(), Front));
		}

		//ローカル行列ゲッタ
		const Matrix& GetLocalMat();
		//ワールド行列ゲッタ
		const Matrix& GetWorldMat();
		//ワールド行列（ビルボード適用）
		Matrix GetWorldMat(const Matrix& arg_billBoardMat);
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