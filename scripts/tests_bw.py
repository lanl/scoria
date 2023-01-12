#!/usr/bin/env python
import contextlib
import subprocess

def kill_process(process):
    if process.poll() is None:
        process.kill()

with contextlib.ExitStack() as stack:
    processes = []
    for program, outfile in zip(['./build/controller', './build/tests/test_client'], [None, 'client.log']):
        if outfile:
            with open(outfile, "w") as f:
                processes.append(stack.enter_context(subprocess.Popen(program, stdout=f)))
        else:
            processes.append(stack.enter_context(subprocess.Popen(program)))
        stack.callback(kill_process, processes[-1])    

    for process in processes:
        process.wait()
