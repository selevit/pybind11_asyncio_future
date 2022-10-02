import asyncio
import pdb
import uvloop
from time import monotonic
uvloop.install()


class Ticker:
    __slots__ = ('_loop', '_asyncio_future_blocking', '_result', '_done_callback')

    def __init__(self, *, loop=None):
        self._loop = loop or asyncio.get_event_loop()
        self._asyncio_future_blocking = False
        self._result = None
        self._done_callback = None

    def result(self):
        return self._result

    def set_result(self, result):
        self._result = result
        self._loop.call_soon_threadsafe(self._done_callback, self)

    def add_done_callback(self, fn, *, context=None):
        self._done_callback = fn

    def __await__(self):
        if self._result is None:
            self._asyncio_future_blocking = True
            yield self
        result = self._result
        self._result = None
        return result

    __iter__ = __await__


async def run_ticker_subscr(ticker: Ticker):
    while True:
        ticker._result = monotonic()
        ticker._loop.call_soon_threadsafe(ticker._done_callback, ticker)
        await asyncio.sleep(0.1)


async def main():
    ticker = Ticker()
    asyncio.create_task(run_ticker_subscr(ticker))
    while True:
        t = await ticker
        print('Delay', int((monotonic() - t) * 1_000_000), 'us')


asyncio.run(main())
