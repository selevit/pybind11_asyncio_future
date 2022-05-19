import asyncio
from example import Ticker, monotonic_time
from time import sleep

async def main():
    loop = asyncio.get_running_loop()
    tk = Ticker(loop)
    tk.subscribe()
    #sleep(5)
    while True:
        print("[py] Awaiting to the ticker")
        ticker = await tk.next_ticker()
        rcv_time = monotonic_time()
        delay = rcv_time - tk.last_ticker_time
        print(f"[py] Ticker received: {ticker}, [delay: {delay}us]")

asyncio.run(main())
