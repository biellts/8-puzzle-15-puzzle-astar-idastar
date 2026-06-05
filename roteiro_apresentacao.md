# Roteiro de Apresentação — 8-puzzle e 15-puzzle
## Tempo: 15 a 20 minutos | 14 slides

---

> **Como usar este roteiro:** cada seção corresponde a um slide. Leia o conceito antes de memorizar a fala. Se você entender O QUE está acontecendo, não precisa decorar nada — as palavras vêm naturalmente.

---

## Slide 1 — Capa

### O que está acontecendo aqui?
O **8-puzzle** é aquele quebra-cabeça deslizante de 3×3 com peças numeradas 1 a 8 e um espaço vazio. Você pode mover qualquer peça adjacente ao espaço para o lugar vazio. O objetivo é chegar da configuração embaralhada até a configuração final `[1,2,3 / 4,5,6 / 7,8,_]`.

O **15-puzzle** é a versão 4×4: 15 peças numeradas de 1 a 15 e um espaço vazio numa grade de 4 colunas por 4 linhas. A mecânica é idêntica — mover qualquer peça adjacente ao vazio — mas o objetivo é `[1,2,3,4 / 5,6,7,8 / 9,10,11,12 / 13,14,15,_]`. Por ser 4×4, o espaço de estados é astronomicamente maior: cerca de 10 trilhões de configurações possíveis, contra 181 mil do 8-puzzle. Isso torna o 15-puzzle computacionalmente muito mais difícil.

O trabalho consiste em implementar dois algoritmos de busca — **A\*** e **IDA\*** — que encontram a sequência de movimentos mais curta possível para resolver qualquer configuração desses puzzles.

**Fala:**
> "O nosso trabalho implementa dois algoritmos clássicos de busca em inteligência artificial — o A* e o IDA* — para resolver o 8-puzzle e o 15-puzzle. Dado qualquer estado inicial embaralhado, os algoritmos encontram o menor número possível de movimentos para chegar ao objetivo, e mostram cada passo do caminho."

---

## Slide 2 — Requisitos

### O que o professor pediu?
Três coisas principais:
1. **Modelar** o espaço de configurações como um grafo de estados
2. **Tabela hash** para verificar e armazenar estados visitados eficientemente
3. **A\* e IDA\*** para ambos os puzzles, mostrando tempo, nós expandidos e a solução passo a passo

**Fala:**
> "O enunciado tem três pedidos centrais. Primeiro: modelar cada configuração do tabuleiro como um nó de grafo, e cada movimento válido como uma aresta de custo 1. Segundo: usar tabela hash para verificar estados visitados — sem isso, a busca revisita estados e nunca termina. Terceiro: implementar A* e IDA*, que produzem a solução ótima e mostram cada movimento do caminho."

---

## Slide 3 — Espaço de Estados

### O que é um 'espaço de estados'?
Imagine todos os jeitos possíveis de arranjar as peças no tabuleiro. Cada arranjo é um **estado** — um nó no grafo. Quando você move uma peça, vai de um estado para outro — isso é uma **aresta**.

**Por que isso é um grafo?**
- Nós = configurações do tabuleiro
- Arestas = movimentos válidos (mover peça para o espaço vazio)
- Resolver o puzzle = encontrar o menor caminho entre dois nós (inicial → objetivo)

**Tamanhos do espaço de busca:**

O 8-puzzle tem 9 posições, então existem 9! = 362.880 formas de arranjar as peças. Mas só metade delas é alcançável — o grafo tem dois grupos separados de 181.440 estados que nunca se comunicam. O 15-puzzle tem 16 posições: 16! dividido por 2 dá cerca de 10 trilhões de estados alcançáveis.

**Por que só metade é alcançável?** Pense assim: se você pegar um 8-puzzle físico, tirar duas peças vizinhas e recolocar elas trocadas, o tabuleiro vai parecer quase normal — mas nunca vai ter solução, não importa quantos movimentos você faça. Isso acontece porque cada movimento válido no puzzle preserva uma propriedade matemática do arranjo. Quando você troca duas peças na mão, quebra essa propriedade. Os dois grupos — os que têm solução e os que não têm — nunca se misturam. Por isso, antes de começar qualquer busca, o código verifica se aquela configuração tem solução. Se não verificasse, o algoritmo tentaria encontrar um caminho que não existe — gastando tempo à toa, sem nunca terminar.

