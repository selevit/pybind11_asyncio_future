#include <chrono>
#include <thread>
#include <iostream>

#include <pybind11/pybind11.h>
using namespace std;
namespace py = pybind11;

class Ticker {
    public:
        Ticker(py::object loop) : loop(loop)  {
            python_awaiting = false;
        }

        py::object next_ticker_async() {
            fut = loop.attr("create_future")();
            python_awaiting = true;
            cout << "[cpp] Python awaiting for the next ticker" << endl;
            return fut;
        }

        void subscribe() {
            sub = std::thread([this] {
                cout << "[cpp] Ticker subscribtion started" << endl;
                int ticker = 1;
                while (true) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                    if (python_awaiting) {
                        cout << "[cpp] Updating ticker" << endl;
                        py::gil_scoped_acquire acquire;
                        loop.attr("call_soon_threadsafe")(fut.attr("set_result"), ticker++);
                        cout << "[cpp] Ticker updated" << endl;
                        python_awaiting = false;
                    }
                }
            });
        }

    private:
        py::object loop;
        py::object fut;
        std::thread sub;
        bool python_awaiting;
};

PYBIND11_MODULE(example, m) {
    py::class_<Ticker>(m, "Ticker")
        .def(py::init<py::object>())
        .def("subscribe", &Ticker::subscribe)
        .def("next_ticker", &Ticker::next_ticker_async);
}
