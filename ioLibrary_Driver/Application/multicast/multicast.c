#include "multicast.h"
#include <stdio.h>
#include "socket.h"
#include "wizchip_conf.h"

/**
 * @brief   UDP Multicast loopback test
 * @param   sn: Socket Number
 * @param   buf: Data sending and receiving cache
 * @param   multicast_mac: Multicast MAC address
 * @param   multicast_ip:  Multicast IP address
 * @param   multicast_port:Multicast port
 * @return  value for SOCK_ERRORs,return 1:no error
*/
int32_t multicast_loopback(uint8_t sn, uint8_t* buf, uint8_t* multicast_mac, uint8_t* multicast_ip, uint16_t multicast_port)
{
   int32_t  ret;
   uint16_t size, sentsize;
   uint8_t destip[4];
   uint16_t destport, port=50000;
   
   switch(getSn_SR(sn))
   {
      case SOCK_UDP :
         if((size = getSn_RX_RSR(sn)) > 0)
         {
            if(size > DATA_BUF_SIZE) size = DATA_BUF_SIZE;
            ret = recvfrom(sn, buf, size, destip, (uint16_t*)&destport);
            buf[ret]=0x00;
            printf("recv from [%d.%d.%d.%d][%d]: %s\r\n",destip[0],destip[1],destip[2],destip[3],destport,buf);
            if(ret <= 0)
            {
#ifdef _MULTICAST_DEBUG_
               printf("%d: recvfrom error. %ld\r\n",sn,ret);
#endif
               return ret;
            }
            size = (uint16_t) ret;
            sentsize = 0;
            while(sentsize != size)
            {
               ret = sendto(sn, buf+sentsize, size-sentsize, destip, destport);
               if(ret < 0)
               {
#ifdef _MULTICAST_DEBUG_
                  printf("%d: sendto error. %ld\r\n",sn,ret);
#endif
                  return ret;
               }
               sentsize += ret; // Don't care SOCKERR_BUSY, because it is zero.
            }
         }

         break;
      case SOCK_CLOSED:
#ifdef _MULTICAST_DEBUG_
         printf("%d:Multicast Loopback start\r\n",sn);
#endif
         setSn_DIPR(0, multicast_ip);
         setSn_DPORT(0, multicast_port);
         setSn_DHAR(0, multicast_mac);
         if((ret = socket(sn, Sn_MR_UDP, port, Sn_MR_MULTI)) != sn)
            return ret;
#ifdef _MULTICAST_DEBUG_
         printf("%d:Opened, UDP Multicast Socket\r\n", sn);
         printf("%d:Multicast Group IP - %d.%d.%d.%d\r\n", sn, multicast_ip[0], multicast_ip[1], multicast_ip[2], multicast_ip[3]);
         printf("%d:Multicast Group Port - %d\r\n", sn, multicast_port);
#endif
         break;
      default :
         break;
   }
   return 1;
}

int32_t multicast_recv(uint8_t sn, uint8_t* buf, uint8_t* multicast_mac, uint8_t* multicast_ip, uint16_t multicast_port)
{
   int32_t  ret;
   uint16_t size, port=3000;
   uint8_t destip[4];
   uint16_t destport;
   
   switch(getSn_SR(sn))
   {
      case SOCK_UDP :
         if((size = getSn_RX_RSR(sn)) > 0)
         {
            if(size > DATA_BUF_SIZE) size = DATA_BUF_SIZE;
            ret = recvfrom(sn, buf, size, destip, (uint16_t*)&destport);
            if(ret <= 0)
            {
#ifdef _MULTICAST_DEBUG_
               printf("%d: recvfrom error. %ld\r\n",sn,ret);
#endif
               return ret;
            }
            size = (uint16_t) ret;
#ifdef _MULTICAST_DEBUG_
            printf("\r\nrecv size : %d\r\n", size);
            for(int i=0; i<size; i++)
            	printf("%c", buf[i]);
            printf("\r\n");
#endif
         }
         break;
      case SOCK_CLOSED:
#ifdef _MULTICAST_DEBUG_
         printf("%d:Multicast Recv start\r\n",sn);
#endif
         setSn_DHAR(sn, multicast_mac);
         setSn_DIPR(sn, multicast_ip);
         setSn_DPORT(sn, multicast_port);
         if((ret = socket(sn, Sn_MR_UDP, port, Sn_MR_MULTI)) != sn)
            return ret;
#ifdef _MULTICAST_DEBUG_
         printf("%d:Opened, UDP Multicast Socket\r\n", sn);
         printf("%d:Multicast Group IP - %d.%d.%d.%d\r\n", sn, multicast_ip[0], multicast_ip[1], multicast_ip[2], multicast_ip[3]);
         printf("%d:Multicast Group Port - %d\r\n", sn, multicast_port);
#endif
         break;
      default :
         break;
   }
   return 1;
}
