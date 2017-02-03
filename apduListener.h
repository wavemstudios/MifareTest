/*
 * apduListener.h
 *
 *  Created on: 10 Oct 2016
 *      Author: steve
 */

#ifndef APDULISTENER_H_
#define APDULISTENER_H_

int socketInitialise();
int socketReadBase();
int socketReadDesfire(int fd, const union tech_data *tech_data);
int socketReadMifare(int fd, const union tech_data *tech_data);
int socketWrite();

#endif /* APDULISTENER_H_ */
