cc -o Assembly Assembler/Assembler.c
./Assembly
cc -o Process Processor/RiscVProcessor.c
./Process 
rm Assembly
rm Process
