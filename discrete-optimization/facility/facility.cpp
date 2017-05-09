#include "gurobi_c++.h"

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
  double capacity = 0;
  double x = 0;
  double y = 0;
};
std::vector<Facility> facilities;

struct Customer
{
  double demand = 0;
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

int main(int argc, char * argv[])
{
  if (argc != 2)
    return 1;
  readData(argv[1]);

  GRBEnv env;
  GRBModel model = GRBModel(env);
  model.set(GRB_StringAttr_ModelName, "facility");

  // Facility open decision variables: open[f] == 1 if facility f is open.
  std::unique_ptr<GRBVar[]> open( model.addVars((int)facilities.size(), GRB_BINARY) );
  for (int f = 0; f < facilities.size(); ++f)
  {
    std::ostringstream vname;
    vname << "Open" << f;
    open[f].set(GRB_DoubleAttr_Obj, facilities[f].setupCost);
    open[f].set(GRB_StringAttr_VarName, vname.str());
  }

  // Transportation decision variables: 
  // whether to transport from a facility f to a customer c
  std::vector< std::unique_ptr<GRBVar[]> > transport(customers.size());
  for (int c = 0; c < customers.size(); ++c)
  {
    transport[c].reset( model.addVars((int)facilities.size(), GRB_BINARY) );

    for (int f = 0; f < facilities.size(); ++f)
    {
      std::ostringstream vname;
      vname << "Trans" << f << "." << c;
      double dx = customers[c].x - facilities[f].x;
      double dy = customers[c].y - facilities[f].y;
      transport[c][f].set(GRB_DoubleAttr_Obj, sqrt(dx*dx + dy*dy));
      transport[c][f].set(GRB_StringAttr_VarName, vname.str());
    }
  }

  // The objective is to minimize the total fixed and variable costs
  model.set(GRB_IntAttr_ModelSense, GRB_MINIMIZE);

  // Production constraints
  // Note that the right-hand limit sets the production to zero if
  // the plant is closed
  for (int f = 0; f < facilities.size(); ++f)
  {
    GRBLinExpr totalDemand = 0;
    for (int c = 0; c < customers.size(); ++c)
    {
      totalDemand += customers[c].demand * transport[c][f];
    }
    std::ostringstream cname;
    cname << "Capacity" << f;
    model.addConstr(totalDemand <= facilities[f].capacity * open[f], cname.str());
  }

  // Demand constraints
  for (int c = 0; c < customers.size(); ++c)
  {
    GRBLinExpr dtot = 0;
    for (int f = 0; f < facilities.size(); ++f)
    {
      dtot += transport[c][f];
    }
    std::ostringstream cname;
    cname << "Demand" << c;
    model.addConstr(dtot == 1, cname.str());
  }

  // First, open all facilities
  for (int f = 0; f < facilities.size(); ++f)
  {
    open[f].set(GRB_DoubleAttr_Start, 1.0);
  }

  // Use barrier to solve root relaxation
  model.set(GRB_IntParam_Method, GRB_METHOD_BARRIER);

  // Write optimization details in this file
  model.set(GRB_StringParam_LogFile, "optimize.txt");
  model.set(GRB_IntParam_LogToConsole, 0);

  // Solve
  try
  {
    model.optimize();
  }
  catch (const GRBException & e)
  {
    std::cout << "Error code = " << e.getErrorCode() << std::endl;
    std::cout << e.getMessage() << std::endl;
    return 1;
  }

  // Print solution
  std::cout << model.get(GRB_DoubleAttr_ObjVal) << " 0\n";
  for (int c = 0; c < customers.size(); ++c)
  {
    for (int f = 0; f < facilities.size(); ++f)
    {
      if (transport[c][f].get(GRB_DoubleAttr_X) > 0.99)
      {
        std::cout << f << ' ';
        break;
      }
    }
  }
  std::cout << std::endl;

  return 0;
}
