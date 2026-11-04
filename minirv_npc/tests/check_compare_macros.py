#!/usr/bin/env python3
"""Exercise minirv comparison macros with saved registers as destinations.
Run after building obj_dir/Vtop. All test artifacts go in a temporary directory.
"""
import os
from pathlib import Path
import subprocess
import tempfile

npc = Path(__file__).resolve().parents[1]
am = Path(os.environ.get('AM_HOME', npc.parent / 'abstract-machine')).resolve()
header = am / 'tools/minirv/inst-replace.h'
lut = am / 'tools/minirv/lut.bin'
# Each case sets inputs, executes a macro, and compares the destination with
# the mathematical result. Non-destination saved registers must survive.
cases = []
for rd in ('s0', 's1', 'a0'):
    for x, y in ((0, 0), (7, 7), (7, 8), (0x12345678, 0x12345679)):
        cases.append((f'li a1, {x}\nli a2, {y}\nseq {rd}, a1, a2', rd, int(x == y)))
for rd in ('s0', 's1', 'a0'):
    for x in (0, 7, -1):
        cases.append((f'li a1, {x}\nseqz {rd}, a1', rd, int(x == 0)))
cases += [('li s0, 7\nseqz s0, s0', 's0', 0),
          ('li s0, 9\nli s1, 9\nseq s0, s0, s1', 's0', 1)]
asm = [f'#include "{header}"', '.section .text', '.globl _start', '_start:', 'la sp, stack_end']
for i, (body, rd, expected) in enumerate(cases, 1):
    asm += ['li s0, 85', 'li s1, 102', body,
            f'addi a5, {rd}, {-expected}', f'bne a5, zero, fail_{i}']
    if rd != 's0':
        asm += ['addi a5, s0, -85', f'bne a5, zero, fail_{i}']
    if rd != 's1' and i != len(cases):
        asm += ['addi a5, s1, -102', f'bne a5, zero, fail_{i}']
asm += ['li a0, 0', 'ebreak']
for i in range(1, len(cases) + 1):
    asm += [f'fail_{i}:', f'li a0, {i}', 'ebreak']
asm += ['.section .bss', '.balign 16', 'stack:', '.space 4096', 'stack_end:']
with tempfile.TemporaryDirectory(prefix='minirv-compare-') as tmp:
    p = Path(tmp)
    (p / 'compare.S').write_text('\n'.join(asm) + '\n')
    subprocess.run(['riscv64-linux-gnu-gcc', '-march=rv32e_zicsr', '-mabi=ilp32e',
                    '-nostdlib', '-static', '-fno-pic', '-no-pie', '-Wl,--build-id=none',
                    '-Wl,-Ttext=0x80000000', '-Wl,-e,_start',
                    f'-D_LUT_BIN_PATH="{lut}"', str(p / 'compare.S'), '-o', str(p / 'compare.elf')], check=True)
    subprocess.run(['riscv64-linux-gnu-objcopy', '-O', 'binary', str(p / 'compare.elf'), str(p / 'compare.bin')], check=True)
    result = subprocess.run([str(npc / 'obj_dir/Vtop'), str(p / 'compare.bin')], cwd=p,
                            text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=30)
    print(result.stdout)
    if result.returncode or 'HIT GOOD TRAP' not in result.stdout or 'HIT BAD TRAP' in result.stdout or 'OUT OF BOUND' in result.stdout:
        raise SystemExit('FAIL: comparison macro regression (trap code identifies the case)')
    print(f'PASS: {len(cases)} comparison cases')
