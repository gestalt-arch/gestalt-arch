import asyncio
from bleak import BleakClient

address = "c0:98:e5:49:f0:c2"
address = "A0:4E:CF:F1:1A:05"
MODEL_NBR_UUID = "60267643-592e-11ec-bf63-0242ac130002"

async def main(address):
    async with BleakClient(address) as client:
        model_number = await client.read_gatt_char(MODEL_NBR_UUID)
        print("Model Number: {0}".format("".join(map(chr, model_number))))

asyncio.run(main(address))