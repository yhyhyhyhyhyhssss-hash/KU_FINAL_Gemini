/*
 * Copyright (C) 2018 Xilinx, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 */

#include "eth_server.h"

#include "../src/bsp.h"
#include "../src/dbuffer.h"

 static struct sockaddr_in address_openamip,remote_openamip;
 static int sd_openamip = 0;

 static const char http_html_hdr[] =
     "HTTP/1.1 200 OK\r\nContent-type: text/html\r\n\r\n";
 static const char http_index_html[] =
 "<html><head><title>Congrats!</title></head>\
 <body><h1 align=\"center\">Hello World!</h1>\
 <h2 align=\"center\">Welcome to Fire lwIP HTTP Server!</h1>\
 <p align=\"center\">This is a small test page, served by httpserver-netconn.</p>\
 <p align=\"center\"><a href=\"http://www.firebbs.cn/forum.php/\">\
 <font size=\"6\"> 野火电子论坛 </font> </a></p>\
 <a href=\"http://www.firebbs.cn/forum.php/\">\
 <p align=\"center\"><img src=\"http://www.firebbs.cn/data/attachment/portal/\
 201806/05/163015rhz7mbgbt0zfujzh.jpg\" /></a>\
 </body></html>";

 static const char http_index_htm2[] =
 "<html><title>LOGIN</title> <body bgcolor=\"green\"><h1 align=\"center\">登录界面</h1>\
 <form method='get' action='Login.cgi'> 用户名:<input type='text' name='username'><input type='submit' value='设 置'> </form><form method='get' action='Login.cgi'> 用户名:<input type='text' name='username'><br> 密 码:<input type='password' name='password'><input type='submit' value='设 置'> </form></body></html>";

 static const char shht_menu1_http_html[] =
"<html><head><title>页面标题</title><meta >\
<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><style>\
* {box-sizing: border-box;}\
body {font-family: Arial; margin: 0;}\
.header {padding: 80px;text-align: center;background: #1abc9c;color: white;}\
.header h1 {font-size: 40px;}\
.row {  display: -ms-flexbox; display: flex;-ms-flex-wrap: wrap; flex-wrap: wrap;}\
.side {-ms-flex: 30%; flex: 30%;background-color: #f1f1f1;padding: 20px;}\
.main {   -ms-flex: 70%; flex: 70%;background-color: white;padding: 20px;}\
@media screen and (max-width: 700px) {.row {   flex-direction: column;}}";

 static const char shht_menu2_http_html[] =
"</style></head><body>\
<div class=\"header\"><h1>上合航天7020网页测试</h1><p></p></div>\
<div class=\"row\"><div class=\"side\"><h2>项目介绍</h2><p>关于项目的介绍..</p><h3>其他说明</h3><p>更多内容</p></div>\
<div class=\"main\"><h2>参数配置</h2>";
 static const char shht_menu_para1_http_html[] =
