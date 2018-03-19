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

  adj.assign(n, vi());

  for (int i = 0; i < m; i += 1) {
    int u, v;
    fscanf(instanceFile, "%d %d", &u, &v);
    printf("edge (%d, %d)\n", u - 1, v - 1);
    u--, v--;
    adj[u].push_back(v);
    adj[v].push_back(u);
  }  

  fclose(instanceFile);
}

/*GRBModel optimize() {

  return nullptr;
  }*/

int main(int argc, char **argv) {

  if (argc < 2) {
    printf("Missing arguments...\n");
    return -1;
  }

  try {
    GRBEnv env = GRBEnv();
    GRBModel model = GRBModel(env);

    readGraph(argv[1]);

    //Adding the variables to the model.
    GRBVar vars[n];
    for (int idx = 0; idx < n; idx += 1) {
      ostringstream vname;
      vname << "var" << idx;
      vars[idx] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, vname.str());
    }

    //Adding the constraints to the model.
    for (int i = 0; i < n; i++) {
      for (int j = 0; j < (int) adj[i].size(); j++) {
        int v = adj[i][j];
        ostringstream cname;
        cname << "constr" << (i * n) + v;
        model.addConstr(vars[i] + vars[v] <= 1, cname.str());
      }
    }

    //Adding the objective.
    GRBLinExpr obj = 0.0;
    for (int var = 0; var < n; var++) {
      obj += vars[var];
    }
    
    model.setObjective(obj, GRB_MAXIMIZE);
    model.optimize();

    int status = model.get(GRB_IntAttr_Status);
    printf("Status: %d\n", status);
    if (status == GRB_OPTIMAL) {
      printf("Solution found is optimal!\n");
      for (int var = 0; var < n; var++) {
        cout << vars[var].get(GRB_DoubleAttr_X) << " ";
      }
      cout << endl;
    }
  } catch (GRBException ex) {
    cout << "Error code = " << ex.getErrorCode() << endl;
    cout << ex.getMessage() << endl;
  } catch (...) {
    cout << "fora temer" << endl;
  }  

  return 0;
}
