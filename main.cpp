#include <algorithm>
#include <chrono>
#include <climits>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <queue>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace fs = std::filesystem;

// ─── Codificacao do estado ──────────────────────────────────────────────────────
// Cada celula ocupa 4 bits.
// 8-puzzle:  9 celulas x 4 bits = 36 bits  (cabe em uint64_t)
// 15-puzzle: 16 celulas x 4 bits = 64 bits (uint64_t exato)
// O uint64_t funciona como chave nativa da tabela hash — O(1) por consulta.

static uint64_t packState(const std::vector<int>& cells) {
    uint64_t s = 0;
    for (int i = 0; i < (int)cells.size(); ++i)
        s |= static_cast<uint64_t>(cells[i]) << (i * 4);
    return s;
}

static std::vector<int> unpackState(uint64_t s, int n) {
    std::vector<int> cells(n);
    for (int i = 0; i < n; ++i)
        cells[i] = static_cast<int>((s >> (i * 4)) & 0xFu);
    return cells;
}

// ─── Posicao objetivo de cada peca ─────────────────────────────────────────────
// 8-puzzle  (width=3): goal = [1,2,...,8,0]  — peca v no indice v-1
// 15-puzzle (width=4): goal = [0,1,...,15]   — peca v no indice v

static inline int goalRow(int v, int width) {
    return (width == 3) ? (v - 1) / 3 : v / width;
}
static inline int goalCol(int v, int width) {
    return (width == 3) ? (v - 1) % 3 : v % width;
}

// ─── Distancia de Manhattan ─────────────────────────────────────────────────────
// Heuristica admissivel: nunca superestima o numero real de movimentos.

static int manhattan(uint64_t s, int width) {
    int n = width * width;
    int d = 0;
    for (int i = 0; i < n; ++i) {
        int v = static_cast<int>((s >> (i * 4)) & 0xFu);
        if (v == 0) continue;
        d += std::abs(i / width - goalRow(v, width))
           + std::abs(i % width - goalCol(v, width));
    }
    return d;
}

// ─── Conflito Linear ────────────────────────────────────────────────────────────
// Detecta pares de pecas na mesma linha/coluna alvo em ordem invertida.
// Cada conflito exige pelo menos +2 movimentos extra — ainda admissivel.

static int linearConflict(uint64_t s, int width) {
    int n = width * width;
    auto cells = unpackState(s, n);
    int extra = 0;

    for (int r = 0; r < width; ++r) {
        for (int c1 = 0; c1 < width - 1; ++c1) {
            int v1 = cells[r * width + c1];
            if (!v1 || goalRow(v1, width) != r) continue;
            int gc1 = goalCol(v1, width);
            for (int c2 = c1 + 1; c2 < width; ++c2) {
                int v2 = cells[r * width + c2];
                if (!v2 || goalRow(v2, width) != r) continue;
                if (gc1 > goalCol(v2, width)) extra += 2;
            }
        }
    }
    for (int c = 0; c < width; ++c) {
        for (int r1 = 0; r1 < width - 1; ++r1) {
            int v1 = cells[r1 * width + c];
            if (!v1 || goalCol(v1, width) != c) continue;
            int gr1 = goalRow(v1, width);
            for (int r2 = r1 + 1; r2 < width; ++r2) {
                int v2 = cells[r2 * width + c];
                if (!v2 || goalCol(v2, width) != c) continue;
                if (gr1 > goalRow(v2, width)) extra += 2;
            }
        }
    }
    return extra;
}

static int heuristic(uint64_t s, int width) {
    return manhattan(s, width) + linearConflict(s, width);
}

// ─── Geracao de vizinhos ────────────────────────────────────────────────────────

static std::vector<uint64_t> getNeighbors(uint64_t s, int width) {
    int n = width * width;
    auto cells = unpackState(s, n);
    int blank = 0;
    for (; blank < n && cells[blank] != 0; ++blank) {}
    int r = blank / width, c = blank % width;

    static constexpr int dr[] = {-1, 1, 0, 0};
    static constexpr int dc[] = {0, 0, -1, 1};

    std::vector<uint64_t> result;
    result.reserve(4);
    for (int d = 0; d < 4; ++d) {
        int nr = r + dr[d], nc = c + dc[d];
        if (nr < 0 || nr >= width || nc < 0 || nc >= width) continue;
        int nb = nr * width + nc;
        std::swap(cells[blank], cells[nb]);
        result.push_back(packState(cells));
        std::swap(cells[blank], cells[nb]);
    }
    return result;
}

