

  ______ ______         _____ ____  _   _ ______ _____ _____ 
 |  ____|___  /        / ____/ __ \| \ | |  ____|_   _/ ____|
 | |__     / /        | |   | |  | |  \| | |__    | || |  __ 
 |  __|   / /         | |   | |  | | . ` |  __|   | || | |_ |
 | |____ / /__  _____ | |___| |__| | |\  | |     _| || |__| |
 |______/_____||_____| \_____\____/|_| \_|_|    |_____\_____|                                 



This is a single header library to handle configuration-tables.


To use this library you simply have to include the header file and define
EZC_DEF a single time in one of your file(but only once in general).
And thats it. <3

These are the most important functions:

  void ezc_reset(void)
  
    Reset the configuration table. This should be called on the start and on
    the end of the use of the library.

 
  int ezc_parse(char *pth)

    With this function a configuration file can be loaded which should conform
    to the following format:

      # This is a comment
      newval1 = 12.17
      anotherval = helloworld
      serveraddr = ::1

     If everything has gone according to plan, this function will then return 0.
     In case of an error -1 will be returned.


  char *ezc_get(char *key)

    You can get the value of a keyword by using this function. If the key has
    been found, the according value will be returned as a nullterminated string.
    Otherwise NULL will be returned.


  int ezc_set(char *key, char *val)

    Through this function you can modify and set key-value-pairs in the table.
    If the key exists, it will be overwritten, if not then a new entry will be
    created.    


