#include <iostream>
#include <vector>
#include <warp/RBF_warp.h>

WarperRBF::WarperRBF(
    int n,
    const std::vector<ImVec2>& starts,
    const std::vector<ImVec2>& ends, 
    float sigma)
    : ImageWarper(n, starts, ends),
      sigma(sigma)
{
    // left
    Eigen::SparseMatrix<float> A(n + 3, n + 3);

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            float r = euclidean_distance(starts[i], starts[j]);
            float value = radial_basis_function(r, sigma);
            A.insert(i, j) = value;
        }
    }
    // 补充约束
    for (int i = 0; i < n; i++)
    {
        A.insert(i, n) = starts[i].x;
        A.insert(i, n + 1) = starts[i].y;
        A.insert(i, n + 2) = 1;
    }

    for (int j = 0; j < n; j++)
    {
        A.insert(n, j) = starts[j].x;
        A.insert(n + 1, j) = starts[j].y;
        A.insert(n + 2, j) = 1;
    }
    A.makeCompressed();

    // right
    Eigen::SparseMatrix<float> b(n + 3, 2);

    for (int i = 0; i < n; i++)
    {
        b.insert(i, 0) = ends[i].x;
        b.insert(i, 1) = ends[i].y;
    }
    b.makeCompressed();


    Eigen::SparseLU<Eigen::SparseMatrix<float>> solver;

    solver.compute(A);

    if (solver.info() != Eigen::Success)
    {
        std::cerr << "LU decomposition failed." << std::endl;
        return;
    }

    coef = solver.solve(b);

    // 检查求解是否成功
    if (solver.info() != Eigen::Success)
    {
        std::cerr << "Linear system solving failed." << std::endl;
        return;
    }
}

std::pair<int, int> WarperRBF::warping(int x, int y) const
{
    float x_new = 0;
    float y_new = 0;
    for (int i = 0; i < n; i++)
    {
        float r = euclidean_distance(ImVec2(x, y), starts[i]);
        float value = radial_basis_function(r, sigma);
        x_new += coef.coeff(i, 0) * value;
        y_new += coef.coeff(i, 1) * value;
    }
    x_new +=
        coef.coeff(n, 0) * x + coef.coeff(n + 1, 0) * y + coef.coeff(n + 2, 0);
    y_new +=
        coef.coeff(n, 1) * x + coef.coeff(n + 1, 1) * y + coef.coeff(n + 2, 1);

    return std::pair<int, int>(x_new, y_new);
}