#include "Pch.h"
#include "Frustum.h"

using namespace DirectX;


Frustum::Frustum()
	: m_screenDepth(0.0f){
	m_planes[0][0] = 0.0f;
} // Frustum

Frustum::~Frustum() {} // ~Frustum

void Frustum::Init(float screenDepth) {
	m_screenDepth = screenDepth;
	return;
} // Init


void Frustum::BuildFrustum(XMMATRIX viewMatrix, XMMATRIX projectionMatrix) {
	XMFLOAT4X4 pMatrix, matrix;
	float zMinimum, r, length;
	XMMATRIX finalMatrix;

	// 투영 행렬을 4x4 부동 소수점 형식으로 변환
	XMStoreFloat4x4(&pMatrix, projectionMatrix);

	// 원뿔대에서 최소 Z 거리를 계산
	zMinimum = -pMatrix._43 / pMatrix._33;
	r = m_screenDepth / (m_screenDepth - zMinimum);

	// 업데이트된 값을 투영 행렬에 다시 로드
	pMatrix._33 = r;
	pMatrix._43 = -r * zMinimum;
	projectionMatrix = XMLoadFloat4x4(&pMatrix);

	// 뷰 행렬과 업데이트된 투영 행렬로부터 절두체 행렬을 생성
	finalMatrix = XMMatrixMultiply(viewMatrix, projectionMatrix);

	// 최종 행렬을 4x4 부동소수점 형식으로 변환
	XMStoreFloat4x4(&matrix, finalMatrix);

	// 평면 근처를 계산
	m_planes[0][0] = matrix._14 + matrix._13;
	m_planes[0][1] = matrix._24 + matrix._23;
	m_planes[0][2] = matrix._34 + matrix._33;
	m_planes[0][3] = matrix._44 + matrix._43;

	// 근접 평면을 정규화
	length = sqrtf((m_planes[0][0] * m_planes[0][0]) + (m_planes[0][1] * m_planes[0][1]) + (m_planes[0][2] * m_planes[0][2]));
	m_planes[0][0] /= length;
	m_planes[0][1] /= length;
	m_planes[0][2] /= length;
	m_planes[0][3] /= length;

	// frustum 원거리 평면을 계산
	m_planes[1][0] = matrix._14 - matrix._13;
	m_planes[1][1] = matrix._24 - matrix._23;
	m_planes[1][2] = matrix._34 - matrix._33;
	m_planes[1][3] = matrix._44 - matrix._43;

	// frustum 원거리 평면을 정규화
	length = sqrtf((m_planes[1][0] * m_planes[1][0]) + (m_planes[1][1] * m_planes[1][1]) + (m_planes[1][2] * m_planes[1][2]));
	m_planes[1][0] /= length;
	m_planes[1][1] /= length;
	m_planes[1][2] /= length;
	m_planes[1][3] /= length;

	// frustum 왼쪽 평면을 계산
	m_planes[2][0] = matrix._14 + matrix._11;
	m_planes[2][1] = matrix._24 + matrix._21;
	m_planes[2][2] = matrix._34 + matrix._31;
	m_planes[2][3] = matrix._44 + matrix._41;

	// frustum 왼쪽 평면을 정규화
	length = sqrtf((m_planes[2][0] * m_planes[2][0]) + (m_planes[2][1] * m_planes[2][1]) + (m_planes[2][2] * m_planes[2][2]));
	m_planes[2][0] /= length;
	m_planes[2][1] /= length;
	m_planes[2][2] /= length;
	m_planes[2][3] /= length;

	//frustum 오른쪽 평면을 계산
	m_planes[3][0] = matrix._14 - matrix._11;
	m_planes[3][1] = matrix._24 - matrix._21;
	m_planes[3][2] = matrix._34 - matrix._31;
	m_planes[3][3] = matrix._44 - matrix._41;

	// frustum 오른쪽 평면을 정규화
	length = sqrtf((m_planes[3][0] * m_planes[3][0]) + (m_planes[3][1] * m_planes[3][1]) + (m_planes[3][2] * m_planes[3][2]));
	m_planes[3][0] /= length;
	m_planes[3][1] /= length;
	m_planes[3][2] /= length;
	m_planes[3][3] /= length;

	// frustum 윗쪽 평면을 계산
	m_planes[4][0] = matrix._14 - matrix._12;
	m_planes[4][1] = matrix._24 - matrix._22;
	m_planes[4][2] = matrix._34 - matrix._32;
	m_planes[4][3] = matrix._44 - matrix._42;

	// 상단 평면을 정규화.
	length = sqrtf((m_planes[4][0] * m_planes[4][0]) + (m_planes[4][1] * m_planes[4][1]) + (m_planes[4][2] * m_planes[4][2]));
	m_planes[4][0] /= length;
	m_planes[4][1] /= length;
	m_planes[4][2] /= length;
	m_planes[4][3] /= length;

	// frustum 밑면을 계산
	m_planes[5][0] = matrix._14 + matrix._12;
	m_planes[5][1] = matrix._24 + matrix._22;
	m_planes[5][2] = matrix._34 + matrix._32;
	m_planes[5][3] = matrix._44 + matrix._42;

	// 하단 평면을 정규화.
	length = sqrtf((m_planes[5][0] * m_planes[5][0]) + (m_planes[5][1] * m_planes[5][1]) + (m_planes[5][2] * m_planes[5][2]));
	m_planes[5][0] /= length;
	m_planes[5][1] /= length;
	m_planes[5][2] /= length;
	m_planes[5][3] /= length;

	return;
} // BuildFrustum

