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
  void cfChanges(int num);

private:
  // returns distance between given customer and given facility
  double dist_(int customer, int facility) const;
  // returns false if the facility cannot serve the customer due to capacity limit
  bool serve_(int customer, int facility);
  // returns cost increase if serve given customer at given facility
  double serveCost_(int customer, int facility) const;
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

int main(int argc, char * argv[])
{
  if (argc != 2)
    return 1;
  readData(argv[1]);

  Solution best;

  std::ofstream log("facility.log", std::ofstream::app);
  log.precision(12);
  for (int iter = 0; iter < 100; ++iter)
  {
    best.cfChanges(10000);
    log << "iter=" << iter
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
