#include <cstdio>
#include <complex>
#include <vector>
#include <algorithm>

using complex = std::complex<double>;
using std::vector;

const double pi = std::acos(-1);

void fft(vector<complex>& f, bool inv){
	int n = f.size();

	for(int i = 1, j = 0; i < n; i ++){
		int bit = n / 2;

		while(j >= bit){
			j -= bit;
			bit /= 2;
		}

		j += bit;
		if(i < j) std::swap(f[i], f[j]);
	}

	for(int k = 1; k < n; k <<= 1){
		double angle = inv?pi/k:-pi/k;
		complex w{cos(angle), sin(angle)};

		for(int i = 0; i < n; i += (k << 1)){
			complex wi{1, 0};

			for(int j = 0; j < k; j ++){
				complex even = f[i+j];
				complex odd = f[i+j+k];
				f[i+j] = even + wi * odd;
				f[i+j+k] = even - wi * odd;
				wi *= w;
			}
		}
	}
	if(inv)
		for(int i = 0; i < n; i++) f[i] /= n;
}

vector<int> mul(vector<int>& x, vector<int>& y){
	vector<complex> tmpx(x.begin(), x.end());
	vector<complex> tmpy(y.begin(), y.end());

	int n = 1;
	while(n <= tmpx.size() || n <= tmpy.size()) 
        n <<= 1;
    tmpx.resize(n); tmpy.resize(n);
	fft(tmpx, false); fft(tmpy, false);
	
    for(int i = 0; i < n; i ++) tmpx[i] *= tmpy[i];

	fft(tmpx, true); 
	
	vector<int> result(n);
	for(int i = 0; i < n; i ++){
		if(round(tmpx[i].real()) != 0) 
            result[i] = 1;
	}
	return result;
}

int main(){
	int n, k;
	scanf("%d %d", &n, &k);
	vector<int> x(1024);
	for(int i = 0; i < n; i ++){
		int v;
		scanf("%d", &v);
		x[v] = 1;
	}

	vector<int> r(x.begin(), x.end());
	int p = k-1;
	while(p){
		if(p & 1) r = mul(r, x);
		x = mul(x, x);
		p >>= 1;
	}

	for(int i = 0; i < r.size(); i ++){
		if(r[i] != 0)
			printf("%d ", i);
	}
	printf("\n");
	return 0;
}
