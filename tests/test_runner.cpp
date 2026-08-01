#include <iostream>

void TestProtocolFraming();
void TestProtocolCorruptedFrame();
void TestSimulationEngineExecution();

int main()
{
    std::cout << "========================================" << std::endl;
    std::cout << " RUNNING TEMPLATE UNIT TEST SUITE " << std::endl;
    std::cout << "========================================" << std::endl;

    try
    {
        TestProtocolFraming();
        TestProtocolCorruptedFrame();
        TestSimulationEngineExecution();

        std::cout << "========================================" << std::endl;
        std::cout << " ALL TESTS PASSED SUCCESSFULLY! (3/3) " << std::endl;
        std::cout << "========================================" << std::endl;
        return 0;
    }
    catch (const std::exception& ex)
    {
        std::cerr << "[TEST FAILED] Exception: " << ex.what() << std::endl;
        return 1;
    }
    catch (...)
    {
        std::cerr << "[TEST FAILED] Unknown exception." << std::endl;
        return 1;
    }
}
