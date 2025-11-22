// 优先分支示例 - 演示如何使用从文件读取的变量列表强制分支顺序
#include "src/cadical.hpp"
#include "src/priority_branching_propagator.hpp"
#include <iostream>

using namespace std;
using namespace CaDiCaL;

int main(int argc, char** argv) {
  if (argc < 3) {
    cout << "用法: " << argv[0] << " <CNF文件> <分支列表文件> [-v]" << endl;
    cout << endl;
    cout << "分支列表文件格式:" << endl;
    cout << "  每行一个带符号的整数" << endl;
    cout << "  正数: 变量=true,  负数: 变量=false" << endl;
    cout << "  # 或 c 开头的行是注释" << endl;
    cout << endl;
    cout << "示例分支列表:" << endl;
    cout << "  # 强制分支顺序" << endl;
    cout << "  -3   # 先将变量3设为false" << endl;
    cout << "  1    # 再将变量1设为true" << endl;
    cout << "  -2   # 再将变量2设为false" << endl;
    return 1;
  }
  
  const char* cnf_file = argv[1];
  const char* branch_file = argv[2];
  bool verbose = (argc > 3 && string(argv[3]) == "-v");
  
  // 1. 创建求解器和传播器
  Solver solver;
  PriorityBranchingPropagator propagator(verbose);
  
  cout << "=== CaDiCaL 优先分支求解器 ===" << endl;
  cout << "CNF文件: " << cnf_file << endl;
  cout << "分支列表: " << branch_file << endl << endl;
  
  // 2. 读取CNF文件
  int max_var = 0;
  const char* err = solver.read_dimacs(cnf_file, max_var, 1);
  if (err) {
    cerr << "错误: 无法读取CNF文件: " << err << endl;
    return 1;
  }
  
  max_var = solver.vars();
  cout << "已加载: " << max_var << " 个变量, " 
       << solver.active() << " 个子句" << endl;
  
  // 3. 连接传播器
  solver.connect_external_propagator(&propagator);
  
  // 4. 从文件加载分支列表
  if (!propagator.load_branch_list_from_file(branch_file)) {
    cerr << "错误: 无法加载分支列表" << endl;
    solver.disconnect_external_propagator();
    return 1;
  }
  
  // 5. 将所有分支变量标记为观察变量（重要！）
  cout << "\n标记观察变量..." << endl;
  for (int lit : propagator.get_branch_on_list()) {
    int var = abs(lit);
    if (var <= max_var) {
      solver.add_observed_var(var);
      if (verbose) {
        cout << "  观察变量: " << var << endl;
      }
    } else {
      cerr << "警告: 变量 " << var << " 超出范围" << endl;
    }
  }
  
  // 6. 求解
  cout << "\n开始求解..." << endl;
  if (verbose) cout << "========================" << endl;
  
  int result = solver.solve();
  
  if (verbose) cout << "========================" << endl;
  
  // 7. 输出结果
  cout << "\n结果: ";
  if (result == 10) {
    cout << "SAT (可满足)" << endl;
    cout << "\n解:" << endl;
    cout << "v ";
    for (int i = 1; i <= max_var; i++) {
      int val = solver.val(i);
      if (val > 0) cout << i << " ";
      else if (val < 0) cout << -i << " ";
    }
    cout << "0" << endl;
    
    cout << "\n统计:" << endl;
    cout << "  优先变量总数: " << propagator.get_branch_on_list().size() << endl;
    cout << "  已处理: " << propagator.get_processed_count() << endl;
    cout << "  剩余: " << propagator.get_remaining_count() << endl;
    
  } else if (result == 20) {
    cout << "UNSAT (不可满足)" << endl;
  } else {
    cout << "UNKNOWN (未知)" << endl;
  }
  
  // 8. 断开传播器
  solver.disconnect_external_propagator();
  
  return 0;
}

