/*
 * File submission CGI script.
 *
 * Note: ../submissions directory must have been created.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define CLASS_NAME "CS 330"
#define NAME "Wayne O. Cochran"
#define EMAIL "wcochran@vancouver.wsu.edu"
#define MAX_CONTENT_LENGTH 1e6L

static void fatalError(char *message) {
  printf("Content-type: text/html\n");
  printf("\n");
  printf("<html>\n");
  printf("<head>\n");
  printf("<title>%s Submission Fatal Error!</title>\n", CLASS_NAME);
  printf("</head>\n");
  printf("<body>\n");
  printf("<h3>%s Submission Fatal Error!</h3><p>\n", CLASS_NAME);
  printf("message: <i>%s</i>\n<br>", message);
  printf("Please contact <a href=\"mailto:%s\">", EMAIL);
  printf("%s (%s)</a> for assistance.\n", NAME, EMAIL);
  printf("Write down the error message printed above.\n");
  printf("</body>\n");
  printf("</html>\n");
  exit(2);
}
  
static void error(char *message) {
  printf("Content-type: text/html\n");
  printf("\n");
  printf("<html>\n");
  printf("<head>\n");
  printf("<title>%s Submission failed :(</title>\n", CLASS_NAME);
  printf("</head>\n");
  printf("<body>\n");
  printf("<h3>%s Submission failed :(</h3><p>\n", CLASS_NAME);
  printf("Reason: <i>%s</i>\n<br>", message);
  printf("Please contact <a href=\"mailto:%s\">", EMAIL);
  printf("%s (%s)</a> for assistance.\n", NAME, EMAIL);
  printf("</body>\n");
  printf("</html>\n");
  exit(1);
}

static void success(char *name, char *id, char *assign, 
		    char *fname, char *local_fname,
		    char *timestamp) {
  printf("Content-type: text/html\n");
  printf("\n");
  printf("<html>\n");
  printf("<head>\n");
  printf("<title>%s Submission Successful!</title>\n", CLASS_NAME);
  printf("</head>\n");
  printf("<body>\n");
  printf("<h3>%s Submission Successful!</h3><p>\n", CLASS_NAME);
  printf("Name: <i>%s</i>\n<br>",name);
  printf("ID: <i>%s</i>\n<br>", id);
  printf("Assignment: <i>%s</i>\n<br>", assign);
  printf("File Name: <i>%s</i>\n<br>", fname);
  printf("Local File Name: <i>%s</i>\n<br>", local_fname);
  printf("Time Stamp: <i>%s</i>\n<p>", timestamp);
  printf("<hr>\n");
  printf("<a href=\"mailto:%s\">", EMAIL);
  printf("%s</a><br>\n", NAME);
  printf("<i>%s %s</i>\n", __DATE__, __TIME__);
  printf("</body>\n");
  printf("</html>\n");
}

#define BUFSIZE 1024
#define DIRBUFSIZE 200
#define BOUNDARYSIZE 200

#define MAXNAME 81
#define MAXSID 9
#define MAXASSIGN 20

#define MAXFNAME 200

static char buf[BUFSIZE];
static char dirbuf[DIRBUFSIZE];
static int boundaryLength;
static char boundary[BOUNDARYSIZE];

static int validBoundary(char s[]) {
  return strncmp("--", s, 2) == 0;
}

static int validSID(char *sid) {
  int i;
  for (i = 0; i < 8; i++)
    if (sid[i] < '0' || sid[i] > '9')
      return 0;
  return (i < 9);
}

static int checkSID(char *sid) {
  FILE *f = fopen("../submissions/sidcrypt.txt", "r");
  static char buf[100];
  if (f == NULL)
    return 1;   /* no encrypted list, let everyone in */
  while (fgets(buf, sizeof(buf), f) != NULL && strlen(buf) > 10) {
    char salt[3];
    char *cypher;
    strtok(buf,"\n");  /* chop off linefeed */
    strncpy(salt, buf, 2);
    salt[2] = '\0';
    cypher = crypt(sid, salt);
    if (strcmp(buf, cypher) == 0) {
      fclose(f);
      return 1;  /* found */
    }
  }
  fclose(f);
  return 0; /* not found */
}

