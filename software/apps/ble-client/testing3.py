import asyncio
from bleak import discover


async def run():
    devices = await discover(adapter="hci1")
    for d in devices:
        #print("{0}: Address type: {1}".format(d, self.details["props"].get("AddressType", "Not Available")))
        print(d)
    print(len(devices))

loop = asyncio.get_event_loop()
loop.run_until_complete(run())