**Fala:**
> "Vamos entender o espaço do problema. Cada configuração possível do tabuleiro é um nó do grafo. Cada movimento — mover uma peça para o espaço vazio — é uma aresta de custo 1. Resolver o puzzle é encontrar o menor caminho entre o estado inicial e o objetivo.
>
> O 8-puzzle tem 181.440 estados alcançáveis — isso é tratável. O 15-puzzle tem cerca de 10 trilhões — isso torna o BFS inviável. Por isso precisamos de algoritmos mais inteligentes."

---

## Slide 4 — Estado como uint64_t + Tabela Hash

### O que está acontecendo aqui?


**Parte 1 — Como guardar o tabuleiro como número**

O algoritmo precisa comparar e guardar milhares de configurações do tabuleiro. Guardar cada configuração como uma lista de 9 ou 16 números seria lento e pesado.

A solução foi guardar o tabuleiro inteiro como **um único número de 64 bits**. Funciona assim:

Cada célula do tabuleiro tem um valor de 0 a 15 (0 = espaço vazio). Um número de 0 a 15 cabe em 4 bits. Então:
- 8-puzzle: 9 células × 4 bits = 36 bits → cabe num número de 64 bits
- 15-puzzle: 16 células × 4 bits = 64 bits → ocupa exatamente um número de 64 bits

Na prática: em vez de guardar o tabuleiro `[5, 6, 2, 7, 1, 8, 3, 4, 0]`, guardamos um único número como `0x43817265`. É a mesma informação, mas compacta e rápida de comparar.

---

**Parte 2 — Por que a tabela hash é indispensável**

Durante a busca, o algoritmo explora muitos caminhos diferentes — e vários desses caminhos chegam no mesmo estado do tabuleiro. Sem registrar quais estados já foram visitados, o algoritmo ficaria em loop: geraria o estado A, depois B, depois voltaria para A, depois B, infinitamente.

A tabela hash resolve isso: ela guarda cada estado já visitado. Antes de explorar um estado, o algoritmo consulta a tabela. Se já estiver lá, ignora. Se não estiver, processa e registra.

A chave da tabela é exatamente o número de 64 bits que representa o tabuleiro — isso torna a consulta instantânea (O(1)), independente de quantos estados já foram guardados.

**Fala:**
> "Para guardar e comparar os estados do tabuleiro de forma eficiente, empacotamos cada configuração num único número de 64 bits — 4 bits por célula. Com isso, dois estados são comparados com uma única operação do processador.
>
> A tabela hash guarda todos os estados já visitados. Toda vez que o algoritmo gera um novo estado, ele consulta a tabela: se já foi visitado, descarta; se não foi, processa. Sem isso, a busca entraria em loop e nunca terminaria."

---

## Slide 5 — A* — Busca pelo Menor f = g + h

### O que mostrar no slide
O slide tem o grafo SVG à esquerda e dois bullets à direita. O grafo é a peça principal — percorra-o de cima para baixo enquanto fala.

**Explicando o grafo (de cima para baixo):**

- **START** — estado inicial, `g = 0`, sem movimentos feitos ainda.
- Três filhos gerados a partir do START, cada um com seu `f`:
  - `f = 9` (cinza, esquerda) — descartado por agora
  - **`f = 7` (amarelo, centro)** — menor f → A* expande esse primeiro
  - `f = 11` (cinza, direita) — ignorado
- Do nó `f = 7`, dois filhos:
  - **GOAL** (estrela verde) — solução encontrada pelo caminho ótimo
  - `f = 10` (muito apagado) — não precisa mais expandir
- A linha **amarela** mostra o caminho ótimo: START → f=7 → ··· → GOAL
- Os **pontinhos** no meio da linha verde indicam que o caminho tem 6 passos (h=6) antes do GOAL — o grafo está comprimido por espaço

**A fórmula f = g + h — o que cada variável significa:**
- **g** = movimentos já feitos (custo real, cresce a cada passo)
- **h** = estimativa dos movimentos restantes (heurística — nunca superestima)
- **f = g + h** = estimativa do custo total do caminho

