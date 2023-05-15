#pragma once
#include"KuroEngine.h"
#include<memory>
#include<string>
#include"Common/Transform.h"

namespace KuroEngine
{
	class ConstantBuffer;
	class Camera
	{
	public:
		enum ProjMatMode
		{
			Perspective,		//透視射影行列
			Ortho,			//平行投影
		};
	private:
		class ConstData
		{
		public:
			Matrix matView = XMMatrixIdentity(); // ビュー行列
			Matrix matProjection = XMMatrixIdentity();	//プロジェクション行列
			Matrix billboardMat = XMMatrixIdentity();
			Matrix billboardMatY = XMMatrixIdentity();
			Vec3<float> eye = { 0,0,0 }; // カメラ座標（ワールド座標）
			float nearClip = 0.1f;
			float farClip = 6000.0f;
		}cameraInfo;
		std::shared_ptr<ConstantBuffer>buff;

		//プロジェクション行列が透視射影か平行投影かどうか
		ProjMatMode projMatMode = Perspective;

		Transform m_transform;
		Angle angleOfView = Angle(60);	//画角
		float nearZ = 0.1f;
		float farZ = 6000.0f;
		float aspect = 0.0f;

		//平行投影用
		float width = 1280.0f;
		float height = 720.0f;

		Matrix viewInvMat; //ビュー行列の逆行列

		//マッピングの判断用ダーティフラグ
		bool dirty = true;
		void CameraInfoUpdate();

	public:
		const std::string name;
		Camera(const std::string& Name);

		void operator=(const Camera& Other)
		{
			SetProjMatMode(Other.projMatMode);
			SetNearZ(Other.nearZ);
			SetFarZ(Other.farZ);
			SetViewAngle(Other.angleOfView);
			SetWidth(Other.width);
			SetHeight(Other.height);
			SetAspect(Other.aspect);
		}

		//セッタ
		void SetProjMatMode(const ProjMatMode& Mode)
		{
			projMatMode = Mode;
			dirty = true;
		}
		void SetNearZ(const float& NearZ)
		{
			nearZ = NearZ;
			dirty = true;
		}
		void SetFarZ(const float& FarZ)
		{
			farZ = FarZ;
			dirty = true;
		}
		void SetViewAngle(const Angle& Angle)
		{
			angleOfView = Angle;
			dirty = true;
		}
		void SetWidth(const float& Width)
		{
			width = Width;
			dirty = true;
		}
		void SetHeight(const float& Height)
		{
			height = Height;
			dirty = true;
		}
		void SetAspect(const float& Aspect)
		{
			aspect = Aspect;
			dirty = true;
		}

		//ゲッタ
		Transform& GetTransform() { return m_transform; }
		const Vec3<float>& GetEye() { return cameraInfo.eye; }
		const Angle& GetAngleOfView() { return angleOfView; }
		const Matrix& GetViewMat() { CameraInfoUpdate(); return cameraInfo.matView; }
		const Matrix& GetProjectionMat() { CameraInfoUpdate(); return cameraInfo.matProjection; }
		const Matrix& GetBillBoardMat() { CameraInfoUpdate(); return cameraInfo.billboardMat; }
		const float& GetNearZ() { return nearZ; }
		const float& GetFarZ() { return farZ; }

		const std::shared_ptr<ConstantBuffer>& GetBuff();
	};
}