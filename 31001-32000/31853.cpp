#include <assert.h>
#include <cstdio>
#include <vector>
#include <algorithm>
#include <unordered_map>

using std::pair;
using std::vector;
using ll = long long;

constexpr int N = 14001;
constexpr int get_left(int x){ return (x << 1) + 1; }
constexpr int get_right(int x){ return (x << 1) + 2; }

bool compare(pair<int, int>& x, pair<int, int>& y){
	if(x.second == y.second) 
		return x.first < y.first;
	return x.second < y.second;
}

ll tree[4*N+4][2] = { 0, };

void propagate(int l, int r, int idx){
	if(tree[idx][1] == 0) return;
	if(l != r){
		tree[get_left(idx)][1] = 1;
		tree[get_right(idx)][1] = 1;
	}
	tree[idx][0] = 0;
	tree[idx][1] = 0;
}

ll update(int l, int r, int k, int idx){
	propagate(l, r, idx);
	if(!(l <= k && k <= r)) return tree[idx][0];
	if(l == r) return tree[idx][0] = 1;
	int m = (l + r) >> 1;
	return tree[idx][0] = update(l, m, k, get_left(idx)) +
					   update(m+1, r, k, get_right(idx));
}

ll query(int l, int r, int s, int e, int idx){
	propagate(l, r, idx);
	if(r <= s || e <= l) return 0;
	if(s < l && r < e) return tree[idx][0];
	int m = (l + r) >> 1;
	return query(l, m, s, e, get_left(idx)) + 
		   query(m+1, r, s, e, get_right(idx));
}


int main(){
	int n, m;
	vector<int> dict{};
	std::unordered_map<int, int> pos{};
	vector<pair<int, int>> line{};
	scanf("%d %d", &n, &m);
	for(int i = 0; i < m; i ++){
		int u, v;
		scanf("%d %d", &u, &v);
		if(u > v){
			ll tmp = u; u = v; v = tmp;
		}
		dict.push_back(u);
		dict.push_back(v);

		line.push_back({u, v});
	}
	std::sort(dict.begin(), dict.end());
	std::sort(line.begin(), line.end(), compare);

	for(int i = 0; i < dict.size(); i ++){
		pos[dict[i]] = i;
	}

	ll result = 0;
	for(int i = 2; i < line.size(); i ++){
		auto& [u, v] = line[i];
	    int u_id = pos[u];
		tree[0][1] = 1;
		for(int j = i-1; j >=0; j --){
			auto& [p, q] = line[j];
			if(!((p < u) && (u < q && q < v))) continue;
			int p_id = pos[p];
			ll pl = query(0, N-1, p_id, u_id, 0);
			result += pl;
			update(0, N-1, p_id, 0);
		}
	}
	printf("%lld\n", result);
	return 0;

}
