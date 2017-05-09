#include <algorithm>
#include <assert.h>
#include <chrono>
#include <iostream>
#include <memory>
#include <set>
#include <vector>
#include <fstream>
#include <sstream>
#include <random>

std::default_random_engine re;

struct Customer
{
  int demand = 0;
  double x = 0;
  double y = 0;
};
std::vector<Customer> customers;

int V = 0; //number of vehicles
int C = 0; //capacity of each vehicle

void readData(const char * filename)
{
  std::ifstream f(filename);
  int N;
  f >> N >> V >> C;
  customers.resize(N);
  for (auto & c : customers)
  {
    f >> c.demand >> c.x >> c.y;
  }
}

// returns distance between given two customers
double dist(int c0, int c1)
{
  const auto & cu0 = customers[c0];
  const auto & cu1 = customers[c1];
  double dx = cu0.x - cu1.x;
  double dy = cu0.y - cu1.y;
  return sqrt(dx*dx + dy*dy);
}

// path of one vehicle
class VPath
{
public:
  double len() const;
  friend std::ostream & operator <<(std::ostream & o, const VPath & v);
  // computes the path increase in case of given customer insertion
  double insertCost(int c) const;
  // inserts given customer in the path
  bool insert(int c);
private:
  int consumed_ = 0;
  std::vector<int> path_ = { 0, 0 };
};

double VPath::len() const
{
  double res = 0;
  for (int i = 0; i + 1 < path_.size(); ++i)
    res += dist(path_[i], path_[i + 1]);
  return res;
}

std::ostream & operator <<(std::ostream & o, const VPath & v)
{
  for (int c : v.path_)
    o << c << ' ';
  return o;
}

double VPath::insertCost(int c) const
{
  const auto & cu = customers[c];
  if (consumed_ + cu.demand > C)
    return DBL_MAX;

  double res = DBL_MAX;
  for (int i = 0; i + 1 < path_.size(); ++i)
  {
    double x = dist(path_[i], c) + dist(c, path_[i + 1]) - dist(path_[i], path_[i + 1]);
    if (x < res)
      res = x;
  }
  return res;
}

bool VPath::insert(int c)
{
  const auto & cu = customers[c];
  if (consumed_ + cu.demand > C)
    return false;

  double minIncr = DBL_MAX;
  int minPos = -1;
  for (int i = 0; i + 1 < path_.size(); ++i)
  {
    double x = dist(path_[i], c) + dist(c, path_[i + 1]) - dist(path_[i], path_[i + 1]);
    if (x < minIncr)
    {
      minIncr = x;
      minPos = i;
    }
  }
  path_.insert(path_.begin() + minPos + 1, c);
  consumed_ += cu.demand;
  return true;

}

class Solution
{
public:
  Solution();
  double cost() const;
  void print(std::ostream & os) const;

private:
  std::vector<VPath> vs_;
};

Solution::Solution()
  : vs_( V )
{
  std::vector<std::pair<int, int>> demandCust;
  for (int c = 1; c < customers.size(); ++c)
  {
    demandCust.emplace_back(customers[c].demand, c);
  }
  std::sort(demandCust.rbegin(), demandCust.rend());

  for (int i = 0; i < demandCust.size(); ++i)
  {
    int c = demandCust[i].second;
    double minCost = DBL_MAX;
    int bestV = -1;
    for (int v = 0; v < V; ++v)
    {
      double cost = vs_[v].insertCost(c);
      if (cost < minCost)
      {
        minCost = cost;
        bestV = v;
      }
    }
    vs_[bestV].insert(c);
  }
}

double Solution::cost() const
{
  double res = 0;
  for (const auto & v : vs_)
  {
    res += v.len();
  }
  return res;
}

void Solution::print(std::ostream & os) const
{
  os.precision(12);
  os << cost() << " 0\n";

  for (const auto & v : vs_)
  {
    os << v << '\n';
  }
  os << std::endl;
}


int main(int argc, char * argv[])
{
  if (argc != 2)
    return 1;
  readData(argv[1]);

  Solution best;

/*  std::ofstream log("facility.log", std::ofstream::app);
  log.precision(12);
  for (int iter = 0; iter < 1000; ++iter)
  {
    best.ffChanges(1000);
    best.ccChanges(10000);
    best.cfChanges(10000);
    log << "N=" << facilities.size()
      << "\tM=" << customers.size()
      << "\titer=" << iter
      << "\tbest=" << best.cost()
      << std::endl;
  }*/

  std::ostringstream os;
  os << customers.size() << ".sol";
  std::ofstream sol(os.str());
  best.print(sol);

  best.print(std::cout);

  return 0;
}
