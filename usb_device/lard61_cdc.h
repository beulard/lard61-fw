/*
** file: lard61_cdc.h
** author: beulard (Matthias Dubouchet)
** creation date: 10/07/2024
**
** Provides the l61_printf service, which allows the lard61 to communicate
** with its host via the CDC USB interface.
*/

#ifndef _LARD61_CDC_H
#define _LARD61_CDC_H

// Max length + 1 of a string formatted by lard61_printf
#define LARD61_PRINTF_BUFFER_SIZE 256
// Max char count + 1 in the command buffer
#define LARD61_COMMAND_BUFFER_SIZE 32

//-----------------------------------------------------------------------------
// Public API
//-----------------------------------------------------------------------------

// Setup for the lard61 mini shell
void l61_cdc_setup();

// Formatted print via the lard61 CDC USB interface
void l61_printf(const char* fmt, ...);

#endif /* _LARD61_CDC_H */