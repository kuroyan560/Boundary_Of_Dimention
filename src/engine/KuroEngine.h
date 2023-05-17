#pragma once
#include<string>
#include"Common/Vec.h"
#include"Common/Angle.h"
#include"Common/Color.h"
#include<memory>
#include<DirectXMath.h>
#include<functional>

namespace KuroEngine
{
    using Matrix = DirectX::XMMATRIX;
    using Quaternion = DirectX::XMVECTOR;
    using namespace DirectX;

    //通常の文字列からワイド文字列取得
	std::wstring GetWideStrFromStr(const std::string& Str);
	std::wstring GetWideStrFromStr(const char* Str);

    //拡張子取得
	std::string GetExtension(const std::string& Path);

	void GetDivideStr(const std::string& Str, std::string* Dir, std::string* FileName);

    Vec2<float> ConvertWorldToScreen(Vec3<float> WorldPos,
        const Matrix& ViewMat, const Matrix& ProjMat, const Vec2<float>& WinSize, float* CamDist = nullptr);
	Vec3<float> ConvertScreenToWorld(Vec2<float> ScreenPos, float Z,
		const Matrix& ViewMat, const Matrix& ProjMat, const Vec2<int>&WinSize);

    //メッセージボックスの表示
    void AppearMessageBox(std::string WindowName, std::string Text);
    //メッセージボックスの表示（Yes or No）
    bool AppearMessageBoxYN(std::string WindowName, std::string Text);

	//中心座標が画面内に映っているか
	bool InScreen(Vec2<float>ScreenPos, Vec2<float> WinSize);
	bool InScreen(Vec3<float> WorldPos, const Matrix& ViewMat, const Matrix& ProjMat, Vec2<float> WinSize);

	bool ExistFile(const std::string FilePass);

	//ベジエ用
	float GetYFromXOnBezier(float x, const Vec2<float>& a, const Vec2<float>& b, uint8_t n);
	//ベジエ(制御点指定)
	float GetBezierFromControlPoint(float t, int ControlPointNum, float* ControlPointArray);

	//乱数
	int GetRand(int Min, int Max);
	int GetRand(int Max);
	float GetRand(float Min, float Max);
	Vec2<float> GetRand(Vec2<float> Min, Vec2<float> Max);
	Vec3<float> GetRand(Vec3<float> Min, Vec3<float> Max);
	float GetRand(float Max);
	Vec2<float> GetRand(Vec2<float> Max);
	Vec3<float> GetRand(Vec3<float> Max);
	//正負ランダム
	int GetRandPlusMinus();
	//基準の値から前後乱数算出
	int GetRandFromCenter(int CenterNum, int Range);

	//確率(MAX:100%)、引数に渡した確率でtrueを返す
	bool Probability(float Rate);

	/// <summary>
	/// 指定した桁数の数字を取得
	/// </summary>
	/// <param name="From">取得先</param>
	/// <param name="Digit">指定桁数</param>
	/// <param name="CountFromLeft">桁数を左から数えるか</param>
	/// <returns></returns>
	int GetSpecifiedDigitNum(int From, int Digit, bool CountFromLeft);
	//桁数を算出
	int GetDigit(int Num);

	//符号取得
	int GetNumSign(int Num);
	int GetNumSign(float Num);

	//データ読み込み
	void LoadData(FILE* Fp, std::string DataName, void* Data, size_t Size, int ElementNum);
	//データ保存
	void SaveData(FILE* Fp, std::string DataName, const void* Data, size_t Size, int ElementNum);

    bool operator!=(const Matrix& lhs, const Matrix& rhs);
    bool operator==(const Matrix& lhs, const Matrix& rhs);

    enum EASE_CHANGE_TYPE
    {
        In, Out, InOut, OutIn, EASE_CHANGE_TYPE_NUM
    };
    enum EASING_TYPE
    {
        Liner, Quad, Cubic, Quart, Quint, Sine, Exp, Circ, Elastic, Back, Bounce, EASING_TYPE_NUM
    };

    class Math
    {
    private:
        static float LinerFunc(float t, float totaltime, float min, float max);

        static float QuadIn(float t, float totaltime, float min, float max);
        static float QuadOut(float t, float totaltime, float min, float max);
        static float QuadInOut(float t, float totaltime, float min, float max);
        static float QuadOutIn(float t, float totaltime, float min, float max);

        static float CubicIn(float t, float totaltime, float min, float max);
        static float CubicOut(float t, float totaltime, float min, float max);
        static float CubicInOut(float t, float totaltime, float min, float max);
        static float CubicOutIn(float t, float totaltime, float min, float max);

        static float QuartIn(float t, float totaltime, float min, float max);
        static float QuartOut(float t, float totaltime, float min, float max);
        static float QuartInOut(float t, float totaltime, float min, float max);
        static float QuartOutIn(float t, float totaltime, float min, float max);

