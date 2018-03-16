// This file contains the original 2MIS formulation WITH HEURISTICS
#include <assert.h>
#include <fstream>
#include <map>
#include <set>
#include <stdlib.h>
#include <sstream>
#include <deque>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <vector>
#include <map>
#include <cmath>
#include "gurobi_c++.h"

using namespace std;

typedef vector<int> vi;

vector<vi> graph1;
int n, m;

class mycallback: public GRBCallback {
public:
  double lastiter;
  double lastnode;
  int numvars;
  GRBVar* vars;
  ofstream* logfile;
  vector<vi> adjList;
  int nVertex;
  mycallback(int xnumvars, GRBVar* xvars, ofstream* xlogfile, vector<vi> &adjList) {
    lastiter = lastnode = -GRB_INFINITY;
    numvars = xnumvars;
    vars = xvars;
    logfile = xlogfile;
    this->adjList = adjList;
    nVertex = (int) adjList.size();
  }
protected:
  void callback () {
    try {
      if (where == GRB_CB_MIPNODE) {
        if (getIntInfo(GRB_CB_MIPNODE_STATUS) == GRB_OPTIMAL) {
          double* x = getNodeRel(vars, numvars);
          double* y = getNodeRel(vars, numvars); 
          RND1(x), RND2(y); //Apply the heuristics
          int sum1 = 0, sum2 = 0;
          for (int i = 0; i < numvars; i++) {
            sum1 += (int) x[i];
            sum2 += (int) y[i];
          }
          if (sum1 > sum2) {
            setSolution(vars, x, numvars);
          } else setSolution(vars, y, numvars);
          delete[] x;
          delete[] y;          
        }
      }
    } catch (GRBException e) {
      cout << "Error number: " << e.getErrorCode() << endl;
      cout << e.getMessage() << endl;
    }    
  }

private:
  bool isEveryoneIsZeroOrOne(double *variables) {
    for (int i = 0; i < numvars; i++) {
      if (variables[i] != 0.0 && variables[i] != 1.0)
        return false;
    }
    return true;
  }
  //-----------------------Heuristics----------------------------
  //Set to 1.0 the selected variable and to 0.0 all its adjacents (for each MIS)
  void RND1(double *variables, int l, int r) {
    double value = numeric_limits<double>::lowest();
    int idx = -100;
    int factor = (r == numvars / 2 ? r : -r);
    for (int i = l; i < r; i++) {
      if (variables[i] < 1.0 && variables[i] > value) {
        if (variables[i] > variables[i + factor]) {
          value = variables[i];
          idx = i;
        }
      }
    }

    assert(idx >= 0 && (idx + factor >= 0 && idx + factor < numvars));
    variables[idx] = 1.0;
    variables[idx + factor] = 0.0;

    if (factor == (numvars / 2)) { //Se estou no primeiro conjunto
      for (const auto &v : adjList[idx]) {
        variables[v] = 0.0;
      }
    } else {
      for (const auto &v : adjList[idx - factor]) { //Se estou no segundo conjunto
        variables[v - factor] = 0.0;
      }
    }
  }
  void RND1(double *variables) {
    while (!isEveryoneIsZeroOrOne(variables)) {
      RND1(variables, 0, numvars / 2);
      RND1(variables, numvars / 2, numvars);
    }
  }
  
  void RND2(double *variables, int l, int r) {
    double value = numeric_limits<double>::max();
    int idx = -100;
    int factor = (r == numvars / 2 ? r : -r);
    for (int i = l; i < r; i++) {
      if (variables[i] > 0.0 && variables[i] < 1.0) {
        if (variables[i] < variables[i + factor]) {
          double sum = variables[i];
          if (factor == (numvars / 2)) {
            for (const auto &v : adjList[i]) {
              sum += variables[v];
            }
          } else {
            for (const auto &v : adjList[i + factor]) {
              sum += variables[v];
            }
          }

          if (sum > value) {
            value = sum;
            idx = i;
          }
        }
      }

      assert(idx >= 0 && (idx + factor >= 0 && idx + factor < numvars));
      variables[idx] = 1.0;
      variables[idx + factor] = 0.0;
      if (factor == (numvars / 2)) { //Se estou no primeiro conjunto
        for (const auto &v : adjList[idx]) {
          variables[v] = 0.0;
        }
      } else {
        for (const auto &v : adjList[idx - factor]) { //Se estou no segundo conjunto
          variables[v - factor] = 0.0;
        }
      }
    }
  }
  
