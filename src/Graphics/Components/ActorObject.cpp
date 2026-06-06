#include "Pch.h"
#include "ActorObject.h"

using namespace DirectX;

ActorObject::ActorObject() : m_RenderCount(0) {
} // ActorObject

void ActorObject::SetPosition(const XMFLOAT3& pos) {
	m_transform.SetPosition(pos);
} // SetPosition

void ActorObject::SetPosition(float x, float y, float z) {
	m_transform.SetPosition(x, y, z);
} // SetPosition

void ActorObject::SetRotation(const XMFLOAT3& rot) {
	m_transform.SetRotation(rot);
} // SetRotation

void ActorObject::SetRotation(float x, float y, float z) {
	m_transform.SetRotation(x, y, z);
} // SetRotation

void ActorObject::SetScale(const XMFLOAT3& scale) {
	m_transform.SetScale(scale);
} // SetScale

void ActorObject::SetScale(float x, float y, float z) {
	m_transform.SetScale(x, y, z);
} // SetScale

void ActorObject::Translate(const XMFLOAT3& offset) {
	m_transform.Translate(offset);
} // Translate

void ActorObject::Translate(float x, float y, float z) {
	m_transform.Translate(x, y, z);
} // Translate

void ActorObject::Rotate(const XMFLOAT3& rot) {
	m_transform.Rotate(rot);
} // Rotate

void ActorObject::Rotate(float x, float y, float z) {
	m_transform.Rotate(x, y, z);
} // Rotate

XMFLOAT3 ActorObject::GetPosition() const {
	return m_transform.GetPosition();
} // GetPosition

XMMATRIX ActorObject::GetWorldMatrix() {
	return m_transform.GetWorldMatrix();
} // GetWorldMatrix

unsigned int ActorObject::GetRenderCount() const {
	return m_RenderCount;
} // GetRenderCount