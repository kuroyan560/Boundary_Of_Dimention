#include "Collision.h"
#include"KuroEngineDevice.h"
#include"Render/RenderObject/Camera.h"
#include<map>
#include"ForUser/DrawFunc/3D/DrawFunc3D.h"
#include"ForUser/DrawFunc/BillBoard/DrawFuncBillBoard.h"

std::shared_ptr<KuroEngine::GraphicsPipeline> KuroEngine::CollisionPrimitive::GetPrimitivePipeline()
{
	static std::shared_ptr<GraphicsPipeline>PIPELINE;

	if (!PIPELINE)
	{
		//パイプライン設定
		static PipelineInitializeOption PIPELINE_OPTION(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		PIPELINE_OPTION.m_fillMode = D3D12_FILL_MODE_WIREFRAME;
		PIPELINE_OPTION.m_calling = D3D12_CULL_MODE_NONE;

		//シェーダー情報
		static Shaders SHADERS;
		SHADERS.m_vs = D3D12App::Instance()->CompileShader("resource/engine/CollisionPrimitive/Primitive.hlsl", "VSmain", "vs_6_4");
		SHADERS.m_ps = D3D12App::Instance()->CompileShader("resource/engine/CollisionPrimitive/Primitive.hlsl", "PSmain", "ps_6_4");

		//インプットレイアウト
		static std::vector<InputLayoutParam>INPUT_LAYOUT =
		{
			InputLayoutParam("POSITION",DXGI_FORMAT_R32G32B32_FLOAT),
		};

		//ルートパラメータ
		static std::vector<RootParam>ROOT_PARAMETER =
		{
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"カメラ情報バッファ"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"ワールド行列と衝突判定"),
		};

		//レンダーターゲット描画先情報
		std::vector<RenderTargetInfo>RENDER_TARGET_INFO = { RenderTargetInfo(D3D12App::Instance()->GetBackBuffFormat(), AlphaBlendMode_Trans) };

		PIPELINE = D3D12App::Instance()->GenerateGraphicsPipeline(PIPELINE_OPTION, SHADERS, INPUT_LAYOUT, ROOT_PARAMETER, RENDER_TARGET_INFO, { WrappedSampler(false, false) });
	}

	return PIPELINE;
}

void KuroEngine::CollisionPoint::DebugDraw(const bool& arg_hit, Camera& arg_cam, const Matrix& arg_parentMat)
{
	KuroEngine::DrawFuncBillBoard::Box(arg_cam, GetWorldPos(arg_parentMat), {0.4f,0.4f }, arg_hit ? Color(1.0f, 0.0f, 0.0f, 1.0f) : Color());
}

bool KuroEngine::CollisionPoint::HitCheck(const Matrix& arg_myMat, const Matrix& arg_otherMat, CollisionFloorMesh* arg_other, CollisionResultInfo* arg_info)
{
	for (auto& tri : arg_other->GetTriangles())
	{
		auto normal = tri.GetNormal();

		//法線が下方向なら反転
		if (normal.y < 0)normal.y *= -1;

		//ポイントのワールド座標
		auto pt = this->GetWorldPos(arg_myMat);

		//三角形の頂点A
		auto triPtA = tri.m_p0;

		//床の高さを求める
		float floorY = triPtA.y - 1.0f / normal.y * (normal.x * (pt.x - triPtA.x) + normal.z * (pt.z - triPtA.z));

		//床より上なのでめり込んでいない
		if (floorY < pt.y)continue;

		if (arg_info)
		{
			arg_info->m_inter = { pt.x,floorY,pt.z };
			arg_info->m_hitOtherPrimitive = this;
		}
		return true;
	}
	return false;
}

void KuroEngine::CollisionLine::DebugDraw(const bool& Hit, Camera& Cam, const Matrix& arg_parentMat)
{
	auto startPos = GetStartWorldPos(arg_parentMat);
	auto endPos = GetEndWorldPos(arg_parentMat);
	KuroEngine::DrawFunc3D::DrawLine(Cam, startPos, endPos, Hit ? Color(1, 0, 0, 1) : Color(), 0.01f);
}

