# X-Makina-16bit-ISA-Emulator
 
XMX is an extended version of the original X-Makina 16-bit Instruction Set Architecture. The XMX architecture doubles the number of registers from eight to sixteen and increasing the number of instructions.

The original 16-bit Instruction Set Architecture (ISA) with 64-KiB of random-access memory remains unchanged. Instructions and data are fetched from, and data is written to, memory by the central processing unit (CPU) over the 16-bit system bus. XMX can communicate with the outside world using any of its eight devices. Device status changes can be detected by the CPU polling or the device signaling the Interrupt Controller to cause a CPU interrupt.
The basic architecture of XMX is shown in Figure 1.

![image](https://user-images.githubusercontent.com/61470710/210111175-7ab75540-77a4-45e0-a580-90aa724de6f1.png)

Figure 1: The XMX architecture
XMX is load-store machine that supports the following features:

• A total of 32 instructions supporting memory access, initialization, arithmetic, logic, and transfer of control. Each instruction is 16-bits wide. Many instructions can operate on a byte, a word, or both. Most of the mnemonics for the arithmetic and logic instructions are based on those found in a variety of machines, while register initialization, memory access instruction mnemonics, and branching are like ones found in the ARM Cortex.

• A total of sixteen 16-bit registers, eight data registers and eight addressing registers.

• Four addressing registers (A4, A5, A6, and A7) are reserved for the base pointer (BP), link register (LR), stack pointer (SP), and program counter (PC).

• Some instructions allow the programmer to use one of eight pre-defined constants in place of a source register. This can reduce the number of registers used in an arithmetic operation. A similar approach is used by the TI MSP-430.

• An additional 18 instructions that can be emulated by using existing instructions, allowing operations such as subroutine call and return, interrupt return, and stack push and pull. Many of these instructions are based on the TI MSP-430’s emulated instructions.

• Instructions are classified into one of: arithmetic and logical, data manipulation, (consisting of an opcode and one or two operands), transfer of control (a signed PC-relative value), memory access (loading and storing data from and to memory from a register), and special purpose. Each instruction is associated with one or more of XMX’s five addressing modes: register, direct, indexed (pre- and post- auto-increment and auto-decrement), relative, and immediate.

• Eight-levels of priority, up to eight devices for input or output, and 16 interrupt vectors for exceptions (i.e., device interrupts, faults, and traps).
• XMX has a RISC architecture and can be implemented as either a traditional von Neumann or pipelined architecture.
