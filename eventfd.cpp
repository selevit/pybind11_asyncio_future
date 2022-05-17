#include <chrono>
#include <thread>
#include <iostream>
#include <pybind11/functional.h>
#include <pybind11/stl.h>

#include <pybind11/pybind11.h>
using namespace std;
namespace py = pybind11;

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
        if (write(write_fd, &new_event_flag, sizeof(uint64_t)) == -1) {
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
        Ticker(PyTickerUpdateCallback const &ticker_cb) {
            py::object loop = py::module_::import("asyncio").attr("get_running_loop")();
            ticker_callback = ticker_cb;
            loop.attr("add_reader")(event.fd(), py::cpp_function([this] {
                event.ack();
                callback_busy = true;
                ticker_callback();
                callback_busy = false;
            }));
            subscribe();
        }

        void subscribe() {
            producing_thread = std::thread([this] {
                cout << "[cpp] Ticker subscribtion started" << endl;
                for (int i = 0; i < 10000; i++) {
                    ticker_value += 1;
                    event.notify();
                    //std::this_thread::sleep_for(std::chrono::microseconds(1));
                }
            });
        }

        int ticker_value = 1;
    private:
        bool callback_busy = false;
        std::thread producing_thread;
        PyTickerUpdateCallback ticker_callback;
        EventFd event;
};

PYBIND11_MODULE(myeventfd, m) {
    py::class_<Ticker>(m, "Ticker")
        .def(py::init<PyTickerUpdateCallback const &>(), py::arg("on_ticker_update"))
        .def_readonly("ticker_value", &Ticker::ticker_value);
}
