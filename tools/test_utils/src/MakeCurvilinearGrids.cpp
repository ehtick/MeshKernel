#include <MeshKernel/Constants.hpp>
#include <MeshKernelApi/CurvilinearGrid.hpp>
#include <TestUtils/MakeCurvilinearGrids.hpp>

#include <random>

size_t CurvilinearGridCountValidNodes(meshkernelapi::CurvilinearGrid const& curvilinearGrid)
{
    size_t validNodes = 0;
    size_t index = 0;
    for (auto m = 0; m < curvilinearGrid.num_m; ++m)
    {
        for (auto n = 0; n < curvilinearGrid.num_n; ++n)
        {
            if (!meshkernel::IsEqual(curvilinearGrid.node_x[index], meshkernel::constants::missing::doubleValue))
            {
                validNodes++;
            }
            index++;
        }
    }
    return validNodes;
}

size_t CurvilinearGridCountValidNodes(meshkernel::CurvilinearGrid const& curvilinearGrid)
{
    size_t validNodes = 0;
    size_t index = 0;
    for (meshkernel::UInt n = 0; n < curvilinearGrid.NumN(); ++n)
    {
        for (meshkernel::UInt m = 0; m < curvilinearGrid.NumM(); ++m)
        {
            if (curvilinearGrid.GetNode(n, m).IsValid())
            {
                validNodes++;
            }
            index++;
        }
    }
    return validNodes;
}

std::unique_ptr<meshkernel::CurvilinearGrid> MakeSmallCurvilinearGrid()
{
    using namespace meshkernel;
    lin_alg::Matrix<Point> grid(5, 9);

    grid << //
        Point{7.998379637459554942E+04, 3.669368953805413912E+05},
        Point{8.006531634932766610E+04, 3.669913884352280875E+05},
        Point{8.014597083144872158E+04, 3.670473627646105597E+05},
        Point{8.022590004201814008E+04, 3.671046293496827129E+05},
        Point{8.030524375682926620E+04, 3.671630169196527568E+05},
        Point{8.038174798275028297E+04, 3.672241090446270537E+05},
        Point{8.045825220867129974E+04, 3.672852011696013506E+05},
        Point{8.053475643459231651E+04, 3.673462932945756475E+05},
        Point{8.061126066051333328E+04, 3.674073854195499443E+05},
        //
        Point{8.005399692539963871E+04, 3.668271786935172859E+05},
        Point{8.014473294047857053E+04, 3.668725835977831739E+05},
        Point{8.023492419500040705E+04, 3.669191881611925783E+05},
        Point{8.032467176522142836E+04, 3.669667697959434590E+05},
        Point{8.041405739198261290E+04, 3.670151484942396637E+05},
        Point{8.050509647671248240E+04, 3.670564889889827464E+05},
        Point{8.059518333982788317E+04, 3.670991234714745078E+05},
        Point{8.068433399410264974E+04, 3.671430301817245199E+05},
        Point{8.077256729947395797E+04, 3.671881834906909498E+05},
        //
        Point{8.010795455132638745E+04, 3.667094073585610604E+05},
        Point{8.019987610980382306E+04, 3.667511485812049941E+05},
        Point{8.029244910801970400E+04, 3.667925081235445105E+05},
        Point{8.038589000320057676E+04, 3.668319564031905611E+05},
        Point{8.047986028559294937E+04, 3.668702388785204384E+05},
        Point{8.058312060736966669E+04, 3.668916236319428426E+05},
        Point{8.068569967994136096E+04, 3.669153340029007522E+05},
        Point{8.078764917724052793E+04, 3.669411437571864226E+05},
        Point{8.088902409693629306E+04, 3.669688415973505471E+05},
        //
        Point{8.013393074394566065E+04, 3.666299991322114947E+05},
        Point{8.022657945494366868E+04, 3.666560012267631828E+05},
        Point{8.032039638097764691E+04, 3.666791259040951263E+05},
        Point{8.041521952488367970E+04, 3.667002404040259426E+05},
        Point{8.051096004005162104E+04, 3.667200678389416425E+05},
        Point{8.062315126642497489E+04, 3.667184374811393791E+05},
        Point{8.073580093592485355E+04, 3.667174321629589540E+05},
        Point{8.084895945629126800E+04, 3.667167463103650953E+05},
        Point{8.096213238536757126E+04, 3.667181047524145106E+05},
        //
        Point{8.015508428991910478E+04, 3.665319944788168068E+05},
        Point{8.024288669981209387E+04, 3.665448795976713882E+05},
        Point{8.033120056414291321E+04, 3.665524094749971409E+05},
        Point{8.041979925108155294E+04, 3.665552071627837140E+05},
        Point{8.050854555095927208E+04, 3.665539175055876258E+05},
        Point{8.062162400649990013E+04, 3.665473022831943817E+05},
        Point{8.073472454110847320E+04, 3.665398382516169222E+05},
        Point{8.084783925515387091E+04, 3.665315881046829163E+05},
        Point{8.096094935050149797E+04, 3.665225607507624663E+05};

    return std::make_unique<CurvilinearGrid>(grid, Projection::cartesian);
}

