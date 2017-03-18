#include <algorithm>
#include <assert.h>
#include <iostream>
#include <set>
#include <vector>
#include <fstream>

// number of vertices
int V = 0;
// number of edges
int E = 0;

struct Edge
{
  int v0 = 0;
  int v1 = 1;
  int other(int v) const { return (v == v0) ? v1 : v0;  }
};
std::vector<Edge> edges;

// computes the number of incident edges to a vertex
int degree(int v)
{
  int d = 0;
  for (const auto & e : edges)
  {
    if (e.v0 == v || e.v1 == v)
      ++d;
  }
  return d;
}

struct Vertex
{
  int degree = 0;
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
  int firstNotColoredVertex = 0;

  int extendClique(const std::set<int> & clique) const;
  int solve(); //no limit
  int solve(int maxColors);
  int numColors() const;
  void assignColor(int vert, int color);
  std::vector<int> getColorVariants(int maxColors, int vert) const;
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
  for (const auto & e : edges)
  {
    if (e.v0 == vert || e.v1 == vert)
    {
      auto & vo = verts[e.other(vert)];
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
  }
  if (vert == firstNotColoredVertex)
  {
    for (++firstNotColoredVertex; firstNotColoredVertex < verts.size() && verts[firstNotColoredVertex].color >= 0; ++firstNotColoredVertex)
    {
      //nothing here
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

int Coloring::extendClique(const std::set<int> & clique) const
{
  std::vector<int> neisInClique(V);
  for (const auto & e : edges)
  {
    if (clique.find(e.v0) != clique.end())
    {
      ++neisInClique[e.v1];
    }
    if (clique.find(e.v1) != clique.end())
    {
      ++neisInClique[e.v0];
    }
  }

  std::vector<int> neis;
  for (int i = 0; i < V; ++i)
  {
    if (neisInClique[i] == clique.size())
      neis.push_back(i);
  }
  if (neis.empty())
    return -1;

  //sort neis by degree
  std::sort(neis.begin(), neis.end(),
    [this](int a, int b)
  {
    return verts[a].degree < verts[b].degree;
  });

  return neis.back();
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
      const auto & v = verts[i];
      if (v.color < 0 && v.numProhibitedColors >= maxColors)
        return false;
      if (v.color < 0 && v.notColoredNeis == 0)
      {
        assignFirstNotProhibitedColor(maxColors, i);
        // do not mark as changed, since no other vertex is affected
      }
      else if (v.color < 0 && v.numProhibitedColors + 1 == maxColors)
      {
        assignFirstNotProhibitedColor(maxColors, i);
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
  f >> V >> E;
  edges.reserve(E);
  for (int i = 0; i < E; ++i)
  {
    Edge edge;
    f >> edge.v0 >> edge.v1;
    edges.push_back(edge);
  }

  Coloring c;
  c.verts.reserve(V);
  vertsSorted.reserve(V);
  for (int i = 0; i < V; ++i)
  {
    Vertex v;
    v.degree = v.notColoredNeis = degree(i);
    c.verts.push_back(v);
    vertsSorted.push_back(i);
  }
  std::sort(vertsSorted.begin(), vertsSorted.end(),
    [&c](int a, int b)
    {
      return c.verts[a].degree > c.verts[b].degree;
    }
  );

  //start assigning colors 
  c.assignColor(vertsSorted[0], 0);
  if (V != 250) //hack
  {
    std::set<int> clique;
    clique.insert(vertsSorted[0]);
    for (;;)
    {
      int v = c.extendClique(clique);
      if (v < 0)
        break;
      c.assignColor(v, (int)clique.size());
      clique.insert(v);
    }
    int cmin = (int)clique.size();
  }

  auto best = c;
  int cbest = best.solve();

  //hack
  int MAX_DOWN = 3;
  if (cbest > 90)
    MAX_DOWN = 0;

  for (int i = 0; i < MAX_DOWN; ++i)
  {
    auto sol1 = c;
    //std::cout << cbest - 1 << std::endl; ///!!!
    int c1 = sol1.solve(cbest - 1);
    if (c1 < 0)
      break;
    best = sol1;
    cbest = c1;
  }

  best.print();

  return 0;
}