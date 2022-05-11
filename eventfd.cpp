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
            int buf[2];
            pipe(buf);
            read_fd = buf[0];
            write_fd = buf[1];
        #endif
    }

    void clear() {
        uint8_t buf[8];
        read(read_fd, &buf, sizeof(uint64_t));
    }

    void notify() {
        uint64_t new_event_flag = 1;
        write(write_fd, &new_event_flag, sizeof(uint64_t));
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

using PyTickerCallback = std::function<void(int)>;

class Ticker {
    public:
        Ticker(PyTickerCallback const& ticker_cb) {
            ticker_value = 1;
            ticker_callback = ticker_cb;
            py::object loop = py::module_::import("asyncio").attr("get_running_loop")();
            loop.attr("add_reader")(event.fd(), py::cpp_function([this] {
                event.clear();
                this->ticker_callback(ticker_value);
            }));
            subscribe();
        }

        void subscribe() {
            producing_thread = std::thread([this] {
                cout << "[cpp] Ticker subscribtion started" << endl;
                while (true) {
                    ticker_value += 1;
                    event.notify();
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            });
        }

        int ticker_value;
    private:
        std::thread producing_thread;
        PyTickerCallback ticker_callback;
        EventFd event;
};

PYBIND11_MODULE(myeventfd, m) {
    py::class_<Ticker>(m, "Ticker")
        .def(py::init<PyTickerCallback const&>(), py::arg("ticker_callback"))
        .def_readonly("ticker_value", &Ticker::ticker_value);
}
