import asyncio
from time import sleep

from myeventfd import Ticker

async def main():
    def on_ticker_cb():
        print(f"New ticker received: {t.ticker_value}")

    t = Ticker(on_ticker_cb)
    await asyncio.Future()

asyncio.run(main())