void KuroEngine::CollisionSphere::DebugDraw(const bool& Hit,Camera& Cam, const Matrix& arg_parentMat)
{
	static std::shared_ptr<VertexBuffer>vertBuff;
	static std::shared_ptr<IndexBuffer>idxBuff;

	//頂点バッファとインデックスバッファはクラスで共通のものを使い回す
	if (!vertBuff)
	{
		static const int uMax = 24;
		static const int vMax = 12;
		static const int vertNum = uMax * (vMax + 1);
		static const int idxNum = 2 * vMax * (uMax + 1);

		std::vector<Vec3<float>>vertices(vertNum);
		for (int v = 0; v <= vMax; ++v)
		{
			for (int u = 0; u < uMax; ++u)
			{
				const auto theta = Angle::ConvertToRadian(180.0f * v / vMax);
				const auto phi = Angle::ConvertToRadian(360.0f * u / uMax);
				float fx = static_cast<float>(sin(theta) * cos(phi));
				float fy = static_cast<float>(cos(theta));
				float fz = static_cast<float>(sin(theta) * sin(phi));
				vertices[uMax * v + u] = Vec3<float>(fx, fy, fz);
			}
		}
		vertBuff = D3D12App::Instance()->GenerateVertexBuffer(sizeof(Vec3<float>), vertNum, vertices.data(), "CollisionSphere - VertexBuffer");

		int i = 0;
		std::vector<unsigned int>indices(idxNum);
		for (int v = 0; v < vMax; ++v)
		{
			for (int u = 0; u <= uMax; ++u)
			{
				if (u == uMax)
				{
					indices[i++] = v * uMax;
					indices[i++] = (v + 1) * uMax;
				}
				else
				{
					indices[i++] = (v * uMax) + u;
					indices[i++] = indices[i - 1] + uMax;
				}
			}
		}
		idxBuff = D3D12App::Instance()->GenerateIndexBuffer(idxNum, indices.data(), "CollisionSphere - IndexBuffer");
	}


	if (!m_constBuff)
	{
		m_constBuff = D3D12App::Instance()->GenerateConstantBuffer(sizeof(ConstData), 1, nullptr, "Collision_Sphere - ConstantBuffer");
	}

	ConstData constData;
	constData.m_parent = XMMatrixScaling(m_hitBoxRadius, m_hitBoxRadius, m_hitBoxRadius) * XMMatrixTranslation(m_offset.x, m_offset.y, m_offset.z) * arg_parentMat;
	constData.m_hit = Hit;
	m_constBuff->Mapping(&constData);

	KuroEngine::KuroEngineDevice::Instance()->Graphics().SetGraphicsPipeline(KuroEngine::CollisionPrimitive::GetPrimitivePipeline());

	KuroEngine::KuroEngineDevice::Instance()->Graphics().ObjectRender(
		vertBuff,
		idxBuff,
		{
			{Cam.GetBuff(),CBV},
			{m_constBuff,CBV }
		}, 
		0,
		true);
}

bool KuroEngine::CollisionSphere::HitCheck(const Matrix& arg_myMat, const Matrix& arg_otherMat, CollisionSphere* arg_other, CollisionResultInfo* arg_info)
{
	//２つの球のワールド中心座標を求める
	const auto centerA = this->GetCenter(arg_myMat);
	const auto centerB = arg_other->GetCenter(arg_otherMat);

	// 中心点の距離の２乗 <= 半径の和の２乗なら交差
	const float distSq = centerA.DistanceSq(centerB);
	const float radius2 = pow(this->m_hitBoxRadius + arg_other->m_hitBoxRadius, 2.0f);

	if (distSq <= radius2)
	{
		if (arg_info)
		{
			//２つの中心間の中心点
			arg_info->m_inter = centerA.GetCenter(centerB);
			arg_info->m_hitOtherPrimitive = this;
		}
		return true;
	}
	return false;
}

bool KuroEngine::CollisionSphere::HitCheck(const Matrix& arg_myMat, const Matrix& arg_otherMat, CollisionPlane* arg_other, CollisionResultInfo* arg_info)
{
	//球のワールド中心座標を求める
	const auto center = this->GetCenter(arg_myMat);

	// 座標系の原点から球の中心座標への距離から
	// 平面の原点距離を減算することで、平面と球の中心との距離が出る
	float dist = XMVector3Dot(center, arg_other->m_normal).m128_f32[0] - arg_other->m_distance;
	// 距離の絶対値が半径より大きければ当たっていない
	if (fabsf(dist) > this->m_hitBoxRadius)	return false;

	// 擬似交点を計算
	if (arg_info)
	{
		// 平面上の再接近点を、疑似交点とする
		arg_info->m_inter = arg_other->m_normal * -dist + center;
		arg_info->m_hitOtherPrimitive = this;
	}
	return true;
}

