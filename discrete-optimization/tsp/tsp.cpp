#include <algorithm>
#include <assert.h>
#include <chrono>
#include <iostream>
#include <set>
#include <vector>
#include <fstream>
#include <sstream>
#include <random>

int N = 0;

std::default_random_engine re;

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
  double value() const;
  void print(std::ostream & os) const;
  void read(std::istream & is);
  bool threeOpt(int i, int j, int k, double T, double prob);
  void manyThreeOpts(int tries, double T);
  void fullOptimize(double T);
private:
  int next(int i) const { return dblList_[2 * i]; }
  int & next(int i) { return dblList_[2 * i]; }
  int & prev(int i) { return dblList_[2 * i + 1]; }
  std::vector<int> dblList_;
};

Path::Path()
{
  dblList_.resize(2 * N);
  for (int i = 0; i < N; ++i)
  {
    next(i) = (i + 1) % N;
    prev(i) = (i + N - 1) % N;
  }
}

bool Path::threeOpt(int i, int j, int k, double T, double prob)
{
  if (i == j || j == k || k == i)
    return false;
  int i1 = next(i);
  int j1 = next(j);
  int k1 = next(k);

  double iOld = dist(points[i], points[i1]);
  double jOld = dist(points[j], points[j1]);
  double kOld = dist(points[k], points[k1]);
  double iNew = dist(points[i], points[j1]);
  double jNew = dist(points[j], points[k1]);
  double kNew = dist(points[k], points[i1]);
  double delta = iOld + jOld + kOld - iNew - jNew - kNew;
  if (delta <= 0)
  {
    if (T <= 0)
      return 0;
    if (exp(delta / T) < prob)
      return 0;
  }

  for (int ii = i1; ii != j; ii = next(ii))
  {
    if (ii == k)
      return false;
  }

  next(i) = j1;
  prev(j1) = i;
  next(j) = k1;
  prev(k1) = j;
  next(k) = i1;
  prev(i1) = k;

  return true;
}

void Path::manyThreeOpts(int tries, double T)
{
  std::uniform_int_distribution<> dis(0, N - 1);
  std::uniform_real_distribution<> probDist(0, 1);
  for (int i = 0; i < tries; ++i)
  {
    threeOpt(dis(re), dis(re), dis(re), T, probDist(re));
  }
}

void Path::fullOptimize(double T)
{
  int LEVELS = 7;
  int TRIES = 25000;
  for (int l = 0; l < LEVELS; ++l)
  {
    if (l + 1 == LEVELS)
      T = 0;
    else if (l > 0)
      T /= 1.5;
    manyThreeOpts(TRIES, T);
    TRIES *= 2;
  }
}

double Path::value() const
{
  double res = 0;
  int i = 0;
  do
  {
    int i1 = next(i);
    res += dist(points[i], points[i1]);
    i = i1;
  } while (i != 0);
  return res;
}

void Path::print(std::ostream & os) const
{
  os << value() << " 0\n";

  int i = 0;
  do
  {
    os << i << ' ';
    i = next(i);
  } while (i != 0);
}

void Path::read(std::istream & is)
{
  double v;
  int opt;
  is >> v >> opt;

  int previous = -1;
  int first = -1;
  for (int i = 0; i < N; ++i)
  {
    int curr;
    is >> curr;
    if (previous >= 0)
    {
      next(previous) = curr;
      prev(curr) = previous;
    }
    if (i == 0)
    {
      first = curr;
    }
    previous = curr;
  }
  next(previous) = first;
  prev(first) = previous;
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

  std::ostringstream os;
  os << N << ".sol";

  Path best;
  std::ifstream start("start/" + os.str());
  if (start)
    best.read(start);
  double bestValue = best.value();

  std::ofstream log("tsp.log", std::ofstream::app);

  int A = std::min(50000, 10*N);
  Path p = best;
  int bestIter = -1;
  for (int iter = 0; iter < A; ++iter)
  {
    double T = 1.5 * p.value() / (2*N);
    p.fullOptimize(T);
    double pValue = p.value();
    log << "iter=" << iter 
        << "\tlast=" << pValue 
        << "\tbestIter=" << bestIter
        << "\tbest=" << bestValue 
        << std::endl;
    if (pValue < bestValue)
    {
      best = p;
      bestValue = pValue;
      bestIter = iter;
    }
  }

  std::cout.precision(12);
  best.print(std::cout);
  std::cout << std::endl;

  std::ofstream res("best.txt", std::ofstream::app);
  res.precision(12);
  res << "N=" << N << "\tbestIter=" << bestIter << "\tmaxIter=" << A << "\tbest=" << bestValue << '\n';

  std::ofstream sol(os.str());
  sol.precision(12);
  best.print(sol);

  return 0;
}
