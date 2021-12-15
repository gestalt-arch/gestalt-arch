import asyncio
from bleak import BleakClient

address = "c0:98:e5:49:f0:c2"
address = "C0:98:E5:49:F0:C2"
#address = "A0:4E:CF:F1:1A:05"
MODEL_NBR_UUID = "60267643-592e-11ec-bf63-0242ac130002"

async def main(address):
    async with BleakClient(address, adapter="hci1") as client:
        model_number = await client.read_gatt_char(MODEL_NBR_UUID)
        print("Model Number: {0}".format("".join(map(chr, model_number))))


# loop = asyncio.get_event_loop()
# result = loop.run_until_complete(main(address))
asyncio.get_event_loop().run_until_complete(main(address))