#include <chrono>
#include <thread>
#include <iostream>
#include <pybind11/functional.h>
#include <pybind11/stl.h>

#include <pybind11/pybind11.h>
using namespace std;
namespace py = pybind11;

uint64_t monotonic_time()
{
    return chrono::duration_cast<chrono::microseconds>(chrono::steady_clock::now().time_since_epoch()).count();
}

#ifdef __linux__
    #include <sys/eventfd.h>
#else
    #include <unistd.h>
#endif

class EventFd {
public:
    EventFd() {
        #ifdef __linux__
            read_fd = eventfd(0, EFD_NONBLOCK);
            write_fd = read_fd;
        #else
            int p[2];
            pipe(p);
            read_fd = p[0];
            write_fd = p[1];
        #endif
    }

    void ack() {
        uint64_t tmp;
        if (read(read_fd, &tmp, sizeof(uint64_t)) == -1) {
            throw std::system_error(errno, std::generic_category(), "Failed to read from eventfd");
        }
    }

    void notify() {
        uint64_t new_event_flag = 1;
        if (write(write_fd, &new_event_flag, 1) == -1) {
            throw std::system_error(errno, std::generic_category(), "Failed to write to eventfd");
        }
    }

    int fd() {
        return read_fd;
    }

    ~EventFd() {
        close(read_fd);
        close(write_fd);
    }

private:
    int read_fd;
    int write_fd;
};

using PyTickerUpdateCallback = std::function<void()>;

class Ticker {
    public:
        void subscribe() {
            producing_thread = std::thread([this] {
                cout << "[cpp] Ticker subscribtion started" << endl;
                for (int i = 0; i < 10000; i++) {
                    ticker_value = monotonic_time();
                    event.notify();
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            });
        }

        int get_event_fd() {
            return event.fd();
        }

        std::atomic<uint64_t> ticker_value = 0;
    private:
        std::thread producing_thread;
        EventFd event;
};

PYBIND11_MODULE(myeventfd, m) {
    py::class_<Ticker>(m, "Ticker")
        .def(py::init<>())
        .def("subscribe", &Ticker::subscribe)
        .def("get_event_fd", &Ticker::get_event_fd)
        .def_property("ticker_value", [](Ticker& ticker) {
            return ticker.ticker_value.load();
        }, nullptr);
    m.def("monotonic_time", &monotonic_time);
}
