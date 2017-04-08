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
private:
  std::vector<int> order_;
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
  int A = 10000;
  for (int i = 0; i < A; ++i)
  {
    Path p;
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
