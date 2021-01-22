//This is a Pin tool that will split the execution trace into per-message traces by monitoring the program's
//use of netwokring functions that read or write from sockets.
//Output:
//1. Taintdata execution traces
//2. Function Each function in the memory address

#include <iostream>
#include <string>
#include <list>
#include <map>
#include <fstream>
#include "pin.H"

using std;
using std::ofstream;


#if defined(TARGET_MAC)
#define MALLOC "_malloc"
#define CALLOC "_calloc"
#define FREE "_free"
#else
#define MALLOC "malloc"
#define CALLOC "__libc_calloc"
#define FREE "__cfree"
#define MEMCPY "memcpy"
#define MEMPCPY "mempcpy"
#define MEMSET "__memset_sse2_rep"
#endif

bool start_ins = false;
bool first_time = true;

std::map<ADDRINT, string> opcmap;
std::string function_name;
FILE *fp;

ofstream RTN_FP;

enum class Libc{
    nonelibc,
    malloc,
    calloc,
    free,
    memcpy,
    mempcpy,
    memset
};

Libc CurrentFunction = Libc::nonelibc;


// We don't combine rtn_recv and rtn_send together becasue we need compilers try to make them incline
VOID static RTN_start(char *name, char *rtn_name, uint8_t *pass, uint8_t bufsize) {
    if (start_ins == false && first_time) {
        cout << "Function: " << name << " found. " << "RTN: " << rtn_name << endl;
        start_ins = true;
        first_time = false;
        fprintf(fp, "Start; %p; %d; \n", pass, bufsize);
        uint8_t value = 0;
        int i = 0;
        while( i < bufsize )
        {
            PIN_SafeCopy(&value, pass++, sizeof(uint8_t));
            fprintf(fp, "%x; ", value);
            printf("Key:  %d %x \n", i, value);
            ++i;
        }
        fprintf(fp, "\n");
        return;
    }
}

VOID static RTN_end(char *name) {
    if (start_ins) {
        cout << "END " << "function name: " << name << endl;
    }
}

INT32 Usage() {
    std::cerr << "command line error\n";
    std::cout << "pin -t MyPinTool.so -f function_name -- target" << std::endl;
    return -1;
}

VOID MemsetBefore(char *name, ADDRINT* destination, ADDRINT value, ADDRINT num)
{
    if(first_time || (CurrentFunction != Libc::nonelibc)){
        return;
    }
    fprintf(fp, "libc_s; %s; %p; %x; %x\n", name, destination, value, num);
    start_ins = false;
    CurrentFunction = Libc::memset;
}

VOID MemsetAfter(char *name, ADDRINT ret)
{
    if(first_time || (CurrentFunction != Libc::memset)){
        return;
    }
    fprintf(fp, "libc_e; %s; %x\n", name, ret);
    start_ins = true;
    CurrentFunction = Libc::nonelibc;
}

VOID CallocBefore(CHAR *name, ADDRINT num, ADDRINT size)
{
    if(first_time || (CurrentFunction != Libc::nonelibc)){
        return;
    }
    fprintf(fp, "libc_s; %s; %x; %x\n", name, num, size);
    start_ins = false;
    CurrentFunction = Libc::calloc;
}

VOID CallocAfter(CHAR *name, ADDRINT ret)
{
    if(first_time || (CurrentFunction != Libc::calloc)){
        return;
    }
    fprintf(fp, "libc_e; %s; %x\n", name, ret);
    start_ins = true;
    CurrentFunction = Libc::nonelibc;
}

VOID FreeBefore(char *name, ADDRINT* ptr)
{
    if(first_time || (CurrentFunction != Libc::nonelibc)){
        return;
    }
    fprintf(fp, "libc_s; %s; %p\n", name, ptr);
    start_ins = false;
    CurrentFunction = Libc::free;
}

VOID FreeAfter(char *name)
{
    if(first_time || (CurrentFunction != Libc::free)){
        return;
    }
    fprintf(fp, "libc_e; %s\n", name);
    start_ins = true;
    CurrentFunction = Libc::nonelibc;
}

VOID MemcpyBefore(char *name, ADDRINT* destination, ADDRINT* source, ADDRINT size )
{
    if(first_time || (CurrentFunction != Libc::nonelibc)){
        return;
    }
    fprintf(fp, "libc_s; %s; %p; %p; %x\n", name, destination, source, size);
    start_ins = false;
    CurrentFunction = Libc::memcpy;
}

VOID MemcpyAfter(char *name, ADDRINT* destination)
{
    if(first_time || (CurrentFunction != Libc::memcpy)){
        return;
    }
    fprintf(fp, "libc_e; %s; %p\n", name, destination);
    start_ins = true;
    CurrentFunction = Libc::nonelibc;
}

VOID MempcpyBefore(char *name, ADDRINT* destination, ADDRINT* source, ADDRINT size )
{
    if(first_time || (CurrentFunction != Libc::nonelibc)){
        return;
    }
    fprintf(fp, "libc_s; %s; %p; %p; %x\n", name, destination, source, size);
    start_ins = false;
    CurrentFunction = Libc::mempcpy;
}

