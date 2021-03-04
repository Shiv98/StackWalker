#pragma once
#include <iostream>
#include <map>
#include <string>
#include <vector>

class CallTreeNode;
typedef std::vector<CallTreeNode>           CallTreeNodes_t;
typedef std::vector<std::string>            CallStack_t;
typedef std::vector<CallStack_t>            CallStacks_t;
typedef std::map<std::string, CallStacks_t> VICallStacksMap_t;

class CallTreeNode
{
private:
  std::string               name;
  int                       count = 0;
  double                    percentage = 0;
  std::vector<CallTreeNode> children;
  std::vector<CallTreeNode> parents;

public:
  CallTreeNode(std::string name, int count) // Add to createdNodes when creating a node.
  {
    this->name = name;
    this->count = count;
  }

  void SetParent(CallTreeNode parent) { this->parents.push_back(parent); }

  CallTreeNode() {}

  ~CallTreeNode() {}

  void             AddChild(CallTreeNode child) { children.push_back(child); }
  void             SetCount(int count) { this->count = count; }
  int              GetCount() { return this->count; }
  int              GetChildCount() { return (int)children.size(); }
  std::string      GetName() { return name; }
  CallTreeNodes_t* GetChildren() { return &children; }
  CallTreeNode*    GetChildAt(int i) { return &(children[i]); }
  void             DeleteChildAt(int i) { children.erase(children.begin() + i); }
  void             SetUpdatedChildAt(int i, CallTreeNode updatedNode) { children[i] = updatedNode; }

  std::string SerialiseToJSON();
  std::string GetSerialisedName();
  std::string GetSerialisedCount();
  std::string GetSerialisedPercentage();
  std::string GetSerialisedChildren();
};
