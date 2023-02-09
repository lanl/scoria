#!/usr/bin/env python
import contextlib
import subprocess

def kill_process(process):
    if process.poll() is None:
        process.kill()

def run_test_bw(fnames, bind_options, N):
    with contextlib.ExitStack() as stack:
        processes = []
        for program, bindings, outfile in zip(['./build/scoria', './build/tests/test_client'], bind_options, fnames):
            cmds = []
            if bindings != "":
                cmds.append('hwloc-bind')
                cmds.append(bindings)
            cmds.append(program)
            if program == './build/tests/test_client':
                cmds.append(str(N))
            
            print('Executing command:', ' '.join(cmds))

            if outfile:
                with open(outfile, "w") as f:
                    processes.append(stack.enter_context(subprocess.Popen(cmds, stdout=f)))
            else:
                processes.append(stack.enter_context(subprocess.Popen(cmds)))

            stack.callback(kill_process, processes[-1])    

        for process in processes:
            process.wait()
