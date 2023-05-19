#include"KuroEngine.h"
#include<windows.h>
#include <cassert>

#include<fstream>
#include<iostream>
#include<utility>

bool KuroEngine::operator!=(const Matrix& lhs, const Matrix& rhs)
{
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			if (lhs.r[i].m128_f32[j] != rhs.r[i].m128_f32[j])return true;
		}
	}
	return false;
}

bool KuroEngine::operator==(const Matrix& lhs, const Matrix& rhs)
{
	return !(lhs != rhs);
}

std::wstring KuroEngine::GetWideStrFromStr(const std::string& Str)
{
	auto num1 = MultiByteToWideChar(
		CP_ACP, MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
		Str.c_str(), -1, nullptr, 0);

	std::wstring wstr;
	wstr.resize(num1);

	auto num2 = MultiByteToWideChar(
		CP_ACP, MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
		Str.c_str(), -1, &wstr[0], num1);

	assert(num1 == num2);

	return wstr;
}

std::wstring KuroEngine::GetWideStrFromStr(const char* Str)
{
	return GetWideStrFromStr(std::string(Str));
}

std::string KuroEngine::GetExtension(const std::string& Path)
{
	int idx = static_cast<int>(Path.rfind('.'));
	return Path.substr(idx + 1, Path.length() - idx - 1);
}

void KuroEngine::GetDivideStr(const std::string& Str, std::string* Dir, std::string* FileName)
{
	int idx = static_cast<int>(Str.rfind('/'));
	std::string fileName = Str.substr(idx + 1, Str.length() - idx - 1);
	if (FileName != nullptr)
	{
		*FileName = fileName;
	}
	if (Dir != nullptr)
	{
		*Dir = Str;
		for (int i = 0; i < fileName.size(); ++i)Dir->pop_back();
	}
}


KuroEngine::Vec2<float> KuroEngine::ConvertWorldToScreen(Vec3<float> WorldPos, const Matrix& ViewMat, const Matrix& ProjMat, const Vec2<float>& WinSize, float* CamDist)
{
	//ビュー行列適応
	auto pos = Math::TransformVec3(WorldPos, ViewMat);
	//プロジェクション行列適応
	pos = Math::TransformVec3(pos, ProjMat);
	if (CamDist)*CamDist = pos.z;
	pos /= abs(pos.z);

	//ビューポート行列
	Matrix viewPortMat = XMMatrixIdentity();
	viewPortMat.r[0].m128_f32[0] = WinSize.x / 2.0f;
	viewPortMat.r[1].m128_f32[1] = -WinSize.y / 2.0f;
	viewPortMat.r[3].m128_f32[0] = WinSize.x / 2.0f;
	viewPortMat.r[3].m128_f32[1] = WinSize.y / 2.0f;
	//ビューポート行列適応
	pos = Math::TransformVec3(pos, viewPortMat);

	return Vec2<float>(pos.x, pos.y);
}

KuroEngine::Vec3<float> KuroEngine::ConvertScreenToWorld(Vec2<float> ScreenPos, float Z, const Matrix& ViewMat, const Matrix& ProjMat, const Vec2<int>& WinSize)
{
	Matrix invView, invPrj, invViewPort;
	invView = XMMatrixInverse(nullptr, ViewMat);
	invPrj = XMMatrixInverse(nullptr, ProjMat);

	invViewPort = XMMatrixIdentity();
	invViewPort.r[0].m128_f32[0] = (float)WinSize.x / 2.0f;
	invViewPort.r[1].m128_f32[1] = (float)-WinSize.y / 2.0f;
	invViewPort.r[3].m128_f32[0] = (float)WinSize.x / 2.0f;
	invViewPort.r[3].m128_f32[1] = (float)WinSize.y / 2.0f;
	invViewPort = XMMatrixInverse(nullptr, invViewPort);

	XMVECTOR pos;
	auto screenPosXMFLOAT3 = XMFLOAT3(ScreenPos.x, ScreenPos.y, Z);
	pos = XMLoadFloat3(&screenPosXMFLOAT3);
	/*
		※授業資料05_03.射影変換　透視投影変換を参照
		z(0.0f ~ 1.0f)
		0.0f で proj のnear面
		1.0f で far面の位置
	*/
	pos = XMVector3Transform(pos, invViewPort);
	pos = XMVector3TransformCoord(pos, invPrj);
	//Coordでw除算を行ってくれる
	/*
		w除算をすることによって遠くのものがすぼまっていき、
		画面内に収まるようにしてくれる
		Coordがないとスクリーンから見切れた、みたいになる
	*/
	pos = XMVector3Transform(pos, invView);

	Vec3<float> result;
	result.x = pos.m128_f32[0];
	result.y = pos.m128_f32[1];
	result.z = pos.m128_f32[2];
	//result.x = base.r[0].m128_f32[0];
	//result.y = base.r[0].m128_f32[1];

	return result;
}

void KuroEngine::AppearMessageBox(std::string WindowName, std::string Text)
{
	MessageBeep(MB_OK);
	MessageBox(NULL, GetWideStrFromStr(Text).c_str(), GetWideStrFromStr(WindowName).c_str(), MB_OK);
}

