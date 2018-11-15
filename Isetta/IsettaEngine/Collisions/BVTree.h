/*
 * Copyright (c) 2018 Isetta
 */
#pragma once
#include <unordered_set>
#include "AABB.h"
#include "Collider.h"
#include "CollisionUtil.h"

namespace Isetta {

class BVTree {
  // BVNode serves two purposes: leaf and branch
  struct Node {
    explicit Node(AABB aabb) : aabb(std::move(aabb)) {}
    explicit Node(class Collider* const collider)
        : collider(collider), aabb(collider->GetFatAABB()) {}

    void UpdateBranchAABB();
    void UpdateLeafAABB();
    void SwapOutChild(Node* oldChild, Node* newChild);

    bool IsLeaf() const { return left == nullptr; }
    bool IsInFatAABB() const { return aabb.Contains(collider->GetAABB()); }

    class Collider* collider{nullptr};
    AABB aabb;

    Node* parent{nullptr};
    Node* left{nullptr};
    Node* right{nullptr};
  };

 public:
  ~BVTree();

  void AddCollider(class Collider* collider);
  void RemoveCollider(class Collider* collider);
  void Update();

  bool Raycast(const class Ray& ray, class RaycastHit* hitInfo,
               float maxDistance) const;
  bool Raycast(Node* node, const class Ray& ray, class RaycastHit* hitInfo,
               float maxDistance) const;

  const CollisionUtil::ColliderPairSet& GetCollisionPairs();
  Array<Collider*> GetPossibleColliders(class Collider* collider) const;

 private:
  BVTree() = default;
  void AddNode(Node* newNode);
  void RemoveNode(Node* node, bool deleteNode);
  void DebugDraw() const;

  CollisionUtil::ColliderPairSet colliderPairSet;
  std::unordered_map<class Collider*, Node*> colNodeMap;
  Node* root = nullptr;

  friend class CollisionsModule;
};

}  // namespace Isetta
