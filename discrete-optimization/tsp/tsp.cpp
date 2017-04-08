#include <algorithm>
#include <assert.h>
#include <chrono>
#include <iostream>
#include <set>
#include <vector>
#include <fstream>

int N = 0;

struct Point
{
  double x = 0, y = 0;
};

inline double dist(const Point & a, const Point & b)
{
  double dx = a.x - b.x;
  double dy = a.y - b.y;
  return sqrt(dx*dx + dy*dy);
}

std::vector<Point> points;

class Path
{
public:
  // creates random path
  Path();
  double value() const;
  void print() const;
  bool twoOptAll();
private:
  std::vector<int> order_;
  bool twoOpt_(int i);
};

Path::Path()
{
  order_.resize(N);
  for (int i = 0; i < N; ++i)
  {
    order_[i] = i;
  }
  std::random_shuffle(order_.begin(), order_.end());
}

bool Path::twoOpt_(int i)
{
  std::rotate(order_.begin(), order_.begin() + i + 1, order_.end());
  double iOld = dist(points[order_.back()], points[order_.front()]);
  double best = -DBL_MAX;
  int bestJ = -1;
  for (int j = 0; j + 1 < order_.size(); ++j)
  {
    double jOld = dist(points[order_[j]], points[order_[j + 1]]);
    double iNew = dist(points[order_.back()], points[order_[j]]);
    double jNew = dist(points[order_.front()], points[order_[j+1]]);
    double v = iOld + jOld - iNew - jNew;
    if (v > best)
    {
      best = v;
      bestJ = j;
    }
  }
  if (best <= 0)
    return false;
  std::reverse(order_.begin(), order_.begin() + bestJ + 1);
  return true;
}

bool Path::twoOptAll()
{
  auto v0 = value();
  bool res = false;
  for (int i = 0; i < N; ++i)
  {
    if (twoOpt_(0))
      res = true;
    auto v = value();
  }
  return res;
}

double Path::value() const
{
  double res = dist(points[order_.back()], points[order_.front()]);
  for (int i = 0; i + 1 < N; ++i)
  {
    res += dist(points[order_[i]], points[order_[i+1]]);
  }
  return res;
}

void Path::print() const
{
  for (int i = 0; i < N; ++i)
  {
    std::cout << order_[i] << ' ';
  }
}

int main(int argc, char * argv[])
{
  if (argc != 2)
    return 1;

  std::ifstream f(argv[1]);
  f >> N;
  points.resize(N);
  for (int i = 0; i < N; ++i)
  {
    f >> points[i].x >> points[i].y;
  }

  Path best;
  double bestValue = best.value();
  int A = 50000 / N;
  int B = 30;
  for (int i = 0; i < A; ++i)
  {
    Path p;
    for (int j = 0; j < B; ++j)
    {
      if (!p.twoOptAll())
        break;
    }
    double pValue = p.value();
    if (pValue < bestValue)
    {
      best = p;
      bestValue = pValue;
    }
  }

  std::cout.precision(12);
  std::cout << bestValue << " 0\n";
  best.print();
  std::cout << std::endl;

  return 0;
}
