# Roteiro de Apresentação — 8-puzzle e 15-puzzle
## Tempo: 15 a 20 minutos | 14 slides

---

> **Como usar este roteiro:** cada seção corresponde a um slide. Leia o conceito antes de memorizar a fala. Se você entender O QUE está acontecendo, não precisa decorar nada — as palavras vêm naturalmente.

---

## Slide 1 — Capa

### O que está acontecendo aqui?
O **8-puzzle** é aquele quebra-cabeça deslizante de 3×3 com peças numeradas 1 a 8 e um espaço vazio. Você pode mover qualquer peça adjacente ao espaço para o lugar vazio. O objetivo é chegar da configuração embaralhada até a configuração final `[1,2,3 / 4,5,6 / 7,8,_]`.

O **15-puzzle** é a versão 4×4: 15 peças numeradas de 1 a 15 e um espaço vazio numa grade de 4 colunas por 4 linhas. A mecânica é idêntica — mover qualquer peça adjacente ao vazio — mas o objetivo é `[_, 1, 2, 3 / 4, 5, 6, 7 / 8, 9, 10, 11 / 12, 13, 14, 15]` (vazio no canto superior esquerdo — convenção adotada pelas instâncias fornecidas). Por ser 4×4, o espaço de estados é astronomicamente maior: cerca de 10 trilhões de configurações possíveis, contra 181 mil do 8-puzzle. Isso torna o 15-puzzle computacionalmente muito mais difícil.

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

### Uma distinção importante sobre "executar todas as instâncias"
O enunciado diz *"Execute para todas as instâncias postadas em anexo"*. Isso significa **rodar o algoritmo em cada instância e reportar o resultado** — não necessariamente resolver todas até o fim.

Para o 8-puzzle, os dois algoritmos resolvem 100% das instâncias sem dificuldade. Para o 15-puzzle, o espaço de estados tem ~10 trilhões de configurações: instâncias com soluções de 50+ movimentos podem exigir bilhões de expansões mesmo com a melhor heurística prática. Resolver todas sem restrição de tempo exigiria técnicas especializadas além do escopo do trabalho (como Pattern Databases).

Por isso, definimos um **limite de nós por instância** — uma decisão de projeto explícita que equilibra cobertura e tempo de execução. As instâncias que atingem o limite têm solução; simplesmente precisariam de mais recursos. Isso é reportado transparentemente nos resultados e faz parte da análise.

**Fala:**
> "O enunciado tem três pedidos centrais. Primeiro: modelar cada configuração do tabuleiro como um nó de grafo, e cada movimento válido como uma aresta de custo 1. Segundo: usar tabela hash para verificar estados visitados — sem isso, a busca revisita estados e nunca termina. Terceiro: implementar A* e IDA*, que produzem a solução ótima e mostram cada movimento do caminho.
>
> Vale destacar uma decisão de projeto: o enunciado pede para executar todas as instâncias, e fazemos isso — rodamos em todas e reportamos cada resultado. Para as instâncias mais difíceis do 15-puzzle, o resultado é 'limite atingido', não um erro. Isso é esperado e faz parte da análise: mostramos quantas instâncias cada algoritmo resolve, com que custo, e por que algumas exigem mais recursos do que o orçamento definido."

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

A chave da tabela é exatamente o número de 64 bits que representa o tabuleiro — isso permite que a consulta seja, em média, O(1) amortizado, sem precisar comparar o estado inteiro ou calcular uma hash complexa. No pior caso teórico, se muitos estados colidirem na mesma posição, a consulta pode chegar a O(n), mas isso é muito improvável com a hash padrão de uint64_t.

**Fala:**
> "Para guardar e comparar os estados do tabuleiro de forma eficiente, empacotamos cada configuração num único número de 64 bits — 4 bits por célula. Com isso, dois estados são comparados com uma única operação do processador.
>
> A tabela hash guarda todos os estados já visitados. Toda vez que o algoritmo gera um novo estado, ele consulta a tabela: se já foi visitado, descarta; se não foi, processa. Sem isso, a busca entraria em loop e nunca terminaria."

---

## Slide 5 — A* — Busca pelo Menor f = g + h

### O que mostrar no slide
O foco aqui é explicar como o A* decide qual estado explorar a seguir. Comece pelo gráfico e depois apresente a fórmula, porque o motivo é mais importante do que a matemática imediata.

