#include "RandBox2D.h"
#include"ForUser/Object/Sprite.h"
#include"DirectX12/D3D12App.h"
#include"Common/PerlinNoise.h"

KuroEngine::Transform2D& KuroEngine::RandBox2D::Transform()
{
	return m_sprite->m_transform;
}

KuroEngine::RandBox2D::RandBox2D()
{
	m_sprite = std::make_shared<Sprite>(nullptr, "RandBox2D");
}

void KuroEngine::RandBox2D::Init()
{
	m_t = 0.0f;
}

void KuroEngine::RandBox2D::Update()
{
	m_t += 1.0f / static_cast<float>(m_interval);

	//オフセット
	Vec2<float>upL =
	{
		PerlinNoise::GetRand(m_t,0.0f,0) * m_maxVal.x,
		PerlinNoise::GetRand(0.0f,m_t,0) * m_maxVal.y
	};
	m_sprite->m_mesh.SetOffset(KuroEngine::SpriteMesh::LT, upL);


	Vec2<float>upR =
	{
		PerlinNoise::GetRand(m_t,0.0f,1) * m_maxVal.x,
		PerlinNoise::GetRand(0.0f,m_t,1) * m_maxVal.y
	};
	m_sprite->m_mesh.SetOffset(KuroEngine::SpriteMesh::RT, upR);

	Vec2<float>bottomL =
	{
		PerlinNoise::GetRand(m_t,0.0f,2) * m_maxVal.x,
		PerlinNoise::GetRand(0.0f,m_t,2) * m_maxVal.y
	};
	m_sprite->m_mesh.SetOffset(KuroEngine::SpriteMesh::LB, bottomL);

	Vec2<float>bottomR =
	{
		PerlinNoise::GetRand(m_t,0.0f,3) * m_maxVal.x,
		PerlinNoise::GetRand(0.0f,m_t,3) * m_maxVal.y
	};
	m_sprite->m_mesh.SetOffset(KuroEngine::SpriteMesh::RB, bottomR);
}

void KuroEngine::RandBox2D::Draw()
{
	m_sprite->Draw();
}

#include"FrameWork/ImguiApp.h"
void KuroEngine::RandBox2D::ImguiDebug()
{
	ImGui::Begin("RandBox");

	auto size = m_sprite->m_mesh.GetSize();
	ImGui::DragFloat("SizeX", &size.x);
	ImGui::DragFloat("SizeY", &size.y);
	m_sprite->m_mesh.SetSize(size);

	ImGui::DragInt("Interval", &m_interval);
	if (m_interval < 1)m_interval = 1;


	ImGui::DragFloat("MaxOffsetX", &m_maxVal.x);
	ImGui::DragFloat("MaxOffsetY", &m_maxVal.y);

	ImGui::End();
}

void KuroEngine::RandBox2D::SetSize(Vec2<float> arg_size)
{
	m_sprite->m_mesh.SetSize(arg_size);
}

void KuroEngine::RandBox2D::SetAnchorPoint(Vec2<float> arg_anchor)
{
	m_sprite->m_mesh.SetAnchorPoint(arg_anchor);
}

void KuroEngine::RandBox2D::SetColor(Color arg_color)
{
	m_sprite->SetColor(arg_color);
}