VOID MempcpyAfter(char *name, ADDRINT* destination)
{
    if(first_time || (CurrentFunction != Libc::mempcpy)){
        return;
    }
    fprintf(fp, "libc_e; %p\n", destination);
    start_ins = true;
    CurrentFunction = Libc::nonelibc;
}

//Only read or write register
void static getctx(ADDRINT addr, CONTEXT *fromctx, ADDRINT memaddr) {
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
void static getctxRead(ADDRINT addr, CONTEXT *fromctx, ADDRINT memaddr) {
    //Only collect traces and a recv function
    if (start_ins == FALSE)
        return;
    uint32_t value = 0;
    const VOID *src = (const VOID *) (memaddr - memaddr % 4);
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

VOID static printFunctionName(char *rtn_name, char* module_name, ADDRINT startaddr, USIZE rtn_size)
{
    RTN_FP << std::hex << startaddr << "; "
           << module_name << "; "
           << rtn_name << "; "
           << std::dec << rtn_size << "\n";

    if(start_ins == FALSE)
    {
        return;
    }
    
    /*
    if(CurrentFunction == Libc::nonelibc)
    {
        fprintf(fp, "start: %s nonelibc\n", rtn_name);
    }

    if(CurrentFunction == Libc::calloc)
    {
        fprintf(fp, "start: %s calloc\n", rtn_name);
    }

    if(CurrentFunction == Libc::malloc)
    {
        fprintf(fp, "start: %s malloc\n", rtn_name);
    }

    if(CurrentFunction == Libc::free)
    {
        fprintf(fp, "start: %s free\n", rtn_name);
    }

    if(CurrentFunction == Libc::memcpy)
    {
        fprintf(fp, "start: %s memcpy\n", rtn_name);
    }
    
    if(CurrentFunction == Libc::mempcpy)
    {
        fprintf(fp, "start: %s mempcpy\n", rtn_name);
    }

    if(CurrentFunction == Libc::memset)
    {
        fprintf(fp, "start: %s memset\n", rtn_name);
    }
    */
    
}

VOID static endFunctionName(char *rtn_name)
{
    if(start_ins == FALSE)
    {
        return;
    }
    /*

    if(CurrentFunction == Libc::nonelibc)
    {
        fprintf(fp, "end: %s nonelibc\n", rtn_name);
    }

    if(CurrentFunction == Libc::calloc)
    {
        fprintf(fp, "end: %s calloc\n", rtn_name);
    }

    if(CurrentFunction == Libc::malloc)
    {
        fprintf(fp, "end: %s malloc\n", rtn_name);
    }

    if(CurrentFunction == Libc::free)
    {
        fprintf(fp, "end: %s free\n", rtn_name);
    }

    if(CurrentFunction == Libc::memcpy)
    {
        fprintf(fp, "end: %s memcpy\n", rtn_name);
    }
    
    if(CurrentFunction == Libc::mempcpy)
    {
        fprintf(fp, "end: %s mempcpy\n", rtn_name);
    }

    if(CurrentFunction == Libc::memset)
    {
        fprintf(fp, "end: %s memset\n", rtn_name);
    }
    */
}


VOID Instruction(INS ins, VOID *v) {
    ADDRINT addr = INS_Address(ins);
    if (opcmap.find(addr) == opcmap.end()) {
        opcmap.insert(std::pair<ADDRINT, string>(addr, INS_Disassemble(ins)));
    }

    if (INS_IsMemoryRead(ins)) {
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR) getctxRead, IARG_INST_PTR, IARG_CONST_CONTEXT, IARG_MEMORYREAD_EA,
                       IARG_END);
    } else if (INS_IsMemoryWrite(ins)) {
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR) getctxRead, IARG_INST_PTR, IARG_CONST_CONTEXT, IARG_MEMORYWRITE_EA,
                       IARG_END);
    } else {
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR) getctx, IARG_INST_PTR, IARG_CONST_CONTEXT, IARG_ADDRINT, 0,
                       IARG_END);
    }
}

VOID Routine(RTN rtn, VOID *v) {
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

    RTN_InsertCall(rtn, IPOINT_AFTER, (AFUNPTR)endFunctionName,
                   IARG_ADDRINT, RTN_Name(rtn).c_str(),
                   IARG_END);

    if (rtn_name.find(function_name) == std::string::npos) {
        RTN_Close(rtn);
        return;
    }
    if (rtn_name.find("@plt") != std::string::npos) {
        RTN_Close(rtn);
        return;
    }

    RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR) RTN_start,
                   IARG_ADDRINT, function_name.c_str(),
                   IARG_ADDRINT, IMG_Name(IMG_FindByAddress(rtn_start)).c_str(),
                   IARG_FUNCARG_ENTRYPOINT_VALUE, 5,
                   IARG_FUNCARG_ENTRYPOINT_VALUE, 6,
                   IARG_END);

    RTN_InsertCall(rtn, IPOINT_AFTER, (AFUNPTR) RTN_end,
                   IARG_PTR, RTN_Name(rtn).c_str(), IARG_END);


    RTN_Close(rtn);

}

