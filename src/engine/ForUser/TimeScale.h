#pragma once
namespace KuroEngine
{
	class TimeScale
	{
		float m_timeScale = 1.0f;

	public:
		void Set(float arg_timeScale)
		{
			m_timeScale = arg_timeScale;
		}
		const float& GetTimeScale()const { return m_timeScale; }
	};
}