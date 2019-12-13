#include "include/readData.h"
#include <iostream>
#include <vector>
#include <ilcplex/ilocplex.h>

int dimension;
double** distances;

void solveMTZ();
void solveFlux();
void solveStages();
void solveExponential();

int main(int argc, char** argv) {
    if(argc != 3) {
        printf("Usage:\n./bin instance [mtz||flx||stg||exp]\n");
        return 0;
    }

    readData(argv[1], &dimension, &distances);

    for(int i = 0; i < dimension; i++) {
        for(int j = 0; j < dimension; j++) {
            distances[i][j] = distances[i+1][j+1];
        }
    }

    if(argv[2] == "mtz") {
        solveMTZ();
    } else if(argv[2] == "flx") {
        solveFlux();
    } else if(argv[2] == "stg") {
        solveStages();
    }else if(argv[2] == "exp"){
        solveExponential();
    } else {
        std::cout << "Undefine model!!";
    }
    
    return 0;
}

void solveMTZ() {
    
    IloEnv env;
    IloModel modelo(env);

    ///////////////////////////////////////////////////////
    //Variaveis

    //variavel Xij: Xij = 1, se o arco ligando os vertices 
    //i e j esta sendo utilizado
    IloArray< IloBoolVarArray > x(env, dimension);
    for(int i = 0; i < dimension; i++) {
        IloBoolVarArray vetor(env, dimension);
        x[i] = vetor;
    }

    //Variavel Uj: 0 <= Uj <= dimension, onde Uj e a posicao 
    //do vertice j na solucao
    IloNumVarArray u(env, dimension, 0, dimension);

    //adiciona as variaveis U e X ao modelo
    for(int i = 0; i <  dimension; i++) {

        modelo.add(u[i]);

        for(int j = 0; j < dimension; j++) {
            if(j == i)
                continue;
            char name[100];
            sprintf(name, "X(%d,%d)", i, j);
            x[i][j].setName(name);
            modelo.add(x[i][j]);
        }
    }
    //fim das variaveis
    ///////////////////////////////////////////////////////
    
    
    ////////////////////////////////////////////////////////
    //Criando a Função Objetivo (FO) 
    IloExpr sumX(env);
    for(int i = 0; i < dimension; i++) {

        for(int j = 0; j < dimension; j++) {
            if(j == i)
                continue;
            sumX += distances[i][j] * x[i][j];
        }
    }
    // Adicionando a FO
    modelo.add(IloMinimize(env, sumX));
    //////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////
    //Restricoes
    
    //Restricoes grau de entrada
    for(int i = 0; i < dimension; i++) {
        IloExpr sumJI(env);
        for(int j = 0; j < dimension; j++) {
            if(j == i)
                continue;
            sumJI += x[j][i];
        }

        IloRange r = (sumJI == 1);
        char name[100];
        sprintf(name, "GRAU_IN(%d)", i);
        r.setName(name);
        modelo.add(r);
    }
    
    //Restricoes grau de saida
    for(int i = 0; i < dimension; i++) {
        IloExpr sumIJ(env);
        for(int j = 0; j < dimension; j++) {
            if(j == i)
                continue;
            sumIJ += x[i][j];
        }

        IloRange r = (sumIJ == 1);
        char name[100];
        sprintf(name, "GRAU_OUT(%d)", i);
        r.setName(name);
        modelo.add(r);
    }

    //Restricao Vertice Inicial
    IloRange r = (u[0] == 1);
    r.setName("VI");
    modelo.add(r);

    //Restricoes de Posicoes Sequenciais
    for(int i = 0; i < dimension; i++) {
        for(int j = 0; j < dimension; j++) {
            if(j == i || j == 1)
                continue;
            
            IloExpr sum(env);
            sum += u[i] - u[j] + dimension * x[i][j];
            IloRange r = (sum <= dimension - 1);
            char name[100];
            sprintf(name, "SEQ(%d,%d)", i , j);
            r.setName(name);
            modelo.add(r);
        }
    }

    //fim das restricoes
    ////////////////////////////////////////////////////////

    //resolve o modelo
    IloCplex MTZ(modelo);
    MTZ.setParam(IloCplex::TiLim, 2*60*60);
    MTZ.setParam(IloCplex::Threads, 1);
    MTZ.exportModel("modelo.lp");

    try {
        MTZ.solve();
        std::cout << std::endl << "status: " << MTZ.getStatus() << std::endl;
        std::cout << "custo: " << MTZ.getObjValue() << std::endl;
        
        int count = 1;
        int atual = 0;
        std::cout << "{" << atual+1 << ", ";
        while(count < dimension) {
            for(int j = 0 ; j < dimension; j++) {
                if(j == atual)
                    continue;
                if(MTZ.getValue(x[atual][j]) > 0.9) {
                    std::cout << j + 1 << ", ";
                    atual = j;
                    count++;
                }
            }
        }
        std::cout << "}" << std::endl;
        
    }
    catch(IloException& e) {
        std::cout << e;
    }

}

