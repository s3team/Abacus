//This is a Pin tool that will split the execution trace into per-message traces by monitoring the program's
//use of netwokring functions that read or write from sockets.
//Output:
//1. Taintdata execution traces
//2. Function Each function in the memory address

#include "pin.H"
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <string>

// #include <openssl/bn.h>
// #include <openssl/rsa.h>
// #include "../../target/openssl_0.9.7/openssl/include/openssl/bn.h"
// #include "../../target/openssl_0.9.7/openssl/crypto/rsa/rsa.h"
#include "./openssl-1.1.0f/openssl_rsa.h"

using namespace std;

bool start_ins = FALSE;
bool first_time = true;
std::map<ADDRINT, std::string> opcmap;
std::string function_name;
FILE *fp;

ofstream RTN_FP;

// We don't combine rtn_recv and rtn_send together because we need compilers try to make them incline
VOID static RTN_start(char *name, char *rtn_name, RSA *rsa)
{
    if (start_ins == false && first_time)
    {
        uint8_t value;
        // int i = 0;
        const unsigned char *key;

        cout << "Function: " << name << " found. "
             << "RTN: " << rtn_name << endl;
        start_ins = true;
        first_time = false;

        // printf("N:  neg: %d, size: %u address: %p\n", rsa->n->neg, rsa->n->top, rsa->n->d);
        // fprintf(fp, "Start; %p; %d; \n", rsa->n->d, rsa->n->top * 8 * 4);
        // key = (const unsigned char *)rsa->n->d;
        // for (int i = 0; i < rsa->n->top * 4; i++)
        // {
        //     PIN_SafeCopy(&value, key++, sizeof(uint8_t));
        //     fprintf(fp, "%x; ", value);
        //     printf("Key:  %d %x \n", i, value);
        // }
        // fprintf(fp, "\n");

        // printf("E:  neg: %d, size: %u address: %p\n", rsa->e->neg, rsa->e->top, rsa->e->d);
        // fprintf(fp, "Start; %p; %d; \n", rsa->e->d, rsa->e->top * 8 * 4);
        // key = (const unsigned char *)rsa->e->d;
        // for (int i = 0; i < rsa->e->top * 4; i++)
        // {
        //     PIN_SafeCopy(&value, key++, sizeof(uint8_t));
        //     fprintf(fp, "%x; ", value);
        //     printf("Key:  %d %x \n", i, value);
        // }
        // fprintf(fp, "\n");

        printf("P:  neg: %d, size: %u address: %p\n", rsa->p->neg, rsa->p->top, rsa->p->d);
        fprintf(fp, "Start; %p; %d; \n", rsa->p->d, rsa->p->top * 8 * 4);
        key = (const unsigned char *)rsa->p->d;
        for (int i = 0; i < rsa->p->top * 4; i++)
        {
            PIN_SafeCopy(&value, key++, sizeof(uint8_t));
            fprintf(fp, "%x; ", value);
            printf("Key:  %d %x \n", i, value);
        }
        fprintf(fp, "\n");

        printf("Q:  neg: %d, size: %u address: %p\n", rsa->q->neg, rsa->q->top, rsa->q->d);
        fprintf(fp, "Start; %p; %d; \n", rsa->q->d, rsa->q->top * 8 * 4);
        key = (const unsigned char *)rsa->q->d;
        for (int i = 0; i < rsa->q->top * 4; i++)
        {
            PIN_SafeCopy(&value, key++, sizeof(uint8_t));
            fprintf(fp, "%x; ", value);
            printf("Key:  %d %x \n", i, value);
        }
        fprintf(fp, "\n");

        printf("dmp1:  neg: %d, size: %u address: %p\n", rsa->dmp1->neg, rsa->dmp1->top, rsa->dmp1->d);
        fprintf(fp, "Start; %p; %d; \n", rsa->dmp1->d, rsa->dmp1->top * 8 * 4);
        key = (const unsigned char *)rsa->dmp1->d;
        for (int i = 0; i < rsa->dmp1->top * 4; i++)
        {
            PIN_SafeCopy(&value, key++, sizeof(uint8_t));
            fprintf(fp, "%x; ", value);
            printf("Key:  %d %x \n", i, value);
        }
        fprintf(fp, "\n");

        printf("dmq1:  neg: %d, size: %u address: %p\n", rsa->dmq1->neg, rsa->dmq1->top, rsa->dmq1->d);
        fprintf(fp, "Start; %p; %d; \n", rsa->dmq1->d, rsa->dmq1->top * 8 * 4);
        key = (const unsigned char *)rsa->dmq1->d;
        for (int i = 0; i < rsa->dmq1->top * 4; i++)
        {
            PIN_SafeCopy(&value, key++, sizeof(uint8_t));
            fprintf(fp, "%x; ", value);
            printf("Key:  %d %x \n", i, value);
        }
        fprintf(fp, "\n");

        printf("iqmp:  neg: %d, size: %u address: %p\n", rsa->iqmp->neg, rsa->iqmp->top, rsa->iqmp->d);
        fprintf(fp, "Start; %p; %d; \n", rsa->iqmp->d, rsa->iqmp->top * 8 * 4);
        key = (const unsigned char *)rsa->iqmp->d;
        for (int i = 0; i < rsa->iqmp->top * 4; i++)
        {
            PIN_SafeCopy(&value, key++, sizeof(uint8_t));
            fprintf(fp, "%x; ", value);
            printf("Key:  %d %x \n", i, value);
        }
        fprintf(fp, "\n");

        return;
    }
}