bool KuroEngine::AppearMessageBoxYN(std::string WindowName, std::string Text)
{
	MessageBeep(MB_OK);
	int button = MessageBox(NULL, GetWideStrFromStr(Text).c_str(), GetWideStrFromStr(WindowName).c_str(), MB_YESNO | MB_ICONQUESTION);
	return button == IDYES;
}


bool KuroEngine::InScreen(Vec2<float> ScreenPos, Vec2<float> WinSize)
{
	if (ScreenPos.x < 0)return false;
	if (WinSize.x < ScreenPos.x)return false;
	if (ScreenPos.y < 0)return false;
	if (WinSize.y < ScreenPos.y)return false;
	return true;
}

bool KuroEngine::InScreen(Vec3<float> WorldPos, const Matrix& ViewMat, const Matrix& ProjMat, Vec2<float> WinSize)
{
	Vec2<float> screenPos = KuroEngine::ConvertWorldToScreen(WorldPos, ViewMat, ProjMat, WinSize);
	return InScreen(screenPos, WinSize);
}

bool KuroEngine::ExistFile(const std::string FilePass)
{
	std::ifstream ifs(FilePass);
	return ifs.is_open();
}

float KuroEngine::GetYFromXOnBezier(float x, const Vec2<float>& a, const Vec2<float>& b, uint8_t n)
{
	if (a.x == a.y && b.x == b.y)
	{
		return x;		//計算不要
	}

	float t = x;
	const float k0 = 1 + 3 * a.x - 3 * b.x;	//t^3の係数
	const float k1 = 3 * b.x - 6 * a.x;	//t^2の係数
	const float k2 = 3 * a.x;	//tの係数

	constexpr float epsilon = 0.0005f;	//誤差許容範囲

	//tを近似で求める
	for (int i = 0; i < n; i++)
	{
		//f(t)を求める
		auto ft = k0 * t * t * t + k1 * t * t + k2 * t - x;
		//もし結果が０に近い（誤差の範囲内）なら打ち切る
		if (ft <= epsilon && ft >= -epsilon)
		{
			break;
		}
		t -= ft / 2;	//刻む
	}

	//求めたいtは既に求めているのでyを計算
	auto r = 1 - t;
	return t * t * t + 3 * t * t * r * b.y + 3 * t * r * r * a.y;
}

//階乗を求める
int GetFactorial(int num)
{
	int result = 1;
	for (int i = num; 0 < i; --i)
	{
		result *= i;
	}
	return result;
}

//n個の中からk個を選ぶときの組み合わせの数
int GetSelectPatternNum(int n, int k)
{
	if (k == 0)return 1;

	//分子
	int upper = GetFactorial(n);
	//分母
	int a = GetFactorial(k);
	int b = GetFactorial(n - k);
	int lower = a * b;

	return upper / lower;
}

float KuroEngine::GetBezierFromControlPoint(float t, int ControlPointNum, float* ControlPointArray)
{
	double result = 0;
	for (int i = 0; i < ControlPointNum; ++i)
	{
		int coefficient = GetSelectPatternNum(ControlPointNum, i);	//二項係数
		double a = pow(t, i);	// t の i 乗
		double b = pow((1 - t), (ControlPointNum - i));	// (1-t) の n-i 乗
		result += coefficient * a * b * ControlPointArray[i];		//制御点の重み付けをして加算(Σ)
	}
	return static_cast<float>(result);
}

int KuroEngine::GetRand(int Min, int Max)
{
	int result = (Max - Min) * rand() / RAND_MAX + Min;
	return result;
}

int KuroEngine::GetRand(int Max)
{
	return GetRand(0, Max);
}

float KuroEngine::GetRand(float Min, float Max)
{
	double result = (Max - Min) * ((double)rand() / RAND_MAX) + Min;
	return static_cast<float>(result);
}

KuroEngine::Vec2<float> KuroEngine::GetRand(Vec2<float> Min, Vec2<float> Max)
{
	return Vec2<float>(GetRand(Min.x, Max.x), GetRand(Min.y, Max.y));
}

KuroEngine::Vec3<float> KuroEngine::GetRand(Vec3<float> Min, Vec3<float> Max)
{
	return Vec3<float>(GetRand(Min.x, Max.x), GetRand(Min.y, Max.y), GetRand(Min.z, Max.z));
}

float KuroEngine::GetRand(float Max)
{
	return GetRand(0.0f, Max);
}

KuroEngine::Vec2<float> KuroEngine::GetRand(Vec2<float> Max)
{
	return Vec2<float>(GetRand(Max.x), GetRand(Max.y));
}

KuroEngine::Vec3<float> KuroEngine::GetRand(Vec3<float> Max)
{
	return Vec3<float>(GetRand(Max.x), GetRand(Max.y), GetRand(Max.z));
}

int KuroEngine::GetRandPlusMinus()
{
	int r = GetRand(1, 100);
	if (50 <= r)return 1;
	else return -1;
}

