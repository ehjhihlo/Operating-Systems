// addrspace.cc 
//	Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//
//	1. link with the -n -T 0 option 
//	2. run coff2noff to convert the object file to Nachos format
//		(Nachos object code format is essentially just a simpler
//		version of the UNIX executable object code format)
//	3. load the NOFF file into the Nachos file system
//		(if you haven't implemented the file system yet, you
//		don't need to do this last step)
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "main.h"
#include "addrspace.h"
#include "machine.h"
#include "noff.h"

bool AddrSpace::usedPhysicalPage[NumPhysPages] = {0};

//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the 
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void 
SwapHeader (NoffHeader *noffH)
{
    noffH->noffMagic = WordToHost(noffH->noffMagic);
    noffH->code.size = WordToHost(noffH->code.size);
    noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
    noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
    noffH->initData.size = WordToHost(noffH->initData.size);
    noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
    noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
    noffH->uninitData.size = WordToHost(noffH->uninitData.size);
    noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
    noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
// 	Create an address space to run a user program.
//	Set up the translation from program memory to physical 
//	memory.  For now, this is really simple (1:1), since we are
//	only uniprogramming, and we have a single unsegmented page table
//----------------------------------------------------------------------

AddrSpace::AddrSpace()
{
    //pageTable = new TranslationEntry[NumPhysPages];
    //for (unsigned int i = 0; i < NumPhysPages; i++) {
	//pageTable[i].virtualPage = i;	// for now, virt page # = phys page #
	//pageTable[i].physicalPage = i;
//	pageTable[i].physicalPage = 0;
	//pageTable[i].valid = TRUE;
//	pageTable[i].valid = FALSE;
	//pageTable[i].use = FALSE;
	//pageTable[i].dirty = FALSE;
	//pageTable[i].readOnly = FALSE;  
    //}
    
    // zero out the entire address space
//    bzero(kernel->machine->mainMemory, MemorySize);
    ID=(kernel->machine->Identity)++;
    kernel->machine->Identity=(kernel->machine->Identity)++;
}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
// 	Dealloate an address space.
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
   for(int i = 0; i < numPages; i++)
	AddrSpace::usedPhysicalPage[pageTable[i].physicalPage]=FALSE;
   delete pageTable;
}


//----------------------------------------------------------------------
// AddrSpace::Load
// 	Load a user program into memory from a file.
//
//	Assumes that the page table has been initialized, and that
//	the object code file is in NOFF format.
//
//	"fileName" is the file containing the object code to load into memory
//----------------------------------------------------------------------

bool 
AddrSpace::Load(char *fileName) 
{
    OpenFile *executable = kernel->fileSystem->Open(fileName);
    NoffHeader noffH;
    unsigned int size;

    if (executable == NULL) {
	cerr << "Unable to open file " << fileName << "\n";
	return FALSE;
    }
    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) && 
		(WordToHost(noffH.noffMagic) == NOFFMAGIC))
    	SwapHeader(&noffH);
    ASSERT(noffH.noffMagic == NOFFMAGIC);

// how big is address space?
    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size 
			+ UserStackSize;	// we need to increase the size
						// to leave room for the stack
    numPages = divRoundUp(size, PageSize);
//	cout << "number of pages of " << fileName<< " is "<<numPages<<endl;
		// check we're not trying
						// to run anything too big --
						// at least until we have
						// virtual memory

    pageTable = new TranslationEntry[numPages]; // create a pagetable 
    // for (unsigned int i = 0, j = 0; i < numPages; i++) { 
	// pageTable[i].virtualPage = i; // for now, virt page # = phys page #
	// while(j<NumPhysPages && AddrSpace::usedPhysicalPage[j]==true){
	//     j++;
	// } //usedPhysPage[j++] is True -> page been occupied -> see next page
	// AddrSpace::usedPhysicalPage[j]=TRUE; //turn it to occupy
	// pageTable[i].physicalPage = j;
	// // pageTable[i].physicalPage = i;
	// pageTable[i].valid = TRUE;
	// pageTable[i].use = FALSE;
	// pageTable[i].dirty = FALSE;
	// pageTable[i].readOnly = FALSE;
	// }

    // size = numPages * PageSize;

    // ASSERT(numPages <= NumPhysPages);
    // DEBUG(dbgAddr, "Initializing address space: " << numPages << ", " << size);

