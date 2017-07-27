//#include <catch.hpp>
//#include "gurobi_c++.h"

//TEST_CASE("run gurobi for coins example", "[gurobi]") {
//    GRBEnv env = GRBEnv();

//    GRBModel model = GRBModel(env);

//    GRBVar x = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, "x");
//    GRBVar y = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, "y");
//    GRBVar z = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, "z");

//    model.setObjective(x + y + 2 * z, GRB_MAXIMIZE);

//    model.addConstr(x + 2 * y + 3 * z <= 4, "c0");
//    model.addConstr(x + y >= 1, "c1");

//    model.optimize();

//    std::cout << x.get(GRB_StringAttr_VarName) << " " <<
//                 x.get(GRB_DoubleAttr_X) << std::endl;
//    std::cout << y.get(GRB_StringAttr_VarName) << " " <<
//                 y.get(GRB_DoubleAttr_X) << std::endl;
//    std::cout << z.get(GRB_StringAttr_VarName) << " " <<
//                 z.get(GRB_DoubleAttr_X) << std::endl;

//    std::cout << "Obj: " << model.get(GRB_DoubleAttr_ObjVal) << std::endl;
//}
