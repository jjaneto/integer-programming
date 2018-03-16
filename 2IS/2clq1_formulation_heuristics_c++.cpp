#include <assert.h>
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

vector<set<int> > graph1;
set<vi> cliques_main;
int n, m;
FILE *error_out;

class mycallback: public GRBCallback {
public:
  double lastiter;
  double lastnode;
  int numvars;
  GRBVar* vars;
  ofstream* logfile;
  vector<set<int> > adjList;
  int nVertex;
  mycallback(int xnumvars, GRBVar* xvars, ofstream* xlogfile, vector<set<int> > adjList) {
    lastiter = lastnode = -GRB_INFINITY;
    numvars = xnumvars;
    vars = xvars;
    logfile = xlogfile;
    this->adjList = adjList;
    nVertex = (int) numvars / 2;
  }
protected:
  void callback () {
    try {
      if (where == GRB_CB_MIPNODE) {
        if (getIntInfo(GRB_CB_MIPNODE_STATUS) == GRB_OPTIMAL) {
          double* x = getNodeRel(vars, numvars);
          double* y = getNodeRel(vars, numvars); 
          RND1(x);//, RND2(y); //Apply the heuristics
          int sum1 = 0, sum2 = 0;
          for (int i = 0; i < numvars; i++) {
            sum1 += ((int) x[i]);
            sum2 += ((int) y[i]);
          }
          if (sum1 > sum2) {
            setSolution(vars, x, numvars);
          } else setSolution(vars, y, numvars);
          delete[] x;
          delete[] y;
        }
      }
    } catch (GRBException e) {
      cout << "Error number: " << e.getErrorCode() << endl;
      cout << e.getMessage() << endl;
    }    
  }

private:
  bool isEveryoneIsZeroOrOne(double *variables) {
    for (int i = 0; i < numvars; i++) {
      if (variables[i] != 0.0 && variables[i] != 1.0)
        return false;
    }
    return true;
  }
  //-----------------------Heuristics----------------------------
  //Set to 1.0 the selected variable and to 0.0 all its adjacents (for each MIS)
  void RND1(double *variables, int l, int r) {
    cout << "rnd1 " << l << " " << r << '\n';
    puts("The variables are:");
    for (int i = l; i < r; i++) {
      printf("%.2lf ", variables[i]);
    }
    puts("");
    double value = numeric_limits<double>::lowest();
    int idx = -100;
    int factor = (r == numvars / 2 ? r : -(r/2));
    for (int i = l; i < r; i++) {
      if (variables[i] < 1.0 && variables[i] > value) {
        if (variables[i] > variables[i + factor]) {
          value = variables[i];
          idx = i;
        }
      }
    }

    if (idx == -100) {
      return;
    }

    //printf("idx %d factor %d\n", idx, factor);
    assert((idx + factor >= 0 && idx + factor < numvars));
    variables[idx] = 1.0;
    variables[idx + factor] = 0.0;

    if (factor == (numvars / 2)) { //Se estou no primeiro conjunto
      for (const auto &v : adjList[idx]) {
        variables[v] = 0.0;
      }
    } else {
      for (const auto &v : adjList[idx + factor]) { //Se estou no segundo conjunto
        variables[v - factor] = 0.0;
      }
    }
  }
  void RND1(double *vars) {
    double variables[numvars];
    puts("all variables");
    for (int i = 0; i < numvars; i++) {
      variables[i] = vars[i];
      printf("%.2lf ", vars[i]);
    }
    puts("");
    while (!isEveryoneIsZeroOrOne(variables)) {
      RND1(variables, 0, numvars / 2);
      RND1(variables, numvars / 2, numvars);
    }
  }
  
  void RND2(double *variables, int l, int r) {
    cout << "rnd2 " << l << " " << r << '\n';
    double value = numeric_limits<double>::max();
    int idx = -100;
    int factor = (r == numvars / 2 ? r : -(r/2));
    puts("The variables are:");
    for (int i = l; i < r; i++) {
      printf("%.2lf ", variables[i]);
    }
    puts("");
    for (int i = l; i < r; i++) {
      if (variables[i] > 0.0 && variables[i] < 1.0) {
        //printf("oi!\n");
        if (variables[i] < variables[i + factor]) {
          //printf("KKKKK \n");
          double sum = variables[i];
          if (factor == (numvars / 2)) {
            for (const auto &v : adjList[i]) {
              sum += variables[v];
            }
          } else {
            for (const auto &v : adjList[i + factor]) {
              sum += variables[v - factor];
            }
          }

          //printf("SUM %.2lf\n", sum);
          if (sum < value) {
            exit(10);
            value = sum;
            idx = i;
          }
        }
      }

      printf("%d %d\n", idx, factor);
      if (idx == -100)
        return;

      assert(idx >= 0 && (idx + factor >= 0 && idx + factor < numvars));
      variables[idx] = 1.0;
      variables[idx + factor] = 0.0;
      if (factor == (numvars / 2)) { //Se estou no primeiro conjunto
        for (const auto &v : adjList[idx]) {
          variables[v] = 0.0;
        }
      } else {
        for (const auto &v : adjList[idx + factor]) { //Se estou no segundo conjunto
          variables[v - factor] = 0.0;
        }
      }
    }
  }
  
  void RND2(double *vars) {
    double variables[numvars];
    puts("all variables");
    for (int i = 0; i < numvars; i++) {
      variables[i] = vars[i];
      printf("%.2lf ", vars[i]);
    }
    puts("");
    while (!isEveryoneIsZeroOrOne(variables)) {
      RND2(variables, 0, numvars / 2);
      RND2(variables, numvars / 2, numvars);
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


void readGraph(char *name, vector<set<int> > &adjList) {
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

  adjList.assign(n, set<int>());

  int szName = strlen(name);
  bool clq = (name[szName - 1] == 'q' && name[szName - 2] == 'l' && name[szName - 3] == 'c');

  for (int i = 0; i < m; i += 1) {
    int u, v; char c;

    if (clq) {
      //scanf("%c %d %d\n", &c, &u, &v);
      cin >> c >> u >> v;
    } else{
      //scanf("%d %d\n", &u, &v);
      cin >> u >> v;
    }

    //printf("%c %d %d\n", c, u, v);
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
  
  /*printf("new clique\n");
    for (auto &x : newClique_) {
    printf("%d ", x + 1);
    }  
    puts("");*/
}

void clq1(const vector<set<int> > &adjList, set<vi> &cliques) {
  set<ii> marked;
  //map<int, vi> cliques_map;
  int clique_number = 0;
  
  for (int u = 0; u < (int) adjList.size(); u++) {
    for (const auto &v : adjList[u]) {
      if ((marked.find(ii(u, v)) == marked.end()) && (marked.find(ii(v, u)) == marked.end())) {
        vi newClique_;
        expandClique(ii(u, v), adjList, newClique_);
        cliques.insert(newClique_);
        
        //cliques_map[clique_number++] = newClique_;
        
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
    printf("%d ", clique_[i] + 1);
    }
    puts("");
    }*/
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

  //------- Callback
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
    runOptimization(graph1, cliques_main);
  } catch (GRBException ex) {
    cout << "Error code = " << ex.getErrorCode() << endl;
    cout << ex.getMessage() << endl;
  } catch (...) {
    cout << "fora temer" << endl;
  }
  
  return 0;
}