void solveFlux() {
    
    IloEnv env;
    IloModel modelo(env);

    ///////////////////////////////////////////////////////
    
    //variavel Xij: Xij = 1, se o arco ligando os vertices 
    //i e j esta sendo utilizado
    IloArray< IloBoolVarArray > x(env, dimension);
    //Variavel Fij: Fij >= 0, Fluxo que passa no arco que
    //liga os vertices i e j
    IloArray< IloNumVarArray > f(env, dimension);
    for(int i = 0; i < dimension; i++) {
        IloBoolVarArray vetorX(env, dimension);
        x[i] = vetorX;
        IloNumVarArray vetorF(env,dimension, 0 , IloInfinity, ILOFLOAT);
        f[i] = vetorF;
    }

    

    //adiciona as variaveis F e X ao modelo
    for(int i = 0; i <  dimension; i++) {
        for(int j = 0; j < dimension; j++) {
            if(j == i)
                continue;
            
            char nameX[100];
            sprintf(nameX, "X(%d,%d)", i, j);
            x[i][j].setName(nameX);
            modelo.add(x[i][j]);

            char nameF[100];
            sprintf(nameF, "F(%d,%d)", i, j);
            f[i][j].setName(nameF);
            modelo.add(f[i][j]);
        }
    }
    //fim das variaveis
    ///////////////////////////////////////////////////////
    
    
    ////////////////////////////////////////////////////////
    //Criando a Função Objetivo (FO) 
    IloExpr sumX(env);
    for(int i = 0; i < dimension; i++) {

        for(int j = 0; j < dimension; j++) {
            if(j == i)
                continue;
            sumX += distances[i][j] * x[i][j];
        }
    }
    // Adicionando a FO
    modelo.add(IloMinimize(env, sumX));
    //////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////
    //Restricoes
    
    //Restricoes grau de entrada
    for(int i = 0; i < dimension; i++) {
        IloExpr sumJI(env);
        for(int j = 0; j < dimension; j++) {
            if(j == i)
                continue;
            sumJI += x[j][i];
        }

        IloRange r = (sumJI == 1);
        char name[100];
        sprintf(name, "GRAU_IN(%d)", i);
        r.setName(name);
        modelo.add(r);
    }
    
    //Restricoes grau de saida
    for(int i = 0; i < dimension; i++) {
        IloExpr sumIJ(env);
        for(int j = 0; j < dimension; j++) {
            if(j == i)
                continue;
            sumIJ += x[i][j];
        }

        IloRange r = (sumIJ == 1);
        char name[100];
        sprintf(name, "GRAU_OUT(%d)", i);
        r.setName(name);
        modelo.add(r);
    }

    //Restricao de fluxo do vertice inicial
    IloExpr sumF1J(env);
    for(int j = 1; j < dimension; j++) {
        sumF1J += f[0][j]; 
    }
    IloRange r = (sumF1J == dimension-1);
    r.setName("VI");
    modelo.add(r);

    //Restricoes de fluxo de vertices nao utilizados
    for(int i = 0; i < dimension; i++) {
        for(int j = 0; j < dimension; j++) {
            if(j == i)
                continue;
            
            IloConstraint r = (f[i][j] <= ( (dimension-1) * x[i][j] ) );
            char name[100];
            sprintf(name, "F(%d,%d)", i , j);
            r.setName(name);
            modelo.add(r);
        }
    }

    //Restricoes de conservacao de fluxo
    for(int i = 1; i < dimension; i++) {
        IloExpr sum(env);
        for(int j = 0; j < dimension; j++) {
            if(j == i)
                continue;

            sum += f[j][i] - f[i][j]; 
        }
        IloRange r = (sum == 1 );
        char name[100];
        sprintf(name, "CON(%d)", i);
        r.setName(name);
        modelo.add(r);
    }

    //fim das restricoes
    ////////////////////////////////////////////////////////

    //resolve o modelo
    IloCplex flux(modelo);
    flux.setParam(IloCplex::TiLim, 2*60*60);
    flux.setParam(IloCplex::Threads, 1);
    flux.exportModel("modelo.lp");

    try {
        flux.solve();
        std::cout << std::endl << "status: " << flux.getStatus() << std::endl;
        std::cout << "custo: " << flux.getObjValue() << std::endl;
        
        int count = 1;
        int atual = 0;
        std::cout << "{" << atual+1 << ", ";
        while(count < dimension) {
            for(int j = 0 ; j < dimension; j++) {
                if(j == atual)
                    continue;
                if(flux.getValue(x[atual][j]) > 0.9) {
                    std::cout << j + 1 << ", ";
                    atual = j;
                    count++;
                    break;
                }
            }
        }
        std::cout << "}" << std::endl;
        
    }
    catch(IloException& e)
    {
        std::cout << e;
    }

}

