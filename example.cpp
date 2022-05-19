#include <chrono>
#include <thread>
#include <iostream>
#include <chrono>

#include <pybind11/pybind11.h>
using namespace std;
namespace py = pybind11;

uint64_t monotonic_time() {
    return chrono::duration_cast<chrono::microseconds>(chrono::steady_clock::now().time_since_epoch()).count();
}

class Ticker {
    public:
        Ticker(py::object loop) : loop(loop) {
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
                auto call_soon_threadsafe = loop.attr("call_soon_threadsafe");
                int ticker = 1;
                while (true) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                    if (python_awaiting) {
                        cout << "[cpp] Updating ticker" << endl;
                        ++ticker;
                        last_ticker_time = monotonic_time();
                        py::gil_scoped_acquire acquire;
                        call_soon_threadsafe(fut.attr("set_result"), ticker++);
                        cout << "[cpp] Ticker updated" << endl;
                        python_awaiting = false;
                    }
                }
            });
        }

    uint64_t last_ticker_time;
    private:
        py::object fut;
        std::thread sub;
        py::object loop;
        bool python_awaiting;
};

PYBIND11_MODULE(example, m) {
    py::class_<Ticker>(m, "Ticker")
        //.def(py::init<>())
        .def(py::init<py::object>())
        .def("subscribe", &Ticker::subscribe)
        .def_readonly("last_ticker_time", &Ticker::last_ticker_time)
        .def("next_ticker", &Ticker::next_ticker_async);

    m.def("monotonic_time", &monotonic_time);
}
