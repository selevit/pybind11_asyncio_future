#include <chrono>
#include <thread>
#include <iostream>

#include <pybind11/pybind11.h>
using namespace std;
namespace py = pybind11;

class Ticker {
    public:
        Ticker() {
            python_awaiting = false;
        }

        py::object next_ticker_async() {
            py::object loop = py::module_::import("asyncio.events").attr("get_event_loop")();
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
                        fut.attr("set_result")(ticker++);
                        cout << "[cpp] Ticker updated" << endl;
                        python_awaiting = false;
                    }
                }
            });
        }

    private:
        py::object fut;
        std::thread sub;
        bool python_awaiting;
};

PYBIND11_MODULE(example, m) {
    py::class_<Ticker>(m, "Ticker")
        .def(py::init<>())
        .def("subscribe", &Ticker::subscribe)
        .def("next_ticker", &Ticker::next_ticker_async);
}
