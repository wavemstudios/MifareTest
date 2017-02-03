/*
 * main.c
 *
 *  Created on: 18 Jan 2017
 *      Author: steve
 */

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/stat.h>

#include <feig/fetpf.h>
#include <feig/feclr.h>

#include "macros.h"
#include "ledBuzzerController.h"
#include "apduListener.h"

static int fd_feclr;

static int closed_loop_surrender(struct fetpf *client, uint32_t proceed)
{

	if (socketListen()){
		int RestVal = socketReadBase();
	}

	printf("Surrender Closed Loop (proceed: %i)!\n", proceed);
	return FETPF_RC_OK;
}

static int closed_loop_card_found(struct fetpf *client, uint64_t card,
			uint64_t tech, const union tech_data *tech_data,
		const struct timeval *first_response, uint32_t *proceed)
{
	*proceed = FETPF_PROCEED_RESTART;

	if (card == FETPF_CARD_ISO14443A) {
		printf("ISO14443-A\n");
		int RestVal = socketReadMifare(fd_feclr, tech_data);

		 printf("Forcing new card read3...\n");

	} else if (card == FETPF_CARD_DESFIRE_NATIVE) {
		printf("DESFire Native\n");
		int RestVal = socketReadDesfire(fd_feclr, tech_data);
/*
		int rc = 0;
		uint8_t tx_frame[] = { 0x5A, 0x00, 0x00, 0x00 };
		uint8_t rx_frame[256];
		uint8_t rx_last_bits = 0;
		size_t rx_frame_size = sizeof(rx_frame);
		uint64_t status;

		printf("DESFire Native\n");

		// Select DESFire App0
		rc = feclr_transceive(fd_feclr, 0, tx_frame, sizeof(tx_frame),
				  0, rx_frame, sizeof(rx_frame), &rx_frame_size,
						     &rx_last_bits, 0, &status);
		if (rc < 0) {
			printf("Transceive rc: 0x%08x: %m\n", rc);
			*proceed = FETPF_PROCEED_RESTART;
		} else if (status != FECLR_STS_OK) {
			printf("Transceive status: 0x%08llX\n", status);
			*proceed = FETPF_PROCEED_RESTART;
		} else if (rx_frame_size >= 1) {
			printf("DESFire Status: 0x%02x\n",
						     rx_frame[rx_frame_size-1]);
		}
	*/
	} else if (card == FETPF_CARD_PAYMENT_CARD) {
		printf("EMV Payment Card\n");
	}

	return 0;
}

int main(void)
{
	const struct fetpf_process_card_callbacks cbs = {
		.surrender = closed_loop_surrender,
		.card_found = closed_loop_card_found
	};
	uint64_t card_types = FETPF_CARD_ISO14443A | FETPF_CARD_DESFIRE_NATIVE | FETPF_CARD_PAYMENT_CARD;
	struct fetpf *client = NULL;
	int rc = 0;

	/* Initialize buzzer interface */
	rc = initialise_buzzer();
	if (rc < 0) {
		printf("buzzer_init failed with error: \"%s\"\n", strerror(rc));
	}

	/* Initialize led interface */
	rc = initialise_leds();
	if (rc < 0) {
		printf("led_init failed with error: \"%s\"\n", strerror(rc));
	}

	startup_visualization();

	socketInitialise();

	fd_feclr = open("/dev/feclr0", O_RDWR);
	if (fd_feclr < 0) {
		fprintf(stderr, "feclr open failed: %m\n");
		return EXIT_FAILURE;
	}

	client = fetpf_new(FEPKCS11_APP0_TOKEN_SLOT_ID);
	if (!client) {
		fprintf(stderr, "%s: fetpf_new failed!\n", __func__);
		goto done;
	}

	rc = fetpf_register_card_processor(client, card_types);
	if (rc) {
		fprintf(stderr, "%s: fetpf_register_card_processor failed "
						    "(rc %d)!\n", __func__, rc);
		goto done;
	}

	rc = fetpf_process_card(client, &cbs);
	if (rc) {
		fprintf(stderr, "%s: fetpf_process_card failed (rc %d)!\n",
								  __func__, rc);
		goto done;
	}

done:
	if (client)
		fetpf_free(client);
	close(fd_feclr);

	return rc ? EXIT_FAILURE : EXIT_SUCCESS;
}