VOID Image(IMG img, VOID *v){
    // Find the Calloc() Function
    
    RTN callocRtn = RTN_FindByName(img, CALLOC);
    
    if (RTN_Valid(callocRtn))
    {
        RTN_Open(callocRtn);

        // Instrument calloc() to print the input argument value and the return value.
        RTN_InsertCall(callocRtn, IPOINT_BEFORE, (AFUNPTR)CallocBefore,
                       IARG_ADDRINT, RTN_Name(callocRtn).c_str(),
                       IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
                       IARG_FUNCARG_ENTRYPOINT_VALUE, 1,
                       IARG_END);
        RTN_InsertCall(callocRtn, IPOINT_AFTER, (AFUNPTR)CallocAfter,
                       IARG_ADDRINT, RTN_Name(callocRtn).c_str(),
                       IARG_FUNCRET_EXITPOINT_VALUE, IARG_END);

        RTN_Close(callocRtn);
    }
    

    // Find the mempcpy() function.
    RTN mempcpyRtn = RTN_FindByName(img, MEMPCPY);
    if (RTN_Valid(mempcpyRtn))
    {
        RTN_Open(mempcpyRtn);

        // Instrument mempcpy() to print the input argument value and the return value.
        RTN_InsertCall(mempcpyRtn, IPOINT_BEFORE, (AFUNPTR)MempcpyBefore,
                       IARG_ADDRINT, RTN_Name(mempcpyRtn).c_str(),
                       IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
                       IARG_FUNCARG_ENTRYPOINT_VALUE, 1,
                       IARG_FUNCARG_ENTRYPOINT_VALUE, 2,
                       IARG_END);
        RTN_InsertCall(mempcpyRtn, IPOINT_AFTER, (AFUNPTR)MempcpyAfter,
                       IARG_ADDRINT, RTN_Name(mempcpyRtn).c_str(),
                       IARG_FUNCRET_EXITPOINT_VALUE, IARG_END);

        RTN_Close(mempcpyRtn);
    }

    // Find the memcpy() function.
    RTN memcpyRtn = RTN_FindByName(img, MEMCPY);
    if (RTN_Valid(memcpyRtn))
    {
        RTN_Open(memcpyRtn);

        // Instrument mempcpy() to print the input argument value and the return value.
        RTN_InsertCall(memcpyRtn, IPOINT_BEFORE, (AFUNPTR)MemcpyBefore,
                       IARG_ADDRINT, RTN_Name(memcpyRtn).c_str(),
                       IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
                       IARG_FUNCARG_ENTRYPOINT_VALUE, 1,
                       IARG_FUNCARG_ENTRYPOINT_VALUE, 2,
                       IARG_END);
        RTN_InsertCall(memcpyRtn, IPOINT_AFTER, (AFUNPTR)MemcpyAfter,
                       IARG_ADDRINT, RTN_Name(memcpyRtn).c_str(),
                       IARG_FUNCRET_EXITPOINT_VALUE, IARG_END);

        RTN_Close(memcpyRtn);
    }

    // Find the memset() function.
    RTN memsetRtn = RTN_FindByName(img, MEMSET);
    if (RTN_Valid(memsetRtn))
    {
        RTN_Open(memsetRtn);
        // Instrument memset() to print the input argument value and the return value.
        RTN_InsertCall(memsetRtn, IPOINT_BEFORE, (AFUNPTR)MemsetBefore,
                       IARG_ADDRINT, RTN_Name(memsetRtn).c_str(),
                       IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
                       IARG_FUNCARG_ENTRYPOINT_VALUE, 1,
                       IARG_FUNCARG_ENTRYPOINT_VALUE, 2,
                       IARG_END);
        RTN_InsertCall(memsetRtn, IPOINT_AFTER, (AFUNPTR)MemsetAfter,
                       IARG_ADDRINT, RTN_Name(memsetRtn).c_str(),
                       IARG_FUNCRET_EXITPOINT_VALUE, IARG_END);

        RTN_Close(memsetRtn);
    }

}

static void Fini(INT32, void *v) {
    fclose(fp);
    RTN_FP.close();
}

KNOB<string> KnobFunctionName(KNOB_MODE_WRITEONCE, "pintool",
    "f", "main", "specify the name of function");

int main(int argc, char *argv[]) {
    // Initialize pin & symbol manager
    if (PIN_Init(argc, argv)) {
        return Usage();
    }
    function_name = KnobFunctionName.Value();

    fp = fopen("Inst_data.txt", "w");
    RTN_FP.open("Function.txt");
    PIN_InitSymbols();
    // Register Image to be called to instrument functions.
    RTN_AddInstrumentFunction(Routine, 0);
    INS_AddInstrumentFunction(Instruction, 0);
    IMG_AddInstrumentFunction(Image, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}
