#include <iostream>
#include <vector>
#include <cstdlib>

const int MAX_M = 200;

int main(int argc, char **argv) {
    // ナップザック問題を動的計画法で解決する
    std::vector<int> size = {2, 3, 5, 7, 9};  // 品物の大きさ
    std::vector<int> value = {2, 4, 7, 11, 14};  // 品物の価値
    const int N = size.size();

    if (argc != 2) {
        std::cerr << "ナップザックの大きさを入力してください" << std::endl;
        return 1;
    }

    int m = std::atoi(argv[1]);
    std::cout << "ナップザックのサイズは、 " << m << std::endl;

    if (m >= MAX_M) {
        std::cerr << "ナップザックよりも大きいmです" << std::endl;
        return 1;
    }

    std::vector<int> total(m + 1, 0);  // 合計の価値を記録
    std::vector<int> choice(m + 1, -1);  // 最後に何を選んだかを記録する

    for (int i = 0; i < N; i++) {
        for (int j = size[i]; j <= m; j++) {
            int repack_total = total[j - size[i]] + value[i];
            if (repack_total > total[j]) {
                total[j] = repack_total;
                choice[j] = i;
            }
        }

        std::cout << "i = " << i << std::endl;
        std::cout << "total = ";
        for (int j = 0; j <= m; j++)
            std::cout << total[j] << " ";
        std::cout << "\nchoice = ";
        for (int j = 0; j <= m; j++)
            std::cout << choice[j] << " ";
        std::cout << std::endl;
    }

    for (int i = m; choice[i] >= 0; i -= size[choice[i]])
        std::cout << "品物 " << choice[i] << " (価値" << value[choice[i]] << ")を詰め込む" << std::endl;

    std::cout << "価値の合計 = " << total[m] << std::endl;

    return 0;
}
