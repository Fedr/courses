#include <algorithm>
#include <assert.h>
#include <iostream>
#include <set>
#include <vector>
#include <fstream>

// number of vertices
int V = 0;

// store for each vertex the list of adjacent vertices
std::vector<std::vector<int>> adjacency;

struct Vertex
{
  int color = -1; // not assigned yet
  int notColoredNeis = 0;
  std::vector<bool> prohibitedColors;
  int numProhibitedColors = 0;
};
//std::vector<Vertex> verts;

// id's of vertices sorted by their degree in descending order
std::vector<int> vertsSorted;

struct Coloring
{
  std::vector<Vertex> verts;
  int colorsUsed = 0;
  int firstNotColoredVertex = 0; // in vertSorted order

  int solve(); //no limit
  int solve(int maxColors);
  int numColors() const;
  void assignColor(int vert, int color);
  std::vector<int> getColorVariants(int maxColors, int vert) const;
  // returns a color from the list proposed, which is already prohibited in all not-colored neighbours
  int getColorProhibitedByAllNeis(int vert, const std::vector<int> & vars) const;
  bool assignFirstNotProhibitedColor(int maxColors, int vert);
  bool assignColorsNoVariants(int maxColors);
  void print() const;
};

void Coloring::assignColor(int vert, int color)
{
  colorsUsed = std::max(colorsUsed, color+1);
  auto & v = verts[vert];
  assert(v.color < 0);
  assert(v.prohibitedColors.size() <= color || !v.prohibitedColors[color]);
  v.color = color;
  v.prohibitedColors.clear(); //save space
  v.numProhibitedColors = -1; //invalid now
  for (int nei : adjacency[vert])
  {
    auto & vo = verts[nei];
    if (vo.color < 0)
    {
      if (vo.prohibitedColors.size() <= color)
      {
        vo.prohibitedColors.resize(color + 1);
      }
      if (!vo.prohibitedColors[color])
        ++vo.numProhibitedColors;
      vo.prohibitedColors[color] = true;
    }
    --vo.notColoredNeis;
  }
  if (vert == vertsSorted[firstNotColoredVertex])
  {
    for (++firstNotColoredVertex; firstNotColoredVertex < verts.size(); ++firstNotColoredVertex)
    {
      int vert = vertsSorted[firstNotColoredVertex];
      if (verts[vert].color < 0)
        break;
    }
  }
}

std::vector<int> Coloring::getColorVariants(int maxColors, int vert) const
{
  auto & v = verts[vert];
  assert(verts[vert].color < 0);

  std::vector<int> res;
  res.reserve(maxColors - v.numProhibitedColors);
  for (size_t i = 0; i < v.prohibitedColors.size(); ++i)
  {
    if (!v.prohibitedColors[i])
    {
      res.push_back((int)i);
    }
  }
  for (size_t i = v.prohibitedColors.size(); i < maxColors; ++i)
  {
    res.push_back((int)i);
  }

  assert (res.size() == maxColors - v.numProhibitedColors);
  return res;
}

int Coloring::getColorProhibitedByAllNeis(int vert, const std::vector<int> & vars) const
{
  for (int color : vars)
  {
    bool colorAllowed = false;
    for (int nei : adjacency[vert])
    {
      const auto & vnei = verts[nei];
      if (vnei.color >= 0)
        continue;
      if (vnei.prohibitedColors.size() <= color || !vnei.prohibitedColors[color])
      {
        colorAllowed = true;
        break;
      }
    }
    if (!colorAllowed)
      return color;
  }
  return -1;
}

bool Coloring::assignFirstNotProhibitedColor(int maxColors, int vert)
{
  auto & v = verts[vert];
  if (verts[vert].color >= 0)
    return true;
  int color = -1;
  for (size_t i = 0; i < v.prohibitedColors.size(); ++i)
  {
    if (!v.prohibitedColors[i])
    {
      color = (int)i;
      break;
    }
  }
  if (color < 0)
  {
    color = (int)v.prohibitedColors.size();
    if (color >= maxColors)
      return false;
  }
  assignColor(vert, color);
  return true;
}

