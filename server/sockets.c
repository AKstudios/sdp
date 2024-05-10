/*
 * Copyright (C) 2007-2023 Argonne National Laboratory
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/*******************************************************************
 *
 * @author Jason D. Harper, Michael Dipperstein, Akram S. Ali
 * @@version 0.9
 * @contact jharper@anl.gov
 *
 ********************************************************************/
#include "sockets.h"
#include "sdpserver.h"
#include "evse_server.h"
#ifdef ABC170
	#include "ABC170CAN.h"
#endif
#ifdef UPER
	#include "UPER.h"
#endif
#ifdef MAXWELL
	#include "maxwell.h"
#endif
#include "main.h"
#include "configkeyfile.h"
#include "apptimer.h"
#include "J1772.h"
#include "server_setup.h"
#include "req_res.h"
#include "adc.h"
#include "display.h"
#include "mqttexample.h"
#include "mqttconfig.h"
#include "evse_stateMachine.h"
#include "nbclient.h"
#include "API.hpp"
#include "mqttclient.h"
#include "common.h"

uint8_t EVSESHUTDOWN = 0;
uint8_t MQTT_conn_attempts = 0;
uint8_t MQTT_pub_attempts = 0;
int8_t res;
// int a;
// t;sss
//==========================================================================================================
int PollServer(void)
{
	fd_set read_fd_set;
	int i, s;
	struct sockaddr_storage peer_addr;
	socklen_t peer_addr_len = sizeof(struct sockaddr_storage);
	uint16_t inStreamLength = 0;
	uint8_t inStream[MAX_STREAM_SIZE]; /* define MAX_STREAM_SIZE before */
	int16_t nread;
	struct signalfd_siginfo info;
	struct timeval timeout;

	timeout.tv_sec  = 0;
	timeout.tv_usec = 100; /*wait 100 us for input on sockets*/

	/* Block for timeout until input arrives on one or more active sockets.  */
	read_fd_set = active_fd_set;
	if (select (FD_SETSIZE, &read_fd_set, NULL, NULL, &timeout) < 0)
	{
		logtime();
		perror ("select");
		return -1;
	}

	/* Service all the sockets with input pending.  */
	for (i = 0; i < FD_SETSIZE; ++i)
	{
		if (FD_ISSET(i, &read_fd_set))
		{
			// printf("FD_ISSET: %d\n", i);

			/**************************************************************************************************/
			if (i == sdListen) /* Connection request on SECC server socket  */
			{
				// printf("sdListen here\n");
				SECCfd = accept(sdListen,
								(struct sockaddr *) &peer_addr, &peer_addr_len);


				if (SECCfd < 0)
				{
					logtime();
					perror ("accept");
					exit (EXIT_FAILURE);
				}
				else
					printf("Connected to SECC: %d\n", SECCfd);


				s = getnameinfo((struct sockaddr *) &peer_addr,
							peer_addr_len, host, sizeof(host),
							service, sizeof(service),NI_NUMERICHOST|NI_NUMERICSERV);
				if (s == 0)
				{
					logtime();
					printf("\nConnected to EVCC %s:%s\n\n", host, service);
					hlc_state = INITILIZATION_HandShake;
				}
				else
				{
					logtime();
					fprintf(stderr, "getnameinfo: %s\n", gai_strerror(s));
				}

				alarm(0); 	/*clear the alarm*/
				if (alarm(SECC_Sequence_Timeout) != 0) /*set SECC Sequence Timeout alarm to 60 secs per DIN70121*/
				{
					logtime();
					fprintf(stderr, "alarm was already set\n");
				}

				FD_SET (SECCfd, &active_fd_set);  /*SECCfd is socket # of connected client*/
			}
			/**************************************************************************************************/


			/**************************************************************************************************/
			else if (i == SECCfd) /*Data arriving on an already-connected SECC server socket */
			{
				// printf("SECCfd 2: %d\n", SECCfd);

				res = ReadEVCCReq(i,inStream,&inStreamLength);
				if (res == 0)
					{
					/*EVCC closed socket*/
						close (i);
						FD_CLR (i, &active_fd_set);
						printf("hlc_state = %d\n",hlc_state);
						if (!(hlc_state == SHUTDOWN_SessionPause) || (hlc_state == SHUTDOWN_SessionPause_Wait))
						{
							logtime();
							fprintf(stderr, "EVCC Closed TCP Connection, SECC Ending Session, SECC Closing Socket\n");
							printf("hlc_state? = %d\n",hlc_state);
							/*shutoff oscillator*/
							closetcp();
						}
						else {
							logtime();
							fprintf(stderr, "EVCC Closed TCP Connection, but will resume later.\n");
							closetcp();
						}

					}
				else if (res < 0)
				{
					/*error reading data from socket*/
					//do nothing
				}
				else
				{
					if (ParseEVCCReq(i,inStream,inStreamLength) < 0)
					{
						/*error message already printed to terminal*/
						/*shutoff oscillator*/
						/*must send ResponseCode "Failed" then close tcp*/
						/*GM DEBUG
							* Do not close tcp on EVSE failure*/
						#ifdef evseresponsefailure
						close (i);
						FD_CLR (i, &active_fd_set);
						closetcp();
						#endif
					}
				}
			}
			/**************************************************************************************************/


			/**************************************************************************************************/
			else if (i == sdSDP)  /*Data arriving on SDP server socket */
			{
				//printf("SDP HERE!\n");

				/*Parse SDP Request and Send SDP Response*/
				if (ReadSDPReq(i,inStream,&inStreamLength) < 0)
				{
					FD_CLR (i, &active_fd_set);
					logtime();
					fprintf(stderr, "Error Reading SDPReq\n");
				}
				else
				{
					if (ParseSDPReq(i,inStream,inStreamLength) < 0)
					{
						logtime();
						fprintf(stderr, "Error ParseSDPReq()\n");
					}

					alarm(0); 	/*clear the alarm*/
					if (alarm(SECC_Sequence_Timeout) != 0) /*set alarm*/
					{  /*set SECC Sequence Timeout alarm to 60 secs per DIN70121*/
						logtime();
						fprintf(stderr, "alarm was already set\n");
					}
				}
			}
			/**************************************************************************************************/


			/**************************************************************************************************/
			#ifdef ABC170
				else if (i == sd) /*Data arriving on an already-connected CAN server socket */
				{
					// printf("CAN message received\n");

					while(ReadBCM(i, &rxmsg) == 0)
					{
						if (ParseRxCAN(&rxmsg) < 0)
						{
							logtime();
							fprintf(stderr, "Error Parsing CAN Frame!\n\n");
						}
					}
				}
			#endif
			/**************************************************************************************************/


			/**************************************************************************************************/
			#ifdef UPER
				else if (i == sd_CAN) /*Data arriving on an already-connected CAN server socket */
				{
					// printf("CAN message received\n");

					while(Read_BCM(i, &RX_msg) == 0)
					{
						if (Parse_RX_CAN(&RX_msg) < 0)
						{
							logtime();
							fprintf(stderr, "Error Parsing CAN Frame!\n");
						}
					}
				}
			#endif
			/**************************************************************************************************/


			/**************************************************************************************************/
			#ifdef MAXWELL
				else if (i == mx_sd_CAN) /*Data arriving on an already-connected CAN server socket */
				{
					// printf("CAN message received\n");

					while(read_CAN(i, &rx_msg) == 0)
					{
						if (mx_parse_RX_CAN(&rx_msg) < 0)
						{
							logtime();
							fprintf(stderr, "Error Parsing CAN Frame!\n");
						}
					}
				}
			#endif
			/**************************************************************************************************/



			/**************************************************************************************************/
			else if (i == alarmsfd) /*Sequence Timer*/
			{
						 logtime();
						 fprintf(stderr, "Error SECC Sequence Timeout.\n");
						 /*
						  * 2.  Terminate V2G Session
						  * 3.  Close TCP connection*/
						 /*disable EVSE output*/
						 closetcp();

						 /*read file descriptor*/
						nread = read(alarmsfd, &info, sizeof (info));
						 if (nread < 0)
						  {
							logtime();
							fprintf(stderr, "Error reading sequence timer alarm.\n");
						  }
			}
			/**************************************************************************************************/


			/**************************************************************************************************/
			else if (i == mqtt_timerfd) /*MQTT Subscription Timer*/
			{
				char output[700];
				char j1772[300];
				char status[800];
				char meter[700];
				int rc;

				//PRINTF("MQTT Client State: (%d)", mqttCtx.stat);
				if((mqttCtx.stat != WMQ_WAIT_MSG))  //maybe not connected or disconnected from broker
				{
					//printf("MQTT Client Disconnected?....mqttCtx.stat = %d\n\r", mqttCtx.stat);
					//usleep(15000); //stagger time between messages
					rc = mqttclient_state_machine(&mqttCtx);  //try to connect again
					if (rc != MQTT_CODE_CONTINUE)  //did not connect
					{
						PRINTF("\n");
					}
				}

				if((mqttCtx.stat == WMQ_WAIT_MSG))
				{
					SerializeStatusObj(output);
					sprintf(status, "%s", output);
					PublishMQTTObj(vPUB_TOPICS[0], status);

					SerializeJ1772Obj(output);
					sprintf(j1772, "%s", output);
					PublishMQTTObj(vPUB_TOPICS[1], j1772);

					if(ocpp_state == Charging)
					{
						SerializemeterObj(output);
						sprintf(meter, "%s", output);
						PublishMQTTObj(vPUB_TOPICS[2], meter);
					}
						MQTT_conn_attempts = 0;
						MQTT_pub_attempts = 0;
					}
					else
					{
					if((mqttCtx.stat == WMQ_PUB))
					{
						//PRINTF("Did not publish\n");
						MQTT_pub_attempts++;
						if(MQTT_pub_attempts > 4)
						{
							mqttCtx.stat = WMQ_BEGIN; //reset the state machine and try again
							MQTT_pub_attempts = 0;
						}
					}
					if((mqttCtx.stat == WMQ_TCP_CONN))
					{
						//won't connect to broker
						MQTT_conn_attempts++;
						if ((MQTT_conn_attempts > 9) && (ocpp_state == Available))
						{
							PRINTF("Exiting Application Due to MQTT Connection Attempts\n");
							return -1; //this will close the application
						}
					}
				}

				/*read file descriptor*/
				nread = read(mqtt_timerfd, &info, sizeof (info));
				if (nread < 0)
				{
				logtime();
				fprintf(stderr, "Error reading mqtt subscription timer alarm.\n");
				}
			}
			/**************************************************************************************************/


			/**************************************************************************************************/
			else if (i == timersfd) /*Oscillator Timer*/
			{

				if((hlc_state >= PRECHARGE_CableCheck)&&(hlc_state < SHUTDOWN_PowerDelivery)) /*PEV Should have closed S2 within 1.5 sec*/
				{
					if(!((J1772.pilot_state == C2) || (J1772.pilot_state == D2)))
					{
						logtime();
						fprintf(stderr, "Error Oscillator Timeout, PEV did not close S2 within 1.5 sec.\n");
						EVSESHUTDOWN = 1; //EVSEStatus will be updated
					}
					else
						StateC_Donly = 1; /*set flag true (will be monitored from while (1)*/
				}

				if ((hlc_state == SHUTDOWN_PowerDelivery) || (hlc_state == SHUTDOWN_SessionStop) || (hlc_state == SHUTDOWN_WeldingDetection))
				{	/*PEV Should had opened S2 after 1.5 sec*/
					if((J1772.pilot_state == C2) || (J1772.pilot_state == D2))
					{
						logtime();
						fprintf(stderr, "Error Oscillator Timeout, PEV did not open S2 within 1.5 sec.\n");
						EVSESHUTDOWN = 1; //EVSEStatus will be updated
					}
				}

				if(hlc_state == SHUTDOWN_Connected) /*1.5 seconds has elapses after SessionStopRes sent*/
				{
					printf("Shut off Pilot PWM\n");
					bitbucket = system(PWMOFF); /*shutoff oscillator*/
				}
				/*
				* 2.  Terminate V2G Session
				* 3.  Close TCP connection*/
				/*closetcp();*/

				/*read file descriptor*/
				nread = read(timersfd, &info, sizeof (info));
				if (nread < 0)
				{
					logtime();
					fprintf(stderr, "Error reading oscillator timer alarm.\n");
				}
			}
			/**************************************************************************************************/


			/**************************************************************************************************/
			else if (i == killsfd) /*SIGINT*/
			{
				logtime();
				bitbucket = system(PWMOFF);  /*turn off 5% pwm*/
				// Disable ADC periodic trigger and buffer
				adc_control(OFF);
				// printf("\nShutting down Application..\n");
				set_text_color(color.RESET);

				printf("Closing Application.\n");
				/*EVSE Emergency Shutdown
				* 1. Turn off PWM Oscillator
				* 2.  Terminate V2G Session
				* 3.  Close TCP connection*/
				
				/*disable EVSE output*/
				closetcp();
				shutdown(sdSDP, 1);  //inform we are going to close
				close(sdSDP); /*close SDP socket as well*/
				#ifdef ABC170
					close(sd); /*close CAN socket*/
				#endif
				#ifdef UPER
					close(sd_CAN);
				#endif
				exit(0);
			}
			/**************************************************************************************************/


			/**************************************************************************************************/
			else
			{
				logtime();
				printf("Unknown Socket\n\r");
				return -1; /*this will close application*/
			}
			/**************************************************************************************************/
		}
	}

	return 0;
}
//==========================================================================================================
