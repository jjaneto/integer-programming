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

char instanceName[50];
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
  strcat(instanceName, name);
  char pathInstance[50];
  memset(pathInstance, 0, sizeof pathInstance);
  strcat(pathInstance, "./instancias/");
  strcat(pathInstance, name);
  //printf("%s\n", pathInstance);
  FILE *instanceFile = fopen(pathInstance, "r");

  if (instanceFile == NULL) {
    printf("File cannot be open! Ending the program...\n");
    exit(-1);
  }

  char s, t[30];
  fscanf(instanceFile, "%c %s", &s, t);
  fscanf(instanceFile, "%d %d", &n, &m);
  printf("There is %d vertex and %d edges. The array have size of %d\n", n, m, (n * m));

  adjList.assign(n, vi());

  for (int i = 0; i < m; i += 1) {
    int u, v;
    fscanf(instanceFile, "%d %d", &u, &v);
    u--, v--;
    adjList[u].push_back(v);
    adjList[v].push_back(u);
  }
  
  fclose(instanceFile);
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
    return -1;
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