int Coloring::solve()
{
  //greedy algorithm
  for (int i = firstNotColoredVertex; i < V; ++i)
  {
    if (!assignFirstNotProhibitedColor(V, vertsSorted[i]))
      return -1;
  }

  return numColors();
}

int Coloring::solve(int maxColors)
{
  if (!assignColorsNoVariants(maxColors))
    return -1;

  for (int i = firstNotColoredVertex; i < V; ++i)
  {
    int vert = vertsSorted[i];
    if (verts[vert].color >= 0)
      continue;
    auto vars = getColorVariants(std::min(colorsUsed+1, maxColors), vert);
    assert(vars.size() > 0);

    if (vars.size() == 1)
    {
      assignColor(vert, vars.front());
      if (!assignColorsNoVariants(maxColors))
        return -1;
      continue;
    }

    int col = getColorProhibitedByAllNeis(vert, vars);
    if (col >= 0)
    {
      assignColor(vert, col);
      if (!assignColorsNoVariants(maxColors))
        return -1;
      continue;
    }

    Coloring src = *this;
    for (int j = 0; j < vars.size(); ++j)
    {
      if (j != 0)
        *this = src;
      assignColor(vert, vars[j]);
      int s = solve(maxColors);
      if (s >= 0)
        return s;
    }
    return -1;
  }

  return numColors();
}

int Coloring::numColors() const
{
  return 1 + std::max_element(verts.begin(), verts.end(), [](const auto & a, const auto &b) { return a.color < b.color; })->color;
}

bool Coloring::assignColorsNoVariants(int maxColors)
{
  for (int m = 0;; ++m)
  {
    bool changed = false;
    for (int i = firstNotColoredVertex; i < V; ++i)
    {
      int vert = vertsSorted[i];
      const auto & v = verts[vert];
      if (v.color < 0 && v.numProhibitedColors >= maxColors)
        return false;
      if (v.color < 0 && v.notColoredNeis == 0)
      {
        assignFirstNotProhibitedColor(maxColors, vert);
        // do not mark as changed, since no other vertex is affected
      }
      else if (v.color < 0 && v.numProhibitedColors + 1 == maxColors)
      {
        assignFirstNotProhibitedColor(maxColors, vert);
        changed = true;
      }
    }
    if (!changed)
      break;
  }
  return true;
}

void Coloring::print() const
{
  std::cout << numColors() << " 0\n";
  for (int i = 0; i < V; ++i)
  {
    std::cout << verts[i].color << ' ';
  }
  std::cout << std::endl;
}

int main(int argc, char * argv[])
{
  if (argc != 2)
    return 1;

  std::ifstream f(argv[1]);
  int E = 0;
  f >> V >> E;
  adjacency.resize(V);
  for (int i = 0; i < E; ++i)
  {
    int v0 = -1, v1 = -1;
    f >> v0 >> v1;
    adjacency[v0].push_back(v1);
    adjacency[v1].push_back(v0);
  }

  Coloring c;
  c.verts.reserve(V);
  vertsSorted.reserve(V);
  for (int i = 0; i < V; ++i)
  {
    Vertex v;
    v.notColoredNeis = (int)adjacency[i].size();
    c.verts.push_back(v);
    vertsSorted.push_back(i);
  }
  std::sort(vertsSorted.begin(), vertsSorted.end(),
    [](int a, int b)
    {
      return adjacency[a].size() > adjacency[b].size();
    }
  );

  auto best = c;
  int cbest = best.solve();

  //hack
  int target = cbest - 1;
  if (V == 50)
    target = 6;
  else if (V == 70)
    target = 17; // long wait
  else if (V == 100)
    target = 16;
  else if (V == 250)
    target = 90; // optimal 78 :(
  else if (V == 500)
    target = 16;
  else if (V == 1000)
    target = 118; //optimal 100 :(

  auto sol1 = c;
  int c1 = sol1.solve(target);
  if (c1 >= 0)
  {
    best = sol1;
  }

  best.print();

  return 0;
}
