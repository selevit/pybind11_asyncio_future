import asyncio
import ctypes.util
import selectors
from time import sleep
_libc = ctypes.CDLL(ctypes.util.find_library("c"))
from threading import Thread
from myeventfd import Ticker

EFD_SEMAPHORE = 1
EFD_CLOEXEC = 0o2000000
EFD_NONBLOCK = 0o4000

loop = asyncio.get_event_loop()

async def idle():
    await asyncio.Future()


async def main():
    t = Ticker()
    fd = open(t.event_fd, 'r')

    def on_ticker():
        fd.read()
        print(f"New ticker received: {t.ticker_value}")

    loop.add_reader(t.event_fd, on_ticker)
    await idle()

loop.run_until_complete(main())
