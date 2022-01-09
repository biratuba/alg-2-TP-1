#include <iostream>
#include <vector>
#include <utility>
#include <queue>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <algorithm>
#include <random>
#include<tuple>

using namespace std;

struct point{
  vector<double> c;
  int classification;
};

class kdtree{
  private:
    //nos externos da arvore
    vector<point> external_node;
    //nos internos da arvore
    vector<double> internal_node;
    //numero de pontos desejados da querry
    int k;
    //ponto da querry
    point p;
    int dim;
    //guarda a distancia e o indice dos pontos a serem retornados deixando sempre o ponto mais distante no topo
    //poderia ter implementado como pair<double,ponto>, mas ai eu teria que implementar um comparador para esse tipo, o que seria complicado
    priority_queue<pair<double,int>> pq;

    //pode ser substituido por external_node[i].c[d] < external_node[j].c[d]
    //o resto do codigo serve para desempatar a a comparacao com as outras dimencoes, isso e para evitar o caso quadratico do quickselect
    bool comp(int i, int j, int d){
      for(int k = d; k < dim; k++){
        if(external_node[i].c[d] != external_node[j].c[d]){
          return external_node[i].c[d] < external_node[j].c[d];
        }
      }
      for(int k = 0; k < d; k++){
        if(external_node[i].c[d] != external_node[j].c[d]){
          return external_node[i].c[d] < external_node[j].c[d];
        }
      }
      return false;

    }

    //nao vou esplicar como que funciona(pois nao e o foco do trabalho e so uma otimizacao). Mas o quickselect coloca o ponto que estaria no indice m se estivesse ordenado em relação a dimencao d no indice m, ele faz isso em O(n) no caso medio e em O(n^2) no pior caso. O pior caso é muito improvavel ja que embaralhamos os pontos.
    void quickselect(int i, int j, int m, int d){
      if(i >= j)return;
      
      int x = j;
      int l = i;
      for(int r = i; r <= j-1; r++){
        //external_node[r].c[d] <= external_node[x].c[d]
        if(comp(r, x, d)){
          swap(external_node[l],external_node[r]);
          l++;
        }
      }
      swap(external_node[l],external_node[x]);
      swap(l,x);

      if(x > m) quickselect(i, x - 1, m, d);
      else if(x < m) quickselect(x + 1, j, m, d);

    }

    //cria a arvore kd, (i,j) funciona como indice/ponteiro do no atual, leia a documentação para mais detalhes.
    void build(int i, int j, int d){
      if(d == dim)d = 0;
      if(i >= j) return;
      int m = (i+j)/2;
      quickselect(i, j, m, d);
      internal_node[m] = external_node[m].c[d];

      build(i, m, d + 1);
      build(m + 1, j, d + 1);
    }

    //retorna o quadrato da distancia euclidiana, isso evita a utilização do sqrt que é uma funcao devagar.
    //o motivo que podemos fazer isso e por que para x em (0,+inf], x^2 e uma funcao crescente. Logo comparar x ou x^2 sempre da o mesmo resultado(exceto possivelmente por erros de arredondamento)
    double sq_d(int i){
      double dist = 0.0;
      for(int j = 0; j < dim; j++){
        dist += (pow(external_node[i].c[j] - p.c[j],2));
      }
      return (dist);
    }

    //coloca o ponto do indice i na fila se a fila ainda nao tem o tamanho desejado ou se ele esta mais proximo do que o ponto mais distante da fila
    void push(int i){
      pair<double,int> p = {sq_d(i),i};
      if(pq.size() < k){
        pq.push(p);
      }else if(p.first < pq.top().first){
        pq.pop();
        pq.push(p);
      }
    }

