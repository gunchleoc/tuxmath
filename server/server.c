/*
*  C Implementation: server.c
*
*       Description: Server program for LAN-based play in Tux,of Math Command.
*
*
* Author: Akash Gangil, David Bruce, and the TuxMath team, (C) 2009
* Developers list: <tuxmath-devel@lists.sourceforge.net>
*
* Copyright: See COPYING file that comes with this distribution.  (Briefly, GNU GPL).
*
* NOTE: This file was initially based on example code from The Game Programming Wiki
* (http://gpwiki.org), in a tutorial covered by the GNU Free Documentation License 1.2.
* No invariant sections were indicated, and no separate license for the example code
* was listed. The author was also not listed. AFAICT,this scenario allows incorporation of
* derivative works into a GPLv2+ project like TuxMath - David Bruce 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
#include "SDL_net.h"
#include "transtruct.h"
#include "mathcards.h"
#include "server.h" 

TCPsocket sd, csd; /* Socket descriptor, Client socket descriptor */

int main(int argc, char **argv)
{ 
  IPaddress ip, *remoteIP;
  int quit, quit2;
  char buffer[NET_BUF_LEN];
  int network_function = -1;
  //     size_t length;
  MC_FlashCard flash;

        
  if (SDLNet_Init() < 0)
  {
    fprintf(stderr, "SDLNet_Init: %s\n", SDLNet_GetError());
    exit(EXIT_FAILURE);
  }
 
  /* Resolving the host using NULL make network interface to listen */
  if (SDLNet_ResolveHost(&ip, NULL, DEFAULT_PORT) < 0)
  {
    fprintf(stderr, "SDLNet_ResolveHost: %s\n", SDLNet_GetError());
    exit(EXIT_FAILURE);
  }
 
  /* Open a connection with the IP provided (listen on the host's port) */
  if (!(sd = SDLNet_TCP_Open(&ip)))
  {
    fprintf(stderr, "SDLNet_TCP_Open: %s\n", SDLNet_GetError());
    exit(EXIT_FAILURE);
  }
 
  /* Wait for a connection, send data and term */
  quit = 0;
  while (!quit)
  {
    /* This check the sd if there is a pending connection.
     * If there is one, accept that, and open a new socket for communicating */
    if ((csd = SDLNet_TCP_Accept(sd)))
    {
      /* Now we can communicate with the client using csd socket
       * sd will remain opened waiting other connections */
 
      /* Get the remote address */
      if ((remoteIP = SDLNet_TCP_GetPeerAddress(csd)))
        /* Print the address, converting in the host format */
        printf("Host connected: %x %d\n",
	       SDLNet_Read32(&remoteIP->host),
	       SDLNet_Read16(&remoteIP->port));
      else
        fprintf(stderr, "SDLNet_TCP_GetPeerAddress: %s\n", SDLNet_GetError());
                        
      quit2 = 0;
      while (!quit2)
      {
        if (SDLNet_TCP_Recv(csd, buffer, NET_BUF_LEN) > 0)
        {
          network_function = -1;
          printf("Client say: %s\n", buffer);
                                       
          //'a' for the setting up the question list                                           
          if(strcmp(buffer,"a") == 0)
          {
            network_function = SETUP_QUESTION_LIST;              
          } 
                                       
          //'b' for asking for a question(flashcard)
          if(strcmp(buffer,"b") == 0)
          {
            network_function = SEND_A_QUESTION;              
          } 

          if(strcmp(buffer, "exit") == 0) /* Terminate this connection */
          {
            quit2 = 1;
            printf("Terminate connection\n");
          }

          if(strcmp(buffer, "quit") == 0) /* Quit the program */
          {
            quit2 = 1;
            quit = 1;
            printf("Quit program\n");
          }

          switch(network_function)
          {
            case SETUP_QUESTION_LIST:  //mainly to setup the question list
            {
              if (!MC_StartGame())
              {
                fprintf(stderr, "\nMC_StartGame() failed!");
                return 0;
              }                                                                                  
                                                 
              break;                                           
            } 

            case SEND_A_QUESTION:
            {
              if (!MC_NextQuestion(&flash))
              { 
                /* no more questions available */
                printf("MC_NextQuestion() returned NULL - no questions available\n");
              }
              else
              {                                     
                printf("WILL SEND >>\n");  
                printf("QUESTION_ID       :      %d\n", flash.question_id);
                printf("FORMULA_STRING    :      %s\n", flash.formula_string);
                printf("ANSWER STRING     :      %s\n", flash.answer_string);
                printf("ANSWER            :      %d\n",flash.answer);
                printf("DIFFICULTY        :      %d\n",flash.difficulty);

                if(!SendQuestion(flash))
                {
                  printf("Unable to send Question\n");
                }
              } 

              break;
            } 
        
            default:
              break;
          }

        }
      }
 
      /* Close the client socket */
      SDLNet_TCP_Close(csd);
    }
  }
 
  SDLNet_TCP_Close(sd);
  SDLNet_Quit();
 
  return EXIT_SUCCESS;
}


//function to send a flashcard(question) from the server to the client
int SendQuestion(MC_FlashCard flash)
{
  char *ch;
  int x;

  char buf[NET_BUF_LEN];
  snprintf(buf, NET_BUF_LEN, 
                "%s\t%d\t%d\t%d\t%s\t%s\n",
                "SEND_QUESTION",
                flash.question_id,
                flash.difficulty,
                flash.answer,
                flash.answer_string,
                flash.formula_string);
  printf("buf is: %s\n", buf);
  x = SDLNet_TCP_Send(csd, buf, sizeof(buf));
  printf("SendQuestion() - buf sent:::: %d bytes\n", x);


  //old code:
  x = SDLNet_TCP_Send(csd, &(flash.question_id), sizeof(flash.question_id));
  printf("no:(1):::QUESTION_ID::::Sent %d bytes\n", x);
      
  x = SDLNet_TCP_Send(csd,&(flash.difficulty),sizeof(flash.difficulty));
  printf("no:(2):::DIFFICULTY::::Sent %d bytes\n", x);

  x = SDLNet_TCP_Send(csd, &(flash.answer), sizeof(flash.answer));
  printf("no:(3)::::ANSWER:::Sent %d bytes\n",x);

  x = SDLNet_TCP_Send(csd, flash.answer_string, strlen(flash.answer_string) + 1);
  printf("no:(4):::ANSWER_STRING::::Sent %d bytes\n", x);

  x = SDLNet_TCP_Send(csd, flash.formula_string, strlen(flash.formula_string) + 1);
  printf("no:(5):::FORMULA_STRING::::Sent %d bytes\n", x);
    
  return 1;


}






