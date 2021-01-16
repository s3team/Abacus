// This is a Pin tool that will split the execution trace into per-message
// traces by monitoring the program's use of netwokring functions that read or
// write from sockets. Output:
// 1. Taintdata execution traces
// 2. Function Each function in the memory address

#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <string>

#include "pin.H"

#include "openssl-1.0.2k/ec.h"

using namespace std;

bool capture_flag = false;
bool function_flag = false;
bool nonce_flag = false;
int nonce_function_counter = 2;
std::map<ADDRINT, std::string> opcmap;
std::string secret_function_name;
std::string nonce_function_name;
std::string caller_function_name;
FILE *fp, *fp_nonce;

ofstream RTN_FP;

VOID static RTN_prep_start(char *name, char *rtn_name)
{
    function_flag = true;

    cout << "Function: " << name << " found. "
         << "RTN: " << rtn_name << endl;

    return;
}

VOID static RTN_prep_end(char *name)
{
    function_flag = false;

    cout << "END "
         << "function name: " << name << endl;

    return;
}

// We don't combine rtn_recv and rtn_send together becasue we need compilers try to make them incline
VOID static RTN_secret_start(char *name, char *rtn_name, EC_KEY *eckey)
{
    if (!capture_flag)
    {
        capture_flag = true;
        uint8_t value;
        int i = 0;
        const unsigned char *key;

        cout << "Function: " << name << " found. "
             << "RTN: " << rtn_name << endl;

        // struct bignum_st {
        //     BN_ULONG *d;                /* Pointer to an array of 'BN_BITS2' bit
        //                                  * chunks. */
        //     int top;                    /* Index of last used d +1. */
        //     /* The next are internal book keeping for bn_expand. */
        //     int dmax;                   /* Size of the d array. */
        //     int neg;                    /* one if the number is negative */
        //     int flags;
        // };

        printf("priv_key:  sign: %d, size: %u p: %p\n", eckey->priv_key->neg, eckey->priv_key->dmax, eckey->priv_key->d);
        fprintf(fp, "Start; %p; %d; \n", eckey->priv_key->d, eckey->priv_key->dmax * 8 * 4);
        key = (const unsigned char *)eckey->priv_key->d;
        while (i < eckey->priv_key->dmax * 4)
        {
            PIN_SafeCopy(&value, key++, sizeof(uint8_t));
            fprintf(fp, "%x; ", value);
            printf("Key:  %d %x \n", i, value);
            ++i;
        }
        fprintf(fp, "\n");

        return;
    }
}

// We don't combine rtn_recv and rtn_send together because we need compilers try
// to make them incline
VOID static RTN_nonce_start(char *name, char *rtn_name, bignum_st *kinv)
{
    if (capture_flag && function_flag && !nonce_flag)
    {
        nonce_function_counter--;
        if (nonce_function_counter == 0)
        {
            nonce_flag = true;
            uint8_t value;
            int i = 0;
            const unsigned char *key;

            cout << "Function: " << name << " found. "
                 << "RTN: " << rtn_name << endl;

            // struct bignum_st {
            //     BN_ULONG *d;                /* Pointer to an array of 'BN_BITS2' bit
            //                                  * chunks. */
            //     int top;                    /* Index of last used d +1. */
            //     /* The next are internal book keeping for bn_expand. */
            //     int dmax;                   /* Size of the d array. */
            //     int neg;                    /* one if the number is negative */
            //     int flags;
            // };

            printf("kinv:  sign: %d, size: %u p: %p\n", kinv->neg, kinv->dmax, kinv->d);
            fprintf(fp, "Start; %p; %d; \n", kinv->d, kinv->dmax * 8 * 4);
            key = (const unsigned char *)kinv->d;
            while (i < kinv->dmax * 4)
            {
                PIN_SafeCopy(&value, key++, sizeof(uint8_t));
                fprintf(fp, "%x; ", value);
                printf("Key:  %d %x \n", i, value);
                ++i;
            }
            fprintf(fp, "\n");

            return;
        }
    }
}

VOID static RTN_secret_end(char *name)
{
    if (capture_flag)
    {
        cout << "END "
             << "function name: " << name << endl;
        //start_ins = false;
    }
}

VOID static RTN_nonce_end(char *name)
{
    if (capture_flag && function_flag && nonce_flag && (nonce_function_counter == 0))
    {
        cout << "END "
             << "function name: " << name << endl;
        function_flag = false;
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
    if (capture_flag)
    // return;

    {
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
}

//Read or write memory
void static getctxRead(ADDRINT addr, CONTEXT *fromctx, ADDRINT memaddr)
{
    //Only collect traces and a recv function
    if (capture_flag)
    // return;
    {
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

    if (rtn_name.find("@plt") != std::string::npos)
    {
        RTN_Close(rtn);
        return;
    }

    if (rtn_name.find(caller_function_name) != std::string::npos)
    {
        RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)RTN_prep_start,
                       IARG_ADDRINT, caller_function_name.c_str(),
                       IARG_ADDRINT, IMG_Name(IMG_FindByAddress(rtn_start)).c_str(),
                       IARG_END);

        RTN_InsertCall(rtn, IPOINT_AFTER, (AFUNPTR)RTN_prep_end,
                       IARG_PTR, RTN_Name(rtn).c_str(), IARG_END);

        RTN_Close(rtn);
        return;
    }

    if (nonce_function_name == rtn_name)
    {
        RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)RTN_nonce_start,
                       IARG_ADDRINT, nonce_function_name.c_str(),
                       IARG_ADDRINT, IMG_Name(IMG_FindByAddress(rtn_start)).c_str(),
                       IARG_FUNCARG_ENTRYPOINT_VALUE, 2,
                       IARG_END);

        RTN_InsertCall(rtn, IPOINT_AFTER, (AFUNPTR)RTN_nonce_end,
                       IARG_PTR, RTN_Name(rtn).c_str(), IARG_END);
        RTN_Close(rtn);
        return;
    }

    if (rtn_name.find(secret_function_name) != std::string::npos)
    {
        RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)RTN_secret_start,
                       IARG_ADDRINT, secret_function_name.c_str(),
                       IARG_ADDRINT, IMG_Name(IMG_FindByAddress(rtn_start)).c_str(),
                       IARG_FUNCARG_ENTRYPOINT_VALUE, 2,
                       IARG_END);

        RTN_InsertCall(rtn, IPOINT_AFTER, (AFUNPTR)RTN_secret_end,
                       IARG_PTR, RTN_Name(rtn).c_str(), IARG_END);
        RTN_Close(rtn);
        return;
    }

    RTN_Close(rtn);
    return;
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
    secret_function_name = "ECDSA_do_sign";
    caller_function_name = "ecdsa_do_sign";
    nonce_function_name = "BN_mod_mul";
    nonce_function_counter = 2;
    // function_name = argv[argc - 1];d

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