int KuroEngine::GetRandFromCenter(int CenterNum, int Range)
{
	return CenterNum + GetRand(0, Range) * GetRandPlusMinus();
}

bool KuroEngine::Probability(float Rate)
{
	Rate = std::clamp(Rate, 0.0f, 100.0f);
	float rand = GetRand(100.0f);
	return rand < Rate;
}


int KuroEngine::GetSpecifiedDigitNum(int From, int Digit, bool CountFromLeft)
{
	int result = 0;
	int iMax = CountFromLeft ? (GetDigit(From) - Digit - 1) : Digit;
	for (int i = iMax; 0 <= i; i--)
	{
		result = From % 10;				//6-i桁目の数字を記録
		From /= 10;					//桁を減らす
	}
	return result;
}

int KuroEngine::GetDigit(int Num)
{
	if (Num == 0)return 1;

	int result = 0;
	while (Num != 0)
	{
		Num /= 10;
		result++;
	}
	return result;
}

int KuroEngine::GetNumSign(int Num)
{
	if (Num == 0)return 0;
	if (Num < 0)return -1;
	return 1;
}

int KuroEngine::GetNumSign(float Num)
{
	if (Num == 0.0f)return 0;
	if (Num < 0.0f)return -1;
	return 1;
}

void KuroEngine::LoadData(FILE* Fp, std::string DataName, void* Data, size_t Size, int ElementNum)
{
	if (fread(Data, Size, ElementNum, Fp) < 1)
	{
		printf("　%sの読み込みに失敗\n", DataName.c_str());
		assert(0);
	}
}

void KuroEngine::SaveData(FILE* Fp, std::string DataName, const void* Data, size_t Size, int ElementNum)
{
	if (fwrite(Data, Size, ElementNum, Fp) < 1)
	{
		printf("　%sの書き込みに失敗\n", DataName.c_str());
		assert(0);
	}
}


#pragma region Easing
KuroEngine::Math::EasingFunction KuroEngine::Math::easing[EASE_CHANGE_TYPE_NUM][EASING_TYPE_NUM] =
{
	{LinerFunc,QuadIn,CubicIn,QuartIn,QuintIn,SineIn,ExpIn,CircIn,ElasticIn,BackIn,BounceIn},
	 {LinerFunc,QuadOut,CubicOut,QuartOut,QuintOut,SineOut,ExpOut,CircOut,ElasticOut,BackOut,BounceOut},
	 {LinerFunc,QuadInOut,CubicInOut,QuartInOut,QuintInOut,SineInOut,ExpInOut,CircInOut,ElasticInOut,BackInOut,BounceInOut},
	 {LinerFunc,QuadOutIn,CubicOutIn,QuartOutIn,QuintOutIn,SineOutIn,ExpOutIn,CircOutIn,ElasticOutIn,BackOutIn,BounceOutIn}
};

const float PI = 3.14159265359f;

float KuroEngine::Math::LinerFunc(float t, float totaltime, float min, float max)
{
	float rate = t / totaltime;
	if (abs(min - max) < 0.001f)return max;
	return (1 - rate) * min + rate * max;
}

float KuroEngine::Math::QuadIn(float t, float totaltime, float min, float max)
{
	max -= min;
	t /= totaltime;
	return max * t * t + min;
}

float KuroEngine::Math::QuadOut(float t, float totaltime, float min, float max)
{
	max -= min;
	t /= totaltime;
	return -max * t * (t - 2) + min;
}

float KuroEngine::Math::QuadInOut(float t, float totaltime, float min, float max)
{
	if (t < totaltime / 2.0f)
	{
		return QuadIn(t, totaltime / 2.0f, min, max * 0.5f);
	}
	return QuadOut(t - totaltime / 2.0f, totaltime / 2.0f, max * 0.5f, max);
}

float KuroEngine::Math::QuadOutIn(float t, float totaltime, float min, float max)
{
	if (t < totaltime / 2.0f)
	{
		return QuadOut(t, totaltime / 2.0f, min, max * 0.5f);
	}
	return QuadIn(t - totaltime / 2.0f, totaltime / 2.0f, max * 0.5f, max);
}

float KuroEngine::Math::CubicIn(float t, float totaltime, float min, float max)
{
	max -= min;
	t /= totaltime;
	return max * t * t * t + min;
}

float KuroEngine::Math::CubicOut(float t, float totaltime, float min, float max)
{
	max -= min;
	t = t / totaltime - 1;
	return max * (t * t * t + 1) + min;
}

float KuroEngine::Math::CubicInOut(float t, float totaltime, float min, float max)
{
	if (t < totaltime / 2.0f)
	{
		return CubicIn(t, totaltime / 2.0f, min, max * 0.5f);
	}
	return CubicOut(t - totaltime / 2.0f, totaltime / 2.0f, max * 0.5f, max);
}

float KuroEngine::Math::CubicOutIn(float t, float totaltime, float min, float max)
{
	if (t < totaltime / 2.0f)
	{
		return CubicOut(t, totaltime / 2.0f, min, max * 0.5f);
	}
	return CubicIn(t - totaltime / 2.0f, totaltime / 2.0f, max * 0.5f, max);
}

