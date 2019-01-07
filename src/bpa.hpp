/*
 * bpa.hpp
 *
 *  Created on: 6 Jan 2019
 *      Author: mahmoudp95cs
 */

#ifndef BPA_HPP_
#define BPA_HPP_
#include <cmath>

class BPA {
	int finds = 0;
	public:
	const int N;
	Graph dependency;
	Set V;
	vector<Path> BP;
	vector<string> labels;

	BPA(const int N, const int M) :
			N(N), dependency(M) {
		//	assert(N < 21);
		for (int v = 0; v < N; v++) {
			V.push_back(v);
		}
		labels.assign(dependency.V, "");
	}

	void print_BP() {
		for (Path P : BP) {
			print_path(P);
		}
	}

	void write_graphviz() {
		char name[10];
		sprintf(name, "K%d_%d.gv", N, finds);
		ofstream dotFile("gviz/" + string(name));
		dotFile << "digraph {\n";
		for (int i = 0; i < dependency.V; ++i) {
//			printf("lbl[%lu]=%s; ", i, labels[i].c_str());
			for (auto j = dependency.adj[i].begin(); j != dependency.adj[i].end(); ++j) {
				dotFile << labels[i] << "->" << labels[*j] << "\n";
			}
		}
		dotFile << "}";
		dotFile.close();
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
		char lbl[10];
		sprintf(lbl, "%lu%lu", P.front(), P.back());
		labels[e] = string(lbl);
//		printf("labels[%lu]=%s; ", e, labels[e].c_str());
		for (auto i = 0; i < P.size() - 1; ++i) {
			size_t e_i = LID(P[i], P[i + 1]);
//			printf("e=%lu;e_i=%lu; ", e, e_i);
			if (rem) {
				dependency.removeEdge(e, e_i);
			} else {
				dependency.addEdge(e, e_i);
			}
		}
	}

	void clear() {
		dependency.clear();
	}

	bool hasShortCycle(const Path& P) { // check if the new dependencies create short cycle
		size_t e = LID(P[0], P.back());
		for (auto i = 0; i < P.size() - 1; ++i) {
			size_t e_i = LID(P[i], P[i + 1]);
			if (size_t x = dependency.shortest_path(e_i, e); x < N - 2) {
				//printf("shortest_path(%lu,%lu)=%lu\n", e_i, e, x);
				return true;
			}
		}
		return false;
	}

	bool isFeasible(const Path& P) {
		auto u = P.front();
		auto v = P.back();
		auto l1 = LID(u, P[1]);
		auto l2 = LID(v, P[P.size() - 2]);
		for (Vertex i = 0; i < N; ++i) {
			if (i == u || i == v) {
				continue;
			}
			auto e_i = LID(u, i);
			if (dependency.hasArc(e_i, l1)) {
				return false;
			}

			e_i = LID(v, i);
			if (dependency.hasArc(e_i, l2)) {
				return false;
			}
		}
		return true;
	}

	void search(const vector<Edge> &links, const int i = 0) {
//		printf("run i=%lu\n", i);
		Set V1 = V;
		auto u = links[i].first;
		auto v = links[i].second;
		V1.remove(u);
		V1.remove(v);
		Powerset ps(V1);
//		bool deadend = true;
		ps.forEach([&](const Path& choice)->bool {	// for each backup path
					// Branch-cut rules
					if(u==0) {
						if( choice.size() > 1) {
							return true; // skip this choice
						}
					} else if( choice.size() > 3) {
						return true; // skip this choice
					}

					Path P = {u};
					P.insert(P.begin() + 1, choice.begin(), choice.end());
					P.push_back(v);
					updateDependencies(P, false);

					if(!isFeasible(P) || hasShortCycle(P)) { // greedy heuristic
						updateDependencies(P, true);
						return false;// continue
					}
//					deadend = false;
					BP.push_back(P);
//					print_path(P);
					if(i < links.size() - 1) {
						search(links, i+1);
						if(finds > 0) {
							return true; // terminate on first find
						}

					} else { // finished BP allocation successfully
						printf("found it!!!!!!!!!!!!!!! %d\n",++finds);
						print_BP();
						write_graphviz();
						printf("\n");
						return true;
					}
					updateDependencies(P, true);
					BP.pop_back();
					return false;
				});
#if 0
		if (deadend) { // greedy didn't work
			print_BP();
			printf(" greedy didn't work, u,v = %lu,%lu\n", u, v);
			exit(0);
		}
#endif
	}

	size_t plus1(size_t i) {
		return (i) % (N - 1) + 1;
	}
	void greedy() {
		for (auto i = 1; i < N; ++i) {
			Path P = { 0 };
			P.push_back(plus1(i));
			P.push_back(i);
			BP.push_back(P);
			print_path(P);
			updateDependencies(P, false);
		}

		for (auto i = 1; i < N; ++i) {
			for (auto l = 1; l < (N - 1) / 2 + 1; ++l) {
				auto j = (i + l - 1) % (N - 1) + 1;
				Path P;
				P.push_back(i);
				if (l == 1) {
					P.push_back(plus1(j));
					P.push_back(0);

				} else {
					P.push_back(plus1(j));
					P.push_back(0);
					P.push_back(plus1(i));
				}
				P.push_back(j);
				BP.push_back(P);
				print_path(P);
				updateDependencies(P, false);
				if (hasShortCycle(P)) {
					printf("\nhasShortCycle!\n");
					exit(0);
				}
			}
		}
		printf("%lu-resilient!\n", N - 2);
	}
};

#endif /* BPA_HPP_ */
