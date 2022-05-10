import asyncio
from example import Ticker


async def main():
    tk = Ticker()
    tk.subscribe()
    while True:
        print("Awaiting to the ticker")
        ticker = await tk.next_ticker()
        print(f"Ticker received: {ticker}")

asyncio.run(main())
