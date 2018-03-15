#include <limits>
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
typedef pair<int, int> ii;

const int NODE_WEIGHT = 1;

vi hardness_main, value_main;
vector<set<int> > graph1;
set<vi> cliques_main;
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


void readGraph(char *name, vector<set<int> > &adjList) {
  FILE *instanceFile = freopen(name, "r", stdin);

  if (instanceFile == NULL) {
    printf("File cannot be open! Ending the program...\n");
    exit(-1);
  }

  char s, t[30];
  fscanf(instanceFile, "%c %s", &s, t);
  fscanf(instanceFile, "%d %d", &n, &m);
  printf("There is %d vertex and %d edges. The array have size of %d\n", n, m, (n * m));
  
  adjList.assign(n, set<int>());

  for (int i = 0; i < m; i += 1) {
    int u, v;
    scanf("%d %d", &u, &v);
    u--, v--;
    adjList[u].insert(v);
    adjList[v].insert(u);
  }
}

bool adjacentToAll(const int v, const vector<set<int> > &adjList, const vi &newClique_) {
  for (int i = 0; i < (int) newClique_.size(); i++) {
    if (adjList[newClique_[i]].find(v) == adjList[newClique_[i]].end()) {
      return false;
    }
  }

  return true;
}

void expandClique(const ii edge, const vector<set<int> > &adjList, vi &newClique_) {
  int u, v;
  newClique_.push_back((u = edge.first));
  newClique_.push_back((v = edge.second));
  
  for (int i = 0; i < (int) adjList.size(); i++) {
    if (adjacentToAll(i, adjList, newClique_)) {
      newClique_.push_back(i);
    }
  }
  
  printf("new clique\n");
  for (auto &x : newClique_) {
    printf("%d ", x + 1);
  }  
  puts("");
}

void clq1(const vector<set<int> > &adjList, set<vi> &cliques) {
  set<ii> marked;
  map<int, vi> cliques_map;
  int clique_number = 0;
  
  for (int u = 0; u < (int) adjList.size(); u++) {
    for (const auto &v : adjList[u]) {
      //printf("looking edge %d %d\n", u + 1, v + 1);
      if ((marked.find(ii(u, v)) == marked.end()) && (marked.find(ii(v, u)) == marked.end())) {
        vi newClique_;
        expandClique(ii(u, v), adjList, newClique_);
        cliques.insert(newClique_);
        
        cliques_map[clique_number++] = newClique_;
        
        int cliqueSize = (int) newClique_.size();
        for (int i = 0; i < cliqueSize; i++) {
          for (int j = i + 1; j < cliqueSize; j++) {
            int x = newClique_[i], y = newClique_[j];
            marked.insert(ii(newClique_[i], newClique_[j]));
          }
        }        
      }
    }
  }

  printf("The cliques are as follows:\n");
  for (const auto &clique_ : cliques) {
    for (int i = 0; i < (int) clique_.size(); i++) {
      printf("%d ", clique_[i] + 1);
    }
    puts("");
  }
}

void runOptimization(vector<set<int> > &adj, set<vi> &cliques) {
  int nVertex = (int) adj.size();
  int nvars = (int) (2 * adj.size());

  GRBEnv env = GRBEnv();
  GRBModel model = GRBModel(env);
  GRBVar vars[nvars];

  //------------ Adding the variables to the model.
  for (int idx = 0; idx < nvars; idx += 1) {
    ostringstream vname;
    vname << "var_" << idx;// << (idx < nVertex ? 1 : 2);
    vars[idx] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, vname.str());
  }

  //------------ Adding the constraints to the model.
  // 1 - The two IS are disjoints
  for (int idx = 0; idx < nVertex; idx += 1) {
    ostringstream cname;
    cname << "const_disjoints" << idx;
    model.addConstr(vars[idx] + vars[idx + nVertex] <= 1.0, cname.str());
  }
  
  int idx = 0;
  for (const auto &clique_ : cliques) { // For each clique
    GRBLinExpr var_sideone;
    GRBLinExpr var_sidetwo;
    ostringstream cname_clique, cname_clique2;
    cname_clique << "constr_clique_" << idx;
    cname_clique2 << "const_clique2_" << idx;
    idx++;
    //cout << "looking at clique: ";
    for (int i = 0; i < (int) clique_.size(); i++) {
      //printf("%d ", clique_[i] + 1);
      var_sideone += vars[clique_[i]];
      var_sidetwo += vars[clique_[i] + nVertex];
    }
    //puts("");
    model.addConstr(var_sideone <= 1.0, cname_clique.str());
    model.addConstr(var_sidetwo <= 1.0, cname_clique2.str());
  }
  
  //------------ Adding the objective.
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

  try {
    readGraph(argv[1], graph1);
    clq1(graph1, cliques_main);
    //runOptimization(graph1, cliques_main);
  } catch (GRBException ex) {
    cout << "Error code = " << ex.getErrorCode() << endl;
    cout << ex.getMessage() << endl;
  } catch (...) {
    cout << "fora temer" << endl;
  }
  
  return 0;
}
