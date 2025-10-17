# Interrupts and Exception Handling
In this section, we will discuss more about Exception Handling and Interrupts. There will be faults and errors in any system but an efficient one will handle it without stopping its fundamental task. **Exceptions** are crucial part of building firmware. According to ARM, an exception is any event that disrupts the normal flow of program execution. This would require the processor to suspend the current task to execute an interrupt/exception handler. This will jump to the instructions that will handle what happens. Afterwards, it will return to the normal program execution. An example of this can be undefined commands in the instructions or an error detected during an operation.  An **interrupt** is a type of *external* exception that occur when there are independent events. For example, whenever you plug in a device, this sends an **IRQ (Interrupt Request)** to the processor asking for attention.

## Exception Levels
To help understand what these are, we can use an analogy. In a company, we have different roles like the HR manager, the CEO, the workers, etc. We can think of **Exception Levels** as the floors of the building and its roles. The higher you go, the more permissions you get. *EL3* (Exception Level 3) has the highest security clearance and it controls the entire building. *EL2* can be the management floor where it can oversee the employees. *EL1* can be where the employees work and *EL0* would be the public access area. For more technical functions of the Exception Level, *EL0* are mostly for **user applications**, *EL1* are mostly for **Operatering Systems** & **Kernel Modules**, *EL2* are for hypervisors or virtualization of EL1 and EL0, and *EL3* are for secure monitors.

In the Raspberry Pi 4B, the default exception level would be *EL3* when it boots in. There's a default Raspberry Pi stub that brings the processor down from EL3 to EL2. However, we do not need EL2 to execute our task so we need to drop the exception level down to *EL1*. To do that we need to add a bit of code to the boot sequence code. In each level, they have their own stack pointer, saved program status, and exception link registers. The stack pointer is the location of the top of the stack memory in RAM. For example, `SP_EL0` is pointing the top of the stack in EL0 but `SP_EL1` points the a different part of the stack for Exception Level 1 execution. The saved program status is used to save the location of the instruction when an exception is taken to a certain level.

## Interrupts
Interrupts are essential for tasks that need immediate attention from the CPU such as hardware interaction, I/O operations, hardware timers, etc. The CPU executes your program instructions endlessly so when something happens how will it handle it? The interrupts will need to jump to some handler to take care of the event. The CPU will need to know where this program location is using an **interrupt vector table**. The table is an array of addresses which is indexed by the IRQ vector number. 

### GIC-400
The design for the Raspberry Pi routes the interrupt signal to the **GIC-400** (Generic Interrupt Controller) and the Legacy Interrupt Controllers. The GIC-400 is the primary interrupt controller and can be disabled by the device tree in the `config.txt` file using `enable_gic=0`. From the BCM2711 Peripheral Sheet, we can see that the output of the Legacy Interrupt Controller goes through the **GIC-400** controller to route to the appropriate ARM core. This primary contain many features such as prioritization, distributor, and a CPU interface for each core in the SoC.

![GIC-400 Memory Map](assets/gic_mem_map.png)

#### Distributor
The distributor receives interrupts and pushes them through the highest priority pending interrupt to every CPU interface. You can configure it to control the CPU interface from which it routes the **SPIs**(Shared Peripheral Interrupts) and **PPIs**(Private Peripheral Interrupts). It can enable or disable interrupts, set priorities, and router interrupts to specific CPU cores. Each interrupt needs to be enabled and will need a priority value and a target. The targets would be the CPU interfaces and in a multi-core environment we need to assign a processor for it to execute. However, we have it only set to core 0 since we are working in a single-core environment for now.

#### CPU Interface
Each interface signals interrupts to the processor and receives acknowledge and *EOI (End of Interrupt)* access from that processor. The interface only signals the interrupts if it has sufficient priority. Prioritization is determined by the CPU interface's configuration and the active interrupts' priority. Each core has its own CPU interface and is responsible for interrupt completion signaling. 

![GIC-400 Partitioning Table](assets/gic_func_block.png)

### Timer Interrupts
To implement the System Timer, we first consult the BCM2711 ARM Peripheral Manual that gives us info on the registers. We can see that there's **four 32-bit timer channels** & **one 64-bit free running counter**. Each channel has a compare register to compare the 32 least significant bits of the free runing counter. When the compare value and the coutner value match, the timer will signal the interrupt controller to read the compare register and add the appropriate offset for the next timer tick.

![System Timer Overview and Registers](assets/sys_timer_info.png)