        static float QuintIn(float t, float totaltime, float min, float max);
        static float QuintOut(float t, float totaltime, float min, float max);
        static float QuintInOut(float t, float totaltime, float min, float max);
        static float QuintOutIn(float t, float totaltime, float min, float max);

        static float SineIn(float t, float totaltime, float min, float max);
        static float SineOut(float t, float totaltime, float min, float max);
        static float SineInOut(float t, float totaltime, float min, float max);
        static float SineOutIn(float t, float totaltime, float min, float max);

        static float ExpIn(float t, float totaltime, float min, float max);
        static float ExpOut(float t, float totaltime, float min, float max);
        static float ExpInOut(float t, float totaltime, float min, float max);
        static float ExpOutIn(float t, float totaltime, float min, float max);

        static float CircIn(float t, float totaltime, float min, float max);
        static float CircOut(float t, float totaltime, float min, float max);
        static float CircInOut(float t, float totaltime, float min, float max);
        static float CircOutIn(float t, float totaltime, float min, float max);

        static float ElasticIn(float t, float totaltime, float min, float max);
        static float ElasticOut(float t, float totaltime, float min, float max);
        static float ElasticInOut(float t, float totaltime, float min, float max);
        static float ElasticOutIn(float t, float totaltime, float min, float max);

        static float BackIn(float t, float totaltime, float min, float max);
        static float BackOut(float t, float totaltime, float min, float max);
        static float BackInOut(float t, float totaltime, float min, float max);
        static float BackOutIn(float t, float totaltime, float min, float max);

        static float BounceIn(float t, float totaltime, float min, float max);
        static float BounceOut(float t, float totaltime, float min, float max);
        static float BounceInOut(float t, float totaltime, float min, float max);
        static float BounceOutIn(float t, float totaltime, float min, float max);

        //ラムダ式のための型
        using EasingFunction = std::function<float(float t, float totaltime, float min, float max)>;
        static EasingFunction easing[EASE_CHANGE_TYPE_NUM][EASING_TYPE_NUM];

        static float GetSplineSection(const float& p0, const float& p1, const float& p2, const float& p3, const float& t);
    public:
        static float Ease(EASE_CHANGE_TYPE EaseChangeType, EASING_TYPE EasingType, float T, float TotalTime, float Min, float Max);
        static Vec2<float> Ease(EASE_CHANGE_TYPE EaseChangeType, EASING_TYPE EasingType, float T, float TotalTime, Vec2<float> Min, Vec2<float> Max);
        static Vec3<float> Ease(EASE_CHANGE_TYPE EaseChangeType, EASING_TYPE EasingType, float T, float TotalTime, Vec3<float> Min, Vec3<float> Max);
        static Vec4<float> Ease(EASE_CHANGE_TYPE EaseChangeType, EASING_TYPE EasingType, float T, float TotalTime, Vec4<float> Min, Vec4<float> Max);

        static float Ease(EASE_CHANGE_TYPE EaseChangeType, EASING_TYPE EasingType, float Rate, float Min, float Max);
        static Vec2<float> Ease(EASE_CHANGE_TYPE EaseChangeType, EASING_TYPE EasingType, float Rate, Vec2<float> Min, Vec2<float> Max);
        static Vec3<float> Ease(EASE_CHANGE_TYPE EaseChangeType, EASING_TYPE EasingType, float Rate, Vec3<float> Min, Vec3<float> Max);
        static Vec4<float> Ease(EASE_CHANGE_TYPE EaseChangeType, EASING_TYPE EasingType, float Rate, Vec4<float> Min, Vec4<float> Max);

        static float Lerp(float Min, float Max, float Rate)
        {
            return Ease(In, Liner, Rate, Min, Max);
        }
        static Vec2<float> Lerp(Vec2<float> Min, Vec2<float> Max, float Rate)
        {
            return Ease(In, Liner, Rate, Min, Max);
        }
        static Vec3<float> Lerp(Vec3<float> Min, Vec3<float> Max, float Rate)
        {
            return Ease(In, Liner, Rate, Min, Max);
        }
        static Vec4<float> Lerp(Vec4<float> Min, Vec4<float> Max, float Rate)
        {
            return Ease(In, Liner, Rate, Min, Max);
        }

        static float Lerp(float Min, float Max, float T, float TotalTime)
        {
            return Ease(In, Liner, T, TotalTime, Min, Max);
        }
        static Vec2<float> Lerp(Vec2<float> Min, Vec2<float> Max, float T, float TotalTime)
        {
            return Ease(In, Liner, T, TotalTime, Min, Max);
        }
        static Vec3<float> Lerp(Vec3<float> Min, Vec3<float> Max, float T, float TotalTime)
        {
            return Ease(In, Liner, T, TotalTime, Min, Max);
        }
        static Vec4<float> Lerp(Vec4<float> Min, Vec4<float> Max, float T, float TotalTime)
        {
            return Ease(In, Liner, T, TotalTime, Min, Max);
        }