**Como usar o gráfico:**
- O nó de cima é o **START**. Ele tem `g = 0` porque ainda não fizemos nenhum movimento.
- A partir dele são gerados três filhos. Cada filho é um estado possível após um movimento.
- Cada filho já tem um custo real (`g`) e uma estimativa do que ainda falta (`h`).
- O A* olha para o valor total `f` e escolhe o menor.

**O que a figura mostra:**
- É um grafo simples com o START no topo e três estados logo abaixo.
- Cada nó tem um valor `f`. O nó do centro é amarelo porque tem o menor `f` e é o primeiro a ser expandido.
- Os nós à esquerda e à direita estão em cinza porque o A* deixa eles para depois.
- O caminho em amarelo representa a escolha mais promissora; os pontinhos na linha verde mostram que o objetivo real exige mais movimentos do que a figura pode desenhar de forma direta.

**Explicando o grafo (de cima para baixo):**
- **START** — estado inicial, `g = 0`.
- Três filhos:
  - `f = 9` (cinza, esquerda) — menos promissor por enquanto.
  - **`f = 7` (amarelo, centro)** — menor `f` e o primeiro a ser expandido.
  - `f = 11` (cinza, direita) — deixado para depois.
- Do nó `f = 7`, aparecem dois filhos:
  - **GOAL** (estrela verde) — solução encontrada pelo caminho ótimo.
  - `f = 10` (apagado) — não precisa ser expandido agora.
- A linha amarela indica o caminho que o A* considera mais promissor.
- Os pontinhos na linha verde mostram que o slide está simplificando o caminho real.

**Antes da fórmula, diga isso:**
- “O A* não escolhe só o estado que parece mais perto do objetivo, nem só o estado que já custou menos para chegar. Ele combina as duas informações.”
- “Isso é o que torna o A* eficiente: ele avalia tanto o que já foi gasto quanto o que ainda falta.”

**A fórmula f = g + h — o que cada variável significa:**
- **g** = custo real até aqui, ou seja, o número de movimentos já feitos.
- **h** = estimativa do custo restante até o objetivo, a heurística.
- **f = g + h** = estimativa do custo total do caminho que passa por esse nó.

**Por que isso importa:**
- `g` sozinho faria o algoritmo preferir caminhos curtos até agora, mesmo que levem a estados ruins.
- `h` sozinho faria o algoritmo preferir estados que “parecem” bons, mesmo que o caminho até eles seja caro.
- `f` combina os dois e busca o menor custo total provável.

**Exemplo do nó amarelo:**
- `g = 1` (um movimento já foi feito)
- `h = 6` (estimativa do restante)
- `f = 7`

Mesmo que outro nó tenha `h` menor ou `g` menor, o A* escolhe o menor `f`.

**Por que o A* garante a solução ótima?**
- O A* sempre expande o nó aberto com o menor `f`.
- Se existisse um caminho mais curto para o objetivo, algum nó desse caminho teria `f` menor e seria expandido antes.
- Portanto, o primeiro objetivo retirado da fila é um objetivo ótimo.

**O que dizer em voz alta:**
> “Este slide mostra como o A* equilibra o custo já gasto com a estimativa do que falta. O START começa com `g = 0` e um `h` estimado. Cada vizinho recebe um `f = g + h`, e o algoritmo expande primeiro o menor `f`.
>
> O resultado é que não ganhamos por olhar só o caminho mais promissor no futuro e nem por olhar só o caminho mais barato até agora. O A* escolhe o nó que parece ter o menor custo total. Por isso o nó amarelo com `f = 7` sai antes dos outros.
>
> Quando o objetivo é finalmente retirado da fila, esse caminho já é o mais curto possível.”

---

## Slide 6 — A* vs IDA* — Memória ou Re-expansão?
### O que este slide mostra (versão enxuta e didática)
Este slide explica por que usamos A* e IDA*: um prioriza evitar trabalho repetido guardando muitos estados (mais memória),
o outro prioriza usar pouca memória e aceita refazer parte do trabalho (mais re‑expansões).

### Por que usar A* ou IDA* (didático)
- A*: guarda a fronteira em memória (os estados gerados que ainda não foram completamente explorados) e uma tabela para não visitar o mesmo estado com custo pior. Resultado: menos nós
  expandidos, mais memória usada.
