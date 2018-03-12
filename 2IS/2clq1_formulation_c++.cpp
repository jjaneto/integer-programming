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

  for (int u = 0; u < (int) adjList.size(); u++) {
    for (int j = 0; j < (int) adjList[u].size(); j++) {
      int v = adjList[u][j];
      //printf("looking edge %d %d\n", u, v);
      if ((marked.find(ii(u, v)) == marked.end()) && (marked.find(ii(v, u)) == marked.end())) {
        vi newClique_;
        expandClique(ii(u, v), adjList, newClique_);
        cliques.insert(newClique_);

        int cliqueSize = (int) newClique_.size();
        for (int i = 0; i < cliqueSize; i++) {
          for (int j = i + 1; j < cliqueSize; j++) {
            int x = newClique_[i], y = newClique_[j];
            //printf("_______ marking %d %d\n", x, y);
            marked.insert(ii(newClique_[i], newClique_[j]));
          }
        }        
      }
    }
  }

  printf("The cliques are as follows:\n");
  for (const auto &clique_ : cliques) {
    for (int i = 0; i < (int) clique_.size(); i++) {
      printf("%d ", clique_[i]);
    }
    puts("");
  }
}

bool eachNodeHasZeroWeight(vi &nodes) {
  for (int i = 0; i < (int) nodes.size(); i++) 
    if (nodes[i] != 0)
      return false;
  
  return true;
}

//TODO: Maybe it is better if I separate DUALH and PRIMALH in two methods. Check this later.
void heuristics(const vector<vi> &adjList, const set<vi> &cliques, vi &hardness, vi &clique_value) {
  // --------- DUALH ----------
  vi node_weight(n, NODE_WEIGHT);
  vi y_c((int) cliques.size(), 0);
  while (!eachNodeHasZeroWeight(node_weight)) {
    vi node_appearence(n, 0);
    hardness.assign(n, 0);
    clique_value.assign((int) cliques.size(), 0);
  
    //Compute the number of times that node v appears in some clique
    for (const auto &clique_ : cliques) {
      for (int i = 0; i < (int) clique_.size(); i++) {
        printf("node appears %d\n", clique_[i]);
        node_appearence[clique_[i]] += 1;
      }
    }
  
    //Then, define the hardness of node v
    int cardinality = (int) cliques.size();
    for (int i = 0; i < (int) adjList.size(); i++) {
      hardness[i] = (cardinality - node_appearence[i]) * NODE_WEIGHT; //FIXME: (node_weight value).
    }

    /*printf("The hardness of the nodes are:\n");
    for (int i = 0; i < n; i++) {
      printf("%d ", hardness[i]);
    }
    puts("");*/

    //And at last, define the value of the clique
    int idx = 0;
    for (const vi &clique_ : cliques) {
      for (int v : clique_) {
        clique_value[idx] += hardness[v];
      }
      idx += 1;
    }

    int maxCliqueValue = getMaxCliqueValue();

    idx = 0;
    for (const auto &clique_ : cliques) {
      int minElement = numeric_limits<int>::max();
      for (const auto &vertex : clique_) {
        minElement = min(minElement, node_weight[vertex]);
      }

      for (const auto &vertex : clique_) {
        node_weight[vertex] = max(0, node_weight[vertex] - minElement);
      }

      y_c[idx] += minElement;
    }
  }
  // End of DUALH.
  
  // --------- PRIMALH ----------
  
}

void runOptimization() {
  int nVars = 0;

  GRBEnv env = GRBEnv();
  GRBModel model = GRBModel(env);
  GRBVar vars[nVars];

  //------------ Adding the variables to the model.

  //------------ adding the constraints to the model.

  //------------ Adding the objective.
}

int main(int argc, char **argv) {

  if (argc < 2) {
    printf("Missing arguments...\n");
    printf("USAGE: ./2mis_ordinary_formulation_c++ 'pathOfInstance'\n");
    exit(EXIT_FAILURE);
  }

  readGraph(argv[1], graph1);
  clq1(graph1, cliques_main);
  processCliques(graph1, cliques_main, hardness_main, value_main);
  /*try {
    readGraph(argv[1], graph1);
    clq1(graph1, cliques_main);
    } catch (GRBException ex) {
    cout << "Error code = " << ex.getErrorCode() << endl;
    cout << ex.getMessage() << endl;
    } catch (...) {
    cout << "fora temer" << endl;
    }*/
  
  return 0;
}
