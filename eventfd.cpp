#include <chrono>
#include <thread>
#include <iostream>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <pybind11/functional.h>
#include <pybind11/stl.h>

#include <pybind11/pybind11.h>
using namespace std;
namespace py = pybind11;


using PyTickerCallback = std::function<void(int)>;

class Ticker {
    public:
        Ticker(PyTickerCallback const& ticker_cb) {
            ticker_value = 1;
            ticker_callback = ticker_cb;
            event_fd = eventfd(0, EFD_NONBLOCK);
            py::object loop = py::module_::import("asyncio").attr("get_running_loop")();
            loop.attr("add_reader")(event_fd, py::cpp_function([this] {
                uint8_t buf[8];
                read(event_fd, &buf, sizeof(uint64_t));
                this->ticker_callback(ticker_value);
            }));
            subscribe();
        }

        void subscribe() {
            producing_thread = std::thread([this] {
                cout << "[cpp] Ticker subscribtion started" << endl;
                uint64_t new_event_flag = 1;
                while (true) {
                    write(event_fd, &new_event_flag, sizeof(uint64_t));
                    ticker_value += 1;
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            });
        }

        int event_fd;
        int ticker_value;
    private:
        std::thread producing_thread;
        PyTickerCallback ticker_callback;
};

PYBIND11_MODULE(myeventfd, m) {
    py::class_<Ticker>(m, "Ticker")
        .def(py::init<PyTickerCallback const&>(), py::arg("ticker_callback"))
        .def_readonly("event_fd", &Ticker::event_fd)
        .def_readonly("ticker_value", &Ticker::ticker_value);
}
