#include <deque>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <vector>
#include <map>
#include <cmath>
#include <set>
//#include "gurobi_c++.h"

using namespace std;

typedef vector<int> vi;
typedef pair<int, int> ii;

const int NODE_WEIGHT = 1;

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

void clq1(const vector<vi> &adjList, set<vi> cliques) {
  set<ii> edges;
  for (int u = 0; u < (int) adjList.size(); u++) {
    for (int j = 0; j < (int) adjList[u].size(); j++) {
      int v = adjList[u][j];
      
      if (edges.find(ii(v, u)) == edges.end()) //Watching for reverse edges.
        edges.insert(ii(u, v));
      
    }
  }

  while (!edges.empty()) {
    const auto &it = edges.begin(); //TODO: Search on the internet why use auto instead ii.
    ii p = make_pair(it->first, it->second);
    //Now I will try to insert this edge in some clique. //TODO: It can be in any clique?
    bool insert = false;
    for (const vi &clique : cliques) {
      if (adjacentToAll(clique, adjList, p)) {
        edges.erase(edges.begin());
        insert = true;
        break;
      }
    }

    if (!insert) { //I could not insert this edge in any clique. Create a new one.
      vi newclique_;
      newclique_.push_back(p.first);
      newclique_.push_back(p.second);
      cliques.insert(newclique_);
    }
  }
}

void processCliques(const vector<vi> &adjList, const set<vi> &cliques, vi &hardness, vi &value) {
  vi node_appearence((int) adjList.size(), 0);
  //Compute the number of times that node v appears in some clique
  for (const auto &clique_ : cliques) {
    for (int i = 0; i < (int) clique_.size(); i++) {
      node_appearence[clique_[i]] += 1;
    }
  }
  
  //Then, define the hardness of node v
  int cardinality = (int) cliques.size();
  for (int i = 0; i < (int) adjList.size(); i++) {
    hardness[i] = (cardinality - node_appearence[i]) * NODE_WEIGHT;
  }

  //And at last, define the value of the clique
  int idx = 0;
  for (const vi &clique_ : cliques) {
    for (int v : clique_) {
      value[idx] += hardness[v];
    }
    idx += 1;
  }
}

int main(/*int argc, char **argv*/) {
  
  return 0;
}
