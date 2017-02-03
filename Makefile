CC=arm-linux-gcc

all:executable

debug: CC += -g -DDEBUG
debug: executable

executable: main.c
	$(CC) main.c apduListener.c conversions.c ledBuzzerController.c -o mifaretest -lfeclr -lfepkcs11 -lcrypto -lfememcard -lfetpf -lleds -lbuzzer
	fesign --module opensc-pkcs11.so --pin 648219 --slotid 1 --keyid 00a0 --infile mifaretest
	
.PHONY: clean
clean:
	rm -f standby standby.backup
	
	