// ─── Estado objetivo ────────────────────────────────────────────────────────────

static uint64_t goalState(int width) {
    int n = width * width;
    std::vector<int> g(n);
    if (width == 3) {
        for (int i = 0; i < n - 1; ++i) g[i] = i + 1;
        g[n - 1] = 0;
    } else {
        for (int i = 0; i < n; ++i) g[i] = i;
    }
    return packState(g);
}

// ─── Verificacao de solucionabilidade ─────────────────────────────────────────

static bool isSolvable(const std::vector<int>& tiles, int width) {
    int n = (int)tiles.size();
    int inversions = 0;
    for (int i = 0; i < n - 1; ++i)
        for (int j = i + 1; j < n; ++j)
            if (tiles[i] && tiles[j] && tiles[i] > tiles[j]) ++inversions;

    if (width % 2 == 1) {
        return inversions % 2 == 0;
    } else {
        int blankRow = 0;
        for (int i = 0; i < n; ++i)
            if (tiles[i] == 0) { blankRow = i / width; break; }
        return (inversions + (width - 1 - blankRow)) % 2 == 1;
    }
}

// ─── Representacao textual de um estado ────────────────────────────────────────

static std::string boardStr(uint64_t s, int width) {
    std::ostringstream out;
    int n = width * width;
    for (int i = 0; i < n; ++i) {
        int v = static_cast<int>((s >> (i * 4)) & 0xFu);
        if (v == 0) out << std::setw(3) << "_";
        else        out << std::setw(3) << v;
        if ((i + 1) % width == 0) out << "\n";
    }
    return out.str();
}

// ─── Resultado da busca ─────────────────────────────────────────────────────────
// path: sequencia de estados do inicial ao objetivo (caminho otimo, passo a passo)

struct SearchResult {
    int  id              = -1;
    int  depth           = -1;
    int  initialH        = 0;
    long long nodesExpanded = 0;
    double timeMs        = 0.0;
    bool solved          = false;
    bool limitHit        = false;
    bool unsolvable      = false;
    std::vector<uint64_t> path; // vazio se nao resolvido
};

static constexpr long long ASTAR_NODE_LIMIT   =   500'000LL;
static constexpr long long IDASTAR_NODE_LIMIT = 2'000'000LL;

// ─── A* ─────────────────────────────────────────────────────────────────────────
// Tabela hash: unordered_map<uint64_t, GNode>
//   - GNode.g      = menor custo g encontrado ate este estado
//   - GNode.parent = estado pai no caminho otimo (para reconstrucao)
// Fila de prioridade: (f=g+h, g, estado) — expande o mais promissor primeiro.
// Ao encontrar o objetivo: reconstroi o caminho seguindo os ponteiros pai.

