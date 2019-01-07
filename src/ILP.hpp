#ifndef GUROBI_ILP_HPP_
#define GUROBI_ILP_HPP_

//#define ILP_REPORT_DETAILS 1

#include <functional>
#include "gurobi_c++.h"
#include <string>
#include <sstream>
#include <iostream>

#include "utils.hpp"

#define DEBUG 1

struct Logger: std::ostream {
	template<typename T>
	Logger& operator <<(const T& x) {
#if DEBUG
		std::cout << x;
		std::cout.flush();
#endif
		return *this;
	}
} mylog;

#if DEBUG
#define PRINTF printf
#else
#define PRINTF(format, args...) ((void)0)
#endif

using namespace std;

class BPA_MIP {
private:
	const size_t V;
	const vector<Edge>& E;
	const int F;
	const size_t INF_;

	void addConstraints(GRBModel& model);

	vector<Path> extract_BPs(GRBModel& model);

	void forEachLink(function<void(const Edge &link)> f) {
		for (auto l = E.begin(); l != E.end(); ++l) {
			f(*l);
		}
	}

	void forEachLinkPair(function<void(const Edge &link1, const Edge &link2)> f) {
		for (auto l1 = E.begin(); l1 != E.end(); ++l1) {
			for (auto l2 = E.begin(); l2 != E.end(); ++l2) {
				if (l1 == l2) {
					continue;
				}
				f(*l1, *l2);
			}
		}
	}

	void forEachNode(function<void(Vertex node)> f) {
		for (Vertex i = 0; i < V; ++i) {
			f(i);
		}
	}

	string l1l2(string varName, const Edge &l1, const Edge &l2) {
		ostringstream name;
		name << varName << "_" << l1.first << "," << l1.second
				<< "_" << l2.first << "," << l2.second;
		return name.str();
	}

	string l_i(string varName, const Edge &l1) {
		ostringstream name;
		name << varName << "_" << l1.first << "," << l1.second;
		return name.str();
	}

	string vl(string varName, const Vertex v, const Edge &l) {
		ostringstream name;
		name << varName << "_" << v << "_" << l.first << "," << l.second;
		return name.str();
	}
public:
	BPA_MIP(const size_t nodes, const vector<Edge>& links, const int resiliency) :
			V(nodes), E(links), F(resiliency), INF_(links.size() * 100) {
	}
	vector<Path> run();
};

void BPA_MIP::addConstraints(GRBModel& lp) {

	// objective
//	lp.set(GRB_IntAttr_ModelSense, GRB_MAXIMIZE);
//	GRBVar R = lp.addVar(0, INF_, 1, GRB_INTEGER, "Resiliency");

// BP variables (flow conservation constraints)
#if 0
	lp.set(GRB_IntAttr_ModelSense, GRB_MINIMIZE);
	const int OBJ = 1;
#else
	const int OBJ = 0;
#endif
	forEachLinkPair([&](const Edge& l1,const Edge& l2) {
		lp.addVar(0, 1, OBJ, GRB_BINARY, l1l2("D", l1,l2));
	});

	lp.update();

	forEachLink([&](const Edge& l1) {
		if(l1.first > l1.second) {
			return;
		}
		forEachNode([&](const Vertex v) {
					GRBLinExpr sum_outFlow = 0,sum_inFlow =0;
					auto s = l1.first, t=l1.second;
					forEachLink([&](const Edge& l2) {
								if(l1==l2) {
									return;
								}
								const GRBVar& D_l1l2 = lp.getVarByName(l1l2("D", l1,l2));
								if(l2.first==v) {
									sum_outFlow += D_l1l2;
								} else if(l2.second==v) {
									sum_inFlow += D_l1l2;
								}
							}); //l2
			if(v==s) {
				lp.addConstr(sum_outFlow-sum_inFlow == 1, vl("path",v,l1));
			} else if(v==t) {
				lp.addConstr(sum_outFlow - sum_inFlow == -1, vl("path",v,l1));
			} else {
				lp.addConstr(sum_outFlow - sum_inFlow == 0, vl("path",v,l1));
			}
		}); //v
}); //l1

#if 1
	// minimize sum of path lengths
	lp.set(GRB_IntAttr_ModelSense, GRB_MAXIMIZE);
	const GRBVar& maxLength = lp.addVar(0, INF_, 1, GRB_INTEGER, "pathLength");
	GRBLinExpr sum_pathlengths = 0;
	forEachLink([&](const Edge& l1) {
		GRBLinExpr l1Length=0;
		forEachLink([&](const Edge& l2) {
					if(l1==l2) {
						return;
					}
					const GRBVar& D_l1l2 = lp.getVarByName(l1l2("D", l1,l2));
					l1Length += D_l1l2;
				});
//		lp.addConstr(l1Length <= maxLength, "max_pathlength");
			sum_pathlengths += l1Length;

//		if(l1.first == 0 ) {
//			lp.addConstr(l1Length <= 2, "l1Length");
//		}
		});
	lp.addConstr(sum_pathlengths <= maxLength, "sum_pathlengths");
#endif

// cycle check constraints
	forEachLink([&](const Edge& l1) {
		const GRBVar& d_l1l1 = lp.addVar(0, INF_, 0, GRB_CONTINUOUS, l1l2("d", l1,l1));
		lp.addConstr(d_l1l1 == 0, l1l2("d",l1,l1));

		forEachLink([&](const Edge& l2) {
					if(l1==l2) {
						return;
					}
					const GRBVar& d_l1l2 = lp.addVar(0, INF_, 0, GRB_CONTINUOUS, l1l2("d", l1,l2));
					lp.addConstr(d_l1l2 >= 0, l1l2("d",l1,l2));
				});
	});

	lp.update();

	// making dependencies symmetric
	forEachLinkPair([&](const Edge& l1, const Edge& l2) {
		Edge l2_r(l2.second,l2.first);
		if(l1==l2_r) {
			return;
		}
		const GRBVar& d_l1l2 = lp.getVarByName(l1l2("d",l1,l2));
		const GRBVar& d_l1l2_r = lp.getVarByName(l1l2("d",l1,l2_r));
		lp.addConstr(d_l1l2 == d_l1l2_r, l1l2("bidirected",l1,l2));

		// prevent isolated loops
			const GRBVar& D_l1l2 = lp.getVarByName(l1l2("D", l1,l2));
			const GRBVar& D_l1l2_r = lp.getVarByName(l1l2("D", l1,l2_r));
			lp.addConstr(D_l1l2 + D_l1l2_r <= 1, l1l2("noloop",l1,l2));
		});

	forEachLink([&](const Edge& l1) {
		forEachLinkPair([&](const Edge& l2,const Edge& l3) {
					if(l1==l3 ) {
						return;
					}
					const GRBVar& D_l2l3 = lp.getVarByName(l1l2("D",l2,l3));
					const GRBVar& d_l1l2 = lp.getVarByName(l1l2("d",l1,l2));
					const GRBVar& d_l1l3 = lp.getVarByName(l1l2("d",l1,l3));
					lp.addConstr(d_l1l3 <= d_l1l2 + 1 + INF_ * (1 - D_l2l3), l1l2("d",l1,l2));
				}); //l2,l3
		}); //l1

	forEachLinkPair([&](const Edge& l1,const Edge& l2) {
		const GRBVar& d_l1l2 = lp.getVarByName(l1l2("d",l1,l2));
		const GRBVar& d_l2l1 = lp.getVarByName(l1l2("d",l2,l1));
		lp.addConstr(d_l1l2 + d_l2l1 >= F+1, l1l2("d",l1,l2));
	});
}

