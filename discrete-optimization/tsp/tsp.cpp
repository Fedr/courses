#include <algorithm>
#include <assert.h>
#include <chrono>
#include <iostream>
#include <set>
#include <vector>
#include <fstream>
#include <random>

int N = 0;

struct Point
{
  double x = 0, y = 0;
};

inline double dist2(const Point & a, const Point & b)
{
  double dx = a.x - b.x;
  double dy = a.y - b.y;
  return dx*dx + dy*dy;
}

inline double dist(const Point & a, const Point & b)
{
  return sqrt(dist2(a, b));
}

std::vector<Point> points;

class Path
{
public:
  // creates random path
  Path();
  // creates greedy path with given start vertex
  Path(int v0);
  double value() const;
  void print() const;
  int twoOpt(int i, int j);
  int twoOpt(int i);
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

Path::Path(int v0)
{
  order_.resize(N);
  for (int i = 0; i < N; ++i)
  {
    order_[i] = i;
  }

  for (int i = 0; i + 1 < N; ++i)
  {
    double minDist2 = dist2(points[order_[i]], points[order_[i + 1]]);
    int minJ = i + 1;
    for (int j = i + 2; j < N; ++j)
    {
      double d2 = dist2(points[order_[i]], points[order_[j]]);
      if (d2 < minDist2)
      {
        minDist2 = d2;
        minJ = j;
      }
    }
    std::swap(order_[i + 1], order_[minJ]);
  }
}

// attempts to reverse the order of traversal in [i+1, j] or in [j+1,i] (whatever is smaller)
// returns the number of changed elements or 0 if the reverse is rejected
int Path::twoOpt(int i, int j)
{
  int i1 = i + 1 < N ? i + 1 : 0;
  int j1 = j + 1 < N ? j + 1 : 0;
  int n = j - i;
  if (n < 0)
    n += N;
  assert(n >= 0 && n <= N);
  int nb = N - n;
  if (nb < n)
  {
    std::swap(i, j);
    std::swap(i1, j1);
    std::swap(n, nb);
  }
  if (n <= 1)
    return 0;

  double iOld = dist(points[order_[i]], points[order_[i1]]);
  double jOld = dist(points[order_[j]], points[order_[j1]]);
  double iNew = dist(points[order_[i]], points[order_[j]]);
  double jNew = dist(points[order_[i1]], points[order_[j1]]);
  if (iOld + jOld - iNew - jNew <= 0)
    return 0;

  if (i <= j)
  {
    std::reverse(order_.begin() + i + 1, order_.begin() + j + 1);
  }
  else if (i + j + 2 < N)
  {
    for (int k = 0; k < j + 1; ++k)
    {
      std::swap(order_[i + 1 + k], order_[j - k]);
    }
    std::reverse(order_.begin() + i + 2 + j, order_.end());
  }
  else
  {
    for (int k = 0; i + 1 + k < N; ++k) // k_max = N - i - 2
    {
      std::swap(order_[i + 1 + k], order_[j - k]);
    }
    std::reverse(order_.begin(), order_.begin() + (j + i + 2 - N));
  }

  return n;
}

int Path::twoOpt(int i)
{
  for (int j = i + 2; j < N; ++j)
  {
    int n = twoOpt(i, j);
    if (n > 0)
      return n;
  }
  for (int j = 0; j + 1 < i; ++j)
  {
    int n = twoOpt(i, j);
    if (n > 0)
      return n;
  }
  return 0;
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

  std::default_random_engine re;
  std::uniform_int_distribution<> dis(0, N-1);

  Path best;
  double bestValue = best.value();
  int A = 6000000 / N;
  int MAX_CHANGES = 100000;
  int MAX_TRIES = 100;
  for (int iter = 0; iter < A; ++iter)
  {
    Path p;
    if (iter < N)
      p = Path(iter);
    int changes = 0;
    int tries = 0;
    while (changes < MAX_CHANGES && tries < MAX_TRIES)
    {
      int n = p.twoOpt(dis(re));
      if (n > 0)
      {
        changes += n;
        tries = 0;
      }
      else
        ++tries;
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
