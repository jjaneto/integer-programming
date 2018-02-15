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
int n, m;

int main(int argc, char **argv) {

  if (argc < 2) {
    printf("Missing arguments...\n");
    return -1;
  }

  try {
    GRBEnv env = GRBEnv();
    GRBModel model = GRBModel(env);
    char *name = argv[1];
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
    int adjMatrix[n * n];
    memset(adjMatrix, 0, sizeof adjMatrix);
    for (int i = 0; i < m; i += 1) {
      int u, v;
      fscanf(instanceFile, "%d %d", &u, &v);
      printf("edge (%d, %d)\n", u - 1, v - 1);
      u--, v--;
      adjMatrix[(u * n) + v] = 1;
      adjMatrix[(v * n) + u] = 1;
    }

    fclose(instanceFile);

    GRBVar vars[n];
    for (int idx = 0; idx < n; idx += 1) {
      ostringstream vname;
      vname << "var" << idx;
      vars[idx] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, vname.str());
    }

    for (int i = 0; i < n; i++) {
      for (int j = 0; j < n; j++) {
        if (i != j && adjMatrix[(i * n) + j] == 1) {
          printf("Adding constraint in %d and %d [%d]\n", i, j, (i * n) + j);
          ostringstream cname;
          cname << "cosntr" << (i * n) + j;      
          model.addConstr(vars[i] + vars[j] <= 1, cname.str());
        }
      }      
    }

    GRBLinExpr obj = 0.0;
    for (int var = 0; var < n; var++) {
      obj += vars[var];
    }
    
    model.setObjective(obj, GRB_MAXIMIZE);
    //model.set(GRB_IntAttr_ModelSense, GRB_MAXIMIZE);
    model.optimize();

    int status = model.get(GRB_IntAttr_Status);
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
