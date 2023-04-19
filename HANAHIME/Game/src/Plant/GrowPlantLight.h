#pragma once
#include"Common/Vec.h"
#include"Common/Transform.h"
#include"Common/Angle.h"
#include<vector>
//植物を繁殖させる光
class GrowPlantLight
{
private:
	//シーンに存在するライトの配列
	static std::vector<GrowPlantLight*>s_lightArray;

public:
	enum TYPE { POINT, SPOT };

	//シーンに存在するライトの配列初期化
	static void ResetRegisteredLight() { s_lightArray.clear(); }
	static const std::vector<GrowPlantLight*>& GrowPlantLightArray() { return s_lightArray; }

private:
	TYPE m_type;

protected:
	GrowPlantLight(TYPE arg_type) :m_type(arg_type) {}
	//トランスフォーム
	KuroEngine::Transform m_transform;
	//アクティブ状態
	bool m_active = true;

public:
	//登録
	void Register()
	{
		s_lightArray.emplace_back(this);
	}

	//親のトランスフォーム設定
	void SetParent(KuroEngine::Transform* arg_parent)
	{
		m_transform.SetParent(arg_parent);
	}

	//四角との当たり判定
	virtual bool HitCheckWithBox(KuroEngine::Vec3<float>arg_center, KuroEngine::Vec3<float>arg_size) = 0;

	//種別のゲッタ
	const TYPE& GetType()const { return m_type; }
};

//点光源
class GrowPlantLight_Point : public GrowPlantLight
{
public:
	//影響範囲（半径）
	float m_influenceRange = 8.0f;

	GrowPlantLight_Point(float arg_influenceRange, KuroEngine::Transform* arg_parent = nullptr)
		:m_influenceRange(arg_influenceRange), GrowPlantLight(POINT)
	{
		SetParent(arg_parent);
	}

	//GPUに送信する際のフォーマット
	class ConstData
	{
	public:
		KuroEngine::Vec3<float>m_pos = { 0,0,0 };
		float m_influenceRange = 8.0f;
		unsigned int m_active = 1;
		int pad[3];
	};

	//GPUに送信するデータ形式になおして出力
	ConstData GetSendData()
	{
		ConstData data;
		data.m_pos = m_transform.GetPosWorld();
		data.m_influenceRange = m_influenceRange;
		data.m_active = m_active ? 1 : 0;
		return data;
	}

	//四角との当たり判定
	bool HitCheckWithBox(KuroEngine::Vec3<float>arg_center, KuroEngine::Vec3<float>arg_size)override;
};

//スポットライト
class GrowPlantLight_Spot : public GrowPlantLight
{
public:
	GrowPlantLight_Spot(float arg_influenceRange, KuroEngine::Angle arg_apertureAngle, KuroEngine::Transform* arg_parent = nullptr) :GrowPlantLight(SPOT)
	{
		m_apertureAngle = arg_apertureAngle;
		m_influenceRange = arg_influenceRange;
		SetParent(arg_parent);
	}

	//デフォルト状態（回転がない）での向き
	KuroEngine::Vec3<float>m_defaultVec = { 0,1,0 };
	//影響範囲（距離）
	float m_influenceRange = 20.0f;
	//光の絞りの角度
	KuroEngine::Angle m_apertureAngle;

	//GPUに送信する際のフォーマット
	class ConstData
	{
	public:
		KuroEngine::Vec3<float>m_pos = { 0,0,0 };
		float m_influenceRange = 10.0f;
		KuroEngine::Vec3<float>m_vec = { 0,1,0 };
		float m_angle = 0.0f;
		unsigned int m_active = 1;
		int pad[3];
	};

	//GPUに送信するデータ形式になおして出力
	ConstData GetSendData()
	{
		ConstData data;
		data.m_pos = m_transform.GetPosWorld();
		data.m_influenceRange = m_influenceRange;
		data.m_vec = KuroEngine::Math::TransformVec3(m_defaultVec, m_transform.GetRotateWorld());
		data.m_angle = m_apertureAngle;
		data.m_active = m_active ? 1 : 0;
		return data;
	}

	//四角との当たり判定
	bool HitCheckWithBox(KuroEngine::Vec3<float>arg_center, KuroEngine::Vec3<float>arg_size)override;
};