Exemplo do nó amarelo: `g = 1` (um movimento feito) + `h = 6` (estimativa de 6 restantes) = `f = 7`.

**Por que o A* garante a solução ótima?**
Quando o GOAL é retirado da fila, o `f` naquele momento é o custo real mínimo. Se existisse caminho mais curto, ele teria f menor e já teria sido expandido antes — contradição. Logo, o primeiro caminho encontrado é o mais curto.

**Os dois bullets à direita:**
1. Define cada variável: g (feitos) · h (estimativa) · f = g + h (custo total)
2. Explica a estratégia: nós com f alto ficam para depois — a fila de prioridade sempre entrega o menor f primeiro

**Fala:**
> "Aqui está o A* em ação. Saímos do START e geramos três candidatos — f=9, f=7 e f=11. O A* sempre escolhe o menor f, que é o 7, marcado em amarelo.
>
> O f é g mais h. O g é quantos movimentos já fizemos — aqui, 1. O h é a heurística, estimando quantos ainda faltam — aqui, 6. Logo f = 7.
>
> Ao expandir f=7, encontramos o objetivo pelo caminho ótimo. Os nós cinzas foram gerados mas ignorados — f maior significa caminho mais longo, então a fila não os escolheu. A garantia de otimalidade vem exatamente disso: quando o objetivo sai da fila, não há como existir um caminho mais curto que ainda não foi explorado."

---

## Slide 6 — A* vs IDA* — Memória ou Re-expansão?

### O que este slide mostra
O slide tem dois elementos: o pseudocódigo do IDA* no topo e uma tabela comparando os dois algoritmos linha por linha. Fale primeiro sobre o pseudocódigo e depois percorra a tabela de cima para baixo, destacando a linha **Memória** como a mais importante.

### Pseudocódigo — os 4 passos do IDA*
No topo do slide estão os 4 passos numerados:

- **① limite ← h(início)** — começa com o menor f possível. Antes de qualquer movimento, g=0 e f = h. Esse é o limite mínimo da primeira iteração.
- **② ↻ DFS, corta se f > limite** — faz busca em profundidade. Toda vez que um nó tiver f maior que o limite atual, descarta esse ramo e volta. Não guarda nada — vai fundo por um caminho antes de tentar outro.
- **③ se achou GOAL → retorna** — se dentro do limite a busca chega ao objetivo, a solução foi encontrada. O caminho está na pilha de recursão.
- **④ limite ← menor f cortado** — se não achou o objetivo em nenhum ramo, pega o menor f que foi cortado (o próximo valor "razoável") e usa como novo limite. Na próxima iteração, a busca vai um pouco mais fundo.

O processo repete até encontrar o objetivo.

### Linha por linha da tabela

**Algoritmo**: são abordagens opostas. A* usa fila de prioridade e sempre sabe qual nó expandir a seguir (o de menor f). IDA* usa DFS — vai até o fundo de um caminho antes de tentar outro.

**Memória** ← linha mais importante:
- A* guarda **todos os estados visitados** na hash com g e estado pai. Para o 15-puzzle com soluções de 50+ movimentos, isso pode ser centenas de milhares de entradas.
- IDA* não guarda estados fechados. Só usa a **pilha de recursão** — no máximo ~50 estados para o 15-puzzle.
- Essa é a diferença fundamental que explica todos os outros números da tabela.

**Re-expansões**: A* nunca revisita o mesmo estado (a hash impede). O IDA* re-expande nós entre iterações — na iteração com limite=42, ele revisita tudo que viu na iteração com limite=40. Na prática, esse custo extra é pequeno perto do ganho em memória.

**Limite de nós por instância**: são os tetos que o nosso código impõe. O IDA* tem um limite maior (2M vs 500K) porque cada nó custa muito menos memória.

**8-puzzle**: os dois resolvem 100% das 100 instâncias. O IDA* expande mais nós por instância (~1606 vs ~852) — re-expande entre iterações — mas os dois chegam lá.

**15-puzzle**: aqui a diferença aparece. A* resolve 25/100, IDA* resolve 30/100. O IDA* vai mais longe porque não esgota memória tão rápido. Os 70/75 restantes atingiram o limite de nós antes de encontrar solução.

