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
vector<vi> graph1, graph2;
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

void runOptimization(vector<vi> &adj1, vector<vi> &adj2) {
  GRBEnv env = GRBEnv();
  GRBModel model = GRBModel(env);
  GRBVar vars[(int) adj1.size()];

  int nvars = (int) adj1.size();

  //Adding the variables to the model.
  for (int idx = 0; idx < nvars; idx += 1) {
    ostringstream vname;
    vname << "var" << idx;
    vars[idx] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, vname.str());
  }

  //Adding the constraints to the model.
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < (int) adj1[i].size(); j++) {
      int v = adj1[i][j];
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

  if (status == GRB_OPTIMAL) {
    printf("Solution found is optimal! The variables are:\n");
    vector<int> newVertex;
    for (int var = 0; var < n; var++) {
      printf("[%d=>%d] ", var, (int) vars[var].get(GRB_DoubleAttr_X));      
      if (vars[var].get(GRB_DoubleAttr_X) == 0) {
        //TODO: This vertex should be in the second graph to be used in the search for the second indepent set.
        newVertex.push_back(var);
      }
    }
    printf("\n");

    adj2.assign(newVertex.size(), vi());
    int vertexCount = 0;
    map<int, int> seen;
    
    for (int i = 0; i < (int) newVertex.size(); i++) {
      int u = newVertex[i];
      //if (vars[u].get(GRB_DoubleAttr_X) == 0) {
      if (seen.find(u) == seen.end()) {
        seen[u] = vertexCount++;
        printf("(1) vertex %d has not been seen until now! He turned %d\n", u, seen[u]);
      }
      //printf("Looking at vertex %d...\n", u);
      for (int j = 0; j < (int) adj1[u].size(); j++) {
        int v = adj1[u][j];
        if (vars[v].get(GRB_DoubleAttr_X) == 0) {
          if (seen.find(v) == seen.end()) {
            seen[v] = vertexCount++;
            printf("(1) vertex %d has not been seen until now! He turned %d\n", v, seen[v]);
          }
          adj2[seen[u]].push_back(seen[v]);
          adj2[seen[v]].push_back(seen[u]);
          //adj2[u].push_back(v);
          //adj2[v].push_back(u);
        }/* else {
            printf("   |__ Vertex %d is inside the MIS!\n", v);
            }*/
      }
      //}
    }
  }
}

void runOptimization_(vector<vi> &adj1) {
  GRBEnv env = GRBEnv();
  GRBModel model = GRBModel(env);
  GRBVar vars[(int) adj1.size()];
  int nvars = (int) adj1.size();

  //Adding the variables to the model.
  

  //Adding the constraints to the model.
  
  //Adding the objective.
  /*GRBLinExpr obj = 0.0;
  for (int var = 0; var < nvars; var++) {
    obj += vars[var];
  }*/
    
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
    //runOptimization(graph1, graph2);
    //printGraph(graph2);
    //runOptimization_(graph2);
  } catch (GRBException ex) {
    cout << "Error code = " << ex.getErrorCode() << endl;
    cout << ex.getMessage() << endl;
  } catch (...) {
    cout << "fora temer" << endl;
  } 

  return 0;
}