    //procura pelos pontos no no (i,j)
    void _querry(int i, int j, int d){
      if(d == dim)d = 0;

      if(i == j){
        push(i);
        return;
      }
      int m = (i+j)/2;
      
      pair<int,int> nb, ob;

      if(p.c[d] > internal_node[m]){ 
        nb = {m+1,j};
        ob = {i,m};
      }else{
        nb = {i,m};
        ob = {m+1,j};
      }

      _querry(nb.first, nb.second, d + 1);
      //testa se o outro galho pode ter pontos uteis, tem como fazer um teste mais preciso(condiderando multiplas dimencoes) mas se torna muito complicado para o TP(e eu nao acho que conseguiria fazer funcionar a tempo)
      if(pq.size() < k or pow(internal_node[m] - p.c[d],2) <= (pq.top().first)){
        _querry(ob.first, ob.second, d + 1);
      }
    }

  public:

    //incicializa a kdtree, definindoo numero de dimencoes e contruinsdo a arvore
    void initialize(int _dim,vector<point> training){
      dim = _dim;
      external_node = training;
      //controi a arvore kd
      internal_node.resize(external_node.size()-1);
      build(0,external_node.size()-1,0);
    }

    //retorna os k pontos mais proximos de p
    vector<point> querry(int _k, point _p){
      //ao inves de passar p e k para toda chamada recursiva, guardamos em variaveis da classe onde as funcoes tem acesso
      p = _p;
      k = _k;
      //resetamos a fila(da minha analise do codigo sempre chegamos nesse ponto com ela vazia, mas estou previnindo de ocorrer mudancas), fora isso, por algum motivo ela nao tem funcao .clear()
      pq = priority_queue<pair<double,int>>();
      //considerei fazer um d como variavel da classe ao inves de passar como parametro, mas cheguei a conclusao que deixaria o codigo confuso pois precisamos modificar ele durante a recursao

      _querry(0, external_node.size()-1, 0);

      vector<point> r;
      while(!pq.empty()){
        r.push_back(external_node[pq.top().second]);
        pq.pop();
      }

      //revertemos para deixar em ordem crescente de distancia
      reverse(r.begin(),r.end());

      return r;
    } 

    //funcoes usadas para testar se a arvore foi criada corretamente e fazer um querry com brute force (se quiser pode descomentalas para testar)
    /*vector<point> brute_querry(int _k, point _p){
      p = _p;
      k = _k;
      _querry(0, external_node.size()-1, 0);

      for (int i = 0; i < internal_node.size(); i++){
        push(i);
      }

      vector<point> r;
      while(!pq.empty()){
        r.push_back(external_node[pq.top().second]);
        pq.pop();
      }
      reverse(r.begin(),r.end());
      return r;
    } 

    bool check(){
      return _check(0, external_node.size()-1, 0);
    }

    bool _check(int i, int j, int d){
      if(d == dim)d = 0;
      if(i == j){
        return true;
      }
      int m = (i+j)/2;
      for(int ii = i; ii <= m; ii++){
        if(external_node[ii].c[d] > internal_node[m]){
          cout << false;
        }
      }
      for(int ii = m+1; ii <= j; ii++){
        if(external_node[ii].c[d] < internal_node[m]){
          cout << false;
        }
      }
      
      return _check(i, m, d + 1) and _check(m + 1, j, d + 1);
    }*/
};


class kNearestNeigbour{
  private:
  
    //numero de classes que o ponto pode ter, utilizado para calcular as estatisticas
    int n_classes;

    kdtree kd;

  public:

    //define os pontos de treino e 
    void initialize(int _dim, int _n_classes, vector<point> training){
      kd.initialize(_dim, training);
      n_classes = _n_classes;
    }

    vector<pair<point,int>> classify(int x,vector<point> test){
    vector<pair<point,int>> ans;
      
      for(auto p: test){
        vector<point> closetpoints = kd.querry(x,p);
        vector<int> rep(n_classes,0);
        int M_class = 0;

        //como os pontos estao em ordem crescente de distancia, em caso de empate pegamos a classe com o ponto mais distante(na classe) mais proximo(em todos)
        for (auto p: closetpoints){
          rep[p.classification]++;
          if(rep[p.classification] > rep[M_class]){
            M_class = p.classification;
          }
        }
        ans.push_back({p,M_class});
      }
      return ans;
    }