"<form method='get' action='Login.cgi' >  参数1 :<input type='text' name='para1'><input type='submit'value='发 送'>";//(current:123)<br><br></form>";
 static const char shht_menu_para2_http_html[] =
 "<form method='get' action='Login.cgi' >  参数2 :<input type='text' name='para2'><input type='submit' value='发 送'>";//(current:123)<br><br></form>";
 static const char shht_menu_para3_http_html[] =
 "<form method='get' action='Login.cgi' >  参数3 :<input type='text' name='para3'><input type='submit' value='发 送'>";//(current:123)<br><br></form>";
 static const char shht_menu_para4_http_html[] =
 "<form method='get' action='Login.cgi' >  参数4 :<input type='text' name='para4'><input type='submit' value='发 送'>";//(current:123)<br> <br></form>";
 static const char shht_menu_para5_http_html[] =
 "<form method='get' action='Login.cgi' >  参数5 :<input type='text' name='para5'><input type='submit' value='发 送'>";//(current:123)<br><br></form>";
 static const char shht_menu_para6_http_html[] =
 "<form method='get' action='Login.cgi' >  参数6 :<input type='text' name='para6'><input type='submit' value='发 送'>";//(current:123)<br><br></form>";
 static const char shht_menu_para7_http_html[] =
 "<form method='get' action='Login.cgi' >  参数7 :<input type='text' name='para7'><input type='submit' value='发 送'>";//(current:123)<br><br></form>";
 static const char shht_menu_para8_http_html[] =
 "<form method='get' action='Login.cgi' >  参数8 :<input type='text' name='para8'><input type='submit' value='发 送'>";//(current:123)<br><br></form>";
 static const char shht_menu_para9_http_html[] =
 "<form method='get' action='Login.cgi' >  参数9 :<input type='text' name='para9'><input type='submit' value='发 送'>";//(current:123)<br><br></form>";
 static const char shht_menu_para10_http_html[] =
 "<form method='get' action='Login.cgi' >  参数10:<input type='text' name='para10'><input type='submit' value='发 送'>(current:123)<br><br></form>";
 static const char shht_menu3_http_html[] =
 //"<h2>其他功能</h2><p>..............</p><br></div></div></body></html><img src=\"data:image/gif;base64,R0lGODlhHAAmAKIHAKqqqsvLy0hISObm5vf394uLiwAAAP///yH5B…EoqQqJKAIBaQOVKHAXr3t7txgBjboSvB8EpLoFZywOAo3LFE5lYs/QW9LT1TRk1V7S2xYJADs=\">";
 "<img src=\"data:image/png;base64,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\" width=\"169\" height=\"69\">";

extern struct netif server_netif;
static shht_para_typedef shht_para = {
		.para1 ="                    ",
		.para2 ="                    ",
		.para3 ="                    ",
		.para4 ="                    ",
		.para5 ="                    ",
		.para6 ="                    ",
		.para7 ="                    ",
		.para8 ="                    ",
		.para9 ="                    ",
};
/* Interval time in seconds */
#define REPORT_INTERVAL_TIME (INTERIM_REPORT_INTERVAL * 1000)

void print_app_header(void)
{
	xil_printf("TCP server listening on port %d\r\n",
			TCP_CONN_PORT);
#if LWIP_IPV6==1
	xil_printf("On Host: Run $iperf -V -c %s%%<interface> -i %d -t 300 -w 2M\r\n",
			inet6_ntoa(server_netif.ip6_addr[0]),
			INTERIM_REPORT_INTERVAL);
#else
	xil_printf("On Host: Run $iperf -c %s -i %d -t 300 -w 2M\r\n",
			inet_ntoa(server_netif.ip_addr),
			INTERIM_REPORT_INTERVAL);
#endif /* LWIP_IPV6 */
}

void shht_openamip_send(const void *dataptr, size_t size)
{

	if(sd_openamip > 0){
		write(sd_openamip, dataptr, size);
	}
}

static void shht_xkz_send(int sd)
{   static char temp_buf[200];
    int j;

	memset(temp_buf,0,sizeof(temp_buf));
	j = sprintf(temp_buf,"a 1\r\n");
	j =j +sprintf(temp_buf+j,"w 1 -10.123 20.235 123456789 10000 91.0 223.52 0.10 -0.51 91.0\r\n");
	j =j +sprintf(temp_buf+j,"s 0 0 1 0 11 statusCode=12 StatusCode=25\r\n c 0 0 0 0 2\r\n");
	j =j +sprintf(temp_buf+j,"c 0 0 0 0 2\r\n");
	write(sd, temp_buf, j);
	//xil_printf("shht_xkz_send: %s\r\n", temp_buf);

}