SearchResult solveAStar(int id, const std::vector<int>& tiles, int width) {
    SearchResult result;
    result.id = id;

    if (!isSolvable(tiles, width)) { result.unsolvable = true; return result; }

    uint64_t start = packState(tiles);
    uint64_t goal  = goalState(width);
    result.initialH = heuristic(start, width);

    if (start == goal) {
        result.solved = true; result.depth = 0;
        result.path = {start};
        return result;
    }

    auto t0 = std::chrono::steady_clock::now();

    struct GNode { int g; uint64_t parent; };
    std::unordered_map<uint64_t, GNode> gScore;
    gScore.reserve(500'000);
    gScore[start] = {0, UINT64_MAX};

    using Node = std::tuple<int, int, uint64_t>;
    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> pq;
    pq.push({result.initialH, 0, start});

    while (!pq.empty()) {
        auto [f, g, cur] = pq.top(); pq.pop();

        auto it = gScore.find(cur);
        if (it != gScore.end() && it->second.g < g) continue;

        ++result.nodesExpanded;
        if (result.nodesExpanded > ASTAR_NODE_LIMIT) { result.limitHit = true; break; }

        if (cur == goal) {
            result.solved = true;
            result.depth = g;
            // Reconstroi o caminho seguindo ponteiros pai
            uint64_t curr = cur;
            while (curr != UINT64_MAX) {
                result.path.push_back(curr);
                auto it2 = gScore.find(curr);
                if (it2 == gScore.end() || it2->second.parent == UINT64_MAX) break;
                curr = it2->second.parent;
            }
            std::reverse(result.path.begin(), result.path.end());
            break;
        }

        for (uint64_t nb : getNeighbors(cur, width)) {
            int ng = g + 1;
            auto it2 = gScore.find(nb);
            if (it2 != gScore.end() && it2->second.g <= ng) continue;
            gScore[nb] = {ng, cur};
            pq.push({ng + heuristic(nb, width), ng, nb});
        }
    }

    auto t1 = std::chrono::steady_clock::now();
    result.timeMs = std::chrono::duration<double, std::milli>(t1 - t0).count();
    return result;
}

// ─── IDA* ────────────────────────────────────────────────────────────────────────
// DFS com corte f > limite; limite cresce iterativamente (proxima f minima excedida).
// Memoria: O(profundidade) — apenas a pilha de recursao.
// O vetor 'path' e construido durante a DFS e mantido quando o objetivo e encontrado.

struct DFSResult { int nextLimit; bool found; };

static DFSResult idaDFS(uint64_t cur, uint64_t prev, int g, int limit,
                        int width, uint64_t goal,
                        long long& nodes, long long nodeLimit,
                        std::vector<uint64_t>& path) {
    int h = heuristic(cur, width);
    int f = g + h;
    if (f > limit) return {f, false};
    if (nodes >= nodeLimit) return {INT_MAX, false};

    path.push_back(cur);

    if (cur == goal) return {0, true};

    int minExceeded = INT_MAX;
    for (uint64_t nb : getNeighbors(cur, width)) {
        if (nb == prev) continue;
        ++nodes;
        auto res = idaDFS(nb, cur, g + 1, limit, width, goal, nodes, nodeLimit, path);
        if (res.found) return res;        // caminho completo — nao faz pop
        if (res.nextLimit < minExceeded) minExceeded = res.nextLimit;
    }

    path.pop_back(); // retrocede (backtrack)
    return {minExceeded, false};
}

SearchResult solveIDAStar(int id, const std::vector<int>& tiles, int width) {
    SearchResult result;
    result.id = id;

    if (!isSolvable(tiles, width)) { result.unsolvable = true; return result; }

    uint64_t start = packState(tiles);
    uint64_t goal  = goalState(width);
    result.initialH = heuristic(start, width);

    if (start == goal) {
        result.solved = true; result.depth = 0;
        result.path = {start};
        return result;
    }

    auto t0 = std::chrono::steady_clock::now();
    int limit = result.initialH;
    std::vector<uint64_t> currentPath;
    currentPath.reserve(128);

    while (true) {
        currentPath.clear();
        long long iterNodes = 0;
        long long remaining = IDASTAR_NODE_LIMIT - result.nodesExpanded;
        auto res = idaDFS(start, UINT64_MAX, 0, limit, width, goal,
                          iterNodes, remaining, currentPath);
        result.nodesExpanded += iterNodes;

        if (res.found) {
            result.solved = true;
            result.depth  = static_cast<int>(currentPath.size()) - 1;
            result.path   = std::move(currentPath);
            break;
        }
        if (res.nextLimit == INT_MAX || result.nodesExpanded >= IDASTAR_NODE_LIMIT) {
            result.limitHit = true;
            break;
        }
        limit = res.nextLimit;
    }

    auto t1 = std::chrono::steady_clock::now();
    result.timeMs = std::chrono::duration<double, std::milli>(t1 - t0).count();
    return result;
}

// ─── Leitura das instancias ─────────────────────────────────────────────────────

struct PuzzleInstance { int id; std::vector<int> tiles; };

static std::vector<PuzzleInstance> loadInstances(const fs::path& path, int width) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error("Nao foi possivel abrir: " + path.string());

    int n = width * width;
    std::vector<PuzzleInstance> instances;
    std::string line;
    int lineNum = 0;
    while (std::getline(in, line)) {
        ++lineNum;
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) continue;
        std::istringstream ss(line);
        std::vector<int> tiles(n);
        bool ok = true;
        for (int& v : tiles) if (!(ss >> v)) { ok = false; break; }
        if (ok) instances.push_back({lineNum, std::move(tiles)});
    }
    return instances;
}

// ─── Arquivo de detalhe por instancia ──────────────────────────────────────────
// Exibe a solucao otima passo a passo — cada estado do caminho inicial->objetivo.