    //calcula as metricas
    vector<tuple<double,double,double>> metrics(int x,vector<point> test){
      vector<pair<point,int>> classifications = classify(x, test);

      vector<vector<int>> confusion_matrix(n_classes,vector<int>(n_classes,0));
      for(auto &[p,c]: classifications){
        confusion_matrix[p.classification][c]++;
      }

      vector<tuple<double,double,double>> ans (n_classes); 

      int totalpredictions = 0;
      int correctpredictions = 0;
      for(int i = 0; i < n_classes; i++){
        correctpredictions += confusion_matrix[i][i];
        for(int j = 0; j < n_classes; j++){
          totalpredictions += confusion_matrix[i][j];
        }
      }
      double accuracy = (double)correctpredictions/(double)totalpredictions;

      
      for(int i = 0; i < n_classes; i++){
        get<0>(ans[i]) = accuracy;//acuracia
        int total_tests = 0;
        int totalpredictions = 0;
        for(int j = 0; j < n_classes; j++){
          total_tests += confusion_matrix[i][j];
          totalpredictions += confusion_matrix[j][i];
        }
        get<1>(ans[i]) = ((double)confusion_matrix[i][i])/((double)total_tests);//Revocacao(double)total_tests) << endl;
        get<2>(ans[i]) = ((double)confusion_matrix[i][i])/((double)totalpredictions);//Precisao[i])/((double)totalpredictions) << endl;
      }
      return ans;
    }

};

int main() {
  
  vector<string> arquivos = {"balance.dat","banana.dat","ecoli.dat","glass.dat","iris.dat","magic.dat","movement_libras.dat","phoneme.dat","pima.dat","ring.dat","segment.dat","spambase.dat","texture.dat","twonorm.dat","wine.dat","winequality-red.dat","winequality-white.dat"};

  auto rng = std::default_random_engine {};

  for(string arq: arquivos){
    kNearestNeigbour kNN;
    ifstream fin(arq);
    
    if (!fin.is_open())
    {
      cout << "arquivo " << arq << " nao encontrado" << endl;
      continue;  
    }
  
    cout << "teste = " << arq << endl;

    int dim, n_classes;

    fin >> dim >> n_classes;

    vector<point> points;

    point p;
    p.c.resize(dim);

    while(!fin.eof()) {
      for(int i = 0; i < dim; i++){
        fin >> p.c[i];
      }
      fin >> p.classification;
      points.push_back(p);
    }

    vector<tuple<double,double,double>> average_metrics(n_classes,{0,0,0});

    //rodando 10 vezes para pegar as estatisticas medias

    const int number_of_tests = 10;

    for(int i = 1; i <= number_of_tests; i++){
      const double trainingprecent = 0.7;
      std::shuffle(points.begin(), points.end(), rng);
      vector<point> training;
      training.clear();
      vector<point> test;
      test.clear();
      int j;
      for(j = 0; j < trainingprecent*points.size(); j++){
        training.push_back(points[j]);
      }
      for(; j < points.size(); j++){
        test.push_back(points[j]);
      }
      kNN.initialize(dim, n_classes, training);
      vector<tuple<double,double,double>> m = kNN.metrics(5, test);

      for(int j = 0; j < n_classes; j++){

        get<0>(average_metrics[j]) += get<0>(m[j]);
        get<1>(average_metrics[j]) += get<1>(m[j]);
        get<2>(average_metrics[j]) += get<2>(m[j]);

      }
      
    }

    
    cout << "  acuracia = " << 100.0*get<0>(average_metrics[0])/number_of_tests << '%' << endl;

    for(int j = 0; j < n_classes; j++){

        cout << "  classe = " << j << endl;

        cout << "   revocacao = " << 100.0*get<1>(average_metrics[j])/number_of_tests << '%' << endl;
        cout << "   precisao = " << 100.0*get<2>(average_metrics[j])/number_of_tests << '%' << endl;
    }
  
  }
} 