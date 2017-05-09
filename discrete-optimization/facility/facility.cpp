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

struct Facility
{
  double setupCost = 0;
  int capacity = 0;
  double x = 0;
  double y = 0;
};
std::vector<Facility> facilities;

struct Customer
{
  int demand = 0;
  double x = 0;
  double y = 0;
};
std::vector<Customer> customers;

void readData(const char * filename)
{
  std::ifstream f(filename);
  int N, M;
  f >> N >> M;
  facilities.resize(N);
  for (auto & fa : facilities)
  {
    f >> fa.setupCost >> fa.capacity >> fa.x >> fa.y;
  }

  customers.resize(M);
  for (auto & cu : customers)
  {
    f >> cu.demand >> cu.x >> cu.y;
  }
}

class Solution
{
  struct FacilityUse
  {
    int consumed = 0; // <= capacity
  };
  std::vector<FacilityUse> fuse_;
  struct CustomerServed
  {
    int facility = -1;
  };
  std::vector<CustomerServed> cs_;

public:
  Solution();
  double cost() const;
  void print(std::ostream & os) const;
  void ccChanges(int num);
  void cfChanges(int num);
  void ffChanges(int num);

private:
  // returns distance between given customer and given facility
  double dist_(int customer, int facility) const;
  // returns false if the facility cannot serve the customer due to capacity limit
  bool serve_(int customer, int facility);
  // returns cost increase if serve given customer at given facility
  double serveCost_(int customer, int facility) const;
  // moves all customers from facility to facility
  bool moveFacility_(int from, int to);
  // returns cost increase if moves all customers from facility to facility
  double moveFacilityCost_(int from, int to) const;
  // swaps two customers between their serving facilities
  bool swapCustomers_(int c0, int c1);
  // returns cost increase if swap two customers between their serving facilities
  double swapCustomersCost_(int c0, int c1) const;
};

Solution::Solution()
  : fuse_( facilities.size() )
  , cs_( customers.size() )
{
  int f = 0;
  for (int c = 0; c < customers.size(); ++c)
  {
    while (!serve_(c, f))
    {
      ++f;
      assert(f < facilities.size());
    }
  }
}

double Solution::dist_(int customer, int facility) const
{
  const auto & cm = customers[customer];
  const auto & f = facilities[facility];
  double dx = cm.x - f.x;
  double dy = cm.y - f.y;
  return sqrt(dx*dx + dy*dy);
}

double Solution::cost() const
{
  double res = 0;
  for (int f = 0; f < facilities.size(); ++f)
  {
    if (fuse_[f].consumed == 0)
      continue;
    res += facilities[f].setupCost;
  }

  for (int c = 0; c < customers.size(); ++c)
  {
    res += dist_(c, cs_[c].facility);
  }
  return res;
}

void Solution::print(std::ostream & os) const
{
  os.precision(12);
  os << cost() << " 0\n";

  for (int c = 0; c < customers.size(); ++c)
  {
    os << cs_[c].facility << ' ';
  }
  os << std::endl;
}

void Solution::ccChanges(int num)
{
  std::uniform_int_distribution<> cust(0, (int)customers.size() - 1);
  for (int n = 0; n < num; ++n)
  {
    int c0 = cust(re);
    int c1 = cust(re);
    if (swapCustomersCost_(c0, c1) < 0)
      swapCustomers_(c0, c1);
  }
}

void Solution::cfChanges(int num)
{
  std::uniform_int_distribution<> cust(0, (int)customers.size() - 1);
  std::uniform_int_distribution<> faci(0, (int)facilities.size() - 1);
  for (int n = 0; n < num; ++n)
  {
    int c = cust(re);
    int f = faci(re);
    if (serveCost_(c, f) < 0)
      serve_(c, f);
  }
}

void Solution::ffChanges(int num)
{
  std::uniform_int_distribution<> faci(0, (int)facilities.size() - 1);
  for (int n = 0; n < num; ++n)
  {
    int from = faci(re);
    int to = faci(re);
    if (moveFacilityCost_(from, to) < 0)
      moveFacility_(from, to);
  }
}

