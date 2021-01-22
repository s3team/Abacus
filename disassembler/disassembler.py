#!/usr/bin/env python3

import sys
import r2pipe
import json
import os

## This script will disassemble the malweare, and find possible functions that actually do the encryption
FUN_SIZE_THRESHOLD = 1
BLOCK_SIZE_THRESHOLD = 1
CALL_THRESHOLD = 2


def serialize(data):
    _data = []
    for block in data:
        _block = {}
        block['end_addr'] = block['addr'] + block['size']
        for k, v in block.items():
            if isinstance(v, int):
                _block[k] = hex(v)
            else:
                _block[k] = v
        _data.append(_block)
    return _data


## Analyze the binary and get the fucntion information
if len(sys.argv) != 2:
    print("Usage python " + sys.argv[0] + " <Target>")
    print("Make sure you run this from the working directory")
    exit(-1)

file_name = str(sys.argv[1])
TMP_DIRECTORIES = "target_" + file_name +'/'

if not os.path.exists(TMP_DIRECTORIES):
    os.makedirs(TMP_DIRECTORIES)



r = r2pipe.open(file_name)
r.cmd('aaaa')
r.cmd('e asm.comments=false')
r.cmd('e asm.lines.bb=false')
r.cmd('e asm.bytes=false')

loc_data = TMP_DIRECTORIES + 'data.json'
r.cmd('aflj~{} > ' + loc_data)

with open(loc_data) as f:
    data = json.load(f)

data = [x for x in data if x['size'] > FUN_SIZE_THRESHOLD]
data = [x for x in data if x['outdegree'] < 2]

for fun in data:
    addr = fun["offset"]
    size = fun["size"]
    fun_name = fun['name']
    fun_cmd = "s " + str(hex(addr))
    r.cmd(fun_cmd)
    disa_fun_cmd = "pdr  > " + TMP_DIRECTORIES + fun_name + '.txt'
    r.cmd(disa_fun_cmd)
    r.cmd('abj~{} > ' + TMP_DIRECTORIES + 'blocks_tmp.json')

    with open(TMP_DIRECTORIES + 'blocks_tmp.json') as f:
        blocks = json.load(f)

    blocks = [x for x in blocks if x['size'] > BLOCK_SIZE_THRESHOLD]
    for block in blocks:
        block_start = block['addr']
        block_end = block_start + block['size']

    blocks_name = TMP_DIRECTORIES + fun_name + "_blocks.json"

    with open(blocks_name, 'w') as f:
        blocks = serialize(blocks)
        json.dump(blocks, f, indent=4, sort_keys=True)
