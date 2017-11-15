/**
 * @file     debug_lib.c
 * @version  V0.1
 * @date     Oct 2014
 * @author   M. Horauer
 *
 * @brief   This library retargets the printf() function to
 *          generate output via the SWD DEBUG interface.
 *
 */

#include <debug_lib.h>

/******************************************************************** globals */
volatile int32_t ITM_RxBuffer = ITM_RXBUFFER_EMPTY;


FILE __stdout;
FILE __stdin;


/*!
 *  @brief Initialize the SWD/TPI/ITM debug modules to output debug messages
 *         via the Debug interface. This will enable the outputz of printf()
 *         and fputc() like messages.
 *         \par
 *         In order to enable the output add the following command at the GDB
 *         prompt (not needed when using the GNU ARM Plugin from within Eclipse)
 *
 *         (gdb) monitor SWO EnableTarget 16000000 0 1 0
 *
 *         Open up a separate terminal and connect to the TERMINAL OUTPUT
 *         CHANNEL of the Segger JLink.
 *
 *         $ telnet localhost 2333
 *
 *  @param  -
 *  @return -
 *  @note Checkout:
 *        (1) CMSIS Core Documentation found along the CMSIS sources:
 *            CMSIS-Core > Reference > Debug Access
 *        (2) ARM Cortex-M4 Processor Technical Reference Manual
 *        (3) ARMv7-M Architecture Reference Manual
 */
void initRetargetSwo (void)
{
	// P2.1: After a system reset, this pin selects HWO0 (DB.TDO/DB.TRACESWO)
	// PORT2->HWSEL ... clear bits [3:2]
	PORT2->HWSEL &= ~PORT2_HWSEL_HW1_Msk;
	// PORT2->HWSEL ... set bit [2] -> HWO0
	PORT2->HWSEL |= 0x01UL << PORT2_HWSEL_HW1_Pos;

	// Enable trace systems
	// Source: ARMv7-M Architecture Reference Manual
	// DEMCR = Debug Exception and Monitor Control Register
	//         TRCENA ... global enable for DWT and ITM blocks
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	// SWO baud rate prescaler value
	// ACPR = Asynchronous Clock Prescaler Register
	//        fSWO = fCPU / (SWOSCALAR + 1)
	//        fSWO = 1MHz (maximum speed JLINK Lite CortexM)
	//        fCPU = 120MHz
	TPI->ACPR = 119UL;
	// Asynchronous SWO, using NRZ encoding
	// Source: ARMv7-M Architecture Reference Manual
	// SPPR = Selected Pin Control Register
	//        0x0 ... Synchronous Trace Port Mode
	//        0x1 ... Asynchronous Serial Wire Output (Manchester)
	//        0x2 ... Asynchronous Serial Wire Output (NRZ)
	TPI->SPPR = 0x2UL;
	// Source: ARM Cortex-M4 Processor Technical Reference Manual
	// Source: infineonforums -> 358-ITM-printf-style-debugging-not-working
	// FFCR = Formatter and Flash Control Register
	//        0x0 ... continuous formatting disabled
	//        0x1 ... continuous formatting enabled
	TPI->FFCR &= ~TPI_FFCR_EnFCont_Msk;

	// Unlock ITM
	// Source: infineonforums -> 358-ITM-printf-style-debugging-not-working
	ITM->LAR = 0xC5ACCE55;
	// Source: ARMv7-M Architecture Reference Manual, Sec. C1-7
	// TCR = Trace Control Register
	//       ITMENA ... enable ITM
	ITM->TCR |= ITM_TCR_ITMENA_Msk;
	// Source: ARMv7-M Architecture Reference Manual, Sec. C1-7
	// TER = Trace Enable Register
	//       STIMENA ... Stimulus Port #N is enabled when bit STIMENA[N]
	ITM->TER = 0xFFFFFFFF;
}

/*!
 * @brief Retarget the printf() function to output via the ITM Debug module.
 *        This function relies on ITM_SendChar() defined in core_cm4.h of the
 *        CMSIS library.
 */
int _write (int file, char *buf, int nbytes)
{
	int i;

	for (i = 0; i < nbytes; i++) {
		ITM_SendChar ( (*buf++));
	}
	return nbytes;
}

/*!
 * @brief Output a single charcter via the ITM Debug module.
 */
int fputc (int ch, FILE *f)
{
	return (ITM_SendChar (ch));
}

/*!
 * @brief Read a single character via the ITM module.
 *
 * @note  Doesn't seem to work on the XMC board so far!
 */
int fgetc (FILE *f)
{
	while (ITM_CheckChar() != 1);
	return (ITM_ReceiveChar());
}


/*!
 * @brief Debug Error output via the ITM module.
 *
 * @note  Not implemented so far ...
 */
#if DEBUG
int ferror (FILE *f)
{
	/* Your implementation of ferror */
	return EOF;
}
#endif

/** EOF */