- IDA*: faz uma busca em profundidade limitada por um critério `f = g + h`; não guarda a fronteira. Resultado: memória
  pequena (proporcional à profundidade), mas pode re‑explorar estados várias vezes.

### Pseudocódigo (em palavras) — como o IDA* funciona
1. Defina `limite = h(inicial)` (g = 0 no começo).
2. Faça uma busca em profundidade seguindo um único caminho; em cada nó calcule `f = g + h`.
3. Se `f > limite`, pare de seguir esse ramo (poda) e registre o menor `f` cortado.
4. Se encontrar o objetivo dentro do limite, pare — a solução é ótima. Caso contrário, atualize `limite` para o menor `f`
   que foi cortado e repita a DFS.

### Tabela — explicação linha a linha (didático)
- Forma de busca: A* = fila de prioridade (escolhe menor `f` globalmente). IDA* = DFS iterativa com poda por `f`.
- Memória: A* usa muita memória (fila + tabela); IDA* usa memória baixa (pilha de recursão, O(profundidade)).
- Re‑expansões: A* evita re‑expansões com a tabela; IDA* re‑expande estados entre iterações porque não guarda
  os fechados.
- Resultado prático: com limites práticos A* tende a expandir menos nós, mas pode esgotar memória; IDA* expande
  mais nós, mas cada nó é mais barato — por isso, em instâncias muito difíceis (15‑puzzle) o IDA* pode resolver
  mais casos dentro do orçamento.

### Execução do programa (linguagem simples)
O programa lê um conjunto de instâncias e, para cada instância, tenta resolver com A* e com IDA*. Para cada tentativa
registra: profundidade da solução (se encontrada), nós expandidos, tempo e se um limite foi atingido. No final ele grava
esses resultados em arquivos CSV e gera um relatório resumo; também salva, para cada instância resolvida, um arquivo com
os passos da solução. Usamos limites práticos para controle de tempo (ex.: A* ≈ 500K nós, IDA* ≈ 20M nós).

### Linha por linha da tabela

O texto da tabela deve ser lido como um contraste direto.
Primeiro, o A* mantém uma fila de prioridade e escolhe sempre o nó com menor `f` entre todos os nós abertos. Ele também mantém uma tabela hash `unordered_map<uint64_t, GNode>` para registrar o menor custo `g` encontrado por estado, de modo que não re-expanda estados com custo pior.

O IDA* faz o contrário: ele não guarda estados fechados, ele só guarda o caminho atual na pilha de recursão e alguns contadores. Isso é o que torna sua memória proporcional à profundidade da solução, em vez do tamanho da fronteira de busca.

Como consequência, o A* evita revisitar estados e expande menos nós, enquanto o IDA* pode visitar o mesmo estado várias vezes em diferentes iterações. Essa re-expansão aumenta o número total de nós expandidos no IDA*, mas cada nó custa menos porque não há heap nem hash de fechados para gerenciar.

No nosso experimento, isso se confirma em como os limites foram definidos: colocamos um teto de 500 mil nós para o A* e 20 milhões para o IDA*, porque cada nó do IDA* pesa muito menos em memória. No 8-puzzle, os dois algoritmos resolvem todas as 100 instâncias. No 15-puzzle, a diferença de memória vira um fator decisivo: o A* resolve menos instâncias que o IDA*, pois esgota memória antes.

O ponto principal do slide é esse trade-off: A* troca memória por menos trabalho repetido; IDA* troca pouca memória por mais re-expansões.

**O que dizer em voz alta:**
> “Este slide responde à pergunta do título. O A* usa memória para evitar trabalho repetido; o IDA* usa pouca memória e aceita re-expansões.
>
> O IDA* começa com um limite igual a `h` do início e faz DFS cortando qualquer ramo cujo `f = g + h` ultrapasse esse limite. Se não encontrar o objetivo, ele aumenta o limite e tenta de novo. Por isso ele usa muito pouca memória.
>
> O A* faz o contrário: ele guarda todos os estados visitados e escolhe o próximo nó pelo menor `f` entre todos os nós abertos. Isso exige mais memória, mas reduz nós expandidos.
>
> No 8-puzzle, os dois resolvem todas as instâncias. No 15-puzzle, o IDA* resolve mais instâncias dentro do limite porque não esgota a memória. Por isso o título é ‘Memória ou Re-expansão’.”

