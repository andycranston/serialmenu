static char *version = "@(!--#) @(#) serialmenu.c, sversion 0.1.0, fversion 005, 01-july-2024";

/*
#define DEBUG
*/

/*
 * serialmenu.c
 *
 * display a menu for connecting to serial ports
 *
 */

/**********************************************************************/

/*
 *  includes
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

/**********************************************************************/

/*
 *  defines
 */

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define MAX_LINE_LENGTH 8191

#define CONFIG_FILENAME   "/usr/local/etc/serialmenu.conf"
#define LOGDIR            "/var/local/serial"
#define EXPECT_SCRIPT     "/usr/local/bin/serialconnect.exp"
#ifndef EXPECT_EXECUTABLE
#define EXPECT_EXECUTABLE "/usr/bin/expect"
#endif
#define EXPECT_NAME       "expect"

/**********************************************************************/

/*
 *  globals
 */

char *progname;

int   term;

/**********************************************************************/

/*
 *  data types
 */

struct menu {
  char   *opt;
  char   *dev;
  char   *speed;
  char   *desc;
  char   *cmd;
  struct menu *next;
};

/**********************************************************************/

/*
 *  writeterm
 */

void writeterm(s)
  char *s;
{
  int   lens;
  int   n;

  lens = strlen(s);

  n = write(term, s, lens);

  if (n != lens) {
    fprintf(stderr, "\n%s: write error (n=%d)\n", progname, n);
  }

  tcdrain(term);

  return;
}


/**********************************************************************/

/*
 *  rstrip
 *
 *  trim whitespace from right hand side of a string
 *
 */

void rstrip(s)
  char *s;
{
  int   lenstring;
  int   c;

  lenstring = strlen(s);

  while (lenstring > 0) {
    lenstring--;

    c = s[lenstring];

    if ((c == ' ') || (c == '\t') || (c == '\n') || (c == '\r')) {
      s[lenstring] = '\0';
    } else {
      break;
    }
  }

  return;
}

/**********************************************************************/

struct menu *readconfigfile(cfn)
  char *cfn;
{
  FILE *cf;
  char  line[MAX_LINE_LENGTH + sizeof(char)];
  int   linenum;
  char *token;
  char *opt;
  char *dev;
  char *speed;
  char *desc;
  char *cmd;
  struct menu *menuhead;
  struct menu *menunext;
  struct menu *menutail;

  if ((cf = fopen(cfn, "r")) == NULL) {
    return NULL;
  }

  linenum = 0;

  menuhead = NULL;

  while (fgets(line, MAX_LINE_LENGTH, cf) != NULL) {
    linenum++;

    if (line[0] == '#') {
      continue;
    }

    if (isspace(line[0])) {
      continue;
    }

    token = strtok(line, ":");

    if (token == NULL) {
      continue;
    }

    opt = token;

    token = strtok(NULL, ":");

    if (token == NULL) {
      fprintf(stderr, "%s: ignoring line %d in config file \"%s\" - no device parameter\n", progname, linenum, cfn);
      continue;
    }

    dev = token;

    token = strtok(NULL, ":");

    if (token == NULL) {
      fprintf(stderr, "%s: ignoring line %d in config file \"%s\" - no speed parameter\n", progname, linenum, cfn);
      continue;
    }

    speed = token;

    token = strtok(NULL, ":\n\r");

    if (token == NULL) {
      fprintf(stderr, "%s: ignoring line %d in config file \"%s\" - no description parameter\n", progname, linenum, cfn);
      continue;
    }

    desc = token;

    token = strtok(NULL, ":\n\r");

    if (token == NULL) {
      fprintf(stderr, "%s: ignoring line %d in config file \"%s\" - no cmd parameter\n", progname, linenum, cfn);
      continue;
    }

    cmd = token;

    menunext = malloc(sizeof(struct menu));

    menunext->opt   = strdup(opt);
    menunext->dev   = strdup(dev);
    menunext->speed = strdup(speed);
    menunext->desc  = strdup(desc);
    menunext->cmd   = strdup(cmd);
    menunext->next  = NULL;

    if (menuhead == NULL) {
      menuhead = menunext;
      menutail = menunext;
    } else {
      menutail->next = menunext;
      menutail = menunext;
    }

#ifdef DEBUF
    printf("[%s] [%s] [%s] [%s] [%s]\n", opt, dev, speed, desc, cmd);
#endif
  }

  fclose(cf);

  return menuhead;
}

/**********************************************************************/

int longestdesc(m)
  struct menu *m;
{
  int   longest;

  longest = 0;

