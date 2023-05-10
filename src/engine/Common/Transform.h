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
		const XMVECTOR& GetRotate()const { return m_rotate; }

		//親のトランスフォームも考慮した座標
		Vec3<float>GetPosWorld()
		{
			return m_pos + (m_parent ? m_parent->GetPosWorld() : Vec3<float>(0, 0, 0));
		}
		//親のトランスフォームも考慮した座標（ワールド行列より）
		Vec3<float>GetPosWorldByMatrix()
		{
			const auto& worldMat = GetMatWorld();
			return { worldMat.r[3].m128_f32[0],worldMat.r[3].m128_f32[1],worldMat.r[3].m128_f32[2] };
		}
		//親のトランスフォームも考慮したスケーリング
		Vec3<float>GetScaleWorld()
		{
			return m_scale * (m_parent ? m_parent->GetScaleWorld() : Vec3<float>(1, 1, 1));
		}
		//親のトランスフォームも考慮したスケーリング(ワールド行列より)
		Vec3<float>GetScaleWorldByMatrix()
		{
			const auto& worldMat = GetMatWorld();
			return { worldMat.r[0].m128_f32[0],worldMat.r[1].m128_f32[1],worldMat.r[2].m128_f32[2] };
		}
		//親の回転も考慮したクォータニオン
		XMVECTOR GetRotateWorld()const
		{
			XMVECTOR parent = m_parent ? m_parent->GetRotateWorld() : XMQuaternionIdentity();
			return XMQuaternionMultiply(parent, m_rotate);
		}

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
		//前・右・上ベクトルゲッタ
		Vec3<float> GetFront()const{
			return KuroEngine::Math::TransformVec3(Vec3<float>::GetZAxis(), m_rotate);
		}
		Vec3<float> GetRight()const {
			return KuroEngine::Math::TransformVec3(Vec3<float>::GetXAxis(), m_rotate);
		}
		Vec3<float> GetUp()const {
			return KuroEngine::Math::TransformVec3(Vec3<float>::GetYAxis(), m_rotate);
		}
		//前・右・上ベクトルゲッタ（親の回転考慮）
		Vec3<float> GetFrontWorld()const {
			return KuroEngine::Math::TransformVec3(Vec3<float>::GetZAxis(), GetRotateWorld());
		}
		Vec3<float> GetRightWorld()const {
			return KuroEngine::Math::TransformVec3(Vec3<float>::GetXAxis(), GetRotateWorld());
		}
		Vec3<float> GetUpWorld()const {
			return KuroEngine::Math::TransformVec3(Vec3<float>::GetYAxis(), GetRotateWorld());
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

		//回転の代入
		void SetRotate(const DirectX::XMVECTOR& Quaternion) {
			m_rotate = Quaternion;
			m_rotate = XMQuaternionNormalize(m_rotate);
			MatReset();
		}
		void SetRotate(const Angle& X, const Angle& Y, const Angle& Z) {
			SetRotate(XMQuaternionRotationRollPitchYaw(X, Y, -Z));
		}
		void SetRotate(const Matrix& RotateMat) {
			SetRotate(XMQuaternionRotationMatrix(RotateMat));
		}
		void SetRotate(const Vec3<float>& Axis, const Angle& Angle) {
			SetRotate(XMQuaternionRotationAxis(XMVectorSet(Axis.x, Axis.y, Axis.z, 1.0f), Angle));
		}

		void SetFront(Vec3<float>Front) {
			SetRotate(Math::GetLookAtQuaternion(Vec3<float>::GetZAxis(), Front));
		}
		void SetRight(Vec3<float>Right) {
			SetRotate(Math::GetLookAtQuaternion(Vec3<float>::GetXAxis(), Right));
		}
		void SetUp(Vec3<float>Up) {
			SetRotate(Math::GetLookAtQuaternion(Vec3<float>::GetYAxis(), Up));
		}
		void SetLookAtRotate(Vec3<float>Target) {
			SetFront((Target - m_pos).GetNormal());
		}

		//現在の回転をさらに回転させて合わせる
		void SetFrontBySpin(Vec3<float>Front)	{
			SetRotate(XMQuaternionMultiply(Math::GetLookAtQuaternion(GetFront(), Front), m_rotate));
		}
		void SetRightBySpin(Vec3<float>Right)	{
			SetRotate(XMQuaternionMultiply(Math::GetLookAtQuaternion(GetRight(), Right), m_rotate));
		}
		void SetUpBySpin(Vec3<float>Up){
			SetRotate(XMQuaternionMultiply(Math::GetLookAtQuaternion(GetUp(), Up), m_rotate));
		}


		//大石が追加した関数----------------------------------------
		void SetWorldMat(const Matrix &mat)
		{
			m_worldMat = mat;
			MatReset();
		};
		//Vec3で設定した情報からMatrixを計算し直さない行列を返す
		const Matrix &GetMatWorldWithOutDirty()
		{
			return m_worldMat;
		}
		//回転行列のみコピー
		void SetRotaMatrix(const Matrix src_matrix)
		{
			KuroEngine::Vec3<float>pos = GetPosWorld();
			m_worldMat = src_matrix;
			m_worldMat.r[3].m128_f32[0] = pos.x;
			m_worldMat.r[3].m128_f32[1] = pos.y;
			m_worldMat.r[3].m128_f32[2] = pos.z;
		}
		//行列から座標と角度を設定する
		void CalucuratePosRotaBasedOnWorldMatrix()
		{
			m_pos = { m_worldMat.r[3].m128_f32[0], m_worldMat.r[3].m128_f32[1], m_worldMat.r[3].m128_f32[2] };
			m_rotate = XMQuaternionRotationMatrix(m_worldMat);
		}
		//大石が追加した関数----------------------------------------



		//ローカル行列ゲッタ
		const Matrix& GetMatLocal();
		//ワールド行列ゲッタ
		const Matrix& GetMatWorld();
		//ワールド行列（ビルボード適用）
		Matrix GetMatWorld(const Matrix& arg_billBoardMat);
		//Dirtyフラグゲッタ
		bool IsDirty()const{
			return m_dirty || (m_parent != nullptr && (m_parent->IsFrameDirty() || m_parent->IsDirty()));
		}
		//フレーム単位のDirtyフラグゲッタ
		bool IsFrameDirty()const{
			return m_frameDirty || (m_parent != nullptr && m_parent->IsFrameDirty());
		}
	};
}