**Garantia**: os dois garantem solução ótima com heurística admissível — mesma qualidade de resultado, estratégias opostas de uso de memória.

**Fala:**
> "O pseudocódigo no topo mostra como o IDA* funciona em 4 passos. Começa definindo o limite igual a h do estado inicial. Depois faz DFS, cortando qualquer ramo onde f ultrapassa esse limite. Se achar o objetivo, retorna. Se não achar, pega o menor f que foi cortado como novo limite — e repete. Cada iteração vai um pouco mais fundo.
>
> Agora a tabela. Eu quero chamar atenção para a linha Memória — essa é a diferença fundamental. O A* guarda todos os estados visitados na hash. Para o 15-puzzle isso pode ser centenas de milhares de entradas. O IDA* não guarda nada disso — só a pilha de recursão, uns 50 estados.
>
> No 8-puzzle os dois resolvem 100%. No 15-puzzle a diferença aparece: A* 25 de 100, IDA* 30 de 100. O IDA* vai mais longe porque não esgota memória. E os dois garantem solução ótima — mesma qualidade, estratégias opostas."

---

## Slide 7 — Heurísticas: Manhattan + Conflito Linear

### O que o slide mostra
O slide explica a **heurística h(s)** — a estimativa de movimentos restantes usada pelo A* e IDA*. Quanto mais precisa a estimativa, menos nós o algoritmo precisa explorar. Um h fraco gasta muito; um h forte economiza muito.

A fórmula na barra do topo:  
**h(s) = manhattan(s) + 2 × conflitos_lineares(s)**  
A etiqueta verde "admissível ✓" à direita confirma: essa soma nunca superestima.

### Card azul — Distância de Manhattan

**O que é:** para cada peça, conta quantas casas ela precisa se mover (linhas + colunas) até chegar na posição certa. É como a distância em quarteirões de cidade — sem diagonal, sem atalho.

**Fórmula:** `d = |linha_atual − linha_goal| + |col_atual − col_goal|`

**O tabuleiro do slide** mostra o estado **[2, 1, 3, 4, 5, 6, 7, 8, _]** com o goal **[1, 2, 3, 4, 5, 6, 7, 8, _]**. O tabuleiro está centralizado com 9 peças em grade 3×3, cada tile mostrando o número e o `d` abaixo:

- Peça **2** — está na coluna 0, precisa ir para coluna 1 → **d = 1** (borda e número em vermelho)
- Peça **1** — está na coluna 1, precisa ir para coluna 0 → **d = 1** (borda e número em vermelho)
- Peças **3 a 8** — já estão na posição certa → **d = 0** cada (cinza, "d = 0" em verde)
- Espaço vazio — canto inferior direito, não conta

Resultado: **h_manhattan = 1 + 1 + 0×6 = 2**

Por que é admissível? Cada peça, independente das outras, precisa de ao menos essa distância. Não há como chegar mais rápido — logo, nunca superestima.

### Card vermelho — Conflito Linear

**O problema do Manhattan sozinho:** ele ignora que peças na mesma linha podem se bloquear. Resultado: subestima mais do que precisaria.

**O Conflito Linear detecta** a situação onde movimentos extras são inevitáveis: se duas peças estão na **linha ou coluna correta** (exatamente onde pertencem no goal), mas em **ordem trocada**, elas não conseguem chegar ao objetivo sem se cruzar. Uma deve sair da linha, a outra passa, a primeira volta — mínimo **+2 movimentos** por par.

**O visual do conflito no slide:**

O card vermelho mostra os tiles **[2] [1] [3]** em linha com labels abaixo:
- Tile **2** → "→ pos 1" (quer ir para a direita)
- Tile **1** → "← pos 0" (quer ir para a esquerda)
- Tile **3** → "pos 2 ✓" (já está no lugar, não participa do conflito)

Entre os tiles 2 e 1 há um indicador visual de cruzamento: **→ ✕ ←** — as setas mostram que as duas peças querem passar pelo mesmo espaço em sentidos opostos. Uma linha vertical separa o tile 3, indicando que ele está fora do conflito.

Abaixo dos tiles, um badge vermelho: **"uma deve sair da linha → h += 2"**

