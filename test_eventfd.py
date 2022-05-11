import asyncio
from time import sleep

from myeventfd import Ticker

async def main():
    def on_ticker_cb(val):
        print(f"New ticker received: {val}")

    t = Ticker(on_ticker_cb)
    await asyncio.Future()

asyncio.run(main())
