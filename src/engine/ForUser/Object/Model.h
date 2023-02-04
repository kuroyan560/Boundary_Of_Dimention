#pragma once
#include<string>
#include<vector>
#include"Common/ImportHeader.h"
#include"Render/RenderObject/ModelInfo/Skeleton.h"
#include"Render/RenderObject/ModelInfo/ModelMesh.h"
#include"ForUser/Collision/Collision.h"

namespace KuroEngine
{
	class Model
	{
	public:
		//ヘッダ（モデル情報）
		ImportHeader m_header;
		//メッシュ
		std::vector<ModelMesh>m_meshes;
		//スケルトン（ボーン構成）
		std::shared_ptr<Skeleton> m_skelton;

		Model(const std::string& Dir, const std::string& FileName) :m_header(Dir, FileName) {}

		//全メッシュをスムージング
		void AllMeshSmoothing()
		{
			for (auto& m : m_meshes)
			{
				m.Smoothing();
			}
		}

		//全メッシュにおける各軸の頂点座標の最小値最大値を取得
		Vec3<ValueMinMax>GetAllMeshPosMinMax();

		void AllMeshBuildTangentAndBiNormal()
		{
			for (auto& m : m_meshes)
			{
				m.BuildTangentAndBiNormal();
			}
		}
	};
}