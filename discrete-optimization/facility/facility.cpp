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

private:
  // returns false if the facility cannot serve the customer due to capacity limit
  bool serve_(int customer, int facility);
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
    const auto & cm = customers[c];
    const auto & f = facilities[cs_[c].facility];
    double dx = cm.x - f.x;
    double dy = cm.y - f.y;
    res += sqrt(dx*dx + dy*dy);
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

int main(int argc, char * argv[])
{
  if (argc != 2)
    return 1;
  readData(argv[1]);

  Solution s;
  s.print(std::cout);

  return 0;
}