static void shht_menu_http_send(int sd)
{   static char temp_buf[200];

	write(sd, http_html_hdr, sizeof(http_html_hdr)-1);
	write(sd, shht_menu1_http_html, sizeof(shht_menu1_http_html)-1);
	write(sd, shht_menu2_http_html, sizeof(shht_menu2_http_html)-1);

	memset(temp_buf,0,sizeof(temp_buf));
	sprintf(temp_buf,"%s(current:%.20s)<br><br></form>",shht_menu_para1_http_html,shht_para.para1);
	write(sd, temp_buf, sizeof(temp_buf)-1);

	memset(temp_buf,0,sizeof(temp_buf));
	sprintf(temp_buf,"%s(current:%.20s)<br><br></form>",shht_menu_para2_http_html,shht_para.para2);
	write(sd, temp_buf, sizeof(temp_buf)-1);

	memset(temp_buf,0,sizeof(temp_buf));
	sprintf(temp_buf,"%s(current:%.20s)<br><br></form>",shht_menu_para3_http_html,shht_para.para3);
	write(sd, temp_buf, sizeof(temp_buf)-1);

	sprintf(temp_buf,"%s(current:%.20s)<br><br></form>",shht_menu_para4_http_html,shht_para.para4);
	write(sd, temp_buf, sizeof(temp_buf)-1);

	sprintf(temp_buf,"%s(current:%.20s)<br><br></form>",shht_menu_para5_http_html,shht_para.para5);
	write(sd, temp_buf, sizeof(temp_buf)-1);

	sprintf(temp_buf,"%s(current:%.20s)<br><br></form>",shht_menu_para6_http_html,shht_para.para6);
	write(sd, temp_buf, sizeof(temp_buf)-1);

	sprintf(temp_buf,"%s(current:%.20s)<br><br></form>",shht_menu_para7_http_html,shht_para.para7);
	write(sd, temp_buf, sizeof(temp_buf)-1);

	sprintf(temp_buf,"%s(current:%.20s)<br><br></form>",shht_menu_para8_http_html,shht_para.para8);
	write(sd, temp_buf, sizeof(temp_buf)-1);

	sprintf(temp_buf,"%s(current:%.20s)<br><br></form>",shht_menu_para9_http_html,shht_para.para9);
	write(sd, temp_buf, sizeof(temp_buf)-1);

	write(sd, shht_menu_para10_http_html, sizeof(shht_menu_para10_http_html)-1);

	write(sd, shht_menu3_http_html, sizeof(shht_menu3_http_html)-1);


}

static void udp_openamip_socket_thread(void *arg)
{
	err_t err;
	int i,sock;

	int count;
	char recv_buf[RECV_BUF_SIZE] = {0};
	struct sockaddr_in from;
	socklen_t fromlen = sizeof(from);

	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		xil_printf("UDP server: Error creating Socket\r\n");
		return;
	}

	err = bind(sock, (struct sockaddr *)&address_openamip, sizeof(address_openamip));
	if (err != ERR_OK) {
		xil_printf("UDP server: Error on bind: %d\r\n", err);
		close(sock);
		return;
	}
	xil_printf("udp_openamip_thread\r\n");
	while (1) {
			if((count = lwip_recvfrom(sock, recv_buf, RECV_BUF_SIZE, 0,
					(struct sockaddr *)&from, &fromlen)) <= 0) {
				continue;
			}

			xil_printf("openamip_udp_recv_buf[%d]:%s\r\n",count,recv_buf);
			for(i=0;i<count;i++)
			{
				CLBUF_WR(LWIP_RECE.data, recv_buf[i]);
			}
			memset(recv_buf,0,RECV_BUF_SIZE);
			//gs_print("openamip_udp_recv_buf:%s\r\n", recv_buf);
			/*
			if (sendto(sock, recv_buf, count, 0,
							(struct sockaddr *)&from, fromlen) < 0) {
							xil_printf("Error in write\n\r");
						}
						*/
	}

}

extern void delay_ms(unsigned int mseconds);

