#define __private_include__
#include <flipper/gpio.h>
#include <flipper/uart0.h>
#include <flipper/modules.h>
#include <platforms/atsam4s16b.h>

/* The fmr_device object containing global state about this device. */
struct _lf_device self = {
	{
		"flipper",
		0xc713,
		LF_VERSION,
		(lf_device_32bit | lf_device_little_endian)
	},
	NULL,
	E_OK,
	false,
	NULL
};

void uart0_pull_wait(void *destination, lf_size_t length) {
	/* Set the transmission length and destination pointer. */
	UART0 -> UART_RCR = length;
	UART0 -> UART_RPR = (uintptr_t)(destination);
	/* Disable the PDC receive complete interrupt. */
	UART0 -> UART_IDR = UART_IDR_ENDRX;
	/* Enable the receiver. */
	UART0 -> UART_PTCR = UART_PTCR_RXTEN;
	/* Wait until the transfer has finished. */
	while (!(UART0 -> UART_SR & UART_SR_ENDRX));
	/* Clear the PDC RX interrupt flag. */
	UART0 -> UART_RCR = 1;
	/* Disable the PDC receiver. */
	UART0 -> UART_PTCR = UART_PTCR_RXTDIS;
	/* Enable the PDC receive complete interrupt. */
	UART0 -> UART_IER = UART_IER_ENDRX;
}

/* Helper functions to libflipper. */
void fmr_push(fmr_module module, fmr_function function, lf_size_t length) {
	void *swap = malloc(length);
	if (!swap) {
		error_raise(E_MALLOC, NULL);
		return;
	}
	/* Pull, not asynchronously. */
	uart0_pull_wait(swap, length);
	uint32_t types = fmr_type(lf_size_t) << 2 | fmr_type(void *);
	struct {
		void *source;
		lf_size_t length;
	} args = { swap, length };
	fmr_execute(module, function, 2, types, &args);
	free(swap);
}

void fmr_pull(fmr_module module, fmr_function function, lf_size_t length) {
	void *swap = malloc(length);
	if (!swap) {
		error_raise(E_MALLOC, NULL);
		return;
	}
	uint32_t types = fmr_type(lf_size_t) << 2 | fmr_type(void *);
	struct {
		void *source;
		lf_size_t length;
	} args = { swap, length };
	/* Call the function. */
	fmr_execute(module, function, 2, types, &args);
	uart0_push(swap, length);
	free(swap);
}

struct _fmr_packet packet;

void system_task(void) {
	/* ~ Configure the USART peripheral. ~ */
	usart_configure();
	/* ~ Configure the UART peripheral. */
	uart0_configure();
	/* ~ Configure the GPIO peripheral. */
	gpio_configure();

	/* ~ Configure the SPI peripheral. ~ */

	/* Enable the SPI clock. */
	PMC -> PMC_PCER0 = (1 << ID_SPI);
	/* Create a pinmask for the peripheral pins. */
	const unsigned int SPI_PIN_MASK = (PIO_PA14A_SPCK | PIO_PA13A_MOSI | PIO_PA12A_MISO | PIO_PA31A_NPCS1);
	/* Disable PIOA interrupts on the peripheral pins. */
	PIOA -> PIO_IDR = SPI_PIN_MASK;
	/* Disable the peripheral pins from use by the PIOA. */
	PIOA -> PIO_PDR = SPI_PIN_MASK;
	/* Hand control of the peripheral pins to peripheral A. */
	PIOA -> PIO_ABCDSR[0] &= ~SPI_PIN_MASK;
	PIOA -> PIO_ABCDSR[1] &= ~SPI_PIN_MASK;
	/* Reset the SPI. */
	SPI -> SPI_CR = SPI_CR_SWRST;
	/* Reset the SPI again. Eratta. */
	SPI -> SPI_CR = SPI_CR_SWRST;
	/* Enable the mode fault interrupt. */
	SPI -> SPI_IER = SPI_IER_MODF;
	/* Enter master mode, no mode fault detection, activate user SPI peripheral. */
	SPI -> SPI_MR = SPI_MR_MSTR | SPI_MR_PCS(1);
	/* Configure the user SPI peripheral. 8 bits per transfer. SPI mode 3. SCK = MCK / 8. */
	SPI -> SPI_CSR[1] = SPI_CSR_SCBR(8) | SPI_CSR_DLYBCT(1) | SPI_CSR_BITS_8_BIT | SPI_CSR_NCPHA | SPI_CSR_CPOL;
	/* Disable the PDC channels. */
	SPI -> SPI_PTCR = SPI_PTCR_TXTDIS | SPI_PTCR_RXTDIS;
	/* Clear the secondary PDC channel. */
	SPI -> SPI_TNCR = 0;
	SPI -> SPI_TNPR = (uintptr_t)(NULL);
	/* Enable the SPI interrupt. */
	NVIC_EnableIRQ(SPI_IRQn);
	/* Enable the SPI. */
	SPI -> SPI_CR = SPI_CR_SPIEN;

	/* Insantiate an outgoing PDC transfer. */
	const char hello[] = "Hello!";

	gpio_enable(PIO_PA0, 0);
	PIOA -> PIO_OWER = PIO_PA0;
	while (1) {
		PIOA -> PIO_ODSR ^= PIO_PA0;

		/* Disable the PDC transmitter. */
		SPI -> SPI_PTCR = SPI_PTCR_TXTDIS;
		/* Queue the PDC transfer. */
		SPI -> SPI_TCR = sizeof(hello);
		SPI -> SPI_TPR = (uintptr_t)(hello);
		/* Enable the PDC transmitter to start the transmission. */
		SPI -> SPI_PTCR = SPI_PTCR_TXTEN;
		/* Wait until the transfer has finished. */
		while (!(SPI -> SPI_SR & SPI_SR_ENDTX));

		for (int i = 0; i < 5000000; i ++);
	}

	/* Enable the PDC receive complete interrupt. */
	UART0 -> UART_IER = UART_IER_ENDRX;
	/* Pull an FMR packet asynchronously. */
	uart0_pull(&packet, sizeof(struct _fmr_packet));
}

