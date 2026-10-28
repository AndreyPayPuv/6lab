#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#endif

#define MAXN 100
#define LMAX 16
#define LBL  24

typedef struct {
    int n;
    char L[MAXN][LBL];
    int M[MAXN][MAXN];
} MGraph;

typedef struct {
    int n;
    char names[LMAX][LBL];
    int deg[LMAX];
    char nb[LMAX][LMAX][LBL];
} LGraph;


void printMatrix(const MGraph* g, const char* title) {
    int i, j, w = 0;
    if (title[0]) printf("%s\n", title);
    if (g->n == 0) { printf("  (пустой граф)\n"); return; }
    for (i = 0; i < g->n; i++) {
        int l = (int)strlen(g->L[i]);
        if (l > w) w = l;
    }
    w += 1;
    printf("%*s", w, "");
    for (i = 0; i < g->n; i++) printf("%*s", w, g->L[i]);
    printf("\n");
    for (i = 0; i < g->n; i++) {
        printf("%*s", w, g->L[i]);
        for (j = 0; j < g->n; j++) printf("%*d", w, g->M[i][j]);
        printf("\n");
    }
}

void printLists(const LGraph* g, const char* title) {
    int i, j;
    if (title[0]) printf("%s\n", title);
    if (g->n == 0) { printf("  (пустой граф)\n"); return; }
    for (i = 0; i < g->n; i++) {
        printf("  %s: ", g->names[i]);
        if (g->deg[i] == 0) printf("-");
        for (j = 0; j < g->deg[i]; j++)
            printf("%s%s", j ? ", " : "", g->nb[i][j]);
        printf("\n");
    }
}


int readLine(char* buf, int size) {
    if (!fgets(buf, size, stdin)) return 0;
    buf[strcspn(buf, "\r\n")] = '\0';
    return 1;
}

int askInt(const char* prompt, int lo, int hi) {
    char line[256];
    for (;;) {
        int x;
        printf("%s", prompt);
        fflush(stdout);
        if (!readLine(line, sizeof line)) { printf("\nВвод завершён.\n"); exit(1); }
        if (sscanf(line, "%d", &x) == 1 && x >= lo && x <= hi) return x;
        printf("  Введите целое число от %d до %d.\n", lo, hi);
    }
}

int askVertex(const char* name, int n) {
    char p[64];
    sprintf(p, "  Вершина %s (1..%d): ", name, n);
    return askInt(p, 1, n) - 1;
}


void numLabels(MGraph* g, int n) {
    int i;
    g->n = n;
    for (i = 0; i < n; i++) sprintf(g->L[i], "%d", i + 1);
}

void genGraph(MGraph* g, int n, double p) {
    int i, j;
    numLabels(g, n);
    memset(g->M, 0, sizeof g->M);
    for (i = 0; i < n; i++)
        for (j = i; j < n; j++)
            if (rand() / (RAND_MAX + 1.0) < p) g->M[i][j] = g->M[j][i] = 1;
}

void toLists(const MGraph* g, LGraph* a) {
    int i, j;
    a->n = g->n;
    for (i = 0; i < g->n; i++) {
        strcpy(a->names[i], g->L[i]);
        a->deg[i] = 0;
        for (j = 0; j < g->n; j++)
            if (g->M[i][j]) strcpy(a->nb[i][a->deg[i]++], g->L[j]);
    }
}

void identifyM(const MGraph* g, int u, int v, MGraph* r) {
    int keep[MAXN], Q[MAXN], nq = 0, m = 0, i, a, b, k;
    for (i = 0; i < g->n; i++) {
        if (i == u || i == v) continue;
        keep[m++] = i;
        if (g->M[u][i] || g->M[v][i]) Q[nq++] = m - 1;
    }
    r->n = m + 1;
    memset(r->M, 0, sizeof r->M);
    for (a = 0; a < m; a++) {
        strcpy(r->L[a], g->L[keep[a]]);
        for (b = 0; b < m; b++) r->M[a][b] = g->M[keep[a]][keep[b]];
    }
    strcpy(r->L[m], "z");
    for (k = 0; k < nq; k++) r->M[Q[k]][m] = r->M[m][Q[k]] = 1;
    if (g->M[u][v] || g->M[u][u] || g->M[v][v]) r->M[m][m] = 1;
}