### Como o programa executa isso
O `main.cpp` lê as instâncias de `8puzzle_instances.txt` e `15puzzle_instances.txt` e aplica `solveAStar(...)` e `solveIDAStar(...)` a cada instância de 8-puzzle e 15-puzzle. No terminal ele mostra apenas o andamento e os resumos, enquanto os detalhes completos são gravados em arquivos.

Os resultados são gravados em arquivos separados:

* `output/8puzzle_astar_results.csv`
* `output/8puzzle_idastar_results.csv`
* `output/15puzzle_astar_results.csv`
* `output/15puzzle_idastar_results.csv`

O programa também gera relatórios em Markdown em `output/report_astar.md` e `output/report_idastar.md`, e grava os passos da solução em `output/detalhes/astar_*` e `output/detalhes/idastar_*`.

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

### Como a solução passo a passo é gerada

Quando o objetivo é encontrado, percorremos os ponteiros de pai armazenados na hash de trás para frente (objetivo → inicial), acumulando os estados num vetor. Depois revertemos esse vetor para ter a sequência na ordem correta (inicial → objetivo).

Para imprimir cada passo, decodificamos o `uint64_t` de volta para uma grade: extraímos 4 bits por célula (`(state >> (4*i)) & 0xF`) e formatamos como matriz. O espaço vazio (valor 0) é impresso como `_`. O resultado é salvo num arquivo `.txt` em `output/detalhes/` — um arquivo por instância resolvida, com o estado inicial seguido de cada movimento numerado até o estado objetivo.

O IDA* faz o mesmo processo, mas de forma diferente: como não guarda a hash de estados, ele mantém um vetor de caminho ativo durante a DFS recursiva. Quando a recursão encontra o objetivo, esse vetor já contém a sequência completa e é impresso diretamente, sem precisar de reconstrução por ponteiros de pai.

**Fala:**
> "Aqui vemos o A* trabalhando com um estado real. Partimos daqui — peças fora do lugar, h=15. Geramos dois vizinhos possíveis: mover o vazio para a esquerda ou para baixo. Cada um entra na fila de prioridade com seu f calculado.
>
> O A* continua assim — sempre expandindo o menor f — até chegar no objetivo em 25 movimentos, com todos os tiles no lugar.
>
> A hash registra o g e o estado pai de cada nó visitado. Quando o objetivo é encontrado, seguimos os pais de volta até o início, revertemos a sequência e imprimimos cada estado como uma grade — é o arquivo passo a passo que aparece em output/detalhes."

---

## Slide 10 — Resultados: 8-puzzle

### O que o slide mostra
O slide tem três partes: uma linha de estatísticas no topo, um gráfico de barras à esquerda e uma comparação A* vs IDA* à direita.

**Linha de estatísticas (topo):**
- **100 / 100** — ambos resolveram todas as instâncias, zero falhas
- **21,4** — profundidade média (movimentos para resolver)
- **383 / 81 ms** — tempo total (A* · IDA*) · por instância: A* < 13 ms, IDA* < 7 ms

**Gráfico de barras — distribuição de profundidade:**
Mostra quantas das 100 instâncias caem em cada faixa de dificuldade:
- 10–14 movimentos: **4 instâncias** (azul) — as mais triviais
- 15–19 movimentos: **25 instâncias** (amarelo)
- 20–24 movimentos: **51 instâncias** (verde) — a maioria, faixa central
- 25–28 movimentos: **20 instâncias** (vermelho) — as mais difíceis

A maior barra é a verde (51%) — confirma que a média de 21,4 está no centro da distribuição.

**Comparação A* vs IDA* (direita):**
Dois cards lado a lado:
- **A***: ~852 nós/instância · 383 ms total
- **IDA***: ~1606 nós/instância · 81 ms total

Por que IDA* expande mais nós mas é muito mais rápido no total? Cada nó é mais barato de processar (sem heap, sem hash de fechados). O dobro de nós com fração do overhead resulta em tempo muito menor.

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

O 15-puzzle tem ~10 trilhões de estados alcançáveis. Instâncias com soluções de 50+ movimentos exigem expandir milhões de nós mesmo com a melhor heurística prática. O limite de nós (500 K para A*, 20 M para IDA*) foi calibrado para maximizar instâncias resolvidas dentro de um tempo de execução aceitável. Resolver todas as instâncias exigiria técnicas além do escopo do trabalho, como Pattern Databases.