void solveStages() {
    
    IloEnv env;
    IloModel modelo(env);

    ///////////////////////////////////////////////////////
    
    //variavel Xijt: Xijt = 1, se o arco ligando os vertices 
    //i e j esta sendo utilizado no estagio t
    IloArray< IloArray < IloBoolVarArray > > x(env, dimension);
    for(int i = 0; i < dimension; i++) {
        IloArray<IloBoolVarArray> y(env,dimension);
        x[i] = y;

        for(int j = 0; j < dimension; j++) {
            IloBoolVarArray z(env,dimension);
            x[i][j] = z;
        }
    }


    //adiciona a varaivel X ao modelo
    for(int i = 0; i <  dimension; i++) {
        for(int j = 0; j < dimension; j++) {
            if(j == i)
                continue;
            for(int t =0; t < dimension; t++) {
                char name[100];
                sprintf(name, "X(%d,%d,%d)", i, j, t);
                x[i][j][t].setName(name);
                modelo.add(x[i][j][t]);    
            }
        }
    }
    //fim das variaveis
    ///////////////////////////////////////////////////////
    
    
    ////////////////////////////////////////////////////////
    //Criando a Função Objetivo (FO) 
    IloExpr sumX(env);
    for(int i = 0; i < dimension; i++) {
        for(int j = 0; j < dimension; j++) {
            if(j == i)
                continue;
            for(int t = 0; t < dimension; t++){
                sumX += distances[i][j] * x[i][j][t];
            }
            
        }
    }
    // Adicionando a FO
    modelo.add(IloMinimize(env, sumX));
    //////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////
    //Restricoes
    
    //Restricoes grau de entrada
    for(int i = 0; i < dimension; i++) {
        IloExpr sumJI(env);
        for(int j = 0; j < dimension; j++) {
            if(j == i)
                continue;
            for(int t = 0; t < dimension; t++) {
                sumJI += x[j][i][t];
            }
        }

        IloRange r = (sumJI == 1);
        char name[100];
        sprintf(name, "GRAU_IN(%d)", i);
        r.setName(name);
        modelo.add(r);
    }
    
    //Restricoes grau de saida
    for(int i = 0; i < dimension; i++) {
        IloExpr sumIJ(env);
        for(int j = 0; j < dimension; j++) {
            if(j == i)
                continue;
            for(int t = 0; t < dimension; t++) {
                sumIJ += x[i][j][t];
            }
        }

        IloRange r = (sumIJ == 1);
        char name[100];
        sprintf(name, "GRAU_OUT(%d)", i);
        r.setName(name);
        modelo.add(r);
    }

    //Restricoes de exclusividade do estagio
    for(int t = 0; t < dimension; t++) {
        IloExpr sumT(env);
        for(int i = 0; i < dimension; i++) {
            for(int j = 0; j < dimension; j++) {
                if(j == i)
                    continue;
                
                sumT += x[i][j][t];
            }
        }
        IloRange r = (sumT == 1);
        char name[100];
        sprintf(name, "EXC(%d)", t);
        r.setName(name);
        modelo.add(r);
    }

    //Restricao de sequencia de estagios
    for(int i = 1; i < dimension; i++) {
        IloExpr sum(env);
        for(int j = 0; j < dimension; j++) {
            if(j == i)
                continue;
            for(int t = 0; t < dimension; t++) {
                sum += t * x[i][j][t] - t * x[j][i][t];
            }
        }

        IloRange r = (sum == 1);
        char name[100];
        sprintf(name, "SEQ(%d)", i);
        r.setName(name);
        modelo.add(r);
    }

    //fim das restricoes
    ////////////////////////////////////////////////////////

    //resolve o modelo
    IloCplex stages(modelo);
    stages.setParam(IloCplex::TiLim, 2*60*60);
    stages.setParam(IloCplex::Threads, 1);
    stages.exportModel("modelo.lp");

    try {
        stages.solve();
        std::cout << std::endl << "status: " << stages.getStatus() << std::endl;
        std::cout << "custo: " << stages.getObjValue() << std::endl;
        
        int count = 1;
        int atual = 0;
        std::cout << "solucao: {";
        for(int t = 0; t < dimension; t++) {
            for(int i = 0; i < dimension; i++) {
                for(int j = 0; j < dimension; j++) {
                    if(i == j)
                        continue;
                    if(stages.getValue(x[i][j][t]) > 0.9) {
                        std::cout << i+1 << ", "; 
                    }
                }
            }
        }
        std::cout << "}" << std::endl;
        
    }
    catch(IloException& e)
    {
        std::cout << e;
    }

   
    

}

