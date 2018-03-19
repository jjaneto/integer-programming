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

bool heuristic, cuts; 
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
  mycallback(int xnumvars, GRBVar* xvars, ofstream* xlogfile, vector<vi> adjList) {
    lastiter = lastnode = -GRB_INFINITY;
    numvars = xnumvars;
    vars = xvars;
    logfile = xlogfile;
    this->adjList = adjList;
    nVertex = (int) numvars / 2;
  }
protected:
  void callback () {
    try {
      if (where == GRB_CB_MIPNODE) {
        if (getIntInfo(GRB_CB_MIPNODE_STATUS) == GRB_OPTIMAL) {
          if (heuristic) {
            double* x = getNodeRel(vars, numvars);
            double* y = getNodeRel(vars, numvars); 
            RND1(x), RND2(y); //Apply the heuristics
            int sum1 = 0, sum2 = 0;
            for (int i = 0; i < numvars; i++) {
              sum1 += ((int) x[i]);
              sum2 += ((int) y[i]);
            }
            if (sum1 > sum2) {
              setSolution(vars, x, numvars);
            } else setSolution(vars, y, numvars);
            delete[] x;
            delete[] y;
          }

          if (cuts) {
            double* x = getNodeRel(vars, numvars);
            violatedCliqueConstraint(x);
          }
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
      if (variables[i] != 0.0 && variables[i] != 1.0) {
        //printf("var %d is %.2lf\n", i, variables[i]);
        return false;
      }
    }
    //printf("all right\n");
    return true;
  }
  //-----------------------Heuristics----------------------------
  //Set to 1.0 the selected variable and to 0.0 all its adjacents (for each MIS)
  void RND1(double *vars) {
    //cout << "begin rnd1" << endl;
    double variables[numvars];
    for (int i = 0; i < numvars; i++) {
      variables[i] = vars[i];
    }
    while (!isEveryoneIsZeroOrOne(variables)){
      double value = numeric_limits<double>::lowest();
      int idx = -100;

      for (int i = 0; i < numvars; i++) {
        if (variables[i] < 1.0 && variables[i] > value) {
          idx = i;
          value = variables[i];
        }
      }

      variables[idx] = 1.0;
      if (idx >= nVertex) {
        variables[idx - nVertex] = 0.0;
        for (const auto &v : adjList[idx - nVertex]) {
          variables[v + nVertex] = 0.0;
        }
      } else {
        variables[idx + nVertex] = 0.0;
        for (const auto &v : adjList[idx]) {
          variables[v] = 0.0;
        }
      }
    }
  }

  void RND2(double *vars) {
    //cout << "begin rnd2" << endl;
    double variables[numvars];
    for (int i = 0; i < numvars; i++) {
      variables[i] = vars[i];
    }
    while (!isEveryoneIsZeroOrOne(variables)) {
      double value = numeric_limits<double>::max();
      int idx = -100;

      for (int i = 0; i < numvars; i++) {
        if (variables[i] > 0.0 && variables[i] < 1.0) {
          double sum = variables[i];
          if (i >= nVertex) {
            for (const auto &v : adjList[i - nVertex]) {
              sum += variables[v + nVertex];
            }
          } else {
            for (const auto &v : adjList[i]) {
              sum += variables[v];
            }
          }

          if (sum < value) {
            idx = i;
            value = sum;
          }
        }
      }

      variables[idx] = 1.0;
      if (idx >= nVertex) {
        variables[idx - nVertex] = 0.0;
        for (const auto &v : adjList[idx - nVertex]) {
          variables[v + nVertex] = 0.0;
        }
      } else {
        variables[idx + nVertex] = 0.0;
        for (const auto &v : adjList[idx]) {
          variables[v] = 0.0;
        }
      }      
    }
  }
//-----------------------Heuristics----------------------------
//--------------------------Cuts-------------------------------
  void violatedCliqueConstraint(double *variables) {
    /*typedef vector<vi> Graph;
    Graph graph(this->adjList);
    vector<int> marked_nodes;
    vector<bool> isAvailable((int) adjList.size(), false);*/
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
    adjList[v].push_back(u);
  }
}

void runOptimization(vector<vi> &adj) {
  int nVertex = (int) adj.size();
  int nvars = (int) (2 * adj.size());
  
  GRBEnv env = GRBEnv();
  GRBModel model = GRBModel(env);
  GRBVar vars[nvars]; 

  model.set(GRB_IntParam_Presolve, 0);
  
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
    printf("First MIS:  ");
    for (int var = 0; var < nvars / 2; var++) {
      printf("%d ", (int) vars[var].get(GRB_DoubleAttr_X));
    }
    puts("");
    printf("Second MIS: ");
    for (int var = nvars/2; var < nvars; var++) {
      printf("%d ", (int) vars[var].get(GRB_DoubleAttr_X));
    }
    puts("");
  }
}

int main(int argc, char **argv) {

  if (argc < 4) {
    printf("Missing arguments...\n");
    printf("USAGE: ./2mis_ordinary_formulation_c++ [h|nh] [c|nc] [pathOfInstance]\n");
    exit(EXIT_FAILURE);
  }

  try {
    if (strcmp(argv[1], "h") == 0) {
      heuristic = true;
    } else if (strcmp(argv[1], "nh") == 0) {
      heuristic = false;
    } else {
      puts("COULD NOT RECOGNIZE THE HEURISTIC ARGS");
      exit(3);
    }

    if (strcmp(argv[2], "c") == 0) {
      cuts = true;
    } else if (strcmp(argv[2], "nc") == 0) {
      cuts = false;
    } else {
      puts("COULD NOT RECOGNIZE THE CUT ARGS");
      exit(3);
    }    
    cout << "Initializing optimization with " << heuristic << " for heuristics and " << cuts << " for cuts..." << endl;
    readGraph(argv[3], graph1); 
    runOptimization(graph1);
  } catch (GRBException ex) {
    cout << "Error code = " << ex.getErrorCode() << endl;
    cout << ex.getMessage() << endl;
  } catch (...) {
    cout << "fora temer" << endl;
  } 

  return 0;
}
