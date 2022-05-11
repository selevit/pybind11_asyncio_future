#include <chrono>
#include <thread>
#include <iostream>
#include <sys/epoll.h>
#include <sys/eventfd.h>

#include <pybind11/pybind11.h>
using namespace std;
namespace py = pybind11;


class Ticker {
    public:
        Ticker() {
            ticker_value = 1;
            event_fd = eventfd(0, EFD_NONBLOCK);

            producer_thread = std::thread([this] {
                cout << "[cpp] Ticker subscribtion started" << endl;
                uint64_t new_event_flag = 1;
                for (;;) {
                    write(event_fd, &new_event_flag, sizeof(uint64_t));
                    ticker_value += 1;
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            });
        }

        int event_fd;
        int ticker_value;
        std::thread producer_thread;
};

PYBIND11_MODULE(myeventfd, m) {
    py::class_<Ticker>(m, "Ticker")
        .def(py::init<>())
        .def_readonly("event_fd", &Ticker::event_fd)
        .def_readonly("ticker_value", &Ticker::ticker_value);
}