VOID static RTN_end(char *name)
{
    if (start_ins)
    {
        cout << "END "
             << "function name: " << name << endl;
        //start_ins = false;
    }
}

INT32 Usage()
{
    std::cerr << "command line error\n";
    std::cout << "pin -t MyPinTool.so -- target function_name" << std::endl;
    return -1;
}

//Only read or write register
void static getctx(ADDRINT addr, CONTEXT *fromctx, ADDRINT memaddr)
{
    //Only collect traces and a recv function
    if (start_ins == FALSE)
        return;
    fprintf(fp, "%x;%s;%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x\n", addr, opcmap[addr].c_str(),
            PIN_GetContextReg(fromctx, REG_EAX),
            PIN_GetContextReg(fromctx, REG_EBX),
            PIN_GetContextReg(fromctx, REG_ECX),
            PIN_GetContextReg(fromctx, REG_EDX),
            PIN_GetContextReg(fromctx, REG_ESI),
            PIN_GetContextReg(fromctx, REG_EDI),
            PIN_GetContextReg(fromctx, REG_ESP),
            PIN_GetContextReg(fromctx, REG_EBP),
            memaddr,
            PIN_GetContextReg(fromctx, REG_EFLAGS),
            0);
}

//Read or write memory
void static getctxRead(ADDRINT addr, CONTEXT *fromctx, ADDRINT memaddr)
{
    //Only collect traces and a recv function
    if (start_ins == FALSE)
        return;
    uint32_t value = 0;
    const VOID *src = (const VOID *)(memaddr - memaddr % 4);
    PIN_SafeCopy(&value, src, sizeof(uint32_t));
    fprintf(fp, "%x;%s;%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x\n", addr, opcmap[addr].c_str(),
            PIN_GetContextReg(fromctx, REG_EAX),
            PIN_GetContextReg(fromctx, REG_EBX),
            PIN_GetContextReg(fromctx, REG_ECX),
            PIN_GetContextReg(fromctx, REG_EDX),
            PIN_GetContextReg(fromctx, REG_ESI),
            PIN_GetContextReg(fromctx, REG_EDI),
            PIN_GetContextReg(fromctx, REG_ESP),
            PIN_GetContextReg(fromctx, REG_EBP),
            memaddr,
            PIN_GetContextReg(fromctx, REG_EFLAGS),
            value);
}

VOID static printFunctionName(char *rtn_name, char *module_name, ADDRINT startaddr, USIZE rtn_size)
{
    RTN_FP << std::hex << startaddr << "; "
           << module_name << "; "
           << rtn_name << "; "
           << std::dec << rtn_size << "\n";
}

VOID Instruction(INS ins, VOID *v)
{
    ADDRINT addr = INS_Address(ins);
    if (opcmap.find(addr) == opcmap.end())
    {
        opcmap.insert(std::pair<ADDRINT, std::string>(addr, INS_Disassemble(ins)));
    }

    if (INS_IsMemoryRead(ins))
    {
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)getctxRead, IARG_INST_PTR, IARG_CONST_CONTEXT, IARG_MEMORYREAD_EA,
                       IARG_END);
    }
    else if (INS_IsMemoryWrite(ins))
    {
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)getctxRead, IARG_INST_PTR, IARG_CONST_CONTEXT, IARG_MEMORYWRITE_EA,
                       IARG_END);
    }
    else
    {
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)getctx, IARG_INST_PTR, IARG_CONST_CONTEXT, IARG_ADDRINT, 0,
                       IARG_END);
    }
}

VOID Routine(RTN rtn, VOID *v)
{
    ADDRINT rtn_start = RTN_Address(rtn);
    string rtn_name = RTN_Name(rtn);
    string img_name = IMG_Name(IMG_FindByAddress(rtn_start));
    RTN_Open(rtn);
    RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)printFunctionName,
                   IARG_PTR, RTN_Name(rtn).c_str(),
                   IARG_PTR, IMG_Name(IMG_FindByAddress(rtn_start)).c_str(),
                   IARG_ADDRINT, rtn_start,
                   IARG_UINT32, RTN_Size(rtn),
                   IARG_END);

    if (rtn_name.find(function_name) == std::string::npos)
    {
        RTN_Close(rtn);
        return;
    }
    if (rtn_name.find("@plt") != std::string::npos)
    {
        RTN_Close(rtn);
        return;
    }

    RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)RTN_start,
                   IARG_ADDRINT, function_name.c_str(),
                   IARG_ADDRINT, IMG_Name(IMG_FindByAddress(rtn_start)).c_str(),
                   IARG_FUNCARG_ENTRYPOINT_VALUE, 3,
                   IARG_END);

    RTN_InsertCall(rtn, IPOINT_AFTER, (AFUNPTR)RTN_end,
                   IARG_PTR, RTN_Name(rtn).c_str(), IARG_END);

    RTN_Close(rtn);
}

static void Fini(INT32, void *v)
{
    fclose(fp);
    RTN_FP.close();
}

int main(int argc, char *argv[])
{
    // Initialize pin & symbol manager
    if (PIN_Init(argc, argv))
    {
        return Usage();
    }
    function_name = "RSA_private_decrypt";
    // function_name = argv[argc - 1];

    fp = fopen("Inst_data.txt", "w");
    RTN_FP.open("Function.txt");
    PIN_InitSymbols();
    // Register Image to be called to instrument functions.
    RTN_AddInstrumentFunction(Routine, 0);
    INS_AddInstrumentFunction(Instruction, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}