int contractM(const MGraph* g, int u, int v, MGraph* out) {
    if (u == v || !g->M[u][v]) return 0;
    identifyM(g, u, v, out);
    out->M[out->n - 1][out->n - 1] = 0;
    return 1;
}

void splitM(const MGraph* g, int v, MGraph* r) {
    int keep[MAXN], m = 0, i, a, b;
    for (i = 0; i < g->n; i++) if (i != v) keep[m++] = i;
    r->n = m + 2;
    memset(r->M, 0, sizeof r->M);
    for (a = 0; a < m; a++) {
        strcpy(r->L[a], g->L[keep[a]]);
        for (b = 0; b < m; b++) r->M[a][b] = g->M[keep[a]][keep[b]];
    }
    sprintf(r->L[m], "%s'", g->L[v]);
    sprintf(r->L[m + 1], "%s''", g->L[v]);
    r->M[m][m + 1] = r->M[m + 1][m] = 1;
    for (a = 0; a < m; a++) {
        if (!g->M[v][keep[a]]) continue;
        r->M[a][m] = r->M[m][a] = 1;
        r->M[a][m + 1] = r->M[m + 1][a] = 1;
    }
    if (g->M[v][v]) {
        r->M[m][m] = 1;
        r->M[m + 1][m + 1] = 1;
    }
}


int findIdx(const LGraph* g, const char* name) {
    int i;
    for (i = 0; i < g->n; i++) if (strcmp(g->names[i], name) == 0) return i;
    return -1;
}

void identifyL(const LGraph* g, int ui, int vi, LGraph* r) {
    int inQ[LMAX] = { 0 }, i, k, p;
    for (k = 0; k < g->deg[ui]; k++) inQ[findIdx(g, g->nb[ui][k])] = 1;
    for (k = 0; k < g->deg[vi]; k++) inQ[findIdx(g, g->nb[vi][k])] = 1;
    int loopZ = inQ[ui] || inQ[vi];
    inQ[ui] = inQ[vi] = 0;
    r->n = 0;
    for (i = 0; i < g->n; i++) {
        if (i == ui || i == vi) continue;
        p = r->n++;
        strcpy(r->names[p], g->names[i]);
        r->deg[p] = 0;
        for (k = 0; k < g->deg[i]; k++)
            if (strcmp(g->nb[i][k], g->names[ui]) != 0 &&
                strcmp(g->nb[i][k], g->names[vi]) != 0)
                strcpy(r->nb[p][r->deg[p]++], g->nb[i][k]);
        if (inQ[i]) strcpy(r->nb[p][r->deg[p]++], "z");
    }
    p = r->n++;
    strcpy(r->names[p], "z");
    r->deg[p] = 0;
    for (i = 0; i < g->n; i++)
        if (inQ[i]) strcpy(r->nb[p][r->deg[p]++], g->names[i]);
    if (loopZ) strcpy(r->nb[p][r->deg[p]++], "z");
}

int contractL(const LGraph* g, int ui, int vi, LGraph* out) {
    int k, found = 0;
    if (ui == vi) return 0;
    for (k = 0; k < g->deg[ui]; k++)
        if (strcmp(g->nb[ui][k], g->names[vi]) == 0) found = 1;
    if (!found) return 0;
    identifyL(g, ui, vi, out);
    {
        int z = out->n - 1;
        if (out->deg[z] > 0 && strcmp(out->nb[z][out->deg[z] - 1], "z") == 0) out->deg[z]--;
    }
    return 1;
}