  void RND2(double *variables) {   
    while (!isEveryoneIsZeroOrOne(variables)) {
      RND2(variables, 0, numvars / 2);
      RND2(variables, numvars / 2, numvars);
    }
  }
};

void printGraph(vector<vi> &adjList) {
  for (int i = 0; i < (int) adjList.size(); i++) {
    printf("Vertex %d: ", i);
    for (int j = 0; j < (int) adjList[i].size(); j++) {
      printf("%d ", adjList[i][j]);
    }
    puts("");
  }
}

void readGraph(char *name, vector<vi> &adjList) {
  FILE *instanceFile = freopen(name, "r", stdin);

  if (instanceFile == NULL) {
    printf("File cannot be open! Ending the program...\n");
    exit(10);
  }

  char s, t[30];
  scanf("%c %s", &s, t);
  scanf("%d %d", &n, &m);
  printf("%c %s %d %d\n", s, t, n, m);
  printf("There is %d vertex and %d edges. The array have size of %d\n", n, m, (n * m));

  adjList.assign(n, vi());

  int szName = strlen(name);
  bool clq = (name[szName - 1] == 'q' && name[szName - 2] == 'l' && name[szName - 3] == 'c');

  for (int i = 0; i < m; i += 1) {
    int u = 0, v = 0; char c = '\0';
    
    if (clq) {
      scanf(" %c %d %d\n", &c, &u, &v);
    } else{
      scanf("%d %d\n", &u, &v);
    }
    u--, v--;
    adjList[u].push_back(v);
    adjList[v].push_back(u); //TODO: Is it necessary add this?
  }
}

void runOptimization(vector<vi> &adj) {
  int nVertex = (int) adj.size();
  int nvars = (int) (2 * adj.size());
  
  GRBEnv env = GRBEnv();
  GRBModel model = GRBModel(env);
  GRBVar vars[nvars]; 

  //Adding the variables to the model.
  for (int idx = 0; idx < nvars; idx += 1) {
    ostringstream vname;
    vname << "var_" << idx;// << (idx < nVertex ? 1 : 2);
    vars[idx] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, vname.str());
  }
  
  //------ Adding the constraints to the model.
  // 1 - The two IS are disjoints
  for (int idx = 0; idx < nVertex; idx += 1) {
    ostringstream cname;
    cname << "constr" << idx;
    model.addConstr(vars[idx] + vars[idx + nVertex] <= 1.0, cname.str());
  }
  
  // 2 - The two IS are valids.
  for (int i = 0; i < nVertex; i += 1) {
    for (int j = 0; j < (int) adj[i].size(); j += 1) {
      int v = adj[i][j];
      ostringstream cname, cname2;
      cname << "constr_1_" << (i * n) + v;
      cname2 << "constr_2_" << (i * n) + v;
      model.addConstr(vars[i] + vars[v] <= 1.0, cname.str());
      model.addConstr(vars[i + nVertex] + vars[v + nVertex] <= 1.0, cname2.str());
    }
  }
  
  //------ Adding the objective.
  GRBLinExpr obj = 0.0;
  for (int var = 0; var < nvars; var++) {
    obj += vars[var];
  }

  //------- Callback
  ofstream logfile("cb.log");
  if (!logfile.is_open()) {
    cout << "Cannot open cb.log for callback message" << endl;
    exit(1);
  }
  
  mycallback cb = mycallback(nvars, vars, &logfile, adj);
  
  model.setCallback(&cb);
    
  model.setObjective(obj, GRB_MAXIMIZE);
  model.optimize();

  int status = model.get(GRB_IntAttr_Status);

  if (status == GRB_OPTIMAL) {
    printf("Solution found is optimal!\n");
    vector<int> newVertex;
    for (int var = 0; var < nvars; var++) {
      printf("%d ", (int) vars[var].get(GRB_DoubleAttr_X));
    }
    printf("\n");
  }
}

int main(int argc, char **argv) {

  if (argc < 2) {
    printf("Missing arguments...\n");
    printf("USAGE: ./2mis_ordinary_formulation_c++ 'pathOfInstance'\n");
    exit(EXIT_FAILURE);
  }

  try {
    readGraph(argv[1], graph1); 
    runOptimization(graph1);
  } catch (GRBException ex) {
    cout << "Error code = " << ex.getErrorCode() << endl;
    cout << ex.getMessage() << endl;
  } catch (...) {
    cout << "fora temer" << endl;
  } 

  return 0;
}