  while (m != NULL) {
    if (strlen(m->desc) > longest) {
      longest = strlen(m->desc);
    }

    m = m->next;
  }

  return longest;
}

/**********************************************************************/

void padleft(s, w)
  char *s;
  int   w;
{
  int   padding;

  padding = w - strlen(s);

  while (padding > 0) {
    /* writeterm(" "); */
    putchar(' ');
    padding--;
  }

  /* writeterm(s); */
  printf("%s", s);

  return;
}

/**********************************************************************/

void padright(s, w)
  char *s;
  int   w;
{
  int   padding;

  padding = w - strlen(s);

  /* writeterm(s); */
  printf("%s", s);

  while (padding > 0) {
    /* writeterm(" "); */
    putchar(' ');
    padding--;
  }

  return;
}

/**********************************************************************/

char *retdevname(dev)
  char *dev;
{
  if (strncmp(dev, "/dev/", 5) == 0) {
    dev += 5;
  }

  if (strncmp(dev, "tty", 3) == 0) {
    dev += 3;
  }

  return dev;
}

/**********************************************************************/

void displaymenu(m)
  struct menu *m;
{
  int   longest;

  longest = longestdesc(m);

  /*
  writeterm("\n");
  writeterm("Serial Menu\n");
  writeterm("-----------\n");
  writeterm("\n");
  */
  printf("\n");
  printf("Serial Menu\n");
  printf("-----------\n");
  printf("\n");

  while (m != NULL) {
    /*
    padleft(m->opt, 4);
    writeterm(") ");
    padright(m->desc, longest);
    writeterm("   (");
    writeterm(retdevname(m->dev));
    writeterm(" - ");
    writeterm(m->speed);
    writeterm(")\n");
    */
    printf("%4s) ", m->opt);
    padright(m->desc, longest);
    printf("   (%s - %s)\n", retdevname(m->dev), m->speed);

    m = m->next;
  }

  padleft("0", 4);
  /*
  writeterm(") Exit\n");
  writeterm("\n");
  writeterm("Selection: ");
  */
  printf(") Exit\n");
  printf("\n");
  printf("Selection: ");
  fflush(stdout);

  return;
}

/**********************************************************************/

struct menu *findmenuoption(m, opt)
  struct menu *m;
  char        *opt;
{
  while (m != NULL) {
    if (strcmp(m->opt, opt) == 0) {
      return m;
    }

    m = m->next;
  }

  return NULL;
}

/**********************************************************************/

void buildspawncmd(spawncmd, cmd, dev, speed)
  char *spawncmd;
  char *cmd;
  char *dev;
  char *speed;
{
  char *s;
  char *d;
  int   i;

  s = cmd;
  d = spawncmd;

  while (*s != '\0') {
    if (*s == '%') {
      switch (*(s+1)) {
        case 'd':
          for (i = 0; i < strlen(dev); i++) {
            *d = dev[i];
            d++;
          }

          s++;
          s++;
          continue;
        case 'b':
          for (i = 0; i < strlen(speed); i++) {
            *d = speed[i];
            d++;
          }

          s++;
          s++;
          continue;
      }
    }

    *d = *s;

    s++;
    d++;
  }

  *d = '\0';

  return;
}

/**********************************************************************/

/*
 *  Main
 */