**Por que ainda é admissível?** Os +2 movimentos são o mínimo necessário — é impossível resolver o cruzamento com menos. Portanto, nunca superestima.

### Resultado combinado (barra amarela)
Para o estado [2, 1, 3, 4, 5, 6, 7, 8, _]:
- **h_manhattan = 2** (peças 1 e 2 com d=1 cada)
- **1 par em conflito linear → +2**
- **h total = 4**

A barra mostra: `h = 2 (manhattan) + 2 (conflito) = 4`

Sem o Conflito Linear, o A* partiria de h=2 e exploraria muito mais nós antes de descartar esse ramo. Com h=4, o algoritmo sabe que esse caminho é custoso e prioriza outros — economizando expansões.

**Fala:**
> "A heurística tem dois componentes. O primeiro é Manhattan: para cada peça, somamos quantas linhas e colunas ela precisa percorrer. No tabuleiro do slide, peça 2 está na posição errada com distância 1, peça 1 igual — as outras já estão no lugar. h_manhattan = 2.
>
> Mas 2 subestima. Olha o card vermelho: a peça 2 quer ir pra direita e a peça 1 quer ir pra esquerda — elas se cruzam. O indicador no meio, seta pra direita, x, seta pra esquerda, mostra exatamente esse bloqueio. Uma precisa sair da linha para a outra passar. Esses 2 movimentos são inevitáveis — Conflito Linear: h += 2.
>
> No total: h sobe de 2 para 4. O A* vai preferir outros caminhos antes de explorar esse, economizando muito trabalho."

---

## Slide 8 — Solucionabilidade

### Por que verificar antes de buscar?
Metade das configurações aleatórias do puzzle são **insolúveis** — nunca alcançam o objetivo, independente de quantos movimentos você faça. Se não verificarmos, o algoritmo vai expandir todos os estados alcançáveis sem nunca encontrar o objetivo.

**O critério — paridade de inversões:**
Uma inversão é um par (i, j) onde i < j mas `tiles[i] > tiles[j]` (ignorando o vazio). Conta inversões no vetor linear das peças.

- **8-puzzle (grade 3×3, ímpar):** solucionável ↔ número de inversões é par
- **15-puzzle (grade 4×4, par):** solucionável ↔ (inversões + linha do vazio contada de baixo) é ímpar

**Por que funciona?** Cada movimento do puzzle altera a paridade de inversões de forma previsível. Estados com paridades diferentes nunca se comunicam — pertencem a componentes diferentes do grafo.

**Fala:**
> "Antes de iniciar qualquer busca, verificamos se a instância tem solução. Metade das configurações aleatórias são matematicamente insolúveis — o grafo tem dois componentes que nunca se comunicam.
>
> A verificação usa paridade de inversões: contamos pares onde uma peça com valor maior vem antes de uma com valor menor. Para o 8-puzzle, inversões devem ser par. Para o 15-puzzle, a paridade envolve também a linha do vazio. Isso é O(n²) e evita rodar a busca por minutos em vão."

---

## Slide 9 — Demonstração A* — Expansão Guiada por f

### O que o slide mostra
O slide é uma demonstração visual com estados reais do 8-puzzle. Mostra o estado inicial, dois vizinhos gerados em g=1, reticências (g=2, 3, …) e o estado objetivo em g=25. No final, um bullet sobre a hash.

**Os tiles do slide:**

- **Estado inicial** (`g=0`): `[1, _, 7 / 5, 2, 4 / 3, 8, 6]` com h=15. O vazio está na posição (0,1).
- **Vizinho A** (`g=1`): vazio move para a esquerda, peça 1 move para a direita. Tile 1 fica em destaque (laranja/moved).
- **Vizinho B** (`g=1`): vazio move para baixo, peça 2 sobe. Tile 2 fica em destaque.
- **⋯** — indica que a busca continua por g=2, 3, …
- **Objetivo** (`g=25`): todos os tiles verdes, `[1,2,3 / 4,5,6 / 7,8,_]`

O A* gerou os dois vizinhos, calculou o f de cada um e colocou na fila. Vai expandindo sempre o menor f até chegar no objetivo.

