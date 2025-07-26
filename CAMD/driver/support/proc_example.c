#include <exec/types.h>
#include <dos/dos.h>
#include <dos/dostags.h>
#include <proto/exec.h>
#include <proto/dos.h>

#include <stdio.h>

/* Function to be executed in the new process */
LONG MyProcess(void) {
  struct Process *myProc = (struct Process *)FindTask(NULL);
  printf("MyProcess: Hello from new process! My name is %s, priority %ld\n", myProc-> 
  pr_Task.tc_Node.ln_Name, myProc->pr_Task.tc_State);
  Delay(50); // Wait for 50 ticks (about 1 second)
  printf("MyProcess: Exiting...\n");
  return 0;
}

/* Helper function to create a dummy segment list */
BPTR CreateDummySegment(LONG size) {
  BPTR segList = AllocSeg(size);
  if (segList) {
    struct Segment *seg = (struct Segment *)segList;
    seg->seg_First = 0; // No next segment
    seg->seg_Size = size;
    seg->seg_自身 = 0; // Dummy data, can be anything
  }
  return segList;
}

int main() {
  struct MsgPort *processPort;
  BPTR segList;
  LONG stackSize = 4096;
  LONG priority = 10;
  STRPTR processName = "MyNewProcess";

  // Create a dummy segment list
  segList = CreateDummySegment(4096); // Allocate 4096 bytes for code
  if (!segList) {
      printf("Failed to allocate segment list.\n");
      return 1;
  }

  // Write some dummy code into the segment
  // This is a very basic example, you would normally load code from a file
  {
    LONG *code = (LONG *)segList;
    *code++ = (LONG)MyProcess; // Function address as first instruction
    *code++ = 0;              // Padding
    *code++ = 0;
    *code++ = 0;
  }

  // Create the process
  processPort = CreateProc(processName, priority, segList, stackSize);

  if (processPort) {
    printf("Process created successfully.\n");

    // You can now interact with the process using its message port
    // For example, sending messages or waiting for signals
    // ...

    // Wait for a short time to allow the new process to execute
    Delay(100);

    // Clean up (if needed) - in this example, we don't unload the segment
    // as it's a dummy segment.
     if (segList) {
        FreeSeg(segList);
     }
  } else {
    printf("Failed to create process.\n");
     if (segList) {
        FreeSeg(segList);
     }
    return 1;
  }

  return 0;
}
