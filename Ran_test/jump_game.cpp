#include <iostream>
#include <vector>
#include <algorithm>
#include <cstring>


using namespace std;

class Solution{
public:
    int jump_game(int A[], int n){
        int dp[n];
        memset(dp, 0, n * sizeof(int));   //  third param is byte
        for (int i = 0; i < n; ++i) {
            int maxPosition = min(i + A[i], n - 1);
            for (int j = i + 1; j <= maxPosition; ++j) {
                if(dp[j] == 0)
                    dp[j] = dp[i] + 1;
            }
            if(dp[n-1]) break;
        }
        return dp[n-1];
    }

    int jump_game_nospace(int A[], int n){
        int jumps = 0;
        int curEnd = 0;
        int curFarthest = 0;
        for (int i = 0; i < n - 1; ++i) {
            curFarthest = max(curFarthest, A[i] + i);
            if(i == curEnd){
                ++jumps;
                curEnd = curFarthest;
            }
        }
        return jumps;
    }
};

int main() {
    Solution a;
//    int A[] = {2, 3, 1, 1, 2};
    int A[] = {1};
//    cout<<a.jump_game(A, sizeof(A)/sizeof(int))<<endl;
    cout<<a.jump_game_nospace(A, sizeof(A)/sizeof(int))<<endl;

    return (0);
}