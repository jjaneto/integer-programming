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
vector<vi> adj;
int n, m;

void readGraph(char *name) {
  strcat(instanceName, name);
  char pathInstance[50];
  memset(pathInstance, 0, sizeof pathInstance);
  strcat(pathInstance, "./instancias/");
  strcat(pathInstance, name);
  printf("%s\n", pathInstance);
  FILE *instanceFile = fopen(pathInstance, "r");

  if (instanceFile == NULL) {
    printf("File cannot be open! Ending the program...\n");
    exit(-1);
  }

  char s, t[30];
  int x = fscanf(instanceFile, "%c %s", &s, t);

  fscanf(instanceFile, "%d %d", &n, &m);
  adj.assign(n + 1, vi());
  
  for (int i = 0; i < m; i += 1) {
    int u, v;
    fscanf(instanceFile, "%d %d", &u, &v);
    adj[u].push_back(v);
    adj[v].push_back(u);
  }

  fclose(instanceFile);
}

int main(int argc, char **argv) {

  if (argc < 2) {
    printf("Missing arguments...\n");
    return -1;
  }

  try {
    readGraph(argv[1]);
    GRBEnv env = GRBEnv();
    GRBModel model = GRBModel(env);

    int adjMatrix[n][m];

    GRBVar vars[n * m];
    for (int idx = 0; idx < n * m; idx += 1) {
      string varName = "var" + idx;
      vars[idx] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, varName);
    }

    for (int i = 0; i < n; i++) {
      for (int j = 0; j < m; j++) {
        if (adjMatrix[i][j] == 1) {
          //GRBLineExpr constr = vars[i] + vars[j] <= 1;
          string constrName = "constr" + i + "_" + j;
          model.addConstr(vars[i] + vars[j] <= 1, constrName);
        }
      }      
    }

    //model.setObjective(w * x, GRB_MAXIMIZE);

    //model.optimize();
  } catch (GRBException ex) {
    cout << "Error code = " << ex.getErrorCode() << endl;
    cout << ex.getMessage() << endl;
  } catch (...) {
    cout << "fora temer" << endl;
  }  

  return 0;
}
