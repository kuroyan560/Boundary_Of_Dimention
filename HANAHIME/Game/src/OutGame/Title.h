#pragma once
#include"../Movie/MovieCamera.h"
#include"ForUser/DrawFunc/2D/DrawFunc2D.h"

/// <summary>
/// �^�C�g����ʌ����̏���
/// </summary>
class Title
{
public:
	Title();
	void Init();
	void Update(KuroEngine::Transform *player_camera);
	void Draw(KuroEngine::Camera &arg_cam, KuroEngine::LightManager &arg_ligMgr);

	bool IsFinish()
	{
		return m_isFinishFlag;
	}
	std::weak_ptr<KuroEngine::Camera>GetCamera()
	{
		return m_camera.GetCamera();
	}
private:
	//�J�����̈ړ����
	std::vector<MovieCameraData>titleCameraMoveDataArray;
	//�Q�[���J�n�t���O
	bool m_startGameFlag;
	//OP�t���O
	bool m_startOPFlag;
	bool m_generateCameraMoveDataFlag;

	//�I���t���O
	bool m_isFinishFlag;

	MovieCamera m_camera;

	//�^�C�g�����S
	KuroEngine::Vec2<float> m_titlePos, m_titleLogoSize;
	std::shared_ptr<KuroEngine::TextureBuffer> m_blackTexBuff;
	KuroEngine::Timer m_alphaRate;


};