// then, copy in the code and data segments into memory
	if (noffH.code.size > 0) {
        //DEBUG(dbgAddr, "Initializing code segment.");
	//DEBUG(dbgAddr, noffH.code.virtualAddr << ", " << noffH.code.size);
        for(int j=0,i=0;i < numPages ;i++){
            j=0;
            while(kernel->machine->Occupied_frame[j] != FALSE && j < NumPhysPages) {
                j += 1;
            }
            //if memory is enough,just put data in without using virtual memory
            if(j<NumPhysPages){
                pageTable[i].physicalPage = j;      
                pageTable[i].use = FALSE;
                pageTable[i].dirty = FALSE;
                pageTable[i].ID =ID;
                pageTable[i].readOnly = FALSE;
                pageTable[i].valid = TRUE;          
                kernel->machine->Occupied_frame[j]=TRUE;
                kernel->machine->FrameName[j]=ID;
                kernel->machine->main_tab[j]=&pageTable[i];        
                pageTable[i].count_LRU++;
                executable->ReadAt(&(kernel->machine->mainMemory[j*PageSize]),PageSize, noffH.code.inFileAddr+(i*PageSize));
            }else{ 
                char *buffer;
                buffer = new char[PageSize];
                unsigned int tmp=0;
                while(kernel->machine->Occupied_virpage[tmp]!=FALSE){tmp++;}
                pageTable[i].virtualPage=tmp;
                pageTable[i].ID =ID;
                pageTable[i].valid = FALSE;
                pageTable[i].dirty = FALSE;
                pageTable[i].readOnly = FALSE;
                pageTable[i].use = FALSE;
                kernel->machine->Occupied_virpage[tmp]=true;
                executable->ReadAt(buffer,PageSize, noffH.code.inFileAddr+(i*PageSize));
                kernel->Swap_Area->WriteSector(tmp, buffer); //call virtual_disk write in virtual memory
            }
        }
    }
	if (noffH.initData.size > 0) {
        DEBUG(dbgAddr, "Initializing data segment.");
	DEBUG(dbgAddr, noffH.initData.virtualAddr << ", " << noffH.initData.size);
        // executable->ReadAt( &(kernel->machine->mainMemory[pageTable[noffH.code.virtualAddr/PageSize].physicalPage*PageSize+(noffH.code.virtualAddr%PageSize)]), noffH.code.size, noffH.code.inFileAddr);
        executable->ReadAt(&(kernel->machine->mainMemory[noffH.initData.virtualAddr]), noffH.initData.size, noffH.initData.inFileAddr);
    }

    delete executable;			// close file
    return TRUE;			// success
}

//----------------------------------------------------------------------
// AddrSpace::Execute
// 	Run a user program.  Load the executable into memory, then
//	(for now) use our own thread to run it.
//
//	"fileName" is the file containing the object code to load into memory
//----------------------------------------------------------------------

void 
AddrSpace::Execute(char *fileName) 
{
    Is_ptable_loaded=FALSE;
    if (!Load(fileName)) {
	cout << "inside !Load(FileName)" << endl;
	return;				// executable not found
    }

    //kernel->currentThread->space = this;
    this->InitRegisters();		// set the initial register values
    this->RestoreState();		// load page table register
    Is_ptable_loaded=TRUE;
    kernel->machine->Run();		// jump to the user progam

    ASSERTNOTREACHED();			// machine->Run never returns;
					// the address space exits
					// by doing the syscall "exit"
}


//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------

void
AddrSpace::InitRegisters()
{
    Machine *machine = kernel->machine;
    int i;

    for (i = 0; i < NumTotalRegs; i++)
	machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, 0);	

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister(NextPCReg, 4);

   // Set the stack register to the end of the address space, where we
   // allocated the stack; but subtract off a bit, to make sure we don't
   // accidentally reference off the end!
    machine->WriteRegister(StackReg, numPages * PageSize - 16);
    DEBUG(dbgAddr, "Initializing stack pointer: " << numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, don't need to save anything!
//----------------------------------------------------------------------

void AddrSpace::SaveState() 
{
    if(Is_ptable_loaded){
        pageTable=kernel->machine->pageTable;
        numPages=kernel->machine->pageTableSize;
    }
}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void AddrSpace::RestoreState() 
{
    kernel->machine->pageTable = pageTable;
    kernel->machine->pageTableSize = numPages;
}
