#include <stdio.h>
#include <float.h>
#include <string.h>
#include <time.h>

#define N 100

int comb(int n, int k)
{
    if (k > n / 2) {
        k = n - k;
    }
    if (k == 0) {
        return 1;
    }
    if (k < 0) {
        return 0;
    }
    int r = 1;
    for (int i = 0; i < k; i++) {
        r *= (n - i);
    }
    for (int i = 2; i <= k; i++) {
        r /= i;
    }

    return r;
}

int count_kn(int edge_record[N][N])
{
    int kn = 0;
    int n = N;
    for (int i = 0; i < n - 3; i++) {
        for (int j = i + 1; j < n - 2; j++) {
            for (int p = j + 1; p < n - 1; p++) {
                for (int q = p + 1; q < n; q++) {
                    int al = edge_record[i][j] + edge_record[i][p] + edge_record[i][q] + edge_record[j][p]
                        + edge_record[j][q] + edge_record[p][q];
                    if (al == 6 || al == (6 << 3)) {
                        kn++;
                    }
                }
            }
        }
    }
    return kn;
}

void dp_move(int color_map[], int color, int edge_colors, int masks[], int bin_len, int ncolors, int next[], int drop[])
{
    if (edge_colors == 0) {
        next[0]++;
    }
    else {
        int ci = 0;
        int cs = 0;
        for (int i = 0; i < ncolors; i++) {
            int cm = edge_colors & masks[i];
            if (cm) {
                if (cs == 0) {
                    cs = color_map[i];
                    ci = cm >> (i * bin_len);
                }
                else {
                    return;
                }
            }
        }
        if (cs == color) {
            next[ci]++;
        }
        else {
            drop[ci]++;
        }
    }
}

int main()
{
    int n = N;
    int k = 4;
    int ncolors = 2;
    //kn中的边数
    int ken = comb(k, 2);
    //总边数
    int en = comb(n, 2);
    //kn个数
    int kn = comb(n, k);
    //编码颜色
    int maxn = ken;
    int bin_len = 0;
    while (maxn) {
        maxn = maxn >> 1;
        bin_len++;
    }
    int masks[ncolors];
    int color_map[ncolors];
    for (int i = 0; i < ncolors; i++) {
        color_map[i] = 1 << (i * bin_len);
        masks[i] = ((1 << bin_len) - 1) << (i * bin_len);
    }
    //预先计算常用的概率
    float p[6];
    p[5] = 1.0 / 2;
    for (int i = 4; i > 0; i--) {
        p[i] = p[i + 1] / 2;
    }
    p[0] = p[1];

    //处于不同染色状态的kn计数
    int sta_cnt[ken];
    for (int i = 0; i < ken; i++) {
        sta_cnt[i] = 0;
    }

    //边染色记录
    int edge_record[n][n];
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            edge_record[i][j] = 0;
        }
    }

    //时间记录
    clock_t start, end;
    double cpu_time_used;
    start = clock();

    //初始染色
    edge_record[0][1] = color_map[0];
    edge_record[1][0] = color_map[0];
    sta_cnt[1] = comb(n - 2, 2);
    sta_cnt[0] = kn - sta_cnt[1];
    //后续染色
    int dp_record[ncolors][2][ken];
    for (int i = 1; i < n; i++) {
        for (int j = 0; j < i; j++) {
            if (edge_record[i][j] == 0) {
                int add_line[n];
                for (int a = 0; a < n; a++) {
                    add_line[a] = edge_record[i][a] + edge_record[j][a];
                }
                for (int color = 0; color < ncolors; color++) {
                    for (int t = 0; t < 2; t++) {
                        for (int m = 0; m < ken; m++) {
                            dp_record[color][t][m] = 0;
                        }
                    }
                }
                for (int a = 0; a < n - 1; a++) {
                    if (a != i && a != j) {
                        for (int b = a + 1; b < n; b++) {
                            if (b != i && b != j) {
                                for (int c = 0; c < ncolors; c++) {
                                    int edge_colors = add_line[a] + add_line[b] + edge_record[a][b];
                                    dp_move(color_map,
                                            color_map[c],
                                            edge_colors,
                                            masks,
                                            bin_len,
                                            ncolors,
                                            dp_record[c][0],
                                            dp_record[c][1]);
                                }
                            }
                        }
                    }
                }

                float min_prob = FLT_MAX;
                int color_idx = 0;
                int sta_cnt_dp[ncolors][ken];
                for (int c = 0; c < ncolors; c++) {
                    float prob = 0;
                    for (int m = 0; m < ken; m++) {
                        sta_cnt_dp[c][m] = sta_cnt[m] - dp_record[c][0][m] - dp_record[c][1][m];
                        if (m > 0) {
                            sta_cnt_dp[c][m] += dp_record[c][0][m - 1];
                        }
                        prob += sta_cnt_dp[c][m] * p[m];
                    }
                    if (prob < min_prob) {
                        min_prob = prob;
                        color_idx = c;
                    }
                }
                edge_record[i][j] = color_map[color_idx];
                edge_record[j][i] = color_map[color_idx];
                memcpy(sta_cnt, sta_cnt_dp[color_idx], ken);
            }
        }
    }

    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

    printf("Complete! Time used: %f\n", cpu_time_used);
    printf("KN: %d\n", count_kn(edge_record));

    return 0;
}