float KuroEngine::Math::QuartIn(float t, float totaltime, float min, float max)
{
	max -= min;
	t /= totaltime;
	return max * t * t * t * t + min;
}

float KuroEngine::Math::QuartOut(float t, float totaltime, float min, float max)
{
	max -= min;
	t = t / totaltime - 1;
	return -max * (t * t * t * t - 1) + min;
}

float KuroEngine::Math::QuartInOut(float t, float totaltime, float min, float max)
{
	if (t < totaltime / 2.0f)
	{
		return QuartIn(t, totaltime / 2.0f, min, max * 0.5f);
	}
	return QuartOut(t - totaltime / 2.0f, totaltime / 2.0f, max * 0.5f, max);
}

float KuroEngine::Math::QuartOutIn(float t, float totaltime, float min, float max)
{
	if (t < totaltime / 2.0f)
	{
		return QuartOut(t, totaltime / 2.0f, min, max * 0.5f);
	}
	return QuartIn(t - totaltime / 2.0f, totaltime / 2.0f, max * 0.5f, max);
}

float KuroEngine::Math::QuintIn(float t, float totaltime, float min, float max)
{
	max -= min;
	t /= totaltime;
	return max * t * t * t * t * t + min;
}

float KuroEngine::Math::QuintOut(float t, float totaltime, float min, float max)
{
	max -= min;
	t = t / totaltime - 1;
	return max * (t * t * t * t * t + 1) + min;
}

float KuroEngine::Math::QuintInOut(float t, float totaltime, float min, float max)
{
	if (t < totaltime / 2.0f)
	{
		return QuintIn(t, totaltime / 2.0f, min, max * 0.5f);
	}
	return QuintOut(t - totaltime / 2.0f, totaltime / 2.0f, max * 0.5f, max);
}

float KuroEngine::Math::QuintOutIn(float t, float totaltime, float min, float max)
{
	if (t < totaltime / 2.0f)
	{
		return QuintOut(t, totaltime / 2.0f, min, max * 0.5f);
	}
	return QuintIn(t - totaltime / 2.0f, totaltime / 2.0f, max * 0.5f, max);
}

float KuroEngine::Math::SineIn(float t, float totaltime, float min, float max)
{
	max -= min;
	return -max * cos(t * (PI * 90 / 180) / totaltime) + max + min;
}

float KuroEngine::Math::SineOut(float t, float totaltime, float min, float max)
{
	max -= min;
	return max * sin(t * (PI * 90 / 180) / totaltime) + min;
}

float KuroEngine::Math::SineInOut(float t, float totaltime, float min, float max)
{
	if (t < totaltime / 2.0f)
	{
		return SineIn(t, totaltime / 2.0f, min, max * 0.5f);
	}
	return SineOut(t - totaltime / 2.0f, totaltime / 2.0f, max * 0.5f, max);
}

float KuroEngine::Math::SineOutIn(float t, float totaltime, float min, float max)
{
	if (t < totaltime / 2.0f)
	{
		return SineOut(t, totaltime / 2.0f, min, max * 0.5f);
	}
	return SineIn(t - totaltime / 2.0f, totaltime / 2.0f, max * 0.5f, max);
}

float KuroEngine::Math::ExpIn(float t, float totaltime, float min, float max)
{
	max -= min;
	return static_cast<float>(t == 0.0 ? min : max * pow(2, 10 * (t / totaltime - 1)) + min);
}

float KuroEngine::Math::ExpOut(float t, float totaltime, float min, float max)
{
	max -= min;
	return static_cast<float>(t == totaltime ? max + min : max * (-pow(2, -10 * t / totaltime) + 1) + min);
}

float KuroEngine::Math::ExpInOut(float t, float totaltime, float min, float max)
{
	if (t < totaltime / 2.0f)
	{
		return ExpIn(t, totaltime / 2.0f, min, max * 0.5f);
	}
	return ExpOut(t - totaltime / 2.0f, totaltime / 2.0f, max * 0.5f, max);
}

float KuroEngine::Math::ExpOutIn(float t, float totaltime, float min, float max)
{
	if (t < totaltime / 2.0f)
	{
		return ExpOut(t, totaltime / 2.0f, min, max * 0.5f);
	}
	return ExpIn(t - totaltime / 2.0f, totaltime / 2.0f, max * 0.5f, max);
}

float KuroEngine::Math::CircIn(float t, float totaltime, float min, float max)
{
	max -= min;
	t /= totaltime;
	return -max * (sqrt(1 - t * t) - 1) + min;
}

float KuroEngine::Math::CircOut(float t, float totaltime, float min, float max)
{
	max -= min;
	t = t / totaltime - 1;
	return max * sqrt(1 - t * t) + min;
}

float KuroEngine::Math::CircInOut(float t, float totaltime, float min, float max)
{
	if (t < totaltime / 2.0f)
	{
		return CircIn(t, totaltime / 2.0f, min, max * 0.5f);
	}
	return CircOut(t - totaltime / 2.0f, totaltime / 2.0f, max * 0.5f, max);
}

