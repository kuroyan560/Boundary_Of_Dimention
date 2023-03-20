#include "Object.h"
#include"FrameWork/Importer.h"
#include"Render/RenderObject/ModelInfo/ModelAnimator.h"
#include"ForUser/Object/Model.h"

void KuroEngine::ModelObject::AttachModel(const std::shared_ptr<Model>& Model)
{
	m_model = Model;
	
	//アニメーション情報を持つならアニメーター生成
	if (!m_model->m_skelton->animations.empty())
	{
		m_animator = std::make_shared<ModelAnimator>(m_model);
	}
}

KuroEngine::ModelObject::ModelObject(const std::string& Dir, const std::string& FileName)
{
	AttachModel(Importer::Instance()->LoadModel(Dir, FileName));
}

const std::shared_ptr<KuroEngine::ConstantBuffer>& KuroEngine::ModelObject::GetTransformBuff()
{
	if (!m_transformBuff)
	{
		m_transformBuff = D3D12App::Instance()->GenerateConstantBuffer(sizeof(Matrix), 1, nullptr, (m_model->m_header.fileName + " - ModelObject - Transform").c_str());
	}

	m_transformBuff->Mapping(&m_transform.GetMatWorld());

	return m_transformBuff;
}