#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <chrono>
#include <cmath>

template <typename F>
void compute(std::vector<std::vector<double>>& data, F const& fun) {
	int len = data[1].size();
	for (int j = 0; j < len; ++j) {
		data[1][j] += data[0][j];
		for (int i = 3; i < data.size(); ++i) {
			data[1][j] += data[i][j];
		}
		data[1][j] /= fun(data[0][j]);
	}
}

void access(std::vector<std::vector<double>>& data, std::vector<double> const& prec) {
	int len = data[2].size();
	for (int j = 0; j < len; ++j) {
		data[2][j] += data[0][j];
		for (int i = 3; i < data.size(); ++i) {
			data[2][j] += data[i][j];
		}
		data[2][j] *= prec[j];
	}
}

class cache_cleaner {
	std::vector<int> trash;

public:
	cache_cleaner() : trash(1<<23) {}

	void clean() {
		for (auto& v: trash) {
			++v;
		}
	}

	void dont_call() {
		for (auto& v: trash) {
			std::cout << v;
		}
	}
};

template <typename F>
void run(int n_arrays, int length, F&& fun) {
	std::vector<std::vector<double>> data(n_arrays+3);
	/*
		Data[0] is the field that would be otherwise precomputed
		data[1] is the field holding the results for the compute only option
		data[2] is the field holding the results when using the precomputed field
	*/
	std::vector<double> precomputed(length);
	cache_cleaner cc;

	for (auto& v: data) {
		v = std::vector<double>(length, 10.);
	}

	// Precompute the values instead of computing in the main loop
	std::transform(data[0].begin(), data[0].end(), precomputed.begin(), [fun](double v) {return 1/fun(v);});

	cc.clean();
	auto start_c = std::chrono::steady_clock::now();
	compute(data, fun);
	auto end_c = std::chrono::steady_clock::now();

	cc.clean();
	auto start_a = std::chrono::steady_clock::now();
	access(data, precomputed);
	auto end_a = std::chrono::steady_clock::now();

	std::chrono::duration<double> elapsed_c = end_c-start_c;
    std::cout << "elapsed time compute: " << elapsed_c.count() << "\n";
	std::chrono::duration<double> elapsed_a = end_a-start_a;
    std::cout << "elapsed time access:  " << elapsed_a.count() << "\n";

	bool ok = true;

	bool res = std::transform_reduce(data[1].begin(), data[1].end(), data[2].begin(), true, 
	std::logical_and{}, 
	[](auto a, auto b) -> bool {
		//std::cout << a << " " << b << "\n";
		return std::abs(a-b) < 1.e-7;
	}
	);

	if (res) 
		std::cout << "SUCCESS\n";
	else
		std::cout << "FAILURE\n";
}

int main(int argc, char** argv) {
	for (int i = 0; i < 20; ++i) {
	 	std::cout << "Try the inverse\n";
		run(atoi(argv[1]), atoi(argv[2]), [](double v) { return v; });
		std::cout << "Try the inverse of the exponential\n";
		run(atoi(argv[1]), atoi(argv[2]), [](double v) { return std::exp(v); });
	}
}