void process_client_tcp(void *p)
{
	int sd = (int)p;
	char recv_buf[RECV_BUF_SIZE];
	int n, nwrote;

	struct sockaddr_in remote_client;


	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			xil_printf("TCP Client: Error in creating Socket\r\n");
			return;
		}

	memset(&remote_client, 0, sizeof(remote_client));
	remote_client.sin_family = AF_INET;
	remote_client.sin_port = htons(6666);
	remote_client.sin_addr.s_addr = inet_addr("169.254.99.173");

	if (connect(sd, (struct sockaddr*)&remote_client, sizeof(remote_client)) < 0) {
						xil_printf("TCP Client: Error on tcp_connect\r\n");
						close(sd);
						return;
					}

	while (1) {
		/* read a max of RECV_BUF_SIZE bytes from socket */
		if ((n = read(sd, recv_buf, RECV_BUF_SIZE)) < 0) {
			xil_printf("%s: error reading from socket %d, closing socket\r\n", __FUNCTION__, sd);
			//break;
		}

		/* break if the recved message = "quit" */
		if (!strncmp(recv_buf, "quit", 4))
			//break;

		/* break if client closed connection */
		if (n <= 5)
			break;


		/* handle request */
		  if ((nwrote = write(sd, recv_buf, n)) < 0) {
			  xil_printf("%s: ERROR responding to client echo request. received = %d, written = %d\r\n",
					__FUNCTION__, n, nwrote);
			  xil_printf("Closing socket %d\r\n", sd);
			  break;
		  }
		}

	/* close connection */
	close(sd);
	vTaskDelete(NULL);
}


