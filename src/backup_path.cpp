#include <iostream>
#include <bitset>
#include <algorithm>
#include <vector>
#include <iterator>     // std::ostream_iterator
#include <functional>

#include "utils.hpp"

using namespace std;

int test() {
	// Create a graph given in the above diagram
	Graph g(4);
	g.addEdge(0, 1);
	g.addEdge(0, 2);
	g.addEdge(1, 2);
	g.addEdge(2, 0);
	g.addEdge(2, 3);
	g.addEdge(3, 3);

	printf("SP(0,3)=%d\n", g.shortest_path(0, 3));
	Set U;
	Powerset ps( { 1, 2, 3 });
	ps.forEach([](const Set& set) {
		std::ostream_iterator<size_t> out_it(std::cout, ",");
		std::copy(set.begin(), set.end(), out_it);
		cout << '\n';
	});
	return 0;
}

class BPA {
public:
	const int N;
	Graph g;
	Set V;
	vector<Path> BP;

	BPA(const int N) :
			N(N), g(choose(N, 2)) {
		for (int v = 0; v < N; v++) {
			V.push_back(v);
		}
	}

	void print_BP() {
		for (Path P : BP) {
			print_path(P);
		}
	}

	size_t LID(Vertex v, Vertex w) { // create unique ID for link (v,w)
		if (v > w) {
			swap(v, w);
		}
//		printf("LID(%lu,%lu)=%lu\n", v, w, sum(N - v, N - 1) + w - v - 1);
		return sum(N - v, N - 1) + w - v - 1;
	}

	void updateDependencies(const Path P, bool rem) {
		size_t e = LID(P.front(), P.back());
		for (auto i = 0; i < P.size() - 1; ++i) {
			size_t e_i = LID(P[i], P[i + 1]);
//			printf("e=%lu;e_i=%lu; ", e, e_i);
			if (rem) {
				g.removeEdge(e, e_i);
			} else {
				g.addEdge(e, e_i);
			}
		}
	}

	bool hasShortCycle(const Path& P) { // check if the new dependencies create short cycle
		size_t e = LID(P[0], P.back());
		for (auto i = 0; i < P.size() - 1; ++i) {
			size_t e_i = LID(P[i], P[i + 1]);
			if (size_t x = g.shortest_path(e_i, e); x < N - 2) {
				//printf("shortest_path(%lu,%lu)=%lu\n", e_i, e, x);
				return true;
			}
		}
		return false;
	}

	void run(const Vertex i = 0, const Vertex j = 1) {
		Set V1 = V;
		V1.remove(i);
		V1.remove(j);
		Powerset ps(V1);
		ps.forEach([&](const Set& set) {	// for each backup path
				//	printf("run i,j=%lu,%lu\n",i,j);
					Path P = {i};
					P.insert(P.begin() + 1, set.begin(), set.end());
					P.push_back(j);
					//print_path(P);
					updateDependencies(P, false);

					if(hasShortCycle(P)) {
						updateDependencies(P, true);
						return;
					}
					BP.push_back(P);
					if(j < N-1) {
						run(i, j+1);
					} else if(i < N-2) {
						run(i+1, i+2);
					} else { // finished BP allocation successfully
						cout<<"found it!!!!!!!!!!!!!!!\n";
						print_BP();
						//exit(0);
					}
					updateDependencies(P, true);
					BP.pop_back();
				});
	}
};

int main() {
	BPA bpa(6);
	bpa.run();
}