static void writeDetailFile(const SearchResult& r, int width,
                             const std::string& algo,
                             const fs::path& dir) {
    fs::create_directories(dir);
    std::ofstream out(dir / (algo + "_inst" + std::to_string(r.id) + ".txt"));
    if (!out) return;

    out << "Algoritmo  : " << algo << "\n";
    out << "Instancia  : " << r.id << "\n";
    out << "Heuristica : Manhattan + Conflito Linear\n";
    out << "Status     : " << (r.solved ? "RESOLVIDO" : (r.limitHit ? "LIMITE ATINGIDO" : "INSOLUVEL")) << "\n";

    if (r.solved) {
        out << "Profundidade otima: " << r.depth << " movimentos\n";
        out << "Nos expandidos    : " << r.nodesExpanded << "\n";
        out << "Tempo             : " << std::fixed << std::setprecision(2) << r.timeMs << " ms\n";

        out << "\n--- Solucao Otima Passo a Passo ---\n\n";
        for (int step = 0; step < (int)r.path.size(); ++step) {
            if (step == 0) out << "Estado inicial:\n";
            else           out << "Passo " << step << ":\n";
            out << boardStr(r.path[step], width) << "\n";
        }
    } else {
        out << "Nos expandidos: " << r.nodesExpanded << "\n";
        out << "Tempo         : " << std::fixed << std::setprecision(2) << r.timeMs << " ms\n";
    }
}

// ─── Saida CSV ──────────────────────────────────────────────────────────────────

static void writeCsv(const std::vector<SearchResult>& results,
                     const fs::path& path, const std::string& algo) {
    std::ofstream out(path);
    out << "id,algoritmo,h_inicial,profundidade,nos_expandidos,tempo_ms,resolvido,limite\n";
    for (const auto& r : results) {
        out << r.id << ',' << algo << ',' << r.initialH << ','
            << (r.solved ? std::to_string(r.depth) : "-") << ','
            << r.nodesExpanded << ','
            << std::fixed << std::setprecision(2) << r.timeMs << ','
            << (r.solved ? "sim" : "nao") << ','
            << (r.limitHit ? "sim" : "nao") << '\n';
    }
}

// ─── Relatorio Markdown ─────────────────────────────────────────────────────────

static void writeReport(const std::vector<SearchResult>& res8,
                        const std::vector<SearchResult>& res15,
                        const fs::path& path,
                        const std::string& algo8,
                        const std::string& algo15) {
    std::ofstream out(path);
    if (!out) return;

    auto block = [&](const std::vector<SearchResult>& rs,
                     const std::string& label, const std::string& algo) {
        int solved = 0, limitHit = 0, unsolvable = 0;
        long long totalNodes = 0;
        double totalTime = 0.0;
        long long sumDepth = 0;
        int minD = INT_MAX, maxD = 0;

        for (const auto& r : rs) {
            if (r.unsolvable) { ++unsolvable; continue; }
            totalNodes += r.nodesExpanded; totalTime += r.timeMs;
            if (r.solved) {
                ++solved; sumDepth += r.depth;
                minD = std::min(minD, r.depth); maxD = std::max(maxD, r.depth);
            }
            if (r.limitHit) ++limitHit;
        }

        out << "## " << label << " — " << algo << "\n\n";
        out << "- Instancias: " << rs.size()
            << "  |  Resolvidas: " << solved
            << "  |  Limite: " << limitHit
            << "  |  Insoluveis: " << unsolvable << "\n";
        if (solved > 0)
            out << "- Profundidade: min=" << minD << "  max=" << maxD
                << "  media=" << std::fixed << std::setprecision(1)
                << (double)sumDepth / solved << "\n";
        out << "- Total nos: " << totalNodes
            << "  |  Tempo total: " << std::setprecision(0) << totalTime << " ms\n\n";

        out << "| ID | h₀ | Prof. | Nos | Tempo (ms) | Status |\n|---:|---:|---:|---:|---:|:---|\n";
        for (const auto& r : rs) {
            std::string st = r.solved ? "OK" : (r.limitHit ? "LIMITE" : (r.unsolvable ? "INSOLUVEL" : "FALHOU"));
            out << '|' << r.id << '|' << r.initialH
                << '|' << (r.solved ? std::to_string(r.depth) : "-")
                << '|' << r.nodesExpanded
                << '|' << std::fixed << std::setprecision(1) << r.timeMs
                << '|' << st << "|\n";
        }
        out << "\n";
    };

    out << "# Relatorio — 8-puzzle e 15-puzzle\n\n";
    out << "- Algoritmos: A* e IDA* com heuristica Manhattan + Conflito Linear\n";
    out << "- Tabela hash: `unordered_map<uint64_t, GNode>` — O(1) amortizado\n";
    out << "- Estado codificado como `uint64_t` (4 bits por celula)\n\n";

    block(res8,  "8-puzzle  (3x3)", algo8);
    block(res15, "15-puzzle (4x4)", algo15);
}