void process_serve_request(void *p)
{
	int sd = (int)p;
	char recv_buf[RECV_BUF_SIZE];
	int n;
	char *t_buf,t_buf_end;

	while (1) {
		/* read a max of RECV_BUF_SIZE bytes from socket */
		if ((n = read(sd, recv_buf, RECV_BUF_SIZE)) < 0) {
			xil_printf("%s: error reading from socket %d, closing socket\r\n", __FUNCTION__, sd);
			break;
		}

		/* break if the recved message = "quit" */
		if (!strncmp(recv_buf, "quit", 4))
			break;

		/* break if client closed connection */
		if (n <= 5)
			break;

		xil_printf("recv_buf: %s\r\n", recv_buf);
		if(memcmp(recv_buf, "GET /", 5)== 0)
		{
			t_buf = strstr(recv_buf,"para");
			for(int i=0;;i++){
				if(t_buf[i] == ' ')
				{
					t_buf_end = i;
					break;
				}
			}
			switch(t_buf[4])
		    {
			  case '1':
				  xil_printf("t_buf_end:%d,sizeof(shht_para.para1):%d\n",t_buf_end,sizeof(shht_para.para1));
				  memset(shht_para.para1,' ',sizeof(shht_para.para1));
				  if(t_buf_end>6){
				     memcpy(shht_para.para1,t_buf+6,MIN(19,t_buf_end-6));
				  }
				  xil_printf("shht_para.para1:%s\n",shht_para.para1); break;

			  case '2':
				  xil_printf("t_buf_end:%d,sizeof(shht_para.para2):%d\n",t_buf_end,sizeof(shht_para.para2));
				  memset(shht_para.para2,' ',sizeof(shht_para.para2));
				  if(t_buf_end>6){
				     memcpy(shht_para.para2,t_buf+6,MIN(19,t_buf_end-6));
				  }
				  xil_printf("shht_para.para2:%s\n",shht_para.para2); break;

			  case '3':
				  xil_printf("t_buf_end:%d,sizeof(shht_para.para3):%d\n",t_buf_end,sizeof(shht_para.para3));
				  memset(shht_para.para3,' ',sizeof(shht_para.para3));
				  if(t_buf_end>6){
				     memcpy(shht_para.para3,t_buf+6,MIN(19,t_buf_end-6));
				  }
				  xil_printf("shht_para.para3:%s\n",shht_para.para3); break;

			  case '4':
				  xil_printf("t_buf_end:%d,sizeof(shht_para.para4):%d\n",t_buf_end,sizeof(shht_para.para4));
				  memset(shht_para.para4,' ',sizeof(shht_para.para4));
				  if(t_buf_end>6){
				     memcpy(shht_para.para4,t_buf+6,MIN(19,t_buf_end-6));
				  }
				  xil_printf("shht_para.para4:%s\n",shht_para.para4); break;

			  case '5':
				  xil_printf("t_buf_end:%d,sizeof(shht_para.para5):%d\n",t_buf_end,sizeof(shht_para.para5));
				  memset(shht_para.para5,' ',sizeof(shht_para.para5));
				  if(t_buf_end>6){
				     memcpy(shht_para.para5,t_buf+6,MIN(19,t_buf_end-6));
				  }
				  xil_printf("shht_para.para5:%s\n",shht_para.para5); break;

			  case '6':
				  xil_printf("t_buf_end:%d,sizeof(shht_para.para6):%d\n",t_buf_end,sizeof(shht_para.para6));
				  memset(shht_para.para6,' ',sizeof(shht_para.para6));
				  if(t_buf_end>6){
				     memcpy(shht_para.para6,t_buf+6,MIN(19,t_buf_end-6));
				  }
				  xil_printf("shht_para.para6:%s\n",shht_para.para6); break;

			  case '7':
				  xil_printf("t_buf_end:%d,sizeof(shht_para.para7):%d\n",t_buf_end,sizeof(shht_para.para7));
				  memset(shht_para.para7,' ',sizeof(shht_para.para7));
				  if(t_buf_end>6){
				     memcpy(shht_para.para7,t_buf+6,MIN(19,t_buf_end-6));
				  }
				  xil_printf("shht_para.para7:%s\n",shht_para.para7); break;

			  case '8':
				  xil_printf("t_buf_end:%d,sizeof(shht_para.para8):%d\n",t_buf_end,sizeof(shht_para.para8));
				  memset(shht_para.para8,' ',sizeof(shht_para.para8));
				  if(t_buf_end>6){
				     memcpy(shht_para.para8,t_buf+6,MIN(19,t_buf_end-6));
				  }
				  xil_printf("shht_para.para8:%s\n",shht_para.para8); break;

			  case '9':
				  xil_printf("t_buf_end:%d,sizeof(shht_para.para9):%d\n",t_buf_end,sizeof(shht_para.para9));
				  memset(shht_para.para9,' ',sizeof(shht_para.para9));
				  if(t_buf_end>6){
				     memcpy(shht_para.para9,t_buf+6,MIN(19,t_buf_end-6));
				  }
				  xil_printf("shht_para.para9:%s\n",shht_para.para9); break;

			  default:xil_printf("error%d\n",t_buf[0]);

		    }
			 //xil_printf("recv_buf:%s\n",recv_buf);

			shht_menu_http_send(sd);
		}else{
		/* handle request */
			//shht_xkz_send(sd);
		}
	}

	/* close connection */
	xil_printf("close(sd)\n");
	close(sd);
	vTaskDelete(NULL);
}

void process_serve_openamip(void *p)
{
	int sd = (int)p;
	char recv_buf[RECV_BUF_SIZE] = {0};
    int i,n;

	while (1) {
		/* read a max of RECV_BUF_SIZE bytes from socket */
		if ((n = read(sd, recv_buf, RECV_BUF_SIZE)) < 0) {
			xil_printf("%s: error reading from socket %d, closing socket\r\n", __FUNCTION__, sd);
			break;
		}

		xil_printf("openamip_tcp_recv_buf[%d]:%s\r\n",n, recv_buf);
		//gs_print("openamip_tcp_recv_buf:%s\r\n", recv_buf);
		for(i=0;i<n;i++)
		{
			CLBUF_WR(LWIP_RECE.data, recv_buf[i]);
		}
		//shht_xkz_send(sd);
		memset(recv_buf,0,RECV_BUF_SIZE);

	}

	/* close connection */
	xil_printf("close(sd)\n");
	close(sd);
	vTaskDelete(NULL);
}