std::unique_ptr<meshkernel::CurvilinearGrid> MakeSmallCurvilinearGridWithMissingFaces()
{
    using namespace meshkernel;
    lin_alg::Matrix<Point> grid(5, 9);
    grid << //
        Point{7.998379637459554942E+04, 3.669368953805413912E+05},
        Point{8.006531634932766610E+04, 3.669913884352280875E+05},
        Point{8.014597083144872158E+04, 3.670473627646105597E+05},
        Point{8.022590004201814008E+04, 3.671046293496827129E+05},
        Point{8.030524375682926620E+04, 3.671630169196527568E+05},
        Point{8.038174798275028297E+04, 3.672241090446270537E+05},
        Point{8.045825220867129974E+04, 3.672852011696013506E+05},
        Point{8.053475643459231651E+04, 3.673462932945756475E+05},
        Point{8.061126066051333328E+04, 3.674073854195499443E+05},
        //
        Point{8.005399692539963871E+04, 3.668271786935172859E+05},
        Point{8.014473294047857053E+04, 3.668725835977831739E+05},
        Point{8.023492419500040705E+04, 3.669191881611925783E+05},
        Point{8.032467176522142836E+04, 3.669667697959434590E+05},
        Point{8.041405739198261290E+04, 3.670151484942396637E+05},
        Point{8.050509647671248240E+04, 3.670564889889827464E+05},
        Point{8.059518333982788317E+04, 3.670991234714745078E+05},
        Point{8.068433399410264974E+04, 3.671430301817245199E+05},
        Point{8.077256729947395797E+04, 3.671881834906909498E+05},
        //
        Point{8.010795455132638745E+04, 3.667094073585610604E+05},
        Point{8.019987610980382306E+04, 3.667511485812049941E+05},
        Point{8.029244910801970400E+04, 3.667925081235445105E+05},
        Point{8.038589000320057676E+04, 3.668319564031905611E+05},
        Point{8.047986028559294937E+04, 3.668702388785204384E+05},
        Point{8.058312060736966669E+04, 3.668916236319428426E+05},
        Point{8.068569967994136096E+04, 3.669153340029007522E+05},
        Point{8.078764917724052793E+04, 3.669411437571864226E+05},
        Point{8.088902409693629306E+04, 3.669688415973505471E+05},
        //
        Point{8.013393074394566065E+04, 3.666299991322114947E+05},
        Point{8.022657945494366868E+04, 3.666560012267631828E+05},
        Point{8.032039638097764691E+04, 3.666791259040951263E+05},
        Point{constants::missing::doubleValue, constants::missing::doubleValue},
        Point{constants::missing::doubleValue, constants::missing::doubleValue},
        Point{8.062315126642497489E+04, 3.667184374811393791E+05},
        Point{8.073580093592485355E+04, 3.667174321629589540E+05},
        Point{8.084895945629126800E+04, 3.667167463103650953E+05},
        Point{8.096213238536757126E+04, 3.667181047524145106E+05},
        //
        Point{8.015508428991910478E+04, 3.665319944788168068E+05},
        Point{8.024288669981209387E+04, 3.665448795976713882E+05},
        Point{8.033120056414291321E+04, 3.665524094749971409E+05},
        Point{-9.990000000000000000E+02, -9.990000000000000000E+02},
        Point{-9.990000000000000000E+02, -9.990000000000000000E+02},
        Point{8.062162400649990013E+04, 3.665473022831943817E+05},
        Point{8.073472454110847320E+04, 3.665398382516169222E+05},
        Point{8.084783925515387091E+04, 3.665315881046829163E+05},
        Point{8.096094935050149797E+04, 3.665225607507624663E+05};

    return std::make_unique<CurvilinearGrid>(grid, Projection::cartesian);
}

std::unique_ptr<meshkernel::CurvilinearGrid> MakeCurvilinearGrid(double originX, double originY, double deltaX, double deltaY, size_t nx, size_t ny)
{
    double y = originY;

    lin_alg::Matrix<meshkernel::Point> points(ny, nx);

    for (size_t n = 0; n < ny; ++n)
    {
        double x = originX;

        for (size_t m = 0; m < nx; ++m)
        {
            points(n, m) = meshkernel::Point(x, y);
            x += deltaX;
        }

        y += deltaY;
    }

    return std::make_unique<meshkernel::CurvilinearGrid>(points, meshkernel::Projection::cartesian);
}

std::unique_ptr<meshkernel::CurvilinearGrid> MakeCurvilinearGridRand(double originX, double originY, double deltaX, double deltaY, size_t nx, size_t ny, double fraction, bool displaceBoundary)
{
    double y = originY;
    // Create a uniform distribution in 0 .. 1.
    std::uniform_real_distribution<double> distribution(0.0, 1.0);
    std::default_random_engine engine;

    lin_alg::Matrix<meshkernel::Point> points(ny, nx);

    for (size_t n = 0; n < ny; ++n)
    {
        double x = originX;
        bool onVerticalBoundary = n == 0 || n == ny - 1;

        for (size_t m = 0; m < nx; ++m)
        {
            bool onHorizontalBoundary = m == 0 || m == nx - 1;

            meshkernel::Vector displacement(distribution(engine) * fraction * deltaX,
                                            distribution(engine) * fraction * deltaY);
            meshkernel::Point meshPoint(x, y);

            if ((displaceBoundary && (onVerticalBoundary || onHorizontalBoundary)) || (!onVerticalBoundary && !onHorizontalBoundary))
            {
                meshPoint += displacement;
            }

            points(n, m) = meshPoint;
            x += deltaX;
        }

        y += deltaY;
    }

    return std::make_unique<meshkernel::CurvilinearGrid>(points, meshkernel::Projection::cartesian);
}
