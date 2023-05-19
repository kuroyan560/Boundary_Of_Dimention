#pragma once
#include<algorithm>

namespace KuroEngine
{
	class Timer
	{
		//設定時間
		float m_intervalTime = 0.0f;
		//経過時間
		float m_elaspedTime = 0.0f;
		//タイムアップトリガー記録
		bool m_onTimeUpTrigger = false;

	public:
		Timer(float arg_intervalTime = 1.0f) :m_intervalTime(arg_intervalTime) {}

		//設定時間が経過したか
		bool IsTimeUp()const { return m_intervalTime <= m_elaspedTime; }

		//タイムアップした瞬間
		const bool& IsTimeUpOnTrigger()const { return m_onTimeUpTrigger; }

		//経過時間 / 設定時間
		float GetTimeRate(float arg_rate = 1.0f)const
		{
			return std::clamp(m_elaspedTime / (m_intervalTime * arg_rate), 0.0f, 1.0f);
		}

		//
		float GetTimeRate(float arg_startTimeRate, float arg_endTimeRate)const
		{
			auto timeRate = GetTimeRate();
			return std::clamp((timeRate - arg_startTimeRate) / (arg_endTimeRate - arg_startTimeRate), 0.0f, 1.0f);
		}

		//タイマースタートした瞬間
		bool IsTimeStartOnTrigger()const { return GetTimeRate() == 0.0f; }

		//(1.0f - 経過時間 / 設定時間)
		float GetInverseTimeRate()const { return 1.0f - GetTimeRate(); }

		//残り時間
		float GetLeftTime()const
		{
			if (IsTimeUp())return 0.0f;
			return m_intervalTime - m_elaspedTime;
		}

		//経過時間
		const float& GetElaspedTime()const { return m_elaspedTime; }

		//設定時間
		const float& GetIntervalTime()const { return m_intervalTime; }

		/// <summary>
		/// 時間経過
		/// </summary>
		/// <param name="arg_timeScale">タイムスケール</param>
		/// <returns>設定時間が経過したか</returns>
		bool UpdateTimer(float arg_timeScale = 1.0f)
		{
			//前フレームでタイムアップしたか
			bool oldTimeUp = IsTimeUp();
			m_elaspedTime += arg_timeScale;

			//タイムアップのトリガー記録
			m_onTimeUpTrigger = !oldTimeUp && IsTimeUp();

			return IsTimeUp();
		}

		//強制的に時間経過完了
		void ForciblyTimeUp()
		{
			m_elaspedTime = m_intervalTime;
		}

		//リセット
		void Reset() { m_elaspedTime = 0.0f; }
		void Reset(float arg_interval)
		{
			m_intervalTime = arg_interval;
			Reset();
		}
		void Reset(int arg_interval)
		{
			Reset(static_cast<float>(arg_interval));
		}

		void operator=(float arg) = delete;
	};
}