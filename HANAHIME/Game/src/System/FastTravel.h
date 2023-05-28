#pragma once
#include "../OutGame/PazzleStageSelect.h"
#include <Common/Vec.h>
#include <Common/Transform.h>
#include <array>
#include <vector>
#include <memory>
#include <Common/Singleton.h>

class GameScene;

#include <windows.h>

namespace ImageAdjust {

	template<typename T>
	class SingletonAdjust
	{
	protected:
		SingletonAdjust() {}
		virtual ~SingletonAdjust() {}
		SingletonAdjust(const SingletonAdjust& instance) {}
		SingletonAdjust& operator=(const SingletonAdjust& r) {}
	public:
		static T* Instance() {
			static T inst;
			return &inst;
		}
	};

#define API_KEY_A               0x41
#define API_KEY_D               0x44
#define API_KEY_S               0x53
#define API_KEY_W               0x57
#define API_KEY_LEFT            0x25
#define API_KEY_UP              0x26
#define API_KEY_RIGHT           0x27
#define API_KEY_DOWN            0x28
#define API_KEY_LCONTROL        0xA2


	class WinAPIInput : public SingletonAdjust<WinAPIInput> {
		friend class SingletonAdjust<WinAPIInput>;
	private:
		SHORT keys[256];
		SHORT oldkeys[256];
	public:
		void Update()
		{
			for (int i = 0; i < 256; i++) {
				oldkeys[i] = keys[i];
				keys[i] = GetAsyncKeyState(i);
			}
		}
		bool isKey(int keyIndex)
		{
			return keys[keyIndex];
		}
		bool oldkey(int keyIndex)
		{
			return oldkeys[keyIndex];
		}
		bool isKeyTrigger(int keyIndex)
		{
			return keys[keyIndex] && !oldkeys[keyIndex];
		}
	};

	template <class T>
	static void Adjust(T* value, float width) {

		WinAPIInput::Instance()->Update();

		if (WinAPIInput::Instance()->isKey(API_KEY_LCONTROL)) {
			if (WinAPIInput::Instance()->isKeyTrigger(API_KEY_W)) value->y -= width;
			if (WinAPIInput::Instance()->isKeyTrigger(API_KEY_S)) value->y += width;
			if (WinAPIInput::Instance()->isKeyTrigger(API_KEY_A)) value->x -= width;
			if (WinAPIInput::Instance()->isKeyTrigger(API_KEY_D)) value->x += width;
		}
		else {
			if (WinAPIInput::Instance()->isKey(API_KEY_W)) value->y -= width;
			if (WinAPIInput::Instance()->isKey(API_KEY_S)) value->y += width;
			if (WinAPIInput::Instance()->isKey(API_KEY_A)) value->x -= width;
			if (WinAPIInput::Instance()->isKey(API_KEY_D)) value->x += width;
		}
	}
}

//�t�@�X�g�g���x��
class FastTravel 
{
private:
	bool m_isActive = false;

	//�`�F�b�N�|�C���g�̔z��
	std::vector<std::vector<KuroEngine::Transform>>m_checkPointVector;

	//���ݒ�������z��̃X�e�[�W�ԍ�
	int m_nowStageNum;
	//���ݒ�������z��̗v�f
	int m_nowTargetCheckPoint;

	KuroEngine::Transform m_rotate;			//XZ���ʂł̃J������]�Ɏg�p����g�����X�t�H�[��
	const float ADD_XZANGLE = 0.01f;		//XZ���ʂł̉�]�̉��Z��
	KuroEngine::Vec3<float> m_cameraPos;	//�J�����̍��W

	//�t�@�X�g�g���x���p�̃J����
	std::shared_ptr<KuroEngine::Camera> m_fastTravelCamera;

	//UI�̕\���Ɏg�p����e�N�X�`��
	std::vector<std::shared_ptr<KuroEngine::TextureBuffer>> m_stageNameTexArray;
	std::shared_ptr<KuroEngine::TextureBuffer> m_underLineTex;
	std::array<std::shared_ptr<KuroEngine::TextureBuffer>, 10> m_numMainTexArray;

	//�t�@�X�g�g���x����ʂɈȍ~����܂��̃X�e�[�W�ԍ�
	int m_beforeStageNum;

	void GetTargetPosAndRotate(KuroEngine::Quaternion* arg_resultRotate, KuroEngine::Vec3<float>* arg_resultPos);

public:
	FastTravel();

	void Init(std::vector<std::vector<KuroEngine::Transform>>arg_checkPointVector, int arg_selectStageNum, int arg_selectTransIdx);
	void Update(GameScene* arg_gameScene);
	void Draw(KuroEngine::Camera& arg_cam);

	std::shared_ptr<KuroEngine::Camera> GetCamera() { return m_fastTravelCamera; }

	//���ݑI�����Ă���v�f�B
	int GetNowSelectStage() { return m_nowTargetCheckPoint; }

	int GetDisit(int arg_value, int arg_disit) {
		int mod_value;
		int result;
		mod_value = arg_value % (int)pow(10, arg_disit + 1);
		result = static_cast<int>(mod_value / pow(10, arg_disit));
		return result;
	}

	void Activate() { m_isActive = true; }
	void DisActivate() { m_isActive = false; }

	const bool& IsActive()const { return m_isActive; }
};