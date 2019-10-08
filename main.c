#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "dyad.h"

#define PFX "."

int prt = 6667;
char *srv = "irc.undernet.org";
char *chn = "#gametime";
char *nck = "siesto";
char *pss = NULL;
char *mst = "siesta";

int isReg = 0;
int isAut = 0;

int ticks = 0;

char *trim(char *str)
{
	size_t len = 0;
	char *frontp = str;
	char *endp = NULL;

	if (str == NULL) {
		return NULL;
	}
	if (str[0] == '\0') {
		return str;
	}

	len = strlen(str);
	endp = str + len;

	/* Move the front and back pointers to address the first non-whitespace
	 * characters from each end.
	 */
	while (isspace((unsigned char)*frontp)) {
		++frontp;
	}
	if (endp != frontp) {
		while (isspace((unsigned char)*(--endp)) && endp != frontp) {
		}
	}

	if (frontp != str && endp == frontp) {
		*str = '\0';
	} else if (str + len - 1 != endp) {
		*(endp + 1) = '\0';
	}

	/* Shift the string so that it starts at str so that if it's dynamically
	 * allocated, we can still free it on the returned pointer.  Note the reuse
	 * of endp to mean the front of the string buffer now.
	 */
	endp = str;
	if (frontp != str) {
		while (*frontp) {
			*endp++ = *frontp++;
		}
		*endp = '\0';
	}

	return str;
}

static char *skip(char *s, char c)
{
	while (*s != c && *s != '\0')
		s++;
	if (*s != '\0')
		*s++ = '\0';
	return s;
}

static void onConnect(dyad_Event * e)
{
	if (pss)
		dyad_writef(e->stream, "PASS %s\r\n", pss);
	dyad_writef(e->stream, "NICK %s\r\n", nck);
	dyad_writef(e->stream, "USER %s %s %s :%s\r\n", nck, nck, nck, nck);
}

static void onError(dyad_Event * e)
{
	printf("error: %s\n", e->msg);
}

static void onLine(dyad_Event * e)
{
	char *cmd, *usr, *par, *txt;

//  printf("%s\n",e->data);

	cmd = strdup(e->data);

	usr = srv;
	if (!cmd || !*cmd)
		return;
	if (cmd[0] == ':') {
		usr = cmd + 1;
		cmd = skip(usr, ' ');
		if (cmd[0] == '\0')
			return;
		skip(usr, '!');
	}
	skip(cmd, '\r');
	par = skip(cmd, ' ');
	txt = skip(par, ':');
	trim(par);

	trim(txt);

/*
	printf("usr: %s\n",usr);
	printf("cmd: %s\n",cmd);
	printf("par: %s\n",par);
	printf("txt: %s\n",txt);
	printf("\n");
//*/

	if (!strcmp(cmd, "PING")) {
		dyad_writef(e->stream, "PONG %s\r\n", txt);
	} else if (!strcmp(cmd, "001")) {
		printf("connected.\n");
		dyad_writef(e->stream, "JOIN %s\r\n", chn);
		isReg = 1;
	} else if (!strcmp(cmd, "PRIVMSG")) {

		printf("<%s> %s\n", usr, txt);

		if (!strcmp(usr, mst)) {
			if (!strncmp(txt, PFX "quit", 5)) {
				if (strlen(txt) > 6) {
					dyad_writef(e->stream, "QUIT :%s\r\n",
						    txt + 6);
				} else {
					dyad_writef(e->stream, "QUIT\r\n");
				}
			}
		}

	} else if (!strcmp(cmd, "JOIN")) {
		printf("%s joined %s\n", usr, strlen(par) ? par : txt);
	} else if (!strcmp(cmd, "PART")) {
		printf("%s parted %s\n", usr, strlen(par) ? par : txt);
	} else if (!strcmp(cmd, "QUIT")) {
		printf("%s exits irc message '%s'\n", usr, txt);
	}

	free(cmd);
	cmd = NULL;

}

static void onTick(dyad_Event * e)
{
	ticks++;
}

int main(int argc, char **argv)
{

	dyad_Stream *s;

	dyad_init();

	s = dyad_newStream();

	dyad_addListener(s, DYAD_EVENT_CONNECT, onConnect, NULL);
	dyad_addListener(s, DYAD_EVENT_ERROR, onError, NULL);
	dyad_addListener(s, DYAD_EVENT_LINE, onLine, NULL);
	dyad_addListener(s, DYAD_EVENT_TICK, onTick, NULL);

	printf("connecting...\n");

	dyad_connect(s, srv, prt);

	while (dyad_getStreamCount() > 0) {
		dyad_update();
	}

	dyad_shutdown();

	return 0;
}
