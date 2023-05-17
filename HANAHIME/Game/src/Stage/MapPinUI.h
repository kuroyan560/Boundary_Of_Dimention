#pragma once
#include<memory>
#include"Common/Transform2D.h"
#include"Common/Transform.h"
#include"Common/Vec.h"
#include<vector>
#include<array>

namespace KuroEngine
{
	class TextureBuffer;
	class Camera;
}

//�ړI�n�������}�b�v�s��
class MapPinUI
{
	struct Content
	{
		bool m_active = true;
		std::shared_ptr<KuroEngine::TextureBuffer>m_tex;
		KuroEngine::Transform2D m_transform;
		float m_alpha = 1.0f;
		Content(std::string arg_texPath, KuroEngine::Transform2D* arg_parent);
		Content(std::shared_ptr<KuroEngine::TextureBuffer>arg_tex, KuroEngine::Transform2D* arg_parent)
			:m_tex(arg_tex)
		{
			m_transform.SetParent(arg_parent);
		}
	};

	//�s��
	std::shared_ptr<Content>m_smallSquare;
	std::shared_ptr<Content>m_middleSquare;
	std::shared_ptr<Content>m_largeSquare;

	//�����̐���
	std::array<std::shared_ptr	<KuroEngine::TextureBuffer>, 10>m_numTex;
	static const int NUM_DIGIT_MAX = 4;
	std::array<std::shared_ptr<Content>,NUM_DIGIT_MAX>m_distanceNum;
	//���[�g��
	std::shared_ptr<Content>m_meter;

	enum PIN_MODE { PIN_MODE_IN_SCREEN, PIN_MODE_OUT_SCREEN, PIN_MODE_NUM };
	std::array<std::vector<std::weak_ptr<Content>>, PIN_MODE_NUM>m_mapPinUI;

	//�}�b�v�s���̕`��T�C�Y
	float m_pinSize = 62.0f;
	//���S�̂�`�悷�邽�߂̍��W�N�����v��
	float m_arrowClampOffset = 65.0f;
	//�����\�L�̕`��I�t�Z�b�gY
	float m_meterDrawOffsetY = 16.0f;
	//�����̐����̎��ԃI�t�Z�b�g
	float m_distStrDrawSpace = -2.0f;
	//������m�̎���
	float m_meterStrDrawSpace = 3.0f;

	//UI�S�̂̃g�����X�t�H�[��
	KuroEngine::Transform2D m_canvasTransform;

	void UpdateDistance(PIN_MODE arg_pinMode, float arg_distance);

public:
	MapPinUI();
	void Draw(KuroEngine::Camera& arg_cam, KuroEngine::Vec3<float>arg_destinationPos, KuroEngine::Vec3<float>arg_playerPos);
};