ILOLAZYCONSTRAINTCALLBACK2(CYCLELC_CONJCOMPL, IloArray<IloBoolVarArray> &, x, int, numVertices) {
    bool visited[numVertices] = {false};
    int atual = 0, count= 0;

    while(count < numVertices) {

        for(int i = 0; i < numVertices; i++) {
            if(!visited[i]) {
                atual = i;
                break;
            }
        }

        std::vector<int> cicle = {atual};

        while(!visited[atual]) {
            for(int i = 0; i < numVertices; i++) {
                if(i == atual) continue;
                if(getValue(x[atual][i]) >= 0.9) {
                    visited[atual] = true;
                    cicle.push_back(i);
                    atual = i;
                    break;
                }
            }
        }

        int size = cicle.size() -1;
        if(size < numVertices) {
            IloExpr exp(getEnv());
            
            for(int i = 0; i < size; i++) {
                for(int j = i+1; j < size;j++) {
                    exp += x[cicle[i]][cicle[j]] + x[cicle[j]][cicle[i]];
                }
            }
            add(exp <= size - 1);
        }

        count += size;
    }
}

void solveExponential() {
    IloEnv env;
    IloModel modelo(env);

    ///////////////////////////////////////////////////////
    //Variaveis

    //variavel Xij: Xij = 1, se o arco ligando os vertices 
    //i e j esta sendo utilizado
    IloArray< IloBoolVarArray > x(env, dimension);
    for(int i = 0; i < dimension; i++) {
        IloBoolVarArray vetor(env, dimension);
        x[i] = vetor;
    }

    //adiciona as variaveis X ao modelo
    for(int i = 0; i <  dimension; i++) {
        for(int j = 0; j < dimension; j++) {
            if(j == i)
                continue;
            char name[100];
            sprintf(name, "X(%d,%d)", i, j);
            x[i][j].setName(name);
            modelo.add(x[i][j]);
        }
    }
    //fim das variaveis
    ///////////////////////////////////////////////////////
    
    
    ////////////////////////////////////////////////////////
    //Criando a Função Objetivo (FO) 
    IloExpr sumX(env);
    for(int i = 0; i < dimension; i++) {

        for(int j = 0; j < dimension; j++) {
            if(j == i)
                continue;
            sumX += distances[i][j] * x[i][j];
        }
    }
    // Adicionando a FO
    modelo.add(IloMinimize(env, sumX));
    //////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////
    //Restricoes
    
    //Restricoes grau de entrada
    for(int i = 0; i < dimension; i++) {
        IloExpr sumJI(env);
        for(int j = 0; j < dimension; j++) {
            if(j == i)
                continue;
            sumJI += x[j][i];
        }

        IloRange r = (sumJI == 1);
        char name[100];
        sprintf(name, "GRAU_IN(%d)", i);
        r.setName(name);
        modelo.add(r);
    }
    
    //Restricoes grau de saida
    for(int i = 0; i < dimension; i++) {
        IloExpr sumIJ(env);
        for(int j = 0; j < dimension; j++) {
            if(j == i)
                continue;
            sumIJ += x[i][j];
        }

        IloRange r = (sumIJ == 1);
        char name[100];
        sprintf(name, "GRAU_OUT(%d)", i);
        r.setName(name);
        modelo.add(r);
    }

    //fim das restricoes
    ////////////////////////////////////////////////////////

    //resolve o modelo
    IloCplex exp(modelo);
    exp.use(CYCLELC_CONJCOMPL(env, x, dimension));
    exp.setParam(IloCplex::TiLim, 2*60*60);
    exp.setParam(IloCplex::Threads, 1);
    exp.exportModel("modelo.lp");

    try {
        exp.solve();
        std::cout << std::endl << "status: " << exp.getStatus() << std::endl;
        std::cout << "custo: " << exp.getObjValue() << std::endl;
        
        int count = 1;
        int atual = 0;
        std::cout << "{" << atual+1 << ", ";
        while(count < dimension) {
            for(int j = 0 ; j < dimension; j++) {
                if(j == atual)
                    continue;
                if(exp.getValue(x[atual][j]) > 0.9) {
                    std::cout << j + 1 << ", ";
                    atual = j;
                    count++;
                }
            }
        }
        std::cout << "}" << std::endl;
        
    }
    catch(IloException& e) {
        std::cout << e;
    }
}