// ─── main ───────────────────────────────────────────────────────────────────────

int main(int argc, char** argv) {
    try {
        fs::path file8  = "8puzzle_instances.txt";
        fs::path file15 = "15puzzle_instances.txt";
        fs::path outDir = "output";

        for (int i = 1; i < argc; ++i) {
            std::string a = argv[i];
            if      (a == "--8puzzle"  && i + 1 < argc) file8  = argv[++i];
            else if (a == "--15puzzle" && i + 1 < argc) file15 = argv[++i];
            else if (a == "--output"   && i + 1 < argc) outDir = argv[++i];
            else throw std::runtime_error("Argumento invalido: " + a);
        }

        fs::create_directories(outDir);
        fs::path detailDir = outDir / "detalhes";

        auto printSep = [](char c = '=', int n = 60) {
            for (int i = 0; i < n; ++i) std::cout << c;
            std::cout << '\n';
        };
        auto printRow = [](const SearchResult& r) {
            std::string status = r.solved    ? "prof=" + std::to_string(r.depth)
                               : r.limitHit  ? "LIMITE"
                               : r.unsolvable ? "INSOLUVEL"
                               : "FALHOU";
            std::cout << "| #" << std::setw(3) << r.id
                      << " | " << std::setw(12) << std::left << status << std::right
                      << " | " << std::setw(9) << r.nodesExpanded
                      << " | " << std::setw(8) << std::fixed << std::setprecision(1)
                      << r.timeMs << " ms |\n";
        };

        // ──────────────────────────────────────────────────────────────────
        // 8-puzzle — A*
        // ──────────────────────────────────────────────────────────────────
        printSep();
        std::cout << "  8-PUZZLE — A* (Manhattan + Conflito Linear)\n";
        std::cout << "  Goal: [1,2,...,8,0]  |  Limite: " << ASTAR_NODE_LIMIT << " nos\n";
        printSep();
        auto inst8 = loadInstances(file8, 3);
        std::cout << "  Instancias carregadas: " << inst8.size() << "\n";
        printSep('-');
        std::cout << "| ID   | Status       | Nos gerados | Tempo     |\n";
        printSep('-');

        std::vector<SearchResult> res8;
        res8.reserve(inst8.size());
        int solved8 = 0;
        long long sumD8 = 0;
        int minD8 = INT_MAX, maxD8 = 0;

        for (const auto& inst : inst8) {
            auto r = solveAStar(inst.id, inst.tiles, 3);
            res8.push_back(r);
            printRow(r);
            if (r.solved) {
                ++solved8; sumD8 += r.depth;
                minD8 = std::min(minD8, r.depth); maxD8 = std::max(maxD8, r.depth);
                writeDetailFile(r, 3, "astar_8p", detailDir);
            }
        }
        printSep('-');
        std::cout << "  Resolvidas: " << solved8 << "/" << inst8.size()
                  << "  |  Prof. min=" << minD8 << " max=" << maxD8
                  << " media=" << std::fixed << std::setprecision(1)
                  << (solved8 > 0 ? (double)sumD8 / solved8 : 0.0) << "\n";
        printSep();

        // ──────────────────────────────────────────────────────────────────
        // 8-puzzle — IDA*
        // ──────────────────────────────────────────────────────────────────
        std::cout << "\n";
        printSep();
        std::cout << "  8-PUZZLE — IDA* (Manhattan + Conflito Linear)\n";
        std::cout << "  Memoria: O(profundidade)  |  Limite: " << IDASTAR_NODE_LIMIT << " nos\n";
        printSep();
        printSep('-');
        std::cout << "| ID   | Status       | Nos gerados | Tempo     |\n";
        printSep('-');

        std::vector<SearchResult> res8ida;
        res8ida.reserve(inst8.size());
        int solved8i = 0;

        for (const auto& inst : inst8) {
            auto r = solveIDAStar(inst.id, inst.tiles, 3);
            res8ida.push_back(r);
            printRow(r);
            if (r.solved) {
                ++solved8i;
                writeDetailFile(r, 3, "idastar_8p", detailDir);
            }
        }
        printSep('-');
        std::cout << "  Resolvidas: " << solved8i << "/" << inst8.size() << "\n";
        printSep();

        // ──────────────────────────────────────────────────────────────────
        // 15-puzzle — A*
        // ──────────────────────────────────────────────────────────────────
        std::cout << "\n";
        printSep();
        std::cout << "  15-PUZZLE — A* (Manhattan + Conflito Linear)\n";
        std::cout << "  Goal: [0,1,...,15]  |  Limite: " << ASTAR_NODE_LIMIT << " nos\n";
        printSep();
        auto inst15 = loadInstances(file15, 4);
        std::cout << "  Instancias carregadas: " << inst15.size() << "\n";
        printSep('-');
        std::cout << "| ID   | Status       | Nos gerados | Tempo     |\n";
        printSep('-');

        std::vector<SearchResult> res15astar;
        res15astar.reserve(inst15.size());
        int solved15a = 0, limit15a = 0;
        int minD15a = INT_MAX, maxD15a = 0;
        long long sumD15a = 0;

        for (const auto& inst : inst15) {
            auto r = solveAStar(inst.id, inst.tiles, 4);
            res15astar.push_back(r);
            printRow(r);
            if (r.solved) {
                ++solved15a; sumD15a += r.depth;
                minD15a = std::min(minD15a, r.depth); maxD15a = std::max(maxD15a, r.depth);
                writeDetailFile(r, 4, "astar_15p", detailDir);
            }
            if (r.limitHit) ++limit15a;
        }
        printSep('-');
        std::cout << "  Resolvidas: " << solved15a << "/" << inst15.size()
                  << "  |  Limite: " << limit15a;
        if (solved15a > 0)
            std::cout << "  |  Prof. min=" << minD15a << " max=" << maxD15a
                      << " media=" << std::fixed << std::setprecision(1)
                      << (double)sumD15a / solved15a;
        std::cout << "\n";
        printSep();

        // ──────────────────────────────────────────────────────────────────
        // 15-puzzle — IDA*
        // ──────────────────────────────────────────────────────────────────
        std::cout << "\n";
        printSep();
        std::cout << "  15-PUZZLE — IDA* (Manhattan + Conflito Linear)\n";
        std::cout << "  Memoria: O(profundidade)  |  Limite: " << IDASTAR_NODE_LIMIT << " nos\n";
        printSep();
        printSep('-');
        std::cout << "| ID   | Status       | Nos gerados | Tempo     |\n";
        printSep('-');

        std::vector<SearchResult> res15ida;
        res15ida.reserve(inst15.size());
        int solved15i = 0, limit15i = 0;
        int minD15i = INT_MAX, maxD15i = 0;
        long long sumD15i = 0;

        for (const auto& inst : inst15) {
            auto r = solveIDAStar(inst.id, inst.tiles, 4);
            res15ida.push_back(r);
            printRow(r);
            if (r.solved) {
                ++solved15i; sumD15i += r.depth;
                minD15i = std::min(minD15i, r.depth); maxD15i = std::max(maxD15i, r.depth);
                writeDetailFile(r, 4, "idastar_15p", detailDir);
            }
            if (r.limitHit) ++limit15i;
        }
        printSep('-');
        std::cout << "  Resolvidas: " << solved15i << "/" << inst15.size()
                  << "  |  Limite: " << limit15i;
        if (solved15i > 0)
            std::cout << "  |  Prof. min=" << minD15i << " max=" << maxD15i
                      << " media=" << std::fixed << std::setprecision(1)
                      << (double)sumD15i / solved15i;
        std::cout << "\n";
        printSep();

        // ──────────────────────────────────────────────────────────────────
        // Saidas
        // ──────────────────────────────────────────────────────────────────
        writeCsv(res8,       outDir / "8puzzle_astar_results.csv",   "A*");
        writeCsv(res8ida,    outDir / "8puzzle_idastar_results.csv", "IDA*");
        writeCsv(res15astar, outDir / "15puzzle_astar_results.csv",  "A*");
        writeCsv(res15ida,   outDir / "15puzzle_idastar_results.csv","IDA*");
        writeReport(res8,       res15astar, outDir / "report_astar.md",   "A*",  "A*");
        writeReport(res8ida,    res15ida,   outDir / "report_idastar.md", "IDA*","IDA*");

        std::cout << "\nArquivos gerados em: " << outDir.string() << "\n";
        std::cout << "  8puzzle_astar_results.csv\n";
        std::cout << "  8puzzle_idastar_results.csv\n";
        std::cout << "  15puzzle_astar_results.csv\n";
        std::cout << "  15puzzle_idastar_results.csv\n";
        std::cout << "  report_astar.md  |  report_idastar.md\n";
        std::cout << "  detalhes/   <- solucao passo a passo por instancia\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Erro: " << e.what() << "\n";
        return 1;
    }
}