float KuroEngine::Math::CircOutIn(float t, float totaltime, float min, float max)
{
	if (t < totaltime / 2.0f)
	{
		return CircOut(t, totaltime / 2.0f, min, max * 0.5f);
	}
	return CircIn(t - totaltime / 2.0f, totaltime / 2.0f, max * 0.5f, max);
}

float KuroEngine::Math::ElasticIn(float t, float totaltime, float min, float max)
{
	max -= min;
	t /= totaltime;

	float s = 1.70158f;
	float p = totaltime * 0.3f;
	float a = max;

	if (t == 0) return min;
	if (t == 1) return min + max;

	if (a < abs(max))
	{
		a = max;
		s = p / 4;
	}
	else
	{
		s = p / (2 * PI) * asin(max / a);
	}

	t = t - 1;
	return static_cast<float>(-(a * pow(2, 10 * t) * sin((t * totaltime - s) * (2 * PI) / p)) + min);
}

float KuroEngine::Math::ElasticOut(float t, float totaltime, float min, float max)
{
	max -= min;
	t /= totaltime;

	float s = 1.70158f;
	float p = totaltime * 0.3f; ;
	float a = max;

	if (t == 0) return min;
	if (t == 1) return min + max;

	if (a < abs(max))
	{
		a = max;
		s = p / 4;
	}
	else
	{
		s = p / (2 * PI) * asin(max / a);
	}

	return static_cast<float>(a * pow(2, -10 * t) * sin((t * totaltime - s) * (2 * PI) / p) + max + min);
}

float KuroEngine::Math::ElasticInOut(float t, float totaltime, float min, float max)
{
	if (t < totaltime / 2.0f)
	{
		return ElasticIn(t, totaltime / 2.0f, min, max * 0.5f);
	}
	return ElasticOut(t - totaltime / 2.0f, totaltime / 2.0f, max * 0.5f, max);
}

float KuroEngine::Math::ElasticOutIn(float t, float totaltime, float min, float max)
{
	if (t < totaltime / 2.0f)
	{
		return ElasticOut(t, totaltime / 2.0f, min, max * 0.5f);
	}
	return ElasticIn(t - totaltime / 2.0f, totaltime / 2.0f, max * 0.5f, max);
}

float KuroEngine::Math::BackIn(float t, float totaltime, float min, float max)
{
	float s = 1.70158f;
	max -= min;
	t /= totaltime;
	return max * t * t * ((s + 1) * t - s) + min;
}

float KuroEngine::Math::BackOut(float t, float totaltime, float min, float max)
{
	float s = 1.70158f;
	max -= min;
	t = t / totaltime - 1;
	return max * (t * t * ((s + 1) * t + s) + 1) + min;
}

float KuroEngine::Math::BackInOut(float t, float totaltime, float min, float max)
{
	if (t < totaltime / 2.0f)
	{
		return BackIn(t, totaltime / 2.0f, min, max * 0.5f);
	}
	return BackOut(t - totaltime / 2.0f, totaltime / 2.0f, max * 0.5f, max);
}

float KuroEngine::Math::BackOutIn(float t, float totaltime, float min, float max)
{
	if (t < totaltime / 2.0f)
	{
		return BackOut(t, totaltime / 2.0f, min, max * 0.5f);
	}
	return BackIn(t - totaltime / 2.0f, totaltime / 2.0f, max * 0.5f, max);
}

float KuroEngine::Math::BounceIn(float t, float totaltime, float min, float max)
{
	max -= min;
	return max - BounceOut(totaltime - t, totaltime, 0, max) + min;
}

float KuroEngine::Math::BounceOut(float t, float totaltime, float min, float max)
{
	max -= min;
	t /= totaltime;

	if (t < 1.0f / 2.75f)
	{
		return max * (7.5625f * t * t) + min;
	}
	else if (t < 2.0f / 2.75f)
	{
		t -= 1.5f / 2.75f;
		return max * (7.5625f * t * t + 0.75f) + min;
	}
	else if (t < 2.5f / 2.75f)
	{
		t -= 2.25f / 2.75f;
		return max * (7.5625f * t * t + 0.9375f) + min;
	}
	else
	{
		t -= 2.625f / 2.75f;
		return max * (7.5625f * t * t + 0.984375f) + min;
	}
}

float KuroEngine::Math::BounceInOut(float t, float totaltime, float min, float max)
{
	if (t < totaltime / 2.0f)
	{
		return BounceIn(t, totaltime / 2.0f, min, max * 0.5f);
	}
	return BounceOut(t - totaltime / 2.0f, totaltime / 2.0f, max * 0.5f, max);
}

float KuroEngine::Math::BounceOutIn(float t, float totaltime, float min, float max)
{
	if (t < totaltime / 2.0f)
	{
		return BounceOut(t, totaltime / 2.0f, min, max * 0.5f);
	}
	return BounceIn(t - totaltime / 2.0f, totaltime / 2.0f, max * 0.5f, max);
}