**O bullet no final:**
"Hash registra o melhor g por estado e o estado pai — descarta revisitas e reconstrói o caminho ao chegar no objetivo." Isso fecha o loop: a hash não é só para evitar loops, é também quem permite reconstruir o caminho ótimo ao fim.

**Fala:**
> "Aqui vemos o A* trabalhando com um estado real. Partimos daqui — peças fora do lugar, h=15. Geramos dois vizinhos possíveis: mover o vazio para a esquerda ou para baixo. Cada um entra na fila de prioridade com seu f calculado.
>
> O A* continua assim — sempre expandindo o menor f — até chegar no objetivo em 25 movimentos, com todos os tiles no lugar.
>
> A hash registra o g e o estado pai de cada nó visitado. Quando o objetivo é encontrado, seguimos os pais de volta até o início — e temos o caminho completo, passo a passo."

---

## Slide 10 — Resultados: 8-puzzle

### O que o slide mostra
O slide tem três partes: uma linha de estatísticas no topo, um gráfico de barras à esquerda e uma comparação A* vs IDA* à direita.

**Linha de estatísticas (topo):**
- **100 / 100** — ambos resolveram todas as instâncias, zero falhas
- **21,4** — profundidade média (movimentos para resolver)
- **< 22 ms** — tempo máximo por instância (A* < 18 ms, IDA* < 22 ms)

**Gráfico de barras — distribuição de profundidade:**
Mostra quantas das 100 instâncias caem em cada faixa de dificuldade:
- 10–14 movimentos: **4 instâncias** (azul) — as mais triviais
- 15–19 movimentos: **25 instâncias** (amarelo)
- 20–24 movimentos: **51 instâncias** (verde) — a maioria, faixa central
- 25–28 movimentos: **20 instâncias** (vermelho) — as mais difíceis

A maior barra é a verde (51%) — confirma que a média de 21,4 está no centro da distribuição.

**Comparação A* vs IDA* (direita):**
Dois cards lado a lado:
- **A***: ~852 nós/instância · 362 ms total
- **IDA***: ~1606 nós/instância · 226 ms total

Por que IDA* expande mais nós mas é mais rápido no total? Cada nó é mais barato de processar (sem heap, sem hash de fechados). O dobro de nós com metade do overhead resulta em tempo parecido — ou até menor.

**Extremos (lado a lado abaixo dos cards):**
- Mais fácil: instância 45 — 10 movimentos, 11 nós
- Mais difíceis: instâncias 16, 64, 66, 72 — 28 movimentos cada

**Nota de corretude:** A* e IDA* concordam na profundidade em todas as 100 instâncias. Se um estivesse errado, divergiria — a coincidência é a prova mais forte de corretude.

**Fala:**
> "No 8-puzzle, ambos resolveram 100 de 100 — zero falhas. A profundidade média foi 21,4 movimentos. O gráfico mostra que a maioria das instâncias cai entre 20 e 24 movimentos.
>
> À direita, a comparação A* e IDA*: o A* expande cerca de 852 nós por instância, o IDA* cerca de 1606 — quase o dobro. Mas o IDA* termina em menos tempo total porque cada nó custa menos para processar.
>
> O detalhe mais importante: os dois concordam na profundidade em todas as instâncias. Isso prova que os dois estão encontrando a solução ótima — se houvesse um bug em qualquer um deles, esses números divergiriam."

---

## Slide 11 — Resultados: 15-puzzle

### Por que nem todas as instâncias resolvem?
O 15-puzzle com soluções de 50+ movimentos exige expandir milhões de nós mesmo com heurística forte. Impusemos um limite (500 mil para A*, 2 milhões para IDA*) para a apresentação terminar em tempo razoável. As instâncias que "atingem limite" têm solução — apenas precisam de mais computação.

**Resultados reais:**
- A* (limite 500 K nós): **25 / 100** resolvidas · profundidade 41–56 movimentos
- IDA* (limite 2 M nós): **30 / 100** resolvidas · mesmas profundidades

**Tabela resumo — 15-puzzle (100 instâncias):**

