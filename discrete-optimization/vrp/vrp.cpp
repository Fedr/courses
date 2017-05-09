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
  int stops() const { return (int)path_.size() - 2; }
  int customerAt(int i) const { return path_[i + 1]; }
  friend std::ostream & operator <<(std::ostream & o, const VPath & v);
  // computes the path increase in case of given customer insertion
  double insertCost(int c) const;
  // inserts given customer in the path
  bool insert(int c);
  // computes the path increase (always negative) in case of customer given by path index deletion
  double eraseAtCost(int i) const;
  // erases customer given by path index
  void eraseAt(int i);
  // moves customer given by path index into a better path position
  bool repositionFrom(int i);
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

double VPath::eraseAtCost(int i) const
{
  assert(i >= 0 && i + 2 < path_.size());
  return dist(path_[i], path_[i + 2])
    - dist(path_[i], path_[i + 1]) 
    - dist(path_[i + 1], path_[i + 2]);
}

void VPath::eraseAt(int i)
{
  assert(i >= 0 && i + 2 < path_.size());
  consumed_ -= customers[path_[i + 1]].demand;
  path_.erase(path_.begin() + i + 1);
}

bool VPath::repositionFrom(int oldPos)
{
  int c = customerAt(oldPos);
  double minIncr = DBL_MAX;
  int minPos = -1;
  for (int i = 0; i + 1 < path_.size(); ++i)
  {
    if (i == oldPos || i == oldPos + 1)
      continue;
    double x = dist(path_[i], c) + dist(c, path_[i + 1]) - dist(path_[i], path_[i + 1]);
    if (x < minIncr)
    {
      minIncr = x;
      minPos = i;
    }
  }
  if (eraseAtCost(oldPos) + minIncr >= 0)
    return false;
  if (minPos > oldPos)
    --minPos;
  path_.erase(path_.begin() + oldPos + 1);
  path_.insert(path_.begin() + minPos + 1, c);
  return true;
}

class Solution
{
public:
  Solution();
  double cost() const;
  void print(std::ostream & os) const;
  void moveCustomers(int tries);
  void swapCustomers(int tries);

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

void Solution::moveCustomers(int tries)
{
  std::uniform_int_distribution<> path(0, V - 1);
  for (int t = 0; t < tries; ++t)
  {
    int fromPath = path(re);
    int toPath = path(re);
    auto & fp = vs_[fromPath];
    auto & tp = vs_[toPath];
    int fromStops = fp.stops();
    if (fromStops <= 0)
      continue;
    int i = std::uniform_int_distribution<>(0, fromStops - 1)(re);
    int c = fp.customerAt(i);
    if (fromPath == toPath)
    {
      fp.repositionFrom(i);
      continue;
    }
    double cost = fp.eraseAtCost(i) + tp.insertCost(c);
    if (cost < 0)
    {
      fp.eraseAt(i);
      tp.insert(c);
      continue;
    }
  }
}

void Solution::swapCustomers(int tries)
{
  std::uniform_int_distribution<> path(0, V - 1);
  for (int t = 0; t < tries; ++t)
  {
    auto & p0 = vs_[path(re)];
    auto & p1 = vs_[path(re)];
    int stops0 = p0.stops();
    int stops1 = p1.stops();
    if (stops0 <= 0 || stops1 <= 0)
      continue;
    int i0 = std::uniform_int_distribution<>(0, stops0 - 1)(re);
    int i1 = std::uniform_int_distribution<>(0, stops1 - 1)(re);
    if (&p0 == &p1)
    {
      p0.repositionFrom(i0);
      p0.repositionFrom(i1);
      continue;
    }
    double eraseCost0 = p0.eraseAtCost(i0);
    double eraseCost1 = p1.eraseAtCost(i1);
    if (eraseCost0 + eraseCost1 >= DBL_MAX)
      continue;
    int c0 = p0.customerAt(i0);
    int c1 = p1.customerAt(i1);
    p0.eraseAt(i0);
    p1.eraseAt(i1);
    double insertCost = p0.insertCost(c1) + p1.insertCost(c0);
    if (eraseCost0 + eraseCost1 + insertCost < 0)
    {
      p0.insert(c1);
      p1.insert(c0);
      continue;
    }
    p0.insert(c0);
    insertCost = p0.insertCost(c1);
    if (eraseCost1 + insertCost < 0)
    {
      p0.insert(c1);
      continue;
    }
    p1.insert(c1);
  }
}

int main(int argc, char * argv[])
{
  if (argc != 2)
    return 1;
  readData(argv[1]);

  Solution best;

  std::ofstream log("vrp.log", std::ofstream::app);
  log.precision(12);
  for (int iter = 0; iter < 100; ++iter)
  {
    best.swapCustomers(10000);
    log << "N=" << customers.size()
      << "\titer=" << iter
      << "\tbest=" << best.cost()
      << std::endl;
  }

  std::ostringstream os;
  os << customers.size() << ".sol";
  std::ofstream sol(os.str());
  best.print(sol);

  best.print(std::cout);

  return 0;
}
