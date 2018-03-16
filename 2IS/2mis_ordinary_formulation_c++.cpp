// This file contains the original and pure 2MIS formulation
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
      //cin >> c >> u >> v;
    } else{
      scanf("%d %d\n", &u, &v);
      //cin >> u >> v;
    }

    //printf("%c %d %d\n", c, u, v);
    u--, v--;
    //printf("%c %d %d\n", c, u, v);
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
      //printf("adding (%d, %d), (%d, %d)\n", i, v, (i + nVertex), (v + nVertex));
      model.addConstr(vars[i] + vars[v] <= 1.0, cname.str());
      model.addConstr(vars[i + nVertex] + vars[v + nVertex] <= 1.0, cname2.str());
    }
  }
  
  //------ Adding the objective.
  GRBLinExpr obj = 0.0;
  for (int var = 0; var < nvars; var++) {
    obj += vars[var];
  }
  
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

  if (argc < 2) {
    printf("Missing arguments...\n");
    printf("USAGE: ./2mis_ordinary_formulation_c++ 'pathOfInstance'\n");
    exit(EXIT_FAILURE);
  }

  try{
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
