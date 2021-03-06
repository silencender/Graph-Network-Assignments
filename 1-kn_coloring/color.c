#include <stdio.h>
#include <float.h>
#include <string.h>
#include <time.h>

#define N 100

// 计算组合数
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

// 对完成染色的 kn 完全图中的 monochromatic K4 进行计数
void count_k4(int edge_record[N][N])
{
    int wkn = 0;
    int bkn = 0;
    int all = 0;
    int n = N;

    for (int i = 0; i < n - 3; i++) {
        for (int j = i + 1; j < n - 2; j++) {
            for (int p = j + 1; p < n - 1; p++) {
                for (int q = p + 1; q < n; q++) {
                    all += 1;
                    int al = edge_record[i][j] + edge_record[i][p] + edge_record[i][q] + edge_record[j][p]
                        + edge_record[j][q] + edge_record[p][q];
                    if (al == 6) {
                        wkn++;
                    }
                    if (al == (6 << 3)) {
                        bkn++;
                    }
                }
            }
        }
    }
    printf("Total K4:%d, White:%d, Black:%d \n", all, wkn, bkn);
}

// 状态转移方法
void dp_move(int color_map[], int color, int edge_colors, int masks[], int bin_len, int ncolors, int next[], int drop[])
{
    if (edge_colors == 0) {
        //子图内其它边均未染色
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
                    //子图内其它边异色
                    return;
                }
            }
        }
        //子图内其它边同色
        if (cs == color) {
            //子图内当前染色边与其他已染色边同色
            next[ci]++;
        }
        else {
            //子图内当前染色边与其他已染色边异色
            drop[ci]++;
        }
    }
}

int main()
{
    int n = N;
    int k = 4;
    int ncolors = 2;
    //目标子图的边数
    int sub_edge_cnt = comb(k, 2);
    //Kn总边数
    int edge_cnt = comb(n, 2);
    //目标子图个数
    int sub_cnt = comb(n, k);
    //编码颜色。k=4，n=2时，颜色编码为0b000001/0b001000。
    //编码后可通过SIMD得到计算加速
    int maxn = sub_edge_cnt;
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
    float p[sub_edge_cnt + 1];
    p[sub_edge_cnt] = 1.0;
    for (int i = sub_edge_cnt - 1; i > 0; i--) {
        p[i] = p[i + 1] / ncolors;
    }
    p[0] = p[1];

    //处于不同染色状态的目标子图计数
    int sta_cnt[sub_edge_cnt + 1];
    memset(sta_cnt, 0, sizeof(int) * (sub_edge_cnt + 1));

    //边染色记录
    int edge_record[n][n];
    memset(edge_record, 0, sizeof(int) * n * n);

    //时间记录
    clock_t start, end;
    double cpu_time_used;
    start = clock();

    //初始染色
    edge_record[0][1] = color_map[0];
    edge_record[1][0] = color_map[0];
    sta_cnt[1] = comb(n - 2, 2);
    sta_cnt[0] = sub_cnt - sta_cnt[1];

    //后续染色
    int dp_next[ncolors][sub_edge_cnt]; //完成一次染色后，转移状态的子图计数
    int dp_drop[ncolors][sub_edge_cnt]; //完成一次染色后，被破坏单色性的子图计数
    int add_line[n]; //辅助快速计算四点六边颜色和
    //遍历需要染色的边
    for (int i = 0; i < n - 1; i++) {
        for (int j = i + 1; j < n; j++) {
            if (edge_record[i][j] == 0) {
                for (int a = 0; a < n; a++) {
                    add_line[a] = edge_record[i][a] + edge_record[j][a];
                }
                memset(dp_next, 0, sizeof(int) * ncolors * sub_edge_cnt);
                memset(dp_drop, 0, sizeof(int) * ncolors * sub_edge_cnt);
                //检查当前染色边会影响到的K4子图
                for (int a = 0; a < n - 1; a++) {
                    if (a != i && a != j) {
                        for (int b = a + 1; b < n; b++) {
                            if (b != i && b != j) {
                                for (int c = 0; c < ncolors; c++) {
                                    int edge_colors = add_line[a] + add_line[b] + edge_record[a][b];
                                    //记录当前K4子图造成的状态转移
                                    dp_move(color_map,
                                            color_map[c],
                                            edge_colors,
                                            masks,
                                            bin_len,
                                            ncolors,
                                            dp_next[c],
                                            dp_drop[c]);
                                }
                            }
                        }
                    }
                }

                float min_prob = FLT_MAX;
                int color_idx = 0;
                int sta_cnt_dp[ncolors][sub_edge_cnt + 1];
                // 选择减少后续同色子图出现期望的染色方案
                for (int c = 0; c < ncolors; c++) {
                    float prob = 0;
                    memcpy(sta_cnt_dp[c], sta_cnt, (sub_edge_cnt + 1) * sizeof(int));
                    for (int m = 0; m < sub_edge_cnt + 1; m++) {
                        if (m > 0) {
                            sta_cnt_dp[c][m] += dp_next[c][m - 1];
                        }
                        if (m != sub_edge_cnt) {
                            sta_cnt_dp[c][m] -= dp_next[c][m] + dp_drop[c][m];
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
                memcpy(sta_cnt, sta_cnt_dp[color_idx], (sub_edge_cnt + 1) * sizeof(int));
            }
        }
    }

    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

    printf("Complete! Time used: %fs\n", cpu_time_used);
    printf("Monochromatic K%d: %d\n", k, sta_cnt[sub_edge_cnt]);
    count_k4(edge_record);
/*
    FILE* file = fopen("record.txt", "w");
    for(int i=0; i<n; i++) {
        for(int j=0; j<n; j++) {
            fprintf(file, "%d ", edge_record[i][j]);
        }
        fprintf(file, "\n");
    }
    fclose(file);
*/
    return 0;
}
