#!/usr/bin/env python
import contextlib
import subprocess

def kill_process(process):
    if process.poll() is None:
        process.kill()

def run_test_bw(fnames, N):
    with contextlib.ExitStack() as stack:
        processes = []
        for program, outfile in zip(['./build/scoria', './build/tests/test_client'], fnames):
            if outfile:
                with open(outfile, "w") as f:
                    if program == './build/tests/test_client':
                        processes.append(stack.enter_context(subprocess.Popen([program, str(N)], stdout=f)))
                    else:
                        processes.append(stack.enter_context(subprocess.Popen(program, stdout=f)))
            else:
                processes.append(stack.enter_context(subprocess.Popen(program)))
            stack.callback(kill_process, processes[-1])    

        for process in processes:
            process.wait()