float KuroEngine::Math::Ease(EASE_CHANGE_TYPE EaseChangeType, EASING_TYPE EasingType, float T, float TotalTime, float Min, float Max)
{
	if (Min == Max)return Min;
	if (TotalTime < T)T = TotalTime;
	return easing[EaseChangeType][EasingType](T, TotalTime, Min, Max);
}

KuroEngine::Vec2<float> KuroEngine::Math::Ease(EASE_CHANGE_TYPE EaseChangeType, EASING_TYPE EasingType, float T, float TotalTime, Vec2<float> Min, Vec2<float> Max)
{
	Vec2<float> result;
	result.x = Ease(EaseChangeType, EasingType, T, TotalTime, Min.x, Max.x);
	result.y = Ease(EaseChangeType, EasingType, T, TotalTime, Min.y, Max.y);
	return result;
}

KuroEngine::Vec3<float> KuroEngine::Math::Ease(EASE_CHANGE_TYPE EaseChangeType, EASING_TYPE EasingType, float T, float TotalTime, Vec3<float> Min, Vec3<float> Max)
{
	Vec3<float> result;
	result.x = Ease(EaseChangeType, EasingType, T, TotalTime, Min.x, Max.x);
	result.y = Ease(EaseChangeType, EasingType, T, TotalTime, Min.y, Max.y);
	result.z = Ease(EaseChangeType, EasingType, T, TotalTime, Min.z, Max.z);
	return result;
}

KuroEngine::Vec4<float> KuroEngine::Math::Ease(EASE_CHANGE_TYPE EaseChangeType, EASING_TYPE EasingType, float T, float TotalTime, Vec4<float> Min, Vec4<float> Max)
{
	Vec4<float> result;
	result.x = Ease(EaseChangeType, EasingType, T, TotalTime, Min.x, Max.x);
	result.y = Ease(EaseChangeType, EasingType, T, TotalTime, Min.y, Max.y);
	result.z = Ease(EaseChangeType, EasingType, T, TotalTime, Min.z, Max.z);
	result.w = Ease(EaseChangeType, EasingType, T, TotalTime, Min.w, Max.w);
	return result;
}

float KuroEngine::Math::Ease(EASE_CHANGE_TYPE EaseChangeType, EASING_TYPE EasingType, float Rate, float Min, float Max)
{
	return Ease(EaseChangeType, EasingType, Rate, 1.0f, Min, Max);
}

KuroEngine::Vec2<float> KuroEngine::Math::Ease(EASE_CHANGE_TYPE EaseChangeType, EASING_TYPE EasingType, float Rate, Vec2<float> Min, Vec2<float> Max)
{
	return Ease(EaseChangeType, EasingType, Rate, 1.0f, Min, Max);
}

KuroEngine::Vec3<float> KuroEngine::Math::Ease(EASE_CHANGE_TYPE EaseChangeType, EASING_TYPE EasingType, float Rate, Vec3<float> Min, Vec3<float> Max)
{
	return Ease(EaseChangeType, EasingType, Rate, 1.0f, Min, Max);
}

KuroEngine::Vec4<float> KuroEngine::Math::Ease(EASE_CHANGE_TYPE EaseChangeType, EASING_TYPE EasingType, float Rate, Vec4<float> Min, Vec4<float> Max)
{
	return Ease(EaseChangeType, EasingType, Rate, 1.0f, Min, Max);
}

#pragma endregion

#pragma region Spline