bool Frustum::CheckPoint(float x, float y, float z) {
	// 점이 여섯 개의 평면 모두 내부에 있는지, 
	// 절두체 내부에 있는지 확인하기 위해 여섯 개의 평면 각각을 검사
	for (unsigned int i = 0; i < 6; i++) {
		// 평면과 3D 점의 내적
		float dotProduct = (m_planes[i][0] * x) + (m_planes[i][1] * y) + (m_planes[i][2] * z) + (m_planes[i][3] * 1.0f);

		// 해당 지점이 현재 평면의 올바른 쪽에 있는지 확인하고 그렇지 않으면 종료
		if (dotProduct <= 0.0f) return false;
	}

	return true;
} // CheckPoint

bool Frustum::CheckCube(float xCenter, float yCenter, float zCenter, float radius) {
	unsigned int i;
	float dotProduct;

	// 정육면체가 원뿔대 안에 있는지 확인하기 위해 6개의 평면 각각을 검사
	for (i = 0; i < 6; i++) {
		// 정육면체의 여덟 꼭짓점이 모두 원뿔대 안에 있는지 확인
		dotProduct = (m_planes[i][0] * (xCenter - radius)) + (m_planes[i][1] * (yCenter - radius)) + (m_planes[i][2] * (zCenter - radius)) + (m_planes[i][3] * 1.0f);
		if (dotProduct > 0.0f) {
			continue;
		}

		dotProduct = (m_planes[i][0] * (xCenter + radius)) + (m_planes[i][1] * (yCenter - radius)) + (m_planes[i][2] * (zCenter - radius)) + (m_planes[i][3] * 1.0f);
		if (dotProduct > 0.0f) {
			continue;
		}

		dotProduct = (m_planes[i][0] * (xCenter - radius)) + (m_planes[i][1] * (yCenter + radius)) + (m_planes[i][2] * (zCenter - radius)) + (m_planes[i][3] * 1.0f);
		if (dotProduct > 0.0f) {
			continue;
		}

		dotProduct = (m_planes[i][0] * (xCenter + radius)) + (m_planes[i][1] * (yCenter + radius)) + (m_planes[i][2] * (zCenter - radius)) + (m_planes[i][3] * 1.0f);
		if (dotProduct > 0.0f) {
			continue;
		}

		dotProduct = (m_planes[i][0] * (xCenter - radius)) + (m_planes[i][1] * (yCenter - radius)) + (m_planes[i][2] * (zCenter + radius)) + (m_planes[i][3] * 1.0f);
		if (dotProduct > 0.0f) {
			continue;
		}

		dotProduct = (m_planes[i][0] * (xCenter + radius)) + (m_planes[i][1] * (yCenter - radius)) + (m_planes[i][2] * (zCenter + radius)) + (m_planes[i][3] * 1.0f);
		if (dotProduct > 0.0f) {
			continue;
		}

		dotProduct = (m_planes[i][0] * (xCenter - radius)) + (m_planes[i][1] * (yCenter + radius)) + (m_planes[i][2] * (zCenter + radius)) + (m_planes[i][3] * 1.0f);
		if (dotProduct > 0.0f) {
			continue;
		}

		dotProduct = (m_planes[i][0] * (xCenter + radius)) + (m_planes[i][1] * (yCenter + radius)) + (m_planes[i][2] * (zCenter + radius)) + (m_planes[i][3] * 1.0f);
		if (dotProduct > 0.0f) {
			continue;
		}

		return false;
	}

	return true;
} // CheckCube

bool Frustum::CheckSphere(float xCenter, float yCenter, float zCenter, float radius) {
	// 구체가 여섯 개의 평면 안에 있는지 아닌지 확인
	for (unsigned int i = 0; i < 6; i++)
	{
		float dotProduct = ((m_planes[i][0] * xCenter) + (m_planes[i][1] * yCenter) + (m_planes[i][2] * zCenter) + (m_planes[i][3] * 1.0f));
		if (dotProduct <= -radius)
            return false;
	}

	return true;
} // CheckSphere


