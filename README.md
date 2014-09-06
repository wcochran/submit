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

  6. Edit `submit.html`

# Files
  
  * `Makefile` : build's cgi program (assumes `../cgi-bin` exists)
  * `submit.c` : cgi source code
  * `submit.html` : html source with proper form element