void spi_isr(void) {
	/* Falls through if a mode fault has occured. This fires when the masters drive the slave out of sync. */
	if (SPI -> SPI_SR & SPI_SR_MODF) {
		/* Re-enable the SPI bus. */
		SPI -> SPI_CR = SPI_CR_SPIEN;
	}
}

void uart0_isr(void) {
	if (UART0 -> UART_SR & UART_SR_ENDRX) {
		/* Disable the PDC receiver. */
		UART0 -> UART_PTCR = UART_PTCR_RXTDIS;
		/* Clear the PDC RX interrupt flag. */
		UART0 -> UART_RCR = 1;
		/* Create a result. */
		struct _fmr_result result = { 0 };
		/* Process the packet. */
		fmr_perform(&packet, &result);
		/* Give the result back. */
		uart0_push(&result, sizeof(struct _fmr_result));
		/* Pull the next packet asynchronously. */
		uart0_pull(&packet, sizeof(struct _fmr_packet));
	}
}

void system_init(void) {

	uint32_t timeout;

	/* Disable the watchdog timer. */
	WDT -> WDT_MR = WDT_MR_WDDIS;

	/* Configure the EFC for 3 wait states. */
	EFC -> EEFC_FMR = EEFC_FMR_FWS(3);

	/* Configure the primary clock source. */
	if (!(PMC -> CKGR_MOR & CKGR_MOR_MOSCSEL)) {
		PMC -> CKGR_MOR = CKGR_MOR_KEY(0x37) | BOARD_OSCOUNT | CKGR_MOR_MOSCRCEN | CKGR_MOR_MOSCXTEN;
		for (timeout = 0; !(PMC -> PMC_SR & PMC_SR_MOSCXTS) && (timeout ++ < CLOCK_TIMEOUT););
	}

	/* Select external 20MHz oscillator. */
	PMC -> CKGR_MOR = CKGR_MOR_KEY(0x37) | BOARD_OSCOUNT | CKGR_MOR_MOSCRCEN | CKGR_MOR_MOSCXTEN | CKGR_MOR_MOSCSEL;
	for (timeout = 0; !(PMC -> PMC_SR & PMC_SR_MOSCSELS) && (timeout ++ < CLOCK_TIMEOUT););
	PMC -> PMC_MCKR = (PMC -> PMC_MCKR & ~(uint32_t)PMC_MCKR_CSS_Msk) | PMC_MCKR_CSS_MAIN_CLK;
	for (timeout = 0; !(PMC -> PMC_SR & PMC_SR_MCKRDY) && (timeout++ < CLOCK_TIMEOUT););

	/* Configure PLLB as the master clock PLL. */
	PMC -> CKGR_PLLBR = BOARD_PLLBR;
	for (timeout = 0; !(PMC -> PMC_SR & PMC_SR_LOCKB) && (timeout++ < CLOCK_TIMEOUT););

	/* Switch to the main clock. */
	PMC -> PMC_MCKR = (BOARD_MCKR & ~PMC_MCKR_CSS_Msk) | PMC_MCKR_CSS_MAIN_CLK;
	for (timeout = 0; !(PMC -> PMC_SR & PMC_SR_MCKRDY) && (timeout++ < CLOCK_TIMEOUT););
	PMC -> PMC_MCKR = BOARD_MCKR;
	for (timeout = 0; !(PMC -> PMC_SR & PMC_SR_MCKRDY) && (timeout++ < CLOCK_TIMEOUT););

	/* Allow the reset pin to reset the device. */
	RSTC -> RSTC_MR = RSTC_MR_KEY(0xA5) | RSTC_MR_URSTEN;
}

void system_deinit(void) {

}