vector<Path> BPA_MIP::extract_BPs(GRBModel &model) {
	vector<Path> BP;
	forEachLink([&](const Edge& l1) {
//		printf("l1=%lu,%lu\n",l1.first,l1.second);
			Path p = {l1.first};
			bool hasBP = true;
			while(p.back() != l1.second && hasBP) {
				hasBP = false;
				forEachLink([&](const Edge& l2) {
							if(l1==l2) {
								return;
							}
							GRBVar D_l1l2 = model.getVarByName(l1l2("D",l1, l2));
							if(D_l1l2.get(GRB_DoubleAttr_X) > 0.0) {
//							printf("l2=%lu,%lu\n",l2.first,l2.second);
								if(l2.first == p.back()) {
									p.push_back(l2.second);
									hasBP = true;
								}
							}
						});
			}
			if(p.size() > 2) {
				BP.push_back(p);
			}
		});
	mylog << "BPs from ILP:\n";
	return BP;
}

#define ILP_REPORT_DETAILS
vector<Path> BPA_MIP::run() {
	try {
		GRBEnv env = GRBEnv();
		GRBModel model = GRBModel(env);
		model.set(GRB_IntParam_OutputFlag, false);

		clock_t t1 = clock();

		addConstraints(model);

		clock_t t2 = clock();

		model.optimize();

		t1 = duration_ms(clock() - t1);
		t2 = duration_ms(clock() - t2);
		printf("MIP model generation: %lu ms, solver: %lu ms\n", t1, t2);

		// report
		int optimstatus = model.get(GRB_IntAttr_Status);
//		PRINTF("optimstatus=%d\n", optimstatus);
		if (optimstatus == GRB_OPTIMAL) {
#ifdef ILP_REPORT_DETAILS
			int numvars = model.get(GRB_IntAttr_NumVars);
			int numconstrs = model.get(GRB_IntAttr_NumConstrs);
			mylog << "\nnumvars=" << numvars << " numconstrs=" << numconstrs;
			auto vars = model.getVars();
			for (int j = 0; j < numvars; j++) {
				GRBVar v = vars[j];
				if (v.get(GRB_DoubleAttr_X) != 0.0) {
					cout << endl << v.get(GRB_StringAttr_VarName) << " "
							<< v.get(GRB_DoubleAttr_X) << endl;
				}
			}
#endif
			double objval = model.get(GRB_DoubleAttr_ObjVal);
			PRINTF("Optimal objective: %f\n", objval);

//			model.write("debug.lp");
			return extract_BPs(model);

		} else if (optimstatus == GRB_INFEASIBLE) {
			PRINTF("Model is infeasible\n");
//			model.computeIIS();
//			model.write("IISmodel.lp");
			return {};
		}
	}
	catch (GRBException e) {
		mylog << "\nError code = " << e.getErrorCode() << '\n';
		mylog << e.getMessage() << '\n';
	} catch (...) {
		mylog << "\nException during optimization" << '\n';
	}
	return {};
}

#undef DEBUG
#endif