bool KuroEngine::CollisionSphere::HitCheck(const Matrix& arg_myMat, const Matrix& arg_otherMat, CollisionLine* arg_other, CollisionResultInfo* arg_info)
{
	auto lineStart = arg_other->GetStartWorldPos(arg_otherMat);
	XMVECTOR m = lineStart - this->GetCenter(arg_myMat);
	float b = XMVector3Dot(m, arg_other->m_dir).m128_f32[0];
	float c = XMVector3Dot(m, m).m128_f32[0] - static_cast<float>(pow(this->m_hitBoxRadius, 2));
	// layの始点がsphereの外側にあり(c > 0)、layがsphereから離れていく方向を
	// 差している場合(b > 0)、当たらない
	if (c > 0.0f && b > 0.0f) {
		return false;
	}

	float discr = b * b - c;
	// 負の判別式はレイが球を外れていることに一致
	if (discr < 0.0f) {
		return false;
	}

	// 交差する最小の値tを計算
	float t = -b - sqrtf(discr);
	// tが負である場合、レイは球の内側から開始しているのでtをゼロにクランプ
	if (t < 0) t = 0.0f;

	//レイの長さより遠ければ外れている
	if (arg_other->m_len < t)return false;

	if (arg_info) 
	{
		arg_info->m_inter = lineStart + arg_other->m_dir * t;
		arg_info->m_hitOtherPrimitive = this;
	}

	return true;
}

bool KuroEngine::CollisionSphere::HitCheck(const Matrix& arg_myMat, const Matrix& arg_otherMat, CollisionAABB* arg_other, CollisionResultInfo* arg_info)
{
	//球の中心座標とAABBとの最短距離を求める
	const auto spCenter = KuroEngine::Math::TransformVec3(this->m_offset, arg_myMat);

	//AABBの各軸の最小値最大値にワールド変換
	const auto& ptVal = arg_other->GetPtValue();

	Vec3<float>ptMin(ptVal.x.m_min, ptVal.y.m_min, ptVal.z.m_min);
	ptMin = KuroEngine::Math::TransformVec3(ptMin, arg_otherMat);
	Vec3<float>ptMax(ptVal.x.m_max, ptVal.y.m_max, ptVal.z.m_max);
	ptMax = KuroEngine::Math::TransformVec3(ptMax, arg_otherMat);

	//回転によって最小・最大が入れ替わっていることがあるので調整
	if (ptMax.x < ptMin.x)std::swap(ptMax.x, ptMin.x);
	if (ptMax.y < ptMin.y)std::swap(ptMax.y, ptMin.y);
	if (ptMax.z < ptMin.z)std::swap(ptMax.z, ptMin.z);

	float distSq = 0.0f;
	if (spCenter.x < ptMin.x)distSq += static_cast<float>(pow((spCenter.x - ptMin.x), 2));
	if (ptMax.x < spCenter.x)distSq += static_cast<float>(pow((spCenter.x - ptMax.x), 2));

	if (spCenter.y < ptMin.y)distSq += static_cast<float>(pow((spCenter.y - ptMin.y), 2));
	if (ptMax.y < spCenter.y)distSq += static_cast<float>(pow((spCenter.y - ptMax.y), 2));

	if (spCenter.z < ptMin.z)distSq += static_cast<float>(pow((spCenter.z - ptMin.z), 2));
	if (ptMax.z < spCenter.z)distSq += static_cast<float>(pow((spCenter.z - ptMax.z), 2));

	if (distSq <= pow(this->m_hitBoxRadius, 2))
	{
		if (arg_info)
		{
			//球の中心とAABBの中心間の中心点
			arg_info->m_inter = spCenter.GetCenter(ptMin.GetCenter(ptMax));
			arg_info->m_hitOtherPrimitive = this;
		}
		return true;
	}
	return false;
}

