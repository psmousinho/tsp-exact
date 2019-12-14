# Travelling Salesman Problem:
Resolução do TSP de forma exata utilizando o seguint modelo:

<img src="https://i.imgur.com/IyLIeUq.png" title="Model" />


## Execução:
`python tspStages.py [-rd] {arquivo de entrada}`

Onde:

-rd: Salva a saida do programa de forma tabular em markdown no arquivo README.md

O arquivo de entrada é qualquer instancia paro o problemas tsp da tsplib: http://elib.zib.de/pub/mp-testdata/tsp/tsplib/tsplib.html.

## Dependências:
- Python
- Cplex

## Resultados:
Os valores otimos de custo para as instâncias podem ser encontrados em:  http://elib.zib.de/pub/mp-testdata/tsp/tsplib/stsp-sol.html.
Os resultados encontrados por essa implementação podem ser encontrados a seguir:

|Instância|Status da Solucao|Custo da Solucao|Duracao(seg)|
|---------|-----------------|----------------|------------|
|../instances/burma14.tsp|MIP_optimal|3323.000000000014|1.9648902416229248|
|../instances/ulysses16.tsp|MIP_optimal|6858.999999999987|10.700449705123901|
|../instances/gr17.tsp|MIP_optimal|2084.999999999999|5.252423524856567|
|../instances/gr21.tsp|MIP_optimal|2706.999999999992|1.9691967964172363|
|../instances/ulysses22.tsp|MIP_optimal|7012.999999993599|46.9362850189209|
|../instances/gr24.tsp|MIP_optimal|1272.0000000000418|125.89890050888062|
|../instances/fri26.tsp|MIP_optimal|937.0000000000975|23.447660207748413|
|../instances/bayg29.tsp|MIP_optimal|1609.9999999999927|529.0106751918793|
|../instances/bays29.tsp|MIP_optimal|2020.0000000000557|307.5237355232239|
|../instances/bayg29.tsp|MIP_optimal|1609.9999999999927|526.47509932518|
|../instances/dantzig42.tsp|MIP_time_limit_feasible|833.9999999999993|1800.0358979701996|
|../instances/swiss42.tsp|MIP_time_limit_feasible|1636.000000000021|1800.0329387187958|
|../instances/att48.tsp|MIP_time_limit_feasible|17393.0|1800.3833413124084|
|../instances/gr48.tsp|MIP_time_limit_feasible|7792.000000000001|1800.0471370220184|
|../instances/hk48.tsp|MIP_time_limit_feasible|15252.999999999996|1800.0396497249603|
|../instances/eil51.tsp|MIP_time_limit_feasible|535.0000000001205|1800.0541179180145|
