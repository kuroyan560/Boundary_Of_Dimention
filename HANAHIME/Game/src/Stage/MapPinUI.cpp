#include "MapPinUI.h"
#include"KuroEngine.h"
#include"Render/RenderObject/Camera.h"
#include"FrameWork/WinApp.h"
#include"DirectX12/D3D12App.h"
#include"ForUser/DrawFunc/2D/DrawFunc2D.h"

MapPinUI::MapPinUI()
{
	std::string texDirPath = "resource/user/tex/in_game/map_pin/";
	m_smallSquare = std::make_shared<Content>(texDirPath + "small_square.png", &m_canvasTransform);
	m_middleSquare = std::make_shared<Content>(texDirPath + "middle_square.png", &m_canvasTransform);
	m_largeSquare = std::make_shared<Content>(texDirPath + "large_square.png", &m_canvasTransform);

	//��ʓ��ɖڕW�n�_���f���Ă���Ƃ���UI
	m_mapPinUI[PIN_MODE_IN_SCREEN].emplace_back(m_largeSquare);
	m_mapPinUI[PIN_MODE_IN_SCREEN].emplace_back(m_middleSquare);
	m_mapPinUI[PIN_MODE_IN_SCREEN].emplace_back(m_smallSquare);

	//��ʓ��ɖڕW�n�_���f���Ă��Ȃ��Ƃ���UI
	m_mapPinUI[PIN_MODE_OUT_SCREEN].emplace_back(m_smallSquare);
}

void MapPinUI::Draw(KuroEngine::Camera& arg_cam, KuroEngine::Vec3<float> arg_destinationPos)
{
	using namespace KuroEngine;

	//��ʃT�C�Y�擾
	const auto winSize = WinApp::Instance()->GetExpandWinSize();
	const auto winCenter = winSize * 0.5f;

	//�ړI�n�̂RD���W���QD�ɕϊ�
	float camDist = 0.0f;
	auto destPos2D = ConvertWorldToScreen(arg_destinationPos, arg_cam.GetViewMat(), arg_cam.GetProjectionMat(), winSize, &camDist);

	//�J���������Ό���
	if (camDist < 0.0f)
	{
		//Y�̂ݕ␳
		if (destPos2D.y < winCenter.y)destPos2D.y = 0.0f;
		else destPos2D.y = winSize.y;
	}

	//��ʊO���i���m�ɂ̓s��UI����ʓ��ɓ��邩���l�����Ă���j
	float pinSizeHalf = m_pinSize * 0.5f;
	bool isOutOfScreen = destPos2D.x < pinSizeHalf || winSize.x - pinSizeHalf < destPos2D.x || destPos2D.y < pinSizeHalf || winSize.y - pinSizeHalf < destPos2D.y;

	//�N�����v�ɗp����I�t�Z�b�g����
	float clampOffset = isOutOfScreen ? m_arrowClampOffset : pinSizeHalf;
	//UI�ɍ��킹�č��W���N�����v
	destPos2D.x = std::clamp(destPos2D.x, clampOffset, winSize.x - clampOffset);
	destPos2D.y = std::clamp(destPos2D.y, clampOffset, winSize.y - clampOffset);

	//UI�S�̂̒��S���W��ݒ�
	m_canvasTransform.SetPos(destPos2D);

	//�\������UI�̌���
	PIN_MODE mode = isOutOfScreen ? PIN_MODE_OUT_SCREEN : PIN_MODE_IN_SCREEN;

	//UI�`��
	for (auto& uiPtr : m_mapPinUI[mode])
	{
		auto& ui = uiPtr.lock();
		auto& transform = uiPtr.lock()->m_transform;
		DrawFunc2D::DrawRotaGraph2D(transform.GetPosWorld(), transform.GetScaleWorld(), 0.0f, ui->m_tex, ui->m_alpha);
	}
}

MapPinUI::Content::Content(std::string arg_texPath, KuroEngine::Transform2D* arg_parent)
{
	m_tex = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer(arg_texPath);
	m_transform.SetParent(arg_parent);
}