void tcp_openamip_thread(void *p)
{
	int sock_openamip;

	int size_openamip;

    memset(&address_openamip, 0, sizeof(address_openamip));

	if ((sock_openamip = lwip_socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		xil_printf("TCP server_openamip: Error creating Socket\r\n");
		return;
	}
	address_openamip.sin_family = AF_INET;
	address_openamip.sin_port = htons(6666);
	address_openamip.sin_addr.s_addr = INADDR_ANY;

	if (bind(sock_openamip, (struct sockaddr *)&address_openamip, sizeof (address_openamip)) < 0) {
		xil_printf("TCP server_openamip: Unable to bind to port 6666\r\n");
		close(sock_openamip);
		return;
	}

	if (listen(sock_openamip, 3) < 0) {
		xil_printf("TCP server_openamip: tcp_listen failed\r\n");
		close(sock_openamip);
		return;
	}

	size_openamip = sizeof(remote_openamip);

	xil_printf("tcp_openamip_thread\r\n");
	while (1) {
		if ((sd_openamip = lwip_accept(sock_openamip, (struct sockaddr *)&remote_openamip, (socklen_t *)&size_openamip)) > 0) {
			sys_thread_new("echos", process_serve_openamip,
				(void*)sd_openamip,
				TCP_SERVER_THREAD_STACKSIZE,
				DEFAULT_THREAD_PRIO);
			xil_printf("sys_thread_new new_sd_openamip = %x,port = %d\r\n",sd_openamip,remote_openamip.sin_port);
			//shht_xkz_send(sd_openamip);
		}
	}
}


void start_application(void)
{
	int sock, new_sd;
#if LWIP_IPV6==1
	struct sockaddr_in6 address, remote;
#else
	struct sockaddr_in address, remote;
#endif /* LWIP_IPV6 */
	int size;

	/* set up address to connect to */
        memset(&address, 0, sizeof(address));

#if LWIP_IPV6==1
	if ((sock = lwip_socket(AF_INET6, SOCK_STREAM, 0)) < 0) {
		xil_printf("TCP server: Error creating Socket\r\n");
		return;
	}
	address.sin6_family = AF_INET6;
	address.sin6_port = htons(TCP_CONN_PORT);
	address.sin6_len = sizeof(address);
#else
	if ((sock = lwip_socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		xil_printf("TCP server: Error creating Socket\r\n");
		return;
	}
	address.sin_family = AF_INET;
	address.sin_port = htons(TCP_CONN_PORT);
	address.sin_addr.s_addr = INADDR_ANY;



#endif /* LWIP_IPV6 */

	if (bind(sock, (struct sockaddr *)&address, sizeof (address)) < 0) {
		xil_printf("TCP server: Unable to bind to port %d\r\n",
				TCP_CONN_PORT);
		close(sock);
		return;
	}

	if (listen(sock, 3) < 0) {
		xil_printf("TCP server: tcp_listen failed\r\n");
		close(sock);
		return;
	}

	size = sizeof(remote);

	//tcp_openamip
	sys_thread_new("tcp_openamip_thread", tcp_openamip_thread,NULL,
											TCP_SERVER_THREAD_STACKSIZE,
											DEFAULT_THREAD_PRIO);
	//udp_openamip
	sys_thread_new("udp_openamip_socket", udp_openamip_socket_thread,NULL ,
											TCP_SERVER_THREAD_STACKSIZE,
											DEFAULT_THREAD_PRIO);
	xil_printf("openamip_thread\r\n");
	while (1) {

		//web
		if ((new_sd = lwip_accept(sock, (struct sockaddr *)&remote, (socklen_t *)&size)) > 0) {
			sys_thread_new("echos", process_serve_request,
				(void*)new_sd,
				TCP_SERVER_THREAD_STACKSIZE,
				DEFAULT_THREAD_PRIO);
			xil_printf("sys_thread_new new_sd = %x,port = %d\r\n",new_sd,remote.sin_port);

		}

	}
}