        static Vec3<float> Slerp(Vec3<float> Min, Vec3<float> Max, float Rate)
        {
            //各単位ベクトルを求める
            Vec3<float>s = Min.GetNormal(), e = Max.GetNormal();

            //2ベクトル間の角度（鋭角側）
            float angle = acos(s.Dot(e));

            //sinθ
            float sinTh = sin(angle);

            //補間係数
            float ps = sin(angle * (1 - Rate));
            float pe = sin(angle * Rate);

            return ((s * ps + e * pe) / sinTh).GetNormal();
        }


        static float GetSpline(const float& T, const int& P1Idx, const std::vector<float>& Array, bool Loop = false);
        static float GetSpline(const int& Timer, const int& TotalTime, const int& P1Idx, const std::vector<float>& Array, bool Loop = false);
        static Vec2<float> GetSpline(const float& T, const int& P1Idx, const std::vector<Vec2<float>>& Array, bool Loop = false);
        static Vec2<float> GetSpline(const int& Timer, const int& TotalTime, const int& P1Idx, const std::vector<Vec2<float>>& Array, bool Loop = false);
        static Vec3<float> GetSpline(const float& T, const int& P1Idx, const std::vector<Vec3<float>>& Array, bool Loop = false);
        static Vec3<float> GetSpline(const int& Timer, const int& TotalTime, const int& P1Idx, const std::vector<Vec3<float>>& Array, bool Loop = false);
        static Vec4<float> GetSpline(const float& T, const int& P1Idx, const std::vector<Vec4<float>>& Array, bool Loop = false);
        static Vec4<float> GetSpline(const int& Timer, const int& TotalTime, const int& P1Idx, const std::vector<Vec4<float>>& Array, bool Loop = false);

        //aをbに向かせる回転を求める
        static Quaternion GetLookAtQuaternion(const Vec3<float>& VecA, const Vec3<float>& VecB);

        //2Dベクトルを回転させる
        static Vec2<float>RotateVec2(const Vec2<float>& Vec, const float& Radian);

        static Vec2<float> GetCenterVec2(Vec2<float> From, Vec2<float> To);
        static Vec3<float> GetCenterVec3(Vec3<float> From, Vec3<float> To);

        //２点を通る直線の傾き
        static float GetLineSlope(Vec2<float> From, Vec2<float> To);
        //1つのベクトルから角度を取得
        static Angle GetAngle(Vec2<float>Vec);
        //２つのベクトルの角度差を取得
        static Angle GetAngleAbs(Vec2<float>From, Vec2<float>To);

        //Vec3にクォータニオンを反映
        static Vec3<float>TransformVec3(const Vec3<float>& Value, const Quaternion& Quaternion);
        //Vec3に行列を反映
        static Vec3<float>TransformVec3(const Vec3<float>& Value, const Matrix& Mat);
        //回転軸を指定して回転
        static Vec3<float>TransformVec3(const Vec3<float>& Value, const Vec3<float>& Axis, const Angle& Angle);

        /// <summary>
        /// 指定した値が範囲内においてどの位置にいるか
        /// </summary>
        /// <param name="arg_min">範囲下限（下回る場合は0）</param>
        /// <param name="arg_max">範囲上限（上回る場合は1）</param>
        /// <param name="arg_val">対象の値</param>
        /// <returns>0.0f ~ 1.0f</returns>
        static float GetRateInRange(float arg_min, float arg_max, float arg_val);
        static float GetRateInRange(int arg_min, int arg_max, int arg_val);
    };

    struct EasingParameter
    {
        EASE_CHANGE_TYPE m_changeType = In;
        EASING_TYPE m_easeType = Quad;

        float Calculate(float T, float TotalTime, float Min, float Max)const
        {
            return Math::Ease(m_changeType, m_easeType, T, TotalTime, Min, Max);
        }
        Vec2<float> Calculate(float T, float TotalTime, Vec2<float> Min, Vec2<float> Max)const
        {
            return Math::Ease(m_changeType, m_easeType, T, TotalTime, Min, Max);
        }
        Vec3<float> Calculate(float T, float TotalTime, Vec3<float> Min, Vec3<float> Max)const
        {
            return Math::Ease(m_changeType, m_easeType, T, TotalTime, Min, Max);
        }
        float Calculate(float Rate, float Min, float Max)const
        {
            return Math::Ease(m_changeType, m_easeType, Rate, Min, Max);
        }
        Vec2<float>Calculate(float Rate, Vec2<float> Min, Vec2<float> Max)const
        {
            return Math::Ease(m_changeType, m_easeType, Rate, Min, Max);
        }
        Vec3<float>Calculate(float Rate, Vec3<float> Min, Vec3<float> Max)const
        {
            return Math::Ease(m_changeType, m_easeType, Rate, Min, Max);
        }
        void ImguiDebug(const std::string& Tag);
    };
}