| Métrica | A* | IDA* |
|---|---|---|
| Instâncias resolvidas | 25 / 100 | 30 / 100 |
| Profundidade mínima | 41 movimentos | 41 movimentos |
| Profundidade máxima | 56 movimentos | 56 movimentos |
| Profundidade média (resolvidas) | 46,7 movimentos | 47,7 movimentos |
| Total de nós expandidos | 43.062.626 | 160.576.158 (~160 M) |
| Tempo total | 471.896 ms (~7,8 min) | 168.133 ms (~2,8 min) |
| Limite por instância | 500.000 nós | 2.000.000 nós |

**Por que o IDA* resolve mais?**
Com 2 milhões de nós e memória O(profundidade), o IDA* processa mais nós por segundo — não precisa gerenciar uma heap nem uma hash de fechados. Resultado: cobre mais do espaço dentro do limite de tempo.

**Confirmação de corretude:** para as instâncias que ambos resolvem, as profundidades coincidem. Isso garante que ambos estão encontrando a solução ótima.

**Fala:**
> "Para o 15-puzzle, definimos limites de nós para manter o tempo de execução razoável. O A* resolveu 25 das 100 instâncias, com profundidades de 41 a 56 movimentos. O IDA* resolveu 30 — 5 a mais — porque processa nós mais rapidamente ao custo de re-expandir alguns estados.
>
> As instâncias que atingem o limite não têm nenhum bug — elas simplesmente precisam de mais recursos. São equivalentes aos benchmarks de Korf (1985), que são referências clássicas em pesquisa de IA."

---

## Slide 12 — Conclusão

### O que o slide mostra
Três bullets que conectam os resultados à teoria — sem repetir nada já dito nos slides anteriores.

**Bullet 1 — Heurística admissível transformou o 15-puzzle**
Sem Manhattan + Conflito Linear, A* vira BFS puro e o 15-puzzle fica completamente inviável. A heurística admissível não só acelera — ela é o que torna o problema tratável.

**Bullet 2 — A* e IDA* garantem ótimo, com trade-off diferente**
Ambos encontram a solução de menor custo. A* usa memória para evitar re-expandir estados; IDA* usa quase zero de memória mas re-expande. No 15-puzzle esse trade-off decide quem resolve mais dentro do limite de nós.

**Bullet 3 — 8-puzzle: 100/100, profundidade média 21,4**
O número que comprova que a implementação está correta: dois algoritmos independentes, mesma resposta em todas as 100 instâncias.

**Fala:**
> "Três pontos para fechar. Primeiro: a heurística admissível não é um detalhe de otimização — é o que torna o 15-puzzle computacionalmente viável. Sem ela, precisaríamos explorar o espaço inteiro.
>
> Segundo: A* e IDA* têm trade-offs complementares. A* usa memória para evitar re-expansões; IDA* economiza memória e paga com re-expansões. Nos nossos experimentos, o IDA* resolveu mais instâncias do 15-puzzle dentro do limite porque processa nós mais rápido.
>
> Terceiro: no 8-puzzle, 100 de 100, profundidade média 21,4. Os dois algoritmos concordam em cada instância — isso é a confirmação de corretude."

---

## Slide 13 — Referências

Mencionar as referências se o professor perguntar sobre a origem dos algoritmos.

---

# GUIA DE CONCEITOS — Estude isso antes de apresentar

## O que é A* (explicação simples)
Imagine que você está num labirinto e quer chegar ao final pelo caminho mais curto. O BFS olharia todos os caminhos de comprimento 1, depois todos de comprimento 2, etc. O A* é mais esperto: para cada caminho que está explorando, ele estima quanto ainda falta até o final (a heurística). Com isso, prioriza os caminhos que parecem mais promissores.

**O que garante a otimalidade:** se a heurística nunca superestima, o A* nunca descarta um caminho que poderia ser ótimo. Quando chega ao objetivo, tem certeza de que não existe caminho mais curto.

## O que é IDA* (explicação simples)
É como o A* mas sem memória. Em vez de guardar todos os estados abertos numa fila, ele faz uma busca em profundidade repetida. Primeiro tenta com um limite f = h(inicial). Se não achar o objetivo, aumenta o limite para o próximo valor possível e tenta de novo. Como não guarda histórico, re-expande alguns nós, mas usa memória mínima.

## O que é uma heurística admissível
Uma estimativa que **nunca promete demais**. Se a realidade for "faltam 10 movimentos", a heurística pode dizer 7 ou 10, mas nunca 15. Se disser 15, o A* poderia achar que existe um caminho mais curto passando por outro lugar, ignorar o caminho ótimo, e não dar a resposta certa.