float KuroEngine::Math::GetSplineSection(const float& p0, const float& p1, const float& p2, const float& p3, const float& t)
{
	return 0.5f * ((2.0f * p1 + (-p0 + p2) * t) +
		(2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * (t * t) +
		(-p0 + 3.0f * p1 - 3.0f * p2 + p3) * (t * t * t));
}

float KuroEngine::Math::GetSpline(const float& T, const int& P1Idx, const std::vector<float>& Array, bool Loop)
{
	//１点以下ではできない
	if (Array.size() < 2)assert(0);

	int endIdx = static_cast<int>(Array.size()) - 1;

	int p0_idx = P1Idx - 1;
	int p1_idx = P1Idx;
	int p2_idx = P1Idx + 1;
	int p3_idx = p2_idx;

	if (p1_idx == 0)    //p1が先頭
	{
		if (Loop)p0_idx = endIdx;   //ループしてるならp0は末尾
		else p0_idx = p1_idx;
	}
	else
	{
		p0_idx = p1_idx - 1;    //一つ前
	}

	if (p2_idx == endIdx)   //p2が末尾
	{
		if (Loop)p3_idx = 0;    //ループしてるなら先頭
		else p3_idx = p2_idx;
	}
	else if (p2_idx == Array.size())
	{
		if (Loop)
		{
			p2_idx = 0;
			p3_idx = 1;
		}
		else
		{
			return Array[endIdx];
		}
	}
	else
	{
		p3_idx = p2_idx + 1;    //一つ後
	}

	float p0 = Array[p0_idx];
	float p1 = Array[p1_idx];
	float p2 = Array[p2_idx];
	float p3 = Array[p3_idx];

	return GetSplineSection(p0, p1, p2, p3, T);
}

float KuroEngine::Math::GetSpline(const int& Timer, const int& TotalTime, const int& P1Idx, const std::vector<float>& Array, bool Loop)
{
	float t = (float)Timer / TotalTime;
	return GetSpline(t, P1Idx, Array, Loop);
}

KuroEngine::Vec2<float> KuroEngine::Math::GetSpline(const float& T, const int& P1Idx, const std::vector<Vec2<float>>& Array, bool Loop)
{
	std::vector<float>x;
	std::vector<float>y;
	for (auto& element : Array)
	{
		x.emplace_back(element.x);
		y.emplace_back(element.y);
	}
	return Vec2<float>(GetSpline(T, P1Idx, x, Loop), GetSpline(T, P1Idx, y, Loop));
}

KuroEngine::Vec2<float> KuroEngine::Math::GetSpline(const int& Timer, const int& TotalTime, const int& P1Idx, const std::vector<Vec2<float>>& Array, bool Loop)
{
	float t = (float)Timer / TotalTime;
	return GetSpline(t, P1Idx, Array, Loop);
}

KuroEngine::Vec3<float> KuroEngine::Math::GetSpline(const float& T, const int& P1Idx, const std::vector<Vec3<float>>& Array, bool Loop)
{
	std::vector<float>x;
	std::vector<float>y;
	std::vector<float>z;
	for (auto& element : Array)
	{
		x.emplace_back(element.x);
		y.emplace_back(element.y);
		z.emplace_back(element.z);
	}
	return Vec3<float>(GetSpline(T, P1Idx, x, Loop), GetSpline(T, P1Idx, y, Loop), GetSpline(T, P1Idx, z, Loop));
}

KuroEngine::Vec3<float> KuroEngine::Math::GetSpline(const int& Timer, const int& TotalTime, const int& P1Idx, const std::vector<Vec3<float>>& Array, bool Loop)
{
	float t = (float)Timer / TotalTime;
	return GetSpline(t, P1Idx, Array, Loop);
}

KuroEngine::Vec4<float> KuroEngine::Math::GetSpline(const float& T, const int& P1Idx, const std::vector<Vec4<float>>& Array, bool Loop)
{
	std::vector<float>x;
	std::vector<float>y;
	std::vector<float>z;
	std::vector<float>w;
	for (auto& element : Array)
	{
		x.emplace_back(element.x);
		y.emplace_back(element.y);
		z.emplace_back(element.z);
		w.emplace_back(element.w);
	}
	return Vec4<float>(GetSpline(T, P1Idx, x, Loop), GetSpline(T, P1Idx, y, Loop), GetSpline(T, P1Idx, z, Loop), GetSpline(T, P1Idx, w, Loop));
}

KuroEngine::Vec4<float> KuroEngine::Math::GetSpline(const int& Timer, const int& TotalTime, const int& P1Idx, const std::vector<Vec4<float>>& Array, bool Loop)
{
	float t = (float)Timer / TotalTime;
	return GetSpline(t, P1Idx, Array, Loop);
}

#pragma endregion

#pragma region Matrix
KuroEngine::Quaternion KuroEngine::Math::GetLookAtQuaternion(const Vec3<float>& VecA, const Vec3<float>& VecB)
{
	//ベクトルAとベクトルBが同じ値だったら単位クォータニオンを返す。
	if ((VecA - VecB).IsZero())return XMQuaternionIdentity();

	//判定に使用する２つのベクトル
	auto a = VecA.GetNormal();
	auto b = VecB.GetNormal();
	//２つのベクトルの外積から法線(回転軸)を取得する。
	auto normal = a.Cross(b).GetNormal();
	//２つのベクトルの内積から回転量を取得し、acosでラジアンに直す。
	auto dot = a.Dot(b);
	float angle = acosf(a.Dot(b));
	//値が壊れていないかをチェック。壊れていたら単位クォータニオンを返す。
	if (std::isnan(angle)) {
		return XMQuaternionIdentity();
	}
	//２つのベクトルが逆方向を向いていないかをチェック。
	if (normal.Length() <= 0.1f) {

		//直行するベクトルを求めるため、無理やりベクトルを反転させる。
		if (fabs(a.y) < fabs(a.x) && fabs(a.z) < fabs(a.x))
		{
			std::swap(a.x, a.z);
			a.z = -a.z;
		}
		else if (fabs(a.z) < fabs(a.y))
		{
		std::swap(a.y, a.z);
			a.z = -a.z;
		}
		else
		{
			a = -a;
		}

		//今度こそ法線を求める。
		normal = a.Cross(b);

	}
	//外積と内積の結果を元にクォータニオンを求める。
	auto q = XMQuaternionRotationAxis(XMVectorSet(normal.x, normal.y, normal.z, 1.0f), angle);
	//正規化して返す。
	return XMQuaternionNormalize(q);
}
KuroEngine::Vec2<float> KuroEngine::Math::RotateVec2(const Vec2<float>& Vec, const float& Radian)
{
	Vec2<float> result;
	result.x = Vec.x * cos(Radian) - Vec.y * sin(Radian);
	result.y = Vec.y * cos(Radian) + Vec.x * sin(Radian);
	return result;
}

KuroEngine::Vec2<float> KuroEngine::Math::GetCenterVec2(Vec2<float> From, Vec2<float> To)
{
	Vec2<float> result = To - From;
	result = result * 0.5f;
	result = From + result;
	return result;
}

KuroEngine::Vec3<float> KuroEngine::Math::GetCenterVec3(Vec3<float> From, Vec3<float> To)
{
	Vec3<float> result = To - From;
	result = result * 0.5f;
	result = From + result;
	return result;
}

float KuroEngine::Math::GetLineSlope(Vec2<float> From, Vec2<float> To)
{
	return (To.y - From.y) / (To.x - From.x);
}

KuroEngine::Angle KuroEngine::Math::GetAngle(Vec2<float> Vec)
{
	return Angle((float)atan2(Vec.y, Vec.x));
}

KuroEngine::Angle KuroEngine::Math::GetAngleAbs(Vec2<float> From, Vec2<float> To)
{
	return acos(From.Dot(To));
}
KuroEngine::Vec3<float> KuroEngine::Math::TransformVec3(const Vec3<float>& Value, const Quaternion& Quaternion)
{
	auto val = XMVectorSet(Value.x, Value.y, Value.z, 1.0f);
	val = XMVector3Rotate(val, Quaternion);
	return Vec3<float>(val.m128_f32[0], val.m128_f32[1], val.m128_f32[2]);
}
KuroEngine::Vec3<float> KuroEngine::Math::TransformVec3(const Vec3<float>& Value, const Matrix& Mat)
{
	XMVECTOR valVec = XMVectorSet(Value.x, Value.y, Value.z, 1.0f);
	valVec = XMVector4Transform(valVec, Mat);
	return Vec3<float>(valVec.m128_f32[0], valVec.m128_f32[1], valVec.m128_f32[2]);
}

KuroEngine::Vec3<float> KuroEngine::Math::TransformVec3(const Vec3<float>& Value, const Vec3<float>& Axis, const Angle& Angle)
{
	//回転軸
	Vec3<float>axis = Axis;
	if (1.0f < axis.Length())axis.Normalize();
	XMVECTOR axisVec = XMVectorSet(axis.x, axis.y, axis.z, 1.0f);

	//回転行列生成
	auto rot = XMMatrixRotationQuaternion(XMQuaternionRotationAxis(axisVec, Angle));

	//回転させる値
	XMVECTOR valueVec = XMVectorSet(Value.x, Value.y, Value.z, 1.0f);
	valueVec = XMVector4Transform(valueVec, rot);

	return Vec3<float>(valueVec.m128_f32[0], valueVec.m128_f32[1], valueVec.m128_f32[2]);
}

float KuroEngine::Math::GetRateInRange(float arg_min, float arg_max, float arg_val)
{
	float result = (arg_val - arg_min) / (arg_max - arg_min);
	result = std::clamp(result, 0.0f, 1.0f);
	return result;
}

float KuroEngine::Math::GetRateInRange(int arg_min, int arg_max, int arg_val)
{
	return GetRateInRange(static_cast<float>(arg_min), static_cast<float>(arg_max), static_cast<float>(arg_val));
}

#include<array>
#include"FrameWork/ImguiApp.h"
#include<magic_enum.h>
void KuroEngine::EasingParameter::ImguiDebug(const std::string& Tag)
{
	//static const std::array<std::string, EASE_CHANGE_TYPE_NUM> CHANGE_TYPE_STR = { "In","Out","InOut" };

	std::string currentChangeType = std::string(magic_enum::enum_name(m_changeType));
	if (ImGui::BeginCombo((Tag + " - EaseChangeType").c_str(), currentChangeType.c_str()))
	{
		for (int n = 0; n < EASE_CHANGE_TYPE_NUM; ++n)
		{
			bool isSelected = (m_changeType == n);
			auto newChangeType = (EASE_CHANGE_TYPE)n;
			//if (ImGui::Selectable(CHANGE_TYPE_STR[n].c_str(), isSelected))
			if (ImGui::Selectable(std::string(magic_enum::enum_name(newChangeType)).c_str(), isSelected))
			{
				m_changeType = newChangeType;
			}
			if (isSelected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}

		ImGui::EndCombo();
	}

	//static const std::array<std::string, EASING_TYPE_NUM> CHANGE_TYPE_STR = { "In","Out","InOut" };
	std::string currentEaseType = std::string(magic_enum::enum_name(m_easeType));
	if (ImGui::BeginCombo((Tag + " - EaseType").c_str(), currentEaseType.c_str()))
	{
		for (int n = 0; n < EASING_TYPE_NUM; ++n)
		{
			bool isSelected = (m_easeType == n);
			auto newChangeType = (EASING_TYPE)n;
			//if (ImGui::Selectable(CHANGE_TYPE_STR[n].c_str(), isSelected))
			if (ImGui::Selectable(std::string(magic_enum::enum_name(newChangeType)).c_str(), isSelected))
			{
				m_easeType = newChangeType;
			}
			if (isSelected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}

		ImGui::EndCombo();
	}

}

#pragma endregion