void splitL(const LGraph* g, int vi, LGraph* r) {
    char a[LBL + 4], b[LBL + 4];
    int i, k, p, pa, pb, dv = g->deg[vi];
    sprintf(a, "%s'", g->names[vi]);
    sprintf(b, "%s''", g->names[vi]);
    r->n = 0;
    for (i = 0; i < g->n; i++) {
        if (i == vi) continue;
        p = r->n++;
        strcpy(r->names[p], g->names[i]);
        r->deg[p] = 0;
        for (k = 0; k < g->deg[i]; k++) {
            if (strcmp(g->nb[i][k], g->names[vi]) != 0) {
                strcpy(r->nb[p][r->deg[p]++], g->nb[i][k]);
            }
            else {
                strcpy(r->nb[p][r->deg[p]++], a);
                strcpy(r->nb[p][r->deg[p]++], b);
            }
        }
    }
    pa = r->n++; strcpy(r->names[pa], a); r->deg[pa] = 0;
    pb = r->n++; strcpy(r->names[pb], b); r->deg[pb] = 0;
    for (k = 0; k < dv; k++) {
        if (strcmp(g->nb[vi][k], g->names[vi]) == 0) {
            strcpy(r->nb[pa][r->deg[pa]++], a);
            strcpy(r->nb[pb][r->deg[pb]++], b);
        }
        else {
            strcpy(r->nb[pa][r->deg[pa]++], g->nb[vi][k]);
            strcpy(r->nb[pb][r->deg[pb]++], g->nb[vi][k]);
        }
    }
    strcpy(r->nb[pa][r->deg[pa]++], b);
    strcpy(r->nb[pb][r->deg[pb]++], a);
}


int getv(const MGraph* g, int i, int j) {
    return (i < g->n && j < g->n) ? g->M[i][j] : 0;
}

void unionG(const MGraph* A, const MGraph* B, MGraph* r) {
    int n = A->n > B->n ? A->n : B->n, i, j;
    numLabels(r, n);
    memset(r->M, 0, sizeof r->M);
    for (i = 0; i < n; i++)
        for (j = 0; j < n; j++) r->M[i][j] = getv(A, i, j) | getv(B, i, j);
}

void intersectG(const MGraph* A, const MGraph* B, MGraph* r) {
    int n = A->n < B->n ? A->n : B->n, i, j;
    numLabels(r, n);
    memset(r->M, 0, sizeof r->M);
    for (i = 0; i < n; i++)
        for (j = 0; j < n; j++) r->M[i][j] = A->M[i][j] & B->M[i][j];
}

void ringSumG(const MGraph* A, const MGraph* B, MGraph* r) {
    static int R[MAXN][MAXN];
    int n = A->n > B->n ? A->n : B->n, keep[MAXN], m = 0, i, j, a, b;
    for (i = 0; i < n; i++)
        for (j = 0; j < n; j++) R[i][j] = getv(A, i, j) ^ getv(B, i, j);
    for (i = 0; i < n; i++) {
        int any = 0;
        for (j = 0; j < n; j++) if (R[i][j]) any = 1;
        if (any) keep[m++] = i;
    }
    r->n = m;
    memset(r->M, 0, sizeof r->M);
    for (a = 0; a < m; a++) {
        sprintf(r->L[a], "%d", keep[a] + 1);
        for (b = 0; b < m; b++) r->M[a][b] = R[keep[a]][keep[b]];
    }
}

void cartesianG(const MGraph* A, const MGraph* B, MGraph* r) {
    int n1 = A->n, n2 = B->n, N = n1 * n2, i, j, k, l;
    r->n = N;
    memset(r->M, 0, sizeof r->M);
    for (i = 0; i < n1; i++)
        for (k = 0; k < n2; k++)
            sprintf(r->L[i * n2 + k], "z%dv%d", i + 1, k + 1);
    for (i = 0; i < n1; i++)
        for (k = 0; k < n2; k++)
            for (j = 0; j < n1; j++)
                for (l = 0; l < n2; l++)
                    if ((i == j && B->M[k][l]) || (k == l && A->M[i][j]))
                        r->M[i * n2 + k][j * n2 + l] = 1;
}


void task2Matrix(const MGraph* X, int g) {
    static MGraph res;
    char title[64];
    int n = X->n, u, v;

    sprintf(title, "Исходная матрица G%d:", g + 1);
    printMatrix(X, title);

    printf("\nа) Отождествление вершин\n");
    u = askVertex("u", n);
    v = askVertex("v", n);
    if (u == v) printf("  Нужны две различные вершины.\n");
    else { identifyM(X, u, v, &res); printMatrix(&res, "  Результат:"); }

    printf("\nб) Стягивание ребра\n");
    u = askVertex("u", n);
    v = askVertex("v", n);
    if (contractM(X, u, v, &res)) printMatrix(&res, "  Результат:");
    else printf("  Ребра uv нет - стягивание невозможно.\n");

    printf("\nв) Расщепление вершины\n");
    v = askVertex("v", n);
    splitM(X, v, &res);
    printMatrix(&res, "  Результат:");
}