static int checkAssign(char *assign_name) {
  FILE *f = fopen("../submissions/projects.txt", "r");
  static char buf[100];
  if (f == NULL)
    fatalError("Unable to open projects.txt\n");     
  while (fgets(buf, sizeof(buf), f) != NULL && strlen(buf) > 0) {
    strtok(buf,"\n");  /* chop off linefeed */
    if (strcmp(assign_name,buf) == 0) {
      fclose(f);
      return 1;  /* found */
    }
  }
  fclose(f);
  return 0; /* not found */
}

static int validName(char *name) {
  int i, nonwhite;
  for (i = nonwhite = 0; name[i] != '\0'; i++)
    if (name[i] != ' '  && name[i] != '\t' && 
	name[i] != '\r' && name[i] !='\n')
      nonwhite++;
  return (nonwhite > 2);
}

static char *removeWhiteSpace(char *s) {
  char *orig = s;
  while (*s != '\0') {
    if (*s == ' ' || *s == '\t') {
      char *t, *p;
      p = t = s;
      while (*(++p) != '\0')
	*(t++) = *p;
      *t = '\0';
    } else
      s++;
  }
  return orig;
}

#define FBUFSIZE 2048

static void copyFile(FILE *src, FILE *dst, long numBytes) {
  static char fbuf[BUFSIZE+1];
  
  while (numBytes > 0) {
    long bytes = (numBytes < BUFSIZE) ? numBytes : BUFSIZE;
    long bytesRead = fread(fbuf, 1, bytes, src);
    if (bytesRead != bytes) {
      sprintf(fbuf, "Error copying file while reading source (%ld != %ld).",
	      bytes, bytesRead);
      fatalError(fbuf);
    }
    bytes = fwrite(fbuf, 1, bytesRead, dst);
    if (bytes != bytesRead) {
      sprintf(fbuf, 
	      "Error copying file while writing destination (%ld != %ld)",
	      bytes, bytesRead);
      fatalError(fbuf);
    }
    numBytes -= bytes;
  }
} 

typedef enum {
  FORM_ERROR, 
  FORM_NAME, 
  FORM_SID, 
  FORM_ASSIGN, 
  FORM_SUBMISSION
} FORM_TYPE;

FORM_TYPE parseFormHeader(char *str, char *fname) {
  char *content[5];
  int i,n, gotfname;

  if (strtok(str, ";\r\n") == NULL) 
    return FORM_ERROR;

  for (n = 0; n < 5; n++)
    if ((content[n] = strtok(NULL, ";\r\n")) == NULL)
      break;

  if (n <= 0)
    return FORM_ERROR;

  for (i = gotfname = 0; i < n; i++) {
    char *field, *param;
    char *s = content[i];
    while (*s == ' ' || *s == '\t')
      s++;
    if ((field = strtok(s,"\"")) == NULL ||
	(param = strtok(NULL,"\"")) == NULL)
      return FORM_ERROR;
    if (strcmp("name=",field) == 0) {
      if (strcmp("name",param) == 0)
	return FORM_NAME;
      if (strcmp("sid",param) == 0)
	return FORM_SID;
      if (strcmp("assign",param) == 0)
	return FORM_ASSIGN;
      if (strcmp("submission",param) != 0)
	return FORM_ERROR;
    } else if (strcmp("filename=",field) == 0) {
      gotfname = 1;
      strncpy(fname, param, MAXFNAME);
    }
  }

  return (gotfname) ? FORM_SUBMISSION : FORM_ERROR;
}

#define GOTNAME   0x01
#define GOTSID    0x02
#define GOTASSIGN 0x04
#define GOTFILE   0x08
#define GOTALL    0x0F
    