KuroEngine::Vec3<float> KuroEngine::CollisionSphere::ClosestPtPoint2Triangle(const Vec3<float>& Pt, const CollisionTriangle& Tri, const Matrix& MeshWorld)
{
	//三角メッシュの座標をワールド変換
	Vec3<float>p0 = KuroEngine::Math::TransformVec3(Tri.m_p0, MeshWorld);
	Vec3<float>p1 = KuroEngine::Math::TransformVec3(Tri.m_p1, MeshWorld);
	Vec3<float>p2 = KuroEngine::Math::TransformVec3(Tri.m_p2, MeshWorld);

	//Ptがp0の外側の頂点領域の中にあるかチェック
	Vec3<float>p0_p1 = p1 - p0;
	Vec3<float>p0_p2 = p2 - p0;
	Vec3<float>p0_pt = Pt - p0;
	float d1 = p0_p1.Dot(p0_pt);
	float d2 = p0_p2.Dot(p0_pt);
	if (d1 <= 0.0f && d2 <= 0.0f)return p0;

	//Ptがp1の外側の頂点領域の中にあるかチェック
	Vec3<float>p1_pt = Pt - p1;
	float d3 = p0_p1.Dot(p1_pt);
	float d4 = p0_p2.Dot(p1_pt);
	if (0.0f <= d3 && d4 <= d3)return p1;

	//Ptがp0_p1の辺領域の中にあるかチェックし、あればp0_p1上に対する射影を返す
	float vc = d1 * d4 - d3 * d2;
	if (vc <= 0.0f && 0.0f <= d1 && d3 <= 0.0f)
	{
		float v = d1 / (d1 - d3);
		return p0 + p0_p1 * v;
	}

	//Ptがp2の外側の頂点領域の中にあるかチェック
	Vec3<float>p2_pt = Pt - p2;
	float d5 = p0_p1.Dot(p2_pt);
	float d6 = p0_p2.Dot(p2_pt);
	if (0.0f <= d6 && d5 <= d6)return p2;

	//Ptがp0_p2の辺領域の中にあるかチェックし、あればPtのp0_p2上に対する射影を返す
	float vb = d5 * d2 - d1 * d6;
	if (vb <= 0.0f && 0.0f <= d2 && d6 <= 0.0f)
	{
		float w = d2 / (d2 - d6);
		return p0 + p0_p2 * w;
	}

	// Ptがp1_p2の辺領域の中にあるかどうかチェックし、あればPtのp1_p2上に対する射影を返す
	float va = d3 * d6 - d5 * d4;
	if (va <= 0.0f && 0.0f <= (d4 - d3) && 0.0f <= (d5 - d6))
	{
		float w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
		return p1 + (p2 - p1) * w;
	}

	// Ptは面領域の中にある。closestを重心座標を用いて計算する
	float denom = 1.0f / (va + vb + vc);
	float v = vb * denom;
	float w = vc * denom;
	return p0 + p0_p1 * v + p0_p2 * w;
}

bool KuroEngine::CollisionSphere::HitCheck(const Matrix& arg_myMat, const Matrix& arg_otherMat, CollisionMesh* arg_other, CollisionResultInfo* arg_info)
{
	const auto spCenter = KuroEngine::Math::TransformVec3(this->m_offset, arg_myMat);

	for (auto& t : arg_other->GetTriangles())
	{
		// 球の中心に対する最近接点である三角形上にある点pを見つける
		Vec3<float>closest = ClosestPtPoint2Triangle(spCenter, t, arg_otherMat);
		Vec3<float>v = closest - spCenter;
		float distSq = v.Dot(v);

		if (pow(this->m_hitBoxRadius, 2.0f) < distSq)continue;

		if (arg_info)
		{
			arg_info->m_inter = closest;
			arg_info->m_hitOtherPrimitive = this;
		}
		return true;
	}

	return false;
}

void KuroEngine::CollisionAABB::DebugDraw(const bool& arg_hit, Camera& arg_cam, const Matrix& arg_parentMat)
{
	static std::shared_ptr<IndexBuffer>INDEX_BUFF;
	if (!INDEX_BUFF)
	{
		static const int IDX_NUM = 15;
		std::array<unsigned int, IDX_NUM>indices =
		{
			LU_NZ,RU_NZ,LB_NZ,
			RB_NZ,RB_FZ,RU_NZ,
			RU_FZ,LU_NZ,	LU_FZ,
			LB_NZ,LB_FZ,	RB_FZ,
			LU_FZ,RU_FZ
		};
		INDEX_BUFF = D3D12App::Instance()->GenerateIndexBuffer(IDX_NUM, indices.data(), "CollisionAABB - IndexBuffer");
	}

	//描画に必要なバッファが未生成
	if (!m_constBuff)
	{
		m_constBuff = D3D12App::Instance()->GenerateConstantBuffer(sizeof(ConstData), 1, nullptr, "Collision_AABB - ConstantBuffer");
	}

	ConstData constData;
	constData.m_parent = arg_parentMat;
	constData.m_hit = arg_hit;
	m_constBuff->Mapping(&constData);

	KuroEngine::KuroEngineDevice::Instance()->Graphics().SetGraphicsPipeline(KuroEngine::CollisionPrimitive::GetPrimitivePipeline());

	KuroEngine::KuroEngineDevice::Instance()->Graphics().ObjectRender(
		m_vertBuff,
		INDEX_BUFF,
		{
			{arg_cam.GetBuff(),CBV},
			{m_constBuff,CBV },
		}, 
		0, 
		true);
}

