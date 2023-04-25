#pragma once
#include<memory>
#include"Common/Transform.h"
#include<string>
#include"Render/RenderObject/ModelInfo/ModelAnimator.h"
#include"ForUser/Object/Model.h"

namespace KuroEngine
{
	class ConstantBuffer;
	class ModelObject
	{
	private:
		std::shared_ptr<ConstantBuffer>m_transformBuff;
		void AttachModel(const std::shared_ptr<Model>& Model);

	public:
		std::shared_ptr<Model>m_model;
		std::shared_ptr<ModelAnimator>m_animator;
		Transform m_transform;

		ModelObject(const std::string& Dir, const std::string& FileName);
		ModelObject(const std::shared_ptr<Model>& Model) { AttachModel(Model); }

		const std::shared_ptr<ConstantBuffer>& GetTransformBuff();
	};
}