// 중심점(Center)과 반지름(Extent/Size) 을 기준
// 보통 물리 엔진이나 특정 캐릭터의 바운딩 박스를 다룰 때 중심 위치를 기준으로 크기를 더하고 빼는 방식이 편리할 때 주로 사용한다함
bool Frustum::CheckBoundingBox(float xCenter, float yCenter, float zCenter, float xSize, float ySize, float zSize) {
	// 직사각형이 원뿔대 안에 있는지 여부를 확인하기 위해 여섯 개의 평면 각각을 체크
	for (unsigned int i = 0; i < 6; i++) {
		float dotProduct = (m_planes[i][0] * (xCenter - xSize)) + (m_planes[i][1] * (yCenter - ySize)) + (m_planes[i][2] * (zCenter - zSize)) + (m_planes[i][3] * 1.0f);
		if (dotProduct >= 0.0f) continue;

		dotProduct = (m_planes[i][0] * (xCenter + xSize)) + (m_planes[i][1] * (yCenter - ySize)) + (m_planes[i][2] * (zCenter - zSize)) + (m_planes[i][3] * 1.0f);
		if (dotProduct >= 0.0f) continue;

		dotProduct = (m_planes[i][0] * (xCenter - xSize)) + (m_planes[i][1] * (yCenter + ySize)) + (m_planes[i][2] * (zCenter - zSize)) + (m_planes[i][3] * 1.0f);
		if (dotProduct >= 0.0f) continue;

		dotProduct = (m_planes[i][0] * (xCenter + xSize)) + (m_planes[i][1] * (yCenter + ySize)) + (m_planes[i][2] * (zCenter - zSize)) + (m_planes[i][3] * 1.0f);
		if (dotProduct >= 0.0f) continue;

		dotProduct = (m_planes[i][0] * (xCenter - xSize)) + (m_planes[i][1] * (yCenter - ySize)) + (m_planes[i][2] * (zCenter + zSize)) + (m_planes[i][3] * 1.0f);
		if (dotProduct >= 0.0f) continue;

		dotProduct = (m_planes[i][0] * (xCenter + xSize)) + (m_planes[i][1] * (yCenter - ySize)) + (m_planes[i][2] * (zCenter + zSize)) + (m_planes[i][3] * 1.0f);
		if (dotProduct >= 0.0f) continue;

		dotProduct = (m_planes[i][0] * (xCenter - xSize)) + (m_planes[i][1] * (yCenter + ySize)) + (m_planes[i][2] * (zCenter + zSize)) + (m_planes[i][3] * 1.0f);
		if (dotProduct >= 0.0f) continue;

		dotProduct = (m_planes[i][0] * (xCenter + xSize)) + (m_planes[i][1] * (yCenter + ySize)) + (m_planes[i][2] * (zCenter + zSize)) + (m_planes[i][3] * 1.0f);
		if (dotProduct >= 0.0f) continue;

		return false;
	}

	return true;
} // CheckBoundingBox


// 최소점(Min)과 최대점(Max) 좌표를 직접 입력
// Terrain의 Cell이나 버텍스 데이터의 전체 범위를 계산할 때 가장 직관적으로 얻을 수 있는 값(Min/Max)을 그대로 사용할 때 편리하다 함
bool Frustum::CheckBoundingBoxMinMax(float maxWidth, float maxHeight, float maxDepth, float minWidth, float minHeight, float minDepth) {
	float dotProduct;

	// 직사각형의 6개 평면 중 어느 하나라도 뷰 절두체 내부에 있는지 확인
	for (unsigned int i = 0; i < 6; i++) {
		dotProduct = ((m_planes[i][0] * minWidth) + (m_planes[i][1] * minHeight) + (m_planes[i][2] * minDepth) + (m_planes[i][3] * 1.0f));
		if (dotProduct >= 0.0f) continue;
		
		dotProduct = ((m_planes[i][0] * maxWidth) + (m_planes[i][1] * minHeight) + (m_planes[i][2] * minDepth) + (m_planes[i][3] * 1.0f));
		if (dotProduct >= 0.0f) continue;

		dotProduct = ((m_planes[i][0] * minWidth) + (m_planes[i][1] * maxHeight) + (m_planes[i][2] * minDepth) + (m_planes[i][3] * 1.0f));
		if (dotProduct >= 0.0f) continue;

		dotProduct = ((m_planes[i][0] * maxWidth) + (m_planes[i][1] * maxHeight) + (m_planes[i][2] * minDepth) + (m_planes[i][3] * 1.0f));
		if (dotProduct >= 0.0f) continue;

		dotProduct = ((m_planes[i][0] * minWidth) + (m_planes[i][1] * minHeight) + (m_planes[i][2] * maxDepth) + (m_planes[i][3] * 1.0f));
		if (dotProduct >= 0.0f) continue;

		dotProduct = ((m_planes[i][0] * maxWidth) + (m_planes[i][1] * minHeight) + (m_planes[i][2] * maxDepth) + (m_planes[i][3] * 1.0f));
		if (dotProduct >= 0.0f) continue;

		dotProduct = ((m_planes[i][0] * minWidth) + (m_planes[i][1] * maxHeight) + (m_planes[i][2] * maxDepth) + (m_planes[i][3] * 1.0f));
		if (dotProduct >= 0.0f) continue;
		
		dotProduct = ((m_planes[i][0] * maxWidth) + (m_planes[i][1] * maxHeight) + (m_planes[i][2] * maxDepth) + (m_planes[i][3] * 1.0f));
		if (dotProduct >= 0.0f) continue;

		return false;
	}

	return true;
} // CheckBoundingBoxMinMax