bool KuroEngine::CollisionAABB::HitCheck(const Matrix& arg_myMat, const Matrix& arg_otherMat, CollisionLine* arg_other, CollisionResultInfo* arg_info)
{
	Matrix invMat = XMMatrixInverse(nullptr, arg_myMat);

	auto p_l = KuroEngine::Math::TransformVec3(arg_other->m_start, invMat);
	invMat.r[3].m128_f32[0] = 0.0f;
	invMat.r[3].m128_f32[1] = 0.0f;
	invMat.r[3].m128_f32[2] = 0.0f;
	auto dir_l = KuroEngine::Math::TransformVec3(arg_other->m_dir, invMat);

	//交差判定
	float p[3] = { p_l.x,p_l.y,p_l.z };
	float d[3] = { dir_l.x,dir_l.y,dir_l.z };
	float min[3] = { m_pValues.x.m_min,m_pValues.y.m_min ,m_pValues.z.m_min };
	float max[3] = { m_pValues.x.m_max,m_pValues.y.m_max ,m_pValues.z.m_max };

	float t = -arg_other->m_len;
	float t_max = arg_other->m_len;

	for (int i = 0; i < 3; ++i)
	{
		if (FLT_EPSILON < abs(d[i]))
		{
			if (p[i] < min[i] || max[i] < p[i])
			{
				//交差していない
				return false;
			}
			else
			{
				float odd = 1.0f / d[i];
				float t1 = (min[i] - p[i]) * odd;
				float t2 = (max[i] - p[i]) * odd;
				if (t2 < t1)
				{
					float tmp = t1;
					t1 = t2;
					t2 = tmp;
				}
				if (t < t1)t = t1;
				if (t2 < t_max)t_max = t2;

				if (t_max <= t)return false;
			}
		}
	}

	//交差している点
	Vec3<float>inter = arg_other->m_start + arg_other->m_dir * t;

	//距離が足りていない
	//if (arg_other->m_len < t)return false;

	if (arg_info)
	{
		arg_info->m_inter = inter;
		arg_info->m_hitOtherPrimitive = this;
	}

	return true;
}

bool KuroEngine::CollisionAABB::HitCheck(const Matrix& arg_myMat, const Matrix& arg_otherMat, CollisionAABB* arg_other, CollisionResultInfo* arg_info)
{
	Vec3<float>thisMinPt = this->GetTransformedMin(arg_myMat);
	Vec3<float>thisMaxPt = this->GetTransformedMax(arg_myMat);
	Vec3<float>otherMinPt = arg_other->GetTransformedMin(arg_otherMat);
	Vec3<float>otherMaxPt = arg_other->GetTransformedMax(arg_otherMat);

	float thisPt[3][2] =
	{
		{thisMinPt.x,thisMaxPt.x},
		{thisMinPt.y,thisMaxPt.y},
		{thisMinPt.z,thisMaxPt.z},
	};

	float otherPt[3][2] =
	{
		{otherMinPt.x,otherMaxPt.x},
		{otherMinPt.y,otherMaxPt.y},
		{otherMinPt.z,otherMaxPt.z},
	};

	static const int MIN = 0;
	static const int MAX = 1;

	for (int i = 0; i < 3; ++i)
	{
		if (!(otherPt[i][MIN] < thisPt[i][MAX]))return false;
		if (!(thisPt[i][MIN] < otherPt[i][MAX]))return false;
	}

	if (arg_info)
	{
		//双方の中心を結んだ線の中間地点
		const auto myCenter = thisMinPt.GetCenter(thisMaxPt);
		const auto otherCenter = otherMinPt.GetCenter(otherMaxPt);
		arg_info->m_inter = myCenter.GetCenter(otherCenter);
		arg_info->m_hitOtherPrimitive = this;
	}

	return true;
}

