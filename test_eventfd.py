import asyncio
from curses import nonl
import sys
from time import sleep

from myeventfd import Ticker

async def main():
    counter = 0
    def on_ticker_cb():
        nonlocal counter
        counter += 1
        #print(f"New ticker received: {t.ticker_value}")
        print(counter)
    t = Ticker(on_ticker_cb)
    await asyncio.Future()

asyncio.run(main())
