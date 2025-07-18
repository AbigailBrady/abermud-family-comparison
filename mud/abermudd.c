/*
**  Accept connections to a socket and run a program using the socket
**  for I/O.
**
**  Another fine hack by Mycroft The Maintainer (Charles Hannum)
*/

#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
extern int errno;

#define	DEF_PORT	6715
#define	DEF_PROG	"Exec"
#define	EXEC_TRIES	6
#define	EXEC_DELAY	10
#define	LOGFILE		"DATA/logs/socket.log"

inline
reapchild()
{
  while (1)
    {
      switch (wait3(NULL, WNOHANG, NULL))
	{
	case 0 :
	  return;
	case -1 :
	  if (errno == ECHILD)
	    return;
	  if (errno == EINTR)
	    break;
	  perror("wait3()");
	  exit(errno);
	default:
	  break;
	}
    }
}


int
main (int argc, char **argv)
{
  int s, ns, opt, rc, t;
  struct sockaddr_in name;
  char *host;
  struct hostent *hinfo;
  time_t now;
  int port;
  char *prog;

  argc--;

  if (argc)
    {
      prog = *++argv;
      argc--;
    }
  else
    prog = DEF_PROG;

  if (argc)
    {
      port = atoi(*++argv);
      argc--;
    }
  else
    port = DEF_PORT;

  printf ("Using port %d to run %s ...\n", port, prog);

  /* Who cares about children?  Let's kill 'em! */
  signal(SIGCHLD, reapchild);

  /* Create a socket */
  s = socket(AF_INET, SOCK_STREAM, 0);
  if (s == -1)
    {
      perror("socket()");
      exit(errno);
    }
#ifdef DEBUG
  printf("Socket %d, error %d\n",s,errno);
#endif

  /* Make it reusable */
  opt = 1;
  setsockopt (s, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt));

  /* Assign it to a port */
  name.sin_family = AF_INET;
  name.sin_addr.s_addr = INADDR_ANY;
  name.sin_port = htons(port);

  /* Bind it to the port */
  rc = bind(s, (struct sockaddr *) & name, sizeof(name));
  if (rc == -1)
    {
      perror("bind()");
      exit(errno);
    }

  /* Listen */
  rc = listen(s, 5);
  if (rc == -1)
    {
      perror("listen()");
      exit(errno);
    }

  /* Open the log file */
  t = open(LOGFILE, O_RDWR|O_CREAT|O_APPEND, 0644);
  if (t == -1)
    {
      perror("open(logfile)");
      exit(errno);
    }

  /* Redirect stdout and stderr to the log file */
  dup2(t, fileno(stdout));
  dup2(t, fileno(stderr));
  close(t);

  /* Put us in the background */
  switch (fork())
    {
    case -1 :
      perror("fork(server)");
      exit(errno);
    case 0 :
      break;
    default :
      now = time(NULL);
      printf("Server started at %.24s\n", ctime(&now));
      exit(0);
    }

  /* Get rid of our control terminal */
  t = open("/dev/tty", O_RDWR);
  if (t >= 0)
    {
      ioctl(t, TIOCNOTTY, 0);
      close(t);
    }

  /* Okay, I'm sick of <defunct> processes.  We're going to do something
     REALLY disgusting here. */

  /* Make the socket non-blocking so we won't hang inside the following loop */
  {
    int opt = 1;
    ioctl(s, FIONBIO, &opt);
  }

  /* Wait for connections */
  while (1)
    {
      int addrlen = sizeof(name),
      timeout;
      fd_set fds;

      timeout = 15;
      FD_ZERO(&fds);
      FD_SET(s, &fds);

    reselect:
      switch (select(FD_SETSIZE, &fds, NULL, NULL, &timeout))
	{
	case -1 :
	  if (errno == EINTR)
	    goto reselect;
	  perror("select()");
	  exit(errno);
	case 0 :
	  reapchild();
	  continue;
	default :
	  do {
	    ns = accept(s, (struct sockaddr *)&name, &addrlen);
	  } while ((ns == -1) && (errno == EINTR));
	  if (errno == EWOULDBLOCK)
	    continue;
	  if (ns == -1)
	    {
	      perror("accept()");
	      exit(errno);
	    }
	}

      /* We want our new socket to do blocking I/O by default */
      {
	int opt = 0;
	ioctl(ns, FIONBIO, &opt);
      }

      /* I told you it was disgusting.  But it should reap all the <defunct>
	 processes every 15 seconds.  Trust me, it's worth it.  - Mycroft */

#ifdef DEBUG
      printf("New socket %d, error %d\n", ns, errno);
#endif DEBUG

      /* Get host information */
      hinfo = gethostbyaddr (&name.sin_addr, sizeof(struct in_addr),
			     name.sin_family);
      if (hinfo != NULL)
	host = hinfo->h_name;
      else
	host = inet_ntoa(name.sin_addr);
      now = time(NULL);
      printf("connect at %.24s from %s\n", ctime(&now), host);
      fflush(stdout);

      /* Fork a new process and hand off the socket */
      if ((rc = fork()) == -1)
	perror("fork(client)");
      else if (!rc)
	{
	  int tries = EXEC_TRIES;

	  /* We care about children! */
	  signal(SIGCHLD, SIG_DFL);

	  /* Make the socket sdtin, stdout, and stderr */
	  dup2(ns, fileno(stdin));
	  dup2(ns, fileno(stdout));
	  dup2(ns, fileno(stderr));

	  while (tries--)
	    {
	      /* exec() a program using the socket for I/O */
	      execl(prog, prog, NULL);

	      /* Program didn't run; try again in a few seconds */
	      if (tries)
		{
		  printf("We are experiencing technical difficulties; please hold.  (%d)\n", tries);
		  fflush(stdout);
		  sleep(EXEC_DELAY);
		}
	    }

	  /* Couldn't run the program!  Oh well! */
	  printf("\nWe seem to be having trouble right now.\nTry again later.\n\n");
	  fflush(stdout);
	  exit(1);
	}

      /* Wait for another connection */
      close(ns);
    }
}