void KuroEngine::CollisionAABB::StructBox(const Vec3<ValueMinMax>& PValues)
{
	m_pValues = PValues;
	//大小関係がおかしいものがないか確認
	assert(m_pValues.x && m_pValues.y && m_pValues.z);

	if (!m_vertBuff)
	{
		m_vertBuff = D3D12App::Instance()->GenerateVertexBuffer(sizeof(Vec3<float>), VERT_NUM, nullptr, "CollisionAABB - VertexBuffer");
	}

	std::array<Vec3<float>, VERT_NUM>vertices;
	vertices[LU_NZ] = { m_pValues.x.m_min,m_pValues.y.m_max,m_pValues.z.m_min };
	vertices[RU_NZ] = { m_pValues.x.m_max,m_pValues.y.m_max,m_pValues.z.m_min };
	vertices[RB_NZ] = { m_pValues.x.m_max,m_pValues.y.m_min,m_pValues.z.m_min };
	vertices[LB_NZ] = { m_pValues.x.m_min,m_pValues.y.m_min,m_pValues.z.m_min };
	vertices[LU_FZ] = { m_pValues.x.m_min,m_pValues.y.m_max,m_pValues.z.m_max };
	vertices[RU_FZ] = { m_pValues.x.m_max,m_pValues.y.m_max,m_pValues.z.m_max };
	vertices[RB_FZ] = { m_pValues.x.m_max,m_pValues.y.m_min,m_pValues.z.m_max };
	vertices[LB_FZ] = { m_pValues.x.m_min,m_pValues.y.m_min,m_pValues.z.m_max };
	m_vertBuff->Mapping(vertices.data());
}

void KuroEngine::CollisionMesh::SetTriangles(const std::vector<CollisionTriangle>& Triangles)
{
	m_triangles = Triangles;

	std::vector<Vec3<float>>vertices;
	for (auto& t : m_triangles)
	{
		vertices.emplace_back(t.m_p0);
		vertices.emplace_back(t.m_p1);
		vertices.emplace_back(t.m_p2);
	}
	m_vertBuff = D3D12App::Instance()->GenerateVertexBuffer(sizeof(Vec3<float>), static_cast<int>(vertices.size()), vertices.data(), "CollisionMesh - VertexBuffer");
	m_constBuff = D3D12App::Instance()->GenerateConstantBuffer(sizeof(ConstData), 1, nullptr, "Collision_Mesh - ConstantBuffer");

}

void KuroEngine::CollisionMesh::DebugDraw(const bool& arg_hit, Camera& arg_cam, const Matrix& arg_parentMat)
{
	static std::shared_ptr<GraphicsPipeline>PIPELINE;
	if (!PIPELINE)
	{
		//パイプライン設定
		static PipelineInitializeOption PIPELINE_OPTION(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		PIPELINE_OPTION.m_fillMode = D3D12_FILL_MODE_WIREFRAME;

		//シェーダー情報
		static Shaders SHADERS;
		SHADERS.m_vs = D3D12App::Instance()->CompileShader("resource/engine/CollisionPrimitive/Mesh.hlsl", "VSmain", "vs_6_4");
		SHADERS.m_ps = D3D12App::Instance()->CompileShader("resource/engine/CollisionPrimitive/Mesh.hlsl", "PSmain", "ps_6_4");

		//ルートパラメータ
		static std::vector<RootParam>ROOT_PARAMETER =
		{
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"カメラ情報バッファ"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"ワールド行列と衝突判定"),
		};

		//インプットレイアウト
		static std::vector<InputLayoutParam>INPUT_LAYOUT =
		{
			InputLayoutParam("POSITION",DXGI_FORMAT_R32G32B32_FLOAT),
		};

		//レンダーターゲット描画先情報
		std::vector<RenderTargetInfo>RENDER_TARGET_INFO = { RenderTargetInfo(D3D12App::Instance()->GetBackBuffFormat(), AlphaBlendMode_Trans) };

		PIPELINE = D3D12App::Instance()->GenerateGraphicsPipeline(PIPELINE_OPTION, SHADERS, INPUT_LAYOUT, ROOT_PARAMETER, RENDER_TARGET_INFO, { WrappedSampler(false, false) });
	}

	ConstData constData;
	constData.m_parent = XMMatrixMultiply(XMMatrixScaling(1.1f, 1.1f, 1.1f), arg_parentMat);
	constData.m_hit = arg_hit;
	m_constBuff->Mapping(&constData);

	KuroEngine::KuroEngineDevice::Instance()->Graphics().SetGraphicsPipeline(PIPELINE);

	KuroEngine::KuroEngineDevice::Instance()->Graphics().ObjectRender(
		m_vertBuff,
		{
			{arg_cam.GetBuff(),CBV},
			{m_constBuff,CBV }
		}, 
		0, 
		true);
}