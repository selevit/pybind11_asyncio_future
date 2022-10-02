import os
import asyncio
import pdb
from myeventfd import Ticker, monotonic_time
import uvloop
uvloop.install()


async def main():
    loop = asyncio.get_running_loop()
    ticker = Ticker()
    ticker.subscribe()
    fd = os.fdopen(ticker.get_event_fd(), 'rb', buffering=0)
    stream_reader = asyncio.StreamReader()
    def protocol_factory():
        return asyncio.StreamReaderProtocol(stream_reader)
    transport, proto = await loop.connect_read_pipe(protocol_factory, fd)
    while True:
        await stream_reader.readexactly(1)
        delay = monotonic_time() - ticker.ticker_value
        print('delay', delay, 'us')

asyncio.run(main())
