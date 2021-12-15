import asyncio
import time
import random

from bleak import BleakClient, BleakError

#from ble_utils import parse_ble_args, handle_sigint, LAB11
# args = parse_ble_args('Communicates with buckler over BLE')
# addr = args.addr.lower()
# timeout = args.timeout
timeout = 10.0
#handle_sigint()

## Example of how to connect to multiple BLE Peripherals

# list of ids to connect to with BLE
device_ids = [
    "c0:98:e5:49:F0:C2"
]
#    "c0:98:e5:49:F0:C2"

MAIN_SERVICE_UUID = "60267642-592e-11ec-bf63-0242ac130002"
RX_CHAR_UUID      = "60267643-592e-11ec-bf63-0242ac130002"
TX_CHAR_UUID      = "60267644-592e-11ec-bf63-0242ac130002"

class BLEPeripheral():
    def __init__(self, client):
        self.client = client
        for svc in client.services:
            print(f"Service: {str(svc.uuid)}")
            for char in svc.characteristics:
                print(f"    Characteristic: {str(char.uuid)}")

    async def read_data(self):
        return int.from_bytes(await self.client.read_gatt_char(TX_CHAR_UUID), byteorder='little')

    async def write_data(self, value):
        await self.client.write_gatt_char(RX_CHAR_UUID, value.to_bytes(4, byteorder='little'))

async def ble_connect_to(address):
    print(f"searching for device {address} ({timeout}s timeout)")
    try:
        async with BleakClient(address,timeout=timeout) as client:
            print(f"Connected to device {client.address}: {client.is_connected}")
            try:
                peripheral = BLEPeripheral(client)
                value = 0
                while True:
                    await peripheral.write_data(value)
                    response = await peripheral.read_data()
                    print(f"sent {str(value)} recv {response} to {address}")
                    value += 1
                    await asyncio.sleep(0.1)
            except Exception as e:
                print(f"\t{e}")
    except BleakError as e:
        print("not found")

async def main(adresses):
    running_tasks = []
    while True:
        active_addrs = [task[0] for task in running_tasks]
        active_tasks = [task[1] for task in running_tasks]
        for addr in adresses:
            if addr not in active_addrs:
                task = asyncio.create_task(ble_connect_to(addr))
                running_tasks.append((addr, task))
                active_tasks.append(task)
                
        done, pending = await asyncio.wait(active_tasks, return_when="FIRST_COMPLETED")
        running_tasks = [task for task in running_tasks if task[1] in pending]


if __name__ == "__main__":
    asyncio.run(main(device_ids))