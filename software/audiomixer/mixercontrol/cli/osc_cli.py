#!/usr/bin/env python3

import os
import socket
import sys
import time

parent = os.path.dirname(os.path.dirname(os.path.realpath(__file__)))
sys.path.append(parent)

import tabulate
import click

from click_shell import make_click_shell

from osc import *
from osc import helpers

osc: OSCController


class AliasedGroup(click.Group):
    def get_command(self, ctx, cmd_name):
        rv = click.Group.get_command(self, ctx, cmd_name)
        if rv is not None:
            return rv
        matches = [x for x in self.list_commands(ctx)
                   if x.startswith(cmd_name)]
        if not matches:
            return None
        elif len(matches) == 1:
            return click.Group.get_command(self, ctx, matches[0])
        ctx.fail(f"Too many matches: {', '.join(sorted(matches))}")

    def resolve_command(self, ctx, args):
        # always return the full command name
        _, cmd, args = super().resolve_command(ctx, args)
        return cmd.name, cmd, args


def autocomplete_inputs(ctx, param, input):
    return [x for x in osc.inputs if x.lower().startswith(input.lower())]


def autocomplete_outputs(ctx, param, input):
    return [x for x in osc.inputs if x.lower().startswith(input.lower())]


@click.command(invoke_without_command=True, cls=AliasedGroup)
@click.argument('device', type=str, required=True)
@click.pass_context
def cli(ctx, device: str):
    try:
        global osc
        osc = OSCController(device)
    except OSError as e:
        click.echo('Cannot connect to device: %s' % str(e), err=True)
        sys.exit(e.errno)

    if ctx.invoked_subcommand is None:
        shell = make_click_shell(ctx,
                                 prompt='mixer@%s> ' % socket.gethostname(),
                                 hist_file=".mixer_history")

        shell.cmdloop()


# def exit_app():
#     if osc:
#         osc.close()

@cli.command()
def matrix():
    head = ['#'] + osc.outputs

    result = osc.get_matrix()

    formatted = []
    for i, line in enumerate(result):
        formatted.append([osc.inputs[i], *line])

    click.echo(tabulate.tabulate(formatted, headers=head))


@cli.command()
def channels():
    click.echo('\t'.join(osc.inputs))


@cli.command()
def buses():
    click.echo('\t'.join(osc.outputs))


@cli.command()
def info():
    click.echo('-' * 80)
    click.echo('FOSDEM Audio Control @%s' % socket.gethostname())
    click.echo('Connected to device %s' % osc.device)
    click.echo('-' * 80)


@cli.command()
@click.argument('channel', shell_complete=autocomplete_inputs)
@click.argument('bus', shell_complete=autocomplete_outputs)
def get_gain(channel: int | str, bus: int | str):
    channel = helpers.parse_channel(osc, channel)
    bus = helpers.parse_bus(osc, bus)

    click.echo(osc.get_gain(channel, bus))


@cli.command()
@click.argument('channel', shell_complete=autocomplete_inputs)
@click.argument('bus', shell_complete=autocomplete_outputs)
@click.argument('level', type=float)
def set_gain(channel, bus, level):
    channel = helpers.parse_channel(osc, channel)
    bus = helpers.parse_bus(osc, bus)
    level = helpers.parse_level(osc, level)

    osc.set_gain(channel, bus, level)


@cli.command()
def all_100():
    for i in range(0, 6):
        for j in range(0, 6):
            osc.set_gain(i, j, 1.0)
            time.sleep(0.1)


@cli.command()
def all_0():
    for i in range(0, 6):
        for j in range(0, 6):
            osc.set_gain(i, j, 0.0)
            time.sleep(0.1)


@cli.command()
def cls():
    click.clear()


# TODO: Find a better library to allow aliases in interactive mode

@cli.command()
@click.pass_context
def m(ctx):
    ctx.forward(matrix)


@cli.command()
@click.argument('channel', shell_complete=autocomplete_inputs)
@click.argument('bus', shell_complete=autocomplete_outputs)
@click.pass_context
def g(ctx, channel, bus):
    ctx.forward(get_gain)


@cli.command()
@click.argument('channel', shell_complete=autocomplete_inputs)
@click.argument('bus', shell_complete=autocomplete_outputs)
@click.argument('level', type=float)
@click.pass_context
def s(ctx, channel, bus, level):
    ctx.forward(set_gain)


if __name__ == '__main__':
    cli()
