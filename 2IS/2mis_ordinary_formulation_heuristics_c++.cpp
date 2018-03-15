// This file contains the original 2MIS formulation WITH HEURISTICS
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
  mycallback(int xnumvars, GRBVar* xvars, ofstream* xlogfile, vector<vi> &adjList) {
    lastiter = lastnode = -GRB_INFINITY;
    numvars = xnumvars;
    vars = xvars;
    logfile = xlogfile;
    this->adjList = adjList;
  }
protected:
  void callback () {
    try {
      if (where == GRB_CB_MIPNODE) {
        if (getIntInfo(GRB_CB_MIPNODE_STATUS) == GRB_OPTIMAL) {
          double* x = getNodeRel(vars, numvars);
          RND1(x), RND2(x); //Apply the heuristics
          setSolution(vars, x, numvars);
          delete[] x;
        }
      }
    } catch (GRBException e) {
      cout << "Error number: " << e.getErrorCode() << endl;
      cout << e.getMessage() << endl;
    }    
  }

private:
  //-----------------------Heuristics----------------------------
  //Set to 1.0 the selected variable and to 0.0 all its adjacents
  void RND1(double *variables) {
    double varValue = 0.0;
    int indexVar = -1;
    for (int i = 0 ; i < numvars; i++) {
      if (variables[i] < 1.0 && (variables[i] > varValue)) {
        indexVar = i;
        varValue = variables[i];
      }
    }

    variables[indexVar] = 1.0;
    for (int i = 0; i < (int) adjList[indexVar].size(); i++) {
      variables[adjList[indexVar][i]] = 0.0;
    }    
  }
  
  void RND2(double *variables) {
    double varValue = numeric_limits<double>::max();
    int indexVar = -1;
    for (int i = 0; i < numvars; i++) {
      if (variables[i] > 0.0 && variables[i] < 1.0) {
        double sum = variables[i];
        for (int j = 0; j < (int) adjList[i].size(); j++) {
          sum += variables[adjList[i][j]];
        }

        if (sum < varValue) {
          indexVar = i;
          varValue = sum;
        }
      }
    }

    variables[indexVar] = 1.0;
    for (int i = 0; i < (int) adjList[indexVar].size(); i++) {
      variables[adjList[indexVar][i]] = 0.0;
    }
  }
  //-----------------------Heuristics----------------------------
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
    exit(-1);
  }

  char s, t[30];
  scanf("%c %s", &s, t);
  scanf("%d %d", &n, &m);
  printf("There is %d vertex and %d edges. The array have size of %d\n", n, m, (n * m));

  adjList.assign(n, vi());

  for (int i = 0; i < m; i += 1) {
    int u, v;
    scanf("%d %d", &u, &v);
    u--, v--;
    adjList[u].push_back(v);
    adjList[v].push_back(u);
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