int main(int argc, char *argv[]) {
  FORM_TYPE type;
  static char msg[50];
  int i, fd, lfd;
  FILE *f, *ftmp;
  char *tmp_fname = NULL;
  char *ext;
  char *timeStamp;
  int goodext;
  struct stat statbuf;
  time_t tp;
  unsigned got = 0;
  long bytesWritten = 0;
  int attempts;
  static struct {
    char name[MAXNAME+1];
    char sid[MAXSID+1];
    char assign[MAXASSIGN+1];
    char fname[MAXFNAME+1];
  } info;
  char *env;
  long contentLength;

  if ((env = getenv("CONTENT_LENGTH")) == NULL)
    fatalError("No environment variable CONTENT_LENGTH.");
  contentLength = atol(env);
  if (contentLength > MAX_CONTENT_LENGTH)
    error("Submission too dang big!");  
  if (contentLength < 200)
    error("Submission too small -- check your archive!");

  if (fgets(boundary, BOUNDARYSIZE, stdin) == NULL)
    fatalError("No data from form.");    
  if (!validBoundary(boundary))
    fatalError(boundary);
  strtok(boundary,"\r\n");  /* chop off at carriage return or linefeed */
  boundaryLength = strlen(boundary);

  while (1) {
    int readBoundary = 0;

    if (fgets(buf, BUFSIZE, stdin) == NULL)
      fatalError("Unexpected end of input. Expecting form content header.");
    if ((type = parseFormHeader(buf, info.fname)) == FORM_ERROR)
      fatalError("Invalid form header.");
    
    do {
      if (fgets(buf, BUFSIZE, stdin) == NULL)
	fatalError("Unexpected end of input. Expecting empty line.");
    } while(buf[0] != '\r' && buf[0] != '\n');
    
    switch(type) {
    case FORM_ERROR:
      fatalError("bogus cgi script!");
    case FORM_NAME:
      if (fgets(buf, BUFSIZE, stdin) == NULL)
	fatalError("Unexpected end of input. Expecting user name.");
      strtok(buf,"\r\n");
      strncpy(info.name,buf,MAXNAME+1);
      if (!validName(info.name)) {
	sprintf(msg, "The Name '%s' is not valid.", info.name);
	error(msg);
      }
      got |= GOTNAME;
      break;
    case FORM_SID:
      if (fgets(buf, BUFSIZE, stdin) == NULL)
	fatalError("Unexpected end of input. Expecting user ID.");
      strtok(buf,"\r\n");
      strncpy(info.sid,buf,MAXSID+1);
      if (!validSID(info.sid)) {
	sprintf(msg, "The ID number '%s' is not valid.", info.sid);
	error(msg);
      }
      if (!checkSID(info.sid)) {
	sprintf(msg, "No student with ID number '%s' is enrolled.", info.sid);
	error(msg);
      }
      got |= GOTSID;
      break;
    case FORM_ASSIGN:
      if (fgets(buf, BUFSIZE, stdin) == NULL)
	fatalError("Unexpected end of input. Expecting assignment name.");
      strtok(buf,"\r\n");      
      strncpy(info.assign,buf,MAXASSIGN+1);
      removeWhiteSpace(info.assign);
      if (!checkAssign(info.assign)) {
	sprintf(msg, "No project named '%s'.", info.assign);
	error(msg);
      }
      got |= GOTASSIGN;
      break;
    case FORM_SUBMISSION: 
      tmp_fname = tempnam(".","sub");
      if (tmp_fname == NULL)
	fatalError("Unable to create temporary file name!");
      if ((fd = open(tmp_fname, O_CREAT | O_WRONLY, 0600)) < 0)
	fatalError("Unable to open temporary file!");
      f = fdopen(fd,"wb");
      if (f == NULL)
	fatalError("Error opening file for writing.");
      while(1) {
	int eol;
	if (fgets(buf, BUFSIZE, stdin) == NULL) {
	  fclose(f);
	  unlink(tmp_fname);
	  fatalError("Unexpected end of input. Expecting boundary.");
	}
	if (strncmp(buf,boundary,boundaryLength) == 0) {
	  readBoundary = 1;
	  break;
	}
	for (eol = 0; eol < BUFSIZE-1; eol++)
	  if (buf[eol] == '\n')
	    break;
	if (eol >= BUFSIZE-1) {
	  if (fwrite(buf, 1, BUFSIZE-1, f) != BUFSIZE-1) {
	    fclose(f);
	    unlink(tmp_fname);
	    fatalError("Error writing file.");
	  }
	  bytesWritten += BUFSIZE-1;
	} else {
	  eol++;
	  if (fwrite(buf, 1, eol, f) != eol) {
	    fclose(f);
	    unlink(tmp_fname);
	    fatalError("Error writing file.");
	  }
	  bytesWritten += eol;
	}
      }
      fclose(f);
      if (bytesWritten <= 2) {
	sprintf(msg, "Empty file: '%s'!", info.fname);
	error(msg);
      }
      got |= GOTFILE;
    }

    if (!readBoundary) {
      if (fgets(buf, BUFSIZE, stdin) == NULL) {
	unlink(tmp_fname);
	fatalError("Unexpected end of input. Expecting Boundary.");
      }
      if (strncmp(buf,boundary,boundaryLength) != 0) {
	unlink(tmp_fname);
	fatalError("Bogus boundary.");
      }
    }
    if (strncmp(buf+boundaryLength,"--",2) == 0)
      break;
  }

  if (got != GOTALL) {
    if (tmp_fname != NULL)
      unlink(tmp_fname);
    error("You must fill out the entire form.");
  }

  goodext = 0;

  if((ext = strrchr(info.fname,'.')) != NULL) {
    if (strcmp(".zip", ext) == 0 || strcmp(".ZIP", ext) == 0 ||
	strcmp(".jar", ext) == 0 || strcmp(".JAR", ext) == 0)
      goodext = 1;
    else {
      while (ext > info.fname && *(--ext) != '.')
	;
      if (strcmp(".tar.Z", ext) == 0 || strcmp(".tar.gz", ext) == 0)
	goodext = 1;
    }
  }

  if (!goodext) {
    unlink(tmp_fname);
    error("Submitted file must have a " 
	  "'.tar.gz', '.zip', or '.jar'  extension.");
  }

  sprintf(dirbuf,"../submissions/%s", info.assign);
  if (mkdir(dirbuf,0700) < 0 && (errno != EEXIST)) {
    unlink(tmp_fname);
    fatalError(dirbuf);
  }
  for (attempts = 1; attempts <= 20; attempts++) {
    sprintf(buf,"%s/%s-%03d%s", dirbuf, info.sid, attempts, ext);
    if (stat(buf, &statbuf) < 0) {
      if (errno == ENOENT)
	break;
      else {
	unlink(tmp_fname);
	fatalError(buf);
      }
    }
  }

  if (attempts > 20) {
    unlink(tmp_fname);
    error("Too many submissions (> 20) for this assignment.");
  }

  strcpy(dirbuf,buf);
  if ((fd = open(dirbuf, O_CREAT | O_WRONLY, 0600)) < 0 ||
      (f = fdopen(fd, "wb")) == NULL) {
    unlink(tmp_fname);
    fatalError("Unable to create permanent file.");
  }

  if ((ftmp = fopen(tmp_fname,"rb")) == NULL) {
    unlink(tmp_fname);
    fatalError("Unable to reopen tmp file");
  }

  copyFile(ftmp, f, bytesWritten-2);  /* chop of CR/LF at end */
  fclose(f);
  fclose(ftmp);

  lfd = open("../submissions/log", O_CREAT | O_WRONLY | O_APPEND, 0700);
  if (lfd < 0) {
    unlink(tmp_fname);
    fatalError("Unable to open log file.");
  }

  if ((f = fdopen(lfd, "a")) == NULL) {
    unlink(tmp_fname);
    fatalError("Unable to open log file (from file desc).");
  }

  time(&tp);
  timeStamp = ctime(&tp);

  i = strlen(timeStamp)-1;
  if (timeStamp[i] == '\n')
    timeStamp[i] = '\0';

  fprintf(f, "name='%s' sid=%s asn=%s  rname='%s' fname='%s' time='%s'\n",
	  info.name, info.sid, info.assign,
	  info.fname, dirbuf, timeStamp);
  fclose(f);

  unlink(tmp_fname);

  success(info.name, info.sid, info.assign, 
	  info.fname, dirbuf, timeStamp);

  return 0;
}
