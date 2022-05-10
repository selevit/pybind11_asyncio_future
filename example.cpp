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

        py::object async_next_ticker() {
            py::object loop = py::module_::import("asyncio.events").attr("get_event_loop")();
            fut = loop.attr("create_future")();
            python_awaiting = true;
            return fut;
        }

        void subscribe() {
            sub = std::thread([this] {
                cout << "subscribing" << endl;
                int ticker = 1;
                while (true) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                    if (python_awaiting) {
                        cout << "updating ticker" << endl;
                        py::gil_scoped_acquire acquire;
                        fut.attr("set_result")(ticker++);
                        cout << "ticker updated" << endl;
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
        .def("next_ticker", &Ticker::async_next_ticker);// py::return_value_policy::reference);
}
