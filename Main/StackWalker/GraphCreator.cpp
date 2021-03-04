#include "GraphCreator.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <memory>
#include <set>

std::shared_ptr<CallTreeNode> GetBranch(std::vector<std::string> callStack)
{
  // Have to DFS to check if callStack[0]] already exists. If it does, increment all its parents (?)
  std::shared_ptr<CallTreeNode> child =
      std::make_shared<CallTreeNode>(callStack[0], 1); // Top of the stack. Leaf of the call tree.
  std::shared_ptr<CallTreeNode> parent;

  for (size_t i = 1; i < callStack.size(); i++)
  {
    // Set parent's count to immediate child's count.
    parent = std::make_shared<CallTreeNode>(callStack[i],
                                            child->GetCount()); // Set count to LeafSampleCount();
    //child->SetParent(*parent);
    parent->AddChild(*child);
    child = parent;
  }

  return parent;
}

int max(int a, int b)
{
  return (a > b) ? a : b;
}

std::set<std::string> CreateSetOfChildren(std::vector<CallTreeNode> children)
{
  std::set<std::string> funcSet;

  for (size_t i = 0; i < children.size(); i++)
  {
    funcSet.insert(children[i].GetName());
  }

  return funcSet;
}

bool comparator(CallTreeNode& lhs, CallTreeNode& rhs)
{
  return lhs.GetName() < rhs.GetName();
}

std::shared_ptr<CallTreeNode> MergeNodes(std::shared_ptr<CallTreeNode> root)
{
  std::sort((*(root->GetChildren())).begin(), (*(root->GetChildren())).end(), &comparator);

  for (int i = root->GetChildCount() - 1; i > 0; i--)
  {
    if (((root->GetChildAt(i))->GetName()) == ((root->GetChildAt(i - 1))->GetName()))
    {
      //Merge
      CallTreeNode* node = (root->GetChildAt(i - 1));
      CallTreeNode* nodeToErase = root->GetChildAt(i);
      node->SetCount((nodeToErase->GetCount()) + 1);
      // Add children of the node that will get erased.
      for (int j = 0; j < nodeToErase->GetChildCount(); j++)
      {
        CallTreeNode* adoptedChild = nodeToErase->GetChildAt(j);
        node->AddChild(*adoptedChild);
      }
      root->DeleteChildAt(i);
    }
  }
  for (int i = 0; i < root->GetChildCount(); i++)
  {
    // This level has finished merging at this point. So count the number of samples.
    auto updatedNode = MergeNodes(std::make_shared<CallTreeNode>(*(root->GetChildAt(i))));
    root->SetUpdatedChildAt(i, *updatedNode);
  }
  return root;
}

std::shared_ptr<CallTreeNode> CreateTree(std::string                           key,
                                         std::vector<std::vector<std::string>> values)
{
  std::shared_ptr<CallTreeNode> root = std::make_shared<CallTreeNode>(key, (int)values.size());
  //std::vector<std::vector<std::string>>::iterator iter = values.begin();
  for (size_t i = 0; i < values.size(); i++)
  {
    if (values[i].size() > 0)
    {
      std::shared_ptr<CallTreeNode> branch = GetBranch(values[i]);
      root->AddChild(*branch); // Added * here.
    }
  }

  root = MergeNodes(root);

  return root;
}

void CreateGraphAndJSON(std::map<std::string, std::vector<std::vector<std::string>>> callTrees)
{
  std::vector<CallTreeNode>                                              forest;
  std::map<std::string, std::vector<std::vector<std::string>>>::iterator iter = callTrees.begin();

  std::ofstream JSONFile;

  JSONFile.open("C:\\temp\\JSON.json", std::ios ::trunc);

  JSONFile << "[";
  while (iter != callTrees.end())
  {
    std::string                           key = iter->first;
    std::vector<std::vector<std::string>> value = iter->second;
    std::shared_ptr<CallTreeNode>         tree = CreateTree(key, value);
    //(&tree)->percentage = (tree.count / (int)callTrees.size()) * 100.0;
    forest.push_back(*tree);

    std::string JSON = (*tree).SerialiseToJSON();

    //delete tree;
    JSONFile << JSON;

    iter++;

    if (iter != callTrees.end())
      JSONFile << ",";
  }
  JSONFile << "]";

  // This forest has repeated nodes, merge the tree.
  // Set count for number of occurances. Count seems okay for root VI nodes.

  /*CallTreeNode* forestRoot = new CallTreeNode("root", -1);
  forestRoot->children = forest;*/
  //SetCount(forest);
  //forest[0]).SerialiseToJSON();
}