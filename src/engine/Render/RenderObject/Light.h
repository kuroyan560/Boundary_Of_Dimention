#pragma once
#include"KuroEngine.h"

namespace KuroEngine
{
	class LightManager;

	//ライトデータコンテナ
	namespace Light
	{
		class Direction
		{
			friend class LightManager;
			static const Vec3<float>DEFAULT_DIR;
			bool dirty = true;	//LightManagerからの監視用

			class ConstData
			{
			public:
				Color color;
				Vec3<float>dir = DEFAULT_DIR;
				unsigned int active = 1;

				static const ConstData& ZeroData()
				{
					static ConstData data;
					data.active = 0;
					data.color = Color(0, 0, 0, 0);
					data.dir = { 0,0,0 };
					return data;
				}
			}constData;

		public:
			//セッタ
			void SetColor(const Color& Color)
			{
				if (constData.color == Color)return;
				constData.color = Color;
				dirty = true;
			}
			void SetDir(const Vec3<float>& Direction)
			{
				if (constData.dir == Direction)return;
				constData.dir = Direction;
				dirty = true;
			}
			void SetActive(const bool& Active)
			{
				unsigned int val = Active ? 1 : 0;
				if (constData.active == val)return;
				constData.active = val;
				dirty = true;
			}
			void SetActive()
			{
				SetActive(1 - constData.active);
			}
			//ゲッタ
			const Color& GetColor()const { return constData.color; }
			const Vec3<float>& GetDir()const { return constData.dir; }
			const bool GetActive()const { return constData.active == 1; }
		};

		class Point
		{
			friend class LightManager;
			bool dirty = true;	//LightManagerからの監視用

			class ConstData
			{
			public:
				Color color;
				Vec3<float>pos = { 0.0f,1.0f,0.0f };
				float influenceRange = 20.0f;
				unsigned int active = 1;

				static const ConstData& ZeroData()
				{
					static ConstData data;
					data.active = 0;
					data.color = Color(0, 0, 0, 0);
					data.pos = { 0,0,0 };
					data.influenceRange = 0.0f;
					return data;
				}
			}constData;

		public:
			const Color& GetColor() { return constData.color; }
			const Vec3<float>& GetPos() { return constData.pos; }
			const float& GetInfluenceRange() { return constData.influenceRange; }
			void SetColor(const Color& Color)
			{
				if (constData.color == Color)return;
				constData.color = Color;
				dirty = true;
			}
			void SetPos(const Vec3<float>& Pos)
			{
				if (constData.pos == Pos)return;
				constData.pos = Pos;
				dirty = true;
			}
			void SetInfluenceRange(const float& Range)
			{
				if (constData.influenceRange == Range)return;
				constData.influenceRange = Range;
				dirty = true;
			}
			void SetActive(const bool& Active)
			{
				unsigned int val = Active ? 1 : 0;
				if (constData.active == val)return;
				constData.active = val;
				dirty = true;
			}
			void SetActive()
			{
				SetActive(1 - constData.active);
			}
			//ゲッタ
			const Color& GetColor()const { return constData.color; }
			const Vec3<float>& GetPos()const { return constData.pos; }
			const float& GetInfluenceRange()const { return constData.influenceRange; }
			const bool GetActive()const { return constData.active == 1; }
		};

		class Spot
		{
			friend class LightManager;
			bool dirty = true;	//LightManagerからの監視用

			class ConstData
			{
			public:
				Color color;
				Vec3<float>pos = { 0.0f,1.0f,0.0f };
				float influenceRange = 20.0f;
				Vec3<float>target = { 0.0f,0.0f,0.0f };
				Angle angle = 30.0f;
				unsigned int active = 1;

				static const ConstData& ZeroData()
				{
					static ConstData data;
					data.active = 0;
					data.color = Color(0, 0, 0, 0);
					data.pos = { 0,0,0 };
					data.influenceRange = 0.0f;
					data.target = { 0,0,0 };
					data.angle = 0.0f;
					return data;
				}
			}constData;

		public:
			//セッタ
			void SetColor(const Color& Color)
			{
				if (constData.color == Color)return;
				constData.color = Color;
				dirty = true;
			}
			void SetPos(const Vec3<float>& Pos)
			{
				if (constData.pos == Pos)return;
				constData.pos = Pos;
				dirty = true;
			}
			void SetInfluenceRange(const float& Range)
			{
				if (constData.influenceRange == Range)return;
				constData.influenceRange = Range;
				dirty = true;
			}
			void SetTarget(const Vec3<float>& Target)
			{
				if (constData.target == Target)return;
				constData.target = Target;
				dirty = true;
			}
			void SetAngle(const Angle& Angle)
			{
				if (constData.angle == Angle)return;
				constData.angle = Angle;
				dirty = true;
			}
			void SetActive(const bool& Active)
			{
				unsigned int val = Active ? 1 : 0;
				if (constData.active == val)return;
				constData.active = val;
				dirty = true;
			}
			void SetActive()
			{
				SetActive(1 - constData.active);
			}

			//ゲッタ
			const Color& GetColor()const { return constData.color; }
			const Vec3<float>& GetPos()const { return constData.pos; }
			const float& GetInfluenceRange()const { return constData.influenceRange; }
			const Vec3<float>& GetTarget()const { return constData.target; }
			const Angle& GetAngle()const { return constData.angle; }
			const bool GetActive()const { return constData.active == 1; }
		};

		class HemiSphere
		{
			friend class LightManager;
			bool dirty = true;	//LightManagerからの監視用

			class ConstData
			{
			public:
				Color skyColor = Color(0.4f, 0.6f, 0.8f, 1.0f);
				Color groundColor = Color(0.15f, 0.7f, 0.95f, 1.0f);
				Vec3<float>groundNormal = { 0.0f,1.0f,0.0f };
				unsigned int active = 1;

				static const ConstData& ZeroData()
				{
					static ConstData data;
					data.active = 0;
					data.skyColor = Color(0, 0, 0, 0);
					data.groundColor = Color(0, 0, 0, 0);
					data.groundNormal = { 0,0,0 };
					return data;
				}
			}constData;

		public:
			//セッタ
			void SetSkyColor(const Color& SkyColor)
			{
				if (constData.skyColor == SkyColor)return;
				constData.skyColor = SkyColor;
				dirty = true;
			}
			void SetGroundColor(const Color& GroundColor)
			{
				if (constData.groundColor == GroundColor)return;
				constData.groundColor = GroundColor;
				dirty = true;
			}
			void SetGroundNormal(const Vec3<float>& GroundNormal)
			{
				if (constData.groundNormal == GroundNormal)return;
				constData.groundNormal = GroundNormal;
				dirty = true;
			}
			void SetActive(const bool& Active)
			{
				unsigned int val = Active ? 1 : 0;
				if (constData.active == val)return;
				constData.active = val;
				dirty = true;
			}
			void SetActive()
			{
				SetActive(1 - constData.active);
			}

			//ゲッタ
			const Color& GetSkyColor()const { return constData.skyColor; }
			const Color& GetGroundColor()const { return constData.groundColor; }
			const Vec3<float>& GetGroundNormal()const { return constData.groundNormal; }
			const bool GetActive()const { return constData.active == 1; }
		};

		enum TYPE
		{
			DIRECTION, POINT, SPOT, HEMISPHERE, TYPE_NUM
		};
	}
}