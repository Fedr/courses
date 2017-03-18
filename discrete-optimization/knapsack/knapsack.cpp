#include <algorithm>
#include <assert.h>
#include <iostream>
#include <set>
#include <vector>
#include <fstream>

struct Item
{
  int value = 0;
  int weight = 0;
};

std::vector<Item> items;

// knapsack capacity
int K = 0;

// id's of Items sorted by value density in descending order
std::vector<int> itemsSorted;

class Node
{
  std::vector<bool> selectedItems;
  int value = 0;
  int weight = 0;
  double estimation = 0;
public:
  // tree root
  Node();
  bool isValid() const { return weight <= K; }
  bool isFinal() const { return weight == K || selectedItems.size() == items.size(); }
  int getValue() const { return value; }
  double getEstimation() const { return estimation; }
  Node leftChild() const;
  Node rightChild() const;
  void print() const;
  bool operator < (const Node & rhs) const { return estimation < rhs.estimation; }
private:
  void computeEstimation();
};

Node::Node()
{
  computeEstimation();
}

Node Node::leftChild() const
{
  assert(!isFinal());
  Node res = *this;
  const Item & item = items[itemsSorted[selectedItems.size()]];
  res.selectedItems.push_back(true);
  res.weight += item.weight;
  res.value += item.value;
  res.computeEstimation();
  return res;
}

Node Node::rightChild() const
{
  assert(!isFinal());
  Node res = *this;
  res.selectedItems.push_back(false);
  res.computeEstimation();
  return res;
}

void Node::print() const
{
  std::cout << value << " 1\n";
  std::vector<int> sel(items.size());
  for (size_t i = 0; i < selectedItems.size(); ++i)
  {
    if (selectedItems[i])
      sel[itemsSorted[i]] = 1;
  }
  for (size_t i = 0; i < sel.size(); ++i)
  {
    std::cout << sel[i] << ' ';
  }
  std::cout << '\n';
}

void Node::computeEstimation()
{
  estimation = value;
  int w = weight;
  for (size_t i = selectedItems.size(); i < itemsSorted.size(); ++i)
  {
    if (w >= K)
      break;
    const Item & item = items[itemsSorted[i]];
    if (w + item.weight <= K)
    {
      estimation += item.value;
      w += item.weight;
    }
    else
    {
      estimation += (double)item.value / item.weight * (K - w);
      w = K;
      break;
    }
  }
}

int main(int argc, char * argv[])
{
  if (argc != 2)
    return 1;

  std::ifstream f(argv[1]);
  int numItems = 0;
  f >> numItems >> K;
  items.reserve(numItems);
  itemsSorted.reserve(numItems);
  for (int i = 0; i < numItems; ++i)
  {
    Item item;
    f >> item.value >> item.weight;
    items.push_back(item);
    itemsSorted.push_back(i);
  }

  std::sort(itemsSorted.begin(), itemsSorted.end(),
    [](int a, int b)
    {
      // int32 gets overflown here
      return (double)items[a].value * items[b].weight > (double)items[b].value * items[a].weight;
    }
  );

  Node best;
  std::set<Node> options;
  options.insert(Node());
  while (!options.empty())
  {
    Node curr = *--options.end();
    options.erase(--options.end());

    if (curr.isFinal())
    {
      if (best.getValue() < curr.getValue())
      {
        best = std::move(curr);
        //eliminate options with less estimation
        while (!options.empty() && options.begin()->getEstimation() <= best.getValue())
          options.erase(*options.begin());
      }
      continue;
    }

    Node l = curr.leftChild();
    if (l.isValid())
      options.insert(std::move(l));
    Node r = curr.rightChild();
    if (r.isValid())
      options.insert(std::move(r));
  }

  best.print();

  return 0;
}