bool Solution::serve_(int customer, int facility)
{
  CustomerServed & c = cs_[customer];
  if (c.facility == facility)
    return true;

  int demand = customers[customer].demand;

  if (facility >= 0)
  {
    if (fuse_[facility].consumed + demand > facilities[facility].capacity)
      return false;
    fuse_[facility].consumed += demand;
  }
  if (c.facility >= 0)
  {
    fuse_[c.facility].consumed -= demand;
    assert(fuse_[c.facility].consumed >= 0);
  }
  c.facility = facility;
  return true;
}

double Solution::serveCost_(int customer, int facility) const
{
  int oldFacility = cs_[customer].facility;
  if (oldFacility == facility)
    return 0;

  int demand = customers[customer].demand;

  assert(facility >= 0);
  assert(oldFacility >= 0);

  if (fuse_[facility].consumed + demand > facilities[facility].capacity)
    return DBL_MAX;

  double res = 0;
  if (fuse_[oldFacility].consumed == demand)
    res -= facilities[oldFacility].setupCost;
  if (fuse_[facility].consumed == 0)
    res += facilities[facility].setupCost;

  res -= dist_(customer, oldFacility);
  res += dist_(customer, facility);
  return res;
}

bool Solution::moveFacility_(int from, int to)
{
  if (to == from)
    return true;

  auto & fromConsumned = fuse_[from].consumed;
  auto & toConsumned = fuse_[to].consumed;
  if (fromConsumned + toConsumned > facilities[to].capacity)
    return false;

  for (auto & c : cs_)
  {
    if (c.facility == from)
      c.facility = to;
  }

  toConsumned += fromConsumned;
  fromConsumned = 0;
  return true;
}

double Solution::moveFacilityCost_(int from, int to) const
{
  if (to == from)
    return 0;

  auto fromConsumned = fuse_[from].consumed;
  auto toConsumned = fuse_[to].consumed;
  if (fromConsumned + toConsumned > facilities[to].capacity)
    return DBL_MAX;

  double res = -facilities[from].setupCost;
  if (fuse_[to].consumed == 0)
    res += facilities[to].setupCost;

  for (int c = 0; c < customers.size(); ++c)
  {
    if (cs_[c].facility == from)
      res += dist_(c, to) - dist_(c, from);
  }

  return res;
}

bool Solution::swapCustomers_(int c0, int c1)
{
  int & f0 = cs_[c0].facility;
  int & f1 = cs_[c1].facility;
  if (f0 == f1)
    return true;
  int demandDelta = customers[c1].demand - customers[c0].demand;
  if (demandDelta > 0 && fuse_[f0].consumed + demandDelta > facilities[f0].capacity)
    return false;
  if (demandDelta < 0 && fuse_[f1].consumed - demandDelta > facilities[f1].capacity)
    return false;

  fuse_[f0].consumed += demandDelta;
  fuse_[f1].consumed -= demandDelta;
  std::swap(f0, f1);
  return true;
}

double Solution::swapCustomersCost_(int c0, int c1) const
{
  int f0 = cs_[c0].facility;
  int f1 = cs_[c1].facility;
  if (f0 == f1)
    return 0;
  int demandDelta = customers[c1].demand - customers[c0].demand;
  if (demandDelta > 0 && fuse_[f0].consumed + demandDelta > facilities[f0].capacity)
    return DBL_MAX;
  if (demandDelta < 0 && fuse_[f1].consumed - demandDelta > facilities[f1].capacity)
    return DBL_MAX;

  double res =
      dist_(c0, f1) + dist_(c1, f0)
    - dist_(c0, f0) - dist_(c1, f1);
  return res;
}

int main(int argc, char * argv[])
{
  if (argc != 2)
    return 1;
  readData(argv[1]);

  Solution best;

  std::ofstream log("facility.log", std::ofstream::app);
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
  }

  std::ostringstream os;
  os << facilities.size() << '_' << customers.size() << ".sol";
  std::ofstream sol(os.str());
  best.print(sol);

  best.print(std::cout);

  return 0;
}
