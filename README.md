# Programming Project Web Submission System

 * Author: Wayne O. Cochran 
 * Email: <a href="mailto:wcochran@vancouver.wsu.edu">wcochran@vancouver.wsu.edu</a>
 * Modification History
     - Original version written in 1998?
     
# Description
   
  HTML-FORM / CGI electronic submission system

# Steps to Install

  * This archive assumes the username is `cs330` (my numerical computing course)
      so it will need to be tailored for other classes.
  * Assumptions
      - Home directory lies in `~/Sites/submit`
      - Use the following directory structure 

       ~/Sites/submit      # source C / HTML here
       ~/Sites/cgi-bin     # CGI executable goes here
       ~/Sites/submissions # project arcchives placed here

  1. Make directories for cgi script if it doesn't exists

    mkdir ~/Sites/cgi-bin

  2. Configure Apache to allow `ExecCGI` for user in `cgi-bin` directory.

  3. Make directories for project submissions

    mkdir ~/Sites/submissions

  4. Edit defines at top of `submit.c` (`CLASS_NAME`, etc...)

  5. Build/install cgi program  (must be in `~/Sites/submit` dir)

    make

  6. Create list of possible projects in `../submissions/projects.txt`

  7. Edit `submit.html`

       - List of project options should be a subset of 
         `../submissions/projects.txt`.

  8. Add encrypted list of SID's (if you wish to force this check)

      # download class roster (call it `roster.html`) from zzusis
      ./roster.pl < roster.html > roster.csv
      chmod 600 roster.csv  # don't let others see this
      ./sidcrypt < roster.csv > ../submissions/sidcrypt.txt
      chmod 600 ../submissions/sidcrypt.txt  # good idea

# Files
  
  * `Makefile` : build's cgi program (assumes `../cgi-bin` exists)
  * `submit.c` : cgi source code
  * `submit.html` : html source with proper form elements
  * `roster.pl` : builds CSV file from roster downloaded via zzusis
  * `sidcrypt.pl` : encrypts student ID's for form verification
  * `sidhash.pl` : hashes student ID's for anonymous score posting