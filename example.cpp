#include <chrono>
#include <thread>
#include <iostream>

#include <pybind11/pybind11.h>
using namespace std;
namespace py = pybind11;

class Ticker {
    public:
        Ticker() {
            ticker = 0;
            loop = py::module_::import("asyncio.events").attr("get_event_loop")();
        }

        py::object async_next_ticker() {
            f = loop.attr("create_future")();
            return f;
        }

        // This should be called from another thread.
        void update_ticker(int ticker) {
            py::gil_scoped_acquire acquire;
            cout << "updating ticker" << endl;
            f.attr("set_result")(ticker);
            cout << "ticker updated" << endl;
        }

        void subscribe() {
            sub = std::thread([this] {
                cout << "subscribing" << endl;
                int ticker = 1;
                while (true) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                    update_ticker(++ticker);
                    break;
                } });
        }

    private:
        int ticker;
        py::object loop;
        py::object f;
        std::thread sub;
};

PYBIND11_MODULE(example, m) {
    py::class_<Ticker>(m, "Ticker")
        .def(py::init<>())
        .def("subscribe", &Ticker::subscribe)
        .def("next_ticker", &Ticker::async_next_ticker);
}
