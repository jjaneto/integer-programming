// This file contains the original CLQ1 formulation WITH HEURISTICS
#include <fstream>
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
vector<vi> graph1;
set<vi> cliques_main;
int n, m;

class mycallback: public GRBCallback {
public:
  double lastiter;
  double lastnode;
  int numvars;
  GRBVar* vars;
  vector<vi> adjList;
  ofstream* logfile;
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
  fscanf(instanceFile, "%c %s", &s, t);
  fscanf(instanceFile, "%d %d", &n, &m);
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

bool adjacentToAll(const vi &clique, const vector<vi> &adjList, const ii edge) {
  for (int k = 0; k < 2; k++) {
    int u = (k == 0 ? edge.first : edge.second);
    for (int i = 0; i < (int) adjList[u].size(); i++) {
      int v = adjList[u][i];
      bool isInside = false;

      for (int j = 0; j < (int) clique.size() && isInside; j++) {
        if (v == clique[j]) {
          isInside = true;
        }
      }
    
      if (!isInside)
        return false;
    }
  }

  return true;
}

void expandClique(const ii edge, const vector<vi> &adjList, vi &newClique_) {
  int u, v;
  newClique_.push_back((u = edge.first));
  newClique_.push_back((v = edge.second));

  for (int i = 0; i < (int) adjList[u].size(); i++) {
    int y = adjList[u][i];
    //Check if the node y is also adjacent to v
    for (int j = 0; j < (int) adjList[v].size(); j++) {
      if (adjList[v][j] == y) {
        newClique_.push_back(y);
      }
    }
  }

  /*printf("new clique\n");
  for (auto &x : newClique_) {
    printf("%d ", x);
  }  
  puts("");*/
}

void clq1(const vector<vi> &adjList, set<vi> &cliques) {
  set<ii> marked;
  map<int, vi> cliques_map;
  int clique_number = 0;
  
  for (int u = 0; u < (int) adjList.size(); u++) {
    for (int j = 0; j < (int) adjList[u].size(); j++) {
      int v = adjList[u][j];
      //printf("looking edge %d %d\n", u, v);
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

  /*printf("The cliques are as follows:\n");
  for (const auto &clique_ : cliques) {
    for (int i = 0; i < (int) clique_.size(); i++) {
      printf("%d ", clique_[i]);
    }
    puts("");
  }*/
}

void runOptimization(vector<vi> &adj, set<vi> &cliques) {
  int nVertex = (int) adj.size();
  int nvars = 0;

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
    GRBLinExpr obj;
    ostringstream cname_clique;
    cname_clique << "constr_clique_" << idx++;
    for (int i = 0; i < (int) clique_.size(); i++) {
      obj += clique_[i];
    }
    model.addConstr(obj <= 1.0, cname_clique.str());
  }
  
  //------------ Adding the objective.
  GRBLinExpr obj = 0.0;
  for (int var = 0; var < nvars; var++) {
    obj += vars[var];
  }

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
  return 0;
}
