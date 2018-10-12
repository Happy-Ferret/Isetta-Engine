/*
 * Copyright (c) 2018 Isetta
 */
#include "Scene/Transform.h"
#include "Core/Debug/Logger.h"
#include "Core/Math/Matrix3.h"
#include "Scene/Entity.h"
#include "Util.h"
#include "Scene/Component.h"
#include "Core/Debug/DebugDraw.h"
#if _DEBUG
#include "Graphics/GUI.h"
#include "Graphics/RectTransform.h"
#endif

namespace Isetta {

Math::Vector4 Transform::sharedV4{};

Transform::Transform(Entity* entity) : entity(entity) {}

Math::Vector3 Transform::GetWorldPos() {
  return GetLocalToWorldMatrix().GetCol(3).GetVector3();
}

Math::Vector3 Transform::GetLocalPos() const { return localPos; }

// TODO(YIDI): test this
void Transform::SetWorldPos(const Math::Vector3& newWorldPos) {
  SetDirty();

  if (parent == nullptr) {
    localPos = newWorldPos;
  } else {
    sharedV4.Set(newWorldPos, 1);
    localPos =
        (parent->GetLocalToWorldMatrix().Inverse() * sharedV4).GetVector3();
  }
}

// TODO(YIDI): test this
void Transform::SetLocalPos(const Math::Vector3& newLocalPos) {
  localPos = newLocalPos;
  SetDirty();
}

// TODO(YIDI): test this
void Transform::TranslateWorld(const Math::Vector3& delta) {
  SetWorldPos(GetWorldPos() + delta);
}

// TODO(YIDI): test this
void Transform::TranslateLocal(const Math::Vector3& delta) {
  SetLocalPos(localPos + delta);
}

// TODO(YIDI): test this
Math::Quaternion Transform::GetWorldRot() {
  if (parent == nullptr) {
    worldRot = localRot;
  } else {
    worldRot = parent->GetWorldRot() * localRot;
  }

  return worldRot;
}

// TODO(YIDI): test this
Math::Quaternion Transform::GetLocalRot() const { return localRot; }

// TODO(YIDI): test this
Math::Vector3 Transform::GetWorldEulerAngles() {
  return GetWorldRot().GetEulerAngles();
}

// TODO(YIDI): test this
Math::Vector3 Transform::GetLocalEulerAngles() const {
  return localRot.GetEulerAngles();
}

void Transform::SetWorldRot(const Math::Quaternion& newWorldRot) {
  worldRot = newWorldRot;
  SetDirty();

  if (parent == nullptr) {
    localRot = worldRot;
  } else {
    localRot = parent->GetWorldRot().GetInverse() * worldRot;
  }
}

void Transform::SetWorldRot(const Math::Vector3& worldEulers) {
  SetWorldRot(Math::Quaternion::FromEulerAngles(worldEulers));
}

void Transform::SetLocalRot(const Math::Quaternion& newLocalRot) {
  localRot = newLocalRot;
  SetDirty();
}

void Transform::SetLocalRot(const Math::Vector3& localEulers) {
  SetLocalRot(Math::Quaternion::FromEulerAngles(localEulers));
}

// passed
void Transform::RotateWorld(const Math::Vector3& eulerAngles) {
  SetWorldRot(Math::Quaternion::FromEulerAngles(eulerAngles) * GetWorldRot());
}

// passed
void Transform::RotateWorld(const Math::Vector3& axis, const float angle) {
  SetWorldRot(Math::Quaternion::FromAngleAxis(axis, angle) * GetWorldRot());
}

// passed
void Transform::RotateLocal(const Math::Vector3& eulerAngles) {
  // first, get the basis vectors in local space
  bool hasParent = GetParent() != nullptr;
  Math::Vector3 left, up, forward;
  if (hasParent) {
    Transform* parent = GetParent();
    left = parent->LocalDirFromWorldDir(GetLeft());
    up = parent->LocalDirFromWorldDir(GetUp());
    forward = parent->LocalDirFromWorldDir(GetForward());
  } else {
    left = GetLeft();
    up = GetUp();
    forward = GetForward();
  }
  // then, map euler angles and basis
  // i.e. Transform the euler angles to local space
  SetLocalRot(Math::Quaternion::FromEulerAngles(left * eulerAngles.x +
                                                up * eulerAngles.y +
                                                forward * eulerAngles.z) *
              localRot);
}

// passed
void Transform::RotateLocal(const Math::Vector3& axisWorldSpace,
                            const float angle) {
  // transform the axis from world space to the space that
  // this object sits in, so it can understand the axis correctly
  Math::Vector3 localAxis =
      GetParent() != nullptr ? GetParent()->LocalDirFromWorldDir(axisWorldSpace)
                             : axisWorldSpace;
  SetLocalRot(Math::Quaternion::FromAngleAxis(localAxis, angle) * localRot);
}

// passed
void Transform::RotateLocal(const Math::Quaternion& rotation) {
  SetLocalRot(rotation * localRot);
}

Math::Vector3 Transform::GetWorldScale() const {
  // TODO(YIDI):  implement this
  return localScale;
}

Math::Vector3 Transform::GetLocalScale() const { return localScale; }

void Transform::SetLocalScale(const Math::Vector3& newScale) {
  localScale = newScale;
  SetDirty();
}

// TODO(YIDI): Test this
void Transform::SetParent(Transform* transform) {
  if (parent == transform) {
    LOG_ERROR(Debug::Channel::Graphics,
              "You are trying to set (%s)'s parent to (%s), whose is already "
              "their parent",
              GetName().c_str(), transform->GetName().c_str());
    return;
  }
  Math::Vector3 originalPos = GetWorldPos();
  Math::Quaternion originalRot = GetWorldRot();

  if (parent != nullptr) {
    parent->RemoveChild(this);
  }
  if (transform != nullptr) {
    transform->AddChild(this);
  }
  parent = transform;
  SetWorldPos(originalPos);
  SetWorldRot(originalRot);
  SetDirty();

  // TODO(YIDI): Keep world transform and rotation and scale
}

Math::Vector3 Transform::GetForward() {
  return GetLocalToWorldMatrix().GetCol(2).GetVector3().Normalized();
}

Math::Vector3 Transform::GetUp() {
  return GetLocalToWorldMatrix().GetCol(1).GetVector3().Normalized();
}

Math::Vector3 Transform::GetLeft() {
  return GetLocalToWorldMatrix().GetCol(0).GetVector3().Normalized();
}

void Transform::LookAt(const Math::Vector3& target,
                       const Math::Vector3& worldUp) {
  Math::Vector3 forwardDir = target - GetLocalPos();
  Math::Vector3 rightDir =
      Math::Vector3::Cross(forwardDir, worldUp).Normalized();
  Math::Vector3 upDir = Math::Vector3::Cross(forwardDir, rightDir);

  SetWorldRot(Math::Quaternion::FromLookRotation(forwardDir, upDir));
}

// TODO(YIDI): Test this
Transform* Transform::GetChild(const U16 childIndex) {
  if (childIndex >= GetChildCount()) {
    throw std::exception{
        Util::StrFormat("Transform::GetChild => transform of (%s) only has %d "
                        "children but you are asking for the %dth one",
                        GetName().c_str(), GetChildCount(), childIndex)};
  }
  return children[childIndex];
}

std::string Transform::GetName() const { return entity->GetName(); }

Math::Vector3 Transform::WorldPosFromLocalPos(const Math::Vector3& localPoint) {
  sharedV4.Set(localPoint, 1);
  return (GetLocalToWorldMatrix() * sharedV4).GetVector3();
}

Math::Vector3 Transform::LocalPosFromWorldPos(const Math::Vector3& worldPoint) {
  sharedV4.Set(worldPoint, 1);
  return (GetLocalToWorldMatrix().Inverse() * sharedV4).GetVector3();
}

Math::Vector3 Transform::WorldDirFromLocalDir(
    const Math::Vector3& localDirection) {
  sharedV4.Set(localDirection, 0);
  return (GetLocalToWorldMatrix() * sharedV4).GetVector3();
}

Math::Vector3 Transform::LocalDirFromWorldDir(
    const Math::Vector3& worldDirection) {
  sharedV4.Set(worldDirection, 0);
  return (GetLocalToWorldMatrix().Inverse() * sharedV4).GetVector3();
}

void Transform::ForChildren(const Action<Transform*>& action) {
  for (auto& child : children) {
    action(child);
  }
}

void Transform::ForDescendents(const Action<Transform*>& action) {
  for (auto& child : children) {
    action(child);
    child->ForDescendents(action);
  }
}

void Transform::ForSelfAndDescendents(const Action<Transform*>& action) {
  action(this);
  ForDescendents(action);
}

void Transform::SetWorldTransform(const Math::Vector3& inPosition,
                                  const Math::Vector3& inEulerAngles,
                                  const Math::Vector3& inScale) {
  SetWorldPos(inPosition);
  SetWorldRot(inEulerAngles);
  SetLocalScale(inScale);
}

void Transform::SetH3DNodeTransform(const H3DNode node, Transform& transform) {
  h3dSetNodeTransMat(node, transform.GetLocalToWorldMatrix().Transpose().data);
}

void Transform::Print() {
  LOG_INFO(Debug::Channel::Graphics,
           "\nName [%s]"
           "\nWorldPos %s"
           "\nLocalPos %s"
           "\nWorldRot %s"
           "\nLocalRot %s"
           "\nLocalQuat %s"
           "\nWorldScale %s",
           GetName().c_str(), GetWorldPos().ToString().c_str(),
           GetLocalPos().ToString().c_str(),
           GetWorldRot().GetEulerAngles().ToString().c_str(),
           GetLocalRot().GetEulerAngles().ToString().c_str(),
           GetLocalRot().ToString().c_str(),
           GetWorldScale().ToString().c_str());
}

void Transform::DrawGUI() {
  std::string parentName = parent == nullptr ? "null" : parent->GetName();
  std::string content =
      GetName() + "\n\n" + "World Position: " + GetWorldPos().ToString() +
      "\n" + "Local Position: " + GetLocalPos().ToString() + "\n" +
      "World Rotation: " + GetWorldEulerAngles().ToString() + "\n" +
      "Local Rotation: " + GetLocalEulerAngles().ToString() + "\n" +
      "Local Scale: " + GetLocalScale().ToString() + "\n" +
      "Parent: " + parentName;
  GUI::Text(RectTransform{Math::Rect{-200, 200, 300, 100}, GUI::Pivot::TopRight,
                          GUI::Pivot::TopRight},
            content);
  if (GUI::Button(RectTransform{Math::Rect{-200, 330, 300, 30},
                                GUI::Pivot::TopRight, GUI::Pivot::TopRight},
                  "Reset")) {
    SetLocalRot(Math::Quaternion::identity);
    SetLocalPos(Math::Vector3::zero);
    SetLocalScale(Math::Vector3::one);
  }
  
  float height = 360;
  float padding = 15;
  GUI::Text(RectTransform{Math::Rect{-200, height, 300, 100},
                          GUI::Pivot::TopRight, GUI::Pivot::TopRight},
            "Components", GUI::TextStyle{Color::white});
  height += padding;
  for (const auto& component : entity->GetComponents()) {
    Component& comp = *component;
    GUI::Text(RectTransform{Math::Rect{-200, height, 300, 100},
                            GUI::Pivot::TopRight, GUI::Pivot::TopRight},
              typeid(comp).name());
    height += padding;
  }
  DebugDraw::Axis(GetLocalToWorldMatrix());
  DebugDraw::AxisSphere(GetLocalToWorldMatrix());
}

const Math::Matrix4& Transform::GetLocalToWorldMatrix() {
  if (isMatrixDirty) {
    RecalculateLocalToWorldMatrix();
    isMatrixDirty = false;
  }
  return localToWorldMatrix;
}

void Transform::RecalculateLocalToWorldMatrix() {
  Math::Matrix4 localToParentMatrix{};
  localToParentMatrix.SetCol(3, localPos, 1);                    // translation
  localToParentMatrix.SetTopLeftMatrix3(localRot.GetMatrix3());  // rotation

  Math::Matrix4 temp;
  temp.SetDiagonal(localScale.x, localScale.y, localScale.z, 1);
  localToParentMatrix = temp * localToParentMatrix;  // scale

  if (parent != nullptr) {
    localToWorldMatrix = parent->GetLocalToWorldMatrix() * localToParentMatrix;
  } else {
    localToWorldMatrix = localToParentMatrix;
  }
}

void Transform::AddChild(Transform* transform) {
  // duplicate child check is in SetParent
  children.push_back(transform);
}

void Transform::RemoveChild(Transform* transform) {
  for (auto it = children.begin(); it != children.end(); ++it) {
    if (*it == transform) {
      children.erase(it);
      return;
    }
  }

  throw std::exception{
      Util::StrFormat("Transform::RemoveChild => child (%s) doesn't exist!",
                      transform->GetName().c_str())};
}

void Transform::SetDirty() {
  ForSelfAndDescendents([](Transform* trans) { trans->isMatrixDirty = true; });
}

}  // namespace Isetta
