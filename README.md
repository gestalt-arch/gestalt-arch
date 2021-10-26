# gestalt-arch
Decentralized architecture for loss tolerant semi-autonomous robotics

## Objective
We demonstrate a decentralized robot control architecture by which a user-controlled interface prescribes an objective to a robot network. This network is then able to complete the task safely without further guidance from the prescriber and is tolerant to losses within the network.

## Hardware 

- iClebo Kobuki - http://kobuki.yujinrobot.com/about2/
- nRF52832 - https://www.nordicsemi.com/products/nrf52832
- Berkeley Buckler - https://github.com/lab11/buckler

## Cloning

The project uses the Berkeley Buckler repo as a submodule

1. Clone as usual
```
$ git clone https://github.com/gestalt-arch/gestalt-arch.git
```

2. Pull submodule contents (this may take some time)
```
$ cd gestalt-arch/firmware
$ git submodule update --init --recursive
```

3. Optionally, run `git submodule update` with `--progress` as it may take some time and runs by default without
```
$ cd gestalt-arch/firmware
$ git submodule update --init --recursive --progress
```

## Requirements

- See Berkeley Buckler build requirements: https://github.com/lab11/buckler#software-requirements 