void task2Lists(const LGraph* Y, int g) {
    static LGraph lres;
    char title[64];
    int m = Y->n, u, v;

    sprintf(title, "Исходные списки G%d:", g + 1);
    printLists(Y, title);

    printf("\nа) Отождествление вершин\n");
    u = askVertex("u", m);
    v = askVertex("v", m);
    if (u == v) printf("  Нужны две различные вершины.\n");
    else { identifyL(Y, u, v, &lres); printLists(&lres, "  Результат:"); }

    printf("\nб) Стягивание ребра\n");
    u = askVertex("u", m);
    v = askVertex("v", m);
    if (contractL(Y, u, v, &lres)) printLists(&lres, "  Результат:");
    else printf("  Ребра uv нет - стягивание невозможно.\n");

    printf("\nв) Расщепление вершины\n");
    v = askVertex("v", m);
    splitL(Y, v, &lres);
    printLists(&lres, "  Результат:");
}

void askGraphs(int* from, int* to) {
    int c = askInt("Выберите граф для операции (1 - G1, 2 - G2, 3 - оба): ", 1, 3);
    if (c == 3) { *from = 0; *to = 1; }
    else { *from = *to = c - 1; }
}

void task2(const MGraph G[2], const LGraph A[2]) {
    int g, from, to;

    printf("\n ЗАДАНИЕ 2 \n");
    printf("\n Матричная форма \n");
    askGraphs(&from, &to);
    for (g = from; g <= to; g++) {
        printf("\n Граф G%d \n", g + 1);
        task2Matrix(&G[g], g);
    }

    printf("\n Списки смежности (*) \n");
    askGraphs(&from, &to);
    for (g = from; g <= to; g++) {
        printf("\n Граф G%d \n", g + 1);
        task2Lists(&A[g], g);
    }
}


int main(void) {
    static MGraph G[2], R, P;
    static LGraph A[2];
    int n1, n2, i, j, e = 0;

#ifdef _WIN32
    {
        const char probe[] = "З";
        UINT cp = ((unsigned char)probe[0] == 0xD0) ? 65001 : 1251;
        SetConsoleCP(cp);
        SetConsoleOutputCP(cp);
    }
#endif
    srand((unsigned)time(NULL));

    printf("\n ЗАДАНИЕ 1 \n");
    n1 = askInt("Число вершин G1 (2..10): ", 2, 10);
    n2 = askInt("Число вершин G2 (2..10): ", 2, 10);
    genGraph(&G[0], n1, 0.5);
    genGraph(&G[1], n2, 0.5);
    toLists(&G[0], &A[0]);
    toLists(&G[1], &A[1]);

    printf("\n"); printMatrix(&G[0], "Матрица смежности M1 (граф G1):");
    printf("\n"); printMatrix(&G[1], "Матрица смежности M2 (граф G2):");
    printf("\n"); printLists(&A[0], "Списки смежности G1:");
    printf("\n"); printLists(&A[1], "Списки смежности G2:");

    task2(G, A);

    printf("\n ЗАДАНИЕ 3 \n");
    printf("\n"); unionG(&G[0], &G[1], &R);     printMatrix(&R, "а) Объединение G = G1 U G2:");
    printf("\n"); intersectG(&G[0], &G[1], &R); printMatrix(&R, "б) Пересечение G = G1 n G2:");
    printf("\n"); ringSumG(&G[0], &G[1], &R);   printMatrix(&R, "в) Кольцевая сумма G = G1 (+) G2:");

    printf("\n ЗАДАНИЕ 4 \n");
    cartesianG(&G[0], &G[1], &P);
    printf("\n"); printMatrix(&P, "Декартово произведение G = G1 x G2:");
    for (i = 0; i < P.n; i++)
        for (j = i; j < P.n; j++) e += P.M[i][j];
    printf("\nВершин: %d, рёбер: %d\n", P.n, e);

#ifdef _WIN32
    printf("\nНажмите Enter для выхода...");
    fflush(stdout);
    { char tmp[16]; readLine(tmp, sizeof tmp); }
#endif
    return 0;
}