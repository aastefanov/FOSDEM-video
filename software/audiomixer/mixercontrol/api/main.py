#!/usr/bin/env python3

import os
import sys
from typing import List

parent = os.path.dirname(os.path.dirname(os.path.realpath(__file__)))
sys.path.append(parent)

import argparse

from fastapi import FastAPI

from osc import OSCController
from osc import helpers
#
# parser = argparse.ArgumentParser()
# parser.add_argument('device')
# args = parser.parse_args()

app = FastAPI()
osc = OSCController('/dev/tty_fosdem_audio_ctl')


@app.get("/")
async def root() -> List[List[float]]:
    return osc.get_matrix()


@app.get("/channels")
async def get_channels() -> List[str]:
    return osc.inputs


@app.get("/buses")
async def get_buses() -> List[str]:
    return osc.outputs


@app.get("/{channel}/{bus}")
async def get_gain(channel: str, bus: str) -> float:
    channel = helpers.parse_channel(osc, channel)
    bus = helpers.parse_bus(osc, bus)
    return osc.get_gain(channel, bus)


@app.post("/{channel}/{bus}")
@app.put("/{channel}/{bus}")
@app.get("/{channel}/{bus}/{level}")
async def set_gain(channel: str, bus: str, level: str) -> None:
    channel = helpers.parse_channel(osc, channel)
    bus = helpers.parse_bus(osc, bus)
    level = helpers.parse_level(osc, level)

    osc.set_gain(channel, bus, level)