## Por que o uint64_t funciona como chave de hash
O `unordered_map` do C++ usa uma função de hash nativa para `uint64_t` — é um tipo fundamental da linguagem. Isso significa que inserir e buscar um estado na tabela custa O(1) amortizado sem nenhum overhead extra. Se usássemos um array ou string, precisaríamos de uma função de hash customizada, mais lenta.

## Por que a paridade de inversões funciona
Cada movimento de peça troca o vazio com uma peça adjacente. Na sequência linear do tabuleiro, isso troca dois elementos. A teoria combinatória mostra que isso altera a paridade de inversões de forma previsível. Estados com paridades diferentes nunca se alcançam mutuamente — pertencem a componentes desconexos do grafo.

---

# PERGUNTAS PROVÁVEIS DA BANCA

**"Por que a tabela hash é necessária e não é só uma otimização?"**
> Sem ela, a busca não termina. O grafo tem ciclos — você pode fazer A→B→A→B indefinidamente. A hash marca estados como visitados e impede essa repetição. Não é sobre velocidade, é sobre terminação.

**"Como o A* garante a solução ótima?"**
> Se h for admissível, quando o objetivo é retirado da fila com custo f, esse f é o ótimo. Prova: se existisse f' < f mais curto, ele estaria na fila com f' < f e teria sido extraído antes. Como não foi, f é mínimo.

**"O IDA* é sempre mais lento que o A*?"**
> Não. O IDA* re-expande nós entre iterações, mas cada expansão é muito mais barata (sem heap, sem hash). Para instâncias difíceis do 15-puzzle, o IDA* frequentemente resolve onde o A* falha com o mesmo orçamento de memória.

**"Por que Manhattan + Conflito Linear é admissível?"**
> Manhattan: cada peça precisa de pelo menos sua distância Manhattan, independente das outras. Conflito Linear: cada conflito detectado exige no mínimo 2 movimentos inevitáveis. A soma nunca superestima o custo real.

**"O que são as instâncias que atingem LIMITE?"**
> São instâncias cujo espaço de busca exige mais nós do que o limite definido (500 K para A*, 2 M para IDA*) — independente da profundidade da solução. Por exemplo, inst. 7 tem h₀=30 e já esgota o limite: a heurística é boa mas o espaço ainda é grande demais naquele orçamento. A solução existe — só precisaríamos de mais tempo ou de uma heurística mais forte (como Pattern Databases). São equivalentes aos benchmarks de Korf 1985.

**"Por que usaram uint64_t e não um array para o estado?"**
> uint64_t é uma chave nativa do C++ — a função de hash padrão do unordered_map já funciona em O(1) sem overhead. Com array, precisaríamos de hash customizada. Além disso, 8 bytes por estado vs 9 ou 16 ints — 4× a 8× menos memória.

**"Como vocês verificam que a solução está correta?"**
> Três formas: (1) A* e IDA* concordam na profundidade ótima para todas as instâncias que ambos resolvem. (2) O arquivo passo a passo pode ser verificado manualmente — cada estado difere do anterior por exatamente um movimento. (3) O último estado da solução é sempre o estado objetivo.

**"Por que vocês não implementaram BFS?"**
> BFS não tem heurística — equivale a h=0 em Dijkstra. Para o 8-puzzle funcionaria, mas para o 15-puzzle seria impossível (10 trilhões de estados). Como o enunciado pede A* e IDA*, implementamos esses dois para ambos os puzzles. A* com h=0 seria BFS; com h=Manhattan+ConflitosLineares, é muito mais eficiente.

---

# ERROS A EVITAR

- **"Compilar as instâncias"** — instâncias se *executam*, código se *compila*
- **"O A* é sempre melhor que BFS"** — depende da heurística; com h=0 são equivalentes
- **"O LIMITE significa que não tem solução"** — tem solução, só precisaria de mais recursos
- **"A heurística é uma otimização"** — sem heurística admissível, A* perde a garantia de otimalidade
- Confundir **expandir um nó** (retirar da fila e gerar vizinhos) com **gerar um vizinho** (colocar na fila)
