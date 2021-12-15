import asyncio
import time
import random

from bleak import BleakClient, BleakError

#from ble_utils import parse_ble_args, handle_sigint, LAB11
# args = parse_ble_args('Communicates with buckler over BLE')
# addr = args.addr.lower()
# timeout = args.timeout
timeout = 6.0
#handle_sigint()

## Example of how to connect to multiple BLE Peripherals

# list of ids to connect to with BLE
device_ids = [
    "c0:98:e5:49:F0:C1",
    "c0:98:e5:49:F0:C2",
    "c0:98:e5:49:F0:C3"
]

# device_ids = [
#     "c0:98:e5:49:F0:C2"
# ]
#    "c0:98:e5:49:F0:C2"

MAIN_SERVICE_UUID = "60267642-592e-11ec-bf63-0242ac130002"
RX_CHAR_UUID      = "60267643-592e-11ec-bf63-0242ac130002"
TX_CHAR_UUID      = "60267644-592e-11ec-bf63-0242ac130002"

in_file = open("solution.txt", "rb") # opening for [r]eading as [b]inary
data = in_file.read(64) # if you only wanted to read 512 bytes, do .read(512)
in_file.close()

class BLEPeripheral():
    def __init__(self, client):
        self.client = client
        for svc in client.services:
            print(f"Service: {str(svc.uuid)}")
            for char in svc.characteristics:
                print(f"    Characteristic: {str(char.uuid)}")

    async def read_data(self):
        return int.from_bytes(await self.client.read_gatt_char(TX_CHAR_UUID), byteorder='little')

    async def write_data(self):
        await self.client.write_gatt_char(RX_CHAR_UUID, data)

async def ble_connect_to(address):
    print(f"searching for device {address} ({timeout}s timeout)")
    try:
        async with BleakClient(address, adapter="hci0", timeout=timeout) as client:
            print(f"Connected to device {client.address}: {client.is_connected}")
            try:
                peripheral = BLEPeripheral(client)
                await peripheral.write_data()
                response = await peripheral.read_data()
                print(f"sent data recv {response} to {address}")
                await client.disconnect()
                device_ids.remove(address)
                await asyncio.sleep(0.1)
            except Exception as e:
                print(f"\t{e}")
    except BleakError as e:
        print("not found")

async def main(adresses):
	while (len(adresses) != 0):
	    for addr in adresses:
	        await ble_connect_to(addr)
    # running_tasks = []
    # while True:
    #     active_addrs = [task[0] for task in running_tasks]
    #     active_tasks = [task[1] for task in running_tasks]
    #     for addr in adresses:
    #         if addr not in active_addrs:
    #             task = asyncio.create_task(ble_connect_to(addr))
    #             running_tasks.append((addr, task))
    #             active_tasks.append(task)
                
    #     done, pending = await asyncio.wait(active_tasks, return_when="FIRST_COMPLETED")
    #     running_tasks = [task for task in running_tasks if task[1] in pending]


if __name__ == "__main__":
    #asyncio.run(main(device_ids))
    asyncio.get_event_loop().run_until_complete(main(device_ids))