**Resultados reais:**
- A* (limite 500 K nós): **25 / 100** resolvidas · profundidade 41–56 movimentos
- IDA* (limite 20 M nós): **68 / 100** resolvidas · profundidade 41–64 movimentos

**Tabela resumo — 15-puzzle (100 instâncias):**

| Métrica | A* | IDA* |
|---|---|---|
| Instâncias resolvidas | 25 / 100 | 68 / 100 |
| Profundidade mínima | 41 movimentos | 41 movimentos |
| Profundidade máxima | 56 movimentos | 64 movimentos |
| Profundidade média (resolvidas) | 46,7 movimentos | 50,8 movimentos |
| Total de nós expandidos | 43.062.626 | 969.367.350 |
| Tempo total | 368,5 s (~6 min) | 736 s (~12,3 min) |
| Limite por instância | 500.000 nós | 20.000.000 nós |

**Por que o IDA* resolve mais?**
Com memória O(profundidade), o IDA* processa mais nós por segundo — não precisa gerenciar heap nem hash de fechados. Resultado: cobre mais espaço dentro do mesmo orçamento de nós.

**Extremos do IDA* (aparecem no slide, lado a lado):**
- **Mais fácil — inst. 55:** 41 movimentos · 47 ms · 77.633 nós. Profundidade mínima entre todas as instâncias resolvidas — h₀=31, solução rasa, IDA* termina rapidamente.
- **Mais difícil — inst. 43:** 64 movimentos · 4.569 ms · 9.609.075 nós. Profundidade máxima — h₀=56, solução mais profunda do conjunto; chegou perto do limite (20M) mas resolveu.

**Limites definidos no código (`main.cpp`):**
```cpp
static constexpr long long ASTAR_NODE_LIMIT   =   500'000LL;
static constexpr long long IDASTAR_NODE_LIMIT = 20'000'000LL;
```
Esses valores são consultados a cada expansão. Se o total de nós expandidos ultrapassar o limite, o resultado é marcado como `LIMITE` — sem crash, sem loop infinito.

**Confirmação de corretude:** para as instâncias que ambos resolvem, as profundidades coincidem. Isso garante que ambos estão encontrando a solução ótima.

**Fala:**
> "Para o 15-puzzle, rodamos em todas as 100 instâncias. O A* resolveu 25 dentro do limite de 500 mil nós, com soluções de 41 a 56 movimentos e profundidade média de 46,7. O IDA* resolveu 68 — quase o triplo — porque processa cada nó mais rapidamente: sem heap nem hash de fechados, cobre muito mais espaço no mesmo orçamento.
>
> No total, o A* expandiu 43 milhões de nós em ~6 minutos; o IDA* expandiu 969 milhões em ~12 minutos. Mais nós, mas com custo individual muito menor — exatamente o trade-off que discutimos.
>
> As instâncias que atingem o limite têm solução — simplesmente precisariam de mais recursos. Isso é esperado, documentado, e faz parte da análise."

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

**"O enunciado pede para executar TODAS as instâncias — vocês não cumpriram isso?"**
> Cumprimos. O código executa A* e IDA* em todas as 100 instâncias do 15-puzzle e reporta o resultado de cada uma. "Executar todas" não é o mesmo que "resolver todas" — para as instâncias difíceis, o resultado é "LIMITE ATINGIDO", que é um resultado válido e esperado. Resolver todas as instâncias do 15-puzzle sem restrição de tempo exigiria técnicas além do escopo do trabalho, como Pattern Databases. Mesmo as implementações de referência da literatura (Korf, 1985) levam horas nas instâncias mais difíceis.

**"O que são as instâncias que atingem LIMITE?"**
> São instâncias cujo espaço de busca exige mais nós do que o limite definido (500 K para A*, 20 M para IDA*) — independente da profundidade da solução. Por exemplo, inst. 7 tem h₀=30 e já esgota o limite: a heurística é boa mas o espaço ainda é grande demais naquele orçamento. A solução existe — só precisaríamos de mais tempo ou de uma heurística mais forte (como Pattern Databases). São equivalentes aos benchmarks de Korf 1985.

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