int main(argc, argv, envp)
  int   argc;
  char *argv[];
  char *envp[];
{
  char        *env;
  char        *shell;
  char        *homedir;
  struct termios termoptions;
  char *e;
  char         cfn[MAX_LINE_LENGTH + sizeof(char)];
  struct menu *mainmenu;
  struct menu *menuopt;
  char         *opt;
  int          lenopt;
  int          n;
  int          i;
  time_t       tnow;
  struct tm   *tmpointer;
  char         escript[MAX_LINE_LENGTH + sizeof(char)];
  char         basefile[MAX_LINE_LENGTH + sizeof(char)];
  char         timefile[MAX_LINE_LENGTH + sizeof(char)];
  char         logfile[MAX_LINE_LENGTH + sizeof(char)];
  char         spawncmd[MAX_LINE_LENGTH + sizeof(char)];
  char         scriptcmd[MAX_LINE_LENGTH + sizeof(char)];
  pid_t        forkpid;
  pid_t        waitpid;
  int          waitstatus;

  opt = malloc(MAX_LINE_LENGTH * sizeof(char));

  if (opt ==  NULL) {
    fprintf(stderr, "%s: unable to allocate memory for string variable \"opt\"\n", argv[0]);
    exit(2);
  }

#ifdef DEBUG
  fprintf(stderr, "\nargv[0]=\"%s\"\n", argv[0]);
  fflush(stderr);
#endif

#ifdef DEBUG
  i = 0;

  while (envp[i] != NULL) {
    fprintf(stderr, "%d %s\n", i, envp[i]);
    i++;
  }
#endif

  setenv("SHELL", "/bin/sh", 1);

#ifdef DEBUG
  i = 0;

  while (envp[i] != NULL) {
    fprintf(stderr, "%d %s\n", i, envp[i]);
    i++;
  }
#endif

  progname = argv[0];

  if ((homedir = getenv("HOME")) == NULL) {
    fprintf(stderr, "%s: unable to get value of HOME environment variable\n", progname);
    exit(1);
  }

  shell = getenv("SHELL");

#ifdef DEBUG
  if (shell == NULL) {
    fprintf(stderr, "\nSHELL is not defined\n");
    fflush(stderr);
  } else {
    fprintf(stderr, "\nSHELL=[%s]\n", shell);
    fflush(stderr);
  }
#endif
      
  /*
  term = open("/dev/tty", O_RDWR | O_NOCTTY);

  if (term == -1) {
    fprintf(stderr, "%s: cannot open /dev/tty terminal device\n", progname);
    exit(1);
  }

  tcgetattr(term, &termoptions);
  termoptions.c_lflag |= (ICANON | ECHO | ECHOE);
  tcsetattr(term, TCSANOW, &termoptions);
  */

  /*
  strncpy(cfn, homedir, MAX_LINE_LENGTH);
  strncat(cfn, "/", MAX_LINE_LENGTH);
  */
  strncat(cfn, CONFIG_FILENAME, MAX_LINE_LENGTH);

  if ((mainmenu = readconfigfile(cfn)) == NULL) {
    fprintf(stderr, "%s: unable to read config file \"%s\"\n", progname, cfn);
    exit(1);
  }

  while (TRUE) {
    displaymenu(mainmenu);

    /* lenopt = read(term, opt, MAX_LINE_LENGTH); */

    scanf("%s", opt);
    lenopt = strlen(opt);

    if (lenopt < 0) {
      break;
    }

    opt[lenopt] = '\0';

    rstrip(opt);

    if (isspace(opt[0])) {
      continue;
    }

    if (strcmp(opt, "0") == 0) {
      break;
    }

    menuopt = findmenuoption(mainmenu, opt);

    if (menuopt == NULL) {
      printf("\n*** Not found ***\n");
    } else {
      tnow = time(NULL);

      tmpointer = localtime(&tnow);

      snprintf(basefile, MAX_LINE_LENGTH, "%s/logs/%s-%04d%02d%02d-%02d%02d%02d",
                                          homedir,
                                          retdevname(menuopt->dev),
                                          1900 + (tmpointer->tm_year),
                                          1    + (tmpointer->tm_mon),
                                          tmpointer->tm_mday,
                                          tmpointer->tm_hour,
                                          tmpointer->tm_min,
                                          tmpointer->tm_sec);

      snprintf(timefile, MAX_LINE_LENGTH + 100, "-t%s.tim", basefile);

      snprintf(logfile, MAX_LINE_LENGTH + 100, "%s.log", basefile);

      buildspawncmd(spawncmd, menuopt->cmd, menuopt->dev, menuopt->speed);

#ifdef DEBUG
      printf("BASE=[%s]\n", basefile);
      printf("TIME=[%s]\n", timefile);
      printf("LOGG=[%s]\n", logfile);
      printf("SCMD=[%s]\n", spawncmd);
#endif

      forkpid = fork();

      if (forkpid < 0) {
        fprintf(stderr, "%s: fork() returned error (pid=%d)\n", progname, forkpid);
        exit(1);
      }

      if (forkpid == 0) {
        /* child */
        execl(EXPECT_EXECUTABLE, EXPECT_NAME, EXPECT_SCRIPT, spawncmd, menuopt->dev, LOGDIR, (char *)NULL);

        fprintf(stderr, "\n%s: execl call has failed\n", progname);

        exit(1);
      }

      /* parent */
#ifdef DEBUG
      fprintf(stderr, "\n\n%s: parent now waiting\n\n", progname);
      fflush(stderr);
#endif

      waitpid = wait(&waitstatus);

#ifdef DEBUG
      fprintf(stderr, "\n\n%s: parent finished waiting\n\n", progname);
      fprintf(stderr, "\n\n%s: child pid %d has wait status %d\n", progname, waitpid, waitstatus);
      fflush(stderr);
#endif

      /* now break out of menu and quit */
/*
      break;
*/
    }
  }

  return 0;
}

/* end of C source file: serial.c */
